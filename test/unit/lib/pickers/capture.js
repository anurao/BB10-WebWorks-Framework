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

describe("capture ", function () {
    var capture,
        libPath = "./../../../../",
        mockedInvocation,
        _callback,
        flag,
        success,
        fail,
        btoa = require("btoa");

    beforeEach(function () {
        capture = require(libPath + "lib/pickers/capture");
        flag = false;
        mockedInvocation = {
            invoke: jasmine.createSpy("invoke").andCallFake(function () {
                var data = {
                    reason: "save",
                    type: "image/jpg",
                    data: "/accounts/1000/shared/camera/IMG_00000001.jpg"
                };
                _callback(data);
                flag = true;
            }),
            addEventListener: jasmine.createSpy("addEventListener").andCallFake(function (event, callback) {
                _callback = callback;
            })
        };
        success = jasmine.createSpy("success");
        fail = jasmine.createSpy("fail");
        GLOBAL.window = {};
        GLOBAL.window.btoa = btoa;
        GLOBAL.window.qnx = {
            webplatform: {
                getApplication: function () {
                    return {
                        invocation: mockedInvocation
                    };
                }
            }
        };
    });
    it("calls invoke capture", function () {
        capture.open(capture.MODE_PHOTO, success, fail);
        expect(mockedInvocation.invoke).toHaveBeenCalledWith({
            action: 'bb.action.CAPTURE',
            target: 'sys.camera.card',
            data: btoa(capture.MODE_PHOTO)
        }, jasmine.any(Function));
    });

    it("calls the success callback", function () {
        runs(function () {
            capture.open(capture.MODE_PHOTO, success, fail);
        });
        waitsFor(function () {
            return flag;
        });
        runs(function () {
            expect(success).toHaveBeenCalled();
        });
    });

    it("calls the fail callback", function () {
        mockedInvocation = {
            invoke: jasmine.createSpy("invoke").andCallFake(function () {
                var data = {
                    reason: "done",
                    type: "image/jpg",
                    data: "/accounts/1000/shared/camera/IMG_00000001.jpg"
                };
                _callback(data);
                flag = true;
            }),
            addEventListener: jasmine.createSpy("addEventListener").andCallFake(function (event, callback) {
                _callback = callback;
            })
        };

        runs(function () {
            capture.open(capture.MODE_PHOTO, success, fail);
        });
        waitsFor(function () {
            return flag;
        });
        runs(function () {
            expect(fail).toHaveBeenCalled();
        });
    });

    it("No callback triggered if reason is unknown", function () {
        mockedInvocation = {
            invoke: jasmine.createSpy("invoke").andCallFake(function () {
                var data = {
                    reason: "Unknown Reason",
                    type: "image/jpg",
                    data: "/accounts/1000/shared/camera/IMG_00000001.jpg"
                };
                _callback(data);
                flag = true;
            }),
            addEventListener: jasmine.createSpy("addEventListener").andCallFake(function (event, callback) {
                _callback = callback;
            })
        };

        runs(function () {
            capture.open(capture.MODE_PHOTO, success, fail);
        });
        waitsFor(function () {
            return flag;
        });
        runs(function () {
            expect(success).not.toHaveBeenCalled();
            expect(fail).not.toHaveBeenCalled();
        });
    });

});
