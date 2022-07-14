

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
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Shapes 1.15
import QtQuick.Templates 2.15 as T
import QtQuickDesignerTheme 1.0
import StudioTheme 1.0 as StudioTheme
import StudioControls 1.0 as StudioControls
import QtQuickDesignerColorPalette 1.0

SecondColumnLayout {
    id: colorEditor

    property color color
    property bool supportGradient: false
    property variant backendValue
    property variant value: {
        if (colorEditor.backendValue === undefined
                || colorEditor.backendValue.value === undefined)
            return "white" // default color for Rectangle

        if (colorEditor.isVector3D)
            return Qt.rgba(colorEditor.backendValue.value.x,
                           colorEditor.backendValue.value.y,
                           colorEditor.backendValue.value.z, 1)
        else
            return colorEditor.backendValue.value
    }
    property alias gradientPropertyName: cePopup.gradientPropertyName

    property bool shapeGradients: false
    property color originalColor
    property bool isVector3D: false

    property alias spacer: spacer

    function resetShapeColor() {
        colorEditor.backendValue.resetValue()
    }

    function updateThumbnail() {
        if (!cePopup.gradientLine.hasGradient)
            return

        if (!colorEditor.shapeGradients) {
            var gradientString = "import QtQuick 2.15; Gradient {"
            var orientation = cePopup.gradientOrientation.currentValue
                    === Gradient.Horizontal ? "Gradient.Horizontal" : "Gradient.Vertical"
            gradientString += "orientation: " + orientation + ";"

            for (var i = 0; i < cePopup.gradientLine.model.count; i++)
                gradientString += "GradientStop {}"

            gradientString += "}"

            var gradientObject = Qt.createQmlObject(
                        gradientString, colorEditor.gradientThumbnail,
                        "dynamicGradient")

            for (i = 0; i < cePopup.gradientLine.model.count; i++) {
                gradientObject.stops[i].color = cePopup.gradientLine.model.getColor(
                            i)
                gradientObject.stops[i].position = cePopup.gradientLine.model.getPosition(
                            i)
            }

            colorEditor.gradientThumbnail.gradient = gradientObject
        } else {
            var gradientStr = "import QtQuick 2.15; import QtQuick.Shapes 1.15; "
                    + cePopup.gradientLine.gradientTypeName + " {"

            if (cePopup.gradientLine.gradientTypeName === "LinearGradient") {
                gradientStr += "x1: 0" + ";x2: " + shape.width + ";y1: 0"
                        + ";y2: " + shape.height + ";"
            } else if (cePopup.gradientLine.gradientTypeName === "RadialGradient") {
                gradientStr += "centerX: " + shape.width * 0.5 + ";centerY: "
                        + shape.height * 0.5 + ";focalX: " + shape.width * 0.5 + ";focalY: "
                        + shape.height * 0.5 + ";centerRadius: " + Math.min(
                            shape.width, shape.height) * 0.5 + ";focalRadius: 0" + ";"
            } else if (cePopup.gradientLine.gradientTypeName === "ConicalGradient") {
                gradientStr += "centerX: " + shape.width * 0.5 + ";centerY: "
                        + shape.height * 0.5 + ";angle: 0" + ";"
            }

            for (var j = 0; j < cePopup.gradientLine.model.count; j++)
                gradientStr += "GradientStop {}"

            gradientStr += "}"

            var gradientObj = Qt.createQmlObject(
                        gradientStr, colorEditor.shapeGradientThumbnail,
                        "dynamicShapeGradient")

            for (j = 0; j < cePopup.gradientLine.model.count; j++) {
                gradientObj.stops[j].color = cePopup.gradientLine.model.getColor(
                            j)
                gradientObj.stops[j].position = cePopup.gradientLine.model.getPosition(
                            j)
            }

            colorEditor.shapeGradientThumbnail.fillGradient = gradientObj
        }
    }

    Connections {
        id: backendConnection
        target: colorEditor

        function onValueChanged() {
            if (cePopup.isNotInGradientMode())
                colorEditor.color = colorEditor.value
        }

        function onBackendValueChanged() {
            if (cePopup.isNotInGradientMode())
                colorEditor.color = colorEditor.value
        }
    }

    Timer {
        id: colorEditorTimer
        repeat: false
        interval: 100
        running: false
        onTriggered: {
            backendConnection.enabled = false

            if (colorEditor.backendValue !== undefined) {
                if (colorEditor.isVector3D)
                    colorEditor.backendValue.value = Qt.vector3d(
                                colorEditor.color.r, colorEditor.color.g,
                                colorEditor.color.b)
                else
                    colorEditor.backendValue.value = colorEditor.color
            }

            backendConnection.enabled = true
        }
    }

    onColorChanged: {
        if (!cePopup.gradientLine.isInValidState)
            return

        if (colorEditor.supportGradient && cePopup.gradientLine.hasGradient) {
            var hexColor = convertColorToString(colorEditor.color)
            hexTextField.text = hexColor
            cePopup.popupHexTextField.text = hexColor
            cePopup.gradientLine.currentColor = colorEditor.color
        }

        if (cePopup.isNotInGradientMode())
            colorEditorTimer.restart(
                        ) // Delay setting the color to keep ui responsive
    }

    Spacer {
        implicitWidth: StudioTheme.Values.actionIndicatorWidth
    }

    Rectangle {
        id: preview
        implicitWidth: StudioTheme.Values.twoControlColumnWidth
        implicitHeight: StudioTheme.Values.height
        color: colorEditor.color
        border.color: StudioTheme.Values.themeControlOutline
        border.width: StudioTheme.Values.border

        Rectangle {
            id: gradientThumbnail
            anchors.fill: parent
            anchors.margins: StudioTheme.Values.border
            visible: !cePopup.isNotInGradientMode()
                     && !colorEditor.shapeGradients
                     && cePopup.hasLinearGradient()
        }

        Shape {
            id: shape
            anchors.fill: parent
            anchors.margins: StudioTheme.Values.border
            visible: !cePopup.isNotInGradientMode()
                     && colorEditor.shapeGradients

            ShapePath {
                id: shapeGradientThumbnail
                startX: shape.x - 1
                startY: shape.y - 1
                strokeWidth: -1
                strokeColor: "green"

                PathLine {
                    x: shape.x - 1
                    y: shape.height
                }
                PathLine {
                    x: shape.width
                    y: shape.height
                }
                PathLine {
                    x: shape.width
                    y: shape.y - 1
                }
            }
        }

        Image {
            anchors.fill: parent
            source: "images/checkers.png"
            fillMode: Image.Tile
            z: -1
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                cePopup.opened ? cePopup.close() : cePopup.open()
                forceActiveFocus()
            }
        }

        ColorEditorPopup {
            id: cePopup
            x: cePopup.__defaultX
            y: cePopup.__defaultY
        }
    }

    Spacer {
        implicitWidth: StudioTheme.Values.twoControlColumnGap
    }

    LineEdit {
        id: hexTextField
        implicitWidth: StudioTheme.Values.twoControlColumnWidth
                       + StudioTheme.Values.actionIndicatorWidth
        width: implicitWidth
        enabled: cePopup.isNotInGradientMode()
        writeValueManually: true
        validator: RegExpValidator {
            regExp: /#[0-9A-Fa-f]{6}([0-9A-Fa-f]{2})?/g
        }
        showTranslateCheckBox: false
        backendValue: colorEditor.backendValue

        onAccepted: colorEditor.color = colorFromString(hexTextField.text)
        onCommitData: {
            colorEditor.color = colorFromString(hexTextField.text)
            if (cePopup.isNotInGradientMode()) {
                if (colorEditor.isVector3D) {
                    backendValue.value = Qt.vector3d(colorEditor.color.r,
                                                     colorEditor.color.g,
                                                     colorEditor.color.b)
                } else {
                    backendValue.value = colorEditor.color
                }
            }
        }
    }

    ExpandingSpacer {
        id: spacer
    }

    StudioControls.Menu {
        id: contextMenu

        StudioControls.MenuItem {
            text: qsTr("Open Color Dialog")
            onTriggered: colorPalette.showColorDialog(colorEditor.color)
        }
    }

    Component.onCompleted: cePopup.determineActiveColorMode()

    onBackendValueChanged: {
        cePopup.determineActiveColorMode()
    }
}
