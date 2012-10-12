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
#include <json/writer.h>
#include <string>
#include <QThread>
//#include "timezone_qt.hpp"
#include "timezone_js.hpp"
#include "timezone_js.moc"

Timezone::Timezone(const std::string& id) : m_id(id), m_threadInfo(NULL)
{
}
/*
Timezone::Timezone() : m_id(""), m_threadInfo(NULL)
{
}
*/
char* onGetObjList()
{
    // Return list of classes in the object
    static char name[] = "Timezone";
    return name;
}

JSExt* onCreateObject(const std::string& className, const std::string& id)
{
    // Make sure we are creating the right class
    if (className != "Timezone") {
        return NULL;
    }

    return new Timezone(id);
}

std::string Timezone::InvokeMethod(const std::string& command)
{
    fprintf(stderr, "Timezone::InvokeMethod: %s\n", command.c_str());

    unsigned int index = command.find_first_of(" ");

    string strCommand;
    string jsonObject;
    Json::Value *obj;

    if (index != std::string::npos) {
        strCommand = command.substr(0, index);
        jsonObject = command.substr(index + 1, command.length());

        // Parse the JSON
        Json::Reader reader;
        obj = new Json::Value;
        bool parse = reader.parse(jsonObject, *obj);

        if (!parse) {
            fprintf(stderr, "%s", "error parsing\n");
            return "Cannot parse JSON object";
        }
    } else {
        strCommand = command;
        obj = new Json::Value;
    }

    TimezoneThreadInfo *thread_info = new TimezoneThreadInfo;
    thread_info->parent = this;
    thread_info->jsonObj = obj;
    thread_info->eventId = obj->removeMember("_eventId").asString();
/*
    QThread* thread = new QThread;
    Timezone* worker = new Timezone;
    worker->setThreadInfo(*thread_info);
    worker->moveToThread(thread);
    connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
    connect(thread, SIGNAL(started()), worker, SLOT(process()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
*/
    /*
    int index = command.find_first_of(" ");

    string strCommand;
    string jsonObject;
    Json::Value *obj;

    if (index != std::string::npos) {
        strCommand = command.substr(0, index);
        jsonObject = command.substr(index + 1, command.length());

        // Parse the JSON
        Json::Reader reader;
        obj = new Json::Value;
        bool parse = reader.parse(jsonObject, *obj);

        if (!parse) {
            fprintf(stderr, "%s", "error parsing\n");
            return "Cannot parse JSON object";
        }
    } else {
        strCommand = command;
    }

    fprintf(stderr, "strCommand: %s\n", strCommand.c_str());

    if (strCommand == "find") {
        //startThread(FindThread, obj);
    } else if (strCommand == "save") {
        //startThread(SaveThread, obj);
    } else if (strCommand == "remove") {
        //startThread(RemoveThread, obj);
    } else if (strCommand == "getCalendarFolders") {
        webworks::PimCalendarQt pim_qt;
        Json::Value result = pim_qt.GetCalendarFolders();
        Json::FastWriter writer;
        std::string str = writer.write(result);
        fprintf(stderr, "Calendar Folders Result: %s", str.c_str());
        return str;
    } else if (strCommand == "getDefaultCalendarFolder") {
        webworks::PimCalendarQt pim_qt;
        Json::Value result = pim_qt.GetDefaultCalendarFolder();
        Json::FastWriter writer;
        std::string str = writer.write(result);
        return str;
    } else if (strCommand == "getTimezones") {
        webworks::PimCalendarQt pim_qt;
        Json::Value result = pim_qt.GetTimezones();
        Json::FastWriter writer;
        std::string str = writer.write(result);
        fprintf(stderr, "Timezone Result: %s", str.c_str());
        return str;
    }
    */
    return "";
}

bool Timezone::CanDelete()
{
    return true;
}

// Notifies JavaScript of an event
void Timezone::NotifyEvent(const std::string& eventId, const std::string& event)
{
    std::string eventString = m_id + " result ";
    eventString.append(eventId);
    eventString.append(" ");
    eventString.append(event);
    SendPluginEvent(eventString.c_str(), m_pContext);
}

void Timezone::process()
{
    fprintf(stderr, "I am in process!%s\n", "");
    Json::Value args = *(m_threadInfo->jsonObj);
    fprintf(stderr, "Test read arguments: %s\n", args["test"].asString().c_str());
    Json::Value result = Json::Value();
    result["success"] = true;
    Json::FastWriter writer;
    std::string str = writer.write(result);
    m_threadInfo->parent->NotifyEvent(m_threadInfo->eventId, str);
    emit finished();
}

void Timezone::finished()
{

}

void Timezone::setThreadInfo(TimezoneThreadInfo* threadInfo)
{
    m_threadInfo = threadInfo;
}
/*
void Timezone::setResults(const Json::Value& resultObj)
{
    m_resultObj = resultObj;
}
*/
/*
bool PimCalendar::startThread(ThreadFunc threadFunction, Json::Value *jsonObj) {
    webworks::PimCalendarThreadInfo *thread_info = new webworks::PimCalendarThreadInfo;
    thread_info->parent = this;
    thread_info->jsonObj = jsonObj;
    thread_info->eventId = jsonObj->removeMember("_eventId").asString();

    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

    pthread_t thread;
    pthread_create(&thread, &thread_attr, threadFunction, static_cast<void *>(thread_info));
    pthread_attr_destroy(&thread_attr);

    if (!thread) {
        return false;
    }

    return true;
}


// Static functions:

void* PimCalendar::FindThread(void *args)
{
    webworks::PimCalendarThreadInfo *thread_info = static_cast<webworks::PimCalendarThreadInfo *>(args);

    webworks::PimCalendarQt pim_qt;
    Json::Value result = pim_qt.Find(*(thread_info->jsonObj));

    Json::FastWriter writer;
    std::string event = writer.write(result);

    thread_info->parent->NotifyEvent(thread_info->eventId, event);
    return NULL;
}

void* PimCalendar::SaveThread(void *args)
{
    webworks::PimCalendarThreadInfo *thread_info = static_cast<webworks::PimCalendarThreadInfo *>(args);

    webworks::PimCalendarQt pim_qt;
    Json::Value result = pim_qt.Save(*(thread_info->jsonObj));

    Json::FastWriter writer;
    std::string event = writer.write(result);

    thread_info->parent->NotifyEvent(thread_info->eventId, event);
    return NULL;
}

void* PimCalendar::RemoveThread(void *args)
{
    webworks::PimCalendarThreadInfo *thread_info = static_cast<webworks::PimCalendarThreadInfo *>(args);

    webworks::PimCalendarQt pim_qt;
    Json::Value result = pim_qt.DeleteCalendarEvent(*(thread_info->jsonObj));

    Json::FastWriter writer;
    std::string event = writer.write(result);

    thread_info->parent->NotifyEvent(thread_info->eventId, event);
    return NULL;
}
*/
