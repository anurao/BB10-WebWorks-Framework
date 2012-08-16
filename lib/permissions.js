/*
 *  Copyright 2012 Research In Motion Limited.
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

var permissions,
    utils =  require("./utils"),
    config = require("./config"),
    _webview;

permissions =  {

    init : function (webview) {
        _webview = webview;
    },
/*
    onGeolocationPermissionRequest : function (request) {
        debugger;
        console.log("Geolocation Request");
        var evt = JSON.Parse(request);

        if (config.permissions && utils.arrayContains(config.permissions, "read_geolocation")) {
            _webview.allowGeolocation(evt.origin);
            console.log("Success location is enabled");
        } else {
            _webview.disallowGeolocation(evt.origin);
            console.log("Woops location is disabled");
        }
        return '{"setPreventDefault": true}';
    }

    /*onNotificationPermissionRequest : function (request) {
        console.log("Notification Request");
        var evt = JSON.Parse(request),
            webview = require('./webview');

        if (config.permissions && utils.arrayContains(config.permissions, "post_notification")) {
            webview.setAllowNotification(evt.origin);
            console.log("Success notifications are enabled");
        } else {
            webview.disallowNotification(evt.origin);
            console.log("Woops notifications are disabled");
        }
        return '{"setPreventDefault": true}';
    }*/
};

module.exports = permissions;


