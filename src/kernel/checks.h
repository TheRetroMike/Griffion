// Copyright (c) 2022 The Bitcoin Core developers
// Copyright (c) 2024 The Griffion Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef GRIFFION_KERNEL_CHECKS_H
#define GRIFFION_KERNEL_CHECKS_H

#include <util/result.h>

namespace kernel {

struct Context;

/**
 *  Ensure a usable environment with all necessary library support.
 */
[[nodiscard]] util::Result<void> SanityChecks(const Context&);
} // namespace kernel

#endif // GRIFFION_KERNEL_CHECKS_H
