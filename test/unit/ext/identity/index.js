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
var _apiDir = __dirname + "./../../../../ext/identity/",
    index;

describe("identity index", function () {
    beforeEach(function () {
        index = require(_apiDir + "index");
        GLOBAL.window = {
            qnx: {
                webplatform: {
                    device: {
                    }
                }
            }
        };
    });

    afterEach(function () {
        GLOBAL.window = null;
        index = null;
    });

    describe("uuid", function () {
        it("calls success when devicepin is truthy", function () {
            var success = jasmine.createSpy(),
                fail = jasmine.createSpy();

            window.qnx.webplatform.device.devicepin = (new Date()).getTime();
            index.uuid(success, fail);
            expect(success).toHaveBeenCalledWith(window.qnx.webplatform.device.devicepin);
            expect(fail).not.toHaveBeenCalled();
        });
        it("calls fail when devicepin is falsey", function () {
            var success = jasmine.createSpy(),
                fail = jasmine.createSpy();

            window.qnx.webplatform.device.devicepin = undefined;
            index.uuid(success, fail);
            expect(fail).toHaveBeenCalledWith(-1, "Failed to retrieve devicepin");
            expect(success).not.toHaveBeenCalled();
        });
    });
});
