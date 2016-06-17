// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chain.h"

using namespace std;

#include <boost/foreach.hpp>

/**
 * CChain implementation
 */

/*
unsigned int LogBase2(uint64_t n)
{
    static const unsigned int table[64] = {
        0, 58, 1, 59, 47, 53, 2, 60, 39, 48, 27, 54, 33, 42, 3, 61,
        51, 37, 40, 49, 18, 28, 20, 55, 30, 34, 11, 43, 14, 22, 4, 62,
        57, 46, 52, 38, 26, 32, 41, 50, 36, 17, 19, 29, 10, 13, 21, 56,
        45, 25, 31, 35, 16, 9, 12, 44, 24, 15, 8, 23, 7, 6, 5, 63 };

    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;

    return table[(n * 0x03f6eaf2cd271461) >> 58];
}
*/

/*
void CChain::AddUnspentOutputValue(const CAmount& nOutputValue, uint64_t nBlockHeight, bool fLast) {
  
  // e.g. evaluating 105119/105120 (block #105120, at height-1 == 105119) registers its utxos at entry 
  //      zero, as does evaluating  0/105120 (block #1 -- the actual first block -- at height-1 == 0).
  //
  //      when entry 0 is finished and block 105121 is having its utxos counted, that counting
  //      happens on entry 1. when an actual on-disk snapshot is taken for the first two years,
  //      the outputs vs values count applies to the file that condenses blocks 1 to 105120, 
  //      as expected.
  //
  uint64_t nSnapHeight = (nBlockHeight - 1) / SNAPSHOTTING_INTERVAL_NUM_BLOCKS;

  // either you're pushing a new balance table with this call, or you're working on the top one
  assert((vOutputValueCounts.size() == nSnapHeight) || (vOutputValueCounts.size() == (nSnapHeight + 1)));
  
  // you're pushing a new balance table with this call, because you are the 
  //  first output of the first block in your snapshotting interval. the counts start from the 
  //  previous interval's 
  //  now finished output value count tables, or all zeroes if we're just pushing block #1 
  //  (snapheight == 0)
  if (vOutputValueCounts.size() == nSnapHeight) {
    if (vOutputValueCounts.size() == 0)
      vOutputValueCounts.push_back(std::vector<int64_t>(UTXO_BALANCE_COUNT_SIZE, 0));
    else
      vOutputValueCounts.push_back(vOutputValueCounts[nSnapHeight - 1]);
  }

  // find the N for this output value: [2^N, 2^(N+1))
  if (nOutputValue > 0) {
    unsigned int nIndex = LogBase2(nOutputValue);
    ++ vOutputValueCounts[nSnapHeight][nIndex];
  }

  // if this is the last push (finishing a block), and this block is the last on its interval,
  // compute/push/refresh the min-balance cached entry for that interval; it's done and WILL be
  // used while we try to connect the very next block!
  if (fLast && (nBlockHeight == SNAPSHOTTING_INTERVAL_NUM_BLOCKS * (nSnapHeight + 1))) {
    
    // the entry must not be there, and the previous entry should be there
    assert(vMinSpendableOutputValues.size() == nSnapHeight);
    
    // find the value
    int64_t nAccumulator = 0;
    CAmount nMinSpendable = MAX_MONEY;
    std::vector<int64_t> & vOutputValueCount = vOutputValueCounts[nSnapHeight];
    for (int nIndex = vOutputValueCount.size() - 1; nIndex >= 0; --nIndex) {
      int64_t nCount = vOutputValueCount[nIndex];
      if (nAccumulator + nCount <= MAX_SNAPSHOT_UTXO_COUNT) {
	nMinSpendable = 1 << nIndex;
      } else {
	break;
      }
    }

    // push the new cache entry
    vMinSpendableOutputValues.push_back(nMinSpendable);
  }
}

void CChain::RemoveUnspentOutputValue(const CAmount& nOutputValue, uint64_t nBlockHeight) {

  // block height 105120 is 105119/105120, so snapheight zero still
  // block height 105121 is snapheight 1
  uint64_t nSnapHeight = (nBlockHeight - 1) / SNAPSHOTTING_INTERVAL_NUM_BLOCKS;

  // either you are working on the top table or the previous table
  // we cannot unstack the empty table even knowing the blockheight because the caller is the one
  //  who iterates over all the transactions/outputs on that block.
  // we can change/fix/improve this if the argument to this call is a list of outputs for a block 
  //  (the whole thing)
  assert((nSnapHeight == (vOutputValueCounts.size() + 1)) || (nSnapHeight == (vOutputValueCounts.size() + 2)));
  
  // if you're working on the previous table, you have to be the last block on that table's interval,
  //  otherwise the caller is buggy (you have to disconnect blocks from the chain in order)
  if (nSnapHeight == (vOutputValueCounts.size() + 2)) {
    // unstacking on snap0 while snap1 table in there, nblockheight better be 105120*(0+1)==105120
    assert(nBlockHeight == SNAPSHOTTING_INTERVAL_NUM_BLOCKS * (nSnapHeight + 1));

    // top snap will be unstacked. assert that it is zeroed out, otherwise there's a counting
    // bug somewhere. (we unstacked the whole thing there, so it should be all zeroes)
    for (int64_t nCount : vOutputValueCounts[nSnapHeight + 1])
      assert(nCount == 0);

    // garbage collected
    vOutputValueCounts.pop_back();
  }

  // find the N for this output value: [2^N, 2^(N+1))
  if (nOutputValue > 0) {
    unsigned int nIndex = LogBase2(nOutputValue);
    -- vOutputValueCounts[nSnapHeight][nIndex];

    assert(vOutputValueCounts[nSnapHeight][nIndex] >= 0);
  }

  // check if we are dirtying the top cached min-spendable value for UTXOs.
  // this happens when you remove ANY unspent output value from a block in the interval
  //  of the top vminspendable cache entry
  assert(nSnapHeight >= vMinSpendableOutputValues.size() + 1); // paranoid debug
  if (nSnapHeight == vMinSpendableOutputValues.size() + 1) {
    vMinSpendableOutputValues.pop_back();
  }  
}
*/

