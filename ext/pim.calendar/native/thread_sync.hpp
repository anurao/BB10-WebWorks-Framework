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

#ifndef PIM_CAL_THREAD_SYNC_HPP_
#define PIM_CAL_THREAD_SYNC_HPP_

#include <time.h>
#include <pthread.h>

class ThreadSync {
public:
	ThreadSync();
	virtual ~ThreadSync(){pthread_mutex_destroy(&_lock);}
protected:
    int MUTEX_LOCK();
    int MUTEX_UNLOCK();
    pthread_mutex_t _lock;
};

#endif // PIM_CAL_THREAD_SYNC_HPP_
