/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#include "pythonlanguageclient.h"

#include "pipsupport.h"
#include "pysideuicextracompiler.h"
#include "pythonconstants.h"
#include "pythonplugin.h"
#include "pythonproject.h"
#include "pythonrunconfiguration.h"
#include "pythonsettings.h"
#include "pythonutils.h"

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/progressmanager/progressmanager.h>
#include <languageclient/languageclientinterface.h>
#include <languageclient/languageclientmanager.h>
#include <languageserverprotocol/textsynchronization.h>
#include <languageserverprotocol/workspace.h>
#include <projectexplorer/extracompiler.h>
#include <projectexplorer/session.h>
#include <projectexplorer/target.h>
#include <texteditor/textdocument.h>
#include <texteditor/texteditor.h>
#include <utils/infobar.h>
#include <utils/qtcprocess.h>
#include <utils/runextensions.h>
#include <utils/variablechooser.h>

#include <QCheckBox>
#include <QComboBox>
#include <QFutureWatcher>
#include <QGridLayout>
#include <QGroupBox>
#include <QJsonDocument>
#include <QPushButton>
#include <QRegularExpression>
#include <QTimer>

using namespace LanguageClient;
using namespace LanguageServerProtocol;
using namespace ProjectExplorer;
using namespace Utils;

