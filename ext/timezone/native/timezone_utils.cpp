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

#include <bb/pim/calendar/CalendarService>
#include <bb/pim/calendar/CalendarSettings>
#include <bb/pim/calendar/Result>
#include <unicode/calendar.h>
#include <unicode/timezone.h>
#include <qDebug>

#include "timezone_utils.hpp"

QString TimezoneUtils::getCurrentTimezone()
{
    bb::pim::calendar::CalendarService service;
    return service.settings().timezoneDatabaseId(); // TODO This should work but it doesn't work, and PIM folks don't reply
}

int TimezoneUtils::offsetFromUtcToTz(QDateTime date, QString timezoneId, bool ignoreDstOffset, bool& error)
{
    int offset = 0;
    UErrorCode errorCode = U_ZERO_ERROR;

    // get timezone object
    TimeZone* tz = TimeZone::createTimeZone((UChar*) timezoneId.data());
    if (!tz) {
        error = true;
        return 0;
    }

    UnicodeString name;
    tz->getDisplayName(name);
    QByteArray string = timezoneId.toLocal8Bit();

    // get offset
    // cal takes ownership of tz - tried to delete it and got an error
    Calendar* cal = Calendar::createInstance(tz, errorCode);
    if (!cal || errorCode > 0) {
        error = true;
        return 0;
    }
    cal->set(date.date().year(), date.date().month(), date.date().day(), date.time().hour(), date.time().minute());
    UDate udate = cal->getTime(errorCode);
    if (errorCode > 0) {
        error = true;
        delete cal;
        return 0;
    }

    UBool isGmt = false;
    int32_t rawOffset = 0;
    int32_t dstOffset = 0;
    tz->getOffset(udate, isGmt, rawOffset, dstOffset, errorCode);
    if (errorCode > 0) {
        error = true;
        delete cal;
        return 0;
    }

    if (!ignoreDstOffset) {
        offset = rawOffset + dstOffset;
    } else {
        offset = rawOffset;
    }
    delete cal;

    offset /= 1000;
    error = false;

    return offset;
}

// from timezoneId to local
// returns offset in seconds
int TimezoneUtils::offsetFromTzToLocal(QDateTime date, QString timezoneId, bool& error) {
    bool utcToTzError = false;
    int utcToTz = offsetFromUtcToTz(date, timezoneId, false, utcToTzError);
    if (utcToTzError) {
        error = true;
        return 0;
    }

    int tzToUtc = -utcToTz;
    bool utcToLocalError = false;
    int utcToLocal = offsetFromUtcToTz(date, getCurrentTimezone(), false, utcToLocalError);
    if (utcToLocalError) {
        error = true;
        return 0;
    }
    int tzToLocal = tzToUtc + utcToLocal;
    error = false;
    return tzToLocal;
}

QDateTime TimezoneUtils::convertToLocalFromUtc(QDateTime date, bool ignoreDstOffset, QString timezoneId)
{
    if (timezoneId == "") {
        timezoneId = getCurrentTimezone();
    }

    bool utcToTzError = false;
    int offset = offsetFromUtcToTz(date, timezoneId, ignoreDstOffset, utcToTzError);

    QDateTime convertedDate = date.addSecs(offset);
    return convertedDate;
}

QDateTime TimezoneUtils::convertToTargetFromUtc(QDateTime date, bool ignoreDstOffset, QString targetTimezoneId, QString sourceTimezoneId)
{
    if (sourceTimezoneId == "") {
        //sourceTimezoneId = getCurrentTimezone();
        //fprintf(stderr, "Current time zone%s\n", sourceTimezoneId.toStdString().c_str());
        sourceTimezoneId = "America/New_York"; // TODO must hard-code current timezone for testing since getCurrentTimezone() does not work, before testing must set to Eastern Time
    }

    if (targetTimezoneId == "") {
        targetTimezoneId = sourceTimezoneId;
    }

    bool utcToTzError = false;
    int offset1 = offsetFromUtcToTz(date, sourceTimezoneId, ignoreDstOffset, utcToTzError);
    int offset2 = sourceTimezoneId != targetTimezoneId ? offsetFromUtcToTz(date, targetTimezoneId, ignoreDstOffset, utcToTzError) : offset1;

    QDateTime convertedDate = date.addSecs(offset1).addSecs(-(offset2 - offset1));
    return convertedDate;
}


QDateTime TimezoneUtils::convertToLocalTimezone(QDateTime date, QString timezoneId)
{
    if (timezoneId == getCurrentTimezone() || timezoneId == "") { // TODO revisit
        return date;
    }

    bool error = false;
    int offset = offsetFromTzToLocal(date, timezoneId, error);
    QDateTime convertedDate = date.addSecs(offset);
    return convertedDate;
}

QDateTime TimezoneUtils::convertFromLocalTimezone(QDateTime date, QString timezoneId)
{
    if (timezoneId == getCurrentTimezone() || timezoneId == "") { // TODO revisit
        return date;
    }

    bool error = false;
    int offset = offsetFromTzToLocal(date, timezoneId, error);

    QDateTime convertedDate = date.addSecs(-offset);
    return convertedDate;
}
