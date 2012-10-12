#include <stdio.h>
#include "qobj.hpp"
#include "timezone_js.hpp"

MyQObj::MyQObj(const TimezoneThreadInfo& info) : m_info(info)
{

}

void MyQObj::process()
{
    fprintf(stderr, "I am in process!%s\n", "");
    // call NotifyEvent
    m_info.parent->NotifyEvent(m_info.eventId, "123456");
    emit finished();
}