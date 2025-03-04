// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2021 The Bitcoin Core developers
// Copyright (c) 2024 The Griffion Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <kernel/chainparams.h>

#include <chainparamsseeds.h>
#include <consensus/amount.h>
#include <consensus/merkle.h>
#include <consensus/params.h>
#include <hash.h>
#include <kernel/messagestartchars.h>
#include <logging.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <script/script.h>
#include <uint256.h>
#include <util/chaintype.h>
#include <util/strencodings.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <type_traits>

static CBlock CreateGenesisBlock(const char* pszTimestamp, uint32_t nTime, uint64_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward, const CScript& genesisOutputScript)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << CScriptNum(0) << 0x1f0affff << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=0008c73af1e55a, ver=4, hashPrevBlock=00000000000000, hashMerkleRoot=9ab6c0, nTime=1740441600, nBits=0x1f0affff, nNonce=0xb52, vtx=1)
 *   CTransaction(hash=9ab6c0, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 0004ffff0a1f01044c575468652054696d6573202d2032352f4665622f32303234202d205472756d7020746f75747320706561636520696e20556b7261696e652077697468696e207765656b73206173206865206d65657473204d6163726f6e2e)
 *     CTxOut(nValue=5000.00000000, scriptPubKey=76a9140160d400545e14a1de78e255c4f3dbe90da649bf88ac)
 *   vMerkleTree: 9ab6c0
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint64_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward, const std::vector<uint8_t>& genesisOutputScriptHex)
{
    const char* pszTimestamp = "The Times - 25/Feb/2024 - Trump touts peace in Ukraine within weeks as he meets Macron.";
    const CScript genesisOutputScript = CScript(genesisOutputScriptHex.begin(), genesisOutputScriptHex.end());
    return CreateGenesisBlock(pszTimestamp, nTime, nNonce, nBits, nVersion, genesisReward, genesisOutputScript);
}

/**
 * Main network on which people trade goods and services.
 */
class CMainParams : public CChainParams {
public:
    CMainParams() {
        m_chain_type = ChainType::MAIN;
        consensus.nSubsidyHalvingInterval = 1051200;
        consensus.BIP16Active = true;
        consensus.BIP34Active = true;
        consensus.BIP65Active = true;
        consensus.BIP66Active = true;
        consensus.CSVActive = true;
        consensus.SegwitActive = true;
        consensus.TaprootActive = true;
        consensus.powLimit = uint256S("0x000affffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetWindow = 25;
        consensus.nPowTargetSpacing = 1 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1815;
        consensus.nMinerConfirmationWindow = 2016;

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0;

        consensus.nMinimumChainWork = uint256S("0x00");
        consensus.defaultAssumeValid = uint256S("0x00");

        consensus.genesisOutputScriptHex = {0x76,0xa9,0x14,0x01,0x60,0xd4,0x00,0x54,0x5e,0x14,0xa1,0xde,0x78,0xe2,0x55,0xc4,0xf3,0xdb,0xe9,0x0d,0xa6,0x49,0xbf,0x88,0xac};

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x87;
        pchMessageStart[1] = 0xaa;
        pchMessageStart[2] = 0xd7;
        pchMessageStart[3] = 0xbc;

        nDefaultPort = 4710;
        nPruneAfterHeight = 100000;
        m_assumed_blockchain_size = 5;
        m_assumed_chain_state_size = 1;

        genesis = CreateGenesisBlock(1740441600, 0xb52, 0x1f0affff, 4, 5000 * COIN, consensus.genesisOutputScriptHex);
        consensus.hashGenesisBlock = genesis.GetHash(genesis.hashMix);

        assert(consensus.hashGenesisBlock == uint256S("0x0008c73af1e55a1a04b613763480876d72c004386746016824c5b9a8c357233b"));
        assert(genesis.hashMerkleRoot == uint256S("0x9ab6c08685ed8f8e078e43ddff9fb39c4892fc5ebed8a7435273a8db6f35add8"));
        assert(genesis.hashMix == uint256S("0xf2236b8d7475a21e682df0ef22fd857da23126661471a7054f0ef4c67b35e81d"));

        vSeeds.emplace_back("dns-mainnet-1.griffion.org");
        vSeeds.emplace_back("dns-mainnet-2.griffion.org");
        vSeeds.emplace_back("dns-mainnet-3.griffion.org");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,38);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,75);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "grf";

        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_main), std::end(chainparams_seed_main));

        fDefaultConsistencyChecks = false;
        m_is_mockable_chain = false;

        checkpointData = {
            {
                { 0, uint256S("0x0008c73af1e55a1a04b613763480876d72c004386746016824c5b9a8c357233b")},
            }
        };

        m_assumeutxo_data = {
            
        };

        chainTxData = ChainTxData{
            .nTime    = 1740441600,
            .nTxCount = 1,
            .dTxRate  = 0
        };
    }
};

