import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Controls.Material

Button {
    id: control

    Material.roundedScale: Math.min(4, theme.radius)
    Material.foreground: control.enabled && (control.highlighted || control.hovered || control.down)
                         ? theme.accentForeground : theme.foreground

    contentItem: IconLabel {
        spacing: control.spacing
        mirrored: control.mirrored
        display: control.display
        icon: control.icon
        defaultIconColor: control.Material.foreground
        text: control.text
        font: control.font
        color: control.Material.foreground
    }

    background: Item {
        Rectangle {
            x: control.down ? 0 : 4
            y: control.down ? 0 : 4
            width: parent.width - (control.down ? 0 : 4)
            height: parent.height - (control.down ? 0 : 4)
            color: control.Material.foreground
            radius: Math.min(4, theme.radius)
        }

        Rectangle {
            anchors.fill: parent
            anchors.rightMargin: control.down ? 0 : 4
            anchors.bottomMargin: control.down ? 0 : 4
            color: !control.enabled ? "#d9d2c2"
                  : control.down || control.hovered || control.highlighted
                    ? control.Material.accent : control.Material.background
            border.color: control.Material.foreground
            border.width: 2
            radius: Math.min(4, theme.radius)
        }
    }
}
