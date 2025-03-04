// Copyright (c) 2021-2022 The Bitcoin Core developers
// Copyright (c) 2024 The Griffion Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef GRIFFION_UTIL_SERFLOAT_H
#define GRIFFION_UTIL_SERFLOAT_H

#include <cstdint>

/* Encode a double using the IEEE 754 binary64 format. All NaNs are encoded as x86/ARM's
 * positive quiet NaN with payload 0. */
uint64_t EncodeDouble(double f) noexcept;
/* Reverse operation of DecodeDouble. DecodeDouble(EncodeDouble(f))==f unless isnan(f). */
double DecodeDouble(uint64_t v) noexcept;

#endif // GRIFFION_UTIL_SERFLOAT_H
