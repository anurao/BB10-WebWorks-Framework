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
    UNKNOWN = "unknown";

Object.defineProperty(_self, "type", {
    get: function () {
        var type;

        try {
            type = window.webworks.execSync(_ID, "type");
        } catch (e) {
            type = UNKNOWN;
            console.error(e);
        }

        return type;
    }
});

/*
 * Define constants for type constants
 */

_self.startSensor = function (sensor, options) {
    var args = { "options" : options };
    args.options.sensor = sensor;
    return window.webworks.execAsync(_ID, "startSensor", args);
};

_self.stopSensor = function (sensor) {
    var args = { "options" : { } };
    args.options.sensor = sensor;
    return window.webworks.execAsync(_ID, "stopSensor", args);
};

window.webworks.defineReadOnlyField(_self, "ACCELEROMETER", "accelerometer");
window.webworks.defineReadOnlyField(_self, "MAGNETOMETER", "magnetometer");
window.webworks.defineReadOnlyField(_self, "GYROSCOPE", "gyroscope");
window.webworks.defineReadOnlyField(_self, "COMPASS", "compass");
window.webworks.defineReadOnlyField(_self, "PROXIMITY", "proximity");
window.webworks.defineReadOnlyField(_self, "LIGHT", "light");
window.webworks.defineReadOnlyField(_self, "GRAVITY", "gravity");
window.webworks.defineReadOnlyField(_self, "LINEAR_ACCELERATION", "linear_acceleration");
window.webworks.defineReadOnlyField(_self, "ROTATION_VECTOR", "rotation_vector");
window.webworks.defineReadOnlyField(_self, "ORIENTATION", "orientation");
window.webworks.defineReadOnlyField(_self, "ROTATION_MATRIX", "rotation_matrix");
window.webworks.defineReadOnlyField(_self, "AZIMUTH_PITCH_ROLL", "azimuth_pitch_roll");
window.webworks.defineReadOnlyField(_self, "FACE_DETECT", "face_detect");
window.webworks.defineReadOnlyField(_self, "HOLSTER", "holster");

window.webworks.execAsync(_ID, "registerEvents", null);

module.exports = _self;
