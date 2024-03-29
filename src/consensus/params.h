// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef INFINITUM_CONSENSUS_PARAMS_H
#define INFINITUM_CONSENSUS_PARAMS_H

#include "uint256.h"
#include <map>
#include <string>

namespace Consensus {

enum DeploymentPos
{
    DEPLOYMENT_TESTDUMMY,
    //DEPLOYMENT_CSV, // Deployment of BIP68, BIP112, and BIP113.
    MAX_VERSION_BITS_DEPLOYMENTS
};

/**
 * Struct for each individual consensus rule change using BIP9.
 */
struct BIP9Deployment {
    /** Bit position to select the particular bit in nVersion. */
    int bit;
    /** Start MedianTime for version bits miner confirmation. Can be a date in the past */
    int64_t nStartTime;
    /** Timeout/expiry MedianTime for the deployment attempt. */
    int64_t nTimeout;
};

/**
 * Parameters that influence chain consensus.
 */
struct Params {
    uint256 hashGenesisBlock;
    int nSubsidyHalvingInterval;
    /** Used to check majorities for block version upgrade */
    int nMajorityEnforceBlockUpgrade;
    int nMajorityRejectBlockOutdated;
    int nMajorityWindow;
    /** Block height and hash at which BIP34 becomes active */
    //int BIP34Height;
    //uint256 BIP34Hash;
    /**
     * Minimum blocks including miner confirmation of the total of 2016 blocks in a retargetting period,
     * (nPowTargetTimespan / nPowTargetSpacing) which is also used for BIP9 deployments.
     * Examples: 1916 for 95%, 1512 for testchains.
     */
    uint32_t nRuleChangeActivationThreshold;
    uint32_t nMinerConfirmationWindow;
    BIP9Deployment vDeployments[MAX_VERSION_BITS_DEPLOYMENTS];
    /** Proof of work parameters */
    uint256 powLimit;
    bool fPowAllowMinDifficultyBlocks;
    bool fPowNoRetargeting;
    int64_t nPowTargetSpacing;

    // Infinitum:: renamed for "digishield"
    //int64_t nPowTargetTimespan;
    int64_t nPowAveragingTimespan;

    // Infinitum:: FIXME/REVIEW not sure if this makes any sense or has any use anymore.
    int64_t DifficultyAdjustmentInterval() const { return nPowAveragingTimespan / nPowTargetSpacing; }
  
    // Infinitum:: our "digishield" stuff
    int64_t nPowAveragingInterval;
    int64_t nMaxAdjustUp;
    int64_t nMaxAdjustDown;
    int64_t MinActualTimespan() const { return nPowAveragingTimespan * (100 - nMaxAdjustUp) / 100; };
    int64_t MaxActualTimespan() const { return nPowAveragingTimespan * (100 + nMaxAdjustDown) / 100; };
};
} // namespace Consensus

#endif // INFINITUM_CONSENSUS_PARAMS_H
