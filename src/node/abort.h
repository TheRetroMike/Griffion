// Copyright (c) 2023 The Bitcoin Core developers
// Copyright (c) 2024 The Griffion Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef GRIFFION_NODE_ABORT_H
#define GRIFFION_NODE_ABORT_H

#include <util/translation.h>

#include <atomic>
#include <string>

namespace util {
class SignalInterrupt;
} // namespace util

namespace node {
void AbortNode(util::SignalInterrupt* shutdown, std::atomic<int>& exit_status, const std::string& debug_message, const bilingual_str& user_message = {});
} // namespace node

#endif // GRIFFION_NODE_ABORT_H
