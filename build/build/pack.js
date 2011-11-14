/*
* Copyright 2011 Research In Motion Limited.
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
var childProcess = require("child_process"),
    utils = require("./utils"),
    util = require("util"),
    _c = require("./conf"),
    fs = require("fs");

function _copyCmd(source, destination) { 
    if (utils.isWindows()) {
        if (destination === '') {
            return 'xcopy /y ' + source + ' ' + _c.DEPLOY + destination;
        } else {
            return 'cmd /c if not exist ' + _c.DEPLOY + destination + 
                   ' md ' + _c.DEPLOY + destination + ' && ' + 
                   'xcopy /y/e ' + source + ' ' + _c.DEPLOY + destination;
        }
    } else {
        if (destination == '') {
            return 'cp -r ' + source + ' ' + _c.DEPLOY + destination;
        } else {
            return 'mkdir -p ' + _c.DEPLOY + destination + ' && ' +
                   'cp -r ' + source + ' ' + _c.DEPLOY + destination;
        } 
    }
}
 
function _copyFiles() {
    var cmdSep = " && ";
    return  _copyCmd(_c.LIB, '') + cmdSep +
            _copyCmd(_c.BIN, '') + cmdSep +
            _copyCmd(_c.NODE_MOD, '') + cmdSep +
            _copyCmd(_c.DEPENDENCIES + '/BBX-Emulator/lib', 'dependencies/BBX-Emulator') + cmdSep +
            _copyCmd(_c.ROOT + 'README.md', '') + cmdSep +
            _copyCmd(_c.ROOT + 'LICENSE', '');
            
}

module.exports = function (src, baton) {
    baton.take();
    
    childProcess.exec(_copyFiles(), function (error, stdout, stderr) {
        if (error) {
            console.log(stdout);
            console.log(stderr);
            baton.pass(error.code);
        } else {
            baton.pass(src);
        }
    });
};
