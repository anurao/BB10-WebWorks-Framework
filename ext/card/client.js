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

var _self = {},
    _ID = require("./manifest.json").namespace,
    _cameraDoneEventId = "invokeCamera.doneEventId",
    _cameraCancelEventId = "invokeCamera.cancelEventId",
    _cameraInvokeEventId = "invokeCamera.invokeEventId";

_self.invokeCamera = function (mode, done, cancel, invokeCallback) {
    if (!window.webworks.event.isOn(_cameraDoneEventId)) {
        window.webworks.event.once(_ID, _cameraDoneEventId, done);
    }
    if (!window.webworks.event.isOn(_cameraCancelEventId)) {
        window.webworks.event.once(_ID, _cameraCancelEventId, cancel);
    }
    if (!window.webworks.event.isOn(_cameraInvokeEventId)) {
        window.webworks.event.once(_ID, _cameraInvokeEventId, invokeCallback);
    }
    return window.webworks.execAsync(_ID, "invokeCamera", {mode: mode || ""});
};


window.webworks.defineReadOnlyField(_self, "CAMERA_MODE_PHOTO", 'photo');
window.webworks.defineReadOnlyField(_self, "CAMERA_MODE_VIDEO", 'video');
window.webworks.defineReadOnlyField(_self, "CAMERA_MODE_FULL", 'full');

module.exports = _self;
