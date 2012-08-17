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

module.exports = {
    init: function () {
        return {
            fileChooserHandler: function (params, baton) {
                var capture,
                    file,
                    details,
                    captureMode;
                params = JSON.parse(params);
                debugger;
                if (params.capture === 'camera' || params.capture === 'camcorder') {
                    baton.take();
                    capture = require('./capture');
                    captureMode = params.capture === 'camera' ? capture.MODE_PHOTO : capture.MODE_VIDEO;

                    capture.open(captureMode, function (path) {
                        baton.pass('{"setPreventDefault": true, "setFileChosen": "' + encodeURIComponent(path) + '"}');
                    },
                    function () {
                        baton.pass('{"setPreventDefault": true}');
                    });
                } else {
                    baton.take();
                    file = require('./file');
                    details = {
                        mode: file.MODE_PICKER,
                        selectMode: file.SELECT_MODE_SINGLE
                    };

                    if (params.allowsMultipleFiles) {
                        details.selectMode = file.SELECT_MODE_MULTIPLE;
                    }
                    // TODO: Change the type property according to acceptMIMETypes and set the filter property on details.
                    file.open(details, function (filepath) {
                        baton.pass('{"setPreventDefault": true, "setFileChosen": "' + encodeURIComponent(filepath) + '"}');
                    },
                    function () {
                        baton.pass('{"setPreventDefault": true}');
                    });
                }
            }
        };
    }
};
