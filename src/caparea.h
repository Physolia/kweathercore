/*
 * SPDX-FileCopyrightText: 2021 Han Young <hanyoung@protonmail.com>
 * SPDX-FileCopyrightText: 2021 Anjani Kumar <anjanik012@gmail.com>
 * SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KWEATHERCORE_CAPAREA_H
#define KWEATHERCORE_CAPAREA_H

#include <kweathercore/kweathercore_export.h>

#include <QMetaType>
#include <QSharedDataPointer>

namespace KWeatherCore
{

using AreaCodeVec = std::vector<std::pair<QString, QString>>;
using Polygon = std::vector<std::pair<float, float>>;

class CAPAreaPrivate;

/** Affected area of a CAP alert message.
 *  @see https://docs.oasis-open.org/emergency/cap/v1.2/CAP-v1.2.html §3.2.4
 */
class KWEATHERCORE_EXPORT CAPArea
{
    Q_GADGET
    Q_PROPERTY(QString description READ description)
public:
    CAPArea();
    CAPArea(const CAPArea &other);
    CAPArea(CAPArea &&other);
    ~CAPArea();
    CAPArea &operator=(const CAPArea &other);
    CAPArea &operator=(CAPArea &&other);

    /** A text description of the message target area. */
    QString description() const;
    void setDescription(const QString &areaDesc);

    /** Geographic polygon(s) enclosing the message target area. */
    const std::vector<Polygon> &polygons() const;
    void addPolygon(Polygon &&polygon);

    /** Any geographically-based code to describe a message target area, as key/value pair. */
    const AreaCodeVec &geoCodes() const;
    void addGeoCode(std::pair<QString, QString> &&geoCode);

    // TODO
    // circle, altitude, ceiling: so far not observed in real world data
private:
    QSharedDataPointer<CAPAreaPrivate> d;
};

}

Q_DECLARE_METATYPE(KWeatherCore::CAPArea)

#endif // KWEATHERCORE_CAPAREA_H
