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
      // The camera can be invoked in three modes.
    MODE_PHOTO: 'photo',
    MODE_VIDEO: 'video',
    MODE_FULL: 'full',

    open: function (mode, success, fail) {

        var application = window.qnx.webplatform.getApplication();
        window.qnx.webplatform.getApplication().invocation.addEventListener("childCardClosed", function (info) {
            if (info.reason === "save") {
                success(info.data);
            } else if (info.reason === "done" || info.reason === "close") {
                fail(info.reason);
            }
        });

        application.invocation.invoke({
            action: "bb.action.CAPTURE",
            target: "sys.camera.card",
            data: window.btoa(mode)
        }, function (error) {
                if (error) {
                    console.log("error:" + error);
                } else {
                    console.log("success");
                }
            });

    }
};

