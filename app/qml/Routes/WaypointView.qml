import QtQuick 2.6
import QtQuick.Layouts 1.12
import Industrial.Controls 1.0 as Controls
import Dreka 1.0

import "../Common"

Item {
    id: root

    signal centerWaypoint(var routeId, int index)

    WaypointController {
        id: controller
        onInvokeMenu: menu.open(x, y)
    }

    Component.onCompleted: map.registerController("waypointController", controller)

    PointedMenu {
        id: menu
        title: qsTr("Edit Waypoint")
        onMenuVisibleChanged: {
            if (menuVisible)
                controller.setCurrentWaypointSelected(true);
            else if (editor.sourceComponent !== editComponent)
                controller.setCurrentWaypointSelected(false);
        }

        Controls.MenuItem {
            text: qsTr("Goto")
            enabled: false // TODO: wpt in mission
            onTriggered: console.log("Go to waypint") // TODO:
        }

        Controls.MenuItem {
            text: qsTr("Parameters")
            onTriggered: editor.open(menu.invokeX, menu.invokeY, editComponent)
        }

        Controls.MenuItem {
            text: qsTr("Remove")
            onTriggered: controller.remove()
        }
    }

    PointedPopup {
        id: editor
    }

    Component {
        id: editComponent

        WaypointEdit {
            waypoint: controller.waypoint
            waypointIndex: controller.waypointIndex
            Component.onCompleted: controller.setCurrentWaypointSelected(true);
            Component.onDestruction: controller.setCurrentWaypointSelected(false);
        }
    }
}
