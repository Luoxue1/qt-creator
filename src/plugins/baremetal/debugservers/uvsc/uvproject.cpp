// Copyright (C) 2020 Denis Shienkov <denis.shienkov@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "uvproject.h"
#include "uvscserverprovider.h"

#include <cppeditor/cppmodelmanager.h>

#include <debugger/debuggerkitinformation.h>
#include <debugger/debuggerruncontrol.h>

#include <projectexplorer/runcontrol.h>
#include <projectexplorer/session.h>
#include <projectexplorer/target.h>

#include <QFileInfo>
#include <QDir>

using namespace CppEditor;
using namespace Debugger;
using namespace ProjectExplorer;
using namespace Utils;

namespace BareMetal::Internal::Uv {

const char kProjectSchema[] = "2.1";

static QString buildToolsetNumber(int number)
{
    return QStringLiteral("0x%1").arg(QString::number(number, 16));
}

static QString buildPackageId(const DeviceSelection::Package &package)
{
    return QStringLiteral("%1.%2.%3").arg(package.vendorName, package.name, package.version);
}

static QString buildCpu(const DeviceSelection &device)
{
    QString cpu;
    for (const DeviceSelection::Memory &memory : device.memories) {
        const QString id = (memory.id == "IRAM1")
                ? "IRAM" : ((memory.id == "IROM1") ? "IROM" : memory.id);
        cpu += id + '(' +  memory.start + ',' +  memory.size + ") ";
    }
    cpu += "CPUTYPE(\"" + device.cpu.core + "\")";
    return cpu;
}

static QString buildCpuDllName(const DriverSelection &driver)
{
    if (driver.cpuDllIndex < 0 || driver.cpuDllIndex >= driver.cpuDlls.count())
        return {};
    return driver.cpuDlls.at(driver.cpuDllIndex);
}

static QString buildCpuDllParameters(bool isSimulator)
{
    QString params = " -MPU";
    if (isSimulator)
        params.prepend(" -REMAP");
    return params;
}

static void extractAllFiles(const DebuggerRunTool *runTool, QStringList &includes,
                            FilePaths &headers, FilePaths &sources, FilePaths &assemblers)
{
    const auto project = runTool->runControl()->project();
    const CppEditor::ProjectInfo::ConstPtr info = CppModelManager::instance()->projectInfo(project);
    if (!info)
        return;
    const QVector<ProjectPart::ConstPtr> parts = info->projectParts();
    for (const ProjectPart::ConstPtr &part : parts) {
        for (const ProjectFile &file : std::as_const(part->files)) {
            if (!file.active)
                continue;
            const auto path = FilePath::fromString(file.path);
            if (file.isHeader() && !headers.contains(path))
                headers.push_back(path);
            else if (file.isSource() && !sources.contains(path))
                sources.push_back(path);
            else if (file.path.endsWith(".s") && !assemblers.contains(path))
                assemblers.push_back(path);
        }
        for (const HeaderPath &include : std::as_const(part->headerPaths)) {
            if (!includes.contains(include.path))
                includes.push_back(include.path);
        }
    }
}

QString buildPackageId(const DeviceSelection &selection)
{
    return buildPackageId(selection.package);
}

// Project

Project::Project(const UvscServerProvider *provider, DebuggerRunTool *runTool)
{
    appendProperty("SchemaVersion", kProjectSchema);
    appendProperty("Header", "### uVision Project, generated by QtCreator");

    const auto targets = appendChild<Gen::Xml::PropertyGroup>("Targets");
    // Fill 'Target' group.
    m_target = targets->appendPropertyGroup("Target");
    m_target->appendProperty("TargetName", "Template");
    const int toolsetNo = provider->toolsetNumber();
    const QString toolsetNumber = buildToolsetNumber(toolsetNo);
    m_target->appendProperty("ToolsetNumber", toolsetNumber);
    // Fill 'TargetOption' group.
    const auto targetOption = m_target->appendPropertyGroup("TargetOption");

    // Fill 'TargetCommonOption' group.
    const auto targetCommonOption = targetOption->appendPropertyGroup("TargetCommonOption");
    const DeviceSelection device = provider->deviceSelection();
    targetCommonOption->appendProperty("Device", device.name);
    targetCommonOption->appendProperty("Vendor", device.vendorName);
    const QString packageId = buildPackageId(device.package);
    targetCommonOption->appendProperty("PackID", packageId);
    targetCommonOption->appendProperty("PackURL", device.package.url);
    const QString cpu = buildCpu(device);
    targetCommonOption->appendProperty("Cpu", cpu);

    // Fill 'DllOption' group (required for debugging).
    const auto dllOption = targetOption->appendPropertyGroup("DllOption");
    const DriverSelection driver = provider->driverSelection();

    const QString cpuDllName = buildCpuDllName(driver);
    const QString simulatorCpuDllParameters = buildCpuDllParameters(true);
    const QString targetCpuDllParameters = buildCpuDllParameters(false);
    dllOption->appendProperty("SimDllName", cpuDllName);
    dllOption->appendProperty("SimDllArguments", simulatorCpuDllParameters);
    dllOption->appendProperty("TargetDllName", cpuDllName);
    dllOption->appendProperty("TargetDllArguments", targetCpuDllParameters);

    QStringList includes;
    FilePaths headers;
    FilePaths sources;
    FilePaths assemblers;
    extractAllFiles(runTool, includes, headers, sources, assemblers);

    if (toolsetNo == UvscServerProvider::ArmAdsToolsetNumber) {
        // Fill 'TargetArmAds' group (required for debugging).
        const auto targetArmAds = targetOption->appendPropertyGroup("TargetArmAds");
        // Fill 'ArmAdsMisc' group.
        const auto armAdsMisc = targetArmAds->appendPropertyGroup("ArmAdsMisc");
        // Fill 'OnChipMemories' group.
        const auto onChipMemories = armAdsMisc->appendPropertyGroup("OnChipMemories");

        static const struct OnChipEntry {
            QString id;
            QByteArray name;
            int type = -1;
        } entries[] = {
            {{"IROM1"}, "OCR_RVCT4",  1},
            {{"IROM2"}, "OCR_RVCT5",  1},
            {{"IRAM1"}, "OCR_RVCT9",  0},
            {{"IRAM2"}, "OCR_RVCT10", 0},
        };

        const auto entryBegin = std::cbegin(entries);
        const auto entryEnd = std::cend(entries);
        for (const DeviceSelection::Memory &memory : device.memories) {
            const auto entryIt = std::find_if(entryBegin, entryEnd,
                                              [memory](const OnChipEntry &entry) {
                return memory.id == entry.id;
            });
            if (entryIt == entryEnd)
                continue;

            // Fill 'OCR_RVCTn' group.
            const auto ocrRvct = onChipMemories->appendPropertyGroup(entryIt->name);
            ocrRvct->appendProperty("Type", entryIt->type);
            ocrRvct->appendProperty("StartAddress", memory.start);
            ocrRvct->appendProperty("Size", memory.size);
        }

        // Fill 'Cads' group.
        const auto cAds = targetArmAds->appendPropertyGroup("Cads");
        const auto variousControls = cAds->appendPropertyGroup("VariousControls");
        variousControls->appendMultiLineProperty("IncludePath", includes, ';');
    }

    fillAllFiles(headers, sources, assemblers);
}

void Project::fillAllFiles(const FilePaths &headers, const FilePaths &sources,
                           const FilePaths &assemblers)
{
    // Add headers, seources, and assembler files.
    const auto groups = m_target->appendPropertyGroup("Groups");
    const auto group = groups->appendPropertyGroup("Group");
    group->appendProperty("GroupName", "All Files");
    const auto filesGroup = group->appendPropertyGroup("Files");

    enum FileType { SourceFile = 1, AssemblerFile = 2, HeaderFile = 5 };
    auto fillFile = [filesGroup](const FilePath &filePath, FileType fileType) {
        const auto fileGroup = filesGroup->appendPropertyGroup("File");
        fileGroup->appendProperty("FileName", filePath.fileName());
        fileGroup->appendProperty("FileType", fileType);
        fileGroup->appendProperty("FilePath", filePath.toString());
    };

    // Add headers.
    for (const auto &header : headers)
        fillFile(header, FileType::HeaderFile);
    // Add sources.
    for (const auto &source : sources)
        fillFile(source, FileType::SourceFile);
    // Add assemblers.
    for (const auto &assembler : assemblers)
        fillFile(assembler, FileType::AssemblerFile);
}

// ProjectOptions

ProjectOptions::ProjectOptions(const UvscServerProvider *provider)
{
    appendProperty("SchemaVersion", kProjectSchema);
    appendProperty("Header", "### uVision Project, generated by QtCreator");

    // Fill 'Target' group.
    const auto target = appendChild<Gen::Xml::PropertyGroup>("Target");
    target->appendProperty("TargetName", "Template");
    const int toolsetNo = provider->toolsetNumber();
    const QString toolsetNumber = buildToolsetNumber(toolsetNo);
    target->appendProperty("ToolsetNumber", toolsetNumber);

    m_targetOption = target->appendPropertyGroup("TargetOption");

    // Fill 'DebugOpt' group (required for dedugging).
    m_debugOpt = m_targetOption->appendPropertyGroup("DebugOpt");
    const bool useSimulator = provider->isSimulator();
    m_debugOpt->appendProperty("uSim", int(useSimulator));
    m_debugOpt->appendProperty("uTrg", int(!useSimulator));
}

} // namespace BareMetal::Internal::Uv
