#include <stdio.h>
#include "qobj.hpp"

MyQObj::MyQObj() {}

void MyQObj::process()
{
    fprintf(stderr, "I am in process!%s\n", "");
    emit finished();
}

void MyQObj::finished()
{
    fprintf(stderr, "I am in finished!%s\n", "");
}