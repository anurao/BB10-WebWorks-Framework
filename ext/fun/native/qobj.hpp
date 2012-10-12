#ifndef QOBJ_JS_HPP_
#define QOBJ_JS_HPP_

#include <QObject>

class MyQObj : public QObject
{
	Q_OBJECT

public:
	explicit MyQObj();
	virtual ~MyQObj() {}

public slots:
	void process();

signals:
	void finished();
};

#endif