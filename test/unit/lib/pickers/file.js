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

describe("file ", function () {
    var file,
        libPath = "./../../../../",
        mockedInvocation,
        _callback,
        flag,
        success,
        fail,
        btoa = require("btoa"),
        details,
        ppsUtils;

    beforeEach(function () {
        ppsUtils = require(libPath + "lib/pps/ppsUtils");
        file = require(libPath + "lib/pickers/file");
        flag = false;
        details = {
            mode: file.MODE_PICKER,
            selectMode: file.SELECT_MODE_SINGLE,

        };
        mockedInvocation = {
            invoke: jasmine.createSpy("invoke").andCallFake(function () {
                var data = {
                    reason: "save",
                    type: "jpg",
                    data: 'dat:json:[{"uri":"file:///accounts/1000/shared/camera/IMG_00000001.jpg"}]'
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
    it("calls invoke file", function () {
        var data = {
            Mode: details.mode,
            SelectMode: details.selectMode,
            DefaultType: details.defaultType,
            Filter: details.filter,
            Directory: details.directory,
            DefaultSaveFileName: details.defaultSaveFileName,
            Title: details.title,
            ViewMode: details.viewMode,
            SortBy: details.sortBy,
            SortOrder: details.sortOrder,
            ImageCrop: details.imageCropEnabled
        };
        if (details.type) {
            data.Type = details.type.join(',');
        } else {
            data.Type = [];
        }
        file.open(details, success, fail);
        expect(mockedInvocation.invoke).toHaveBeenCalledWith({
            action: 'bb.action.OPEN',
            target: 'sys.filepicker.target',
            data: btoa(ppsUtils.ppsEncodeObject(data))
        }, jasmine.any(Function));
    });

    it("calls the success callback", function () {
        runs(function () {
            file.open(details, success, fail);
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
                    reason: "cancel",
                    type: "image/jpg"
                };
                _callback(data);
                flag = true;
            }),
            addEventListener: jasmine.createSpy("addEventListener").andCallFake(function (event, callback) {
                _callback = callback;
            })
        };

        runs(function () {
            file.open(details, success, fail);
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
            file.open(details, success, fail);
        });
        waitsFor(function () {
            return flag;
        });
        runs(function () {
            expect(success).not.toHaveBeenCalled();
            expect(fail).not.toHaveBeenCalled();
        });
    });

    it("will keep working with modified data", function () {
        mockedInvocation = {
            invoke: jasmine.createSpy("invoke").andCallFake(function () {
                var data = {
                    reason: "save",
                    type: "jpg",
                    data: '[{"uri":"file:///accounts/1000/shared/camera/IMG_00000001.jpg"}]'
                };
                _callback(data);
                flag = true;
            }),
            addEventListener: jasmine.createSpy("addEventListener").andCallFake(function (event, callback) {
                _callback = callback;
            })
        };

        runs(function () {
            file.open(details, success, fail);
        });
        waitsFor(function () {
            return flag;
        });
        runs(function () {
            expect(success).toHaveBeenCalledWith("/accounts/1000/shared/camera/IMG_00000001.jpg");
            expect(fail).not.toHaveBeenCalled();
        });
    });

});
