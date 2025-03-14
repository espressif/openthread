/*
 *  Copyright (c) 2019, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file includes the implementation for the SPI interface to radio (RCP).
 */

#include "spi_interface.hpp"

#include "platform-posix.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/ucontext.h>

#include "common/code_utils.hpp"

#if OPENTHREAD_POSIX_CONFIG_SPINEL_SPI_INTERFACE_ENABLE
#include <linux/gpio.h>
#include <linux/ioctl.h>
#include <linux/spi/spidev.h>

namespace ot {
namespace Posix {

const char SpiInterface::kLogModuleName[] = "SpiIntface";

SpiInterface::SpiInterface(const Url::Url &aRadioUrl)
    : mReceiveFrameCallback(nullptr)
    , mReceiveFrameContext(nullptr)
    , mRxFrameBuffer(nullptr)
    , mRadioUrl(aRadioUrl)
    , mSpiDevFd(-1)
    , mResetGpioValueFd(-1)
    , mIntGpioValueFd(-1)
    , mSlaveResetCount(0)
    , mSpiDuplexFrameCount(0)
    , mSpiUnresponsiveFrameCount(0)
    , mSpiTxIsReady(false)
    , mSpiTxRefusedCount(0)
    , mSpiTxPayloadSize(0)
    , mDidPrintRateLimitLog(false)
    , mSpiSlaveDataLen(0)
    , mDidRxFrame(false)
{
}

void SpiInterface::ResetStates(void)
{
    mSpiTxIsReady         = false;
    mSpiTxRefusedCount    = 0;
    mSpiTxPayloadSize     = 0;
    mDidPrintRateLimitLog = false;
    mSpiSlaveDataLen      = 0;
    memset(mSpiTxFrameBuffer, 0, sizeof(mSpiTxFrameBuffer));
    memset(&mInterfaceMetrics, 0, sizeof(mInterfaceMetrics));
    mInterfaceMetrics.mRcpInterfaceType = kSpinelInterfaceTypeSpi;
}

otError SpiInterface::HardwareReset(void)
{
    ResetStates();
    TriggerReset();

    // If the `INT` pin is set to low during the restart of the RCP chip, which triggers continuous invalid SPI
    // transactions by the host, it will cause the function `PushPullSpi()` to output lots of invalid warn log
    // messages. Adding the delay here is used to wait for the RCP chip starts up to avoid outputting invalid
    // log messages.
    usleep(static_cast<useconds_t>(mSpiResetDelay) * kUsecPerMsec);

    return OT_ERROR_NONE;
}

otError SpiInterface::Init(ReceiveFrameCallback aCallback, void *aCallbackContext, RxFrameBuffer &aFrameBuffer)
{
    const char *spiGpioIntDevice;
    const char *spiGpioResetDevice;
    uint8_t     spiGpioIntLine     = 0;
    uint8_t     spiGpioResetLine   = 0;
    uint8_t     spiMode            = OT_PLATFORM_CONFIG_SPI_DEFAULT_MODE;
    uint32_t    spiSpeed           = SPI_IOC_WR_MAX_SPEED_HZ;
    uint32_t    spiResetDelay      = OT_PLATFORM_CONFIG_SPI_DEFAULT_RESET_DELAY_MS;
    uint16_t    spiCsDelay         = OT_PLATFORM_CONFIG_SPI_DEFAULT_CS_DELAY_US;
    uint8_t     spiAlignAllowance  = OT_PLATFORM_CONFIG_SPI_DEFAULT_ALIGN_ALLOWANCE;
    uint8_t     spiSmallPacketSize = OT_PLATFORM_CONFIG_SPI_DEFAULT_SMALL_PACKET_SIZE;

    spiGpioIntDevice   = mRadioUrl.GetValue("gpio-int-device");
    spiGpioResetDevice = mRadioUrl.GetValue("gpio-reset-device");

    VerifyOrDie(spiGpioIntDevice, OT_EXIT_INVALID_ARGUMENTS);
    SuccessOrDie(mRadioUrl.ParseUint8("gpio-int-line", spiGpioIntLine));
    VerifyOrDie(mRadioUrl.ParseUint8("spi-mode", spiMode) != OT_ERROR_INVALID_ARGS, OT_EXIT_INVALID_ARGUMENTS);
    VerifyOrDie(mRadioUrl.ParseUint32("spi-speed", spiSpeed) != OT_ERROR_INVALID_ARGS, OT_EXIT_INVALID_ARGUMENTS);
    VerifyOrDie(mRadioUrl.ParseUint32("spi-reset-delay", spiResetDelay) != OT_ERROR_INVALID_ARGS,
                OT_EXIT_INVALID_ARGUMENTS);
    VerifyOrDie(mRadioUrl.ParseUint16("spi-cs-delay", spiCsDelay) != OT_ERROR_INVALID_ARGS, OT_EXIT_INVALID_ARGUMENTS);
    VerifyOrDie(mRadioUrl.ParseUint8("spi-align-allowance", spiAlignAllowance) != OT_ERROR_INVALID_ARGS,
                OT_EXIT_INVALID_ARGUMENTS);
    VerifyOrDie(mRadioUrl.ParseUint8("spi-small-packet", spiSmallPacketSize) != OT_ERROR_INVALID_ARGS,
                OT_EXIT_INVALID_ARGUMENTS);
    VerifyOrDie(spiAlignAllowance <= kSpiAlignAllowanceMax, OT_EXIT_INVALID_ARGUMENTS);

    mSpiResetDelay      = spiResetDelay;
    mSpiCsDelayUs       = spiCsDelay;
    mSpiSmallPacketSize = spiSmallPacketSize;
    mSpiAlignAllowance  = spiAlignAllowance;

    InitIntPin(spiGpioIntDevice, spiGpioIntLine);

    if (spiGpioResetDevice)
    {
        SuccessOrDie(mRadioUrl.ParseUint8("gpio-reset-line", spiGpioResetLine));
        InitResetPin(spiGpioResetDevice, spiGpioResetLine);
    }
    else
    {
        LogNote("gpio-reset-device is not given.");
    }

    InitSpiDev(mRadioUrl.GetPath(), spiMode, spiSpeed);

    mReceiveFrameCallback = aCallback;
    mReceiveFrameContext  = aCallbackContext;
    mRxFrameBuffer        = &aFrameBuffer;

    return OT_ERROR_NONE;
}

SpiInterface::~SpiInterface(void) { Deinit(); }

void SpiInterface::Deinit(void)
{
    if (mSpiDevFd >= 0)
    {
        close(mSpiDevFd);
        mSpiDevFd = -1;
    }

    if (mResetGpioValueFd >= 0)
    {
        close(mResetGpioValueFd);
        mResetGpioValueFd = -1;
    }

    if (mIntGpioValueFd >= 0)
    {
        close(mIntGpioValueFd);
        mIntGpioValueFd = -1;
    }

    mReceiveFrameCallback = nullptr;
    mReceiveFrameContext  = nullptr;
    mRxFrameBuffer        = nullptr;
}

int SpiInterface::SetupGpioHandle(int aFd, uint8_t aLine, uint32_t aHandleFlags, const char *aLabel)
{
    struct gpiohandle_request req;
    int                       ret;

    assert(strlen(aLabel) < sizeof(req.consumer_label));

    req.flags             = aHandleFlags;
    req.lines             = 1;
    req.lineoffsets[0]    = aLine;
    req.default_values[0] = 1;

    snprintf(req.consumer_label, sizeof(req.consumer_label), "%s", aLabel);

    VerifyOrDie((ret = ioctl(aFd, GPIO_GET_LINEHANDLE_IOCTL, &req)) != -1, OT_EXIT_ERROR_ERRNO);

    return req.fd;
}

int SpiInterface::SetupGpioEvent(int         aFd,
                                 uint8_t     aLine,
                                 uint32_t    aHandleFlags,
                                 uint32_t    aEventFlags,
                                 const char *aLabel)
{
    struct gpioevent_request req;
    int                      ret;

    assert(strlen(aLabel) < sizeof(req.consumer_label));

    req.lineoffset  = aLine;
    req.handleflags = aHandleFlags;
    req.eventflags  = aEventFlags;
    snprintf(req.consumer_label, sizeof(req.consumer_label), "%s", aLabel);

    VerifyOrDie((ret = ioctl(aFd, GPIO_GET_LINEEVENT_IOCTL, &req)) != -1, OT_EXIT_ERROR_ERRNO);

    return req.fd;
}

void SpiInterface::SetGpioValue(int aFd, uint8_t aValue)
{
    struct gpiohandle_data data;

    data.values[0] = aValue;
    VerifyOrDie(ioctl(aFd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) != -1, OT_EXIT_ERROR_ERRNO);
}

uint8_t SpiInterface::GetGpioValue(int aFd)
{
    struct gpiohandle_data data;

    VerifyOrDie(ioctl(aFd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) != -1, OT_EXIT_ERROR_ERRNO);
    return data.values[0];
}

void SpiInterface::InitResetPin(const char *aCharDev, uint8_t aLine)
{
    char label[] = "SOC_THREAD_RESET";
    int  fd;

    LogDebg("InitResetPin: charDev=%s, line=%" PRIu8, aCharDev, aLine);

    VerifyOrDie(aCharDev != nullptr, OT_EXIT_INVALID_ARGUMENTS);
    VerifyOrDie((fd = open(aCharDev, O_RDWR)) != -1, OT_EXIT_ERROR_ERRNO);
    mResetGpioValueFd = SetupGpioHandle(fd, aLine, GPIOHANDLE_REQUEST_OUTPUT, label);

    close(fd);
}

void SpiInterface::InitIntPin(const char *aCharDev, uint8_t aLine)
{
    char label[] = "THREAD_SOC_INT";
    int  fd;

    LogDebg("InitIntPin: charDev=%s, line=%" PRIu8, aCharDev, aLine);

    VerifyOrDie(aCharDev != nullptr, OT_EXIT_INVALID_ARGUMENTS);
    VerifyOrDie((fd = open(aCharDev, O_RDWR)) != -1, OT_EXIT_ERROR_ERRNO);

    mIntGpioValueFd = SetupGpioEvent(fd, aLine, GPIOHANDLE_REQUEST_INPUT, GPIOEVENT_REQUEST_FALLING_EDGE, label);

    close(fd);
}

void SpiInterface::InitSpiDev(const char *aPath, uint8_t aMode, uint32_t aSpeed)
{
    const uint8_t wordBits = kSpiBitsPerWord;
    int           fd;

    LogDebg("InitSpiDev: path=%s, mode=%" PRIu8 ", speed=%" PRIu32, aPath, aMode, aSpeed);

    VerifyOrDie((aPath != nullptr) && (aMode <= kSpiModeMax), OT_EXIT_INVALID_ARGUMENTS);
    VerifyOrDie((fd = open(aPath, O_RDWR | O_CLOEXEC)) != -1, OT_EXIT_ERROR_ERRNO);
    VerifyOrExit(ioctl(fd, SPI_IOC_WR_MODE, &aMode) != -1, LogError("ioctl(SPI_IOC_WR_MODE)"));
    VerifyOrExit(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &aSpeed) != -1, LogError("ioctl(SPI_IOC_WR_MAX_SPEED_HZ)"));
    VerifyOrExit(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &wordBits) != -1, LogError("ioctl(SPI_IOC_WR_BITS_PER_WORD)"));
    VerifyOrExit(flock(fd, LOCK_EX | LOCK_NB) != -1, LogError("flock"));

