#ifndef QOBJ_JS_HPP_
#define QOBJ_JS_HPP_

#include <QObject>
#include <json/value.h>
#include <string>

class Timezone;

struct TimezoneThreadInfo {
    Timezone *parent;
    Json::Value *jsonObj;
    std::string eventId;
};

class MyQObj : public QObject
{
	Q_OBJECT

public:
	explicit MyQObj(const TimezoneThreadInfo& info);
	virtual ~MyQObj() {}

public slots:
	void process();

signals:
	void finished();

private:
	TimezoneThreadInfo m_info;
};

#endif