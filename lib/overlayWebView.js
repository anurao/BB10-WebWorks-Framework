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

var request = require('./request'),
    CHROME_HEIGHT = 0,
    webview,
    _webviewObj;

webview =
    {

    create: function (ready, configSettings) {

        _webviewObj = window.qnx.webplatform.createWebView(function () {

            _webviewObj.visible = true;
            _webviewObj.active = true;
            _webviewObj.zOrder = 1;
            _webviewObj.enableCrossSiteXHR = true;
            _webviewObj.setGeometry(0, 0, screen.width, screen.height);
            _webviewObj.addEventListener("DocumentLoadFinished", function () {
                _webviewObj.visible = true;
            });

            _webviewObj.enableWebEventRedirect("JavaScriptCallback", 1);
            _webviewObj.backgroundColor = 0x00FFFFFF;
            _webviewObj.sensitivity = "SensitivityTest";
            _webviewObj.devicePixelRatio = 1;
            _webviewObj.allowQnxObject = true;

            if (ready && typeof ready === 'function') {
                ready();
            }
        });
    },

    id: function () {
        return _webviewObj.id;
    },

    destroy: function () {
        _webviewObj.destroy();
    },

    setURL: function (url) {
        _webviewObj.url = url;
    },

    setGeometry: function (x, y, width, height) {
        _webviewObj.setGeometry(x, y, width, height);
    },

    setSensitivity : function (sensitivity) {
        _webviewObj.sensitivity = sensitivity;
    },

    setApplicationOrientation: function (angle) {
        _webviewObj.setApplicationOrientation(angle);
    },

    notifyApplicationOrientationDone: function () {
        _webviewObj.notifyApplicationOrientationDone();
    },

    executeJavascript: function (js) {
        _webviewObj.executeJavaScript(js);
    },

    windowGroup: function () {
        return _webviewObj.windowGroup;
    },

    notifyContextMenuCancelled: function () {
        _webviewObj.notifyContextMenuCancelled();
    }
};

webview.__defineSetter__('onPropertyCurrentContextEvent', function (input) {
    _webviewObj.onPropertyCurrentContextEvent = input;
});

webview.__defineSetter__('onContextMenuRequestEvent', function (input) {
    _webviewObj.onContextMenuRequestEvent = input;
});

module.exports = webview;
