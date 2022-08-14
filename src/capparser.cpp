/*
 * SPDX-FileCopyrightText: 2021 Anjani Kumar <anjanik012@gmail.com>
 * SPDX-FileCopyrightText: 2021 Han Young <hanyoung@protonmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "capparser.h"
#include "alertentry.h"
#include "alertinfo.h"
#include "caparea.h"
#include "kweathercore_p.h"
#include <KLocalizedString>
#include <QDateTime>
#include <QDebug>

#include <optional>

namespace KWeatherCore
{

template<typename T>
struct MapEntry {
    const char *name;
    T value;
};

template<typename QStringT, typename EnumT, std::size_t N>
static std::optional<EnumT> stringToValue(const QStringT &s, const MapEntry<EnumT> (&map)[N])
{
    const auto it = std::lower_bound(std::begin(map), std::end(map), s, [](auto lhs, auto rhs) {
        return QLatin1String(lhs.name) < rhs;
    });
    if (it != std::end(map) && QLatin1String((*it).name) == s) {
        return (*it).value;
    }
    return {};
}

// ### important: keep all the following tables sorted by name!
static constexpr const MapEntry<AlertInfo::Category> category_map[] = {
    {"CBRNE", AlertInfo::Category::CBRNE},
    {"Env", AlertInfo::Category::Env},
    {"Fire", AlertInfo::Category::Fire},
    {"Geo", AlertInfo::Category::Geo},
    {"Health", AlertInfo::Category::Health},
    {"Infra", AlertInfo::Category::Infra},
    {"Met", AlertInfo::Category::Met},
    {"Other", AlertInfo::Category::Other},
    {"Rescue", AlertInfo::Category::Rescue},
    {"Safety", AlertInfo::Category::Safety},
    {"Security", AlertInfo::Category::Security},
    {"Transport", AlertInfo::Category::Transport},
};

enum class Tags { ALERT, IDENTIFIER, SENDER, SENT_TIME, STATUS, MSG_TYPE, SCOPE, NOTE, INFO };

static constexpr const MapEntry<Tags> tag_map[] = {
    {"alert", Tags::ALERT},
    {"identifier", Tags::IDENTIFIER},
    {"info", Tags::INFO},
    {"msgType", Tags::MSG_TYPE},
    {"note", Tags::NOTE},
    {"scope", Tags::SCOPE},
    {"sender", Tags::SENDER},
    {"sent", Tags::SENT_TIME},
    {"status", Tags::STATUS},
};

enum class InfoTags {
    HEADLINE,
    DESCRIPTION,
    EVENT,
    EFFECTIVE_TIME,
    ONSET_TIME,
    EXPIRE_TIME,
    CATEGORY,
    INSTRUCTION,
    URGENCY,
    SEVERITY,
    CERTAINITY,
    PARAMETER,
    AREA,
    SENDERNAME,
    LANGUAGE,
    RESPONSETYPE,
    CONTACT,
    WEB,
};

static constexpr const MapEntry<InfoTags> info_tag_map[] = {
    {"area", InfoTags::AREA},
    {"category", InfoTags::CATEGORY},
    {"certainty", InfoTags::CERTAINITY},
    {"contact", InfoTags::CONTACT},
    {"description", InfoTags::DESCRIPTION},
    {"effective", InfoTags::EFFECTIVE_TIME},
    {"event", InfoTags::EVENT},
    {"expires", InfoTags::EXPIRE_TIME},
    {"headline", InfoTags::HEADLINE},
    {"instruction", InfoTags::INSTRUCTION},
    {"language", InfoTags::LANGUAGE},
    {"onset", InfoTags::ONSET_TIME},
    {"parameter", InfoTags::PARAMETER},
    {"responseType", InfoTags::RESPONSETYPE},
    {"senderName", InfoTags::SENDERNAME},
    {"severity", InfoTags::SEVERITY},
    {"urgency", InfoTags::URGENCY},
    {"web", InfoTags::WEB},
};

static constexpr const MapEntry<AlertEntry::Status> status_map[] = {
    {"Actual", AlertEntry::Status::Actual},
    {"Draft", AlertEntry::Status::Draft},
    {"Excercise", AlertEntry::Status::Exercise},
    {"System", AlertEntry::Status::System},
    {"Test", AlertEntry::Status::Test},
};

static constexpr const MapEntry<AlertEntry::MsgType> msgtype_map[] = {
    {"Ack", AlertEntry::MsgType::Ack},
    {"Alert", AlertEntry::MsgType::Alert},
    {"Cancel", AlertEntry::MsgType::Cancel},
    {"Error", AlertEntry::MsgType::Error},
    {"Update", AlertEntry::MsgType::Update},
};

static constexpr const MapEntry<AlertEntry::Scope> scope_map[] = {
    {"Private", AlertEntry::Scope::Private},
    {"Public", AlertEntry::Scope::Public},
    {"Restricted", AlertEntry::Scope::Restricted},
};

static constexpr const MapEntry<AlertInfo::ResponseType> response_type_map[] = {
    {"AllClear", AlertInfo::ResponseType::AllClear},
    {"Assess", AlertInfo::ResponseType::Assess},
    {"Avoid", AlertInfo::ResponseType::Avoid},
    {"Evacuate", AlertInfo::ResponseType::Evacuate},
    {"Execute", AlertInfo::ResponseType::Execute},
    {"Monitor", AlertInfo::ResponseType::Monitor},
    {"None", AlertInfo::ResponseType::None},
    {"Prepare", AlertInfo::ResponseType::Prepare},
    {"Shelter", AlertInfo::ResponseType::Shelter},
};

CAPParser::CAPParser(const QByteArray &data)
    : m_xml(data)
{
    bool flag = false;
    if (!data.isEmpty()) {
        while (m_xml.readNextStartElement()) {
            if (m_xml.name() == QStringLiteral("alert")) {
                flag = true;
                break;
            }
        }
        if (!flag) {
            qWarning() << "Not a CAP XML";
        }
    }
}

AlertEntry CAPParser::parse()
{
    AlertEntry entry;
    while (m_xml.readNextStartElement()) {
        const auto tag = stringToValue(m_xml.name(), tag_map);
        if (!tag) {
            m_xml.skipCurrentElement();
            continue;
        }
        switch (*tag) {
        case Tags::IDENTIFIER:
            entry.setIdentifier(m_xml.readElementText());
            break;
        case Tags::SENDER:
            entry.setSender(m_xml.readElementText());
            break;
        case Tags::SENT_TIME:
            entry.setSentTime(QDateTime::fromString(m_xml.readElementText(), Qt::ISODate));
            break;
        case Tags::STATUS: {
            const auto elementText = m_xml.readElementText();
            const auto status = stringToValue(elementText, status_map);
            if (status) {
                entry.setStatus(*status);
            } else {
                qWarning() << "Unknown status field" << elementText;
            }
            break;
        }
        case Tags::MSG_TYPE: {
            const auto elementText = m_xml.readElementText();
            const auto msgType = stringToValue(elementText, msgtype_map);
            if (msgType) {
                entry.setMsgType(*msgType);
            } else {
                qWarning() << "Unknown msgType field" << elementText;
            }
            break;
        }
        case Tags::SCOPE: {
            const auto elementText = m_xml.readElementText();
            const auto scope = stringToValue(elementText, scope_map);
            if (scope) {
                entry.setScope(*scope);
            } else {
                qWarning() << "Unknown scope field" << elementText;
            }
            break;
        }
        case Tags::NOTE:
            entry.setNote(m_xml.readElementText());
            break;
        case Tags::INFO: {
            auto info = parseInfo();
            entry.addInfo(info);
            break;
        }
        default:
            m_xml.skipCurrentElement();
        }
    }
    return entry;
}

AlertInfo CAPParser::parseInfo()
{
    AlertInfo info;

    if (m_xml.name() == QLatin1String("info")) {
        while (!m_xml.atEnd() && !(m_xml.isEndElement() && m_xml.name() == QLatin1String("info"))) {
            m_xml.readNext();
            if (!m_xml.isStartElement()) {
                continue;
            }
            const auto tag = stringToValue(m_xml.name(), info_tag_map);
            if (tag) {
                switch (*tag) {
                case InfoTags::CATEGORY: {
                    const auto s = m_xml.readElementText();
                    const auto category = stringToValue(s, category_map);
                    if (category) {
                        info.addCategory(*category);
                    }
                    break;
                }
                case InfoTags::EVENT:
                    info.setEvent(m_xml.readElementText());
                    break;
                case InfoTags::URGENCY:
                    info.setUrgency(KWeatherCorePrivate::urgencyStringToEnum(m_xml.readElementText()));
                    break;
                case InfoTags::SEVERITY:
                    info.setSeverity(KWeatherCorePrivate::severityStringToEnum(m_xml.readElementText()));
                    break;
                case InfoTags::CERTAINITY:
                    info.setCertainty(KWeatherCorePrivate::certaintyStringToEnum(m_xml.readElementText()));
                    break;
                case InfoTags::EFFECTIVE_TIME:
                    info.setEffectiveTime(QDateTime::fromString(m_xml.readElementText(), Qt::ISODate));
                    break;
                case InfoTags::ONSET_TIME:
                    info.setOnsetTime(QDateTime::fromString(m_xml.readElementText(), Qt::ISODate));
                    break;
                case InfoTags::EXPIRE_TIME:
                    info.setExpireTime(QDateTime::fromString(m_xml.readElementText(), Qt::ISODate));
                    break;
                case InfoTags::HEADLINE:
                    info.setHeadline(m_xml.readElementText());
                    break;
                case InfoTags::DESCRIPTION:
                    info.setDescription(m_xml.readElementText());
                    break;
                case InfoTags::INSTRUCTION:
                    info.setInstruction(m_xml.readElementText());
                    break;
                case InfoTags::PARAMETER: {
                    std::pair<QString, QString> p;
                    m_xml.readNextStartElement();
                    if (m_xml.name() == QStringLiteral("valueName")) {
                        p.first = m_xml.readElementText();
                    }
                    m_xml.readNextStartElement();
                    if (m_xml.name() == QStringLiteral("value")) {
                        p.second = m_xml.readElementText();
                    }
                    info.addParameter(p);
                    break;
                }
                case InfoTags::AREA: {
                    info.addArea(parseArea());
                    break;
                }
                case InfoTags::SENDERNAME: {
                    info.setSender(m_xml.readElementText());
                    break;
                }
                case InfoTags::LANGUAGE:
                    info.setLanguage(m_xml.readElementText());
                    break;
                case InfoTags::RESPONSETYPE: {
                    const auto elementText = m_xml.readElementText();
                    if (const auto respType = stringToValue(elementText, response_type_map)) {
                        info.addResponseType(*respType);
                    } else {
                        qWarning() << "Unknown respone type value" << elementText;
                    }
                    break;
                }
                case InfoTags::CONTACT:
                    info.setContact(m_xml.readElementText());
                    break;
                case InfoTags::WEB:
                    info.setWeb(m_xml.readElementText());
                    break;
                }
            } else {
                if (m_xml.isStartElement()) {
                    qWarning() << "unknown element: " << m_xml.name();
                }
            }
        }
    }
    return info;
}

CAPArea CAPParser::parseArea()
{
    CAPArea area;
    while (!(m_xml.isEndElement() && m_xml.name() == QStringLiteral("area"))) {
        m_xml.readNext();
        if (m_xml.name() == QStringLiteral("areaDesc") && !m_xml.isEndElement()) {
            area.setDescription(m_xml.readElementText());
        } else if (m_xml.name() == QStringLiteral("geocode") && !m_xml.isEndElement()) {
            std::pair<QString, QString> p;
            m_xml.readNextStartElement();
            if (m_xml.name() == QStringLiteral("valueName")) {
                p.first = m_xml.readElementText();
            }
            m_xml.readNextStartElement();
            if (m_xml.name() == QStringLiteral("value")) {
                p.second = m_xml.readElementText();
            }
            area.addGeoCode(std::move(p));
        } else if (m_xml.name() == QStringLiteral("polygon") && !m_xml.isEndElement()) {
            area.addPolygon(KWeatherCorePrivate::stringToPolygon(m_xml.readElementText()));
        } else if (m_xml.name() == QLatin1String("circle") && !m_xml.isEndElement()) {
            const auto t = m_xml.readElementText();
            const auto commaIdx = t.indexOf(QLatin1Char(','));
            const auto spaceIdx = t.indexOf(QLatin1Char(' '));
            if (commaIdx > 0 && spaceIdx > commaIdx && commaIdx < t.size()) {
                CAPCircle circle;
                circle.latitude = QStringView(t).left(commaIdx).toFloat();
                circle.longitude = QStringView(t).mid(commaIdx + 1, spaceIdx - commaIdx - 1).toFloat();
                circle.radius = QStringView(t).mid(spaceIdx).toFloat();
                area.addCircle(std::move(circle));
            }
        } else if (m_xml.isStartElement()) {
            qDebug() << "unknown area element:" << m_xml.name();
        }
    }
    return area;
}
}
