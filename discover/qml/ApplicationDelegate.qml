

/*
 *   SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
 *                           2021 Wang Rui <wangrui@jingos.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */
import QtQuick 2.1
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.1
import "navigation.js" as Navigation
import org.kde.kirigami 2.6 as Kirigami
import QtGraphicalEffects 1.0
import org.kde.discover 2.0
import org.kde.discover.app 1.0
import "cus"

Kirigami.AbstractCard {
    id: delegateArea
    property alias application: installButton.application
    property bool compact: false
    property bool showRating: true
    property int defaultFontSize: 14//theme.defaultFont.pointSize
    property string clickedBannerName
    property alias installButtonText: installButton.text
    property int itemRadius: 10 * appScaleSize

    onClickedBannerNameChanged: {
        if (clickedBannerName === delegateArea.application.name) {
            trigger()
        }
    }

    function intoDetailPage() {
        trigger()
    }

    showClickFeedback: true

    function trigger() {
        getDetails(application, true)
    }
    Keys.onReturnPressed: trigger()
    onClicked: trigger()
    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    hoverEnabled: true
    background: RectDropshadow {
        anchors.fill: parent
        color: "#FFFFFF"
        radius: itemRadius
        shadowColor: "#80C3C9D9"
    }

    Component {
        id: highlightComponent
        Rectangle {
            width: delegateArea.width
            height: delegateArea.height
            radius: itemRadius
            color: delegateArea.pressed ? "#29787880" : "#1F767680"
            Behavior on y {
                SpringAnimation {
                    spring: 3
                    damping: 0.2
                }
            }
        }
    }

    Loader {
        id: hoverLoader
        anchors.fill: delegateArea
        sourceComponent: highlightComponent
        active: hovered && !pressed
    }

    Loader {
        id: pressLoader
        anchors.fill: delegateArea
        sourceComponent: highlightComponent
        active: pressed
    }

    contentItem: Item {
        id: contentDetailItem

        property int itemMargins: (delegateArea.height) * 1 / 8
        anchors {
            top: parent.top
            topMargin: contentDetailItem.itemMargins
            left: parent.left
            leftMargin: contentDetailItem.itemMargins
            right: parent.right
            rightMargin: contentDetailItem.itemMargins
            bottom: parent.bottom
            bottomMargin: contentDetailItem.itemMargins
        }
        clip: true

        Rectangle {
            id: resourceIcon

            anchors {
                left: parent.left
                top: contentDetailItem.top
            }
            height: (delegateArea.height) / 2
            width: height
            radius: height / 10
            color: "#00000000"
            border.color: "#CDD0D7"
            border.width: 1

            Image {
                id: bigImageView
                anchors.centerIn: resourceIcon
                width: parent.width - 2
                height: parent.height - 2
                source: application.icon
                visible: false
                asynchronous: true
                fillMode: Image.Stretch
            }

            Rectangle {
                id: maskRect
                anchors.centerIn: resourceIcon
                anchors.fill: bigImageView
                visible: false
                clip: true
                radius: resourceIcon.radius
            }

            OpacityMask {
                id: mask
                anchors.centerIn: resourceIcon
                anchors.fill: maskRect
                source: bigImageView
                maskSource: maskRect
            }

//            Image {
//                id: resourceIconImage
////                anchors.fill: parent
//                anchors.centerIn: resourceIcon
////                placeholder: "qrc:/img/ic_app_list_empty.png"
//                source: application.icon
//                height: parent.height - 2
//                width: height
//            }
        }

        InstallApplicationButton {
            id: installButton

            anchors {
                bottom: contentDetailItem.bottom
                left: parent.left
            }
            width: resourceIcon.width
            height: width * 2 / 5

            compact: delegateArea.compact
            textSize: defaultFontSize - 5
            text: ((stateFilter
                    === AbstractResource.Installed) ? (installTextString) : (categoryTextString))
        }

        Column {
            id: textColumn

            readonly property bool bigTitle: (head.implicitWidth
                                              + installButton.width) > parent.width
            anchors {
                right: parent.right
                left: resourceIcon.right
                leftMargin: resourceIcon.height / 6
                top: resourceIcon.top
                bottom: parent.bottom
            }
            Layout.fillWidth: true
            spacing: 10

            Text {
                id: head
                anchors {
                    left: parent.left
                    right: parent.right
                }
                Layout.fillWidth: !category.visible || parent.bigTitle
                elide: Text.ElideRight
                text: delegateArea.application.name
                maximumLineCount: 1
                font.pixelSize: delegateArea.defaultFontSize
                font.bold: true
            }

            Text {
                id: category
                anchors {
                    left: parent.left
                    right: parent.right
                }
                Layout.fillWidth: true
                elide: Text.ElideRight
                text: cppClassModel.currentCategoriesName(delegateArea.application.categoryDisplay)
                maximumLineCount: 1
                color: "#99000000"
                font.pixelSize: delegateArea.defaultFontSize - 4
            }

            Text {
                id: summary
                anchors {
                    left: parent.left
                    right: parent.right
                }
                height: textColumn.height - category.contentHeight - head.contentHeight - 20
                elide: Text.ElideRight
                verticalAlignment: Text.AlignTop
                color: "#4D000000"
                text: delegateArea.application.comment
                maximumLineCount: 3
                wrapMode: Text.WrapAnywhere
                font.pixelSize: delegateArea.defaultFontSize - 4
            }
        }
    }
}
