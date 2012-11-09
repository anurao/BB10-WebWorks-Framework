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
#ifndef ROUTING_IF_HELPER_JS_HPP_
#define ROUTING_IF_HELPER_JS_HPP_

#include <string>
#include <pthread.h>
#include "../public/plugin.h"

class RoutingHelper: public JSExt {

public:
    explicit RoutingHelper(const std::string& id);
    virtual ~RoutingHelper();

// Interfaces of JSExt
    virtual bool CanDelete();
    virtual std::string InvokeMethod(const std::string& command);

private:
// Utility functions relate to memory function
    void getMemory();
    string convertLongToString(long l);
    /**Set the routing interface for the device. Sets the SOCK_SO_BINDTODEVICE
     * environment variable
     * @param: interface name (msm1, msm0 etc)
     */
    int setRoutingInterface(const std::string& interfaceName);
    int resetRoutingInterface();
    string getRoutingInterface();
    string convertIntToString(int i);
    std::string m_id;
};

#endif /* ROUTING_IF_HELPER_JS_HPP_ */