    mSpiDevFd   = fd;
    mSpiMode    = aMode;
    mSpiSpeedHz = aSpeed;
    fd          = -1;

exit:
    if (fd >= 0)
    {
        close(fd);
    }
}

void SpiInterface::TriggerReset(void)
{
    VerifyOrDie(mResetGpioValueFd >= 0, OT_EXIT_RCP_RESET_REQUIRED);

    // Set Reset pin to low level.
    SetGpioValue(mResetGpioValueFd, 0);

    usleep(kResetHoldOnUsec);

    // Set Reset pin to high level.
    SetGpioValue(mResetGpioValueFd, 1);

    LogNote("Triggered hardware reset");
}

uint8_t *SpiInterface::GetRealRxFrameStart(uint8_t *aSpiRxFrameBuffer, uint8_t aAlignAllowance, uint16_t &aSkipLength)
{
    uint8_t       *start = aSpiRxFrameBuffer;
    const uint8_t *end   = aSpiRxFrameBuffer + aAlignAllowance;

    for (; start != end && ((start[0] == 0xff) || (start[0] == 0x00)); start++)
        ;

    aSkipLength = static_cast<uint16_t>(start - aSpiRxFrameBuffer);

    return start;
}

otError SpiInterface::DoSpiTransfer(uint8_t *aSpiRxFrameBuffer, uint32_t aTransferLength)
{
    int                     ret;
    struct spi_ioc_transfer transfer[2];

    memset(&transfer[0], 0, sizeof(transfer));

    // This part is the delay between C̅S̅ being asserted and the SPI clock
    // starting. This is not supported by all Linux SPI drivers.
    transfer[0].tx_buf        = 0;
    transfer[0].rx_buf        = 0;
    transfer[0].len           = 0;
    transfer[0].speed_hz      = mSpiSpeedHz;
    transfer[0].delay_usecs   = mSpiCsDelayUs;
    transfer[0].bits_per_word = kSpiBitsPerWord;
    transfer[0].cs_change     = false;

    // This part is the actual SPI transfer.
    transfer[1].tx_buf        = reinterpret_cast<uintptr_t>(mSpiTxFrameBuffer);
    transfer[1].rx_buf        = reinterpret_cast<uintptr_t>(aSpiRxFrameBuffer);
    transfer[1].len           = aTransferLength;
    transfer[1].speed_hz      = mSpiSpeedHz;
    transfer[1].delay_usecs   = 0;
    transfer[1].bits_per_word = kSpiBitsPerWord;
    transfer[1].cs_change     = false;

    if (mSpiCsDelayUs > 0)
    {
        // A C̅S̅ delay has been specified. Start transactions with both parts.
        ret = ioctl(mSpiDevFd, SPI_IOC_MESSAGE(2), &transfer[0]);
    }
    else
    {
        // No C̅S̅ delay has been specified, so we skip the first part because it causes some SPI drivers to croak.
        ret = ioctl(mSpiDevFd, SPI_IOC_MESSAGE(1), &transfer[1]);
    }

    if (ret != -1)
    {
        otDumpDebgPlat("SPI-TX", mSpiTxFrameBuffer, static_cast<uint16_t>(transfer[1].len));
        otDumpDebgPlat("SPI-RX", aSpiRxFrameBuffer, static_cast<uint16_t>(transfer[1].len));

        mInterfaceMetrics.mTransferredFrameCount++;
    }

    return (ret < 0) ? OT_ERROR_FAILED : OT_ERROR_NONE;
}