CAmount CChain::GetMinSpendableOutputValue(uint64_t nInputBlockHeight, uint64_t nOutputBlockHeight) {

  // SET TIP NOW DOES ALWAYS SYNC EVERYTHING BECAUSE IT IS BLOCK HEADERS BASED HOORAY
  //
  // If this triggers, it means we didn't update the code correctly to make sure callers of 
  // SetTip() also do the work of updating the unspent output value counts stuff.
  //assert(Tip() == vOutputValueSyncLastTip);

  // 1 to 105120 = region 0
  // 105121 .. 105120*2 = region 1 etc.
  uint64_t nStartInterval = (nOutputBlockHeight - 1) / SNAPSHOTTING_INTERVAL_NUM_BLOCKS;
  uint64_t nEndInterval = (nInputBlockHeight - 1) / SNAPSHOTTING_INTERVAL_NUM_BLOCKS;
  
  // e.g. start 0, end 2; will check vmin[0] and vmin[1], i.e. the mins
  //  computed at end of snapshot intervals 0 and 1.
  CAmount nMinimumFound = 0; //MAX_MONEY; lol
  for (uint64_t nInterval = nStartInterval; nInterval < nEndInterval; ++nInterval)
    nMinimumFound = std::max(nMinimumFound, vMinSpendableOutputValues[nInterval]); // lol its max not min
 
  printf("GetMinSpendable(IN:%lu,OUT:%lu)=%li\n", nInputBlockHeight, nOutputBlockHeight, nMinimumFound);

  return nMinimumFound;
}


