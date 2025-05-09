/*
 *  Copyright (c) 2016, The OpenThread Authors.
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
 *   This file implements the OpenThread Network Data API.
 */

#include "openthread-core-config.h"

#include "instance/instance.hpp"

using namespace ot;

otError otNetDataGet(otInstance *aInstance, bool aStable, uint8_t *aData, uint8_t *aDataLength)
{
    AssertPointerIsNotNull(aData);
    AssertPointerIsNotNull(aDataLength);

    return AsCoreType(aInstance).Get<NetworkData::Leader>().CopyNetworkData(
        aStable ? NetworkData::kStableSubset : NetworkData::kFullSet, aData, *aDataLength);
}

uint8_t otNetDataGetLength(otInstance *aInstance)
{
    return AsCoreType(aInstance).Get<NetworkData::Leader>().GetLength();
}

uint8_t otNetDataGetMaxLength(otInstance *aInstance)
{
    return AsCoreType(aInstance).Get<NetworkData::Leader>().GetMaxLength();
}

void otNetDataResetMaxLength(otInstance *aInstance)
{
    AsCoreType(aInstance).Get<NetworkData::Leader>().ResetMaxLength();
}

otError otNetDataGetNextOnMeshPrefix(otInstance            *aInstance,
                                     otNetworkDataIterator *aIterator,
                                     otBorderRouterConfig  *aConfig)
{
    AssertPointerIsNotNull(aIterator);

    return AsCoreType(aInstance).Get<NetworkData::Leader>().GetNextOnMeshPrefix(*aIterator, AsCoreType(aConfig));
}

#if OPENTHREAD_FTD && OPENTHREAD_CONFIG_BORDER_ROUTING_ENABLE
bool otNetDataContainsOmrPrefix(otInstance *aInstance, const otIp6Prefix *aPrefix)
{
    return AsCoreType(aInstance).Get<NetworkData::Leader>().ContainsOmrPrefix(AsCoreType(aPrefix));
}
#endif

otError otNetDataGetNextRoute(otInstance *aInstance, otNetworkDataIterator *aIterator, otExternalRouteConfig *aConfig)
{
    AssertPointerIsNotNull(aIterator);

    return AsCoreType(aInstance).Get<NetworkData::Leader>().GetNextExternalRoute(*aIterator, AsCoreType(aConfig));
}

otError otNetDataGetNextService(otInstance *aInstance, otNetworkDataIterator *aIterator, otServiceConfig *aConfig)
{
    AssertPointerIsNotNull(aIterator);

    return AsCoreType(aInstance).Get<NetworkData::Leader>().GetNextService(*aIterator, AsCoreType(aConfig));
}

otError otNetDataGetNextLowpanContextInfo(otInstance            *aInstance,
                                          otNetworkDataIterator *aIterator,
                                          otLowpanContextInfo   *aContextInfo)
{
    AssertPointerIsNotNull(aIterator);

    return AsCoreType(aInstance).Get<NetworkData::Leader>().GetNextLowpanContextInfo(*aIterator,
                                                                                     AsCoreType(aContextInfo));
}

void otNetDataGetCommissioningDataset(otInstance *aInstance, otCommissioningDataset *aDataset)
{
    return AsCoreType(aInstance).Get<NetworkData::Leader>().GetCommissioningDataset(AsCoreType(aDataset));
}

uint8_t otNetDataGetVersion(otInstance *aInstance)
{
    return AsCoreType(aInstance).Get<Mle::Mle>().GetLeaderData().GetDataVersion(NetworkData::kFullSet);
}

uint8_t otNetDataGetStableVersion(otInstance *aInstance)
{
    return AsCoreType(aInstance).Get<Mle::Mle>().GetLeaderData().GetDataVersion(NetworkData::kStableSubset);
}

otError otNetDataSteeringDataCheckJoiner(otInstance *aInstance, const otExtAddress *aEui64)
{
    return AsCoreType(aInstance).Get<NetworkData::Leader>().SteeringDataCheckJoiner(AsCoreType(aEui64));
}

otError otNetDataSteeringDataCheckJoinerWithDiscerner(otInstance *aInstance, const otJoinerDiscerner *aDiscerner)
{
    return AsCoreType(aInstance).Get<NetworkData::Leader>().SteeringDataCheckJoiner(AsCoreType(aDiscerner));
}
