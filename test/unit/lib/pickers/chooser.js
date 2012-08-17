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

describe("chooser ", function () {
    var chooser,
        libPath = "./../../../../",
        capture,
        file,
        returnValue,
        chooserObj,
        path = "/accounts/1000/shared/camera/IMG_00000001.jpg",
        baton;

    beforeEach(function () {
        chooser = require(libPath + "lib/pickers/chooser");
        file = require(libPath + "lib/pickers/file");
        capture = require(libPath + "lib/pickers/capture");
        baton = {
            take: jasmine.createSpy("take"),
            pass: jasmine.createSpy("pass").andCallFake(function (value) {
                returnValue = value;
            })
        };
        chooserObj = chooser.init();
        spyOn(capture, "open").andCallFake(function (captureMode, success, fail) {
            success(path);
        });
        spyOn(file, "open").andCallFake(function (details, success, fail) {
            success(path);
        });
    });


    it("calls capture with photo", function () {
        chooserObj.fileChooserHandler(JSON.stringify({capture: 'camera'}), baton);
        expect(capture.open).toHaveBeenCalledWith(capture.MODE_PHOTO, jasmine.any(Function), jasmine.any(Function));
    });

    it("calls capture with video", function () {
        chooserObj.fileChooserHandler(JSON.stringify({capture: 'camcorder'}), baton);
        expect(capture.open).toHaveBeenCalledWith(capture.MODE_VIDEO, jasmine.any(Function), jasmine.any(Function));
    });

    it("calls capture and returns correct value on success", function () {
        capture.open = jasmine.createSpy().andCallFake(function (captureMode, success, fail) {
            success(path);
        });
        var returnValueExpected = {
            setPreventDefault: true,
            setFileChosen: encodeURIComponent(path)
        };
        chooserObj.fileChooserHandler(JSON.stringify({capture: 'camcorder'}), baton);
        expect(JSON.parse(returnValue)).toEqual(returnValueExpected);
    });

    it("calls capture and returns correct value on fail", function () {
        capture.open = jasmine.createSpy().andCallFake(function (captureMode, success, fail) {
            fail();
        });
        var returnValueExpected = {
            setPreventDefault: true
        };
        chooserObj.fileChooserHandler(JSON.stringify({capture: 'camcorder'}), baton);
        expect(JSON.parse(returnValue)).toEqual(returnValueExpected);
    });

    it("calls file", function () {
        chooserObj.fileChooserHandler(JSON.stringify({}), baton);
        expect(file.open).toHaveBeenCalled();//With(jasmine.any(Object), jasmine.any(Function), jasmine.any(Function));
    });

    it("calls file and returns correct value on success", function () {
        file.open = jasmine.createSpy().andCallFake(function (details, success, fail) {
            success(path);
        });
        var returnValueExpected = {
            setPreventDefault: true,
            setFileChosen: encodeURIComponent(path)
        };
        chooserObj.fileChooserHandler(JSON.stringify({}), baton);
        expect(JSON.parse(returnValue)).toEqual(returnValueExpected);
    });

    it("calls file and returns correct value on fail", function () {
        file.open = jasmine.createSpy().andCallFake(function (details, success, fail) {
            fail();
        });
        var returnValueExpected = {
            setPreventDefault: true
        };
        chooserObj.fileChooserHandler(JSON.stringify({}), baton);
        expect(JSON.parse(returnValue)).toEqual(returnValueExpected);
    });

});
