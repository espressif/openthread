/*
 *  Copyright (c) 2021, The OpenThread Authors.
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
 *   This file includes definitions for Crypto Internal Trusted Storage (ITS) APIs.
 */

#ifndef STORAGE_HPP_
#define STORAGE_HPP_

#include "openthread-core-config.h"

#include <openthread/platform/crypto.h>

#include "common/as_core_type.hpp"
#include "common/clearable.hpp"
#include "common/code_utils.hpp"
#include "common/error.hpp"
#include "common/locator.hpp"
#include "common/non_copyable.hpp"

namespace ot {
namespace Crypto {

#if OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE

namespace Storage {

/**
 * Defines the key types.
 */
enum KeyType : uint8_t
{
    kKeyTypeRaw   = OT_CRYPTO_KEY_TYPE_RAW,   ///< Key Type: Raw Data.
    kKeyTypeAes   = OT_CRYPTO_KEY_TYPE_AES,   ///< Key Type: AES.
    kKeyTypeHmac  = OT_CRYPTO_KEY_TYPE_HMAC,  ///< Key Type: HMAC.
    kKeyTypeEcdsa = OT_CRYPTO_KEY_TYPE_ECDSA, ///< Key Type: ECDSA.
};

/**
 * Defines the key algorithms.
 */
enum KeyAlgorithm : uint8_t
{
    kKeyAlgorithmVendor     = OT_CRYPTO_KEY_ALG_VENDOR,       ///< Key Algorithm: Vendor Defined.
    kKeyAlgorithmAesEcb     = OT_CRYPTO_KEY_ALG_AES_ECB,      ///< Key Algorithm: AES ECB.
    kKeyAlgorithmHmacSha256 = OT_CRYPTO_KEY_ALG_HMAC_SHA_256, ///< Key Algorithm: HMAC SHA-256.
    kKeyAlgorithmEcdsa      = OT_CRYPTO_KEY_ALG_ECDSA,        ///< Key Algorithm: ECDSA.
};

constexpr uint8_t kUsageNone       = OT_CRYPTO_KEY_USAGE_NONE;        ///< Key Usage: Key Usage is empty.
constexpr uint8_t kUsageExport     = OT_CRYPTO_KEY_USAGE_EXPORT;      ///< Key Usage: Key can be exported.
constexpr uint8_t kUsageEncrypt    = OT_CRYPTO_KEY_USAGE_ENCRYPT;     ///< Key Usage: Encrypt (vendor defined).
constexpr uint8_t kUsageDecrypt    = OT_CRYPTO_KEY_USAGE_DECRYPT;     ///< Key Usage: AES ECB.
constexpr uint8_t kUsageSignHash   = OT_CRYPTO_KEY_USAGE_SIGN_HASH;   ///< Key Usage: Sign Hash.
constexpr uint8_t kUsageVerifyHash = OT_CRYPTO_KEY_USAGE_VERIFY_HASH; ///< Key Usage: Verify Hash.

/**
 * Defines the key storage types.
 */
enum StorageType : uint8_t
{
    kTypeVolatile   = OT_CRYPTO_KEY_STORAGE_VOLATILE,   ///< Key is volatile.
    kTypePersistent = OT_CRYPTO_KEY_STORAGE_PERSISTENT, ///< Key is persistent.
};

/**
 * This datatype represents the key reference.
 */
typedef otCryptoKeyRef KeyRef;

constexpr KeyRef kInvalidKeyRef = 0x80000000; ///< Invalid `KeyRef` value (PSA_KEY_ID_VENDOR_MAX + 1).

#if OPENTHREAD_FTD || OPENTHREAD_MTD

/**
 * Manages and selects the `KeyRef` values.
 */
class KeyRefManager : public InstanceLocator
{
public:
    /**
     * Represents difference `KeyRef` types.
     */
    enum Type : uint8_t
    {
        kNetworkKey               = 1,
        kPskc                     = 2,
        kActiveDatasetNetworkKey  = 3,
        kActiveDatasetPskc        = 4,
        kPendingDatasetNetworkKey = 5,
        kPendingDatasetPskc       = 6,
        kEcdsa                    = 7,
    };

    /**
     * Initializes the `KeyRefManager`.
     *
     * @param[in]  aInstance     A reference to the OpenThread instance.
     */
    explicit KeyRefManager(Instance &aInstance)
        : InstanceLocator(aInstance)
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
        , mExtraOffset(0)
#endif
    {
    }

