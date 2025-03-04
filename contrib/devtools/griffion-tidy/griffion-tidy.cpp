// Copyright (c) 2023 Griffion Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "logprintf.h"

#include <clang-tidy/ClangTidyModule.h>
#include <clang-tidy/ClangTidyModuleRegistry.h>

class GriffionModule final : public clang::tidy::ClangTidyModule
{
public:
    void addCheckFactories(clang::tidy::ClangTidyCheckFactories& CheckFactories) override
    {
        CheckFactories.registerCheck<griffion::LogPrintfCheck>("griffion-unterminated-logprintf");
    }
};

static clang::tidy::ClangTidyModuleRegistry::Add<GriffionModule>
    X("griffion-module", "Adds griffion checks.");

volatile int GriffionModuleAnchorSource = 0;
