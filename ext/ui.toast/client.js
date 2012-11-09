/*
 * Copyright 2010-2011 Research In Motion Limited.
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
var toast = {},
    ID = require("./manifest.json").namespace,
    _listeningForCallbacks = false,
    _storedButtonCallbacks = {},
    _storedDismissCallbacks = {};

function listenForButtonCallback() {
    window.webworks.event.add("blackberry.event", 'toast.callback', function (toastId) {
        _storedButtonCallbacks[toastId]();
        delete _storedButtonCallbacks[toastId];
    });
}

function listenForDismissCallback() {
    window.webworks.event.add("blackberry.event", 'toast.dismiss', function (toastId) {
        _storedDismissCallbacks[toastId]();
        delete _storedDismissCallbacks[toastId];
    });
}

toast.show = function (message, buttonText, buttonCallback, dismissCallback) {
    var toastId = window.webworks.execSync(ID, 'show', {message: message, buttonText: buttonText});

    if (buttonCallback) {
        _storedButtonCallbacks[toastId] = buttonCallback;
        listenForButtonCallback(toastId);
    }
    if (dismissCallback) {
        _storedDismissCallbacks[toastId] = dismissCallback;
        listenForDismissCallback(toastId);
    }

    return toastId;
};

module.exports = toast;