namespace Python {
namespace Internal {

static constexpr char startPylsInfoBarId[] = "Python::StartPyls";
static constexpr char installPylsInfoBarId[] = "Python::InstallPyls";
static constexpr char enablePylsInfoBarId[] = "Python::EnablePyls";

class PythonLanguageServerState
{
public:
    enum {
        CanNotBeInstalled,
        CanBeInstalled,
        AlreadyInstalled,
        AlreadyConfigured,
        ConfiguredButDisabled
    } state;
    FilePath pylsModulePath;
};

FilePath getPylsModulePath(CommandLine pylsCommand)
{
    static QMutex mutex; // protect the access to the cache
    QMutexLocker locker(&mutex);
    static QMap<FilePath, FilePath> cache;
    const FilePath &modulePath = cache.value(pylsCommand.executable());
    if (!modulePath.isEmpty())
        return modulePath;

    pylsCommand.addArg("-h");

    QtcProcess pythonProcess;
    Environment env = pythonProcess.environment();
    env.set("PYTHONVERBOSE", "x");
    pythonProcess.setEnvironment(env);
    pythonProcess.setCommand(pylsCommand);
    pythonProcess.runBlocking();

    static const QString pylsInitPattern = "(.*)"
                                           + QRegularExpression::escape(
                                               QDir::toNativeSeparators("/pylsp/__init__.py"))
                                           + '$';
    static const QRegularExpression regexCached(" matches " + pylsInitPattern,
                                                QRegularExpression::MultilineOption);
    static const QRegularExpression regexNotCached(" code object from " + pylsInitPattern,
                                                   QRegularExpression::MultilineOption);

    const QString output = pythonProcess.allOutput();
    for (const auto &regex : {regexCached, regexNotCached}) {
        const QRegularExpressionMatch result = regex.match(output);
        if (result.hasMatch()) {
            const FilePath &modulePath = FilePath::fromUserInput(result.captured(1));
            cache[pylsCommand.executable()] = modulePath;
            return modulePath;
        }
    }
    return {};
}

QList<const StdIOSettings *> configuredPythonLanguageServer()
{
    using namespace LanguageClient;
    QList<const StdIOSettings *> result;
    for (const BaseSettings *setting : LanguageClientManager::currentSettings()) {
        if (setting->m_languageFilter.isSupported("foo.py", Constants::C_PY_MIMETYPE))
            result << dynamic_cast<const StdIOSettings *>(setting);
    }
    return result;
}

static PythonLanguageServerState checkPythonLanguageServer(const FilePath &python)
{
    using namespace LanguageClient;
    const CommandLine pythonLShelpCommand(python, {"-m", "pylsp", "-h"});
    const FilePath &modulePath = getPylsModulePath(pythonLShelpCommand);
    for (const StdIOSettings *serverSetting : configuredPythonLanguageServer()) {
        if (modulePath == getPylsModulePath(serverSetting->command())) {
            return {serverSetting->m_enabled ? PythonLanguageServerState::AlreadyConfigured
                                             : PythonLanguageServerState::ConfiguredButDisabled,
                    FilePath()};
        }
    }

    QtcProcess pythonProcess;
    pythonProcess.setCommand(pythonLShelpCommand);
    pythonProcess.runBlocking();
    if (pythonProcess.allOutput().contains("Python Language Server"))
        return {PythonLanguageServerState::AlreadyInstalled, modulePath};

    pythonProcess.setCommand({python, {"-m", "pip", "-V"}});
    pythonProcess.runBlocking();
    if (pythonProcess.allOutput().startsWith("pip "))
        return {PythonLanguageServerState::CanBeInstalled, FilePath()};
    else
        return {PythonLanguageServerState::CanNotBeInstalled, FilePath()};
}

class PyLSSettingsWidget : public QWidget
{
    Q_DECLARE_TR_FUNCTIONS(PyLSSettingsWidget)
public:
    PyLSSettingsWidget(const PyLSSettings *settings, QWidget *parent)
        : QWidget(parent)
        , m_name(new QLineEdit(settings->m_name, this))
        , m_interpreter(new QComboBox(this))
        , m_configure(new QPushButton(tr("Configure..."), this))
    {
        int row = 0;
        auto *mainLayout = new QGridLayout;
        mainLayout->addWidget(new QLabel(tr("Name:")), row, 0);
        mainLayout->addWidget(m_name, row, 1);
        auto chooser = new VariableChooser(this);
        chooser->addSupportedWidget(m_name);

        mainLayout->addWidget(new QLabel(tr("Python:")), ++row, 0);
        QString settingsId = settings->interpreterId();
        if (settingsId.isEmpty())
            settingsId = PythonSettings::defaultInterpreter().id;
        updateInterpreters(PythonSettings::interpreters(), settingsId);
        mainLayout->addWidget(m_interpreter, row, 1);
        setLayout(mainLayout);

        mainLayout->addWidget(m_configure, ++row, 0);

        connect(PythonSettings::instance(),
                &PythonSettings::interpretersChanged,
                this,
                &PyLSSettingsWidget::updateInterpreters);

        connect(m_configure, &QPushButton::clicked, this, &PyLSSettingsWidget::switchToPylsConfigurePage);
    }

    void updateInterpreters(const QList<Interpreter> &interpreters, const QString &defaultId)
    {
        QString currentId = interpreterId();
        if (currentId.isEmpty())
            currentId = defaultId;
        m_interpreter->clear();
        for (const Interpreter &interpreter : interpreters) {
            if (!interpreter.command.exists())
                continue;
            const QString name = QString(interpreter.name + " (%1)")
                                     .arg(interpreter.command.toUserOutput());
            m_interpreter->addItem(name, interpreter.id);
            if (!currentId.isEmpty() && currentId == interpreter.id)
                m_interpreter->setCurrentIndex(m_interpreter->count() - 1);
        }
    }

    QString name() const { return m_name->text(); }
    QString interpreterId() const { return m_interpreter->currentData().toString(); }

private:
    void switchToPylsConfigurePage()
    {
        Core::ICore::showOptionsDialog(Constants::C_PYLSCONFIGURATION_PAGE_ID);
    }

