/*
 * Copyright 2012 Research In Motion Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <json/value.h>
#include <bb/pim/calendar/CalendarFolder>
#include <bb/pim/calendar/Notification>
#include <bb/pim/calendar/Recurrence>
#include <bb/pim/calendar/Frequency>
#include <bb/pim/calendar/Attendee>
#include <bb/pim/calendar/AttendeeRole>
#include <bb/pim/calendar/EventSearchParameters>
#include <bb/pim/calendar/SortField>
#include <bb/pim/calendar/FolderKey>
#include <bb/pim/calendar/Sensitivity>
#include <bb/pim/calendar/BusyStatus>
#include <bb/pim/account/AccountService>
#include <bb/pim/account/Account>
#include <bb/pim/account/Service>

#include <stdio.h>
#include <QFile>
#include <QTextStream>
#include <QVariantMap>
#include <QVariant>
#include <QStringList>
#include <QPair>
#include <QSet>
#include <QMap>
#include <QtAlgorithms>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <qDebug>

#include "timezone_qt.hpp"
#include "timezone_utils.hpp"

namespace webworks {
/*
StringToKindMap PimContactsQt::_attributeKindMap;
StringToSubKindMap PimContactsQt::_attributeSubKindMap;
KindToStringMap PimContactsQt::_kindAttributeMap;
SubKindToStringMap PimContactsQt::_subKindAttributeMap;
QList<bbpim::SortSpecifier> PimContactsQt::_sortSpecs;
std::map<bbpim::ContactId, bbpim::Contact> PimContactsQt::_contactSearchMap;
*/
PimCalendarQt::PimCalendarQt()
{
/*
    static bool mapInit = false;

    if (!mapInit) {
        createAttributeKindMap();
        createAttributeSubKindMap();
        createKindAttributeMap();
        createSubKindAttributeMap();
        mapInit = true;
    }
*/
}

PimCalendarQt::~PimCalendarQt()
{
}

/****************************************************************
 * Public Functions
 ****************************************************************/

Json::Value PimCalendarQt::Find(const Json::Value& args)
{
    Json::Value returnObj;
    bbpim::EventSearchParameters searchParams;
    bool isParamsValid = getSearchParams(searchParams, args);

    // TODO(rtse): must escape double quotes and other problematic characters otherwise there might be problems with JSON parsing

    if (isParamsValid) {
        _allFoldersMap.clear();
        _foldersMap.clear();

        bbpim::CalendarService service;
        QList<bbpim::CalendarEvent> events = service.events(searchParams);

        Json::Value searchResults;
        Json::Value folders;

        for (QList<bbpim::CalendarEvent>::const_iterator i = events.constBegin(); i != events.constEnd(); i++) {
            bbpim::CalendarEvent event = *i;
            lookupCalendarFolderByFolderKey(event.accountId(), event.folderId());
            searchResults.append(populateEvent(event, true));
        }

        for (std::map<std::string, bbpim::CalendarFolder>::const_iterator j = _foldersMap.begin(); j != _foldersMap.end(); j++) {
            std::string key = j->first;
            bbpim::CalendarFolder folder = j->second;
            folders[key] = getCalendarFolderJson(folder);
        }

        returnObj["_success"] = true;
        returnObj["events"] = searchResults;
        returnObj["folders"] = folders;
    } else {
        returnObj["_success"] = false;
        returnObj["code"] = INVALID_ARGUMENT_ERROR;
    }

    return returnObj;
}

Json::Value PimCalendarQt::Save(const Json::Value& attributeObj)
{
    bbpim::CalendarService service;

    if (!attributeObj.isMember("id") || attributeObj["id"].isNull() || !attributeObj["id"].isInt()) {
        return CreateCalendarEvent(attributeObj);
    } else {
        int eventId = attributeObj["id"].asInt();
        int accountId = attributeObj["accountId"].asInt();
        bbpim::CalendarEvent event = service.event(accountId, eventId);

        if (event.isValid()) {
            return EditCalendarEvent(event, attributeObj);
        }
    }

    Json::Value returnObj;
    returnObj["_success"] = false;
    returnObj["code"] = INVALID_ARGUMENT_ERROR;
    return returnObj;
}

Json::Value PimCalendarQt::GetCalendarFolders()
{
    bbpim::CalendarService service;
    QList<bbpim::CalendarFolder> folders = service.folders();
    Json::Value folderList;

    for (QList<bbpim::CalendarFolder>::const_iterator i = folders.constBegin(); i != folders.constEnd(); i++) {
        folderList.append(getCalendarFolderJson(*i));
    }

    return folderList;
}

Json::Value PimCalendarQt::GetDefaultCalendarFolder()
{
    bb::pim::account::AccountService accountService;
    bb::pim::account::Account defaultCalAccnt = accountService.defaultAccount(bb::pim::account::Service::Calendars);
    bbpim::AccountId accountId = defaultCalAccnt.id();
    bbpim::FolderId folderId = intToFolderId(accountService.getDefault(bb::pim::account::Service::Calendars));

    bbpim::CalendarService service;
    QList<bbpim::CalendarFolder> folders = service.folders();
    bbpim::CalendarFolder defaultFolder;

    for (QList<bbpim::CalendarFolder>::const_iterator i = folders.constBegin(); i != folders.constEnd(); i++) {
        if (i->id() == folderId && i->accountId() == accountId) {
            return getCalendarFolderJson(*i, true);
        }
    }

    return Json::Value();
}

Json::Value PimCalendarQt::GetTimezones()
{
    Json::Value results;

    QFile file("/usr/share/zoneinfo/tzvalid");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "unable to load list";
    } else {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("\"")) {
                line.remove("\"");
                results.append(line.toStdString());
            }
        }
    }

    return results;
}

Json::Value PimCalendarQt::CreateCalendarEvent(const Json::Value& args)
{
    Json::Value returnObj;

    bbpim::CalendarService service;
    bbpim::CalendarEvent ev;

    fprintf(stderr, "Begin create!%s\n", "");


    if (args.isMember("accountId") && args["accountId"].isInt() &&
        args.isMember("folderId") && args["folderId"].isInt()) {
        ev.setAccountId(args["accountId"].asInt());
        ev.setFolderId(args["folderId"].asInt());
    } else {
        // no account id and folder id specified from JS, makes event goes to default calendar
        bb::pim::account::AccountService accountService;
        bb::pim::account::Account defaultCalAccnt = accountService.defaultAccount(bb::pim::account::Service::Calendars);
        ev.setAccountId(defaultCalAccnt.id());
        ev.setFolderId(intToFolderId(accountService.getDefault(bb::pim::account::Service::Calendars)));
    }

    QString timezone = "";

    if (args.isMember("timezone") && args["timezone"].isString()) {
        timezone = QString(args["timezone"].asCString());
        ev.setTimezone(timezone);
    }

    // TODO(rtse): timezone

    QDateTime startTime = QDateTime::fromString(args["start"].asCString(), "yyyy-MM-dd'T'hh:mm:ss'.000Z'");
    QDateTime endTime = QDateTime::fromString(args["end"].asCString(),  "yyyy-MM-dd'T'hh:mm:ss'.000Z'");

    fprintf(stderr, "After start end!%s\n", "");

    ev.setStartTime(TimezoneUtils::convertToTargetFromUtc(startTime, false, timezone));
    ev.setEndTime(TimezoneUtils::convertToTargetFromUtc(endTime, false, timezone));

    ev.setAllDay(args["allDay"].asBool());
    ev.setSubject(args["summary"].asCString());
    ev.setLocation(args["location"].asCString());

    fprintf(stderr, "After location!%s\n", "");

    if (args.isMember("transparency") && args["transparency"].isInt()) {
        ev.setBusyStatus(bbpim::BusyStatus::Type(args.get("transparency", bbpim::BusyStatus::Busy).asInt())); // use busy as default, same as calendar app
    }
    fprintf(stderr, "After transparency!%s\n", "");
    QList<QDateTime> exceptionDates;

    if (args.isMember("recurrence") && !args["recurrence"].isNull()) {
        Json::Value recurrence_json = args["recurrence"];

        if (recurrence_json["frequency"].isNull()) {
            returnObj["_success"] = false;
            returnObj["code"] = INVALID_ARGUMENT_ERROR;
            return returnObj;
        }

        bbpim::Recurrence recurrence;
        recurrence.setFrequency(bbpim::Frequency::Type(recurrence_json["frequency"].asInt()));
        recurrence.setInterval(recurrence_json.get("interval", 1).asInt());

        if (recurrence_json.isMember("expires") && !recurrence_json["expires"].isNull()) {
            recurrence.setUntil(TimezoneUtils::convertToTargetFromUtc(QDateTime::fromString(recurrence_json["expires"].asCString(), "yyyy-MM-dd'T'hh:mm:ss'.000Z'"), false, timezone));
        }

        if (recurrence_json.isMember("numberOfOccurrences") && !recurrence_json["numberOfOccurrences"].isNull()) {
            recurrence.setNumberOfOccurrences(recurrence_json["numberOfOccurrences"].asInt());
        }

        recurrence.setDayInWeek(recurrence_json.get("dayInWeek", 1 << (startTime.date().dayOfWeek()%7)).asInt());
        recurrence.setWeekInMonth(recurrence_json.get("weekInMonth", (startTime.date().day()/7) + 1).asInt());
        recurrence.setDayInMonth(recurrence_json.get("dayInMonth", startTime.date().day()).asInt());
        recurrence.setMonthInYear(recurrence_json.get("monthInYear", startTime.date().month()).asInt());

        // Note: exceptionDates cannot be added manually. They must be added using CalendarService::createRecurrenceExclusion
        for (unsigned int i = 0; i < recurrence_json["exceptionDates"].size(); i++) {
            //recurrence.addException(QDateTime::fromString(recurrence_json["exceptionDates"][i].asCString(), "yyyy-MM-dd'T'hh:mm:ss'.000Z'"));
            exceptionDates.append(TimezoneUtils::convertToTargetFromUtc(QDateTime::fromString(recurrence_json["exceptionDates"][i].asCString(), "yyyy-MM-dd'T'hh:mm:ss'.000Z'"), true, timezone));
        }

        if (!recurrence.isValid()) {
            returnObj["_success"] = false;
            returnObj["code"] = UNKNOWN_ERROR;
            return returnObj;
        }

        ev.setRecurrence(recurrence);
    }
    fprintf(stderr, "After recurrence!%s\n", "");
    if (args.isMember("attendees")) {
        for (unsigned int i = 0; i < args["attendees"].size(); i++) {
            bbpim::Attendee attendee;
            Json::Value attendee_json = args["attendees"][i];

            attendee.setName(QString(attendee_json.get("name", "").asCString()));
            attendee.setEmail(QString(attendee_json.get("email", "").asCString()));
            attendee.setType((bbpim::Attendee::Type)(attendee_json.get("type", bbpim::Attendee::Host).asInt()));
            attendee.setRole((bbpim::AttendeeRole::Type)(attendee_json.get("role", bbpim::AttendeeRole::Chair).asInt()));
            attendee.setStatus((bbpim::AttendeeStatus::Type)(attendee_json.get("status", bbpim::AttendeeStatus::Unknown).asInt()));
            attendee.setContactId(attendee_json.get("contactId", 0).asInt());
            attendee.setOwner(attendee_json.get("owner", false).asBool());

            if (!attendee.isValid()) {
                returnObj["_success"] = false;
                returnObj["code"] = UNKNOWN_ERROR;
                return returnObj;
            }

            ev.addAttendee(attendee);
        }
    }
    fprintf(stderr, "After attendee!%s\n", "");
    bbpim::Notification notification;
    notification.setComments(QString("This is a test event created by the WebWorks PIM Calendar API."));
    notification.setNotifyAll(true);
    notification.setAccountId(ev.accountId());
    notification.setMessageAccountId(ev.accountId());

    if (args.isMember("parentId") && !args["parentId"].isNull() && args["parentId"].asInt() != 0) {
        // This is a recurrence exception event
        if (!args.isMember("originalStartTime") || args["originalStartTime"].isNull()) {
            returnObj["_success"] = false;
            returnObj["code"] = INVALID_ARGUMENT_ERROR;
            return returnObj;
        }

        ev.setId(args["parentId"].asInt());

        ev.setStartTime(TimezoneUtils::convertToTargetFromUtc(startTime, true, timezone));
        ev.setEndTime(TimezoneUtils::convertToTargetFromUtc(endTime, true, timezone));

        fprintf(stderr, "Original start time to follow%s\n", "");
        service.createRecurrenceException(ev,TimezoneUtils::convertToTargetFromUtc(QDateTime::fromString(args["originalStartTime"].asCString(), "yyyy-MM-dd'T'hh:mm:ss'.000Z'"), true, timezone)/*, notification*/);
    } else {
        //service.createEvent(ev, notification);
            fprintf(stderr, "before create!%s\n", "");
        service.createEvent(ev);
    }

                fprintf(stderr, "after create!%s\n", "");

    for (int i = 0; i < exceptionDates.size(); i++) {
        bbpim::CalendarEvent exceptionEvent;
        fprintf(stderr, "Exception date[%d]=%s\n", i, exceptionDates[i].toString().toStdString().c_str());
        exceptionEvent.setStartTime(exceptionDates[i]);
        exceptionEvent.setId(ev.id());
        exceptionEvent.setAccountId(ev.accountId());
        service.createRecurrenceExclusion(exceptionEvent/*, notification*/);
    }

    if (ev.isValid()) {
        returnObj["event"] = populateEvent(ev, false);
        returnObj["_success"] = true;
        returnObj["id"] = Json::Value(ev.id());
    } else {
        returnObj["_success"] = false;
        returnObj["code"] = UNKNOWN_ERROR;
    }

    return returnObj;
}

