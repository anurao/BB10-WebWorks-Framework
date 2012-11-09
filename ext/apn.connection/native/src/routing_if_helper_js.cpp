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

#include <string>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "routing_if_helper_js.hpp"

using namespace std;

/**
 * Default constructor.
 */
RoutingHelper::RoutingHelper(const std::string& id) : m_id(id) {

}

/**
 *  destructor.
 */
RoutingHelper::~RoutingHelper() {
}

/**
 * This method returns the list of objects implemented by this native
 * extension.
 */
char* onGetObjList() {
    static char name[] = "RoutingHelper";
    return name;
}

/**
 * This method is used by JNext to instantiate the RoutingHelper object when
 * an object is created on the JavaScript server side.
 */
JSExt* onCreateObject(const string& className, const string& id) {
    if (className == "RoutingHelper") {
        return new RoutingHelper(id);
    }

    return NULL;
}

/**
 * Method used by JNext to determine if the object can be deleted.
 */
bool RoutingHelper::CanDelete() {
    return true;
}

/**
 * It will be called from JNext JavaScript side with passed string.
 * This method implements the interface for the JavaScript to native binding
 * for invoking native code. This method is triggered when JNext.invoke is
 * called on the JavaScript side with this native objects id.
 */
string RoutingHelper::InvokeMethod(const string& command) {
    // Determine which function should be executed
	int index = command.find_first_of(" ");
	string strCommand = command.substr(0, index);
	string interfaceName = command.substr(index + 1, command.length());
	int  success = 0;
    if (strCommand == "setRoutingInterface") {
        success = setRoutingInterface(interfaceName);
    }else if (strCommand == "resetRoutingInterface") {
    	success = resetRoutingInterface();
    }else if (strCommand == "getRoutingInterface") {
    	return getRoutingInterface();
    }
    else {
    	//unsupported command
        success = -1;
    }
    return convertIntToString(success);
}

/**
 * Method that binds all sockets to a particular interface
 */
int RoutingHelper::setRoutingInterface(const std::string& interfaceName) {
	 return setenv("SOCK_SO_BINDTODEVICE", interfaceName.c_str(), 1);
	}


/**
 * Method that reset the variable all sockets to a particular interface
 */
int RoutingHelper::resetRoutingInterface() {
	 return unsetenv("SOCK_SO_BINDTODEVICE");
	}

/**
 * Method that binds all sockets to a particular interface
 */
string RoutingHelper::getRoutingInterface() {
	 return getenv("SOCK_SO_BINDTODEVICE");
	}

string RoutingHelper::convertIntToString(int i) {
	stringstream ss;
	ss << i;
	return ss.str();
}
