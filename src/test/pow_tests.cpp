// Copyright (c) 2015-2022 The Bitcoin Core developers
// Copyright (c) 2024 The Griffion Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chain.h>
#include <chainparams.h>
#include <pow.h>
#include <test/util/random.h>
#include <test/util/setup_common.h>
#include <util/chaintype.h>

#include <boost/test/unit_test.hpp>

const std::vector<std::pair<uint32_t, uint32_t>> blockIndexData = {
    {1712232000, 0x1f0affff}, {1712232030, 0x1f0affff}, {1712232060, 0x1f0affff}, {1712232090, 0x1f0affff}, {1712232120, 0x1f0affff}, {1712232150, 0x1f0affff}, 
    {1712232180, 0x1f0affff}, {1712232210, 0x1f0affff}, {1712232240, 0x1f0affff}, {1712232270, 0x1f0affff}, {1712232300, 0x1f0affff}, {1712232330, 0x1f0affff},
    {1712232360, 0x1f0affff}, {1712232390, 0x1f0affff}, {1712232420, 0x1f0affff}, {1712232450, 0x1f0affff}, {1712232480, 0x1f0affff}, {1712232510, 0x1f0affff},
    {1712232540, 0x1f0affff}, {1712232570, 0x1f0affff}, {1712232600, 0x1f0affff}, {1712232630, 0x1f0affff}, {1712232660, 0x1f0affff}, {1712232690, 0x1f0affff},
    {1712232720, 0x1f0affff}, {1712232750, 0x1f0affff}, {1712232780, 0x1f0547ad}, {1712232810, 0x1f05119c}, {1712232840, 0x1f04f494}, {1712232870, 0x1f04d77a},
    {1712232900, 0x1f04b9d5}, {1712232930, 0x1f049ba2}, {1712232960, 0x1f047cde}, {1712232990, 0x1f045d85}, {1712233020, 0x1f043d96}, {1712233050, 0x1f041d0d},
    {1712233080, 0x1f03fbe7}, {1712233110, 0x1f03da22}, {1712233140, 0x1f03b7ba}, {1712233170, 0x1f0394ac}, {1712233200, 0x1f0370f6}, {1712233230, 0x1f034c94},
    {1712233260, 0x1f032783}, {1712233290, 0x1f0301c0}, {1712233320, 0x1f02db46}, {1712233350, 0x1f02b414}, {1712233380, 0x1f028c24}, {1712233410, 0x1f026375},
    {1712233440, 0x1f023a02}, {1712233470, 0x1f020fc7}, {1712233500, 0x1f01e4c1}, {1712233530, 0x1f01b8ec}, {1712233560, 0x1f01a74c}, {1712233590, 0x1f0196d5},
    {1712233620, 0x1f01869e}, {1712233650, 0x1f0176a6}, {1712233680, 0x1f0166ef}, {1712233710, 0x1f01577e}, {1712233740, 0x1f014857}, {1712233770, 0x1f01397d},
    {1712233800, 0x1f012af6}, {1712233830, 0x1f011cc5}, {1712233860, 0x1f010ef0}, {1712233890, 0x1f01017b}, {1712233920, 0x1f00f46a}, {1712233950, 0x1f00e7c3},
    {1712233980, 0x1f00db8b}, {1712234010, 0x1f00cfc7}, {1712234040, 0x1f00c47d}, {1712234070, 0x1f00b9b2}, {1712234100, 0x1f00af6d}, {1712234130, 0x1f00a5b3},
    {1712234160, 0x1f009c8a}, {1712234190, 0x1f0093f8}, {1712234220, 0x1f008c05}, {1712234250, 0x1f0084b7}, {1712234280, 0x1e7e152c}, {1712234310, 0x1e7825da},
    {1712234340, 0x1e727113}, {1712234370, 0x1e6cf039}, {1712234400, 0x1e67a2f5}, {1712234430, 0x1e62890e}, {1712234460, 0x1e5da243}, {1712234490, 0x1e58ee3b},
    {1712234520, 0x1e546c84}, {1712234550, 0x1e501ca0}, {1712234580, 0x1e4bfdee}, {1712234610, 0x1e480fbe}, {1712234640, 0x1e445140}, {1712234670, 0x1e40c18a},
    {1712234700, 0x1e3d5f9e}, {1712234730, 0x1e3a2a59}, {1712234760, 0x1e37207e}, {1712234790, 0x1e3440af}, {1712234820, 0x1e31896b}, {1712234850, 0x1e2ef914},
    {1712234880, 0x1e2c8ddf}, {1712234910, 0x1e2a45e4}, {1712234940, 0x1e281f12}, {1712234970, 0x1e261733}, {1712235000, 0x1e242bde}, {1712235030, 0x1e225a85},
    {1712235060, 0x1e20a067}, {1712235090, 0x1e1efa9a}, {1712235120, 0x1e1d685b}, {1712235150, 0x1e1be90e}, {1712235180, 0x1e1a7c16}, {1712235210, 0x1e1920d3},
    {1712235240, 0x1e17d6a5}, {1712235270, 0x1e169cea}, {1712235300, 0x1e1572fe}    
};

