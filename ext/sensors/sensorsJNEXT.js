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

var sensors,
    triggerCallback = null;

///////////////////////////////////////////////////////////////////
// JavaScript wrapper for JNEXT plugin for connection
///////////////////////////////////////////////////////////////////

JNEXT.Sensors = function () {
    var self = this;

    self.startEvents = function (trigger) {
        triggerCallback = trigger;
        JNEXT.invoke(self.m_id, "startEvents");
    };

    self.stopEvents = function () {
        JNEXT.invoke(self.m_id, "stopEvents");
        triggerCallback = null;
    };

    self.startSensor = function (config) {
        JNEXT.invoke(self.m_id, "startSensor " + JSON.stringify(config));
    };

    self.stopSensor = function (sensor) {
        JNEXT.invoke(self.m_id, "stopSensor " + sensor);
    };

    self.onEvent = function (strData) {
        var arData = strData.split(" "),
            strEventDesc = arData[0],
            jsonData;
        
        if (strEventDesc === "onsensor") {
            jsonData = arData.slice(2, arData.length).join(" ");
            triggerCallback(arData[1], JSON.parse(jsonData));
        }
    };

    self.getId = function () {
        return self.m_id;
    };

    self.init = function () {
        if (!JNEXT.require("libsensors")) {
            return false;
        }

        self.m_id = JNEXT.createObject("libsensors.Sensors");

        if (self.m_id === "") {
            return false;
        }

        JNEXT.registerEvents(self);
    };

    self.m_id = "";

    self.init();
};

sensors = new JNEXT.Sensors();

module.exports = {
    sensors: sensors
};
