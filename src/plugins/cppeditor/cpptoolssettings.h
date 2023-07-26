// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "cppeditor_global.h"

#include <QObject>

namespace TextEditor { class CommentsSettings; }

namespace CppEditor {

class CppCodeStylePreferences;

// This class is meant to go away.

class CPPEDITOR_EXPORT CppToolsSettings : public QObject
{
public:
    CppToolsSettings();
    ~CppToolsSettings() override;

    static CppToolsSettings *instance();

    static CppCodeStylePreferences *cppCodeStyle();

    static const TextEditor::CommentsSettings &commentsSettings();
    static void setCommentsSettings(const TextEditor::CommentsSettings &commentsSettings);

    static bool sortedEditorDocumentOutline();
    static void setSortedEditorDocumentOutline(bool sorted);
};

} // namespace CppEditor
