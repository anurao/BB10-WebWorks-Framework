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

#include <json/reader.h>
#include <string>
#include "sensors_js.hpp"
#include "sensors_ndk.hpp"

bool eventsInitialized = false;

Sensors::Sensors(const std::string& id) : m_id(id)
{
    m_pSensorsController = new webworks::SensorsNDK(this);
}

Sensors::~Sensors()
{
}

char* onGetObjList()
{
    static char name[] = "Sensors";
    return name;
}

JSExt* onCreateObject(const std::string& className, const std::string& id)
{
    if (className != "Sensors") {
        return NULL;
    }

    return new Sensors(id);
}

std::string Sensors::InvokeMethod(const std::string& command)
{
    int index = command.find_first_of(" ");
    std::string strCommand = command.substr(0, index);
    std::string arg = command.substr(index + 1, command.length());

    if (strCommand == "startSensor") {
        Json::Reader reader;
        Json::Value obj;

        bool parse = reader.parse(arg, obj);

        if (!parse)
            return "";

        webworks::SensorConfig sensorConfig;
        sensorConfig.sensor = obj["sensor"].asString();
        sensorConfig.delay = obj["delay"].asInt();
        sensorConfig.background = obj["background"].asBool();
        sensorConfig.queue = obj["queue"].asBool();
        sensorConfig.reducedReporting = obj["reducedReporting"].asBool();
        m_pSensorsController->StartSensor(&sensorConfig);
        return "";
    } else if (strCommand == "stopSensor") {
        m_pSensorsController->StopSensor(arg);
        return "";
    } else if (strCommand == "startEvents") {
        m_pSensorsController->StartEvents();
        return "";
    } else if (strCommand == "stopEvents") {
        m_pSensorsController->StopEvents();
        return "";
    }

    return NULL;
}

bool Sensors::CanDelete()
{
    return true;
}

// Notifies JavaScript of an event
void Sensors::NotifyEvent(const std::string& event)
{
    std::string eventString = m_id + " ";
    eventString.append(event);
    SendPluginEvent(eventString.c_str(), m_pContext);
}

