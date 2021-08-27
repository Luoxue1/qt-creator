/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "generatedfile.h"

#include <coreplugin/editormanager/editormanager.h>

#include <utils/fileutils.h>
#include <utils/textfileformat.h>

#include <QCoreApplication>
#include <QDir>
#include <QString>

using namespace Utils;

namespace Core {

/*!
    \class Core::GeneratedFile
    \inheaderfile coreplugin/generatedfile.h
    \inmodule QtCreator

    \brief The GeneratedFile class represents a file generated by a wizard.

    The BaseFileWizard class checks whether each file already exists and
    reports any errors that may occur during creation of the files.

    \sa Core::WizardDialogParameters, Core::BaseFileWizard,
 */

class GeneratedFilePrivate : public QSharedData
{
public:
    GeneratedFilePrivate() = default;
    explicit GeneratedFilePrivate(const Utils::FilePath &path);
    explicit GeneratedFilePrivate(const QString &path);
    Utils::FilePath path;
    QByteArray contents;
    Id editorId;
    bool binary = false;
    GeneratedFile::Attributes attributes;
};

GeneratedFilePrivate::GeneratedFilePrivate(const QString &path) : // FIXME Don't use - Remove when possible
    path(FilePath::fromString(path).cleanPath()),
    attributes({})
{
}

GeneratedFilePrivate::GeneratedFilePrivate(const Utils::FilePath &path) :
    path(path.cleanPath()),
    attributes({})
{
}

GeneratedFile::GeneratedFile() :
    m_d(new GeneratedFilePrivate)
{
}

GeneratedFile::GeneratedFile(const QString &path) : // FIXME Don't use - Remove when possible
    m_d(new GeneratedFilePrivate(path))
{
}

GeneratedFile::GeneratedFile(const Utils::FilePath &path) :
    m_d(new GeneratedFilePrivate(path))
{
}

GeneratedFile::GeneratedFile(const GeneratedFile &rhs) = default;

GeneratedFile &GeneratedFile::operator=(const GeneratedFile &rhs)
{
    if (this != &rhs)
        m_d.operator=(rhs.m_d);
    return *this;
}

GeneratedFile::~GeneratedFile() = default;

QString GeneratedFile::path() const
{
    return m_d->path.toString();
}

FilePath GeneratedFile::filePath() const
{
    return m_d->path;
}

void GeneratedFile::setPath(const QString &p)
{
    m_d->path = Utils::FilePath::fromString(p).cleanPath();
}


void GeneratedFile::setFilePath(const Utils::FilePath &p)
{
    m_d->path = p;
}

QString GeneratedFile::contents() const
{
    return QString::fromUtf8(m_d->contents);
}

void GeneratedFile::setContents(const QString &c)
{
    m_d->contents = c.toUtf8();
}

QByteArray GeneratedFile::binaryContents() const
{
    return m_d->contents;
}

void GeneratedFile::setBinaryContents(const QByteArray &c)
{
    m_d->contents = c;
}

bool GeneratedFile::isBinary() const
{
    return m_d->binary;
}

void GeneratedFile::setBinary(bool b)
{
    m_d->binary = b;
}

Id GeneratedFile::editorId() const
{
    return m_d->editorId;
}

void GeneratedFile::setEditorId(Id id)
{
    m_d->editorId = id;
}

bool GeneratedFile::write(QString *errorMessage) const
{
    // Ensure the directory
    const QDir dir = m_d->path.parentDir().toDir();
    if (!dir.exists()) {
        if (!dir.mkpath(dir.absolutePath())) {
            *errorMessage = QCoreApplication::translate("BaseFileWizard",
                                                        "Unable to create the directory %1.")
                                .arg(QDir::toNativeSeparators(dir.absolutePath()));
            return false;
        }
    }

    // Write out
    if (isBinary()) {
        QIODevice::OpenMode flags = QIODevice::WriteOnly | QIODevice::Truncate;
        Utils::FileSaver saver(m_d->path, flags);
        saver.write(m_d->contents);
        return saver.finalize(errorMessage);
    }

    Utils::TextFileFormat format;
    format.codec = EditorManager::defaultTextCodec();
    format.lineTerminationMode = EditorManager::defaultLineEnding();
    return format.writeFile(m_d->path, contents(), errorMessage);
}

GeneratedFile::Attributes GeneratedFile::attributes() const
{
    return m_d->attributes;
}

void GeneratedFile::setAttributes(Attributes a)
{
    m_d->attributes = a;
}

} // namespace Core
