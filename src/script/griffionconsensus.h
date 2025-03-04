// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2021 The Bitcoin Core developers
// Copyright (c) 2024 The Griffion Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef GRIFFION_SCRIPT_GRIFFIONCONSENSUS_H
#define GRIFFION_SCRIPT_GRIFFIONCONSENSUS_H

#include <stdint.h>

#if defined(BUILD_GRIFFION_INTERNAL) && defined(HAVE_CONFIG_H)
#include <config/griffion-config.h>
  #if defined(_WIN32)
    #if defined(HAVE_DLLEXPORT_ATTRIBUTE)
      #define EXPORT_SYMBOL __declspec(dllexport)
    #else
      #define EXPORT_SYMBOL
    #endif
  #elif defined(HAVE_DEFAULT_VISIBILITY_ATTRIBUTE)
    #define EXPORT_SYMBOL __attribute__ ((visibility ("default")))
  #endif
#elif defined(MSC_VER) && !defined(STATIC_LIBGRIFFIONCONSENSUS)
  #define EXPORT_SYMBOL __declspec(dllimport)
#endif

#ifndef EXPORT_SYMBOL
  #define EXPORT_SYMBOL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define GRIFFIONCONSENSUS_API_VER 2

typedef enum griffionconsensus_error_t
{
    griffionconsensus_ERR_OK = 0,
    griffionconsensus_ERR_TX_INDEX,
    griffionconsensus_ERR_TX_SIZE_MISMATCH,
    griffionconsensus_ERR_TX_DESERIALIZE,
    griffionconsensus_ERR_AMOUNT_REQUIRED,
    griffionconsensus_ERR_INVALID_FLAGS,
    griffionconsensus_ERR_SPENT_OUTPUTS_REQUIRED,
    griffionconsensus_ERR_SPENT_OUTPUTS_MISMATCH
} griffionconsensus_error;

/** Script verification flags */
enum
{
    griffionconsensus_SCRIPT_FLAGS_VERIFY_NONE                = 0,
    griffionconsensus_SCRIPT_FLAGS_VERIFY_P2SH                = (1U << 0), // evaluate P2SH (BIP16) subscripts
    griffionconsensus_SCRIPT_FLAGS_VERIFY_DERSIG              = (1U << 2), // enforce strict DER (BIP66) compliance
    griffionconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY           = (1U << 4), // enforce NULLDUMMY (BIP147)
    griffionconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY = (1U << 9), // enable CHECKLOCKTIMEVERIFY (BIP65)
    griffionconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY = (1U << 10), // enable CHECKSEQUENCEVERIFY (BIP112)
    griffionconsensus_SCRIPT_FLAGS_VERIFY_WITNESS             = (1U << 11), // enable WITNESS (BIP141)
    griffionconsensus_SCRIPT_FLAGS_VERIFY_TAPROOT             = (1U << 17), // enable TAPROOT (BIPs 341 & 342)
    griffionconsensus_SCRIPT_FLAGS_VERIFY_ALL                 = griffionconsensus_SCRIPT_FLAGS_VERIFY_P2SH | griffionconsensus_SCRIPT_FLAGS_VERIFY_DERSIG |
                                                               griffionconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY | griffionconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY |
                                                               griffionconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY | griffionconsensus_SCRIPT_FLAGS_VERIFY_WITNESS |
                                                               griffionconsensus_SCRIPT_FLAGS_VERIFY_TAPROOT
};

typedef struct {
    const unsigned char *scriptPubKey;
    unsigned int scriptPubKeySize;
    int64_t value;
} UTXO;

/// Returns 1 if the input nIn of the serialized transaction pointed to by
/// txTo correctly spends the scriptPubKey pointed to by scriptPubKey under
/// the additional constraints specified by flags.
/// If not nullptr, err will contain an error/success code for the operation
EXPORT_SYMBOL int griffionconsensus_verify_script(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen,
                                                 const unsigned char *txTo        , unsigned int txToLen,
                                                 unsigned int nIn, unsigned int flags, griffionconsensus_error* err);

EXPORT_SYMBOL int griffionconsensus_verify_script_with_amount(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen, int64_t amount,
                                    const unsigned char *txTo        , unsigned int txToLen,
                                    unsigned int nIn, unsigned int flags, griffionconsensus_error* err);

EXPORT_SYMBOL int griffionconsensus_verify_script_with_spent_outputs(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen, int64_t amount,
                                    const unsigned char *txTo        , unsigned int txToLen,
                                    const UTXO *spentOutputs, unsigned int spentOutputsLen,
                                    unsigned int nIn, unsigned int flags, griffionconsensus_error* err);

EXPORT_SYMBOL unsigned int griffionconsensus_version();

#ifdef __cplusplus
} // extern "C"
#endif

#undef EXPORT_SYMBOL

#endif // GRIFFION_SCRIPT_GRIFFIONCONSENSUS_H