Json::Value PimCalendarQt::DeleteCalendarEvent(const Json::Value& calEventObj)
{
    Json::Value returnObj;

    if (calEventObj.isMember("calEventId") && calEventObj["calEventId"].isInt() && calEventObj.isMember("accountId") && calEventObj["accountId"].isInt()) {
        int accountId = calEventObj["accountId"].asInt();
        int eventId = calEventObj["calEventId"].asInt();

        bbpim::CalendarService service;
        bbpim::CalendarEvent event = service.event(accountId, eventId);

        if (event.isValid()) {
            bbpim::Notification notification;
            notification.setComments(QString("This event was deleted by the WebWorks PIM Calendar API."));
            notification.setNotifyAll(true);
            notification.setAccountId(event.accountId());
            notification.setMessageAccountId(event.accountId());

            service.deleteEvent(event, notification);
            returnObj["_success"] = true;
            return returnObj;
        } else {
            returnObj["_success"] = false;
            returnObj["code"] = UNKNOWN_ERROR;
            return returnObj;
        }
    } else {
        returnObj["_success"] = false;
        returnObj["code"] = INVALID_ARGUMENT_ERROR;
        return returnObj;
    }
}

Json::Value PimCalendarQt::EditCalendarEvent(bbpim::CalendarEvent& calEvent, const Json::Value& attributeObj)
{
    const Json::Value::Members attributeKeys = attributeObj.getMemberNames();
    Json::Value eventFields;

    Json::Value returnObj;
    QList<QDateTime> exceptionDates;

    // TODO Create some stringToEnumMap to use switch or private function?

    for (unsigned int i = 0; i < attributeKeys.size(); i++) {
        const std::string key = attributeKeys[i];

        if (key == "allDay") {
            calEvent.setAllDay(attributeObj[key].asBool());
        }

        if (key == "birthday") {
            calEvent.setBirthday(attributeObj[key].asBool());
        }

        if (key == "start") {
            QDateTime date = QDateTime::fromString(QString(attributeObj[key].asString().c_str()), QString("yyyy-MM-dd'T'hh:mm:ss'.000Z'"));
            calEvent.setStartTime(TimezoneUtils::convertToTargetFromUtc(date, false)); // TODO may need special adjustment to see if timezone is changed...
        }

        if (key == "end") {
            QDateTime date = QDateTime::fromString(QString(attributeObj[key].asString().c_str()), QString("yyyy-MM-dd'T'hh:mm:ss'.000Z'"));
            calEvent.setEndTime(TimezoneUtils::convertToTargetFromUtc(date, false));
        }

        if (key == "description") {
            calEvent.setBody(QString::fromStdString(attributeObj[key].asString()));
        }

        if (key == "summary") {
            calEvent.setSubject(QString::fromStdString(attributeObj[key].asString()));
        }

        if (key == "location") {
            calEvent.setLocation(QString::fromStdString(attributeObj[key].asString()));
        }

        if (key == "timezone") {
            calEvent.setTimezone(QString::fromStdString(attributeObj[key].asString()));
        }

        if (key == "reminder") {
            calEvent.setReminder(attributeObj[key].asInt());
        }

        if (key == "status") {
            calEvent.setMeetingStatus(attributeObj[key].asInt());
        }

        if (key == "timezone") {
            calEvent.setTimezone(QString::fromStdString(attributeObj[key].asString()));
        }

        if (key == "sensitivity") {
            switch (attributeObj[key].asInt()){
                case bbpim::Sensitivity::Normal:
                    calEvent.setSensitivity(bbpim::Sensitivity::Normal);
                    break;
                case bbpim::Sensitivity::Personal:
                    calEvent.setSensitivity(bbpim::Sensitivity::Personal);
                    break;
                case bbpim::Sensitivity::Private:
                    calEvent.setSensitivity(bbpim::Sensitivity::Private);
                    break;
                case bbpim::Sensitivity::Confidential:
                    calEvent.setSensitivity(bbpim::Sensitivity::Confidential);
                    break;
            }
        }

        if (key == "transparency") {
            switch (attributeObj[key].asInt()){
                case bbpim::BusyStatus::Free:
                    calEvent.setBusyStatus(bbpim::BusyStatus::Free);
                    break;
                case bbpim::BusyStatus::Tentative:
                    calEvent.setBusyStatus(bbpim::BusyStatus::Tentative);
                    break;
                case bbpim::BusyStatus::Busy:
                    calEvent.setBusyStatus(bbpim::BusyStatus::Busy);
                    break;
                case bbpim::BusyStatus::OutOfOffice:
                    calEvent.setBusyStatus(bbpim::BusyStatus::OutOfOffice);
                    break;
            }
        }

        if (key == "attendees") {
            QList<bbpim::Attendee> attendees;

            for (unsigned int i = 0; i < attributeObj[key].size(); i++) {
                bbpim::Attendee attendee;
                Json::Value attendee_json = attributeObj[key][i];

                attendee.setName(QString(attendee_json.get("name", "").asCString()));
                attendee.setEmail(QString(attendee_json.get("email", "").asCString()));
                attendee.setType((bbpim::Attendee::Type)(attendee_json.get("type", bbpim::Attendee::Host).asInt()));
                attendee.setRole((bbpim::AttendeeRole::Type)(attendee_json.get("role", bbpim::AttendeeRole::Chair).asInt()));
                attendee.setStatus((bbpim::AttendeeStatus::Type)(attendee_json.get("status", bbpim::AttendeeStatus::Unknown).asInt()));
                attendee.setContactId(attendee_json.get("contactId", 0).asInt());
                attendee.setOwner(attendee_json.get("owner", false).asBool());

                if (!attendee.isValid()) {
                    returnObj["_success"] = false;
                    returnObj["code"] = UNKNOWN_ERROR;
                    return returnObj;
                }

                attendees.append(attendee);
            }

            calEvent.setAttendees(attendees);
        }

        if (key == "recurrence") {
            Json::Value recurrence_json = attributeObj[key];

            if (recurrence_json["frequency"].isNull()) {
                returnObj["_success"] = false;
                returnObj["code"] = INVALID_ARGUMENT_ERROR;
                return returnObj;
            }

            bbpim::Recurrence recurrence = calEvent.recurrence();
            recurrence.setFrequency(bbpim::Frequency::Type(recurrence_json["frequency"].asInt()));
            recurrence.setInterval(recurrence_json.get("interval", 1).asInt());

            if (recurrence_json.isMember("expires") && !recurrence_json["expires"].isNull()) {
                recurrence.setUntil(TimezoneUtils::convertToTargetFromUtc((QDateTime::fromString(recurrence_json["expires"].asCString(), "yyyy-MM-dd'T'hh:mm:ss'.000Z'")), false));
            }

            if (recurrence_json.isMember("numberOfOccurrences") && !recurrence_json["numberOfOccurrences"].isNull()) {
                recurrence.setNumberOfOccurrences(recurrence_json["numberOfOccurrences"].asInt());
            }

            QDateTime startTime = TimezoneUtils::convertToTargetFromUtc(QDateTime::fromString(QString(attributeObj["startTime"].asString().c_str()), QString("yyyy-MM-dd'T'hh:mm:ss'.000Z'")), false);
            recurrence.setDayInWeek(recurrence_json.get("dayInWeek", 1 << (startTime.date().dayOfWeek()%7)).asInt());
            recurrence.setWeekInMonth(recurrence_json.get("weekInMonth", (startTime.date().day()/7) + 1).asInt());
            recurrence.setDayInMonth(recurrence_json.get("dayInMonth", startTime.date().day()).asInt());
            recurrence.setMonthInYear(recurrence_json.get("monthInYear", startTime.date().month()).asInt());

            recurrence.resetExceptions();

            for (unsigned int i = 0; i < recurrence_json["exceptionDates"].size(); i++) {
                //recurrence.addException(QDateTime::fromString(recurrence_json["exceptionDates"][i].asCString(), "yyyy-MM-dd'T'hh:mm:ss'.000Z'"));
                exceptionDates.append(TimezoneUtils::convertToTargetFromUtc((QDateTime::fromString(recurrence_json["exceptionDates"][i].asCString(), "yyyy-MM-dd'T'hh:mm:ss'.000Z'")), false));
            }

            if (!recurrence.isValid()) {
                returnObj["_success"] = false;
                returnObj["code"] = UNKNOWN_ERROR;
                return returnObj;
            }

            calEvent.setRecurrence(recurrence);
        }
    }

    bbpim::CalendarService service;
    service.updateEvent(calEvent, bbpim::Notification());

    for (int i = 0; i < exceptionDates.size(); i++) {
        bbpim::CalendarEvent exceptionEvent;
        exceptionEvent.setStartTime(exceptionDates[i]);
        exceptionEvent.setId(calEvent.id());
        exceptionEvent.setAccountId(calEvent.accountId());
        service.createRecurrenceExclusion(exceptionEvent, bbpim::Notification());
    }

    if (calEvent.isValid()) {
        returnObj["event"] = populateEvent(calEvent, false);
        returnObj["_success"] = true;
    } else {
        returnObj["_success"] = false;
        returnObj["code"] = UNKNOWN_ERROR;
    }

    return returnObj;
}