otError SpiInterface::PushPullSpi(void)
{
    otError          error               = OT_ERROR_FAILED;
    uint16_t         spiTransferBytes    = 0;
    uint8_t          successfulExchanges = 0;
    bool             discardRxFrame      = true;
    uint8_t         *spiRxFrameBuffer;
    uint8_t         *spiRxFrame;
    uint8_t          slaveHeader;
    uint16_t         slaveAcceptLen;
    Spinel::SpiFrame txFrame(mSpiTxFrameBuffer);
    uint16_t         skipAlignAllowanceLength;

    VerifyOrExit((mReceiveFrameCallback != nullptr) && (mRxFrameBuffer != nullptr), error = OT_ERROR_INVALID_STATE);

    if (mInterfaceMetrics.mTransferredValidFrameCount == 0)
    {
        // Set the reset flag to indicate to our slave that we are coming up from scratch.
        txFrame.SetHeaderFlagByte(true);
    }
    else
    {
        txFrame.SetHeaderFlagByte(false);
    }

    // Zero out our rx_accept and our data_len for now.
    txFrame.SetHeaderAcceptLen(0);
    txFrame.SetHeaderDataLen(0);

    // Sanity check.
    if (mSpiSlaveDataLen > kMaxFrameSize)
    {
        mSpiSlaveDataLen = 0;
    }

    if (mSpiTxIsReady)
    {
        // Go ahead and try to immediately send a frame if we have it queued up.
        txFrame.SetHeaderDataLen(mSpiTxPayloadSize);

        spiTransferBytes = OT_MAX(spiTransferBytes, mSpiTxPayloadSize);
    }

    if (mSpiSlaveDataLen != 0)
    {
        // In a previous transaction the slave indicated it had something to send us. Make sure our transaction
        // is large enough to handle it.
        spiTransferBytes = OT_MAX(spiTransferBytes, mSpiSlaveDataLen);
    }
    else
    {
        // Set up a minimum transfer size to allow small frames the slave wants to send us to be handled in a
        // single transaction.
        spiTransferBytes = OT_MAX(spiTransferBytes, mSpiSmallPacketSize);
    }

    txFrame.SetHeaderAcceptLen(spiTransferBytes);

    // Set skip length to make MultiFrameBuffer to reserve a space in front of the frame buffer.
    SuccessOrExit(error = mRxFrameBuffer->SetSkipLength(kSpiFrameHeaderSize));

    // Check whether the remaining frame buffer has enough space to store the data to be received.
    VerifyOrExit(mRxFrameBuffer->GetFrameMaxLength() >= spiTransferBytes + mSpiAlignAllowance);

    // Point to the start of the reserved buffer.
    spiRxFrameBuffer = mRxFrameBuffer->GetFrame() - kSpiFrameHeaderSize;

    // Set the total number of bytes to be transmitted.
    spiTransferBytes += kSpiFrameHeaderSize + mSpiAlignAllowance;

    // Perform the SPI transaction.
    error = DoSpiTransfer(spiRxFrameBuffer, spiTransferBytes);

    if (error != OT_ERROR_NONE)
    {
        LogCrit("PushPullSpi:DoSpiTransfer: errno=%s", strerror(errno));

        // Print out a helpful error message for a common error.
        if ((mSpiCsDelayUs != 0) && (errno == EINVAL))
        {
            LogWarn("SPI ioctl failed with EINVAL. Try adding `--spi-cs-delay=0` to command line arguments.");
        }

        LogStats();
        DieNow(OT_EXIT_FAILURE);
    }

    // Account for misalignment (0xFF or 0x00 bytes at the start)
    spiRxFrame = GetRealRxFrameStart(spiRxFrameBuffer, mSpiAlignAllowance, skipAlignAllowanceLength);

    {
        Spinel::SpiFrame rxFrame(spiRxFrame);

        LogDebg("spi_transfer TX: H:%02X ACCEPT:%" PRIu16 " DATA:%" PRIu16, txFrame.GetHeaderFlagByte(),
                txFrame.GetHeaderAcceptLen(), txFrame.GetHeaderDataLen());
        LogDebg("spi_transfer RX: H:%02X ACCEPT:%" PRIu16 " DATA:%" PRIu16, rxFrame.GetHeaderFlagByte(),
                rxFrame.GetHeaderAcceptLen(), rxFrame.GetHeaderDataLen());

        slaveHeader = rxFrame.GetHeaderFlagByte();
        if ((slaveHeader == 0xFF) || (slaveHeader == 0x00))
        {
            if ((slaveHeader == spiRxFrame[1]) && (slaveHeader == spiRxFrame[2]) && (slaveHeader == spiRxFrame[3]) &&
                (slaveHeader == spiRxFrame[4]))
            {
                // Device is off or in a bad state. In some cases may be induced by flow control.
                if (mSpiSlaveDataLen == 0)
                {
                    LogDebg("Slave did not respond to frame. (Header was all 0x%02X)", slaveHeader);
                }
                else
                {
                    LogWarn("Slave did not respond to frame. (Header was all 0x%02X)", slaveHeader);
                }

                mSpiUnresponsiveFrameCount++;
            }
            else
            {
                // Header is full of garbage
                mInterfaceMetrics.mTransferredGarbageFrameCount++;

                LogWarn("Garbage in header : %02X %02X %02X %02X %02X", spiRxFrame[0], spiRxFrame[1], spiRxFrame[2],
                        spiRxFrame[3], spiRxFrame[4]);
                otDumpDebgPlat("SPI-TX", mSpiTxFrameBuffer, spiTransferBytes);
                otDumpDebgPlat("SPI-RX", spiRxFrameBuffer, spiTransferBytes);
            }

            mSpiTxRefusedCount++;
            ExitNow();
        }

        slaveAcceptLen   = rxFrame.GetHeaderAcceptLen();
        mSpiSlaveDataLen = rxFrame.GetHeaderDataLen();

        if (!rxFrame.IsValid() || (slaveAcceptLen > kMaxFrameSize) || (mSpiSlaveDataLen > kMaxFrameSize))
        {
            mInterfaceMetrics.mTransferredGarbageFrameCount++;
            mSpiTxRefusedCount++;
            mSpiSlaveDataLen = 0;

            LogWarn("Garbage in header : %02X %02X %02X %02X %02X", spiRxFrame[0], spiRxFrame[1], spiRxFrame[2],
                    spiRxFrame[3], spiRxFrame[4]);
            otDumpDebgPlat("SPI-TX", mSpiTxFrameBuffer, spiTransferBytes);
            otDumpDebgPlat("SPI-RX", spiRxFrameBuffer, spiTransferBytes);

            ExitNow();
        }

        mInterfaceMetrics.mTransferredValidFrameCount++;

        if (rxFrame.IsResetFlagSet())
        {
            mSlaveResetCount++;

            LogNote("Slave did reset (%" PRIu64 " resets so far)", mSlaveResetCount);
            LogStats();
        }

        // Handle received packet, if any.
        if ((mSpiSlaveDataLen != 0) && (mSpiSlaveDataLen <= txFrame.GetHeaderAcceptLen()))
        {
            mInterfaceMetrics.mRxFrameByteCount += mSpiSlaveDataLen;
            mSpiSlaveDataLen = 0;
            mInterfaceMetrics.mRxFrameCount++;
            successfulExchanges++;

            // Set the skip length to skip align bytes and SPI frame header.
            SuccessOrExit(error = mRxFrameBuffer->SetSkipLength(skipAlignAllowanceLength + kSpiFrameHeaderSize));
            // Set the received frame length.
            SuccessOrExit(error = mRxFrameBuffer->SetLength(rxFrame.GetHeaderDataLen()));

            // Upper layer will free the frame buffer.
            discardRxFrame = false;

            mDidRxFrame = true;
            mReceiveFrameCallback(mReceiveFrameContext);
        }
    }

    // Handle transmitted packet, if any.
    if (mSpiTxIsReady && (mSpiTxPayloadSize == txFrame.GetHeaderDataLen()))
    {
        if (txFrame.GetHeaderDataLen() <= slaveAcceptLen)
        {
            // Our outbound packet has been successfully transmitted. Clear mSpiTxPayloadSize and mSpiTxIsReady so
            // that uplayer can pull another packet for us to send.
            successfulExchanges++;

            mInterfaceMetrics.mTxFrameCount++;
            mInterfaceMetrics.mTxFrameByteCount += mSpiTxPayloadSize;

            // Clear tx buffer after usage
            memset(&mSpiTxFrameBuffer[kSpiFrameHeaderSize], 0, mSpiTxPayloadSize);
            mSpiTxIsReady      = false;
            mSpiTxPayloadSize  = 0;
            mSpiTxRefusedCount = 0;
        }
        else
        {
            // The slave wasn't ready for what we had to send them. Incrementing this counter will turn on rate
            // limiting so that we don't waste a ton of CPU bombarding them with useless SPI transfers.
            mSpiTxRefusedCount++;
        }
    }

    if (!mSpiTxIsReady)
    {
        mSpiTxRefusedCount = 0;
    }

    if (successfulExchanges == 2)
    {
        mSpiDuplexFrameCount++;
    }

exit:
    if (discardRxFrame)
    {
        mRxFrameBuffer->DiscardFrame();
    }

    return error;
}

