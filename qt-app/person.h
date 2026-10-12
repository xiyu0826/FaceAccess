#ifndef PERSON_H
#define PERSON_H

#include <QString>
#include <opencv2/opencv.hpp>

// 人员信息：id + 姓名 + 人脸特征(128 维)
class Person
{
public:
    Person(int id, QString name, cv::Mat feature);

    int id() const;
    QString name() const;
    cv::Mat feature() const;

private:
    int _id;
    QString _name;
    cv::Mat _feature;
};

#endif // PERSON_H
