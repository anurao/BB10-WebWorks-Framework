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

#ifndef PIM_CALENDAR_QT_H_
#define PIM_CALENDAR_QT_H_

#include <json/value.h>
#include <bb/pim/calendar/DataTypes>
#include <bb/pim/calendar/CalendarService>
#include <bb/pim/calendar/CalendarEvent>
#include <bb/pim/calendar/FolderKey>
#include <string>
#include <utility>
#include <map>
#include <limits>

class PimCalendar;

namespace webworks {

namespace bbpim = bb::pim::calendar;
/*
typedef std::map<std::string, bbpim::AttributeKind::Type> StringToKindMap;
typedef std::map<std::string, bbpim::AttributeSubKind::Type> StringToSubKindMap;
typedef std::map<bbpim::AttributeKind::Type, std::string> KindToStringMap;
typedef std::map<bbpim::AttributeSubKind::Type, std::string> SubKindToStringMap;

typedef std::pair<bbpim::AttributeSubKind::Type, std::string> SubkindValuePair;
*/
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

const quint32 UNDEFINED_UINT = std::numeric_limits<quint32>::max();

class PimCalendarQt {
public:
    PimCalendarQt();
    ~PimCalendarQt();
    Json::Value Find(const Json::Value& args);
    Json::Value Save(const Json::Value& args);
    Json::Value CreateCalendarEvent(const Json::Value& args);
    Json::Value DeleteCalendarEvent(const Json::Value& args);
    Json::Value EditCalendarEvent(bbpim::CalendarEvent& contact, const Json::Value& attributeObj);
    Json::Value GetCalendarFolders();
    Json::Value GetDefaultCalendarFolder();
    Json::Value GetTimezones();

private:
    std::string intToStr(const int val);
    bbpim::FolderId intToFolderId(const quint32 id);
    QVariant getFromMap(QMap<QString, QVariant> map, QStringList keys);
    std::string getFolderKeyStr(bbpim::AccountId accountId, bbpim::FolderId folderId);
    bool getSearchParams(bbpim::EventSearchParameters& searchParams, const Json::Value& args);
    void lookupCalendarFolderByFolderKey(bbpim::AccountId accountId, bbpim::FolderId folderId);
    bool isDefaultCalendarFolder(const bbpim::CalendarFolder& folder);
    Json::Value getCalendarFolderJson(const bbpim::CalendarFolder& folder, bool skipDefaultCheck = false);

    Json::Value populateEvent(const bbpim::CalendarEvent& event, bool isFind);
    Json::Value getCalendarFolderByFolderKey(bbpim::AccountId accountId, bbpim::FolderId folderId);

