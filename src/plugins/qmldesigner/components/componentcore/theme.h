// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <qmldesignercomponents_global.h>

#include <utils/theme/theme.h>

#include <QColor>
#include <QMap>

QT_BEGIN_NAMESPACE
class QQmlEngine;
QT_END_NAMESPACE

namespace QmlDesigner {

class QMLDESIGNERCOMPONENTS_EXPORT Theme : public Utils::Theme
{
    Q_OBJECT
public:
    enum Icon {
        actionIcon,
        actionIconBinding,
        addColumnAfter,
        addColumnBefore,
        addFile,
        addRowAfter,
        addRowBefore,
        addTable,
        add_medium,
        add_small,
        adsClose,
        adsDetach,
        adsDropDown,
        alias,
        aliasAnimated,
        alignBottom,
        alignCenterHorizontal,
        alignCenterVertical,
        alignLeft,
        alignRight,
        alignTo,
        alignToCam_medium,
        alignToCamera_small,
        alignToObject_small,
        alignToView_medium,
        alignTop,
        anchorBaseline,
        anchorBottom,
        anchorFill,
        anchorLeft,
        anchorRight,
        anchorTop,
        anchors_small,
        animatedProperty,
        annotationBubble,
        annotationDecal,
        annotations_large,
        annotations_small,
        applyMaterialToSelected,
        apply_medium,
        apply_small,
        arrange_small,
        arrow_small,
        assign,
        attach_medium,
        back_medium,
        backspace_small,
        bevelAll,
        bevelCorner,
        bezier_medium,
        binding_medium,
        bounds_small,
        branch_medium,
        camera_small,
        centerHorizontal,
        centerVertical,
        cleanLogs_medium,
        closeCross,
        closeFile_large,
        closeLink,
        close_small,
        code,
        colorPopupClose,
        colorSelection_medium,
        columnsAndRows,
        cone_medium,
        cone_small,
        connection_small,
        connections_medium,
        copyLink,
        copyStyle,
        copy_small,
        cornerA,
        cornerB,
        cornersAll,
        createComponent_large,
        createComponent_small,
        create_medium,
        create_small,
        cube_medium,
        cube_small,
        curveDesigner,
        curveDesigner_medium,
        curveEditor,
        customMaterialEditor,
        cylinder_medium,
        cylinder_small,
        decisionNode,
        deleteColumn,
        deleteMaterial,
        deleteRow,
        deleteTable,
        delete_medium,
        delete_small,
        designMode_large,
        detach,
        directionalLight_small,
        distributeBottom,
        distributeCenterHorizontal,
        distributeCenterVertical,
        distributeLeft,
        distributeOriginBottomRight,
        distributeOriginCenter,
        distributeOriginNone,
        distributeOriginTopLeft,
        distributeRight,
        distributeSpacingHorizontal,
        distributeSpacingVertical,
        distributeTop,
        download,
        downloadUnavailable,
        downloadUpdate,
        downloaded,
        dragmarks,
        duplicate_small,
        edit,
        editComponent_large,
        editComponent_small,
        editLightOff_medium,
        editLightOn_medium,
        edit_medium,
        edit_small,
        effects,
        events_small,
        export_medium,
        eyeDropper,
        favorite,
        fitAll_medium,
        fitSelected_small,
        fitSelection_medium,
        fitToView_medium,
        flowAction,
        flowTransition,
        fontStyleBold,
        fontStyleItalic,
        fontStyleStrikethrough,
        fontStyleUnderline,
        forward_medium,
        globalOrient_medium,
        gradient,
        gridView,
        grid_medium,
        group_small,
        help,
        home_large,
        idAliasOff,
        idAliasOn,
        import_medium,
        imported,
        importedModels_small,
        infinity,
        invisible_medium,
        invisible_small,
        keyframe,
        languageList_medium,
        layouts_small,
        lights_small,
        linear_medium,
        linkTriangle,
        linked,
        listView,
        list_medium,
        localOrient_medium,
        lockOff,
        lockOn,
        loopPlayback_medium,
        materialBrowser_medium,
        materialPreviewEnvironment,
        materialPreviewModel,
        material_medium,
        maxBar_small,
        mergeCells,
        merge_small,
        minus,
        mirror,
        more_medium,
        mouseArea_small,
        moveDown_medium,
        moveInwards_medium,
        moveUp_medium,
        moveUpwards_medium,
        move_medium,
        newMaterial,
        nextFile_large,
        normalBar_small,
        openLink,
        openMaterialBrowser,
        orientation,
        orthCam_medium,
        orthCam_small,
        paddingEdge,
        paddingFrame,
        particleAnimation_medium,
        pasteStyle,
        paste_small,
        pause,
        perspectiveCam_medium,
        perspectiveCam_small,
        pin,
        plane_medium,
        plane_small,
        play,
        playFill_medium,
        playOutline_medium,
        plus,
        pointLight_small,
        positioners_small,
        previewEnv_medium,
        previousFile_large,
        promote,
        properties_medium,
        readOnly,
        recordFill_medium,
        recordOutline_medium,
        redo,
        reload_medium,
        remove_medium,
        remove_small,
        rename_small,
        replace_small,
        resetView_small,
        restartParticles_medium,
        reverseOrder_medium,
        roatate_medium,
        rotationFill,
        rotationOutline,
        runProjFill_large,
        runProjOutline_large,
        s_anchors,
        s_annotations,
        s_arrange,
        s_boundingBox,
        s_component,
        s_connections,
        s_edit,
        s_enterComponent,
        s_eventList,
        s_group,
        s_layouts,
        s_merging,
        s_mouseArea,
        s_positioners,
        s_selection,
        s_snapping,
        s_timeline,
        s_visibility,
        saveLogs_medium,
        scale_medium,
        search,
        search_small,
        sectionToggle,
        selectFill_medium,
        selectOutline_medium,
        selectParent_small,
        selection_small,
        settings_medium,
        signal_small,
        snapping_conf_medium,
        snapping_medium,
        snapping_small,
        sphere_medium,
        sphere_small,
        splitColumns,
        splitRows,
        spotLight_small,
        stackedContainer_small,
        startNode,
        step_medium,
        stop_medium,
        testIcon,
        textAlignBottom,
        textAlignCenter,
        textAlignJustified,
        textAlignLeft,
        textAlignMiddle,
        textAlignRight,
        textAlignTop,
        textBulletList,
        textFullJustification,
        textNumberedList,
        textures_medium,
        tickIcon,
        tickMark_small,
        timeline_small,
        toEndFrame_medium,
        toNextFrame_medium,
        toPrevFrame_medium,
        toStartFrame_medium,
        topToolbar_annotations,
        topToolbar_closeFile,
        topToolbar_designMode,
        topToolbar_enterComponent,
        topToolbar_home,
        topToolbar_makeComponent,
        topToolbar_navFile,
        topToolbar_runProject,
        translationCreateFiles,
        translationCreateReport,
        translationExport,
        translationImport,
        translationSelectLanguages,
        translationTest,
        transparent,
        triState,
        triangleArcA,
        triangleArcB,
        triangleCornerA,
        triangleCornerB,
        unLinked,
        undo,
        unify_medium,
        unpin,
        upDownIcon,
        upDownSquare2,
        updateAvailable_medium,
        updateContent_medium,
        visibilityOff,
        visibilityOn,
        visible_medium,
        visible_small,
        wildcard,
        wizardsAutomotive,
        wizardsDesktop,
        wizardsGeneric,
        wizardsMcuEmpty,
        wizardsMcuGraph,
        wizardsMobile,
        wizardsUnknown,
        zoomAll,
        zoomIn,
        zoomIn_medium,
        zoomOut,
        zoomOut_medium,
        zoomSelection
    };
    Q_ENUM(Icon)