bool SpiInterface::CheckInterrupt(void) { return (GetGpioValue(mIntGpioValueFd) == kGpioIntAssertState); }

void SpiInterface::UpdateFdSet(void *aMainloopContext)
{
    struct timeval        timeout = {kSecPerDay, 0};
    otSysMainloopContext *context = reinterpret_cast<otSysMainloopContext *>(aMainloopContext);

    assert(context != nullptr);

    if (mSpiTxIsReady)
    {
        // We have data to send to the slave.
        timeout.tv_sec  = 0;
        timeout.tv_usec = 0;
    }

    if (context->mMaxFd < mIntGpioValueFd)
    {
        context->mMaxFd = mIntGpioValueFd;
    }

    if (CheckInterrupt())
    {
        // Interrupt pin is asserted, set the timeout to be 0.
        timeout.tv_sec  = 0;
        timeout.tv_usec = 0;
        LogDebg("UpdateFdSet(): Interrupt.");
    }
    else
    {
        // The interrupt pin was not asserted, so we wait for the interrupt pin to be asserted by adding it to the
        // read set.
        FD_SET(mIntGpioValueFd, &context->mReadFdSet);
    }

    if (mSpiTxRefusedCount)
    {
        struct timeval minTimeout = {0, 0};

        // We are being rate-limited by the slave. This is fairly normal behavior. Based on number of times slave has
        // refused a transmission, we apply a minimum timeout.
        if (mSpiTxRefusedCount < kImmediateRetryCount)
        {
            minTimeout.tv_usec = kImmediateRetryTimeoutUs;
        }
        else if (mSpiTxRefusedCount < kFastRetryCount)
        {
            minTimeout.tv_usec = kFastRetryTimeoutUs;
        }
        else
        {
            minTimeout.tv_usec = kSlowRetryTimeoutUs;
        }

        if (timercmp(&timeout, &minTimeout, <))
        {
            timeout = minTimeout;
        }

        if (mSpiTxIsReady && !mDidPrintRateLimitLog && (mSpiTxRefusedCount > 1))
        {
            // To avoid printing out this message over and over, we only print it out once the refused count is at two
            // or higher when we actually have something to send the slave. And then, we only print it once.
            LogInfo("Slave is rate limiting transactions");

            mDidPrintRateLimitLog = true;
        }

        if (mSpiTxRefusedCount == kSpiTxRefuseWarnCount)
        {
            // Ua-oh. The slave hasn't given us a chance to send it anything for over thirty frames. If this ever
            // happens, print out a warning to the logs.
            LogWarn("Slave seems stuck.");
        }
        else if (mSpiTxRefusedCount == kSpiTxRefuseExitCount)
        {
            // Double ua-oh. The slave hasn't given us a chance to send it anything for over a hundred frames.
            // This almost certainly means that the slave has locked up or gotten into an unrecoverable state.
            DieNowWithMessage("Slave seems REALLY stuck.", OT_EXIT_FAILURE);
        }
    }
    else
    {
        mDidPrintRateLimitLog = false;
    }

    if (timercmp(&timeout, &context->mTimeout, <))
    {
        context->mTimeout = timeout;
    }
}