/****************************************************************
 * Helper functions for Find
 ****************************************************************/
std::string PimCalendarQt::intToStr(const int val) {
    std::string s;
    std::stringstream out;
    out << val;
    return out.str();
}

bbpim::FolderId PimCalendarQt::intToFolderId(const quint32 id) {
  return (id != UNDEFINED_UINT) ? bbpim::FolderId(id) : -1;
}

QVariant PimCalendarQt::getFromMap(QMap<QString, QVariant> map, QStringList keys) {
    QVariant variant;
    QMap<QString, QVariant> currentMap = map;
    QStringList::iterator i;
    for(i = keys.begin(); i != keys.end(); ++i){
        if (currentMap.contains(*i)) {
            variant = currentMap.value(*i);
        } else {
            variant.clear();
            break;
        }

        if (variant.type() == QVariant::Map) {
            currentMap = variant.toMap();
        }
    }
    return variant;
}

std::string PimCalendarQt::getFolderKeyStr(bbpim::AccountId accountId, bbpim::FolderId folderId) {
    std::string str(intToStr(accountId));
    str += '-';
    str += intToStr(folderId);
    return str;
}

Json::Value PimCalendarQt::getCalendarFolderByFolderKey(bbpim::AccountId accountId, bbpim::FolderId folderId) {
    bbpim::CalendarService service;
    QList<bbpim::CalendarFolder> folders = service.folders();

    // populate map that contains all calendar folders
    for (QList<bbpim::CalendarFolder>::const_iterator i = folders.constBegin(); i != folders.constEnd(); i++) {
        bbpim::CalendarFolder folder = *i;

        if (folder.accountId() == accountId && folder.id() == folderId) {
            return getCalendarFolderJson(folder);
        }
    }

    return Json::Value();
}

void PimCalendarQt::lookupCalendarFolderByFolderKey(bbpim::AccountId accountId, bbpim::FolderId folderId) {
    std::string key = getFolderKeyStr(accountId, folderId);

    if (_allFoldersMap.empty()) {
        bbpim::CalendarService service;
        QList<bbpim::CalendarFolder> folders = service.folders();

        // populate map that contains all calendar folders
        for (QList<bbpim::CalendarFolder>::const_iterator i = folders.constBegin(); i != folders.constEnd(); i++) {
            bbpim::CalendarFolder folder = *i;
            _allFoldersMap.insert(std::pair<std::string, bbpim::CalendarFolder>(getFolderKeyStr(folder.accountId(), folder.id()), folder));
        }
    }

    if (_foldersMap.find(key) == _foldersMap.end()) {
        _foldersMap.insert(std::pair<std::string, bbpim::CalendarFolder>(key, _allFoldersMap[key]));
    }
}

bool PimCalendarQt::isDefaultCalendarFolder(const bbpim::CalendarFolder& folder) {
    bb::pim::account::AccountService accountService;
    bb::pim::account::Account defaultCalAccnt = accountService.defaultAccount(bb::pim::account::Service::Calendars);

    return (folder.accountId() == defaultCalAccnt.id() &&
        intToFolderId(accountService.getDefault(bb::pim::account::Service::Calendars)) == folder.id());
}

Json::Value PimCalendarQt::getCalendarFolderJson(const bbpim::CalendarFolder& folder, bool skipDefaultCheck) {
    Json::Value f;

    bb::pim::account::AccountService accountService;
    bb::pim::account::Account account = accountService.account(folder.accountId());
    QVariantMap variantMap = account.rawData();

    QMap<QString, QVariant> temp;
    temp = variantMap.value("capabilities").toMap();
    fprintf(stderr, "map is empty? %s\n", temp.empty() ? "true" : "false");
    for (QMap<QString, QVariant>::const_iterator i = temp.constBegin(); i != temp.constEnd(); i++) {
        fprintf(stderr, "Key: %s\n", i.key().toStdString().c_str());
    }

    QList<QString> keys;
    QVariant value;

    keys.clear();
    keys << "capabilities" << "supports_infinite_recurrence";
    value = getFromMap(variantMap, keys);
    if (value.isValid() && value.type() == QVariant::Bool) {
        f["supportsInfiniteRecurrence"] = value.toBool();
    } else {
        f["supportsInfiniteRecurrence"] = true; // assume true if not defined, as per Calendar app
    }

    keys.clear();
    keys << "capabilities" << "supports_meeting_participants";
    value = getFromMap(variantMap, keys);
    if (value.isValid() && value.type() == QVariant::Bool) {
        f["supportsParticipants"] = value.toBool();
    } else {
        f["supportsParticipants"] = true; // assume true if not defined, as per Calendar app
    }

    keys.clear();
    keys << "messages" << "supported";
    value = getFromMap(variantMap, keys);
    if (value.isValid() && value.type() == QVariant::Bool) {
        f["supportsMessaging"] = value.toBool();
    } else {
        f["supportsMessaging"] = false;
    }

    if (variantMap.contains("enterprise")) {
        f["enterprise"] = variantMap.value("enterprise").toBool();
    } else {
        f["enterprise"] = false; // assume false if not defined
    }

    f["id"] = intToStr(folder.id());
    f["accountId"] = intToStr(folder.accountId());
    f["name"] = folder.name().toStdString();
    f["readonly"] = folder.isReadOnly();
    f["ownerEmail"] = folder.ownerEmail().toStdString();
    f["type"] = folder.type();
    f["color"] = QString("%1").arg(folder.color(), 6, 16, QChar('0')).toUpper().toStdString();
    f["visible"] = folder.isVisible();
    f["default"] = skipDefaultCheck ? true : isDefaultCalendarFolder(folder);

    return f;
}


bool PimCalendarQt::getSearchParams(bbpim::EventSearchParameters& searchParams, const Json::Value& args) {
    if (args.isMember("options") && args["options"].isMember("filter") && !args["options"]["filter"].isNull()) {
        Json::Value options = args["options"];
        Json::Value filter = options["filter"];
        QDateTime now = QDateTime::currentDateTime();

        // filter - prefix - mandatory
        if (filter.isMember("prefix") && filter["prefix"].isString()) {
            searchParams.setPrefix(QString(filter["prefix"].asCString()));
        } else {
            return false;
        }

        // detail - mandatory
        if (options.isMember("detail") && options["detail"].isInt()) {
            searchParams.setDetails((bbpim::DetailLevel::Type) options["detail"].asInt());
        } else {
            return false;
        }

        // filter - start - optional
        if (!filter["start"].empty()) {
            searchParams.setStart(QDateTime::fromString(filter["start"].asCString(), Qt::ISODate)); // TODO
        } else {
            searchParams.setStart(now.addYears(-100));
        }

        // filter - end - optional
        if (!filter["end"].empty()) {
            searchParams.setEnd(QDateTime::fromString(filter["end"].asCString(), Qt::ISODate)); // TODO
        } else {
            searchParams.setEnd(now.addYears(100));
        }

        // filter - expand recurring - optional
        if (!filter["expandRecurring"].empty() && filter["expandRecurring"].isBool()) {
            searchParams.setExpand(filter["expandRecurring"].asBool());
        }

        // filter - folders - optional
        if (!filter["folders"].empty()) {
            for (unsigned int i = 0; i < filter["folders"].size(); i++) {
                Json::Value folder = filter["folders"][i];
                bbpim::FolderKey folderKey;

                folderKey.setFolderId(folder["id"].asInt());
                folderKey.setAccountId(folder["accountId"].asInt());

                searchParams.addFolder(folderKey);
            }
        }

        // sort - optional
        if (!options["sort"].empty() && options["sort"].isArray()) {
            QList<QPair<bbpim::SortField::Type, bool > > sortSpecsList;

            for (unsigned int i = 0; i < options["sort"].size(); i++) {
                Json::Value sort = options["sort"][i];
                QPair<bbpim::SortField::Type, bool> sortSpec;

                sortSpec.first = (bbpim::SortField::Type) sort["fieldName"].asInt();
                sortSpec.second = !sort["desc"].asBool();

                sortSpecsList.append(sortSpec);
            }

            searchParams.setSort(sortSpecsList);
        }

        // limit - optional
        if (!options["limit"].empty() && options["limit"].isInt() && !options["limit"].asInt() > 0) {
            searchParams.setLimit(options["limit"].asInt());
        }

        return true;
    }

    return false;
}

