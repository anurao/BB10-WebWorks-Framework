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
var _extDir = __dirname + "./../../../../ext",
    _apiDir = _extDir + "/sensors",
    _ID = require(_apiDir + "/manifest").namespace,
    client,
    mockedWebworks = {
        execSync: jasmine.createSpy(),
        execAsync: jasmine.createSpy(),
        defineReadOnlyField: jasmine.createSpy()
    },
    constants = {
        "ACCELEROMETER" : "accelerometer",
        "MAGNETOMETER" : "magnetometer",
        "GYROSCOPE" : "gyroscope",
        "COMPASS" : "compass",
        "PROXIMITY" : "proximity",
        "LIGHT" : "light",
        "GRAVITY" : "gravity",
        "LINEAR_ACCELERATION" : "linear_acceleration",
        "ROTATION_VECTOR" : "rotation_vector",
        "ORIENTATION" : "orientation",
        "ROTATION_MATRIX" : "rotation_matrix",
        "AZIMUTH_PITCH_ROLL" : "azimuth_pitch_roll",
        "FACE_DETECT" : "face_detect",
        "HOLSTER" : "holster"
    },
    defineROFieldArgs = [];

describe("sensors", function () {
    beforeEach(function () {
        GLOBAL.window.webworks = mockedWebworks;
        client = require(_apiDir + "/client");
    });

    describe("sensor constants", function () {
        it("should return constant", function () {
            Object.getOwnPropertyNames(constants).forEach(function (c) {
                defineROFieldArgs.push([client, c, constants[c]]);
            });
            expect(mockedWebworks.defineReadOnlyField.argsForCall).toContain(defineROFieldArgs[Object.getOwnPropertyNames(constants).indexOf("ACCELEROMETER")]);
            expect(mockedWebworks.defineReadOnlyField.argsForCall).toContain(defineROFieldArgs[Object.getOwnPropertyNames(constants).indexOf("MAGNETOMETER")]);
            expect(mockedWebworks.defineReadOnlyField.argsForCall).toContain(defineROFieldArgs[Object.getOwnPropertyNames(constants).indexOf("GYROSCOPE")]);
            expect(mockedWebworks.defineReadOnlyField.argsForCall).toContain(defineROFieldArgs[Object.getOwnPropertyNames(constants).indexOf("COMPASS")]);
            expect(mockedWebworks.defineReadOnlyField.argsForCall).toContain(defineROFieldArgs[Object.getOwnPropertyNames(constants).indexOf("PROXIMITY")]);
            expect(mockedWebworks.defineReadOnlyField.argsForCall).toContain(defineROFieldArgs[Object.getOwnPropertyNames(constants).indexOf("LIGHT")]);
            expect(mockedWebworks.defineReadOnlyField.argsForCall).toContain(defineROFieldArgs[Object.getOwnPropertyNames(constants).indexOf("GRAVITY")]);
            expect(mockedWebworks.defineReadOnlyField.argsForCall).toContain(defineROFieldArgs[Object.getOwnPropertyNames(constants).indexOf("LINEAR_ACCELERATION")]);
            expect(mockedWebworks.defineReadOnlyField.argsForCall).toContain(defineROFieldArgs[Object.getOwnPropertyNames(constants).indexOf("ROTATION_VECTOR")]);
            expect(mockedWebworks.defineReadOnlyField.argsForCall).toContain(defineROFieldArgs[Object.getOwnPropertyNames(constants).indexOf("ORIENTATION")]);
            expect(mockedWebworks.defineReadOnlyField.argsForCall).toContain(defineROFieldArgs[Object.getOwnPropertyNames(constants).indexOf("ROTATION_MATRIX")]);
            expect(mockedWebworks.defineReadOnlyField.argsForCall).toContain(defineROFieldArgs[Object.getOwnPropertyNames(constants).indexOf("AZIMUTH_PITCH_ROLL")]);
            expect(mockedWebworks.defineReadOnlyField.argsForCall).toContain(defineROFieldArgs[Object.getOwnPropertyNames(constants).indexOf("FACE_DETECT")]);
            expect(mockedWebworks.defineReadOnlyField.argsForCall).toContain(defineROFieldArgs[Object.getOwnPropertyNames(constants).indexOf("HOLSTER")]);
        });
    });

    describe("startSensor", function () {
        it("calls execAsync", function () {
            client.startSensor("compass", { delay : 1000 });
            expect(mockedWebworks.execAsync).toHaveBeenCalledWith(_ID, "startSensor", { options : { delay : 1000, sensor : "compass" } });
        });
    });

    describe("stopSensor", function () {
        it("calls execAsync", function () {
            client.stopSensor("compass");
            expect(mockedWebworks.execAsync).toHaveBeenCalledWith(_ID, "stopSensor", { options : { sensor : "compass" } });
        });
    });
});
