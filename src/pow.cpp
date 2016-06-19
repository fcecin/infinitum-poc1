// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2014-2016 The DigiByte Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "arith_uint256.h"
#include "chain.h"
#include "primitives/block.h"
#include "uint256.h"





/*


//
// Global state
//

static const int64_t nTargetTimespan =  0.10 * 24 * 60 * 60; // 2.4 hours
static const int64_t nTargetSpacing = 60; // 60 seconds
static const int64_t nInterval = nTargetTimespan / nTargetSpacing;
static const int64_t nTargetTimespanRe = 1*60; // 60 Seconds
static const int64_t nTargetSpacingRe = 1*60; // 60 seconds
static const int64_t nIntervalRe = nTargetTimespanRe / nTargetSpacingRe; // 1 block

//MultiAlgo Target updates
static const int64_t multiAlgoTargetSpacing = 30*5; // NUM_ALGOS * 30 seconds
static const int64_t multiAlgoTargetSpacingV4 = 15*5; // NUM_ALGOS * 15 seconds

static const int64_t nAveragingInterval = 10; // 10 blocks
static const int64_t nAveragingTargetTimespan = nAveragingInterval * multiAlgoTargetSpacing; // 10* NUM_ALGOS * 30
static const int64_t nAveragingTargetTimespanV4 = nAveragingInterval * multiAlgoTargetSpacingV4; // 10 * NUM_ALGOS * 15

static const int64_t nMaxAdjustDown = 40; // 40% adjustment down
static const int64_t nMaxAdjustUp = 20; // 20% adjustment up
static const int64_t nMaxAdjustDownV3 = 16; // 16% adjustment down
static const int64_t nMaxAdjustUpV3 = 8; // 8% adjustment up
static const int64_t nMaxAdjustDownV4 = 16;
static const int64_t nMaxAdjustUpV4 = 8;
static const int64_t nLocalDifficultyAdjustment = 4; //difficulty adjustment per algo
static const int64_t nLocalTargetAdjustment = 4; //target adjustment per algo

static const int64_t nMinActualTimespan = nAveragingTargetTimespan * (100 - nMaxAdjustUp) / 100;
static const int64_t nMaxActualTimespan = nAveragingTargetTimespan * (100 + nMaxAdjustDown) / 100;

static const int64_t nMinActualTimespanV3 = nAveragingTargetTimespan * (100 - nMaxAdjustUpV3) / 100;
static const int64_t nMaxActualTimespanV3 = nAveragingTargetTimespan * (100 + nMaxAdjustDownV3) / 100;

static const int64_t nMinActualTimespanV4 = nAveragingTargetTimespanV4 * (100 - nMaxAdjustUpV4) / 100;
static const int64_t nMaxActualTimespanV4 = nAveragingTargetTimespanV4 * (100 + nMaxAdjustDownV4) / 100;


*/

