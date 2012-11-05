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
#include <json/writer.h>
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
#include <time.h>
#include <QVariantMap>
#include <QVariant>
#include <QStringList>
#include <QPair>
#include <QSet>
#include <QMap>
#include <QtAlgorithms>
#include <string>
#include <sstream>
#include <utility>
#include <map>
#include <algorithm>

#include "pim_calendar_qt.hpp"
#include "timezone_utils.hpp"

namespace webworks {


ServiceProvider PimCalendarQt::_provider = ServiceProvider();
AccountFolderManager PimCalendarQt::_mgr = AccountFolderManager();
pthread_mutex_t PimCalendarQt::_lock = PTHREAD_MUTEX_INITIALIZER;

PimCalendarQt::PimCalendarQt() /*: _mgr(AccountFolderManager())*/
{
}

PimCalendarQt::~PimCalendarQt()
{
}

int PimCalendarQt::MUTEX_LOCK()
{
    struct timespec abs_time;
    clock_gettime(CLOCK_REALTIME , &abs_time);
    abs_time.tv_sec += 7;
    return pthread_mutex_timedlock (&_lock, &abs_time);
    //return pthread_mutex_trylock(&_lock);
}

int PimCalendarQt::MUTEX_UNLOCK()
{
    return pthread_mutex_unlock(&_lock);
}

/****************************************************************
 * Public Functions
 ****************************************************************/

Json::Value PimCalendarQt::Find(const Json::Value& args)
{
    Json::Value returnObj;
    bbpim::EventSearchParameters searchParams;
    bool isParamsValid = getSearchParams(searchParams, args);
    bbpim::Result::Type result;
    int rc = 0;

    if (isParamsValid) {
        //_allFoldersMap.clear();
        //_foldersMap.clear();
        QList<bbpim::CalendarEvent> events;

        if ((rc = MUTEX_LOCK()) == 0) {
            bbpim::CalendarService* service = getCalendarService();
            events = service->events(searchParams, &result);
            MUTEX_UNLOCK();
        }

        if (rc != 0 || result == bbpim::Result::BackEndError) {
            returnObj["_success"] = false;
            returnObj["code"] = UNKNOWN_ERROR;
            return returnObj;
        }

        Json::Value searchResults;
        Json::Value folders;
        std::map<std::string, bbpim::CalendarFolder> localMap;

        for (QList<bbpim::CalendarEvent>::const_iterator i = events.constBegin(); i != events.constEnd(); i++) {
            bbpim::CalendarEvent event = *i;
            //lookupCalendarFolderByFolderKey(event.accountId(), event.folderId());
            bbpim::CalendarFolder folder = _mgr.GetFolder(event.accountId(), event.folderId());
            std::string key = _mgr.GetFolderKey(event.accountId(), event.folderId());
            localMap.insert(std::pair<std::string, bbpim::CalendarFolder>(key, folder));
            searchResults.append(populateEvent(event, true));
        }

        for (std::map<std::string, bbpim::CalendarFolder>::const_iterator j = localMap.begin(); j != localMap.end(); j++) {
            std::string key = j->first;
            bbpim::CalendarFolder folder = j->second;
            folders[key] = _mgr.GetFolderJson(folder);
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
    Json::Value returnObj;

    fprintf(stderr, "#### PimCalendarQt::Save: Starting");
    if (!attributeObj.isMember("id") || attributeObj["id"].isNull() || !attributeObj["id"].isInt()) {
        returnObj = CreateCalendarEvent(attributeObj);
    } else {
        int eventId = attributeObj["id"].asInt();
        int accountId = attributeObj["accountId"].asInt();
        bbpim::Result::Type result;
        bbpim::CalendarService* service = getCalendarService();
	    if (MUTEX_LOCK() == 0) {
            bbpim::CalendarEvent event = service->event(accountId, eventId, &result);
            MUTEX_UNLOCK();

            if (result == bbpim::Result::Success && event.isValid()) {
                returnObj = EditCalendarEvent(event, attributeObj);
            }
        } else {
            returnObj["_success"] = false;
            returnObj["code"] = UNKNOWN_ERROR;
        }
    }

    if (returnObj.empty()) {
        returnObj["_success"] = false;
        returnObj["code"] = UNKNOWN_ERROR; //INVALID_ARGUMENT_ERROR;
    }

    return returnObj;
}

Json::Value PimCalendarQt::GetCalendarFolders()
{
    Json::Value folders;
    QList<bbpim::CalendarFolder> folderList =  _mgr.GetFolders();

    for (int i = 0; i < folderList.size(); i++) {
        folders.append(_mgr.GetFolderJson(folderList[i]));
    }

    return folders;
}

Json::Value PimCalendarQt::GetDefaultCalendarFolder()
{
    Json::Value defaultFolder;

    defaultFolder = _mgr.GetFolderJson(_mgr.GetDefaultFolder());

    return defaultFolder;
}

Json::Value PimCalendarQt::GetCalendarAccounts()
{
    Json::Value accounts;
    const QList<bbpimAccount::Account>accountList = _mgr.GetAccounts();
    for (int i = 0; i < accountList.size(); i++) {
        accounts.append(_mgr.GetAccountJson(accountList[i]));
    }

    return accounts;
}

Json::Value PimCalendarQt::GetDefaultCalendarAccount()
{
    Json::Value defaultAccount;
//    bbpimAccount::Account defaultAccount = bbpimAccount::AccountService().defaultAccount(bbpimAccount::Service::Calendars);
    defaultAccount = _mgr.GetAccountJson(_mgr.GetDefaultAccount());
    return defaultAccount;
}

Json::Value PimCalendarQt::GetEvent(const Json::Value& args)
{
    Json::Value returnObj;
    Json::Value searchResult;

    if (args.isMember("eventId") && args.isMember("accountId")) {
        try {
            bbpim::CalendarService* service = getCalendarService();
            bbpim::CalendarEvent event;
            bbpim::Result::Type result;
            bbpim::AccountId accountId = strToInt(args["accountId"].asString());
            bbpim::EventId eventId = strToInt(args["eventId"].asString());
fprintf(stderr, "DEBUG: PimCalendarQt::GetEvent(): 0\n");
            // if (MUTEX_LOCK()) {
fprintf(stderr, "DEBUG: PimCalendarQt::GetEvent(): 1\n");
                event = service->event(accountId, eventId, &result);
                // MUTEX_UNLOCK();
fprintf(stderr, "DEBUG: PimCalendarQt::GetEvent(): 2\n");
                if (result == bbpim::Result::Success) {
                    returnObj["_success"] = true;
                    returnObj["event"] = populateEvent(event, false);
                }
            // } 
        } catch(int e) {
            returnObj["_success"] = false;
            returnObj["code"] = INVALID_ARGUMENT_ERROR;
            return returnObj;
        }
    } else {
        returnObj["_success"] = false;
        returnObj["code"] = INVALID_ARGUMENT_ERROR;
    }

    if (returnObj.empty()) {
        returnObj["_success"] = false;
        returnObj["code"] = UNKNOWN_ERROR;
    }

    return returnObj;
}

Json::Value PimCalendarQt::CreateCalendarEvent(const Json::Value& args)
{
    Json::Value returnObj;

    bbpim::CalendarEvent ev;
    QList<QDateTime> exceptionDates;

    fprintf(stderr, "Begin create!%s\n", "");

    if (args.isMember("accountId") && args["accountId"].isInt() &&
        args.isMember("folderId") && args["folderId"].isInt()) {
        ev.setAccountId(args["accountId"].asInt());
        ev.setFolderId(args["folderId"].asInt());
    } else {
        // no account id and folder id specified from JS, makes event goes to default calendar
        bbpimAccount::Account defaultCalAccnt = _mgr.GetDefaultAccount();

        if (defaultCalAccnt.isValid()) {
            ev.setAccountId(defaultCalAccnt.id());
            //ev.setFolderId(intToFolderId(accountService->getDefault(bbpimAccount::Service::Calendars)));
            ev.setFolderId(_mgr.GetDefaultFolder().id());
        } else {
            // if it reaches here, it means the app does not have permission to get the REAL default folder,
            // which is possible if the app is in work perimeter
            QList<bbpim::CalendarFolder> folders = _mgr.GetFolders();

            if (folders.constBegin() != folders.constEnd()) {
                bbpim::CalendarFolder folder = *(folders.constBegin());
                ev.setAccountId(folder.accountId());
                ev.setFolderId(folder.id());
            } else {
                returnObj["_success"] = false;
                returnObj["code"] = UNKNOWN_ERROR;
                return returnObj;
            }
        }
    }

    exceptionDates = setEventFields(ev, args, returnObj);

    if (!returnObj.empty()) {
        return returnObj;
    }

    bbpim::CalendarService *service = getCalendarService();

    if (args.isMember("parentId") && !args["parentId"].isNull() && args["parentId"].asInt() != 0) {
        QString timezone = QString(args["timezone"].asCString());
	    if (MUTEX_LOCK() == 0) {
            bbpim::Result::Type result = service->createRecurrenceException(ev, TimezoneUtils::ConvertToTargetFromUtc(getDate(args["originalStartTime"]), true, timezone));
            MUTEX_UNLOCK();
            if (result != bbpim::Result::Success) {
                returnObj["_success"] = false;
                returnObj["code"] = UNKNOWN_ERROR;
                return returnObj;
            }
        } else {
            returnObj["_success"] = false;
            returnObj["code"] = UNKNOWN_ERROR;
            return returnObj;
        }
    } else {
        fprintf(stderr, "before create!%s\n", "");
	    if (MUTEX_LOCK() == 0) {
            bbpim::Result::Type result = service->createEvent(ev);
            MUTEX_UNLOCK();
            if (result != bbpim::Result::Success) {
                returnObj["_success"] = false;
                returnObj["code"] = UNKNOWN_ERROR;
                return returnObj;
            }
       } else {
            returnObj["_success"] = false;
            returnObj["code"] = UNKNOWN_ERROR;
            return returnObj;
       }
    }

    fprintf(stderr, "after create!%s\n", "");

    for (int i = 0; i < exceptionDates.size(); i++) {
        bbpim::CalendarEvent exceptionEvent;
        fprintf(stderr, "Exception date[%d]=%s\n", i, exceptionDates[i].toString().toStdString().c_str());
        exceptionEvent.setStartTime(exceptionDates[i]);
        exceptionEvent.setId(ev.id());
        exceptionEvent.setAccountId(ev.accountId());
        bbpim::Result::Type result;
	    if (MUTEX_LOCK() == 0) {
            result = service->createRecurrenceExclusion(exceptionEvent);
            MUTEX_UNLOCK();
            if (result == bbpim::Result::BackEndError) {
                returnObj["_success"] = false;
                returnObj["code"] = UNKNOWN_ERROR;
                return returnObj;
            }
        } else {
            returnObj["_success"] = false;
            returnObj["code"] = UNKNOWN_ERROR;
            return returnObj;
        }
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

        if (MUTEX_LOCK() == 0) {
            bbpim::CalendarService* service = getCalendarService();
            bbpim::CalendarEvent event = service->event(accountId, eventId);

            if (event.isValid()) {
                /*
                bbpim::Notification notification;
                notification.setComments(QString("This event was deleted by the WebWorks PIM Calendar API."));
                notification.setNotifyAll(true);
                notification.setAccountId(event.accountId());
                notification.setMessageAccountId(event.accountId());
                */
                if (service->deleteEvent(event/*, notification*/) == bbpim::Result::Success) {
                    returnObj["_success"] = true;
                }
            }

            MUTEX_UNLOCK();
        }
    } else {
        returnObj["_success"] = false;
        returnObj["code"] = INVALID_ARGUMENT_ERROR;
    }

    if (returnObj.empty()) {
        returnObj["_success"] = false;
        returnObj["code"] = UNKNOWN_ERROR;
    }

    return returnObj;
}

Json::Value PimCalendarQt::EditCalendarEvent(bbpim::CalendarEvent& calEvent, const Json::Value& attributeObj)
{
    Json::Value returnObj;
    QList<QDateTime> exceptionDates = setEventFields(calEvent, attributeObj, returnObj);

    if (!returnObj.empty()) {
        return returnObj;
    }

    bbpim::CalendarService* service = getCalendarService();
    if (MUTEX_LOCK() == 0) {
        bbpim::Result::Type result = service->updateEvent(calEvent);
        MUTEX_UNLOCK();
        if (result == bbpim::Result::Success) {
            for (int i = 0; i < exceptionDates.size(); i++) {
                bbpim::CalendarEvent exceptionEvent;
                exceptionEvent.setStartTime(exceptionDates[i]);
                exceptionEvent.setId(calEvent.id());
                exceptionEvent.setAccountId(calEvent.accountId());
                if (MUTEX_LOCK() == 0) { 
                    service->createRecurrenceExclusion(exceptionEvent);
                    MUTEX_UNLOCK();
                }
            }

            if (calEvent.isValid()) {
                returnObj["event"] = populateEvent(calEvent, false);
                returnObj["_success"] = true;
            }
        }
    } 
    if (returnObj.empty()) {
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

int PimCalendarQt::strToInt(const std::string s) {
    int number;

    if (!(std::istringstream(s) >> number)) {
        throw INVALID_ARGUMENT_ERROR;
    }
    return number;
}

Json::Value PimCalendarQt::accountToJson(const bbpimAccount::Account account)
{
    Json::Value accountJson;

    accountJson["id"] = Json::Value(intToStr(account.id()));
    accountJson["name"] = Json::Value(account.displayName().toStdString());
    //accountJson["social"] = Json::Value(account.isSocial());
    accountJson["enterprise"] = Json::Value(account.isEnterprise());

    Json::Value accountFoldersArray;
    bbpim::CalendarService* service = getCalendarService();
    const QList<bbpim::CalendarFolder> folders = service->folders();
    for (int i = 0; i < folders.size(); i++)
    {
        if ( folders[i].accountId() == account.id() ) {
            accountFoldersArray.append(getCalendarFolderJson(folders[i]));
        }
    }
    accountJson["folders"] = accountFoldersArray;
    return accountJson;
}

bbpim::FolderId PimCalendarQt::intToFolderId(const quint32 id) {
  return (id != UNDEFINED_UINT) ? bbpim::FolderId(id) : -1;
}

QVariant PimCalendarQt::getFromMap(QMap<QString, QVariant> map, QStringList keys) {
    QVariant variant;
    QMap<QString, QVariant> currentMap = map;
    QStringList::iterator i;
    for (i = keys.begin(); i != keys.end(); ++i){
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
    
    bbpim::CalendarService* service = getCalendarService();
    bbpim::Result::Type result;
    QList<bbpim::CalendarFolder> folders = service->folders(&result);
    Json::Value returnObj;

    if (result == bbpim::Result::BackEndError) {
        returnObj["_success"] = false;
        returnObj["code"] = UNKNOWN_ERROR;
        return returnObj;
    }

    // populate map that contains all calendar folders
    for (QList<bbpim::CalendarFolder>::const_iterator i = folders.constBegin(); i != folders.constEnd(); i++) {
        bbpim::CalendarFolder folder = *i;

        if (folder.accountId() == accountId && folder.id() == folderId) {
            return getCalendarFolderJson(folder);
        }
    }

    return Json::Value();
    
    /*
    bbpim::CalendarFolder folder = _mgr.GetFolder(accountId, folderId);
    return _mgr.GetFolderJson(folder);
    */
}

void PimCalendarQt::lookupCalendarFolderByFolderKey(bbpim::AccountId accountId, bbpim::FolderId folderId) {
    std::string key = getFolderKeyStr(accountId, folderId);
    QList<bbpim::CalendarFolder> folders;

    if (_allFoldersMap.empty()) {
        if (MUTEX_LOCK() == 0) {
            bbpim::CalendarService* service = getCalendarService();
            folders = service->folders();
            MUTEX_UNLOCK();
        } else {
            return;
        }

        // populate map that contains all calendar folders
        for (QList<bbpim::CalendarFolder>::const_iterator i = folders.constBegin(); i != folders.constEnd(); i++) {
            bbpim::CalendarFolder folder = *i;
            _allFoldersMap.insert(std::pair<std::string, bbpim::CalendarFolder>(getFolderKeyStr(folder.accountId(), folder.id()), folder));
        }
    }

    if (_foldersMap.find(key) == _foldersMap.end()) {
        _foldersMap.insert(FolderPair(key, _allFoldersMap[key]));
    }
}

bool PimCalendarQt::isDefaultCalendarFolder(const bbpim::CalendarFolder& folder) {
    bb::pim::account::AccountService* accountService = getAccountService();
    bb::pim::account::Account defaultCalAccnt = accountService->defaultAccount(bb::pim::account::Service::Calendars);

    return (folder.accountId() == defaultCalAccnt.id() &&
        intToFolderId(accountService->getDefault(bb::pim::account::Service::Calendars)) == folder.id());
}

Json::Value PimCalendarQt::getCalendarFolderJson(const bbpim::CalendarFolder& folder, bool skipDefaultCheck) {
    Json::Value f;
/*
    bb::pim::account::AccountService* accountService = getAccountService();
    bb::pim::account::Account account = accountService->account(folder.accountId());
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
*/
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
    if (args.isMember("options")) {
        Json::Value options = args["options"];
        Json::Value filter = options.isMember("filter") ? options["filter"] : Json::Value();
        QDateTime now = QDateTime::currentDateTime();

        // filter - substring - optional
        if (filter.isMember("substring") && filter["substring"].isString()) {
            searchParams.setPrefix(QString(filter["substring"].asCString()));
        }

        // detail - optional - defaults to Agenda if not set
        if (options.isMember("detail") && options["detail"].isInt()) {
            searchParams.setDetails((bbpim::DetailLevel::Type) options["detail"].asInt());
        } else {
            searchParams.setDetails(bbpim::DetailLevel::Agenda);
        }

        // filter - start - optional
        if (!filter["start"].empty()) {
            QDateTime date = getDate(filter["start"].asCString());
            searchParams.setStart(TimezoneUtils::ConvertToTargetFromUtc(date, false)); // TODO(rtse): check
        } else {
            searchParams.setStart(now.addYears(-100));
        }

        // filter - end - optional
        if (!filter["end"].empty()) {
            QDateTime date = getDate(filter["end"].asCString());
            searchParams.setEnd(TimezoneUtils::ConvertToTargetFromUtc(date, false)); // TODO(rtse): check
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
        if (!options["limit"].empty()) {
            int limit = 0;

            if (options["limit"].isString()) {
                limit = strToInt(options["limit"].asString());
            } else {
                limit = options["limit"].asInt();
            }

            if (limit > 0) {
                searchParams.setLimit(limit);
            }
        }

        return true;
    }

    return false;
}

QList<QDateTime> PimCalendarQt::setEventFields(bbpim::CalendarEvent& ev, const Json::Value& args, Json::Value& returnObj)
{
    QList<QDateTime> exceptionDates;
    QString timezone = "";

    if (args.isMember("timezone") && args["timezone"].isString()) {
        timezone = QString(args["timezone"].asCString());
        ev.setTimezone(timezone);
    }

    QDateTime startTime = getDate(args["start"]);
    QDateTime endTime = getDate(args["end"]);

    ev.setStartTime(TimezoneUtils::ConvertToTargetFromUtc(startTime, false, timezone));
    ev.setEndTime(TimezoneUtils::ConvertToTargetFromUtc(endTime, false, timezone));

    if (args.isMember("allDay") && args["allDay"].isBool()) {
        ev.setAllDay(args["allDay"].asBool());
    } else {
        ev.setAllDay(false); // default to false if not specified
    }

    if (args.isMember("summary") && args["summary"].isString()) {
        ev.setSubject(args["summary"].asCString());
    }

    if (args.isMember("location") && args["location"].isString()) {
        ev.setLocation(args["location"].asCString());
    }

    if (args.isMember("description") && args["description"].isString()) {
        ev.setBody(args["description"].asCString());
    }

    if (args.isMember("transparency") && args["transparency"].isInt()) {
        ev.setBusyStatus(bbpim::BusyStatus::Type(args.get("transparency", bbpim::BusyStatus::Busy).asInt())); // use busy as default, same as calendar app
    }

    if (args.isMember("sensitivity") && args["sensitivity"].isInt()) {
        ev.setSensitivity(bbpim::Sensitivity::Type(args.get("sensitivity", bbpim::Sensitivity::Normal).asInt()));
    }

    if (args.isMember("recurrence") && !args["recurrence"].isNull()) {
        Json::Value recArgs = args["recurrence"];

        if (recArgs["frequency"].isNull()) {
            returnObj["_success"] = false;
            returnObj["code"] = INVALID_ARGUMENT_ERROR;
            //return returnObj;
        }

        bbpim::Recurrence recurrence;
        recurrence.setFrequency(bbpim::Frequency::Type(recArgs["frequency"].asInt()));
        recurrence.setInterval(recArgs.get("interval", 1).asInt());

        if (recArgs.isMember("expires") && !recArgs["expires"].isNull()) {
            recurrence.setUntil(TimezoneUtils::ConvertToTargetFromUtc(getDate(recArgs["expires"]), false, timezone));
        }

        if (recArgs.isMember("numberOfOccurrences") && !recArgs["numberOfOccurrences"].isNull()) {
            recurrence.setNumberOfOccurrences(recArgs["numberOfOccurrences"].asInt());
        }

        recurrence.setDayInWeek(recArgs.get("dayInWeek", 1 << (startTime.date().dayOfWeek()%7)).asInt());
        recurrence.setWeekInMonth(recArgs.get("weekInMonth", (startTime.date().day()/7) + 1).asInt());
        recurrence.setDayInMonth(recArgs.get("dayInMonth", startTime.date().day()).asInt());
        recurrence.setMonthInYear(recArgs.get("monthInYear", startTime.date().month()).asInt());

        // Note: exceptionDates cannot be added manually. They must be added using CalendarService::createRecurrenceExclusion
        for (unsigned int i = 0; i < recArgs["exceptionDates"].size(); i++) {
            exceptionDates.append(TimezoneUtils::ConvertToTargetFromUtc(getDate(recArgs["exceptionDates"][i]), true, timezone));
        }

        if (!recurrence.isValid()) {
            returnObj["_success"] = false;
            returnObj["code"] = UNKNOWN_ERROR;
            //return returnObj;
        }

        ev.setRecurrence(recurrence);
    }

    if (args.isMember("attendees")) {
        for (unsigned int i = 0; i < args["attendees"].size(); i++) {
            bbpim::Attendee attendee;
            Json::Value attArgs = args["attendees"][i];

            attendee.setName(QString(attArgs.get("name", "").asCString()));
            attendee.setEmail(QString(attArgs.get("email", "").asCString()));
            attendee.setType((bbpim::Attendee::Type)(attArgs.get("type", bbpim::Attendee::Host).asInt()));
            attendee.setRole((bbpim::AttendeeRole::Type)(attArgs.get("role", bbpim::AttendeeRole::Chair).asInt()));
            attendee.setStatus((bbpim::AttendeeStatus::Type)(attArgs.get("status", bbpim::AttendeeStatus::Unknown).asInt()));
            attendee.setContactId(attArgs.get("contactId", 0).asInt());
            attendee.setOwner(attArgs.get("owner", false).asBool());

            if (!attendee.isValid()) {
                returnObj["_success"] = false;
                returnObj["code"] = UNKNOWN_ERROR;
                //return returnObj;
            }

            ev.addAttendee(attendee);
        }
    }

    if (args.isMember("parentId") && !args["parentId"].isNull() && args["parentId"].asInt() != 0) {
        // This is a recurrence exception event
        if (!args.isMember("originalStartTime") || args["originalStartTime"].isNull()) {
            returnObj["_success"] = false;
            returnObj["code"] = INVALID_ARGUMENT_ERROR;
        }

        ev.setId(args["parentId"].asInt());
        ev.setStartTime(TimezoneUtils::ConvertToTargetFromUtc(startTime, true, timezone));
        ev.setEndTime(TimezoneUtils::ConvertToTargetFromUtc(endTime, true, timezone));
    }

    return exceptionDates;
}

/****************************************************************
 * Helper functions shared by Find and Save
 ****************************************************************/
bbpim::CalendarService* PimCalendarQt::getCalendarService() {
    return _provider.GetCalendarService();
}

bbpimAccount::AccountService* PimCalendarQt::getAccountService() {
    return _provider.GetAccountService();
}

QDateTime PimCalendarQt::getDate(const Json::Value& arg) {
    return QDateTime::fromString(QString(arg.asCString()), "yyyy-MM-dd'T'hh:mm:ss'.000Z'");
}

std::string PimCalendarQt::getSafeString(const std::string& s) {
    return replaceAll(replaceAll(replaceAll(replaceAll(s), "\n", "\\\\n"), "\r", ""), "\t", "\\\\t");
}

std::string PimCalendarQt::replaceAll(const std::string& s, const std::string& souce, const std::string& target) {
    size_t start = 0;
    std::string temp(s);
    while ((start = temp.find(souce, start)) != std::string::npos) {
        temp.replace(start, souce.length(), target);
        start += target.length();
    }
    return temp;
}

Json::Value PimCalendarQt::populateEvent(const bbpim::CalendarEvent& event, bool isFind)
{
    Json::Value e;

    e["accountId"] = intToStr(event.accountId());
    e["id"] = intToStr(event.id());

    if (!isFind) {
        bbpim::CalendarFolder folder = _mgr.GetFolder(event.accountId(), event.folderId());
        e["folder"] = _mgr.GetFolderJson(folder); //getCalendarFolderByFolderKey(event.accountId(), event.folderId());
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

    e["start"] = QString::number(event.startTime().toUTC().toMSecsSinceEpoch()).toStdString();
    e["end"] = QString::number(event.endTime().toUTC().toMSecsSinceEpoch()).toStdString();

    // sensitivity values (Sensitivity::Type)
    // Normal = 0, To be used for unrestricted events.
    // Personal = 1, Sensitivity value for personal events.
    // Private = 2, Sensitivity level for private events.
    // Confidential = 3, Maximum sensitivity level for events.
    e["sensitivity"] = event.sensitivity();
    e["timezone"] = event.timezone().toStdString();
    e["summary"] = getSafeString(event.subject().toStdString());
    e["description"] = getSafeString(event.body().toStdString());
    e["location"] = getSafeString(event.location().toStdString());
    e["url"] = getSafeString(event.url().toStdString());
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
        a["email"] = getSafeString(attendee.email().toStdString());
        a["name"] = getSafeString(attendee.name().toStdString());
        a["type"] = attendee.type();
        a["role"] = attendee.role();
        a["owner"] = attendee.isOwner();
        a["status"] = attendee.status();

        e["attendees"].append(a);
    }

    if (event.recurrence().isValid()) {
        e["recurrence"] = Json::Value();
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

} // namespace webworks

