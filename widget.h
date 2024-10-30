#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QNetworkReply>
#include <QPoint>
#include "day.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    day weaDays[7];
    Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void getHttpReply();
    void parseWeatherJson(QByteArray data);
    void requstReply();
    void getCityId(QString searchString);
    QTimer *qTimerDate;
    void openJsonFile();
    void mapImgSet();
    void parseWeatherJsonDate(QByteArray data);
    void sevenDaysDate();
    bool eventFilter(QObject *watched,QEvent *event) override;
    void paintLineH();
    void paintLineL();
private slots:
    void on_pushButton_clicked();

private:
    Ui::Widget *ui;
    QMenu *qMenuQuit;//退出菜单对象
    QPoint qPointMove;//窗口移动偏移值
    QNetworkReply *qNetworkReply;//设置网络请求响应对象
    QUrl qUrl;//设置访问的api网址(单日)
    QUrl qUrlDate;//设置访问的api网址(七日)
    std::map<QString,QString> mapCity;//城市id map
    std::map<QString,QString> mapImg;//图片map
};
#endif // WIDGET_H