    std::map<std::string, bbpim::CalendarFolder> _allFoldersMap;
    std::map<std::string, bbpim::CalendarFolder> _foldersMap;
/*
    // Helper functions for Find
    Json::Value assembleSearchResults(const QSet<bbpim::ContactId>& results, const Json::Value& contactFields, int limit);
    Json::Value populateContact(bbpim::Contact& contact, const Json::Value& contactFields);
    void populateField(const bbpim::Contact& contact, bbpim::AttributeKind::Type kind, Json::Value& contactItem, bool isContactField, bool isArray);
    void populateDisplayNameNickName(const bbpim::Contact& contact, Json::Value& contactItem, const std::string& field);
    void populateOrganizations(const bbpim::Contact& contact, Json::Value& contactOrgs);
    void populateAddresses(const bbpim::Contact& contact, Json::Value& contactAddrs);
    void populatePhotos(const bbpim::Contact& contact, Json::Value& contactPhotos);

    static QSet<bbpim::ContactId> singleFieldSearch(const Json::Value& searchFieldsJson, const Json::Value& contactFields, bool favorite);
    static QString getSortFieldValue(const bbpim::SortColumn::Type sortField, const bbpim::Contact& contact);
    static QList<bbpim::SearchField::Type> getSearchFields(const Json::Value& searchFieldsJson);
    static void getSortSpecs(const Json::Value& sort);
    static QSet<bbpim::ContactId> getPartialSearchResults(const Json::Value& filter, const Json::Value& contactFields, const bool favorite);
    static bool lessThan(const bbpim::Contact& c1, const bbpim::Contact& c2);

    // Helper functions for Save
    void addAttributeKind(bbpim::ContactBuilder& contactBuilder, const Json::Value& jsonObj, const std::string& field);
    void addPostalAddress(bbpim::ContactBuilder& contactBuilder, const Json::Value& addressObj);
    void addPhoto(bbpim::ContactBuilder& contactBuilder, const Json::Value& photoObj);

    void syncAttributeKind(bbpim::Contact& contact, const Json::Value& jsonObj, const std::string& field);
    void syncConvertedList(bbpim::ContactBuilder& contactBuilder, bbpim::AttributeKind::Type kind, QList<bbpim::ContactAttribute> savedList, const QList<SubkindValuePair>& convertedList);
    void syncConvertedGroupedList(bbpim::ContactBuilder& contactBuilder, bbpim::AttributeKind::Type kind, QList<bbpim::ContactAttribute> savedList, QList<SubkindValuePair> convertedList, const std::string& groupKey);
    void syncAttributeGroup(bbpim::ContactBuilder& contactBuilder, bbpim::AttributeKind::Type kind, QList<QList<bbpim::ContactAttribute> > savedList, const Json::Value& jsonObj);
    void syncAttributeDate(bbpim::ContactBuilder& contactBuilder, QList<bbpim::ContactAttribute>& savedList, const bbpim::AttributeSubKind::Type subkind, const std::string& value);
    void syncPostalAddresses(bbpim::ContactBuilder& contactBuilder, QList<bbpim::ContactPostalAddress>& savedList, const Json::Value& jsonObj);
    void syncPhotos(bbpim::ContactBuilder& contactBuilder, QList<bbpim::ContactPhoto>& savedList, const Json::Value& jsonObj);

    void addConvertedList(bbpim::ContactBuilder& contactBuilder, const bbpim::AttributeKind::Type kind, const QList<SubkindValuePair>& convertedList);
    void addConvertedGroupedList(bbpim::ContactBuilder& contactBuilder, const bbpim::AttributeKind::Type kind, const QList<SubkindValuePair>& convertedList, const std::string& groupKey);
    void addAttributeDate(bbpim::ContactBuilder& contactBuilder, const bbpim::AttributeKind::Type kind, const bbpim::AttributeSubKind::Type subkind, const std::string& value);
//    void addAttribute(bbpim::ContactBuilder& contactBuilder, const bbpim::AttributeKind::Type kind, const bbpim::AttributeSubKind::Type subkind, const std::string& value);
//    void addAttributeToGroup(bbpim::ContactBuilder& contactBuilder, const bbpim::AttributeKind::Type kind, const bbpim::AttributeSubKind::Type subkind, const std::string& value, const std::string& groupKey);

    QList<SubkindValuePair> convertGroupedAttributes(const Json::Value& fieldsObj);
    QList<SubkindValuePair> convertFieldAttributes(const Json::Value& fieldArray);
    QList<SubkindValuePair> convertStringArray(const Json::Value& stringArray, bbpim::AttributeSubKind::Type subkind);

    // Mappings between JSON strings and attribute kinds/subkinds
    static void createAttributeKindMap();
    static void createAttributeSubKindMap();
    static void createKindAttributeMap();
    static void createSubKindAttributeMap();

    static StringToKindMap _attributeKindMap;
    static StringToSubKindMap _attributeSubKindMap;
    static KindToStringMap _kindAttributeMap;
    static SubKindToStringMap _subKindAttributeMap;
    static QList<bbpim::SortSpecifier> _sortSpecs;

    static std::map<bbpim::ContactId, bbpim::Contact> _contactSearchMap;
*/
};

} // namespace webworks

#endif // PIM_CALENDAR_QT_H_
