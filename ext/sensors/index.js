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

var sensors = require("./sensorsJNEXT").sensors,
    _event = require("../../lib/event"),
    _utils = require("../../lib/utils"),
    _actionMap = {
        onsensor: {
            context: require("./sensorsEvents"),
            event: "onsensor",
            trigger: function (args, obj) {
                _event.trigger("onsensor", args, obj);
            }
        }
    };

module.exports = {
    registerEvents: function (success, fail, args, env) {
        try {
            var _eventExt = _utils.loadExtensionModule("event", "index");
            _eventExt.registerEvents(_actionMap);
            success();
        } catch (e) {
            fail(-1, e);
        }
    },

    startSensor: function (success, fail, args) {
        if (args.options) {
            args.options = JSON.parse(decodeURIComponent(args.options));

            if (args.options.sensor === "") {
                fail(-1, "Must specify a sensor");
            }
            success(sensors.startSensor(args.options));
        } else {
            fail(-1, "Need to specify arguments");
        }
    },

    stopSensor: function (success, fail, args) {
        if (args.options) {
            args.options = JSON.parse(decodeURIComponent(args.options));

            if (args.options.sensor === "") {
                fail(-1, "Must specify a sensor");
            }
            success(sensors.stopSensor(args.options.sensor));
        } else {
            fail(-1, "Need to specify arguments");
        }
    }
};
