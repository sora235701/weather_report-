#ifndef DAY_H
#define DAY_H

#include <QString>


class day
{
public:
    day();
    QString week;//星期
    QString date;//日期
    QString wea;//天气
    QString tem;//温度
    QString tem1;//高温
    QString tem2;//低温
    QString win;//风向
    QString win_speed;//风速
    QString humidity;//湿度
    QString air_pm25;//pm2.5
    QString air_level;//空气质量
    QString air_tips;//tips
};

#endif // DAY_H
