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
var _apiDir = __dirname + "./../../../../ext/sensors/",
    _libDir = __dirname + "./../../../../lib/",
    events = require(_libDir + "event"),
    eventExt = require(__dirname + "./../../../../ext/event/index"),
    index;

describe("sensors index", function () {
    beforeEach(function () {
        GLOBAL.JNEXT = {
            require: jasmine.createSpy().andReturn(true),
            createObject: jasmine.createSpy().andReturn("1"),
            invoke: jasmine.createSpy().andReturn(2),
            registerEvents: jasmine.createSpy().andReturn(true),
        };
        index = require(_apiDir + "index");
    });

    afterEach(function () {
        GLOBAL.JNEXT = null;
        index = null;
    });

    describe("sensors", function () {
        describe("startSensor", function () {
            it("can call success", function () {
                var success = jasmine.createSpy(),
                    options = { "sensor" : "compass", "delay" : 10000 },
                    args = { "options" : JSON.stringify(options) };

                index.startSensor(success, null, args, null);
                expect(JNEXT.invoke).toHaveBeenCalledWith(jasmine.any(String), "startSensor " + JSON.stringify(options));
                expect(success).toHaveBeenCalled();
            });

            it("can call call with invalid parameters", function () {
                var fail = jasmine.createSpy(),
                    args = {};
                
                index.startSensor(null, fail, args, null);
                expect(fail).toHaveBeenCalledWith(-1, "Need to specify arguments");
            });
        });

        describe("startSensor", function () {
            it("can call success", function () {
                var success = jasmine.createSpy(),
                    options = { "sensor" : "compass" },
                    args = { "options" : JSON.stringify(options) };

                index.stopSensor(success, null, args, null);
                expect(JNEXT.invoke).toHaveBeenCalledWith(jasmine.any(String), "stopSensor " + options.sensor);
                expect(success).toHaveBeenCalled();
            });

            it("can call call with invalid parameters", function () {
                var fail = jasmine.createSpy(),
                    args = {};
                
                index.startSensor(null, fail, args, null);
                expect(fail).toHaveBeenCalledWith(-1, "Need to specify arguments");
            });
        });


        describe("onsensor", function () {
            it("can register the 'onsensor' event", function () {
                var eventName = "onsensor",
                    args = {eventName : encodeURIComponent(eventName)},
                    success = jasmine.createSpy(),
                    utils = require(_libDir + "utils");

                spyOn(utils, "loadExtensionModule").andCallFake(function () {
                    return eventExt;
                });

                spyOn(events, "add");
                index.registerEvents(success);
                eventExt.add(null, null, args);
                expect(success).toHaveBeenCalled();
                expect(events.add).toHaveBeenCalled();
                expect(events.add.mostRecentCall.args[0].event).toEqual(eventName);
                expect(events.add.mostRecentCall.args[0].trigger).toEqual(jasmine.any(Function));
            });

            it("can un-register the 'onsensor' event", function () {
                var eventName = "onsensor",
                    args = {eventName : encodeURIComponent(eventName)};

                spyOn(events, "remove");
                eventExt.remove(null, null, args);
                expect(events.remove).toHaveBeenCalled();
                expect(events.remove.mostRecentCall.args[0].event).toEqual(eventName);
                expect(events.remove.mostRecentCall.args[0].trigger).toEqual(jasmine.any(Function));
            });
        });
    });
});
