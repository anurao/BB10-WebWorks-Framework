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

#define SENSOR_MSG_PULSE 64
#define SENSOR_BASE_PULSE 128

#include <json/writer.h>
#include <sensor/libsensor.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <errno.h>
#include <stdio.h>
#include <algorithm>
#include <string>
#include "sensors_ndk.hpp"
#include "sensors_js.hpp"

#define MUTEX_LOCK() pthread_mutex_trylock(&m_lock)
#define MUTEX_UNLOCK() pthread_mutex_unlock(&m_lock)

namespace webworks {

int SensorsNDK::m_sensorChannel;
int SensorsNDK::m_coid;
bool SensorsNDK::m_sensorsEnabled;
ActiveSensorMap SensorsNDK::m_activeSensors;
pthread_mutex_t SensorsNDK::m_lock = PTHREAD_MUTEX_INITIALIZER;

SensorsNDK::SensorsNDK(Sensors *parent) : m_pParent(parent)
{
    createSensorMap();
}

SensorsNDK::~SensorsNDK()
{
    StopEvents();
}

void SensorsNDK::StartEvents()
{
    if (!m_thread) {
        int error = pthread_create(&m_thread, NULL, SensorThread, static_cast<void *>(m_pParent));

        if (error) {
            m_thread = 0;
        }
    }
}

void SensorsNDK::StopEvents()
{
    MsgSendPulse(m_coid, SIGEV_PULSE_PRIO_INHERIT, SENSOR_MSG_PULSE, 0);
    if (m_thread) {
        pthread_join(m_thread, NULL);
        m_thread = 0;
        m_sensorsEnabled = false;
    }
}

void SensorsNDK::StartSensor(SensorConfig *config)
{
    MUTEX_LOCK();
    if (m_sensorsEnabled) {
        const SensorTypeMap::iterator findSensor = _sensorTypeMap.find(config->sensor);
        if (findSensor != _sensorTypeMap.end()) {
            sensor_type_e sensorType = static_cast<sensor_type_e>(findSensor->second);

            const ActiveSensorMap::iterator findActiveSensor = m_activeSensors.find(sensorType);
            if (findActiveSensor == m_activeSensors.end()) {
                sensor_t *sensor = NULL;
                sensor = sensor_new(sensorType);
                SIGEV_PULSE_INIT(&m_sigEvent, m_coid, SIGEV_PULSE_PRIO_INHERIT, SENSOR_BASE_PULSE + sensorType, sensor);
                sensor_event_notify(sensor, &m_sigEvent);
                sensor_set_delay(sensor, config->delay);
                sensor_set_queue(sensor, config->queue);
                sensor_set_batching(sensor, config->batching);
                sensor_set_background(sensor, config->background);
                sensor_set_reduced_reporting(sensor, config->reducedReporting);
                m_activeSensors[sensorType] = sensor;
            }
        }
    }
    MUTEX_UNLOCK();
}

void SensorsNDK::StopSensor(std::string sensor)
{
    MUTEX_LOCK();
    if (m_sensorsEnabled) {
        const SensorTypeMap::iterator findSensor = _sensorTypeMap.find(sensor);
        if (findSensor != _sensorTypeMap.end()) {
            stopActiveSensor(static_cast<sensor_type_e>(findSensor->second));
        }
    }
    MUTEX_UNLOCK();
}

void *SensorsNDK::SensorThread(void *args)
{
    Sensors *parent = reinterpret_cast<Sensors *>(args);
    fprintf(stderr, "event thread is running");
    struct _pulse pulse;

    // create channel for events
    m_sensorChannel = ChannelCreate(0);
    m_coid = ConnectAttach(ND_LOCAL_NODE, 0, m_sensorChannel, _NTO_SIDE_CHANNEL, 0);

    m_sensorsEnabled = true;

    for (;;) {
        MUTEX_UNLOCK();
        if (EOK == MsgReceivePulse(m_sensorChannel, &pulse, sizeof(pulse), NULL)) {
        MUTEX_LOCK();
            if (pulse.code == SENSOR_MSG_PULSE) {
                ActiveSensorMap::iterator it;
                for (it = m_activeSensors.begin(); it != m_activeSensors.end(); ++it) {
                    stopActiveSensor(static_cast<sensor_type_e>((*it).first));
                    return NULL;
                }
            }
            Json::FastWriter writer;
            Json::Value root;
            std::string sensorType;
            sensor_event_t event;
            fprintf(stderr, "pulse type: %d\n", pulse.type);
            sensor_t *sensor = static_cast<sensor_t *>(pulse.value.sival_ptr);
            sensor_get_event(sensor, &event);

            std::string accuracy;
            switch (event.accuracy)
            {
                case SENSOR_ACCURACY_UNRELIABLE:
                    accuracy = "unreliable";
                    break;
                case SENSOR_ACCURACY_LOW:
                    accuracy = "low";
                    break;
                case SENSOR_ACCURACY_MEDIUM:
                    accuracy = "medium";
                    break;
                case SENSOR_ACCURACY_HIGH:
                    accuracy = "high";
                    break;
            }

            root["accuracy"] = accuracy;

            switch (event.type)
            {
                case SENSOR_TYPE_ACCELEROMETER:
                    sensorType = "accelerometer";
                    root["x"] = event.motion.dsp.x;
                    root["y"] = event.motion.dsp.y;
                    root["z"] = event.motion.dsp.z;
                    break;
                case SENSOR_TYPE_MAGNETOMETER:
                    sensorType = "magnetometer";
                    root["x"] = event.motion.dsp.x;
                    root["y"] = event.motion.dsp.y;
                    root["z"] = event.motion.dsp.z;
                    break;
                case SENSOR_TYPE_GYROSCOPE:
                    sensorType = "gyroscope";
                    root["x"] = event.motion.dsp.x;
                    root["y"] = event.motion.dsp.y;
                    root["z"] = event.motion.dsp.z;
                    root["temperature"] = event.motion.gyro.temperature;
                    break;
                case SENSOR_TYPE_COMPASS:
                    sensorType = "compass";
                    root["azimuth"] = event.compass_s.azimuth;
                    root["isFaceDown"] = event.compass_s.is_face_down;
                    break;
                case SENSOR_TYPE_PROXIMITY:
                    sensorType = "proximity";
                    root["distance"] = event.proximity_s.distance;
                    root["normalized"] = event.proximity_s.normalized;
                    break;
                case SENSOR_TYPE_LIGHT:
                    sensorType = "light";
                    root["illuminance"] = event.light_s.illuminance;
                    break;
                case SENSOR_TYPE_GRAVITY:
                    sensorType = "gravity";
                    root["x"] = event.motion.dsp.x;
                    root["y"] = event.motion.dsp.y;
                    root["z"] = event.motion.dsp.z;
                    break;
                case SENSOR_TYPE_LINEAR_ACCEL:
                    sensorType = "linear_acceleration";
                    root["x"] = event.motion.dsp.x;
                    root["y"] = event.motion.dsp.y;
                    root["z"] = event.motion.dsp.z;
                    break;
                case SENSOR_TYPE_ROTATION_VECTOR:
                    sensorType = "rotation_vector";
                    root["x"] = event.motion.dsp.x;
                    root["y"] = event.motion.dsp.y;
                    root["z"] = event.motion.dsp.z;
                    break;
                case SENSOR_TYPE_ORIENTATION:
                    sensorType = "orientation";
                    root["screen"] = event.orientation.screen;
                    root["face"] = event.orientation.face;
                    break;
                case SENSOR_TYPE_AZIMUTH_PITCH_ROLL:
                    sensorType = "azimuth_pitch_roll";
                    root["azimuth"] = event.apr.azimuth;
                    root["pitch"] = event.apr.pitch;
                    root["roll"] = event.apr.roll;
                    break;
                case SENSOR_TYPE_FACE_DETECT:
                    sensorType = "face_detect";
                    root["face_detect"] = event.face_detect_s.face_detect;
                    break;
                case SENSOR_TYPE_HOLSTER:
                    sensorType = "holster";
                    root["holstered"] = event.holster_s.holstered;
                    break;
                case SENSOR_TYPE_ROTATION_MATRIX:
                case SENSOR_TYPE_ALTIMETER:
                case SENSOR_TYPE_TEMPERATURE:
                case SENSOR_TYPE_PRESSURE:
                    // not supported
                    break;
            }

            sensor_event_notify_rearm(sensor);
            parent->NotifyEvent("onsensor " + sensorType + " " +  writer.write(root));
        }
    }
    return NULL;
}

void SensorsNDK::stopActiveSensor(sensor_type_e sensorType)
{
    const ActiveSensorMap::iterator findActiveSensor = m_activeSensors.find(sensorType);
    if (findActiveSensor != m_activeSensors.end()) {
        sensor_t *sensor = m_activeSensors[sensorType];
        sensor_delete(&sensor);
        m_activeSensors.erase(sensorType);
    }
}

void SensorsNDK::createSensorMap()
{
    _sensorTypeMap["accelerometer"] = SENSOR_TYPE_ACCELEROMETER;
    _sensorTypeMap["magnetometer"] = SENSOR_TYPE_MAGNETOMETER;
    _sensorTypeMap["gyroscope"] = SENSOR_TYPE_GYROSCOPE;
    _sensorTypeMap["compass"] = SENSOR_TYPE_COMPASS;
    _sensorTypeMap["proximity"] = SENSOR_TYPE_PROXIMITY;
    _sensorTypeMap["light"] = SENSOR_TYPE_LIGHT;
    _sensorTypeMap["gravity"] = SENSOR_TYPE_GRAVITY;
    _sensorTypeMap["linear_acceleration"] = SENSOR_TYPE_LINEAR_ACCEL;
    _sensorTypeMap["rotation_vector"] = SENSOR_TYPE_ROTATION_VECTOR;
    _sensorTypeMap["orientation"] = SENSOR_TYPE_ORIENTATION;
    _sensorTypeMap["rotation_matrix"] = SENSOR_TYPE_ROTATION_MATRIX;
    _sensorTypeMap["azimuth_pitch_roll"] = SENSOR_TYPE_AZIMUTH_PITCH_ROLL;
    _sensorTypeMap["face_detect"] = SENSOR_TYPE_FACE_DETECT;
    _sensorTypeMap["holster"] = SENSOR_TYPE_HOLSTER;
}
}

