/*
 * Copyright 2010-2011 Research In Motion Limited.
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

var root = __dirname + "/../../../../",
    webview = require(root + "lib/webview"),
    overlayWebView = require(root + "lib/overlayWebView"),
    index;

describe("ui.toast index", function () {
    beforeEach(function () {
        //Set up mocking, no need to "spyOn" since spies are included in mock
        index = require(root + "ext/ui.toast/index");
        spyOn(overlayWebView, "showToast").andReturn(1);
    });

    it("shows toast", function () {
        var success = jasmine.createSpy(),
            fail = jasmine.createSpy(),
            mockArgs = {
                message: encodeURIComponent("\"This is a toast\""),
                buttonText: encodeURIComponent("\"Button Text\"")
            };

        index.show(success, fail, mockArgs, null);
        expect(success).toHaveBeenCalledWith(1);
    });
});