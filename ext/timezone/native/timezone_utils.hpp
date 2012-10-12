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

#ifndef TIMEZONE_UTILS_H_
#define TIMEZONE_UTILS_H_

#include <QDateTime>
#include <QString>

class TimezoneUtils {
public:
    static QString getCurrentTimezone();
    static QDateTime convertToTargetFromUtc(QDateTime date, bool ignoreDstOffset, QString targetTimezoneId = "", QString sourceTimezoneId = "");
    static QDateTime convertToLocalFromUtc(QDateTime date, bool ignoreDstOffset, QString timezoneId = "");
    static QDateTime convertToLocalTimezone(QDateTime date, QString timezoneId = "");
    static QDateTime convertFromLocalTimezone(QDateTime date, QString timezoneId = "");

private:
    static int offsetFromUtcToTz(QDateTime date, QString timezoneId, bool ignoreDstOffset, bool& error);
    static int offsetFromTzToLocal(QDateTime date, QString timezoneId, bool& error);
};

#endif // TIMEZONE_UTILS_H_
