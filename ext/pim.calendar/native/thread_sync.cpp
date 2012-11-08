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

#include "thread_sync.hpp"

//pthread_mutex_t ThreadSync::_lock = PTHREAD_MUTEX_INITIALIZER;

ThreadSync::ThreadSync()
{
	pthread_mutex_init(&_lock, NULL);
}
int ThreadSync::MUTEX_LOCK()
{
    struct timespec abs_time;
    clock_gettime(CLOCK_REALTIME , &abs_time);
    abs_time.tv_sec += 30;
    return pthread_mutex_timedlock (&_lock, &abs_time);
}

int ThreadSync::MUTEX_UNLOCK()
{
    return pthread_mutex_unlock(&_lock);
}