// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "../itestframework.h"

#include "../qtest/qttestsettings.h"

namespace Autotest {
namespace QuickTest {
namespace Constants {

const char FRAMEWORK_NAME[]              = "QtQuickTest";

} // namespace Constants
} // namespace QuickTest

namespace Internal {

class QuickTestFramework : public QtTestFramework
{
public:
    QuickTestFramework() {}
    const char *name() const override;
    QString displayName() const override;
    unsigned priority() const override;

protected:
    ITestParser *createTestParser() override;
    ITestTreeItem *createRootNode() override;
};

QuickTestFramework &theQuickTestFramework();

} // namespace Internal
} // namespace Autotest
