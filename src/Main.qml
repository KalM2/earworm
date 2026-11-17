import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

ApplicationWindow {
    id: win
    width: 960
    height: 680
    minimumWidth: 640
    minimumHeight: 460
    visible: true
    title: qsTr("Earworm")
    color: neoPalette.paper

    Material.theme: theme.light ? Material.Light : Material.Dark
    Material.primary: theme.accent
    Material.background: neoPalette.surface
    Material.foreground: neoPalette.ink
    Material.accent: theme.accent
    Material.roundedScale: theme.radius

    QtObject {
        id: neoPalette
        readonly property color ink: theme.foreground
        readonly property color paper: theme.background
        readonly property color surface: theme.surface
        readonly property color muted: theme.muted
        readonly property color accent: theme.accent
        readonly property color success: "#83c95b"
        readonly property color danger: "#ff786b"
    }

    property bool helpVisible: false
    property bool sleepTimerVisible: false
    property bool bookmarkVisible: false
    property bool themePickerVisible: false
    property bool themeSaveError: false
    property bool errorVisible: false

    function openThemePicker() {
        helpVisible = false
        sleepTimerVisible = false
        bookmarkVisible = false
        themeSaveError = false
        themePickerVisible = true
    }

    function selectPalette(id) {
        if (theme.setPalette(id)) {
            themeSaveError = false
            themePickerVisible = false
        } else {
            themeSaveError = true
        }
    }

    function openBookmarkDialog() {
        if (!backend.currentBook) return

        helpVisible = false
        sleepTimerVisible = false
        bookmarkNameField.text = qsTr("Chapter %1 at %2")
                .arg(backend.player.currentChapter + 1)
                .arg(backend.formatDuration(backend.player.position))
        bookmarkVisible = true
        Qt.callLater(function() {
            bookmarkNameField.forceActiveFocus()
            bookmarkNameField.selectAll()
        })
    }

    function saveBookmark() {
        const label = bookmarkNameField.text.trim()
        backend.addBookmark(label === "" ? qsTr("Bookmark") : label)
        bookmarkVisible = false
    }

    Connections {
        target: backend
        function onErrorChanged() {
            if (backend.error === "") {
                win.errorVisible = false
                errorHideTimer.stop()
            } else {
                win.errorVisible = true
                errorHideTimer.restart()
            }
        }
    }

    Timer {
        id: errorHideTimer
        interval: 8000
        onTriggered: win.errorVisible = false
    }

    // Keyboard shortcuts
    Shortcut { sequence: "?"; context: Qt.ApplicationShortcut; enabled: !win.bookmarkVisible && !win.themePickerVisible; onActivated: win.helpVisible = !win.helpVisible }
    Shortcut { sequence: "Q"; context: Qt.ApplicationShortcut; enabled: !win.bookmarkVisible && !win.themePickerVisible; onActivated: Qt.quit() }
    Shortcut { sequence: "Space"; context: Qt.ApplicationShortcut; enabled: !win.bookmarkVisible && !win.themePickerVisible; onActivated: backend.player.togglePlayPause() }
    Shortcut { sequence: "Right"; context: Qt.ApplicationShortcut; enabled: !win.bookmarkVisible && !win.themePickerVisible; onActivated: backend.player.skipForward(30) }
    Shortcut { sequence: "Left"; context: Qt.ApplicationShortcut; enabled: !win.bookmarkVisible && !win.themePickerVisible; onActivated: backend.player.skipBackward(10) }
    Shortcut { sequence: "N"; context: Qt.ApplicationShortcut; enabled: !win.bookmarkVisible && !win.themePickerVisible; onActivated: backend.player.nextChapter() }
    Shortcut { sequence: "P"; context: Qt.ApplicationShortcut; enabled: !win.bookmarkVisible && !win.themePickerVisible; onActivated: backend.player.previousChapter() }
    Shortcut { sequence: "]"; context: Qt.ApplicationShortcut; enabled: !win.bookmarkVisible && !win.themePickerVisible; onActivated: backend.player.speedUp() }
    Shortcut { sequence: "["; context: Qt.ApplicationShortcut; enabled: !win.bookmarkVisible && !win.themePickerVisible; onActivated: backend.player.speedDown() }
    Shortcut { sequence: "B"; context: Qt.ApplicationShortcut; enabled: !win.bookmarkVisible && !win.themePickerVisible; onActivated: win.openBookmarkDialog() }
    Shortcut { sequence: "S"; context: Qt.ApplicationShortcut; enabled: !win.bookmarkVisible && !win.themePickerVisible; onActivated: win.sleepTimerVisible = !win.sleepTimerVisible }
    Shortcut { sequence: "T"; context: Qt.ApplicationShortcut; enabled: !win.bookmarkVisible; onActivated: {
        if (win.themePickerVisible) win.themePickerVisible = false
        else win.openThemePicker()
    }}
    Shortcut { sequence: "Escape"; context: Qt.ApplicationShortcut; onActivated: {
        if (win.themePickerVisible) win.themePickerVisible = false
        else if (win.bookmarkVisible) win.bookmarkVisible = false
        else if (win.helpVisible) win.helpVisible = false
        else if (win.sleepTimerVisible) win.sleepTimerVisible = false
        else if (backend.view !== "library") backend.backToLibrary()
    }}
    Shortcut { sequence: "O"; context: Qt.ApplicationShortcut; enabled: !win.bookmarkVisible && !win.themePickerVisible; onActivated: backend.chooseFolder() }
    Shortcut { sequence: "C"; context: Qt.ApplicationShortcut; enabled: !win.bookmarkVisible && !win.themePickerVisible; onActivated: {
        if (backend.view === "player") backend.setView("chapters")
        else if (backend.view === "chapters") backend.setView("player")
    }}
    Shortcut { sequence: "M"; context: Qt.ApplicationShortcut; enabled: !win.bookmarkVisible && !win.themePickerVisible; onActivated: {
        if (backend.view === "player") backend.setView("bookmarks")
        else if (backend.view === "bookmarks") backend.setView("player")
    }}

    // Right-to-left languages mirror the window; the Translator sets the
    // application's layout direction.
    Item {
        anchors.fill: parent
        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true

        // Library view
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 16
            visible: backend.view === "library"

            // Header bar
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Label {
                    text: qsTr("Earworm")
                    font.pixelSize: 28
                    font.bold: true
                    color: neoPalette.ink
                }

                Item { Layout.fillWidth: true }

                // Filter tabs
                RowLayout {
                    spacing: 4

                    Repeater {
                        model: [
                            { label: qsTr("All"), value: "all" },
                            { label: qsTr("New"), value: "new" },
                            { label: qsTr("Started"), value: "started" },
                            { label: qsTr("Finished"), value: "finished" }
                        ]

                        NeoButton {
                            text: modelData.label
                            flat: true
                            font.pixelSize: 13
                            highlighted: backend.library.filter === modelData.value
                            Material.roundedScale: Math.min(4, theme.radius)
                            onClicked: backend.library.setFilter(modelData.value)
                        }
                    }
                }

                NeoButton {
                    text: qsTr("Open Folder")
                    icon.name: "folder-open"
                    flat: true
                    font.pixelSize: 13
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: backend.chooseFolder()
                }

                NeoButton {
                    text: qsTr("Theme")
                    flat: true
                    font.pixelSize: 13
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: win.openThemePicker()
                }
            }

            // Empty state
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: backend.library.count === 0

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 16

                    Label {
                        text: backend.library.rootFolder
                              ? qsTr("No audiobooks found")
                              : qsTr("Choose a folder with your audiobooks")
                        font.pixelSize: 18
                        color: neoPalette.muted
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Label {
                        text: qsTr("Each subfolder with audio files becomes a book.\nPress O to open a folder.")
                        font.pixelSize: 13
                        color: neoPalette.muted
                        horizontalAlignment: Text.AlignHCenter
                        Layout.alignment: Qt.AlignHCenter
                    }

                    NeoButton {
                        text: qsTr("Choose Folder")
                        highlighted: true
                        font.pixelSize: 14
                        Material.roundedScale: Math.min(4, theme.radius)
                        Layout.alignment: Qt.AlignHCenter
                        onClicked: backend.chooseFolder()
                    }
                }
            }

            // Book grid
            GridView {
                id: bookGrid
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: backend.library.count > 0
                cellWidth: 200
                cellHeight: 280
                clip: true
                model: backend.library

                delegate: Item {
                    width: bookGrid.cellWidth
                    height: bookGrid.cellHeight

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 8
                        anchors.leftMargin: 12
                        anchors.topMargin: 12
                        color: neoPalette.ink
                        radius: Math.min(4, theme.radius)
                    }

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 8
                        color: bookMouse.containsMouse ? neoPalette.accent : neoPalette.surface
                        radius: Math.min(4, theme.radius)
                        border.color: neoPalette.ink
                        border.width: 2

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 6

                            // Cover
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 160
                                color: neoPalette.surface
                                radius: Math.min(4, theme.radius)
                                border.color: neoPalette.ink
                                border.width: 2
                                clip: true

                                Image {
                                    anchors.fill: parent
                                    source: coverPath ? "file://" + coverPath : ""
                                    fillMode: Image.PreserveAspectCrop
                                    visible: coverPath !== ""
                                }

                                // Fallback: show first letter
                                Label {
                                    anchors.centerIn: parent
                                    text: title ? title.charAt(0).toUpperCase() : "?"
                                    font.pixelSize: 48
                                    font.bold: true
                                    color: neoPalette.ink
                                    visible: !coverPath || coverPath === ""
                                }

                                // Progress bar at bottom of cover
                                Rectangle {
                                    anchors.bottom: parent.bottom
                                    anchors.left: parent.left
                                    width: parent.width * (progress / 100)
                                    height: 3
                                    color: theme.accent
                                    visible: status === "started"
                                }

                                // Status badge
                                Rectangle {
                                    anchors.top: parent.top
                                    anchors.right: parent.right
                                    anchors.margins: 6
                                    width: statusLabel.implicitWidth + 12
                                    height: statusLabel.implicitHeight + 6
                                    radius: Math.min(4, theme.radius)
                                    color: status === "finished" ? neoPalette.success : status === "started" ? theme.accent : "transparent"
                                    visible: status !== "new"
                                    border.color: neoPalette.ink
                                    border.width: 1

                                    Label {
                                        id: statusLabel
                                        anchors.centerIn: parent
                                        text: status === "finished" ? qsTr("Done") : Math.round(progress) + "%"
                                        font.pixelSize: 10
                                        color: status === "started" ? theme.accentForeground : neoPalette.ink
                                    }
                                }
                            }

                            // Title
                            Label {
                                Layout.fillWidth: true
                                text: title
                                font.pixelSize: 13
                                font.bold: true
                                color: bookMouse.containsMouse ? theme.accentForeground : neoPalette.ink
                                elide: Text.ElideRight
                                maximumLineCount: 2
                                wrapMode: Text.WordWrap
                            }

                            // Author
                            Label {
                                Layout.fillWidth: true
                                text: author || ""
                                font.pixelSize: 11
                                color: bookMouse.containsMouse ? theme.accentForeground : neoPalette.muted
                                elide: Text.ElideRight
                                visible: author && author !== ""
                            }

                            // Duration
                            Label {
                                text: backend.formatDuration(totalDuration)
                                font.pixelSize: 11
                                color: bookMouse.containsMouse ? theme.accentForeground : neoPalette.muted
                                visible: totalDuration > 0
                            }

                            Item { Layout.fillHeight: true }
                        }

                        MouseArea {
                            id: bookMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: backend.openBook(index)
                        }
                    }
                }
            }
        }

        // Player view
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 0
            visible: backend.view === "player" && backend.currentBook

            // Top bar
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                NeoButton {
                    text: qsTr("← Library")
                    flat: true
                    font.pixelSize: 13
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: backend.backToLibrary()
                }

                Item { Layout.fillWidth: true }

                NeoButton {
                    text: qsTr("Theme")
                    flat: true
                    font.pixelSize: 13
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: win.openThemePicker()
                }

                NeoButton {
                    text: qsTr("Chapters")
                    flat: true
                    font.pixelSize: 13
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: backend.setView("chapters")
                }

                NeoButton {
                    text: qsTr("Bookmarks")
                    flat: true
                    font.pixelSize: 13
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: backend.setView("bookmarks")
                }
            }

            Item { Layout.fillHeight: true; Layout.maximumHeight: 40 }

            // Cover and info
            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 300
                spacing: 32
                Layout.alignment: Qt.AlignHCenter

                // Cover art
                Rectangle {
                    Layout.preferredWidth: 260
                    Layout.preferredHeight: 260
                    color: neoPalette.surface
                    radius: Math.min(4, theme.radius)
                    border.color: neoPalette.ink
                    border.width: 2
                    clip: true

                    Image {
                        anchors.fill: parent
                        source: backend.currentBook && backend.currentBook.coverPath
                                ? "file://" + backend.currentBook.coverPath : ""
                        fillMode: Image.PreserveAspectCrop
                        visible: backend.currentBook && backend.currentBook.coverPath !== ""
                    }

                    Label {
                        anchors.centerIn: parent
                        text: backend.currentBook ? backend.currentBook.title.charAt(0).toUpperCase() : ""
                        font.pixelSize: 72
                        font.bold: true
                        color: neoPalette.ink
                        visible: !backend.currentBook || !backend.currentBook.coverPath || backend.currentBook.coverPath === ""
                    }
                }

                // Book info
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 8

                    Label {
                        text: backend.currentBook ? backend.currentBook.title : ""
                        font.pixelSize: 24
                        font.bold: true
                        color: neoPalette.ink
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        text: backend.currentBook ? backend.currentBook.author : ""
                        font.pixelSize: 16
                        color: neoPalette.muted
                        visible: backend.currentBook && backend.currentBook.author !== ""
                    }

                    Item { height: 8 }

                    Label {
                        text: backend.player.currentChapterTitle
                        font.pixelSize: 14
                        font.bold: true
                        color: neoPalette.ink
                    }

                    Label {
                        text: backend.currentBook
                              ? qsTr("Chapter %1 of %2").arg(backend.player.currentChapter + 1).arg(backend.currentBook.chapterCount)
                              : ""
                        font.pixelSize: 12
                        color: neoPalette.muted
                    }

                    Item { height: 4 }

                    // Book progress
                    Label {
                        text: backend.currentBook
                              ? qsTr("%1 / %2").arg(backend.formatDuration(backend.currentBook.elapsedMs)).arg(backend.formatDuration(backend.currentBook.totalDurationMs))
                              : ""
                        font.pixelSize: 12
                        color: neoPalette.muted
                    }

                    // Speed indicator
                    Label {
                        text: qsTr("Speed: %1").arg(backend.formatSpeed(backend.player.speed))
                        font.pixelSize: 12
                        color: neoPalette.muted
                    }

                    // Sleep timer indicator
                    Label {
                        text: backend.sleepTimer.active
                              ? qsTr("Sleep in %1").arg(backend.formatDuration(backend.sleepTimer.remainingSeconds * 1000))
                              : ""
                        font.pixelSize: 12
                        font.bold: true
                        color: neoPalette.ink
                        visible: backend.sleepTimer.active
                    }
                }
            }

            Item { Layout.fillHeight: true; Layout.maximumHeight: 32 }

            // Seek bar
            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 40
                Layout.rightMargin: 40
                spacing: 4

                Slider {
                    id: seekSlider
                    Layout.fillWidth: true
                    from: 0
                    to: backend.player.duration > 0 ? backend.player.duration : 1
                    value: backend.player.position
                    Material.accent: theme.accent

                    background: Rectangle {
                        x: seekSlider.leftPadding
                        y: seekSlider.topPadding + (seekSlider.availableHeight - height) / 2
                        width: seekSlider.availableWidth
                        height: 10
                        color: neoPalette.surface
                        border.color: neoPalette.ink
                        border.width: 2
                        radius: Math.min(4, theme.radius)

                        Rectangle {
                            width: seekSlider.visualPosition * parent.width
                            height: parent.height
                            color: theme.accent
                            radius: Math.min(4, theme.radius)
                        }
                    }

                    handle: Rectangle {
                        x: seekSlider.leftPadding + seekSlider.visualPosition * (seekSlider.availableWidth - width)
                        y: seekSlider.topPadding + (seekSlider.availableHeight - height) / 2
                        width: 22
                        height: 22
                        color: theme.accent
                        border.color: neoPalette.ink
                        border.width: 2
                        radius: Math.min(4, theme.radius)
                    }

                    onMoved: backend.player.seekTo(value)
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: backend.formatDuration(backend.player.position)
                        font.pixelSize: 11
                        color: neoPalette.muted
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: backend.formatDuration(backend.player.duration)
                        font.pixelSize: 11
                        color: neoPalette.muted
                    }
                }
            }

            Item { height: 8 }

            // Playback controls
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 24

                // Speed down
                NeoButton {
                    id: speedDownButton
                    contentItem: PlayerControlIcon {
                        iconType: "slower"
                        iconColor: speedDownButton.Material.foreground
                    }
                    flat: true
                    implicitWidth: 44
                    implicitHeight: 44
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: backend.player.speedDown()
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Slow down [")
                }

                // Previous
                NeoButton {
                    id: previousChapterButton
                    contentItem: PlayerControlIcon {
                        iconType: "previous"
                        iconColor: previousChapterButton.Material.foreground
                    }
                    flat: true
                    implicitWidth: 48
                    implicitHeight: 48
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: backend.player.previousChapter()
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Previous chapter (P)")
                }

                // Skip back 10s
                NeoButton {
                    id: rewindButton
                    contentItem: PlayerControlIcon {
                        iconType: "rewind"
                        iconColor: rewindButton.Material.foreground
                    }
                    flat: true
                    implicitWidth: 48
                    implicitHeight: 48
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: backend.player.skipBackward(10)
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Back 10s (←)")
                }

                // Play / Pause
                NeoButton {
                    id: playbackButton
                    text: ""
                    highlighted: true
                    implicitWidth: 64
                    implicitHeight: 64
                    Material.roundedScale: Math.min(4, theme.radius)
                    Material.foreground: theme.accentForeground
                    Accessible.name: backend.player.playing ? qsTr("Pause") : qsTr("Play")
                    onClicked: backend.player.togglePlayPause()
                    ToolTip.visible: hovered
                    ToolTip.text: backend.player.playing ? qsTr("Pause (Space)") : qsTr("Play (Space)")

                    contentItem: PlayerControlIcon {
                        iconType: backend.player.playing ? "pause" : "play"
                        iconColor: playbackButton.Material.foreground
                    }
                }

                // Skip forward 30s
                NeoButton {
                    id: forwardButton
                    contentItem: PlayerControlIcon {
                        iconType: "forward"
                        iconColor: forwardButton.Material.foreground
                    }
                    flat: true
                    implicitWidth: 48
                    implicitHeight: 48
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: backend.player.skipForward(30)
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Forward 30s (→)")
                }

                // Next
                NeoButton {
                    id: nextChapterButton
                    contentItem: PlayerControlIcon {
                        iconType: "next"
                        iconColor: nextChapterButton.Material.foreground
                    }
                    flat: true
                    implicitWidth: 48
                    implicitHeight: 48
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: backend.player.nextChapter()
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Next chapter (N)")
                }

                // Speed up
                NeoButton {
                    id: speedUpButton
                    contentItem: PlayerControlIcon {
                        iconType: "faster"
                        iconColor: speedUpButton.Material.foreground
                    }
                    flat: true
                    implicitWidth: 44
                    implicitHeight: 44
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: backend.player.speedUp()
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Speed up ]")
                }
            }

            Item { height: 16 }

            // Bottom action bar
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 16

                NeoButton {
                    text: qsTr("Bookmark")
                    flat: true
                    font.pixelSize: 12
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: win.openBookmarkDialog()
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Add bookmark (B)")
                }

                NeoButton {
                    text: qsTr("Sleep Timer")
                    flat: true
                    font.pixelSize: 12
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: win.sleepTimerVisible = true
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Sleep timer (S)")
                }
            }

            Item { Layout.fillHeight: true }
        }

        // Chapters view
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 12
            visible: backend.view === "chapters" && backend.currentBook

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                NeoButton {
                    text: qsTr("← Player")
                    flat: true
                    font.pixelSize: 13
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: backend.setView("player")
                }

                Item { Layout.fillWidth: true }

                Label {
                    text: qsTr("Chapters")
                    font.pixelSize: 20
                    font.bold: true
                    color: neoPalette.ink
                }

                Item { Layout.fillWidth: true }

                NeoButton {
                    text: qsTr("Theme")
                    flat: true
                    font.pixelSize: 13
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: win.openThemePicker()
                }
            }

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: backend.currentBook ? backend.currentBook.chapterCount : 0

                delegate: Item {
                    width: ListView.view.width
                    height: 52

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 3
                        color: index === backend.player.currentChapter || chapterMouse.containsMouse ? theme.accent : neoPalette.surface
                        radius: Math.min(4, theme.radius)
                        border.color: neoPalette.ink
                        border.width: 2

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 16
                            anchors.rightMargin: 16
                            spacing: 12

                            Label {
                                text: (index + 1) + "."
                                font.pixelSize: 13
                                color: index === backend.player.currentChapter || chapterMouse.containsMouse
                                       ? theme.accentForeground : neoPalette.ink
                                Layout.preferredWidth: 32
                            }

                            Label {
                                text: backend.currentBook ? backend.currentBook.chapterTitle(index) : ""
                                font.pixelSize: 13
                                color: index === backend.player.currentChapter || chapterMouse.containsMouse
                                       ? theme.accentForeground : neoPalette.ink
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }

                            Label {
                                text: backend.currentBook ? backend.formatDuration(backend.currentBook.chapterDurationMs(index)) : ""
                                font.pixelSize: 11
                                color: index === backend.player.currentChapter || chapterMouse.containsMouse
                                       ? theme.accentForeground : neoPalette.muted
                            }

                            // Playing indicator
                            PlayerControlIcon {
                                iconType: "play"
                                iconColor: theme.accentForeground
                                visible: index === backend.player.currentChapter && backend.player.playing
                            }
                        }

                        MouseArea {
                            id: chapterMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                backend.player.goToChapter(index)
                                backend.player.play()
                                backend.setView("player")
                            }
                        }
                    }
                }
            }
        }

        // Bookmarks view
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 12
            visible: backend.view === "bookmarks" && backend.currentBook

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                NeoButton {
                    text: qsTr("← Player")
                    flat: true
                    font.pixelSize: 13
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: backend.setView("player")
                }

                Item { Layout.fillWidth: true }

                Label {
                    text: qsTr("Bookmarks")
                    font.pixelSize: 20
                    font.bold: true
                    color: neoPalette.ink
                }

                Item { Layout.fillWidth: true }

                NeoButton {
                    text: qsTr("Add bookmark")
                    flat: true
                    font.pixelSize: 12
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: win.openBookmarkDialog()
                }

                NeoButton {
                    text: qsTr("Theme")
                    flat: true
                    font.pixelSize: 13
                    Material.roundedScale: Math.min(4, theme.radius)
                    onClicked: win.openThemePicker()
                }
            }

            // Empty state
            Label {
                text: qsTr("No bookmarks yet. Press B to add one.")
                font.pixelSize: 14
                color: neoPalette.muted
                Layout.alignment: Qt.AlignHCenter
                visible: backend.bookmarkModel.count === 0
            }

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: backend.bookmarkModel
                visible: backend.bookmarkModel.count > 0

                delegate: Item {
                    width: ListView.view.width
                    height: 52

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 3
                        color: bmMouse.containsMouse ? theme.accent : neoPalette.surface
                        radius: Math.min(4, theme.radius)
                        border.color: neoPalette.ink
                        border.width: 2

                        RowLayout {
                            z: 1
                            anchors.fill: parent
                            anchors.leftMargin: 16
                            anchors.rightMargin: 16
                            spacing: 12

                            Label {
                                text: label
                                font.pixelSize: 13
                                color: bmMouse.containsMouse ? theme.accentForeground : neoPalette.ink
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }

                            Label {
                                text: backend.currentBook
                                      ? qsTr("Ch. %1 at %2").arg(chapter + 1).arg(backend.formatDuration(position))
                                      : ""
                                font.pixelSize: 11
                                color: bmMouse.containsMouse ? theme.accentForeground : neoPalette.muted
                            }

                            NeoButton {
                                text: "✕"
                                flat: true
                                font.pixelSize: 14
                                implicitWidth: 32
                                implicitHeight: 32
                                Material.roundedScale: Math.min(4, theme.radius)
                                onClicked: backend.removeBookmark(index)
                            }
                        }

                        MouseArea {
                            id: bmMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: backend.goToBookmark(index)
                        }
                    }
                }
            }
        }

        // Bookmark naming popup
        Rectangle {
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: 4
            anchors.verticalCenterOffset: 4
            width: bookmarkDialog.width
            height: bookmarkDialog.height
            color: neoPalette.ink
            radius: Math.min(8, theme.radius)
            visible: win.bookmarkVisible
            z: 99
        }

        Rectangle {
            id: bookmarkDialog
            objectName: "bookmarkDialog"
            anchors.centerIn: parent
            width: Math.min(parent.width - 40, 440)
            height: bookmarkColumn.implicitHeight + 40
            color: neoPalette.surface
            radius: Math.min(8, theme.radius)
            border.color: neoPalette.ink
            border.width: 2
            visible: win.bookmarkVisible
            z: 100

            ColumnLayout {
                id: bookmarkColumn
                anchors.fill: parent
                anchors.margins: 20
                spacing: 12

                Label {
                    text: qsTr("New Bookmark")
                    font.pixelSize: 18
                    font.bold: true
                    color: neoPalette.ink
                }

                Label {
                    text: qsTr("Chapter %1 at %2")
                            .arg(backend.player.currentChapter + 1)
                            .arg(backend.formatDuration(backend.player.position))
                    font.pixelSize: 13
                    color: neoPalette.muted
                }

                TextField {
                    id: bookmarkNameField
                    objectName: "bookmarkNameField"
                    Layout.fillWidth: true
                    maximumLength: 100
                    placeholderText: qsTr("Bookmark name")
                    selectByMouse: true
                    font.pixelSize: 14
                    color: neoPalette.ink
                    Material.roundedScale: Math.min(4, theme.radius)
                    background: Rectangle {
                        color: neoPalette.surface
                        radius: Math.min(4, theme.radius)
                        border.color: neoPalette.ink
                        border.width: 2
                    }
                    onAccepted: win.saveBookmark()
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Item { Layout.fillWidth: true }

                    NeoButton {
                        text: qsTr("Cancel")
                        flat: true
                        font.pixelSize: 12
                        Material.roundedScale: Math.min(4, theme.radius)
                        onClicked: win.bookmarkVisible = false
                    }

                    NeoButton {
                        text: qsTr("Save")
                        flat: true
                        highlighted: true
                        font.pixelSize: 12
                        Material.roundedScale: Math.min(4, theme.radius)
                        onClicked: win.saveBookmark()
                    }
                }
            }
        }

        // App-local theme picker
        Rectangle {
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: 4
            anchors.verticalCenterOffset: 4
            width: themePicker.width
            height: themePicker.height
            color: neoPalette.ink
            radius: Math.min(8, theme.radius)
            visible: win.themePickerVisible
            z: 109
        }

        Rectangle {
            id: themePicker
            objectName: "themePicker"
            anchors.centerIn: parent
            width: Math.min(parent.width - 32, 620)
            height: Math.min(parent.height - 32, 560)
            color: neoPalette.surface
            radius: Math.min(8, theme.radius)
            border.color: neoPalette.ink
            border.width: 2
            visible: win.themePickerVisible
            z: 110

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: qsTr("Choose theme")
                        font.pixelSize: 18
                        font.bold: true
                        color: neoPalette.ink
                    }

                    Item { Layout.fillWidth: true }

                    Label {
                        text: theme.paletteName
                        font.pixelSize: 12
                        color: neoPalette.muted
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 58
                    color: theme.background
                    radius: Math.min(4, theme.radius)
                    border.color: neoPalette.ink
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        Rectangle {
                            Layout.preferredWidth: 34
                            Layout.fillHeight: true
                            color: theme.accent
                            radius: Math.min(4, theme.radius)
                            border.color: theme.foreground
                            border.width: 1
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Label {
                                text: theme.paletteName
                                font.pixelSize: 13
                                font.bold: true
                                color: theme.foreground
                            }

                            Label {
                                text: qsTr("Preview of the selected palette")
                                font.pixelSize: 10
                                color: theme.muted
                            }
                        }
                    }
                }

                GridView {
                    id: themeGrid
                    objectName: "themeGrid"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    cellWidth: Math.max(1, width / 4)
                    cellHeight: 62
                    model: theme.palettes
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds

                    delegate: Button {
                        id: paletteButton
                        property var paletteEntry: modelData
                        width: themeGrid.cellWidth - 8
                        height: themeGrid.cellHeight - 8
                        padding: 6
                        focusPolicy: Qt.StrongFocus
                        Accessible.name: paletteEntry.name

                        background: Rectangle {
                            color: paletteEntry.surface
                            radius: Math.min(4, theme.radius)
                            border.color: paletteEntry.id === theme.paletteId
                                          ? paletteEntry.accent : paletteEntry.foreground
                            border.width: paletteEntry.id === theme.paletteId ? 3 : 1
                        }

                        contentItem: Column {
                            spacing: 3

                            Row {
                                spacing: 4

                                Repeater {
                                    model: [paletteButton.paletteEntry.background,
                                            paletteButton.paletteEntry.accent,
                                            paletteButton.paletteEntry.foreground]

                                    Rectangle {
                                        width: 18
                                        height: 9
                                        radius: 2
                                        color: modelData
                                        border.color: paletteButton.paletteEntry.foreground
                                        border.width: 1
                                    }
                                }
                            }

                            Label {
                                width: parent.width
                                text: paletteButton.paletteEntry.name
                                font.pixelSize: 10
                                color: paletteButton.paletteEntry.foreground
                                elide: Text.ElideRight
                            }
                        }

                        onClicked: win.selectPalette(paletteEntry.id)
                    }

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }
                }

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Could not save theme choice")
                    font.pixelSize: 11
                    color: neoPalette.danger
                    visible: win.themeSaveError
                }

                NeoButton {
                    text: qsTr("Close")
                    flat: true
                    font.pixelSize: 12
                    Material.roundedScale: Math.min(4, theme.radius)
                    Layout.alignment: Qt.AlignRight
                    onClicked: win.themePickerVisible = false
                }
            }
        }

        // Sleep timer popup
        Rectangle {
            anchors.centerIn: parent
            width: 320
            height: sleepCol.implicitHeight + 40
            color: neoPalette.surface
            radius: Math.min(8, theme.radius)
            border.color: neoPalette.ink
            border.width: 2
            visible: win.sleepTimerVisible
            z: 100

            ColumnLayout {
                id: sleepCol
                anchors.fill: parent
                anchors.margins: 20
                spacing: 12

                Label {
                    text: qsTr("Sleep Timer")
                    font.pixelSize: 18
                    font.bold: true
                    color: neoPalette.ink
                }

                Label {
                    text: backend.sleepTimer.active
                          ? qsTr("Time remaining: %1").arg(backend.formatDuration(backend.sleepTimer.remainingSeconds * 1000))
                          : qsTr("Set a timer to pause playback")
                    font.pixelSize: 13
                    color: neoPalette.muted
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 8

                    Repeater {
                        model: [5, 10, 15, 30, 45, 60]

                        NeoButton {
                            text: qsTr("%1 min").arg(modelData)
                            flat: true
                            font.pixelSize: 12
                            Material.roundedScale: Math.min(4, theme.radius)
                            onClicked: {
                                backend.sleepTimer.start(modelData)
                                win.sleepTimerVisible = false
                            }
                        }
                    }
                }

                RowLayout {
                    spacing: 8
                    visible: backend.sleepTimer.active

                    NeoButton {
                        text: qsTr("+5 min")
                        flat: true
                        font.pixelSize: 12
                        Material.roundedScale: Math.min(4, theme.radius)
                        onClicked: backend.sleepTimer.addMinutes(5)
                    }

                    NeoButton {
                        text: qsTr("Cancel Timer")
                        flat: true
                        font.pixelSize: 12
                        Material.roundedScale: Math.min(4, theme.radius)
                        onClicked: {
                            backend.sleepTimer.stop()
                            win.sleepTimerVisible = false
                        }
                    }
                }

                NeoButton {
                    text: qsTr("Close")
                    flat: true
                    font.pixelSize: 12
                    Material.roundedScale: Math.min(4, theme.radius)
                    Layout.alignment: Qt.AlignRight
                    onClicked: win.sleepTimerVisible = false
                }
            }
        }

        // Help overlay
        Rectangle {
            anchors.centerIn: parent
            width: 400
            height: helpCol.implicitHeight + 40
            color: neoPalette.surface
            radius: Math.min(8, theme.radius)
            border.color: neoPalette.ink
            border.width: 2
            visible: win.helpVisible
            z: 200

            ColumnLayout {
                id: helpCol
                anchors.fill: parent
                anchors.margins: 20
                spacing: 6

                Label {
                    text: qsTr("Keyboard Shortcuts")
                    font.pixelSize: 18
                    font.bold: true
                    color: neoPalette.ink
                }

                Item { height: 8 }

                Repeater {
                    model: [
                        { key: "Space", desc: qsTr("Play / Pause") },
                        { key: "←", desc: qsTr("Back 10 seconds") },
                        { key: "→", desc: qsTr("Forward 30 seconds") },
                        { key: "P", desc: qsTr("Previous chapter") },
                        { key: "N", desc: qsTr("Next chapter") },
                        { key: "[", desc: qsTr("Slow down") },
                        { key: "]", desc: qsTr("Speed up") },
                        { key: "B", desc: qsTr("Add bookmark") },
                        { key: "S", desc: qsTr("Sleep timer") },
                        { key: "C", desc: qsTr("Chapters") },
                        { key: "M", desc: qsTr("Bookmarks") },
                        { key: "T", desc: qsTr("Choose theme") },
                        { key: "O", desc: qsTr("Open folder") },
                        { key: "Esc", desc: qsTr("Back / Close") },
                        { key: "?", desc: qsTr("Toggle this help") },
                        { key: "Q", desc: qsTr("Quit") }
                    ]

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Label {
                            text: modelData.key
                            font.pixelSize: 13
                            font.bold: true
                            color: neoPalette.ink
                            Layout.preferredWidth: 60
                            horizontalAlignment: Text.AlignRight
                        }

                        Label {
                            text: modelData.desc
                            font.pixelSize: 13
                            color: neoPalette.ink
                        }
                    }
                }

                Item { height: 8 }

                NeoButton {
                    text: qsTr("Close")
                    flat: true
                    font.pixelSize: 12
                    Material.roundedScale: Math.min(4, theme.radius)
                    Layout.alignment: Qt.AlignRight
                    onClicked: win.helpVisible = false
                }
            }
        }

        // Dim overlay behind popups
        Rectangle {
            anchors.fill: parent
            color: "#80000000"
            visible: win.helpVisible || win.sleepTimerVisible || win.bookmarkVisible || win.themePickerVisible
            z: 50

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    win.helpVisible = false
                    win.sleepTimerVisible = false
                    win.bookmarkVisible = false
                    win.themePickerVisible = false
                }
            }
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 16
            height: errorLabel.implicitHeight + 20
            color: neoPalette.danger
            radius: Math.min(8, theme.radius)
            visible: win.errorVisible
            z: 300

            Label {
                id: errorLabel
                anchors.fill: parent
                anchors.margins: 10
                text: backend.error
                color: neoPalette.ink
                wrapMode: Text.Wrap
            }
        }
    }
}