/*

  SetTip() is back to what it was. It DOES NOT SYNC vOutputValueCounts
  and vMinSpendableOutputValues. 

  Whoever is CALLING SetTip() has the block data, and only they can push
  or pop blocks and their transactions to synchronize these.

  The CALLER needs to know how to properly call 
  AddUnspentOutputValue()
  RemoveUnspentOutputValue()
  MarkOutputValueCountsSynchronized()

  because there's an order to these. You have to stack and unstack blocks
  transactions in scrict FIFO order, never ever jumping around, and process 
  block transactions in protocol/disk order (for transactions that depend 
  on others in the same block, parent occurs before the child)


  ====================================================================================

  this is no longer so simple.
  
  on set null or set genesis, clear the counters
  
  on CUTTING the chain (fast detect).
  iterate BACKWARDS from the old bigger tip to this shorter tip and unstack the utxos
   from the excluded (disconnected) blocks.
   IF WE CAN I.E. IF THE BLOCKS ARE STILL THERE AND NOT DELETED. (perhaps they are deleted,
   perhaps we should hence put the logic on disconnectblock() or whatever.
  
  on EXPANDING the chain 
  the iteration backwards setting stuff will stumble on the block that was already there.
  iterate FORWARDS from the smaller old tip to the bigger new tip stacking the new utxos
   from the included (connected) blocks.

  then we're probably going to look at connectblock() and validate each transaction based
   on the (output values, output heights) they are trying to use as inputs and see if they 
   were already discarded, invalidating the transaction and hence the block.

  and we pray to have all the info we need at both disconnect/connect (reorg, normal growth
   & when we want to spend we need the input height/age).
  
*/

