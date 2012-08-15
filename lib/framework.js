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

var utils = require('./utils'),
    controllerWebView = require('./controllerWebView'),
    webview = require('./webview'),
    overlayWebView = require('./overlayWebView'),
    network = require("../lib/pps/ppsNetwork"),
    config = require("./config"),
    dialog = require("./ui/dialog/index");

function showWebInspectorInfo() {
    var port = window.qnx.webplatform.getApplication().webInspectorPort,
        messageObj = {};

    network.getNetworkInfo(function (ipAddresses) {
        messageObj.title = "Web Inspector Enabled";
        if (ipAddresses) {
            messageObj.htmlmessage =  "\n ip4:    " + ipAddresses.ipv4Address + ":" + port + "<br/> ip6:    " + ipAddresses.ipv6Address + ":" + port;
        } else {
            messageObj.message = "";
        }
        messageObj.dialogType = 'JavaScriptAlert';
        dialog.show(messageObj);
    });
}

var _self = {
    start: function (url) {
        var rotationHelper = require('./rotationHelper'),
            callback;

        // Set up the controller WebView
        controllerWebView.init(config);

        rotationHelper.addWebview(controllerWebView);

        webview.create(function () {
            rotationHelper.addWebview(webview);

            // Workaround for executeJavascript doing nothing for the first time
            webview.executeJavascript("1 + 1");

            url = url || config.content;
            // Start page
            if (url) {
                webview.setURL(url);
            }

            webview.onGeolocationPermissionRequest = function (request) {
                if (config.permissions && utils.arrayContains(config.permissions, "read_geolocation")) {
                    //webview.setAllowGeolocation(request.origin, true);
                    qnx.callExtensionMethod("webview.setAllowGeolocation", 2, request.origin, true);
                    console.log("Success location is enabled");
                } else {
                    webview.setAllowGeolocation(request.origin, false);
                    console.log("Woops location is disabled");
                }
                return '{"setPreventDefault": true}';
            };
        },
        {
            debugEnabled : config.debugEnabled
        });

        //if debugging is enabled, show the IP and port for webinspector
        if (config.debugEnabled) {
            callback = function () {
                showWebInspectorInfo();

                //Remove listener. Alert should only be shown once.
                webview.removeEventListener("DocumentLoadFinished", callback);
            };

            webview.addEventListener("DocumentLoadFinished", callback);
        }

        overlayWebView.create(function () {
            rotationHelper.addWebview(overlayWebView);

            overlayWebView.setURL("local:///ui-resources/ui.html");
            controllerWebView.dispatchEvent('ui.init', null);
        });
    },
    stop: function () {
        webview.destroy();
    }
};

module.exports = _self;