/*
void PimContactsQt::populateField(const bbpim::Contact& contact, bbpim::AttributeKind::Type kind, Json::Value& contactItem, bool isContactField, bool isArray)
{
    QList<bbpim::ContactAttribute> attrs = contact.filteredAttributes(kind);

    for (QList<bbpim::ContactAttribute>::const_iterator k = attrs.constBegin(); k != attrs.constEnd(); k++) {
        bbpim::ContactAttribute currentAttr = *k;

        // displayName and nickname are populated separately, do not populate within the name object
        if (currentAttr.subKind() == bbpim::AttributeSubKind::NameDisplayName || currentAttr.subKind() == bbpim::AttributeSubKind::NameNickname) {
            continue;
        }

        Json::Value val;
        SubKindToStringMap::const_iterator typeIter = _subKindAttributeMap.find(currentAttr.subKind());

        if (typeIter != _subKindAttributeMap.end()) {
            if (isContactField) {
                val["type"] = Json::Value(typeIter->second);
                val["value"] = Json::Value(currentAttr.value().toStdString());
                contactItem.append(val);
            } else {
                if (isArray) {
                    val = Json::Value(currentAttr.value().toStdString());
                    contactItem.append(val);
                } else {
                    if (kind == bbpim::AttributeKind::Date) {
                        QString format = "yyyy-MM-dd";
                        contactItem[typeIter->second] = Json::Value(currentAttr.valueAsDateTime().date().toString(format).toStdString());
                    } else {
                        if (kind == bbpim::AttributeKind::Note) {
                            contactItem["note"] = Json::Value(currentAttr.value().toStdString());
                        } else {
                            contactItem[typeIter->second] = Json::Value(currentAttr.value().toStdString());
                        }
                    }
                }
            }
        }
    }
}

void PimContactsQt::populateDisplayNameNickName(const bbpim::Contact& contact, Json::Value& contactItem, const std::string& field)
{
    QList<bbpim::ContactAttribute> attrs = contact.filteredAttributes(bbpim::AttributeKind::Name);
    bbpim::AttributeSubKind::Type subkind = (field == "displayName" ? bbpim::AttributeSubKind::NameDisplayName : bbpim::AttributeSubKind::NameNickname);

    for (QList<bbpim::ContactAttribute>::const_iterator k = attrs.constBegin(); k != attrs.constEnd(); k++) {
        bbpim::ContactAttribute currentAttr = *k;

        if (currentAttr.subKind() == subkind) {
            contactItem[field] = Json::Value(currentAttr.value().toStdString());
            break;
        }
    }
}

void PimContactsQt::populateAddresses(const bbpim::Contact& contact, Json::Value& contactAddrs)
{
    bbpim::ContactService contactService;
    bbpim::Contact fullContact = contactService.contactDetails(contact.id());
    QList<bbpim::ContactPostalAddress> addrs = fullContact.postalAddresses();

    for (QList<bbpim::ContactPostalAddress>::const_iterator k = addrs.constBegin(); k != addrs.constEnd(); k++) {
        bbpim::ContactPostalAddress currentAddr = *k;
        Json::Value addr;

        SubKindToStringMap::const_iterator typeIter = _subKindAttributeMap.find(currentAddr.subKind());

        if (typeIter != _subKindAttributeMap.end()) {
            addr["type"] = Json::Value(typeIter->second);
        }

        addr["streetAddress"] = Json::Value(currentAddr.line1().toStdString());
        addr["streetOther"] = Json::Value(currentAddr.line2().toStdString());
        addr["country"] = Json::Value(currentAddr.country().toStdString());
        addr["locality"] = Json::Value(currentAddr.city().toStdString());
        addr["postalCode"] = Json::Value(currentAddr.postalCode().toStdString());
        addr["region"] = Json::Value(currentAddr.region().toStdString());

        contactAddrs.append(addr);
    }
}

void PimContactsQt::populateOrganizations(const bbpim::Contact& contact, Json::Value& contactOrgs)
{
    QList<QList<bbpim::ContactAttribute> > orgAttrs = contact.filteredAttributesByGroupKey(bbpim::AttributeKind::OrganizationAffiliation);

    for (QList<QList<bbpim::ContactAttribute> >::const_iterator j = orgAttrs.constBegin(); j != orgAttrs.constEnd(); j++) {
        QList<bbpim::ContactAttribute> currentOrgAttrs = *j;
        Json::Value org;

        for (QList<bbpim::ContactAttribute>::const_iterator k = currentOrgAttrs.constBegin(); k != currentOrgAttrs.constEnd(); k++) {
            bbpim::ContactAttribute attr = *k;
            SubKindToStringMap::const_iterator typeIter = _subKindAttributeMap.find(attr.subKind());

            if (typeIter != _subKindAttributeMap.end()) {
                org[typeIter->second] = Json::Value(attr.value().toStdString());
            }
        }

        contactOrgs.append(org);
    }
}

void PimContactsQt::populatePhotos(const bbpim::Contact& contact, Json::Value& contactPhotos)
{
    bbpim::ContactService contactService;
    bbpim::Contact fullContact = contactService.contactDetails(contact.id());
    QList<bbpim::ContactPhoto> photos = fullContact.photos();
    bbpim::ContactPhoto primaryPhoto = fullContact.primaryPhoto();

    for (QList<bbpim::ContactPhoto>::const_iterator k = photos.constBegin(); k != photos.constEnd(); k++) {
        Json::Value photo;

        photo["originalFilePath"] = Json::Value((*k).originalPhoto().toStdString());
        photo["largeFilePath"] = Json::Value((*k).largePhoto().toStdString());
        photo["smallFilePath"] = Json::Value((*k).smallPhoto().toStdString());
        photo["pref"] = Json::Value((primaryPhoto.id() == (*k).id()));

        contactPhotos.append(photo);
    }
}

QString PimContactsQt::getSortFieldValue(const bbpim::SortColumn::Type sort_field, const bbpim::Contact& contact)
{
    switch (sort_field) {
        case bbpim::SortColumn::FirstName:
            return contact.sortFirstName();
        case bbpim::SortColumn::LastName:
            return contact.sortLastName();
        case bbpim::SortColumn::CompanyName:
            return contact.sortCompanyName();
    }

    return QString();
}

bool PimContactsQt::lessThan(const bbpim::Contact& c1, const bbpim::Contact& c2)
{
    QList<bbpim::SortSpecifier>::const_iterator i = PimContactsQt::_sortSpecs.constBegin();
    bbpim::SortSpecifier sortSpec;
    QString val1, val2;

    do {
        sortSpec = *i;
        val1 = PimContactsQt::getSortFieldValue(sortSpec.first, c1);
        val2 = PimContactsQt::getSortFieldValue(sortSpec.first, c2);
        ++i;
    } while (val1 == val2 && i != PimContactsQt::_sortSpecs.constEnd());

    if (sortSpec.second == bbpim::SortOrder::Ascending) {
        return val1 < val2;
    } else {
        return !(val1 < val2);
    }
}

Json::Value PimContactsQt::assembleSearchResults(const QSet<bbpim::ContactId>& resultIds, const Json::Value& contactFields, int limit)
{
    QMap<bbpim::ContactId, bbpim::Contact> completeResults;

    // put complete contacts in map
    for (QSet<bbpim::ContactId>::const_iterator i = resultIds.constBegin(); i != resultIds.constEnd(); i++) {
        completeResults.insertMulti(*i, _contactSearchMap[*i]);
    }

    // sort results based on sort specs
    QList<bbpim::Contact> sortedResults = completeResults.values();
    if (!_sortSpecs.empty()) {
        qSort(sortedResults.begin(), sortedResults.end(), lessThan);
    }

    Json::Value contactArray;

    // if limit is -1, returned all available results, otherwise return based on the number passed in find options
    if (limit == -1) {
        limit = sortedResults.size();
    } else {
        limit = std::min(limit, sortedResults.size());
    }

    for (int i = 0; i < limit; i++) {
        Json::Value contactItem = populateContact(sortedResults[i], contactFields);
        contactArray.append(contactItem);
    }

    return contactArray;
}
*/

/****************************************************************
 * Helper functions shared by Find and Save
 ****************************************************************/