    /**
     * Determines the `KeyRef` to use for a given `Type`.
     *
     * @param[in] aType  The key ref type.
     *
     * @returns The `KeyRef` associated with @p aType.
     */
    KeyRef KeyRefFor(Type aType)
    {
        KeyRef keyRef = kPsaItsNvmOffset + aType;

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
        keyRef += mExtraOffset;
#endif
        return keyRef;
    }

    /**
     * Delete all the persistent keys.
     */
    void DestroyPersistentKeys(void);

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    static constexpr uint32_t kKeyRefExtraOffset = 32; ///< Recommended extra offset to use.

    /**
     * Sets the additional `KeyRef` offset value to use when determining the `KeyRef`s.
     *
     * This is intended for when `MULTIPLE_INSTANCE_ENABLE` is enabled to ensure different `ot::Instance`s use
     * different `KeyRef` value ranges.
     *
     * @param[in] aOffset  The offset value.
     */
    void SetKeyRefExtraOffset(uint32_t aOffset) { mExtraOffset = aOffset; }
#endif

private:
    static constexpr KeyRef kPsaItsNvmOffset = OPENTHREAD_CONFIG_PSA_ITS_NVM_OFFSET;

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    uint32_t mExtraOffset;
#endif
};

#endif // OPENTHREAD_FTD || OPENTHREAD_MTD

/**
 * Determine if a given `KeyRef` is valid or not.
 *
 * @param[in] aKeyRef   The `KeyRef` to check.
 *
 * @retval TRUE   If @p aKeyRef is valid.
 * @retval FALSE  If @p aKeyRef is not valid.
 */
inline bool IsKeyRefValid(KeyRef aKeyRef) { return (aKeyRef < kInvalidKeyRef); }

/**
 * Import a key into PSA ITS.
 *
 * @param[in,out] aKeyRef          Reference to the key ref to be used for crypto operations.
 * @param[in]     aKeyType         Key Type encoding for the key.
 * @param[in]     aKeyAlgorithm    Key algorithm encoding for the key.
 * @param[in]     aKeyUsage        Key Usage encoding for the key.
 * @param[in]     aStorageType     Key storage type.
 * @param[in]     aKey             Actual key to be imported.
 * @param[in]     aKeyLen          Length of the key to be imported.
 *
 * @retval kErrorNone          Successfully imported the key.
 * @retval kErrorFailed        Failed to import the key.
 * @retval kErrorInvalidArgs   @p aKey was set to `nullptr`.
 */
inline Error ImportKey(KeyRef        &aKeyRef,
                       KeyType        aKeyType,
                       KeyAlgorithm   aKeyAlgorithm,
                       int            aKeyUsage,
                       StorageType    aStorageType,
                       const uint8_t *aKey,
                       size_t         aKeyLen)
{
    return otPlatCryptoImportKey(&aKeyRef, static_cast<otCryptoKeyType>(aKeyType),
                                 static_cast<otCryptoKeyAlgorithm>(aKeyAlgorithm), aKeyUsage,
                                 static_cast<otCryptoKeyStorage>(aStorageType), aKey, aKeyLen);
}

/**
 * Export a key stored in PSA ITS.
 *
 * @param[in]   aKeyRef        The key ref to be used for crypto operations.
 * @param[out]  aBuffer        Pointer to the buffer where key needs to be exported.
 * @param[in]   aBufferLen     Length of the buffer passed to store the exported key.
 * @param[out]  aKeyLen        Reference to variable to return the length of the exported key.
 *
 * @retval kErrorNone          Successfully exported  @p aKeyRef.
 * @retval kErrorFailed        Failed to export @p aKeyRef.
 * @retval kErrorInvalidArgs   @p aBuffer was `nullptr`.
 */
inline Error ExportKey(KeyRef aKeyRef, uint8_t *aBuffer, size_t aBufferLen, size_t &aKeyLen)
{
    return otPlatCryptoExportKey(aKeyRef, aBuffer, aBufferLen, &aKeyLen);
}

/**
 * Destroy a key stored in PSA ITS.
 *
 * @param[in]   aKeyRef   The key ref to be removed.
 */
inline void DestroyKey(KeyRef aKeyRef)
{
    if (IsKeyRefValid(aKeyRef))
    {
        IgnoreError(otPlatCryptoDestroyKey(aKeyRef));
    }
}

/**
 * Check if the keyRef passed has an associated key in PSA ITS.
 *
 * @param[in]  aKeyRef          The Key Ref for to check.
 *
 * @retval true                 Key Ref passed has a key associated in PSA.
 * @retval false                Key Ref passed is invalid and has no key associated in PSA.
 */
inline bool HasKey(KeyRef aKeyRef) { return otPlatCryptoHasKey(aKeyRef); }

} // namespace Storage

#endif // OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE

/**
 * Represents a crypto key.
 *
 * The `Key` can represent a literal key (i.e., a pointer to a byte array containing the key along with a key length)
 * or a `KeyRef` (if `OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE` is enabled).
 */
class Key : public otCryptoKey, public Clearable<Key>
{
public:
    /**
     * Sets the `Key` as a literal key from a given byte array and length.
     *
     * @param[in] aKeyBytes   A pointer to buffer containing the key.
     * @param[in] aKeyLength  The key length (number of bytes in @p aKeyBytes).
     */
    void Set(const uint8_t *aKeyBytes, uint16_t aKeyLength)
    {
        mKey       = aKeyBytes;
        mKeyLength = aKeyLength;
    }

