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
    _apiDir = _extDir + "/invoke.card",
    _ID = require(_apiDir + "/manifest").namespace,
    client,
    mockedWebworks = {
        execSync: jasmine.createSpy().andCallFake(function (id, name, args) {
            if (name === 'invokeTargetPicker') {
                if (args.request) {
                    return 42;
                }
                else {
                    throw "Bad request!";
                }
            }
        })
    };

describe("invoke.card", function () {
    beforeEach(function () {
        GLOBAL.window.webworks = mockedWebworks;
        client = require(_apiDir + "/client");
    });

    it('exposes a function called invokeTargetPicker', function () {
        expect(client.invokeTargetPicker).toEqual(jasmine.any(Function));
    });

    it('calls invokeTargetPicker in the index and passes back success value', function () {
        var request = {},
            title = 'test title',
            success = jasmine.createSpy(),
            error = jasmine.createSpy();

        client.invokeTargetPicker(request, title, success, error);
        expect(mockedWebworks.execSync).toHaveBeenCalledWith('blackberry.invoke.card', 'invokeTargetPicker', {
            request: request,
            title: title
        });
        expect(success).toHaveBeenCalledWith(42);
    });

    it('calls the error callback if execSync fails', function () {
        var request = null,
            title = 'test title',
            success = jasmine.createSpy(),
            error = jasmine.createSpy();

        client.invokeTargetPicker(request, title, success, error);
        expect(error).toHaveBeenCalled();
    });
});