Json::Value PimCalendarQt::populateEvent(const bbpim::CalendarEvent& event, bool isFind)
{
    Json::Value e;

    e["accountId"] = intToStr(event.accountId());
    e["id"] = intToStr(event.id());

    if (!isFind) {
        e["folder"] = getCalendarFolderByFolderKey(event.accountId(), event.folderId());
    }

    e["folderId"] = intToStr(event.folderId());
    e["parentId"] = intToStr(event.parentId());

    // Reminder can be negative, when an all-day event is created, and reminder is set to "On the day at 9am", reminder=-540 (negative!)
    // For events with all-day=false, default reminder (in calendar app) is 15 mins before start -> reminder=15:
    // Tested:
    // 1 hour before start -> reminder=60
    // 2 days before start -> reminder=2880
    // 1 week before start -> reminder=10080
    e["reminder"] = event.reminder();
    e["birthday"] = event.isBirthday();
    e["allDay"] = event.isAllDay();

    // meeting status values:
    // - 0: not a meeting;
    // - 1 and 9: is a meeting;
    // - 3 and 11: meeting received;
    // - 5 and 13: meeting is canceled;
    // - 7 and 15: meeting is canceled and received.
    e["status"] = event.meetingStatus();

    // busy status values (BusyStatus::Type)
    // Free = 0, Used to inform that the event represents free time (the event's owner is available)
    // Tentative = 1, Tells that an event may or may not happen (the owner may be available).
    // Busy = 2, Tells that the event is confirmed (the owner is busy).
    // OutOfOffice = 3, Indicates that the event owner is out of office.
    e["transparency"] = event.busyStatus();
    // e["start"] = event.startTime().toString(format).toStdString();
    // e["end"] = event.endTime().toString(format).toStdString();
    e["start"] = QString::number(event.startTime().toUTC().toMSecsSinceEpoch()).toStdString();
    e["end"] = QString::number(event.endTime().toUTC().toMSecsSinceEpoch()).toStdString();

    // sensitivity values (Sensitivity::Type)
    // Normal = 0, To be used for unrestricted events.
    // Personal = 1, Sensitivity value for personal events.
    // Private = 2, Sensitivity level for private events.
    // Confidential = 3, Maximum sensitivity level for events.
    e["sensitivity"] = event.sensitivity();
    e["timezone"] = event.timezone().toStdString();
    e["summary"] = event.subject().toStdString();
    e["description"] = event.body().toStdString();
    e["location"] = event.location().toStdString();
    e["url"] = event.url().toStdString();
    e["attendees"] = Json::Value();

    QList<bbpim::Attendee> attendees = event.attendees();

    for (QList<bbpim::Attendee>::const_iterator j = attendees.constBegin(); j != attendees.constEnd(); j++) {
        bbpim::Attendee attendee = *j;
        Json::Value a;

        a["id"] = intToStr(attendee.id());
        a["eventId"] = intToStr(attendee.eventId());

        // contactId is 0 even if contact is on device...maybe it's a permission issue (contact permission not specified in app)
        // would most likely just leave it out
        a["contactId"] = intToStr(attendee.contactId());
        a["email"] = attendee.email().toStdString();
        a["name"] = attendee.name().toStdString();
        a["type"] = attendee.type();
        a["role"] = attendee.role();
        a["owner"] = attendee.isOwner();
        a["status"] = attendee.status();

        e["attendees"].append(a);
    }

    if (event.recurrence().isValid()) {
        e["recurrence"] = Json::Value();
        //e["recurrence"]["start"] =  QString::number(event.recurrence().start().toUTC().toMSecsSinceEpoch()).toStdString();
        //e["recurrence"]["end"] = QString::number(event.recurrence().end().toUTC().toMSecsSinceEpoch()).toStdString();
        e["recurrence"]["frequency"] = event.recurrence().frequency();
        e["recurrence"]["interval"] = event.recurrence().interval();
        e["recurrence"]["numberOfOccurrences"] = event.recurrence().numberOfOccurrences();
        e["recurrence"]["dayInWeek"] = event.recurrence().dayInWeek();
        e["recurrence"]["dayInMonth"] = event.recurrence().dayInMonth();
        e["recurrence"]["weekInMonth"] = event.recurrence().weekInMonth();
        e["recurrence"]["monthInYear"] = event.recurrence().monthInYear();
        e["recurrence"]["exceptionDates"];

        if (event.recurrence().until().isValid()) {
            e["recurrence"]["expires"] = QString::number(event.recurrence().until().toUTC().toMSecsSinceEpoch()).toStdString();
        }

        QList<QDateTime> exceptions = event.recurrence().exceptions();
        for (int i = 0; i < exceptions.size(); i++) {
            e["recurrence"]["exceptionDates"].append(QString::number(exceptions[i].toUTC().toMSecsSinceEpoch()).toStdString());
        }
    }

    return e;
}


/*
Json::Value PimContactsQt::populateContact(bbpim::Contact& contact, const Json::Value& contactFields)
{
    Json::Value contactItem;

    for (int i = 0; i < contactFields.size(); i++) {
        std::string field = contactFields[i].asString();
        std::map<std::string, bbpim::AttributeKind::Type>::const_iterator kindIter = _attributeKindMap.find(field);

        if (kindIter != _attributeKindMap.end()) {
            switch (kindIter->second) {
                case bbpim::AttributeKind::Name: {
                    contactItem[field] = Json::Value();
                    populateField(contact, kindIter->second, contactItem[field], false, false);
                    break;
                }

                case bbpim::AttributeKind::OrganizationAffiliation: {
                    contactItem[field] = Json::Value();
                    populateOrganizations(contact, contactItem[field]);
                    break;
                }

                case bbpim::AttributeKind::Date:
                case bbpim::AttributeKind::Note:
                case bbpim::AttributeKind::Sound: {
                    populateField(contact, kindIter->second, contactItem, false, false);
                    break;
                }

                case bbpim::AttributeKind::VideoChat: {
                    contactItem[field] = Json::Value();
                    populateField(contact, kindIter->second, contactItem[field], false, true);
                    break;
                }

                case bbpim::AttributeKind::Email:
                case bbpim::AttributeKind::Fax:
                case bbpim::AttributeKind::Pager:
                case bbpim::AttributeKind::Phone:
                case bbpim::AttributeKind::Profile:
                case bbpim::AttributeKind::Website:
                case bbpim::AttributeKind::InstantMessaging: {
                    contactItem[field] = Json::Value();
                    populateField(contact, kindIter->second, contactItem[field], true, false);
                    break;
                }

                // Special cases (treated differently in ContactBuilder):
                default: {
                    if (field == "addresses") {
                        contactItem[field] = Json::Value();
                        populateAddresses(contact, contactItem[field]);
                    } else if (field == "photos") {
                        contactItem[field] = Json::Value();
                        populatePhotos(contact, contactItem[field]);
                    }

                    break;
                }
            }
        } else {
            if (field == "displayName" || field == "nickname") {
                populateDisplayNameNickName(contact, contactItem, field);
            }
        }
    }

    contactItem["id"] = Json::Value(contact.id());
    contactItem["favorite"] = Json::Value(contact.isFavourite()); // always populate favorite

    return contactItem;
}
*/
/****************************************************************
 * Helper functions for Save
 ****************************************************************/