    static Theme *instance();
    static QString replaceCssColors(const QString &input);
    static void setupTheme(QQmlEngine *engine);
    static QColor getColor(Color role);
    static QPixmap getPixmap(const QString &id);
    static QString getIconUnicode(Theme::Icon i);
    static QString getIconUnicode(const QString &name);

    static QIcon iconFromName(Theme::Icon i, QColor c = {});

    static int toolbarSize();

    Q_INVOKABLE QColor qmlDesignerBackgroundColorDarker() const;
    Q_INVOKABLE QColor qmlDesignerBackgroundColorDarkAlternate() const;
    Q_INVOKABLE QColor qmlDesignerTabLight() const;
    Q_INVOKABLE QColor qmlDesignerTabDark() const;
    Q_INVOKABLE QColor qmlDesignerButtonColor() const;
    Q_INVOKABLE QColor qmlDesignerBorderColor() const;

    Q_INVOKABLE int smallFontPixelSize() const;
    Q_INVOKABLE int captionFontPixelSize() const;
    Q_INVOKABLE bool highPixelDensity() const;

    Q_INVOKABLE QWindow *mainWindowHandle() const;

private:
    Theme(Utils::Theme *originTheme, QObject *parent);
    QColor evaluateColorAtThemeInstance(const QString &themeColorName);

    QObject *m_constants;
};

} // namespace QmlDesigner
