/*
 * Copyright 2011-2012 Research In Motion Limited.
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

var _extDir = __dirname + "./../../../../ext",
    _apiDir = _extDir + "/card",
    _ID = require(_apiDir + "/manifest").namespace,
    client,
    mockedWebworks = {
        execSync: jasmine.createSpy("webworks.execSync"),
        execAsync: jasmine.createSpy("webworks.execAsync"),
        defineReadOnlyField: jasmine.createSpy(),
        event: {
            isOn: jasmine.createSpy("webworks.event.isOn"),
            once: jasmine.createSpy("webworks.event.once")
        }
    };

describe("invoke.card client", function () {
    beforeEach(function () {
        GLOBAL.window = GLOBAL;
        GLOBAL.window.btoa = jasmine.createSpy("window.btoa").andReturn("base64 string");
        mockedWebworks.event.once = jasmine.createSpy("webworks.event.once");
        GLOBAL.window.webworks = mockedWebworks;
        client = require(_apiDir + "/client");
    });

    afterEach(function () {
        delete GLOBAL.window;
        client = null;
    });

    describe("defines read only fields for modes", function () {
        it("should define photo|video|full", function () {
            expect(mockedWebworks.defineReadOnlyField).toHaveBeenCalledWith(client, "CAMERA_MODE_PHOTO", "photo");
            expect(mockedWebworks.defineReadOnlyField).toHaveBeenCalledWith(client, "CAMERA_MODE_VIDEO", "video");
            expect(mockedWebworks.defineReadOnlyField).toHaveBeenCalledWith(client, "CAMERA_MODE_FULL", "full");
        });
    });

    describe("invoke camera ", function () {
        var done,
            cancel,
            invokeCallback;
        beforeEach(function () {
            done = jasmine.createSpy("done");
            cancel = jasmine.createSpy("cancel");
            invokeCallback = jasmine.createSpy("invokeCallback");
        });
        it("should call execAsyn with correct mode", function () {
            client.invokeCamera("photo");
            expect(mockedWebworks.execAsync).toHaveBeenCalledWith(_ID, "invokeCamera", {mode: "photo"});
        });
        it("should register all the events", function () {
            client.invokeCamera("photo", done, cancel, invokeCallback);
            expect(mockedWebworks.event.once).toHaveBeenCalledWith(_ID, jasmine.any(String), done);
            expect(mockedWebworks.event.once).toHaveBeenCalledWith(_ID, jasmine.any(String), cancel);
            expect(mockedWebworks.event.once).toHaveBeenCalledWith(_ID, jasmine.any(String), invokeCallback);
        });
    });

});