/*
void PimContactsQt::addAttributeKind(bbpim::ContactBuilder& contactBuilder, const Json::Value& jsonObj, const std::string& field)
{
    StringToKindMap::const_iterator kindIter = _attributeKindMap.find(field);

    if (kindIter != _attributeKindMap.end()) {
        switch (kindIter->second) {
            // Attributes requiring group keys:
            case bbpim::AttributeKind::Name: {
                QList<SubkindValuePair> convertedList = convertGroupedAttributes(jsonObj);
                addConvertedGroupedList(contactBuilder, kindIter->second, convertedList, "1");
                break;
            }
            case bbpim::AttributeKind::OrganizationAffiliation: {
                for (int i = 0; i < jsonObj.size(); i++) {
                    std::stringstream groupKeyStream;
                    groupKeyStream << i + 1;

                    QList<SubkindValuePair> convertedList = convertGroupedAttributes(jsonObj[i]);
                    addConvertedGroupedList(contactBuilder, kindIter->second, convertedList, groupKeyStream.str());
                }

                break;
            }

            // String arrays:
            case bbpim::AttributeKind::VideoChat: {
                QList<SubkindValuePair> convertedList = convertStringArray(jsonObj, bbpim::AttributeSubKind::VideoChatBbPlaybook);
                addConvertedList(contactBuilder, kindIter->second, convertedList);
                break;
            }

            // Dates:
            case bbpim::AttributeKind::Date: {
                StringToSubKindMap::const_iterator subkindIter = _attributeSubKindMap.find(field);

                if (subkindIter != _attributeSubKindMap.end()) {
                    std::string value = jsonObj.asString();
                    addAttributeDate(contactBuilder, kindIter->second, subkindIter->second, value);
                }

                break;
            }

            // Strings:
            case bbpim::AttributeKind::Note:
            case bbpim::AttributeKind::Sound: {
                StringToSubKindMap::const_iterator subkindIter = _attributeSubKindMap.find(field);

                if (subkindIter != _attributeSubKindMap.end()) {
                    QList<SubkindValuePair> convertedList;
                    std::string value = jsonObj.asString();
                    convertedList.append(SubkindValuePair(subkindIter->second, value));
                    addConvertedList(contactBuilder, kindIter->second, convertedList);
                }

                break;
            }

            // ContactField attributes:
            case bbpim::AttributeKind::Phone:
            case bbpim::AttributeKind::Email:
            case bbpim::AttributeKind::Fax:
            case bbpim::AttributeKind::Pager:
            case bbpim::AttributeKind::InstantMessaging:
            case bbpim::AttributeKind::Website:
            case bbpim::AttributeKind::Group:
            case bbpim::AttributeKind::Profile: {
                QList<SubkindValuePair> convertedList = convertFieldAttributes(jsonObj);
                addConvertedList(contactBuilder, kindIter->second, convertedList);
                break;
            }

            // Special cases (treated differently in ContactBuilder):
            default: {
                if (field == "addresses") {
                    for (int i = 0; i < jsonObj.size(); i++) {
                        Json::Value addressObj = jsonObj[i];
                        addPostalAddress(contactBuilder, addressObj);
                    }
                } else if (field == "photos") {
                    for (int i = 0; i < jsonObj.size(); i++) {
                        Json::Value photoObj = jsonObj[i];
                        addPhoto(contactBuilder, photoObj);
                    }
                } else if (field == "favorite") {
                    bool isFavorite = jsonObj.asBool();
                    contactBuilder = contactBuilder.setFavorite(isFavorite);
                }

                break;
            }
        }
    }
}

void PimContactsQt::syncAttributeKind(bbpim::Contact& contact, const Json::Value& jsonObj, const std::string& field)
{
    StringToKindMap::const_iterator kindIter = _attributeKindMap.find(field);
    bbpim::ContactBuilder contactBuilder(contact.edit());

    if (kindIter != _attributeKindMap.end()) {
        switch (kindIter->second) {
            // Attributes requiring group keys:
            case bbpim::AttributeKind::Name: {
                QList<QList<bbpim::ContactAttribute> > savedList = contact.filteredAttributesByGroupKey(kindIter->second);
                QList<SubkindValuePair> convertedList = convertGroupedAttributes(jsonObj);

                if (!savedList.empty()) {
                    syncConvertedGroupedList(contactBuilder, kindIter->second, savedList[0], convertedList, "1");
                } else {
                    addConvertedGroupedList(contactBuilder, kindIter->second, convertedList, "1");
                }

                break;
            }
            case bbpim::AttributeKind::OrganizationAffiliation: {
                QList<QList<bbpim::ContactAttribute> > savedList = contact.filteredAttributesByGroupKey(kindIter->second);
                syncAttributeGroup(contactBuilder, kindIter->second, savedList, jsonObj);
                break;
            }

            // String arrays:
            case bbpim::AttributeKind::VideoChat: {
                QList<bbpim::ContactAttribute> savedList = contact.filteredAttributes(kindIter->second);
                QList<SubkindValuePair> convertedList = convertStringArray(jsonObj, bbpim::AttributeSubKind::VideoChatBbPlaybook);
                syncConvertedList(contactBuilder, kindIter->second, savedList, convertedList);
                break;
            }

            // Dates:
            case bbpim::AttributeKind::Date: {
                StringToSubKindMap::const_iterator subkindIter = _attributeSubKindMap.find(field);

                if (subkindIter != _attributeSubKindMap.end()) {
                    QList<bbpim::ContactAttribute> savedList = contact.filteredAttributes(kindIter->second);
                    syncAttributeDate(contactBuilder, savedList, subkindIter->second, jsonObj.asString());
                }

                break;
            }

            // Strings:
            case bbpim::AttributeKind::Note:
            case bbpim::AttributeKind::Sound: {
                QList<bbpim::ContactAttribute> savedList = contact.filteredAttributes(kindIter->second);
                QList<SubkindValuePair> convertedList;
                StringToSubKindMap::const_iterator subkindIter = _attributeSubKindMap.find(field);

                if (subkindIter != _attributeSubKindMap.end()) {
                    std::string value = jsonObj.asString();
                    convertedList.append(SubkindValuePair(subkindIter->second, value));
                }

                syncConvertedList(contactBuilder, kindIter->second, savedList, convertedList);
                break;
            }

            // ContactField attributes:
            case bbpim::AttributeKind::Phone:
            case bbpim::AttributeKind::Email:
            case bbpim::AttributeKind::Fax:
            case bbpim::AttributeKind::Pager:
            case bbpim::AttributeKind::InstantMessaging:
            case bbpim::AttributeKind::Website:
            case bbpim::AttributeKind::Group:
            case bbpim::AttributeKind::Profile: {
                QList<bbpim::ContactAttribute> savedList = contact.filteredAttributes(kindIter->second);
                QList<SubkindValuePair> convertedList = convertFieldAttributes(jsonObj);
                syncConvertedList(contactBuilder, kindIter->second, savedList, convertedList);
                break;
            }

            // Special cases (treated differently in ContactBuilder):
            default: {
                if (field == "addresses") {
                    QList<bbpim::ContactPostalAddress> savedList = contact.postalAddresses();
                    syncPostalAddresses(contactBuilder, savedList, jsonObj);
                } else if (field == "photos") {
                    QList<bbpim::ContactPhoto> savedList = contact.photos();
                    syncPhotos(contactBuilder, savedList, jsonObj);

                } else if (field == "favorite") {
                    bool isFavorite = jsonObj.asBool();
                    contactBuilder.setFavorite(isFavorite);
                }

                break;
            }
        }
    }
}


QList<SubkindValuePair> PimContactsQt::convertGroupedAttributes(const Json::Value& fieldsObj)
{
    const Json::Value::Members fields = fieldsObj.getMemberNames();
    QList<SubkindValuePair> convertedList;

    for (int i = 0; i < fields.size(); i++) {
        const std::string fieldKey = fields[i];
        StringToSubKindMap::const_iterator subkindIter = _attributeSubKindMap.find(fieldKey);

        if (subkindIter != _attributeSubKindMap.end()) {
            convertedList.append(SubkindValuePair(subkindIter->second, fieldsObj[fieldKey].asString()));
        }
    }

    return convertedList;
}

QList<SubkindValuePair> PimContactsQt::convertFieldAttributes(const Json::Value& fieldArray)
{
    QList<SubkindValuePair> convertedList;

    for (int i = 0; i < fieldArray.size(); i++) {
        Json::Value fieldObj = fieldArray[i];
        std::string type = fieldObj.get("type", "").asString();
        std::string value = fieldObj.get("value", "").asString();
        StringToSubKindMap::const_iterator subkindIter = _attributeSubKindMap.find(type);

        if (subkindIter != _attributeSubKindMap.end()) {
            convertedList.append(SubkindValuePair(subkindIter->second, value));
        }
    }

    return convertedList;
}

QList<SubkindValuePair> PimContactsQt::convertStringArray(const Json::Value& stringArray, bbpim::AttributeSubKind::Type subkind)
{
    QList<SubkindValuePair> convertedList;

    for (int i = 0; i < stringArray.size(); i++) {
        std::string value = stringArray[i].asString();
        convertedList.append(SubkindValuePair(subkind, value));
    }

    return convertedList;
}

void PimContactsQt::addConvertedList(bbpim::ContactBuilder& contactBuilder, const bbpim::AttributeKind::Type kind, const QList<SubkindValuePair>& convertedList)
{
    for (int i = 0; i < convertedList.size(); i++) {
        //addAttribute(contactBuilder, kind, convertedList[i].first, convertedList[i].second);
        bbpim::ContactAttribute attribute;
        bbpim::ContactAttributeBuilder attributeBuilder(attribute.edit());

        attributeBuilder = attributeBuilder.setKind(kind);
        attributeBuilder = attributeBuilder.setSubKind(convertedList[i].first);
        attributeBuilder = attributeBuilder.setValue(QString(convertedList[i].second.c_str()));

        contactBuilder.addAttribute(attribute);
    }
}

void PimContactsQt::addConvertedGroupedList(bbpim::ContactBuilder& contactBuilder, const bbpim::AttributeKind::Type kind, const QList<SubkindValuePair>& convertedList, const std::string& groupKey)
{
    for (int i = 0; i < convertedList.size(); i++) {
        //addAttributeToGroup(contactBuilder, kind, convertedList[i].first, convertedList[i].second, groupKey);
        bbpim::ContactAttribute attribute;
        bbpim::ContactAttributeBuilder attributeBuilder(attribute.edit());

        attributeBuilder = attributeBuilder.setKind(kind);
        attributeBuilder = attributeBuilder.setSubKind(convertedList[i].first);
        attributeBuilder = attributeBuilder.setValue(QString(convertedList[i].second.c_str()));
        attributeBuilder = attributeBuilder.setGroupKey(QString(groupKey.c_str()));

        contactBuilder.addAttribute(attribute);
    }
}

void PimContactsQt::addAttributeDate(bbpim::ContactBuilder& contactBuilder, const bbpim::AttributeKind::Type kind, const bbpim::AttributeSubKind::Type subkind, const std::string& value)
{
    bbpim::ContactAttribute attribute;
    bbpim::ContactAttributeBuilder attributeBuilder(attribute.edit());

    QDateTime date = QDateTime::fromString(QString(value.c_str()), QString("ddd MMM dd yyyy"));

    attributeBuilder = attributeBuilder.setKind(kind);
    attributeBuilder = attributeBuilder.setSubKind(subkind);

    if (date.isValid()) {
        attributeBuilder = attributeBuilder.setValue(date);
    } else {
        attributeBuilder = attributeBuilder.setValue(QString(value.c_str()));
    }

    contactBuilder.addAttribute(attribute);
}
*/
/*
void PimContactsQt::addAttribute(bbpim::ContactBuilder& contactBuilder, const bbpim::AttributeKind::Type kind, const bbpim::AttributeSubKind::Type subkind, const std::string& value)
{
    bbpim::ContactAttribute attribute;
    bbpim::ContactAttributeBuilder attributeBuilder(attribute.edit());

    attributeBuilder = attributeBuilder.setKind(kind);
    attributeBuilder = attributeBuilder.setSubKind(subkind);
    attributeBuilder = attributeBuilder.setValue(QString(value.c_str()));

    contactBuilder.addAttribute(attribute);
}
*/

