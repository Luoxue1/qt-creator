// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "quicktestframework.h"
#include "quicktestparser.h"
#include "quicktesttreeitem.h"

#include "../autotesttr.h"

namespace Autotest {
namespace Internal {

QuickTestFramework &theQuickTestFramework()
{
    static QuickTestFramework framework;
    return framework;
}

ITestParser *QuickTestFramework::createTestParser()
{
    return new QuickTestParser(this);
}

ITestTreeItem *QuickTestFramework::createRootNode()
{
    return new QuickTestTreeItem(this, displayName(), {}, ITestTreeItem::Root);
}

const char *QuickTestFramework::name() const
{
    return QuickTest::Constants::FRAMEWORK_NAME;
}

QString QuickTestFramework::displayName() const
{
    return Tr::tr("Quick Test");
}

unsigned QuickTestFramework::priority() const
{
    return 5;
}

} // namespace Internal
} // namespace Autotest
