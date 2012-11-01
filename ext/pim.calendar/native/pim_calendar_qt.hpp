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

#ifndef PIM_CALENDAR_QT_HPP_
#define PIM_CALENDAR_QT_HPP_

#include <json/value.h>
#include <pthread.h>
#include <bb/pim/account/Account>
#include <bb/pim/account/AccountService>
#include <bb/pim/calendar/DataTypes>
#include <bb/pim/calendar/CalendarService>
#include <bb/pim/calendar/CalendarEvent>
#include <bb/pim/calendar/FolderKey>
#include <QDateTime>
#include <string>
#include <utility>
#include <map>
#include <limits>

#include "service_provider.hpp"

class PimCalendar;

namespace webworks {

namespace bbpim = bb::pim::calendar;
namespace bbpimAccount = bb::pim::account;

enum PimCalendarError {
    UNKNOWN_ERROR = 0,
    INVALID_ARGUMENT_ERROR = 1,
    TIMEOUT_ERROR = 2,
    PENDING_OPERATION_ERROR = 3,
    IO_ERROR = 4,
    NOT_SUPPORTED_ERROR = 5,
    PERMISSION_DENIED_ERROR = 20,
};

struct PimCalendarThreadInfo {
    PimCalendar *parent;
    Json::Value *jsonObj;
    std::string eventId;
};

typedef std::pair<std::string, bbpim::CalendarFolder> FolderPair;

const quint32 UNDEFINED_UINT = std::numeric_limits<quint32>::max();

class PimCalendarQt {
public:
    PimCalendarQt();
    ~PimCalendarQt();
    Json::Value Find(const Json::Value& args);
    Json::Value GetEvent(const Json::Value& args);
    Json::Value Save(const Json::Value& args);
    Json::Value CreateCalendarEvent(const Json::Value& args);
    Json::Value DeleteCalendarEvent(const Json::Value& args);
    Json::Value EditCalendarEvent(bbpim::CalendarEvent& contact, const Json::Value& attributeObj);
    static Json::Value GetCalendarFolders();
    static Json::Value GetDefaultCalendarFolder();
    static Json::Value GetCalendarAccounts();
    static Json::Value GetDefaultCalendarAccount();

private:
    static std::string intToStr(const int val);
    static int strToInt(const std::string s);
    static bbpim::FolderId intToFolderId(const quint32 id);
    static QDateTime getDate(const Json::Value& arg);
    static QVariant getFromMap(QMap<QString, QVariant> map, QStringList keys);
    static std::string getSafeString(const std::string& s);
    static std::string replaceAll(const std::string& s, const std::string& souce = "\"", const std::string& target = "\\\"");
    static std::string getFolderKeyStr(bbpim::AccountId accountId, bbpim::FolderId folderId);
    static bool getSearchParams(bbpim::EventSearchParameters& searchParams, const Json::Value& args);
    void lookupCalendarFolderByFolderKey(bbpim::AccountId accountId, bbpim::FolderId folderId);
    static bool isDefaultCalendarFolder(const bbpim::CalendarFolder& folder);
    static Json::Value getCalendarFolderJson(const bbpim::CalendarFolder& folder, bool skipDefaultCheck = false);
    static QList<QDateTime> setEventFields(bbpim::CalendarEvent& ev, const Json::Value& args, Json::Value& returnObj);
    static bbpim::CalendarService* getCalendarService();
    static bbpimAccount::AccountService* getAccountService();
    Json::Value populateEvent(const bbpim::CalendarEvent& event, bool isFind);
    Json::Value getCalendarFolderByFolderKey(bbpim::AccountId accountId, bbpim::FolderId folderId);
    static Json::Value accountToJson(const bbpimAccount::Account account);

    std::map<std::string, bbpim::CalendarFolder> _allFoldersMap;
    std::map<std::string, bbpim::CalendarFolder> _foldersMap;

    static ServiceProvider _provider;
    static pthread_mutex_t _lock;
};

} // namespace webworks

#endif // PIM_CALENDAR_QT_HPP_
