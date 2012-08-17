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


module.exports = {

    MODE_PICKER: 'Picker',
    MODE_SAVER: 'Saver',
    MODE_PICKER_MULTIPLE: 'PickerMultiple',

    SELECT_MODE_SINGLE: 'SingleSelect',
    SELECT_MODE_MULTIPLE: 'MultipleSelect',

    VIEWER_MODE_LIST: 'ListView',
    VIEWER_MODE_GRID: 'GridView',

    SORT_BY_NAME: 'Name',
    SORT_BY_DATE: 'Date',
    SORT_BY_SUFFIX: 'suffix',
    SORT_BY_SIZE: 'Size',

    SORT_ORDER_ASCENDING: 'Ascending',
    SORT_ORDER_DESCENDING: 'Descending',

    TYPE_PICTURE: 'picture',
    TYPE_DOCUMENT: 'document',
    TYPE_MUSIC: 'music',
    TYPE_VIDEO: 'video',
    TYPE_OTHER: 'other',

    open: function (details, success, fail) {
        var application = window.qnx.webplatform.getApplication(),
            data = {
                Mode: details.mode,
                SelectMode: details.selectMode,
                DefaultType: details.defaultType,
                Filter: details.filter,
                Directory: details.directory,
                DefaultSaveFileName: details.defaultSaveFileName,
                Title: details.title,
                ViewMode: details.viewMode,
                SortBy: details.sortBy,
                SortOrder: details.sortOrder,
                ImageCrop: details.imageCropEnabled
            },
            encodedData;
        window.qnx.webplatform.getApplication().invocation.addEventListener("childCardClosed", function (info) {
            if (info.reason === "save") {
                var result = info.data,
                    path;
                if (info.data.match("^dat:json")) { //strip off dat:json
                    result = info.data.slice(9);
                }
                path = JSON.parse(result)[0].uri;
                if (path.match("^file:///")) { //strip off file://
                    path = path.slice(7);
                }
                success(path);
            } else if (info.reason === "cancel") {
                fail(info.reason);
            }
        });

        if (details.type) {
            data.Type = details.type.join(',');
        } else {
            data.Type = [];
        }
        encodedData = require("./../pps/ppsUtils").ppsEncodeObject(data);

        application.invocation.invoke({
            action: "bb.action.OPEN",
            target: "sys.filepicker.target",
            data: window.btoa(encodedData)
        }, function (error) {
            if (error) {
                console.log("error:" + error);
            } else {
                console.log("success");
            }

        });

    }
};

