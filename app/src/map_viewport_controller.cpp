#include "map_viewport_controller.h"

#include <QDebug>
#include <QSettings>

namespace viewport_settings
{
constexpr char camera[] = "viewport/camera";
constexpr char heading[] = "viewport/heading";
constexpr char pitch[] = "viewport/pitch";

const md::domain::Geodetic deafultCamera(55.97101, 37.10610, 400.0);
constexpr float defaultHeading = 0.0;
constexpr float defaultPitch = -15.0;
} // namespace viewport_settings

using namespace md::presentation;

MapViewportController::MapViewportController(QObject* parent) : QObject(parent)
{
}

QJsonObject MapViewportController::cursorPosition() const
{
    return m_cursorPosition.toJson();
}

QJsonObject MapViewportController::centerPosition() const
{
    return m_centerPosition.toJson();
}

QJsonObject MapViewportController::cameraPosition() const
{
    return m_cameraPosition.toJson();
}

float MapViewportController::heading() const
{
    return m_heading;
}

float MapViewportController::pitch() const
{
    return m_pitch;
}

double MapViewportController::pixelScale() const
{
    return m_pixelScale;
}

void MapViewportController::save()
{
    if (!m_cameraPosition.isValid())
        return;

    QSettings settings;

    settings.setValue(::viewport_settings::camera, m_cameraPosition.toJson().toVariantMap());
    settings.setValue(::viewport_settings::heading, m_heading);
    settings.setValue(::viewport_settings::pitch, m_pitch);
}

void MapViewportController::restore()
{
    QSettings settings;

    QJsonObject camera = QJsonObject::fromVariantMap(
        settings.value(::viewport_settings::camera).toMap());
    if (camera.isEmpty())
        camera = ::viewport_settings::deafultCamera.toJson();

    float heading =
        settings.value(::viewport_settings::heading, ::viewport_settings::defaultHeading).toReal();
    float pitch = settings.value(::viewport_settings::pitch, ::viewport_settings::defaultPitch)
                      .toReal();

    emit flyTo(camera, heading, pitch);
}

void MapViewportController::setCursorPosition(const QJsonObject& cursorPosition)
{
    m_cursorPosition = md::domain::Geodetic(cursorPosition);
    emit cursorPositionChanged();
}

void MapViewportController::setCenterPosition(const QJsonObject& centerPosition)
{
    m_centerPosition = md::domain::Geodetic(centerPosition);
    emit centerPositionChanged();
}

void MapViewportController::setCameraPosition(const QJsonObject& cameraPosition)
{
    m_cameraPosition = md::domain::Geodetic(cameraPosition);
    emit cameraPositionChanged();
}

void MapViewportController::setHeading(float heading)
{
    if (qFuzzyCompare(m_heading, heading))
        return;

    m_heading = heading;
    emit headingChanged();
}

void MapViewportController::setPitch(float pitch)
{
    if (qFuzzyCompare(m_pitch, pitch))
        return;

    m_pitch = pitch;
    emit pitchChanged();
}

void MapViewportController::setPixelScale(double pixelScale)
{
    if (qFuzzyCompare(m_pixelScale, pixelScale))
        return;

    m_pixelScale = pixelScale;
    emit pixelScaleChanged();
}
