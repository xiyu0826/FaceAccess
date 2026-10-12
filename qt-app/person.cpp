#include "person.h"

Person::Person(int id, QString name, cv::Mat feature)
    : _id(id), _name(name), _feature(feature.clone())   // clone：深拷贝，避免和查询缓冲区共享内存
{
}

int Person::id() const
{
    return _id;
}

QString Person::name() const
{
    return _name;
}

cv::Mat Person::feature() const
{
    return _feature;
}
