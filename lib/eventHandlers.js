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

var _i18n,
    _event,
    _webview,
    _invocation,
    _controller,
    eventHandlers,
    _pendingRequest,
    _overlayWebView = require("./overlayWebView"),
    _utils =  require("./utils"),
    _config = require("./config"),
    NETWORK_ERROR = "NetworkError",

eventHandlers =  {

    init : function (webview) {
        _webview = webview;
        _controller = window.qnx.webplatform.getController();
        _invocation = window.qnx.webplatform.getApplication().invocation;
        _i18n = window.qnx.webplatform.i18n;
    },

    /*
     * Catch the network errors, this gets tricky since
     * we actually need to check a couple conditions
     * on the client webview before we can determine the action
     * to take:
     *  - If the URL is local, then just trigger a NetworkError on the client
     *  - If the URL is remote, but NOT stored in the appCache, then show dialog
     *  - If it is in appCache, then trust the developer that they properly
     *    built the app cache manifest.
     */
    onNetworkError : function (request) {
        var requestObj = JSON.parse(request);
        _event = require("./event");
        if (requestObj.type === NETWORK_ERROR) {
            if (_webview.currentLocation === undefined && _utils.isRemote(requestObj.url)) {
                // Undefined as initial load
                _webview.executeJavascript("var xhr = new XMLHttpRequest(); xhr.open('GET', 'http://localhost:8472/networkError/response/caching/?cached=' + !(window.applicationCache.status === 0), false); xhr.send();");
                _pendingRequest = requestObj;
            } else {
                _event.trigger('NetworkError', requestObj);
            }
        }

        return '{"setPreventDefault": true, "setResult": "true"}';
    },

    networkCacheResponse : function (cached) {
        var msgObj;
        if (_pendingRequest) {
            if (JSON.parse(cached)) {
                _event.trigger('NetworkError', _pendingRequest);
                _pendingRequest = null;
            } else {
                msgObj = {
                    title : _i18n.translate('Network Error').fetch(),
                    cancellabel : _i18n.translate('No').fetch(),
                    oklabel : _i18n.translate('Try Again').fetch(),
                    thirdOptionLabel : _i18n.translate('Open Settings').fetch(),
                    dialogType : 'JavaScriptConfirm',
                    htmlmessage : _i18n.translate('The network is currently unavailable, ' +
                              'please turn on your internet settings or try again.').fetch()
                };

                _overlayWebView.showDialog(msgObj, function (result) {
                    if (result.ok) {
                        // Try again :)
                        debugger;
                        _webview.setURL(_pendingRequest.url);
                    } else if (result.thirdOption) {
                        _invocation.invoke({
                            action : "bb.action.OPEN",
                            uri : "settings://networkresources"
                        });
                    }
                    // Default is just to swallow it

                    _pendingRequest = null;
                });
            }

        }
    },

    onGeolocationPermissionRequest : function (request) {
        var evt = JSON.parse(request);
        if (_config.permissions && _utils.arrayContains(_config.permissions, "read_geolocation")) {
            _webview.allowGeolocation(evt.origin);
        } else {
            _webview.disallowGeolocation(evt.origin);
        }
        return '{"setPreventDefault": true}';
    }
};

module.exports = eventHandlers;