/*
static unsigned int GetNextWorkRequiredV4(const CBlockIndex* pindexLast, const CBlockHeader *pblock, int algo,bool log)
{
	unsigned int nProofOfWorkLimit = Params().ProofOfWorkLimit(algo).GetCompact();

	// Genesis block
	if (pindexLast == NULL)
		return nProofOfWorkLimit;

	if (TestNet())
	{
		// Special difficulty rule for testnet:
		// If the new block's timestamp is more than 2* 10 minutes
		// then allow mining of a min-difficulty block.
		if (pblock->nTime > pindexLast->nTime + nTargetSpacing*2)
			return nProofOfWorkLimit;
		else
		{
			// Return the last non-special-min-difficulty-rules-block
			const CBlockIndex* pindex = pindexLast;
			while (pindex->pprev && pindex->nHeight % nInterval != 0 && pindex->nBits == nProofOfWorkLimit)
				pindex = pindex->pprev;
			return pindex->nBits;
		}
	}

	if(log)
	{
		LogPrintf("GetNextWorkRequired RETARGET\n");
		LogPrintf("Algo: %s\n", GetAlgoName(algo));
		LogPrintf("Height (Before): %s\n", pindexLast->nHeight);
	}

	// find first block in averaging interval
	// Go back by what we want to be nAveragingInterval blocks per algo
	const CBlockIndex* pindexFirst = pindexLast;
	for (int i = 0; pindexFirst && i < NUM_ALGOS*nAveragingInterval; i++)
	{
		pindexFirst = pindexFirst->pprev;
	}

	const CBlockIndex* pindexPrevAlgo = GetLastBlockIndexForAlgo(pindexLast, algo);
	if (pindexPrevAlgo == NULL || pindexFirst == NULL)
	{
		if(log)
			LogPrintf("Use default POW Limit\n");
		return nProofOfWorkLimit;
	}

	// Limit adjustment step
	// Use medians to prevent time-warp attacks
	int64_t nActualTimespan = pindexLast-> GetMedianTimePast() - pindexFirst->GetMedianTimePast();
	nActualTimespan = nAveragingTargetTimespanV4 + (nActualTimespan - nAveragingTargetTimespanV4)/4;

	if(log)
		LogPrintf("nActualTimespan = %d before bounds\n", nActualTimespan);

	if (nActualTimespan < nMinActualTimespanV4)
		nActualTimespan = nMinActualTimespanV4;
	if (nActualTimespan > nMaxActualTimespanV4)
		nActualTimespan = nMaxActualTimespanV4;

	//Global retarget
	CBigNum bnNew;
	bnNew.SetCompact(pindexPrevAlgo->nBits);

	bnNew *= nActualTimespan;
	bnNew /= nAveragingTargetTimespanV4;

	//Per-algo retarget
	int nAdjustments = pindexPrevAlgo->nHeight + NUM_ALGOS - 1 - pindexLast->nHeight;
	if (nAdjustments > 0)
	{
		for (int i = 0; i < nAdjustments; i++)
		{
			bnNew *= 100;
			bnNew /= (100 + nLocalTargetAdjustment);
		}
	}
	else if (nAdjustments < 0)//make it easier
	{
		for (int i = 0; i < -nAdjustments; i++)
		{
			bnNew *= (100 + nLocalTargetAdjustment);
			bnNew /= 100;
		}
	}

	if (bnNew > Params().ProofOfWorkLimit(algo))
	{
		if(log)
		{
			LogPrintf("bnNew > Params().ProofOfWorkLimit(algo)\n");
		}
		bnNew = Params().ProofOfWorkLimit(algo);
	}

	if(log)
	{
		LogPrintf("nAveragingTargetTimespanV4 = %d; nActualTimespan = %d\n", nAveragingTargetTimespanV4, nActualTimespan);
		LogPrintf("Before: %08x  %s\n", pindexPrevAlgo->nBits, CBigNum().SetCompact(pindexPrevAlgo->nBits).getuint256().ToString());
		LogPrintf("After:  %08x  %s\n", bnNew.GetCompact(), bnNew.getuint256().ToString());
	}

	return bnNew.GetCompact();
}
*/

// Infinitum:: FIXME 
// - will this work? moving to 10 min blocks (600s)...
// - move this elsewhere
//
/*
//static const int64_t multiAlgoTargetSpacingV4 = 15*5; // NUM_ALGOS * 15 seconds
static const int64_t nAveragingInterval = 50; // blocks in the time averaging window.
//static const int64_t nAveragingTargetTimespan = nAveragingInterval * multiAlgoTargetSpacing; // 10* NUM_ALGOS * 30
//static const int64_t nAveragingTargetTimespanV4 = nAveragingInterval * 15; // 10 * NUM_ALGOS * 15  // 750 seconds
static const int64_t nAveragingTargetTimespanV4 = nAveragingInterval * 600; // blocks*seconds == second arg is secs btw blocks
static const int64_t nMaxAdjustDownV4 = 16;
static const int64_t nMaxAdjustUpV4 = 8;
static const int64_t nMinActualTimespanV4 = nAveragingTargetTimespanV4 * (100 - nMaxAdjustUpV4) / 100;
static const int64_t nMaxActualTimespanV4 = nAveragingTargetTimespanV4 * (100 + nMaxAdjustDownV4) / 100;
*/