/*
void PimContactsQt::addAttributeToGroup(bbpim::ContactBuilder& contactBuilder, const bbpim::AttributeKind::Type kind, const bbpim::AttributeSubKind::Type subkind, const std::string& value, const std::string& groupKey)
{
    bbpim::ContactAttribute attribute;
    bbpim::ContactAttributeBuilder attributeBuilder(attribute.edit());

    attributeBuilder = attributeBuilder.setKind(kind);
    attributeBuilder = attributeBuilder.setSubKind(subkind);
    attributeBuilder = attributeBuilder.setValue(QString(value.c_str()));
    attributeBuilder = attributeBuilder.setGroupKey(QString(groupKey.c_str()));

    contactBuilder.addAttribute(attribute);
}
*/
/*
void PimContactsQt::addPostalAddress(bbpim::ContactBuilder& contactBuilder, const Json::Value& addressObj)
{
    bbpim::ContactPostalAddress address;
    bbpim::ContactPostalAddressBuilder addressBuilder(address.edit());

    if (addressObj.isMember("type")) {
        std::string value = addressObj["type"].asString();
        StringToSubKindMap::const_iterator subkindIter = _attributeSubKindMap.find(value);

        if (subkindIter != _attributeSubKindMap.end()) {
            addressBuilder = addressBuilder.setSubKind(subkindIter->second);
        }
    }

    addressBuilder = addressBuilder.setLine1(QString(addressObj.get("streetAddress", "").asCString()));
    addressBuilder = addressBuilder.setLine2(QString(addressObj.get("streetOther", "").asCString()));
    addressBuilder = addressBuilder.setCity(QString(addressObj.get("locality", "").asCString()));
    addressBuilder = addressBuilder.setRegion(QString(addressObj.get("region", "").asCString()));
    addressBuilder = addressBuilder.setCountry(QString(addressObj.get("country", "").asCString()));
    addressBuilder = addressBuilder.setPostalCode(QString(addressObj.get("postalCode", "").asCString()));

    contactBuilder = contactBuilder.addPostalAddress(address);
}

void PimContactsQt::addPhoto(bbpim::ContactBuilder& contactBuilder, const Json::Value& photoObj)
{
    bbpim::ContactPhoto photo;
    bbpim::ContactPhotoBuilder photoBuilder(photo.edit());

    std::string filepath = photoObj.get("originalFilePath", "").asString();
    bool pref = photoObj.get("pref", false).asBool();

    photoBuilder.setOriginalPhoto(QString(filepath.c_str()));
    photoBuilder.setPrimaryPhoto(pref);

    contactBuilder = contactBuilder.addPhoto(photo, pref);
}


void PimContactsQt::syncConvertedList(bbpim::ContactBuilder& contactBuilder, bbpim::AttributeKind::Type kind, QList<bbpim::ContactAttribute> savedList, const QList<SubkindValuePair>& convertedList)
{
    int index;

    for (index = 0; index < savedList.size() && index < convertedList.size(); index++) {
        if (savedList[index].subKind() != convertedList[index].first || savedList[index].value().toStdString() != convertedList[index].second) {
            bbpim::ContactAttributeBuilder attributeBuilder(savedList[index].edit());
            attributeBuilder = attributeBuilder.setSubKind(convertedList[index].first);
            attributeBuilder = attributeBuilder.setValue(QString(convertedList[index].second.c_str()));
        }
    }

    if (index < savedList.size()) {
        for (; index < savedList.size(); index++) {
            contactBuilder = contactBuilder.deleteAttribute(savedList[index]);
        }
    } else if (index < convertedList.size()) {
        for (; index < convertedList.size(); index++) {
            QList<SubkindValuePair> remainingList = convertedList.mid(index);
            addConvertedList(contactBuilder, kind, remainingList);
        }
    }
}

void PimContactsQt::syncConvertedGroupedList(bbpim::ContactBuilder& contactBuilder, bbpim::AttributeKind::Type kind, QList<bbpim::ContactAttribute> savedList, QList<SubkindValuePair> convertedList, const std::string& groupKey)
{
    int index;

    for (index = 0; index < savedList.size() && index < convertedList.size(); index++) {
        bbpim::ContactAttributeBuilder attributeBuilder(savedList[index].edit());
        attributeBuilder = attributeBuilder.setSubKind(convertedList[index].first);
        attributeBuilder = attributeBuilder.setValue(QString(convertedList[index].second.c_str()));
        attributeBuilder = attributeBuilder.setGroupKey(QString(groupKey.c_str()));
    }

    if (index < savedList.size()) {
        for (; index < savedList.size(); index++) {
            contactBuilder = contactBuilder.deleteAttribute(savedList[index]);
        }
    } else if (index < convertedList.size()) {
        for (; index < convertedList.size(); index++) {
            QList<SubkindValuePair> remainingList = convertedList.mid(index);
            addConvertedList(contactBuilder, kind, remainingList);
        }
    }
}

void PimContactsQt::syncAttributeGroup(bbpim::ContactBuilder& contactBuilder, bbpim::AttributeKind::Type kind, QList<QList<bbpim::ContactAttribute> > savedList, const Json::Value& jsonObj)
{
    int i;

    for (i = 0; i < jsonObj.size() && i < savedList.size(); i++) {
        std::stringstream groupKeyStream;
        groupKeyStream << i + 1;

        QList<SubkindValuePair> convertedList = convertGroupedAttributes(jsonObj[i]);
        syncConvertedGroupedList(contactBuilder, kind, savedList[i], convertedList, groupKeyStream.str());
    }

    if (i < savedList.size()) {
        for (; i < savedList.size(); i++) {
            for (int j = 0; j < savedList[i].size(); j++) {
                contactBuilder = contactBuilder.deleteAttribute(savedList[i][j]);
            }
        }
    } else if (i < jsonObj.size()) {
        for (; i < jsonObj.size(); i++) {
            std::stringstream groupKeyStream;
            groupKeyStream << i + 1;

            QList<SubkindValuePair> convertedList = convertGroupedAttributes(jsonObj[i]);
            addConvertedGroupedList(contactBuilder, kind, convertedList, groupKeyStream.str());
        }
    }
}

void PimContactsQt::syncAttributeDate(bbpim::ContactBuilder& contactBuilder, QList<bbpim::ContactAttribute>& savedList, const bbpim::AttributeSubKind::Type subkind, const std::string& value)
{
    bool found = false;

    for (int i = 0; i < savedList.size(); i++) {
        if (savedList[i].subKind() == subkind) {
            if (found) {
                contactBuilder = contactBuilder.deleteAttribute(savedList[i]);
            } else {
                found = true;
                bbpim::ContactAttributeBuilder attributeBuilder(savedList[i].edit());
                QDateTime date = QDateTime::fromString(QString(value.c_str()), QString("ddd MMM dd yyyy"));

                if (date.isValid()) {
                    attributeBuilder = attributeBuilder.setValue(date);
                } else {
                    attributeBuilder = attributeBuilder.setValue(QString(value.c_str()));
                }
            }
        }
    }
}

void PimContactsQt::syncPostalAddresses(bbpim::ContactBuilder& contactBuilder, QList<bbpim::ContactPostalAddress>& savedList, const Json::Value& jsonObj)
{
    int i;

    for (i = 0; i < savedList.size() && i < jsonObj.size(); i++) {
        Json::Value addressObj = jsonObj[i];
        bbpim::ContactPostalAddressBuilder addressBuilder(savedList[i].edit());

        std::string type = addressObj.get("type", "other").asString();
        StringToSubKindMap::const_iterator subkindIter = _attributeSubKindMap.find(type);

        if (subkindIter != _attributeSubKindMap.end()) {
            addressBuilder = addressBuilder.setSubKind(subkindIter->second);
        }

        addressBuilder = addressBuilder.setLine1(QString(addressObj.get("streetAddress", "").asCString()));
        addressBuilder = addressBuilder.setLine2(QString(addressObj.get("streetOther", "").asCString()));
        addressBuilder = addressBuilder.setCity(QString(addressObj.get("locality", "").asCString()));
        addressBuilder = addressBuilder.setRegion(QString(addressObj.get("region", "").asCString()));
        addressBuilder = addressBuilder.setCountry(QString(addressObj.get("country", "").asCString()));
        addressBuilder = addressBuilder.setPostalCode(QString(addressObj.get("postalCode", "").asCString()));
    }

    if (i < savedList.size()) {
        for (; i < savedList.size(); i++) {
            contactBuilder = contactBuilder.deletePostalAddress(savedList[i]);
        }
    } else if (i < jsonObj.size()) {
        for (; i < jsonObj.size(); i++) {
            addPostalAddress(contactBuilder, jsonObj[i]);
        }
    }
}

void PimContactsQt::syncPhotos(bbpim::ContactBuilder& contactBuilder, QList<bbpim::ContactPhoto>& savedList, const Json::Value& jsonObj)
{
    int i;

    for (i = 0; i < savedList.size() && i < jsonObj.size(); i++) {
        std::string filepath = jsonObj[i].get("originalFilePath", "").asString();
        bool pref = jsonObj[i].get("pref", false).asBool();

        bbpim::ContactPhotoBuilder photoBuilder(savedList[i].edit());
        photoBuilder.setOriginalPhoto(QString(filepath.c_str()));
        photoBuilder.setPrimaryPhoto(pref);
    }

    if (i < savedList.size()) {
        for (; i < savedList.size(); i++) {
            contactBuilder = contactBuilder.deletePhoto(savedList[i]);
        }
    } else if (i < jsonObj.size()) {
        for (; i < jsonObj.size(); i++) {
            addPhoto(contactBuilder, jsonObj[i]);
        }
    }
}
*/
/****************************************************************
 * Mapping functions
 ****************************************************************/