void SpiInterface::Process(const void *aMainloopContext)
{
    const otSysMainloopContext *context = reinterpret_cast<const otSysMainloopContext *>(aMainloopContext);

    assert(context != nullptr);

    if (FD_ISSET(mIntGpioValueFd, &context->mReadFdSet))
    {
        struct gpioevent_data event;

        LogDebg("Process(): Interrupt.");

        // Read event data to clear interrupt.
        VerifyOrDie(read(mIntGpioValueFd, &event, sizeof(event)) != -1, OT_EXIT_ERROR_ERRNO);
    }

    // Service the SPI port if we can receive a packet or we have a packet to be sent.
    if (mSpiTxIsReady || CheckInterrupt())
    {
        // We guard this with the above check because we don't want to overwrite any previously received frames.
        IgnoreError(PushPullSpi());
    }
}

otError SpiInterface::WaitForFrame(uint64_t aTimeoutUs)
{
    otError  error = OT_ERROR_NONE;
    uint64_t now   = otPlatTimeGet();
    uint64_t end   = now + aTimeoutUs;

    mDidRxFrame = false;

    while (now < end)
    {
        otSysMainloopContext context;
        int                  ret;

        context.mMaxFd           = -1;
        context.mTimeout.tv_sec  = static_cast<time_t>((end - now) / OT_US_PER_S);
        context.mTimeout.tv_usec = static_cast<suseconds_t>((end - now) % OT_US_PER_S);

        FD_ZERO(&context.mReadFdSet);
        FD_ZERO(&context.mWriteFdSet);

        UpdateFdSet(&context);

        ret = select(context.mMaxFd + 1, &context.mReadFdSet, &context.mWriteFdSet, nullptr, &context.mTimeout);

        if (ret >= 0)
        {
            Process(&context);

            if (mDidRxFrame)
            {
                ExitNow();
            }
        }
        else if (errno != EINTR)
        {
            DieNow(OT_EXIT_ERROR_ERRNO);
        }

        now = otPlatTimeGet();
    }

    error = OT_ERROR_RESPONSE_TIMEOUT;

exit:
    return error;
}