    QLineEdit *m_name = nullptr;
    QComboBox *m_interpreter = nullptr;
    QPushButton *m_configure = nullptr;
};

PyLSSettings::PyLSSettings()
{
    m_settingsTypeId = Constants::PYLS_SETTINGS_ID;
    m_name = "Python Language Server";
    m_startBehavior = RequiresFile;
    m_languageFilter.mimeTypes = QStringList()
                                 << Constants::C_PY_MIMETYPE << Constants::C_PY3_MIMETYPE;
    m_arguments = "-m pylsp";
}

bool PyLSSettings::isValid() const
{
    return !m_interpreterId.isEmpty() && StdIOSettings::isValid();
}

static const char interpreterKey[] = "interpreter";

QVariantMap PyLSSettings::toMap() const
{
    QVariantMap map = StdIOSettings::toMap();
    map.insert(interpreterKey, m_interpreterId);
    return map;
}

void PyLSSettings::fromMap(const QVariantMap &map)
{
    StdIOSettings::fromMap(map);
    m_languageFilter.mimeTypes = QStringList()
                                 << Constants::C_PY_MIMETYPE << Constants::C_PY3_MIMETYPE;
    setInterpreter(map[interpreterKey].toString());
}

bool PyLSSettings::applyFromSettingsWidget(QWidget *widget)
{
    bool changed = false;
    auto pylswidget = static_cast<PyLSSettingsWidget *>(widget);

    changed |= m_name != pylswidget->name();
    m_name = pylswidget->name();

    changed |= m_interpreterId != pylswidget->interpreterId();
    setInterpreter(pylswidget->interpreterId());

    return changed;
}

QWidget *PyLSSettings::createSettingsWidget(QWidget *parent) const
{
    return new PyLSSettingsWidget(this, parent);
}

BaseSettings *PyLSSettings::copy() const
{
    return new PyLSSettings(*this);
}

void PyLSSettings::setInterpreter(const QString &interpreterId)
{
    m_interpreterId = interpreterId;
    if (m_interpreterId.isEmpty())
        return;
    Interpreter interpreter = Utils::findOrDefault(PythonSettings::interpreters(),
                                                   Utils::equal(&Interpreter::id, interpreterId));
    m_executable = interpreter.command;
}

class PyLSInterface : public StdIOClientInterface
{
public:
    PyLSInterface()
        : m_extraPythonPath("QtCreator-pyls-XXXXXX")
    {
        Environment env = Environment::systemEnvironment();
        env.appendOrSet("PYTHONPATH",
                        m_extraPythonPath.path().toString(),
                        OsSpecificAspects::pathListSeparator(env.osType()));
        setEnvironment(env);
    }
    TemporaryDirectory m_extraPythonPath;
};

BaseClientInterface *PyLSSettings::createInterface(ProjectExplorer::Project *project) const
{
    auto interface = new PyLSInterface;
    interface->setCommandLine(command());
    if (project)
        interface->setWorkingDirectory(project->projectDirectory());
    return interface;
}

PyLSClient::PyLSClient(BaseClientInterface *interface)
    : Client(interface)
    , m_extraCompilerOutputDir(static_cast<PyLSInterface *>(interface)->m_extraPythonPath.path())
{
    connect(this, &Client::initialized, this, &PyLSClient::updateConfiguration);
    connect(PythonSettings::instance(), &PythonSettings::pylsConfigurationChanged,
            this, &PyLSClient::updateConfiguration);
}

void PyLSClient::updateConfiguration()
{
    const auto doc = QJsonDocument::fromJson(PythonSettings::pyLSConfiguration().toUtf8());
    if (doc.isArray())
        Client::updateConfiguration(doc.array());
    else if (doc.isObject())
        Client::updateConfiguration(doc.object());
}

void PyLSClient::openDocument(TextEditor::TextDocument *document)
{
    using namespace LanguageServerProtocol;
    if (reachable()) {
        const FilePath documentPath = document->filePath();
        if (PythonProject *project = pythonProjectForFile(documentPath)) {
            if (Target *target = project->activeTarget()) {
                if (auto rc = qobject_cast<PythonRunConfiguration *>(target->activeRunConfiguration()))
                    updateExtraCompilers(project, rc->extraCompilers());
            }
        } else if (isSupportedDocument(document)) {
            const FilePath workspacePath = documentPath.parentDir();
            if (!m_extraWorkspaceDirs.contains(workspacePath)) {
                WorkspaceFoldersChangeEvent event;
                event.setAdded({WorkSpaceFolder(DocumentUri::fromFilePath(workspacePath),
                                                workspacePath.fileName())});
                DidChangeWorkspaceFoldersParams params;
                params.setEvent(event);
                DidChangeWorkspaceFoldersNotification change(params);
                sendMessage(change);
                m_extraWorkspaceDirs.append(workspacePath);
            }
        }
    }
    Client::openDocument(document);
}

void PyLSClient::projectClosed(ProjectExplorer::Project *project)
{
    for (ProjectExplorer::ExtraCompiler *compiler : m_extraCompilers.value(project))
        closeExtraCompiler(compiler);
    Client::projectClosed(project);
}

void PyLSClient::updateExtraCompilers(ProjectExplorer::Project *project,
                                      const QList<PySideUicExtraCompiler *> &extraCompilers)
{
    auto oldCompilers = m_extraCompilers.take(project);
    for (PySideUicExtraCompiler *extraCompiler : extraCompilers) {
        QTC_ASSERT(extraCompiler->targets().size() == 1 , continue);
        int index = oldCompilers.indexOf(extraCompiler);
        if (index < 0) {
            m_extraCompilers[project] << extraCompiler;
            connect(extraCompiler,
                    &ExtraCompiler::contentsChanged,
                    this,
                    [this, extraCompiler](const FilePath &file) {
                        updateExtraCompilerContents(extraCompiler, file);
                    });
            if (extraCompiler->isDirty())
                static_cast<ExtraCompiler *>(extraCompiler)->run();
        } else {
            m_extraCompilers[project] << oldCompilers.takeAt(index);
        }
    }
    for (ProjectExplorer::ExtraCompiler *compiler : oldCompilers)
        closeExtraCompiler(compiler);
}

void PyLSClient::updateExtraCompilerContents(ExtraCompiler *compiler, const FilePath &file)
{
    const QString text = QString::fromUtf8(compiler->content(file));
    const FilePath target = m_extraCompilerOutputDir.pathAppended(file.fileName());

    target.writeFileContents(compiler->content(file));
}

void PyLSClient::closeExtraCompiler(ProjectExplorer::ExtraCompiler *compiler)
{
    const FilePath file = compiler->targets().first();
    m_extraCompilerOutputDir.pathAppended(file.fileName()).removeFile();
    compiler->disconnect(this);
}

PyLSClient *PyLSClient::clientForPython(const FilePath &python)
{
    if (auto setting = PyLSConfigureAssistant::languageServerForPython(python)) {
        if (auto client = LanguageClientManager::clientsForSetting(setting).value(0))
            return qobject_cast<PyLSClient *>(client);
    }
    return nullptr;
}

Client *PyLSSettings::createClient(BaseClientInterface *interface) const
{
    return new PyLSClient(interface);
}

PyLSConfigureAssistant *PyLSConfigureAssistant::instance()
{
    static auto *instance = new PyLSConfigureAssistant(PythonPlugin::instance());
    return instance;
}

const StdIOSettings *PyLSConfigureAssistant::languageServerForPython(const FilePath &python)
{
    const FilePath pythonModulePath = getPylsModulePath({python, {"-m", "pylsp"}});
    return findOrDefault(configuredPythonLanguageServer(),
                         [pythonModulePath](const StdIOSettings *setting) {
                             return getPylsModulePath(setting->command()) == pythonModulePath;
                         });
}

static Client *registerLanguageServer(const FilePath &python)
{
    Interpreter interpreter = Utils::findOrDefault(PythonSettings::interpreters(),
                                                   Utils::equal(&Interpreter::command, python));
    StdIOSettings *settings = nullptr;
    if (!interpreter.id.isEmpty()) {
        auto *pylsSettings = new PyLSSettings();
        pylsSettings->setInterpreter(interpreter.id);
        settings = pylsSettings;
    } else {
        // cannot find a matching interpreter in settings for the python path add a generic server
        auto *settings = new StdIOSettings();
        settings->m_executable = python;
        settings->m_arguments = "-m pylsp";
        settings->m_languageFilter.mimeTypes = QStringList() << Constants::C_PY_MIMETYPE
                                                             << Constants::C_PY3_MIMETYPE;
    }
    settings->m_name = PyLSConfigureAssistant::tr("Python Language Server (%1)")
                           .arg(pythonName(python));
    LanguageClientManager::registerClientSettings(settings);
    Client *client = LanguageClientManager::clientsForSetting(settings).value(0);
    PyLSConfigureAssistant::updateEditorInfoBars(python, client);
    return client;
}

void PyLSConfigureAssistant::installPythonLanguageServer(const FilePath &python,
                                                         QPointer<TextEditor::TextDocument> document)
{
    document->infoBar()->removeInfo(installPylsInfoBarId);

    // Hide all install info bar entries for this python, but keep them in the list
    // so the language server will be setup properly after the installation is done.
    for (TextEditor::TextDocument *additionalDocument : m_infoBarEntries[python])
        additionalDocument->infoBar()->removeInfo(installPylsInfoBarId);

    auto install = new PipInstallTask(python);

    connect(install, &PipInstallTask::finished, this, [=](const bool success) {
        if (success) {
            if (Client *client = registerLanguageServer(python)) {
                if (document)
                    LanguageClientManager::openDocumentWithClient(document, client);
            }
        }
        install->deleteLater();
    });

    install->setPackage(PipPackage{"python-lsp-server[all]", "Python Language Server"});
    install->run();
}

static void setupPythonLanguageServer(const FilePath &python,
                                      QPointer<TextEditor::TextDocument> document)
{
    document->infoBar()->removeInfo(startPylsInfoBarId);
    if (Client *client = registerLanguageServer(python))
        LanguageClientManager::openDocumentWithClient(document, client);
}

static void enablePythonLanguageServer(const FilePath &python,
                                       QPointer<TextEditor::TextDocument> document)
{
    document->infoBar()->removeInfo(enablePylsInfoBarId);
    if (const StdIOSettings *setting = PyLSConfigureAssistant::languageServerForPython(python)) {
        LanguageClientManager::enableClientSettings(setting->m_id);
        if (const StdIOSettings *setting = PyLSConfigureAssistant::languageServerForPython(python)) {
            if (Client *client = LanguageClientManager::clientsForSetting(setting).value(0)) {
                LanguageClientManager::openDocumentWithClient(document, client);
                PyLSConfigureAssistant::updateEditorInfoBars(python, client);
            }
        }
    }
}

void PyLSConfigureAssistant::openDocumentWithPython(const FilePath &python,
                                                    TextEditor::TextDocument *document)
{
    using CheckPylsWatcher = QFutureWatcher<PythonLanguageServerState>;

    QPointer<CheckPylsWatcher> watcher = new CheckPylsWatcher();

    // cancel and delete watcher after a 10 second timeout
    QTimer::singleShot(10000, instance(), [watcher]() {
        if (watcher) {
            watcher->cancel();
            watcher->deleteLater();
        }
    });

    connect(watcher,
            &CheckPylsWatcher::resultReadyAt,
            instance(),
            [=, document = QPointer<TextEditor::TextDocument>(document)]() {
                if (!document || !watcher)
                    return;
                instance()->handlePyLSState(python, watcher->result(), document);
                watcher->deleteLater();
            });
    watcher->setFuture(Utils::runAsync(&checkPythonLanguageServer, python));
}

void PyLSConfigureAssistant::handlePyLSState(const FilePath &python,
                                             const PythonLanguageServerState &state,
                                             TextEditor::TextDocument *document)
{
    if (state.state == PythonLanguageServerState::CanNotBeInstalled)
        return;
    if (state.state == PythonLanguageServerState::AlreadyConfigured) {
        if (const StdIOSettings *setting = languageServerForPython(python)) {
            if (Client *client = LanguageClientManager::clientsForSetting(setting).value(0))
                LanguageClientManager::openDocumentWithClient(document, client);
        }
        return;
    }

    resetEditorInfoBar(document);
    Utils::InfoBar *infoBar = document->infoBar();
    if (state.state == PythonLanguageServerState::CanBeInstalled
        && infoBar->canInfoBeAdded(installPylsInfoBarId)) {
        auto message = tr("Install and set up Python language server (PyLS) for %1 (%2). "
                          "The language server provides Python specific completion and annotation.")
                           .arg(pythonName(python), python.toUserOutput());
        Utils::InfoBarEntry info(installPylsInfoBarId,
                                 message,
                                 Utils::InfoBarEntry::GlobalSuppression::Enabled);
        info.addCustomButton(tr("Install"),
                             [=]() { installPythonLanguageServer(python, document); });
        infoBar->addInfo(info);
        m_infoBarEntries[python] << document;
    } else if (state.state == PythonLanguageServerState::AlreadyInstalled
               && infoBar->canInfoBeAdded(startPylsInfoBarId)) {
        auto message = tr("Found a Python language server for %1 (%2). "
                          "Set it up for this document?")
                           .arg(pythonName(python), python.toUserOutput());
        Utils::InfoBarEntry info(startPylsInfoBarId,
                                 message,
                                 Utils::InfoBarEntry::GlobalSuppression::Enabled);
        info.addCustomButton(tr("Set Up"), [=]() { setupPythonLanguageServer(python, document); });
        infoBar->addInfo(info);
        m_infoBarEntries[python] << document;
    } else if (state.state == PythonLanguageServerState::ConfiguredButDisabled
               && infoBar->canInfoBeAdded(enablePylsInfoBarId)) {
        auto message = tr("Enable Python language server for %1 (%2)?")
                           .arg(pythonName(python), python.toUserOutput());
        Utils::InfoBarEntry info(enablePylsInfoBarId,
                                 message,
                                 Utils::InfoBarEntry::GlobalSuppression::Enabled);
        info.addCustomButton(tr("Enable"), [=]() { enablePythonLanguageServer(python, document); });
        infoBar->addInfo(info);
        m_infoBarEntries[python] << document;
    }
}

void PyLSConfigureAssistant::updateEditorInfoBars(const FilePath &python, Client *client)
{
    for (TextEditor::TextDocument *document : instance()->m_infoBarEntries.take(python)) {
        instance()->resetEditorInfoBar(document);
        if (client)
            LanguageClientManager::openDocumentWithClient(document, client);
    }
}

void PyLSConfigureAssistant::resetEditorInfoBar(TextEditor::TextDocument *document)
{
    for (QList<TextEditor::TextDocument *> &documents : m_infoBarEntries)
        documents.removeAll(document);
    Utils::InfoBar *infoBar = document->infoBar();
    infoBar->removeInfo(installPylsInfoBarId);
    infoBar->removeInfo(startPylsInfoBarId);
    infoBar->removeInfo(enablePylsInfoBarId);
}

PyLSConfigureAssistant::PyLSConfigureAssistant(QObject *parent)
    : QObject(parent)
{
    Core::EditorManager::instance();

    connect(Core::EditorManager::instance(),
            &Core::EditorManager::documentClosed,
            this,
            [this](Core::IDocument *document) {
                if (auto textDocument = qobject_cast<TextEditor::TextDocument *>(document))
                    resetEditorInfoBar(textDocument);
            });
}

} // namespace Internal
} // namespace Python
