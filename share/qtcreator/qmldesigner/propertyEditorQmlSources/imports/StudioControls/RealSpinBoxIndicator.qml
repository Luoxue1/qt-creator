// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

import QtQuick
import QtQuick.Templates as T
import StudioTheme 1.0 as StudioTheme

Rectangle {
    id: control

    property StudioTheme.ControlStyle style: StudioTheme.Values.controlStyle

    property T.Control __parentControl

    property bool hover: spinBoxIndicatorMouseArea.containsMouse
    property bool pressed: spinBoxIndicatorMouseArea.containsPress
    property bool released: false
    property bool realEnabled: true

    signal realPressed
    signal realPressAndHold
    signal realReleased

    property alias iconFlip: spinBoxIndicatorIconScale.yScale

    color: control.style.background.idle
    border.width: 0

    onEnabledChanged: control.syncEnabled()
    onRealEnabledChanged: {
        control.syncEnabled()
        if (control.realEnabled === false) {
            pressAndHoldTimer.stop()
            spinBoxIndicatorMouseArea.pressedAndHeld = false
        }
    }

    // This function is meant to synchronize enabled with realEnabled to avoid
    // the internal logic messing with the actual state.
    function syncEnabled() {
        control.enabled = control.realEnabled
    }

    Timer {
        id: pressAndHoldTimer
        repeat: true
        running: false
        interval: 100
        onTriggered: control.realPressAndHold()
    }

    // This MouseArea is a workaround to avoid some hover state related bugs
    // when using the actual signal 'up.hovered'. QTBUG-74688
    MouseArea {
        id: spinBoxIndicatorMouseArea

        property bool pressedAndHeld: false

        anchors.fill: parent
        hoverEnabled: true
        pressAndHoldInterval: 500
        onPressed: function(mouse) {
            if (control.__parentControl.activeFocus)
                control.forceActiveFocus()

            control.realPressed()
            mouse.accepted = true
        }
        onPressAndHold: {
            pressAndHoldTimer.restart()
            spinBoxIndicatorMouseArea.pressedAndHeld = true
        }
        onReleased: function(mouse) {
            // Only trigger real released when pressAndHold isn't active
            if (!pressAndHoldTimer.running && containsMouse)
                control.realReleased()
            pressAndHoldTimer.stop()
            mouse.accepted = true
            spinBoxIndicatorMouseArea.pressedAndHeld = false
        }
        onEntered: {
            if (spinBoxIndicatorMouseArea.pressedAndHeld)
                pressAndHoldTimer.restart()
        }
        onExited: {
            if (pressAndHoldTimer.running)
                pressAndHoldTimer.stop()
        }
    }

    T.Label {
        id: spinBoxIndicatorIcon
        text: StudioTheme.Constants.upDownSquare2
        color: control.style.icon.idle
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: control.style.smallIconFontSize
        font.family: StudioTheme.Constants.iconFont.family
        anchors.fill: parent
        transform: Scale {
            id: spinBoxIndicatorIconScale
            origin.x: 0
            origin.y: spinBoxIndicatorIcon.height / 2
            yScale: 1
        }

        states: [
            State {
                name: "default"
                when: control.__parentControl.enabled && control.enabled
                      && !control.__parentControl.drag && !control.hover
                      && !control.__parentControl.hover && !control.__parentControl.edit
                PropertyChanges {
                    target: spinBoxIndicatorIcon
                    color: control.style.icon.idle
                }
            },
            State {
                name: "globalHover"
                when: control.__parentControl.enabled && control.enabled
                      && !control.__parentControl.drag && !control.hover
                      && control.__parentControl.hover && !control.__parentControl.edit
                PropertyChanges {
                    target: spinBoxIndicatorIcon
                    color: control.style.icon.idle
                }
            },
            State {
                name: "hover"
                when: control.__parentControl.enabled && control.enabled
                      && !control.__parentControl.drag && control.hover
                      && control.__parentControl.hover && !control.pressed
                PropertyChanges {
                    target: spinBoxIndicatorIcon
                    color: control.style.icon.hover
                }
            },
            State {
                name: "press"
                when: control.__parentControl.enabled && control.enabled
                      && !control.__parentControl.drag && control.pressed
                PropertyChanges {
                    target: spinBoxIndicatorIcon
                    color: control.style.icon.idle
                }
            },
            State {
                name: "edit"
                when: control.__parentControl.edit && control.enabled
                PropertyChanges {
                    target: spinBoxIndicatorIcon
                    color: control.style.icon.idle
                }
            },
            State {
                name: "disable"
                when: !control.__parentControl.enabled || !control.enabled
                PropertyChanges {
                    target: spinBoxIndicatorIcon
                    color: control.style.icon.disabled
                }
            }
        ]
    }

    states: [
        State {
            name: "default"
            when: control.__parentControl.enabled && !control.__parentControl.edit && !control.hover
                  && !control.__parentControl.hover && !control.__parentControl.drag
            PropertyChanges {
                target: spinBoxIndicatorIcon
                visible: false
            }
            PropertyChanges {
                target: control
                color: control.style.background.idle
            }
        },
        State {
            name: "globalHover"
            when: control.__parentControl.enabled && control.enabled
                  && !control.__parentControl.drag && !control.hover
                  && control.__parentControl.hover && !control.__parentControl.edit
            PropertyChanges {
                target: spinBoxIndicatorIcon
                visible: true
            }
            PropertyChanges {
                target: control
                color: control.style.background.globalHover
            }
        },
        State {
            name: "hover"
            when: control.__parentControl.enabled && !control.__parentControl.drag
                  && control.enabled && control.hover && control.__parentControl.hover
                  && !control.pressed
            PropertyChanges {
                target: spinBoxIndicatorIcon
                visible: true
            }
            PropertyChanges {
                target: control
                color: control.style.background.hover
            }
        },
        State {
            name: "press"
            when: control.__parentControl.enabled && control.enabled
                  && !control.__parentControl.drag && control.pressed
            PropertyChanges {
                target: spinBoxIndicatorIcon
                visible: true
            }
            PropertyChanges {
                target: control
                color: control.style.interaction
            }
        },
        State {
            name: "edit"
            when: control.__parentControl.edit && control.__parentControl.enabled && control.enabled
            PropertyChanges {
                target: spinBoxIndicatorIcon
                visible: true
            }
            PropertyChanges {
                target: control
                color: control.style.background.idle
            }
        },
        State {
            name: "drag"
            when: control.__parentControl.drag && control.__parentControl.enabled
            PropertyChanges {
                target: spinBoxIndicatorIcon
                visible: false
            }
            PropertyChanges {
                target: control
                color: control.style.background.interaction
            }
        },
        State {
            name: "disable"
            when: !control.__parentControl.enabled
            PropertyChanges {
                target: spinBoxIndicatorIcon
                visible: false
            }
            PropertyChanges {
                target: control
                color: control.style.background.disabled
            }
        },
        State {
            name: "limit"
            when: !control.enabled && !control.realEnabled && control.__parentControl.hover
            PropertyChanges {
                target: spinBoxIndicatorIcon
                visible: true
            }
            PropertyChanges {
                target: control
                color: control.style.background.idle
            }
        }
    ]
}