//static unsigned int GetNextWorkRequiredV4(const CBlockIndex* pindexLast, const CBlockHeader *pblock, bool log)
unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
  // FIXME
  bool log = true;

  // digibyte
  //unsigned int nProofOfWorkLimit = Params().ProofOfWorkLimit(algo).GetCompact();

  unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

  // Infinitum: FIXME: remove this when done testing with instamine blocks 
  //return nProofOfWorkLimit;

  // Genesis block
  if (pindexLast == NULL)
    return nProofOfWorkLimit;
  
  // changes every block now, so the btc version's "once per interval" is not needed
  
  /*
    this is  the digibyte version, same thing but old code that won't compile here

    if (TestNet())
  	{

		// Special difficulty rule for testnet:
		// If the new block's timestamp is more than 2* 10 minutes
		// then allow mining of a min-difficulty block.
		if (pblock->nTime > pindexLast->nTime + nTargetSpacing*2)
			return nProofOfWorkLimit;
		else
		{
			// Return the last non-special-min-difficulty-rules-block
			const CBlockIndex* pindex = pindexLast;
			while (pindex->pprev && pindex->nHeight % nInterval != 0 && pindex->nBits == nProofOfWorkLimit)
				pindex = pindex->pprev;
			return pindex->nBits;
		}
	}
*/
       // btc version of this 
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }




	if(log)
	{
	  //LogPrintf("GetNextWorkRequired RETARGET\n");
	  //LogPrintf("Algo: %s\n", GetAlgoName(algo));
	  //LogPrintf("Height (Before): %s\n", pindexLast->nHeight);
	}

	// find first block in averaging interval
	// Go back by what we want to be nAveragingInterval blocks
	
	const CBlockIndex* pindexFirst = pindexLast;
	for (int i = 0; pindexFirst && i < params.nPowAveragingInterval; i++)  // was "NUM_ALGOS*nAveragingInterval", still "50" blocks
	{
		pindexFirst = pindexFirst->pprev;

		// Infinitum:: FIXME/REVIEW : Snapshot boundaries on difficulty interval looking-back.
		//    this is annoying as fuck: even the regular difficulty equation needs all the headers.
		//    maybe we can get away keeping only "interval" block headers since the snapshot?
		//    in a sense that's the same as putting this on the snapshot itself...
		//
		// OR, We just can't forget block headers ever. This seems annoying.
		//
		// OR, We assume we have enough block headers looking back from the current snapshot.
		//   and at first we don't bother deleting headers at all. and then at some point in 
		//   the future we start caring and get rid of them.
		// Hmmm... yeah, that will work for now. We're good.
	}

	// if we are just starting the blockchain, 
	if (pindexFirst == NULL)
	  return nProofOfWorkLimit;
	

	/* BTC -- probably has that bug?
    // Go back by what we want to be 14 days worth of blocks
    int nHeightFirst = pindexLast->nHeight - (params.DifficultyAdjustmentInterval()-1);
    assert(nHeightFirst >= 0);
    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
    assert(pindexFirst);
	*/

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetMedianTimePast(), params);


	/*
	  digibyte stuff probably makes no sense

	const CBlockIndex* pindexPrevAlgo = GetLastBlockIndexForAlgo(pindexLast, algo);
	if (pindexPrevAlgo == NULL || pindexFirst == NULL)
	{
		if(log)
			LogPrintf("Use default POW Limit\n");
		return nProofOfWorkLimit;
	}
	*/

	/*
	  FIXME: call calculate next work required instead
	    DELETED

	// Limit adjustment step
	// Use medians to prevent time-warp attacks
	int64_t nActualTimespan = pindexLast-> GetMedianTimePast() - pindexFirst->GetMedianTimePast();
	nActualTimespan = nAveragingTargetTimespanV4 + (nActualTimespan - nAveragingTargetTimespanV4)/4;

	if(log)
		LogPrintf("nActualTimespan = %d before bounds\n", nActualTimespan);

	if (nActualTimespan < nMinActualTimespanV4)
		nActualTimespan = nMinActualTimespanV4;
	if (nActualTimespan > nMaxActualTimespanV4)
		nActualTimespan = nMaxActualTimespanV4;

	//Global retarget
	CBigNum bnNew;
	bnNew.SetCompact(pindexPrevAlgo->nBits);

	bnNew *= nActualTimespan;
	bnNew /= nAveragingTargetTimespanV4;

	//Per-algo retarget
	int nAdjustments = pindexPrevAlgo->nHeight + NUM_ALGOS - 1 - pindexLast->nHeight;
	if (nAdjustments > 0)
	{
		for (int i = 0; i < nAdjustments; i++)
		{
			bnNew *= 100;
			bnNew /= (100 + nLocalTargetAdjustment);
		}
	}
	else if (nAdjustments < 0)//make it easier
	{
		for (int i = 0; i < -nAdjustments; i++)
		{
			bnNew *= (100 + nLocalTargetAdjustment);
			bnNew /= 100;
		}
	}

	if (bnNew > Params().ProofOfWorkLimit(algo))
	{
		if(log)
		{
			LogPrintf("bnNew > Params().ProofOfWorkLimit(algo)\n");
		}
		bnNew = Params().ProofOfWorkLimit(algo);
	}

	if(log)
	{
		LogPrintf("nAveragingTargetTimespanV4 = %d; nActualTimespan = %d\n", nAveragingTargetTimespanV4, nActualTimespan);
		LogPrintf("Before: %08x  %s\n", pindexPrevAlgo->nBits, CBigNum().SetCompact(pindexPrevAlgo->nBits).getuint256().ToString());
		LogPrintf("After:  %08x  %s\n", bnNew.GetCompact(), bnNew.getuint256().ToString());
	}

	return bnNew.GetCompact();
	*/
}

