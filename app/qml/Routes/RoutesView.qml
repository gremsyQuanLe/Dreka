import QtQuick 2.12
import QtQuick.Layouts 1.12
import Industrial.Controls 1.0 as Controls
import Dreka 1.0

RowLayout {
    id: root

    function setActiveMission(missionId) { controller.setActiveMission(missionId); }

    Controls.Menu {
        id: addWaypoint
        title: qsTr("Add Waypoint")
        enabled: controller.selectedRoute !== undefined

        Repeater {
            model: controller.waypointTypes(controller.selectedRoute)

            Controls.MenuItem {
                text: modelData.name
                onTriggered: {
                    var args = {};
                    args["latitude"] = mapMenu.latitude;
                    args["longitude"] = mapMenu.longitude;
                    args["altitude"] = mapMenu.altitude;

                    controller.addWaypoint(controller.selectedRoute, modelData.id, args);
                }
            }
        }
    }

    Connections {
        target: waypointEdit
        onCenterWaypoint: controller.centerWaypoint(routeId, index)
    }

    RoutesController { id: controller }

    Component.onCompleted: {
        map.registerController("routesController", controller);
        mapMenu.addSubmenu(addWaypoint);
    }

    spacing: 1

    Controls.Button {
        visible: controller.selectedRoute !== undefined
        rightCropped: true
        iconSource: "qrc:/icons/left.svg"
        tipText: qsTr("Back to routes")
        onClicked: controller.selectRoute(null)
    }

    Controls.Button {
        leftCropped: controller.selectedRoute !== undefined
        text: controller.selectedRoute ? controller.routeData(controller.selectedRoute).name : ""
        tipText: routeList.visible ? qsTr("Close routes viewer") : qsTr("Open routes viewer")
        iconSource: controller.selectedRoute ? "" : "qrc:/icons/route.svg"
        highlighted: routeList.visible
        onClicked: routeList.visible ? routeList.close() : routeList.open()
    }

    RouteList {
        id: routeList
        y: root.height + Controls.Theme.margins
        x: -root.parent.x
    }
}