otError SpiInterface::SendFrame(const uint8_t *aFrame, uint16_t aLength)
{
    otError error = OT_ERROR_NONE;

    VerifyOrExit(aLength < (kMaxFrameSize - kSpiFrameHeaderSize), error = OT_ERROR_NO_BUFS);

    if (IsSpinelResetCommand(aFrame, aLength))
    {
        ResetStates();
    }

    VerifyOrExit(!mSpiTxIsReady, error = OT_ERROR_BUSY);

    memcpy(&mSpiTxFrameBuffer[kSpiFrameHeaderSize], aFrame, aLength);

    mSpiTxIsReady     = true;
    mSpiTxPayloadSize = aLength;

    IgnoreError(PushPullSpi());

exit:
    return error;
}

void SpiInterface::LogError(const char *aString)
{
    OT_UNUSED_VARIABLE(aString);
    LogWarn("%s: %s", aString, strerror(errno));
}

void SpiInterface::LogStats(void)
{
    LogInfo("INFO: SlaveResetCount=%" PRIu64, mSlaveResetCount);
    LogInfo("INFO: SpiDuplexFrameCount=%" PRIu64, mSpiDuplexFrameCount);
    LogInfo("INFO: SpiUnresponsiveFrameCount=%" PRIu64, mSpiUnresponsiveFrameCount);
    LogInfo("INFO: TransferredFrameCount=%" PRIu64, mInterfaceMetrics.mTransferredFrameCount);
    LogInfo("INFO: TransferredValidFrameCount=%" PRIu64, mInterfaceMetrics.mTransferredValidFrameCount);
    LogInfo("INFO: TransferredGarbageFrameCount=%" PRIu64, mInterfaceMetrics.mTransferredGarbageFrameCount);
    LogInfo("INFO: RxFrameCount=%" PRIu64, mInterfaceMetrics.mRxFrameCount);
    LogInfo("INFO: RxFrameByteCount=%" PRIu64, mInterfaceMetrics.mRxFrameByteCount);
    LogInfo("INFO: TxFrameCount=%" PRIu64, mInterfaceMetrics.mTxFrameCount);
    LogInfo("INFO: TxFrameByteCount=%" PRIu64, mInterfaceMetrics.mTxFrameByteCount);
}
} // namespace Posix
} // namespace ot
#endif // OPENTHREAD_POSIX_CONFIG_SPINEL_SPI_INTERFACE_ENABLE