// passing the " median time past" instead for firstblocktime
// tests call this directly
unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    // Use medians to prevent time-warp attacks
    //int64_t nActualTimespan = pindexLast-> GetMedianTimePast() - pindexFirst->GetMedianTimePast();
    int64_t nActualTimespan = pindexLast-> GetMedianTimePast() - nFirstBlockTime; //pindexFirst->GetMedianTimePast(); // SAME if caller is doing the right thing
    nActualTimespan = params.nPowAveragingTimespan + (nActualTimespan - params.nPowAveragingTimespan)/4;
    //int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;


    if (nActualTimespan < params.MinActualTimespan())
      nActualTimespan = params.MinActualTimespan();
    if (nActualTimespan > params.MaxActualTimespan())
      nActualTimespan = params.MaxActualTimespan();
    /*
    if (nActualTimespan < params.nPowTargetTimespan/4)
        nActualTimespan = params.nPowTargetTimespan/4;
    if (nActualTimespan > params.nPowTargetTimespan*4)
        nActualTimespan = params.nPowTargetTimespan*4;
    */


    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    //bnNew /= params.nPowTargetTimespan;
    bnNew /= params.nPowAveragingTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}


/*
// Bitcoin's version
unsigned int BTGetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    // Infinitum: FIXME: remove this when done testing with instamine blocks 
        return nProofOfWorkLimit;

    
    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    // Only change once per difficulty adjustment interval
    if ((pindexLast->nHeight+1) % params.DifficultyAdjustmentInterval() != 0)
    {
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    // Go back by what we want to be 14 days worth of blocks
    int nHeightFirst = pindexLast->nHeight - (params.DifficultyAdjustmentInterval()-1);
    assert(nHeightFirst >= 0);
    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
    assert(pindexFirst);

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    if (nActualTimespan < params.nPowTargetTimespan/4)
        nActualTimespan = params.nPowTargetTimespan/4;
    if (nActualTimespan > params.nPowTargetTimespan*4)
        nActualTimespan = params.nPowTargetTimespan*4;

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}
*/

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}