void CChain::SetTip(CBlockIndex *pindex) {

  // Infinitum:: no blocks or just genesis block: empty dust vote result cache
  //if (pindex == NULL || pindex->pprev == NULL) {
  //vMinSpendableOutputValues.clear();
    //vOutputValueCounts.clear();
    //MarkOutputValueCountsSynchronized();
    //}

  /*
  // no blocks or just genesis block: empty all utxo count stuff
  if (pindex == NULL || pindex->pprev == NULL) {
    vMinSpendableOutputValues.clear();
    vOutputValueCounts.clear();
  }

  if (pindex == NULL) {
    vChain.clear();
    return;
  }

  if (int(vChain.size()) > pindex->nHeight + 1) {
    // shrink. undo N utxos (vchainsize - pindex->nheight+1)

    for (int nHeight = int(vChain.size()) - 1; nHeight > pindex->nHeight; --nHeight) {

      // foreach transaction in block vChain[nHeight]
      //   foreach unspent output that it had introduced: RemoveUnspentOutputValue
      //   foreach output that it had spended: AddUnspentOutputValue
      //
      // call this removetransaction()
      
    }
    
    // REVIEW: hopefully this is right and we have nothing to do other than resize
    //         if we're shrinking
    vChain.resize(pindex->nHeight + 1);

  } else if (int(vChain.size()) < pindex->nHeight + 1) {
    // grow, with possible shrink if "N>0" below:
    // will, first, undo N utxos (N>=0)
    // then, will push N utxos (for those it replaced)
    // then, will push another M utxos (pindex->nheight+1 - vchainsize), 
    //   the guaranteed growth factor)

    // save the old tip. it has the height into it and
    //  can walk backwards to wherever common height we'll find
    CBlockIndex *pindexOldTip = Tip();
    CBlockIndex *pindexLastReplaced = NULL;

    vChain.resize(pindex->nHeight + 1);
    while (pindex && vChain[pindex->nHeight] != pindex) {
        vChain[pindex->nHeight] = pindex;
	pindexLastReplaced = pindex;
        pindex = pindex->pprev;
    }

    // if pindex last replaced is 
    // e.g. height 37
    // and oldtip height was 36 
    // we're fine (no replacements)
    // if last replaced is <= the oldtip height, we have undos
    // the undos are, inclusive: oldtip height to last replaced height, walking backwards
    //
    // iterate over pindexOldTip and its pprev chain, since we have already changed vChain
    //
    for (int nHeight = pindexOldTip->nHeight; nHeight >= pindexLastReplaced->nHeight; --nHeight) {

      // for all transactions on block pindexOldTip->
      //   foreach unspent output that it had introduced: RemoveUnspentOutputValue
      //   foreach output that it had spended: AddUnspentOutputValue

      pindexOldTip = pindexOldTip->pprev;
    }

    // now push the new blocks' utxo value info, including all undone blocks
    // 
    for (int nHeight = pindexLastReplaced->nHeight; nHeight <= Tip()->nHeight; ++nHeight) {

      // for all transactions on block vChain[nHeight]
      //   foreach unspent output that it had introduced: RemoveUnspentOutputValue
      //   foreach output that it had spended: AddUnspentOutputValue
      //   make sure to call fLast with TRUE when it is the last output
      
    }

  }
  */
  
    if (pindex == NULL) {
        vChain.clear();
	vMinSpendableOutputValues.clear(); // Infinitum::
        return;
    }
    vChain.resize(pindex->nHeight + 1);
    while (pindex && vChain[pindex->nHeight] != pindex) {
        vChain[pindex->nHeight] = pindex;
        pindex = pindex->pprev;
    }

    // Infinitum:: update the spendable values tally
    //   for UTXOs that go through snapshot events.
    //
    // FIXME/TODO: MUST do the nonstupid version of this 
    //  that doesn't recompute the entire thing every time a tip is set
    // This is just to get the rest of the code done faster
    //
    vMinSpendableOutputValues.clear(); // yep we are stupid right now
    std::vector<int> vVoteCounts; // element 0 = ndustvote 0's count; element 1 = ndustvote 1's count etc.
    vVoteCounts.resize(0x10000); // uint16_t possible votes
    int nIntervalVotes = 0; // total votes cast into vVoteCounts (i.e. num block headers tallied so far)
    for (uint64_t nIndex = 1; nIndex < vChain.size(); ++nIndex) {

      int nVote = (vChain[nIndex]->nDustVote & 0xFFFF); // don't read the upper 16 bits; they're something else
      if (nVote < 0)
	nVote = 0;

      if (nVote != 30) {
	printf("LOL nIndex %lu voto tiferente de 30: %i,  raw: %u\n", nIndex, nVote, vChain[nIndex]->nDustVote);
      }

      ++vVoteCounts[nVote];
      ++nIntervalVotes;

      // done tallying 2 whole years of block headers
      if (nIntervalVotes >= SNAPSHOTTING_INTERVAL_NUM_BLOCKS) {
	
	CAmount nWinningVote = 0;

	// will discard the lowest twenty percent votes and keep whatever the next vote is,
	// so the dust value is the lowest common denominator among the 80% of miners that
	// are on the side of wanting the highest dust value.
	int nDiscardBudget = SNAPSHOTTING_INTERVAL_NUM_BLOCKS / 5;
	int nIndex = 0;
	BOOST_FOREACH(int nVoteCount, vVoteCounts) {
	  nDiscardBudget -= nVoteCount;
	  if (nDiscardBudget < 0) {
	    nWinningVote = 1 << nIndex; // votes for "0" say dust is 2^0, votes for "1" say dust is 2^1, etc.
	    break;
	  }
	  ++nIndex;
	}

	//if (nWinningVote != 1073741824)
	//  exit(0);
		
	// the winner for these 2 years of block headers we just went by
	vMinSpendableOutputValues.push_back(nWinningVote);

	//if (vMinSpendableOutputValues.size() == 2) {
	//  printf("lala\n");
	//}
	  

	// reset for the next 2 whole years, if there are any
	vVoteCounts.clear();
	vVoteCounts.resize(0x10000);
	nIntervalVotes = 0;
      }
    }
}

