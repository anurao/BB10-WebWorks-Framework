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
            if (config.enableFlash) {
                //Set webview plugin directory [required for flash]
                webview.setExtraPluginDirectory('/usr/lib/browser/plugins');
            }

            if (config.enablePatternMatching) {
                // Enable pattern matching
                webview.setPatternMatching(true);
                webview.onLocationChanging = function (id, value, eventId, returnValue) {
                    // Sample value: //  {"isModal":false,"isPost":true,"isSend":false,"request":"Unsupported type: QNXWebInvokeRequest const*","type":"InvokeRequestEvent","uri":"http://www.google.com"} 
                    var obj = JSON.parse(value),
                        request,
                        urlData,
                        webkitProtocols = [ 'http:', 'https:', 'file:', 'ftp:', 'local:', 'data:', 'javascript:', 'vs:', 'about:', 'blob:', 'ws:', 'wss:', 'filesystem:', 'invoke:', 'platform:'],
                        i;

                    // Don't start a new browser, etc. for pattern matched webkit links
                    for (i in webkitProtocols) {
                        if (obj.url.indexOf(webkitProtocols[i]) === 0) {
                            returnValue.setPreventDefault = false;  // Let webkit handle it.
                            return;
                        }
                    }

                    // Sample url: invoke://1,default:0,eyJ0eXBlIjoiYXBwbGljYXRpb24vdm5kLmJsYWNrYmVycnkuc3RyaW5nLnBob25lIiwiZGF0YSI6Ik5ERTJMVEV5TXkweE1qTTBBQT09In0=
                    if (obj.url.indexOf('invoke:') === 0) {
                        urlData = obj.url.match(/invoke\:\/\/.*,.*\:(.*)/i)[1];
                        request = atob(urlData); 
                        console.log("request", request);
                    } else {
                        request = {
                            action: 'bb.action.OPEN',
                            uri: obj.url
                        };
                    }

                    // We need to check the custom-registered scheme.
                    // FIXME: Need to check the custom scheme here. PR: 160543, but for now, we will just let "web+" to go through.
                    if (obj.url.indexOf('web+') === 0) {
                        returnValue.setPreventDefault = false;
                        return;
                    }

                    window.qnx.webplatform.getApplication().invocation.invoke(request, function (error) {
                        if (error) {
                            console.log("error:" + error);
                            console.log("request", request);
                            console.log("object", obj);
                        } else {
                            console.log("Success");
                        }
                    });

                    returnValue.setPreventDefault = true;
                };
            }

            rotationHelper.addWebview(webview);

            // Workaround for executeJavascript doing nothing for the first time
            webview.executeJavascript("1 + 1");

            url = url || config.content;
            // Start page
            if (url) {
                webview.setURL(url);
            }
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