/**
 * Testnet (v3): public test network which is reset from time to time.
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        m_chain_type = ChainType::TESTNET;
        consensus.nSubsidyHalvingInterval = 1051200;
        consensus.BIP16Active = true;
        consensus.BIP34Active = true;
        consensus.BIP65Active = true;
        consensus.BIP66Active = true;
        consensus.CSVActive = true;
        consensus.SegwitActive = true;
        consensus.TaprootActive = true;
        consensus.powLimit = uint256S("0x000affffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetWindow = 25;
        consensus.nPowTargetSpacing = 1 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512;
        consensus.nMinerConfirmationWindow = 2016;

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0;

        consensus.nMinimumChainWork = uint256S("0x00");
        consensus.defaultAssumeValid = uint256S("0x00");

        consensus.genesisOutputScriptHex = {0x76,0xa9,0x14,0x9d,0x64,0x04,0x17,0x4b,0x52,0xce,0x9f,0x89,0x77,0x01,0x4a,0xb2,0xd9,0x07,0x9b,0xa1,0xdd,0xcd,0xe8,0x88,0xac};

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xc6;
        pchMessageStart[1] = 0xda;
        pchMessageStart[2] = 0xfa;
        pchMessageStart[3] = 0xe8;

        nDefaultPort = 14710;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 5;
        m_assumed_chain_state_size = 1;

        genesis = CreateGenesisBlock(1740441600, 0x1904, 0x1f0affff, 4, 5000 * COIN, consensus.genesisOutputScriptHex);
        consensus.hashGenesisBlock = genesis.GetHash(genesis.hashMix);

        assert(consensus.hashGenesisBlock == uint256S("0x0006b66c451f2f7eb07d0c5353af869df719b1ba6680d44f774e55a836bae85c"));
        assert(genesis.hashMerkleRoot == uint256S("0x055741e53d6dbf18bcd1e5064b7bdbb66b611622079f243d050f495b7e16214f"));
        assert(genesis.hashMix == uint256S("0x6831b7085643941e5db5d42883c4b399226a02804ee2acb4420c32593e4a2297"));

        vFixedSeeds.clear();
        vSeeds.clear();

        vSeeds.emplace_back("dns-testnet-1.griffion.org");
        vSeeds.emplace_back("dns-testnet-2.griffion.org");
        vSeeds.emplace_back("dns-testnet-3.griffion.org");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,66);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,63);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "trf";

        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_test), std::end(chainparams_seed_test));

        fDefaultConsistencyChecks = false;
        m_is_mockable_chain = false;

        checkpointData = {
            {
                {0, uint256S("0x0006b66c451f2f7eb07d0c5353af869df719b1ba6680d44f774e55a836bae85c")},
            }
        };

        m_assumeutxo_data = {
        };

        chainTxData = ChainTxData{
            .nTime    = 1740441600,
            .nTxCount = 1,
            .dTxRate  = 0
        };
    }
};

/**
 * Regression test: intended for private networks only. Has minimal difficulty to ensure that
 * blocks can be found instantly.
 */
class CRegTestParams : public CChainParams
{
public:
    explicit CRegTestParams()
    {
        m_chain_type = ChainType::REGTEST;
        consensus.nSubsidyHalvingInterval = 1051;
        consensus.BIP16Active = true;
        consensus.BIP34Active = true;
        consensus.BIP65Active = true;
        consensus.BIP66Active = true;
        consensus.CSVActive = true;
        consensus.SegwitActive = true;
        consensus.TaprootActive = true;
        consensus.powLimit = uint256S("0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetWindow = 25;
        consensus.nPowTargetSpacing = 1 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108;
        consensus.nMinerConfirmationWindow = 144;

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0;

        consensus.nMinimumChainWork = uint256S("0x00");
        consensus.defaultAssumeValid = uint256S("0x00");

        consensus.genesisOutputScriptHex = {0x76,0xa9,0x14,0x7f,0x2a,0xc5,0xc1,0x08,0x44,0x21,0xd9,0xc3,0xdd,0xbd,0x81,0x38,0xe5,0x4d,0xae,0x1e,0xbd,0xe9,0x6a,0x88,0xac};

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xec;
        pchMessageStart[1] = 0xdd;
        pchMessageStart[2] = 0xf0;
        pchMessageStart[3] = 0xc1;

        nDefaultPort = 24710;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 0;
        m_assumed_chain_state_size = 0;

        genesis = CreateGenesisBlock(1740441600, 0x01, 0x207fffff, 4, 5000 * COIN, consensus.genesisOutputScriptHex);
        consensus.hashGenesisBlock = genesis.GetHash(genesis.hashMix);

        assert(consensus.hashGenesisBlock == uint256S("0x309071542add6a219c79f3e740407ffd6df2a31c55d34f3fbc4954dd486fddb0"));
        assert(genesis.hashMerkleRoot == uint256S("0x12fbb517a547123c263d050e857810560bf632ba403b5783b2a2a725c61c8943"));
        assert(genesis.hashMix == uint256S("0xb7d118ce9fb412a90eb26443b8f298399ef0b5cbd54f0def4baaba0b2792dec9"));

        vSeeds.clear();
        vSeeds.emplace_back("dummySeed.invalid.");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,66);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,63);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "trf";

        vFixedSeeds.clear();

        fDefaultConsistencyChecks = true;
        m_is_mockable_chain = true;

        checkpointData = {
            {
                {0, uint256S("0x309071542add6a219c79f3e740407ffd6df2a31c55d34f3fbc4954dd486fddb0")},
            }
        };

        m_assumeutxo_data = {
            {
                .height = 110,
                .hash_serialized = AssumeutxoHash{uint256S("0xb8e64fc1f1e2c54a0c67977e09e12d68fc8312af9f3033c4c626f7b6b2f6d549")},
                .nChainTx = 111,
                .blockhash = uint256S("0x446705aaab6a531f893433dd48e12c60ebd22d9b213382ad8e9380b5d66335f9")
            }
        };

        chainTxData = ChainTxData{
            .nTime    = 1740441600,
            .nTxCount = 1,
            .dTxRate  = 0
        };
    }
};

std::unique_ptr<const CChainParams> CChainParams::Main()
{
    return std::make_unique<const CMainParams>();
}

std::unique_ptr<const CChainParams> CChainParams::TestNet()
{
    return std::make_unique<const CTestNetParams>();
}

std::unique_ptr<const CChainParams> CChainParams::RegTest()
{
    return std::make_unique<const CRegTestParams>();
}