CBlockLocator CChain::GetLocator(const CBlockIndex *pindex) const {
    int nStep = 1;
    std::vector<uint256> vHave;
    vHave.reserve(32);

    if (!pindex)
        pindex = Tip();
    while (pindex) {
        vHave.push_back(pindex->GetBlockHash());
        // Stop when we have added the genesis block.
        if (pindex->nHeight == 0)
            break;
        // Exponentially larger steps back, plus the genesis block.
        int nHeight = std::max(pindex->nHeight - nStep, 0);
        if (Contains(pindex)) {
            // Use O(1) CChain index if possible.
            pindex = (*this)[nHeight];
        } else {
            // Otherwise, use O(log n) skiplist.
            pindex = pindex->GetAncestor(nHeight);
        }
        if (vHave.size() > 10)
            nStep *= 2;
    }

    return CBlockLocator(vHave);
}

const CBlockIndex *CChain::FindFork(const CBlockIndex *pindex) const {
    if (pindex == NULL) {
        return NULL;
    }
    if (pindex->nHeight > Height())
        pindex = pindex->GetAncestor(Height());
    while (pindex && !Contains(pindex))
        pindex = pindex->pprev;
    return pindex;
}

/** Turn the lowest '1' bit in the binary representation of a number into a '0'. */
int static inline InvertLowestOne(int n) { return n & (n - 1); }

/** Compute what height to jump back to with the CBlockIndex::pskip pointer. */
int static inline GetSkipHeight(int height) {
    if (height < 2)
        return 0;

    // Determine which height to jump back to. Any number strictly lower than height is acceptable,
    // but the following expression seems to perform well in simulations (max 110 steps to go back
    // up to 2**18 blocks).
    return (height & 1) ? InvertLowestOne(InvertLowestOne(height - 1)) + 1 : InvertLowestOne(height);
}

CBlockIndex* CBlockIndex::GetAncestor(int height)
{
    if (height > nHeight || height < 0)
        return NULL;

    CBlockIndex* pindexWalk = this;
    int heightWalk = nHeight;
    while (heightWalk > height) {
        int heightSkip = GetSkipHeight(heightWalk);
        int heightSkipPrev = GetSkipHeight(heightWalk - 1);
        if (pindexWalk->pskip != NULL &&
            (heightSkip == height ||
             (heightSkip > height && !(heightSkipPrev < heightSkip - 2 &&
                                       heightSkipPrev >= height)))) {
            // Only follow pskip if pprev->pskip isn't better than pskip->pprev.
            pindexWalk = pindexWalk->pskip;
            heightWalk = heightSkip;
        } else {
            pindexWalk = pindexWalk->pprev;
            heightWalk--;
        }
    }
    return pindexWalk;
}

const CBlockIndex* CBlockIndex::GetAncestor(int height) const
{
    return const_cast<CBlockIndex*>(this)->GetAncestor(height);
}

void CBlockIndex::BuildSkip()
{
    if (pprev)
        pskip = pprev->GetAncestor(GetSkipHeight(nHeight));
}

arith_uint256 GetBlockProof(const CBlockIndex& block)
{
    arith_uint256 bnTarget;
    bool fNegative;
    bool fOverflow;
    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);
    if (fNegative || fOverflow || bnTarget == 0)
        return 0;
    // We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
    // as it's too large for a arith_uint256. However, as 2**256 is at least as large
    // as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) / (bnTarget+1)) + 1,
    // or ~bnTarget / (nTarget+1) + 1.
    return (~bnTarget / (bnTarget + 1)) + 1;
}

int64_t GetBlockProofEquivalentTime(const CBlockIndex& to, const CBlockIndex& from, const CBlockIndex& tip, const Consensus::Params& params)
{
    arith_uint256 r;
    int sign = 1;
    if (to.nChainWork > from.nChainWork) {
        r = to.nChainWork - from.nChainWork;
    } else {
        r = from.nChainWork - to.nChainWork;
        sign = -1;
    }
    r = r * arith_uint256(params.nPowTargetSpacing) / GetBlockProof(tip);
    if (r.bits() > 63) {
        return sign * std::numeric_limits<int64_t>::max();
    }
    return sign * r.GetLow64();
}