    /**
     * Gets the pointer to the bye array containing the key.
     *
     * If `OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE` is enabled and `IsKeyRef()` returns `true`, then this
     * method returns `nullptr`.
     *
     * @returns The pointer to the byte array containing the key, or `nullptr` if the `Key` represents a `KeyRef`
     */
    const uint8_t *GetBytes(void) const { return mKey; }

    /**
     * Gets the key length (number of bytes).
     *
     * If `OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE` is enabled and `IsKeyRef()` returns `true`, then this
     * method returns zero.
     *
     * @returns The key length (number of bytes in the byte array from `GetBytes()`), or zero if `Key` represents a
     *          `keyRef`.
     */
    uint16_t GetLength(void) const { return mKeyLength; }

#if OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE
    /**
     * Indicates whether or not the key is represented as a `KeyRef`.
     *
     * @retval TRUE  The `Key` represents a `KeyRef`
     * @retval FALSE The `Key` represents a literal key.
     */
    bool IsKeyRef(void) const { return (mKey == nullptr); }

    /**
     * Gets the `KeyRef`.
     *
     * MUST be used when `IsKeyRef()` returns `true`, otherwise its behavior is undefined.
     *
     * @returns The `KeyRef` associated with `Key`.
     */
    Storage::KeyRef GetKeyRef(void) const { return mKeyRef; }

    /**
     * Sets the `Key` as a `KeyRef`.
     *
     * @param[in] aKeyRef   The `KeyRef` to set from.
     */
    void SetAsKeyRef(Storage::KeyRef aKeyRef)
    {
        mKey       = nullptr;
        mKeyLength = 0;
        mKeyRef    = aKeyRef;
    }

    /**
     * Extracts and return the literal key when the key is represented as a `KeyRef`
     *
     * MUST be used when `IsKeyRef()` returns `true`.
     *
     * @param[out]     aKeyBuffer  Pointer to a byte array buffer to place the extracted key.
     * @param[in,out]  aKeyLength  On input, the size of @p aKeyBuffer.
     *                             On exit, returns the key length (number of bytes written in @p aKeyBuffer).
     *
     * @retval kErrorNone    Successfully extracted the key, @p aKeyBuffer and @p aKeyLength are updated.
     * @retval kErrorNoBufs  Key does not fit in @p aKeyBuffer (extracted key length is larger than @p aKeyLength).
     */
    Error ExtractKey(uint8_t *aKeyBuffer, uint16_t &aKeyLength) const;
#endif // OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE
};

/**
 * Represents a literal key derived from a `Key`.
 */
class LiteralKey : public Clearable<LiteralKey>, private NonCopyable
{
public:
    static constexpr uint16_t kMaxSize = 32; ///< Maximum size of the key.

    /**
     * Initializes the `LiteralKey` from a given `Key`.
     *
     * If the @p aKey is itself represents a literal key the same key buffer pointers are used. If the @p aKey is
     * a `KeyRef` then the literal key is extracted. In this case, the extracted key MUST be smaller than `kMaxSize`.
     *
     * @param[in] aKey   The key to convert from.
     */
    explicit LiteralKey(const Key &aKey);

    /*
     * Gets the pointer to the byte array containing the literal key.
     *
     * @returns The pointer to the byte array containing the literal key.
     */
    const uint8_t *GetBytes(void) const { return mKey; }

    /**
     * Gets the key length.
     *
     * @returns The key length (number of bytes in the byte array from `GetBytes()`).
     */
    uint16_t GetLength(void) const { return mLength; }

private:
    const uint8_t *mKey;
    uint16_t       mLength;
#if OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE
    uint8_t mBuffer[kMaxSize];
#endif
};

} // namespace Crypto

DefineCoreType(otCryptoKey, Crypto::Key);

} // namespace ot

#endif // STORAGE_HPP_
