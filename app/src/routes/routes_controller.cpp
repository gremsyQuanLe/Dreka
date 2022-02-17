#include "routes_controller.h"

#include <QDebug>

#include "locator.h"
#include "route_traits.h"

using namespace md::domain;
using namespace md::presentation;

RoutesController::RoutesController(QObject* parent) :
    QObject(parent),
    m_routesService(md::app::Locator::get<IRoutesService>())
{
    Q_ASSERT(m_routesService);

    connect(m_routesService, &IRoutesService::routeTypesChanged, this,
            &RoutesController::routeTypesChanged);
    connect(m_routesService, &IRoutesService::routeAdded, this, &RoutesController::onRouteAdded);
    connect(m_routesService, &IRoutesService::routeRemoved, this, &RoutesController::onRouteRemoved);

    for (Route* route : m_routesService->routes())
    {
        this->onRouteAdded(route);
    }
}

QJsonArray RoutesController::routeTypes() const
{
    QJsonArray jsons;
    for (auto routeType : m_routesService->routeTypes())
    {
        jsons.append(QJsonObject::fromVariantMap(routeType->toVariantMap()));
    }
    return jsons;
}

QVariantList RoutesController::routeIds() const
{
    return m_routesService->routeIds();
}

QVariant RoutesController::selectedRoute() const
{
    return m_selectedRoute ? m_selectedRoute->id() : QVariant();
}

int RoutesController::selectedRouteItemIndex() const
{
    return m_selectedRouteItemIndex;
}

QJsonObject RoutesController::routeData(const QVariant& routeId) const
{
    Route* route = m_routesService->route(routeId);
    if (!route)
        return QJsonObject();

    QVariantMap routeData = route->toVariantMap();
    routeData[props::items] = route->count();
    routeData[props::type] = route->type()->name;

    return QJsonObject::fromVariantMap(routeData);
}

QJsonObject RoutesController::routeItemData(const QVariant& routeId, int index) const
{
    Route* route = m_routesService->route(routeId);
    if (!route)
        return QJsonObject();

    RouteItem* routeItem = route->item(index);
    if (!routeItem)
        return QJsonObject();

    return QJsonObject::fromVariantMap(routeItem->toVariantMap());
}

QJsonArray RoutesController::routeItemTypes(const QVariant& routeId) const
{
    Route* route = m_routesService->route(routeId);
    if (!route)
        return QJsonArray();

    QJsonArray jsons;
    for (auto wptType : route->type()->itemTypes)
    {
        jsons.append(QJsonObject::fromVariantMap(wptType->toVariantMap()));
    }
    return jsons;
}

QJsonArray RoutesController::routePatternTypes(const QVariant& routeId) const
{
    Route* route = m_routesService->route(routeId);
    if (!route)
        return QJsonArray();

    QJsonArray jsons;
    for (auto pattern : route->type()->patternTypes)
    {
        jsons.append(QJsonObject::fromVariantMap(pattern->toVariantMap()));
    }
    return jsons;
}

void RoutesController::selectRoute(const QVariant& selectedRouteId)
{
    auto selectedRoute = m_routesService->route(selectedRouteId);

    if (m_selectedRoute == selectedRoute)
        return;

    m_selectedRoute = selectedRoute;
    emit selectedRouteChanged(selectedRouteId);

    this->selectRouteItemIndex(-1);
}

void RoutesController::selectRouteItemIndex(int index)
{
    if (m_selectedRouteItemIndex == index)
        return;

    m_selectedRouteItemIndex = index;
    emit selectedRouteItemIndexChanged(index);
}

void RoutesController::addNewRoute(const QString& routeTypeId)
{
    auto routeType = m_routesService->routeType(routeTypeId);
    if (!routeType)
    {
        qWarning() << "Unknown route type";
        return;
    }

    QStringList routeNames;
    for (Route* route : m_routesService->routes())
    {
        routeNames += route->name;
    }

    auto route = new Route(routeType, utils::nameFromType(routeType->name, routeNames));
    m_routesService->saveRoute(route);
    this->selectRoute(route->id);
}

void RoutesController::updateRoute(const QVariant& routeId, const QJsonObject& data)
{
    Route* route = m_routesService->route(routeId);
    if (!route)
        return;

    route->fromVariantMap(data.toVariantMap());
    m_routesService->saveRoute(route);
}

void RoutesController::renameRoute(const QVariant& routeId, const QString& name)
{
    Route* route = m_routesService->route(routeId);
    if (!route)
        return;

    route->name = name;
    m_routesService->saveRoute(route);
}

void RoutesController::removeRoute(const QVariant& routeId)
{
    Route* route = m_routesService->route(routeId);
    if (!route)
        return;

    if (m_selectedRoute == route)
        this->selectRoute(QVariant());

    m_routesService->removeRoute(route);
}

void RoutesController::addRouteItem(const QVariant& routeId, const QString& typeId,
                                    const QVariantMap& position)
{
    Route* route = m_routesService->route(routeId);
    if (!route)
        return;

    const RouteItemType* type = route->type()->itemType(typeId);
    if (!type)
        return;

    // Special case for altitude
    QVariantMap parameters = type->defaultParameters();

    float altitude = route->count() ? route->item(route->count() - 1)->position().altitude()
                                    : position.value(geo::altitude, 0).toFloat();
    QVariantMap newPosition = position;
    newPosition[geo::altitude] = altitude;

    RouteItem* item = new RouteItem(type, type->shortName, utils::generateId(), parameters,
                                    newPosition);
    item->setParameters(parameters);
    route->addItem(item);

    m_routesService->saveItem(route, item);
}

void RoutesController::updateRouteItemData(const QVariant& routeId, int index,
                                           const QJsonObject& data)
{
    Route* route = m_routesService->route(routeId);
    if (!route)
        return;

    RouteItem* routeItem = route->item(index);
    if (!routeItem)
        return;

    routeItem->fromVariantMap(data.toVariantMap());
    m_routesService->saveItem(route, routeItem);

    // TODO: Promoute to the vehicle
}

void RoutesController::onRouteAdded(Route* route)
{
    emit routeAdded(route->id);
    emit routeIdsChanged();

    connect(route, &Route::itemAdded, this, [this, route](int index, RouteItem*) {
        emit routeItemAdded(route->id, index);
    });
    connect(route, &Route::itemRemoved, this, [this, route](int index, RouteItem*) {
        if (index < m_selectedRouteItemIndex)
        {
            m_selectedRouteItemIndex--;
            emit selectedRouteItemIndexChanged(m_selectedRouteItemIndex);
        }
        emit routeItemRemoved(route->id, index);
    });
    connect(route, &Route::itemChanged, this, [this, route](int index, RouteItem*) {
        emit routeItemChanged(route->id, index);
    });
    connect(route, &Route::changed, this, [route, this]() {
        emit routeChanged(route->id);
    });
}

void RoutesController::onRouteRemoved(Route* route)
{
    disconnect(route, nullptr, this, nullptr);

    emit routeRemoved(route->id);
    emit routeIdsChanged();
}