/*
void PimContactsQt::createAttributeKindMap()
{
    _attributeKindMap["phoneNumbers"] = bbpim::AttributeKind::Phone;
    _attributeKindMap["faxNumbers"] = bbpim::AttributeKind::Fax;
    _attributeKindMap["pagerNumbers"] = bbpim::AttributeKind::Pager;
    _attributeKindMap["emails"] = bbpim::AttributeKind::Email;
    _attributeKindMap["urls"] = bbpim::AttributeKind::Website;
    _attributeKindMap["socialNetworks"] = bbpim::AttributeKind::Profile;
    _attributeKindMap["anniversary"] = bbpim::AttributeKind::Date;
    _attributeKindMap["birthday"] = bbpim::AttributeKind::Date;
    _attributeKindMap["categories"] = bbpim::AttributeKind::Group;
    _attributeKindMap["name"] = bbpim::AttributeKind::Name;
    _attributeKindMap["organizations"] = bbpim::AttributeKind::OrganizationAffiliation;
    _attributeKindMap["education"] = bbpim::AttributeKind::Education;
    _attributeKindMap["note"] = bbpim::AttributeKind::Note;
    _attributeKindMap["ims"] = bbpim::AttributeKind::InstantMessaging;
    _attributeKindMap["ringtone"] = bbpim::AttributeKind::Sound;
    _attributeKindMap["videoChat"] = bbpim::AttributeKind::VideoChat;
    _attributeKindMap["addresses"] = bbpim::AttributeKind::Invalid;
    _attributeKindMap["favorite"] = bbpim::AttributeKind::Invalid;
    _attributeKindMap["photos"] = bbpim::AttributeKind::Invalid;
}

void PimContactsQt::createAttributeSubKindMap()
{
    _attributeSubKindMap["other"] = bbpim::AttributeSubKind::Other;
    _attributeSubKindMap["home"] = bbpim::AttributeSubKind::Home;
    _attributeSubKindMap["work"] = bbpim::AttributeSubKind::Work;
    _attributeSubKindMap["mobile"] = bbpim::AttributeSubKind::PhoneMobile;
    _attributeSubKindMap["direct"] = bbpim::AttributeSubKind::FaxDirect;
    _attributeSubKindMap["blog"] = bbpim::AttributeSubKind::Blog;
    _attributeSubKindMap["resume"] = bbpim::AttributeSubKind::WebsiteResume;
    _attributeSubKindMap["portfolio"] = bbpim::AttributeSubKind::WebsitePortfolio;
    _attributeSubKindMap["personal"] = bbpim::AttributeSubKind::WebsitePersonal;
    _attributeSubKindMap["company"] = bbpim::AttributeSubKind::WebsiteCompany;
    _attributeSubKindMap["facebook"] = bbpim::AttributeSubKind::ProfileFacebook;
    _attributeSubKindMap["twitter"] = bbpim::AttributeSubKind::ProfileTwitter;
    _attributeSubKindMap["linkedin"] = bbpim::AttributeSubKind::ProfileLinkedIn;
    _attributeSubKindMap["gist"] = bbpim::AttributeSubKind::ProfileGist;
    _attributeSubKindMap["tungle"] = bbpim::AttributeSubKind::ProfileTungle;
    _attributeSubKindMap["birthday"] = bbpim::AttributeSubKind::DateBirthday;
    _attributeSubKindMap["anniversary"] = bbpim::AttributeSubKind::DateAnniversary;
    _attributeSubKindMap["categories"] = bbpim::AttributeSubKind::GroupDepartment;
    _attributeSubKindMap["givenName"] = bbpim::AttributeSubKind::NameGiven;
    _attributeSubKindMap["familyName"] = bbpim::AttributeSubKind::NameSurname;
    _attributeSubKindMap["honorificPrefix"] = bbpim::AttributeSubKind::Title;
    _attributeSubKindMap["honorificSuffix"] = bbpim::AttributeSubKind::NameSuffix;
    _attributeSubKindMap["middleName"] = bbpim::AttributeSubKind::NameMiddle;
    _attributeSubKindMap["nickname"] = bbpim::AttributeSubKind::NameNickname;
    _attributeSubKindMap["displayName"] = bbpim::AttributeSubKind::NameDisplayName;
    _attributeSubKindMap["phoneticGivenName"] = bbpim::AttributeSubKind::NamePhoneticGiven;
    _attributeSubKindMap["phoneticFamilyName"] = bbpim::AttributeSubKind::NamePhoneticSurname;
    _attributeSubKindMap["name"] = bbpim::AttributeSubKind::OrganizationAffiliationName;
    _attributeSubKindMap["department"] = bbpim::AttributeSubKind::OrganizationAffiliationDetails;
    _attributeSubKindMap["title"] = bbpim::AttributeSubKind::Title;
    _attributeSubKindMap["BbmPin"] = bbpim::AttributeSubKind::InstantMessagingBbmPin;
    _attributeSubKindMap["Aim"] = bbpim::AttributeSubKind::InstantMessagingAim;
    _attributeSubKindMap["Aliwangwang"] = bbpim::AttributeSubKind::InstantMessagingAliwangwang;
    _attributeSubKindMap["GoogleTalk"] = bbpim::AttributeSubKind::InstantMessagingGoogleTalk;
    _attributeSubKindMap["Sametime"] = bbpim::AttributeSubKind::InstantMessagingSametime;
    _attributeSubKindMap["Icq"] = bbpim::AttributeSubKind::InstantMessagingIcq;
    _attributeSubKindMap["Jabber"] = bbpim::AttributeSubKind::InstantMessagingJabber;
    _attributeSubKindMap["MsLcs"] = bbpim::AttributeSubKind::InstantMessagingMsLcs;
    _attributeSubKindMap["Skype"] = bbpim::AttributeSubKind::InstantMessagingSkype;
    _attributeSubKindMap["YahooMessenger"] = bbpim::AttributeSubKind::InstantMessagingYahooMessenger;
    _attributeSubKindMap["YahooMessegerJapan"] = bbpim::AttributeSubKind::InstantMessagingYahooMessengerJapan;
    _attributeSubKindMap["BbPlaybook"] = bbpim::AttributeSubKind::VideoChatBbPlaybook;
    _attributeSubKindMap["ringtone"] = bbpim::AttributeSubKind::SoundRingtone;
    _attributeSubKindMap["note"] = bbpim::AttributeSubKind::Invalid;
}

void PimContactsQt::createKindAttributeMap() {
    _kindAttributeMap[bbpim::AttributeKind::Phone] = "phoneNumbers";
    _kindAttributeMap[bbpim::AttributeKind::Fax] = "faxNumbers";
    _kindAttributeMap[bbpim::AttributeKind::Pager] = "pagerNumber";
    _kindAttributeMap[bbpim::AttributeKind::Email] = "emails";
    _kindAttributeMap[bbpim::AttributeKind::Website] = "urls";
    _kindAttributeMap[bbpim::AttributeKind::Profile] = "socialNetworks";
    _kindAttributeMap[bbpim::AttributeKind::OrganizationAffiliation] = "organizations";
    _kindAttributeMap[bbpim::AttributeKind::Education] = "education";
    _kindAttributeMap[bbpim::AttributeKind::Note] = "note";
    _kindAttributeMap[bbpim::AttributeKind::InstantMessaging] = "ims";
    _kindAttributeMap[bbpim::AttributeKind::VideoChat] = "videoChat";
    _kindAttributeMap[bbpim::AttributeKind::Sound] = "ringtone";
    _kindAttributeMap[bbpim::AttributeKind::Website] = "urls";
}

void PimContactsQt::createSubKindAttributeMap() {
    _subKindAttributeMap[bbpim::AttributeSubKind::Other] = "other";
    _subKindAttributeMap[bbpim::AttributeSubKind::Home] = "home";
    _subKindAttributeMap[bbpim::AttributeSubKind::Work] = "work";
    _subKindAttributeMap[bbpim::AttributeSubKind::PhoneMobile] = "mobile";
    _subKindAttributeMap[bbpim::AttributeSubKind::FaxDirect] = "direct";
    _subKindAttributeMap[bbpim::AttributeSubKind::Blog] = "blog";
    _subKindAttributeMap[bbpim::AttributeSubKind::WebsiteResume] = "resume";
    _subKindAttributeMap[bbpim::AttributeSubKind::WebsitePortfolio] = "portfolio";
    _subKindAttributeMap[bbpim::AttributeSubKind::WebsitePersonal] = "personal";
    _subKindAttributeMap[bbpim::AttributeSubKind::WebsiteCompany] = "company";
    _subKindAttributeMap[bbpim::AttributeSubKind::ProfileFacebook] = "facebook";
    _subKindAttributeMap[bbpim::AttributeSubKind::ProfileTwitter] = "twitter";
    _subKindAttributeMap[bbpim::AttributeSubKind::ProfileLinkedIn] = "linkedin";
    _subKindAttributeMap[bbpim::AttributeSubKind::ProfileGist] = "gist";
    _subKindAttributeMap[bbpim::AttributeSubKind::ProfileTungle] = "tungle";
    _subKindAttributeMap[bbpim::AttributeSubKind::DateBirthday] = "birthday";
    _subKindAttributeMap[bbpim::AttributeSubKind::DateAnniversary] = "anniversary";
    _subKindAttributeMap[bbpim::AttributeSubKind::NameGiven] = "givenName";
    _subKindAttributeMap[bbpim::AttributeSubKind::NameSurname] = "familyName";
    _subKindAttributeMap[bbpim::AttributeSubKind::Title] = "honorificPrefix";
    _subKindAttributeMap[bbpim::AttributeSubKind::NameSuffix] = "honorificSuffix";
    _subKindAttributeMap[bbpim::AttributeSubKind::NameMiddle] = "middleName";
    _subKindAttributeMap[bbpim::AttributeSubKind::NamePhoneticGiven] = "phoneticGivenName";
    _subKindAttributeMap[bbpim::AttributeSubKind::NamePhoneticSurname] = "phoneticFamilyName";
    _subKindAttributeMap[bbpim::AttributeSubKind::NameNickname] = "nickname";
    _subKindAttributeMap[bbpim::AttributeSubKind::NameDisplayName] = "displayName";
    _subKindAttributeMap[bbpim::AttributeSubKind::OrganizationAffiliationName] = "name";
    _subKindAttributeMap[bbpim::AttributeSubKind::OrganizationAffiliationDetails] = "department";
    _subKindAttributeMap[bbpim::AttributeSubKind::Title] = "title";
    _subKindAttributeMap[bbpim::AttributeSubKind::InstantMessagingBbmPin] = "BbmPin";
    _subKindAttributeMap[bbpim::AttributeSubKind::InstantMessagingAim] = "Aim";
    _subKindAttributeMap[bbpim::AttributeSubKind::InstantMessagingAliwangwang] = "Aliwangwang";
    _subKindAttributeMap[bbpim::AttributeSubKind::InstantMessagingGoogleTalk] = "GoogleTalk";
    _subKindAttributeMap[bbpim::AttributeSubKind::InstantMessagingSametime] = "Sametime";
    _subKindAttributeMap[bbpim::AttributeSubKind::InstantMessagingIcq] = "Icq";
    _subKindAttributeMap[bbpim::AttributeSubKind::InstantMessagingJabber] = "Jabber";
    _subKindAttributeMap[bbpim::AttributeSubKind::InstantMessagingMsLcs] = "MsLcs";
    _subKindAttributeMap[bbpim::AttributeSubKind::InstantMessagingSkype] = "Skype";
    _subKindAttributeMap[bbpim::AttributeSubKind::InstantMessagingYahooMessenger] = "YahooMessenger";
    _subKindAttributeMap[bbpim::AttributeSubKind::InstantMessagingYahooMessengerJapan] = "YahooMessegerJapan";
    _subKindAttributeMap[bbpim::AttributeSubKind::VideoChatBbPlaybook] = "BbPlaybook";
    _subKindAttributeMap[bbpim::AttributeSubKind::SoundRingtone] = "ringtone";
}
*/
} // namespace webworks

