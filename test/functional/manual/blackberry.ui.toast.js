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

describe("blackberry.ui.toast", function () {
    var dismissCallback,
        buttonCallback;

    beforeEach(function () {
        dismissCallback = jasmine.createSpy();
        buttonCallback = jasmine.createSpy();
    });

    it('blackerry.ui.toast.show should be able to create a simple toast', function () {
        runs(function () {
            blackberry.ui.toast.show('This is a simple toast');
        });

        waits(2500);

        runs(function () {
            expect(window.confirm('Did you see the simple toast appear?')).toBeTruthy();
        });
    });

    it('blackberry.ui.toast.show should be able to create a complex toast', function () {
        runs(function () {
            window.confirm("Allow the next toast to disappear without selecting the button");
            blackberry.ui.toast.show('This is a simple toast', "ButtonText", null, dismissCallback);
        });

        waitsFor(function () {
            return dismissCallback.callCount === 1;
        }, "waiting for toast to disappear", 10000);

        runs(function () {
            expect(window.confirm('Did you see the complex toast appear?')).toBeTruthy();
        });
    });

    it('blackberry.ui.toast.show should fire dismiss callback', function () {
        runs(function () {
            window.confirm('Allow the next toast to disappear without selecting the button');
            blackberry.ui.toast.show('This is a complex toast', "ButtonText", buttonCallback, dismissCallback);
        });

        waitsFor(function () {
            return dismissCallback.callCount === 1;
        }, "waiting for toast to disappear", 10000);

        runs(function () {
            expect(window.confirm('Did you see the complex toast appear?')).toBeTruthy();
        });
    });

    it('blackberry.ui.toast.show should fire button callback', function () {
        runs(function () {
            window.confirm('Click the button in the next toast');
            blackberry.ui.toast.show('This is a complex toast', "ButtonText", buttonCallback, dismissCallback);
        });

        waitsFor(function () {
            return buttonCallback.callCount === 1 && dismissCallback.callCount === 1;
        }, "waiting for toast to disappear", 10000);

        runs(function () {
            expect(window.confirm('Did you see the complex toast appear?')).toBeTruthy();
        });
    });
});
