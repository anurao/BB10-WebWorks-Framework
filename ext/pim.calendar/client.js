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

 /*
 *   blackberry.pim.Calendar
 *   Properties:
 *      
 *   Methods:
 */
var _self = {},
    _ID = require("./manifest.json").namespace,
    utils = require("./../../lib/utils"),
    CalendarEvent = require("./CalendarEvent"),
    CalendarError = require("./CalendarError"),
    CalendarFindOptions = require("./CalendarFindOptions"),
    CalendarAccount = require("./CalendarAccount"),
    CalendarFolder = require("./CalendarFolder"),
    CalendarRepeatRule = require("./CalendarRepeatRule"),
    CalendarEventFilter = require("./CalendarEventFilter"),
    Attendee = require("./Attendee"),
    calendarUtils = require("./calendarUtils");

function validateFindArguments(findOptions, onFindSuccess, onFindError) {
    var error = false;

    if (!onFindSuccess || typeof onFindSuccess !== "function" || !findOptions || typeof findOptions.limit !== "number") {
        error = true;
    } else {
        switch (findOptions.detail) {
        case CalendarFindOptions.DETAIL_MONTHLY:
        case CalendarFindOptions.DETAIL_WEEKLY:
        case CalendarFindOptions.DETAIL_FULL:
        case CalendarFindOptions.DETAIL_AGENDA:
            break;
        default:
            error = true;
        }

        if (!error && findOptions.sort && findOptions.sort.forEach) {
            findOptions.sort.forEach(function (s) {
                switch (s.fieldName) {
                case CalendarFindOptions.SORT_FIELD_GUID:
                case CalendarFindOptions.SORT_FIELD_SUMMARY:
                case CalendarFindOptions.SORT_FIELD_LOCATION:
                case CalendarFindOptions.SORT_FIELD_START:
                case CalendarFindOptions.SORT_FIELD_END:
                    break;
                default:
                    error = true;
                }

                if (s.desc === undefined || typeof s.desc !== "boolean") {
                    error = true;
                }
            });
        }

        // filter is optional
        if (!error && findOptions.filter) {
            error = error ? error : findOptions.filter.start && !calendarUtils.isDate(findOptions.filter.start);
            error = error ? error : findOptions.filter.end && !calendarUtils.isDate(findOptions.filter.end);
            error = error ? error : typeof findOptions.filter.prefix !== "string";

            if (!error && findOptions.filter.start && findOptions.filter.end) {
                error = !calendarUtils.isBeforeOrEqual(findOptions.filter.start, findOptions.filter.end);
            }

            if (!error && findOptions.filter.start) {
                findOptions.filter.start = calendarUtils.preprocessDate(findOptions.filter.start);
            }

            if (!error && findOptions.filter.end) {
                findOptions.filter.end = calendarUtils.preprocessDate(findOptions.filter.end);
            }

            if (!error && findOptions.filter.folders) {
                findOptions.filter.folders.forEach(function (folder) {
                    if (!folder || !folder.id || isNaN(parseInt(folder.id, 10)) || !folder.accountId || isNaN(folder.accountId, 10)) {
                        error = true;
                    }
                });
            }
        }
    }

    if (error) {
        calendarUtils.invokeErrorCallback(onFindError, CalendarError.INVALID_ARGUMENT_ERROR);
    }

    return !error;
}

function getFolderKeyList(folders) {
    var folderKeys = [];

    if (folders && folders.forEach) {
        folders.forEach(function (folder) {
            folderKeys.push({
                "id": parseInt(folder.id, 10),
                "accountId": parseInt(folder.accountId, 10)
            });
        });
    }

    return folderKeys;
}

_self.createEvent = function (properties, folder) {
    var args = {},
        key;

    for (key in properties) {
        if (properties.hasOwnProperty(key)) {
            args[key] = properties[key];
        }
    }

    args.folder = folder;

    args.id = null;

    return new CalendarEvent(args);
};

_self.getCalendarAccounts = function () {
    var obj = window.webworks.execSync(_ID, "getCalendarAccounts"),
        accounts = [];
    console.log(obj);
    obj.forEach(function (account) {
        accounts.push(new CalendarAccount(account));
    });
    return accounts;
};

_self.getDefaultCalendarAccount = function () {
    var obj = window.webworks.execSync(_ID, "getDefaultCalendarAccount");
    return new CalendarAccount(obj);

};

_self.getCalendarFolders = function () {
    var obj = window.webworks.execSync(_ID, "getCalendarFolders"),
        folders = [];

    obj.forEach(function (props) {
        folders.push(new CalendarFolder(props));
    });

    return folders;
};

_self.getDefaultCalendarFolder = function () {
    var obj = window.webworks.execSync(_ID, "getDefaultCalendarFolder");
    return new CalendarFolder(obj);
};

_self.getEvent = function (eventId, folder) {
    var obj = window.webworks.execSync(_ID, "getEvent", {
            "eventId": eventId,
            "folder": folder
        });

    return new CalendarEvent(obj);
};

_self.findEvents = function (findOptions, onFindSuccess, onFindError) {
    var callback,
        eventId;

    if (!validateFindArguments(findOptions, onFindSuccess, onFindError)) {
        return;
    }

    if (findOptions.filter) {
        findOptions.filter.folders = getFolderKeyList(findOptions.filter.folders);
    }

    callback = function (args) {
        console.log(unescape(args.result));
        var result = JSON.parse(unescape(args.result)),
            events = result.events,
            realEvents = [];

        if (result._success) {
            if (events) {
                events.forEach(function (event) {
                    event["folder"] = result.folders[event.accountId + "-" + event.folderId];
                    realEvents.push(new CalendarEvent(calendarUtils.populateEvent(event)));
                });
            }
            console.log(events);
            onFindSuccess(realEvents);
        } else {
            calendarUtils.invokeErrorCallback(onFindError, result.code);
        }
    };

    eventId = utils.guid();

    window.webworks.event.once(_ID, eventId, callback);

    return window.webworks.execAsync(_ID, "find", {
        "_eventId": eventId,
        "options": findOptions
    });
};

_self.CalendarEvent = CalendarEvent;
_self.CalendarRepeatRule = CalendarRepeatRule;
_self.CalendarError = CalendarError;
_self.CalendarFindOptions = CalendarFindOptions;
_self.CalendarFolder = CalendarFolder;
_self.CalendarEventFilter = CalendarEventFilter;
_self.Attendee = Attendee;

module.exports = _self;