const std::vector<std::unique_ptr<CBlockIndex>> GenerateBlockIndexes(const std::vector<std::pair<uint32_t, uint32_t>>& blockIndexData)
{
    std::vector<std::unique_ptr<CBlockIndex>> blockIndexes;

    for (size_t i = 0; i < blockIndexData.size(); ++i) {
        auto height = i;
        auto time = blockIndexData[i].first;
        auto bits = blockIndexData[i].second;
        CBlockIndex* prevBlock = (i == 0) ? nullptr : blockIndexes.back().get();

        auto blockIndex = std::make_unique<CBlockIndex>();
        blockIndex->nHeight = height;
        blockIndex->nTime = time;
        blockIndex->nBits = bits;
        blockIndex->pprev = prevBlock;

        blockIndexes.push_back(std::move(blockIndex));
    }

    return blockIndexes;
}

BOOST_FIXTURE_TEST_SUITE(pow_tests, BasicTestingSetup)

/* Test calculation of next difficulty target with 30 second block times */
BOOST_AUTO_TEST_CASE(get_next_work)
{
    const auto chainParams = CreateChainParams(ChainType::MAIN);
    const auto blockIndexes = GenerateBlockIndexes(blockIndexData);

    for (const auto &blockIndex : blockIndexes) {
        uint32_t nBits = CalculateNextWorkRequired(blockIndex->pprev, chainParams->GetConsensus());

        BOOST_CHECK_EQUAL(nBits, blockIndex->nBits);
        BOOST_CHECK(PermittedDifficultyTransition(chainParams->GetConsensus(), blockIndex->nBits, nBits));
    }
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_negative_target)
{
    const auto consensus = CreateChainParams(ChainType::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits;
    nBits = UintToArith256(consensus.powLimit).GetCompact(true);
    hash.SetHex("0x1");
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_overflow_target)
{
    const auto consensus = CreateChainParams(ChainType::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits{~0x00800000U};
    hash.SetHex("0x1");
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_too_easy_target)
{
    const auto consensus = CreateChainParams(ChainType::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits;
    arith_uint256 nBits_arith = UintToArith256(consensus.powLimit);
    nBits_arith *= 2;
    nBits = nBits_arith.GetCompact();
    hash.SetHex("0x1");
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_biger_hash_than_target)
{
    const auto consensus = CreateChainParams(ChainType::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits;
    arith_uint256 hash_arith = UintToArith256(consensus.powLimit);
    nBits = hash_arith.GetCompact();
    hash_arith *= 2; // hash > nBits
    hash = ArithToUint256(hash_arith);
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_zero_target)
{
    const auto consensus = CreateChainParams(ChainType::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits;
    arith_uint256 hash_arith{0};
    nBits = hash_arith.GetCompact();
    hash = ArithToUint256(hash_arith);
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(GetBlockProofEquivalentTime_test)
{
    const auto chainParams = CreateChainParams(ChainType::MAIN);
    std::vector<CBlockIndex> blocks(10000);
    for (int i = 0; i < 10000; i++) {
        blocks[i].pprev = i ? &blocks[i - 1] : nullptr;
        blocks[i].nHeight = i;
        blocks[i].nTime = 1712232000 + i * chainParams->GetConsensus().nPowTargetSpacing;
        blocks[i].nBits = 0x207fffff; /* target 0x7fffff000... */
        blocks[i].nChainWork = i ? blocks[i - 1].nChainWork + GetBlockProof(blocks[i - 1]) : arith_uint256(0);
    }

    for (int j = 0; j < 1000; j++) {
        CBlockIndex *p1 = &blocks[InsecureRandRange(10000)];
        CBlockIndex *p2 = &blocks[InsecureRandRange(10000)];
        CBlockIndex *p3 = &blocks[InsecureRandRange(10000)];

        int64_t tdiff = GetBlockProofEquivalentTime(*p1, *p2, *p3, chainParams->GetConsensus());
        BOOST_CHECK_EQUAL(tdiff, p1->GetBlockTime() - p2->GetBlockTime());
    }
}

void sanity_check_chainparams(ChainType chain_type)
{
    const auto chainParams = CreateChainParams(chain_type);
    const auto consensus = chainParams->GetConsensus();

    // hash genesis is correct
    BOOST_CHECK_EQUAL(consensus.hashGenesisBlock, chainParams->GenesisBlock().GetHash());

    // genesis nBits is positive, doesn't overflow and is lower than powLimit
    arith_uint256 pow_compact;
    bool neg, over;
    pow_compact.SetCompact(chainParams->GenesisBlock().nBits, &neg, &over);
    BOOST_CHECK(!neg && pow_compact != 0);
    BOOST_CHECK(!over);
    BOOST_CHECK(UintToArith256(consensus.powLimit) >= pow_compact);

    if (!consensus.fPowNoRetargeting) {
        arith_uint256 targ_max{UintToArith256(uint256S("0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"))};
        targ_max /= consensus.nPowTargetWindow*3;
        BOOST_CHECK(UintToArith256(consensus.powLimit) < targ_max);
    }
}

BOOST_AUTO_TEST_CASE(ChainParams_MAIN_sanity)
{
    sanity_check_chainparams(ChainType::MAIN);
}

BOOST_AUTO_TEST_CASE(ChainParams_TESTNET_sanity)
{
    sanity_check_chainparams(ChainType::TESTNET);
}

BOOST_AUTO_TEST_CASE(ChainParams_REGTEST_sanity)
{
    sanity_check_chainparams(ChainType::REGTEST);
}

BOOST_AUTO_TEST_SUITE_END()