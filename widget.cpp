#include "widget.h"
#include "ui_widget.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPoint>
#include <QTimer>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setFixedSize(370,770);
    this->setWindowTitle("小张的天气预报");
    this->setWindowIcon(QIcon(":/res/weather_icon_sun.svg"));
    //初始化图片map
    mapImgSet();
    //设置右键打开退出菜单
    qMenuQuit = new QMenu(this);
    qMenuQuit->setStyleSheet("QMenu::item { font-weight: bold; color: white; background-color: red; }");//设置菜单样式
    qMenuQuit->setFont(QFont("Arial", 10));//设置菜单字体
    QAction *qActionQuit = new QAction(tr("退出本程序"),this);//设置一个退出行动
    qMenuQuit->addAction(qActionQuit);//添加到qMenuQuit菜单下
    connect(qMenuQuit,&QMenu::triggered,this,[=]{//连接qMenuQuit触发时,执行关闭窗口操作
        this->close();
    });

    //打开城市json文件，并且将键值对存入map中，从而实现打开程序后只打开一次文件
    openJsonFile();

    //初始化api地址
    qUrl = QUrl("http://gfeljm.tianqiapi.com/api?unescape=1&version=v61&appid=33178861&appsecret=u1OWkwtY");
    qUrlDate = QUrl("http://gfeljm.tianqiapi.com/api?unescape=1&version=v9&appid=33178861&appsecret=u1OWkwtY");
    //设置刷新显示，一分钟一刷新
    requstReply();//打开后直接执行一次，之后再刷新
    qTimerDate = new QTimer(this);
    connect(qTimerDate,&QTimer::timeout,this,&Widget::requstReply);
     qTimerDate->start(60000);

    //给高低温度曲线图widget注册事件过滤器
    ui->widget_TempH->installEventFilter(this);
    ui->widget_TempL->installEventFilter(this);
    ui->widget_TempH->update();
    ui->widget_TempL->update();
}


Widget::~Widget()
{
    delete ui;
}

void Widget::mousePressEvent(QMouseEvent *event)//设置鼠标按下事件
{
    if(event->button()==Qt::RightButton){//鼠标右键按下时打开退出菜单
        // qDebug()<<"1";
        qMenuQuit->exec(QCursor::pos());//位置设置为鼠标当前位置,在当前位置执行触发qMenuQuit菜单事件。
    }

    if(event->button()==Qt::LeftButton){//左键拖动窗口
        qPointMove = event->globalPosition().toPoint() - this->pos();//窗口左上角位置this->pos的，当前鼠标在的位置event->globalPosition().toPoint()
        //两者相减得到偏差值，通用这个偏差值设置窗口拖动后的位置。event->globalPosition().toPoint() 在QT6中必须通过转换，将QPointF转为QPoint
    }
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPosition().toPoint() - qPointMove);//根据当前鼠标位置和偏差值设置拖动后的窗口位置。
}
void Widget::parseWeatherJson(QByteArray data){//解析Json数据
    QJsonDocument qJsonDocument = QJsonDocument::fromJson(data);//创建QJsonDocument对象接收Json数据
    if(!qJsonDocument.isNull() && qJsonDocument.isObject()){//判断是否为空以及是否是JsonObject
        QJsonObject qJsonObject = qJsonDocument.object();//获取QJsonObject对象
        QString week = qJsonObject["week"].toString();//获取指定键的值
        QString date = qJsonObject["date"].toString();
        ui->label_Date->setText(date+" "+week);//设置日期

        QString city = qJsonObject["city"].toString();
        ui->label_location->setText(city);//设置城市位置

        QString wea = qJsonObject["wea"].toString();
        ui->label_WeatherTpye->setText(wea);//设置当前天气
        ui->labelWeatherIcon->setPixmap(mapImg[wea]);//设置当前天气图片

        QString update_time = qJsonObject["update_time"].toString();
        ui->label_time->setText("数据更新时间："+update_time);//设置数据更新时间

        QString tem = qJsonObject["tem"].toString();
        ui->label_WeatherCurrentTemp->setText(tem+"℃");//设置当前温度

        QString tem1 = qJsonObject["tem1"].toString();
        QString tem2 = qJsonObject["tem2"].toString();
        ui->label_WeatherTemp->setText(tem2+"-"+tem1+"℃");//设置最高最低温度

        QString win = qJsonObject["win"].toString();
        ui->label_FxTitle->setText(win);//设置风向

        QString win_speed = qJsonObject["win_speed"].toString();
        ui->label_FxType->setText(win_speed);//设置风速

        QString humidity = qJsonObject["humidity"].toString();
        ui->label_SdType->setText(humidity);//设置湿度

         QString air_pm25 = qJsonObject["air_pm25"].toString();
        ui->label_Type_5->setText(air_pm25);//设置pm2.5指数

        QString air_level = qJsonObject["air_level"].toString();
        ui->label_Type_6->setText(air_level);//设置空气质量

        QString air_tips = qJsonObject["air_tips"].toString();
        ui->labelGanMao->setText(air_tips);//设置指南
    }
}

void Widget::requstReply()//请求获取网络响应数据函数
{

    //设置网络响应请求
    QNetworkAccessManager *qNetworkAM = new QNetworkAccessManager(this);//创建一个可以发起网络请求的对象。这个对象可以发送请求到网络服务器，并管理响应。
    connect(qNetworkAM,&QNetworkAccessManager::finished,this,[](){//QNetworkReply 的 finished 信号是在网络请求完成时发出的，无论请求成功还是失败。当这个信号被触发时，
        //意味着 QNetworkReply 对象关联的网络操作（如 GET 或 POST 请求）已经结束，并且所有的数据已经被接收。
        qDebug()<<"finished";
    });

    QNetworkRequest res(qUrlDate);//指定请求的url地址。
    qNetworkReply = qNetworkAM->get(res);//发起一个 HTTP GET 请求,请求的是上文封装好的网络请求
    connect(qNetworkReply,&QNetworkReply::finished,this,&Widget::getHttpReply);//将这个Get请求响应和槽函数getHttpReply绑定，即请求完成时执行该槽函数，进行数据读取
}


void Widget::getHttpReply()//Get响应请求槽函数
{
    int resCode = qNetworkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();//获取状态码
    if(qNetworkReply->error()==QNetworkReply::NoError && resCode==200){//错误检测，如果请求没错误，且状态码为200，则输出数据
        QByteArray data = qNetworkReply->readAll();//存储获取的json数据
        parseWeatherJsonDate(data);//传入解析函数进行解析

        // QString replyMessage = QString::fromUtf8(qNetworkReply->readAll());//从 QNetworkReply 对象中读取网络请求的响应数据，并将其转换为 QString 类型的对象
        // qDebug()<<replyMessage;
    }else{//否则设置提示框，并且显示错误码
        // QMessageBox::warning(this,"接收失败",QString("错误码为%1").arg(resCode),QMessageBox::Ok);

        QMessageBox errMsg;
        errMsg.setWindowTitle("请求错误");
        errMsg.setText(QString("错误码：%1!").arg(resCode));
        errMsg.setStyleSheet("QPushButton {color:white}");
        errMsg.setStandardButtons(QMessageBox::Ok);
        errMsg.exec();
    }
}


void Widget::getCityId(QString searchString)//根据搜索栏用户输入搜索结果，切换城市天气信息
{
    bool isEndsWithShi = searchString.endsWith("市");//判断最后一位是不是‘市’
    if(isEndsWithShi){//如果是则删除它
        searchString.chop(1);//删除最后一位字符
    }
    qDebug()<<mapCity[searchString];
    if(!mapCity[searchString].isNull()){//如果城市map中包含该城市

            QString cityid = mapCity[searchString];//获取到该城市对应的城市id
            qUrl = QUrl("http://gfeljm.tianqiapi.com/api?unescape=1&version=v61&appid=33178861&appsecret=u1OWkwtY&cityid="+cityid);//将该id拼接到url，从而可以切换api地址
            qUrlDate = QUrl("http://gfeljm.tianqiapi.com/api?unescape=1&version=v9&appid=33178861&appsecret=u1OWkwtY&cityid="+cityid);;
            requstReply();//重新接收新城市的数据，刷新
            return;
    }else{
        QMessageBox errMsg;//如果找不到对应城市，执行该提示框
        errMsg.setWindowTitle("搜索错误");
        errMsg.setText(QString("找不到该城市"));
        errMsg.setStyleSheet("QPushButton {color:black}");
        errMsg.setStandardButtons(QMessageBox::Ok);
        errMsg.exec();
    }
}

void Widget::openJsonFile()//设置打开城市json文件，并且将键值对存入map中，从而实现打开程序后只打开一次文件
{
    QFile file(":/cityid.json");//打开城市id对应城市名json文件
    file.open(QIODevice::ReadOnly);
    QByteArray resData = file.readAll();
    file.close();
    QJsonDocument cityDoc = QJsonDocument::fromJson(resData);//QJsonDocument对象接收json数据
    if(cityDoc.isArray()){//如果是一个Json数组
        QJsonArray jsonArray = cityDoc.array();//用QJsonArray数组来接收
        for(QJsonValue value : jsonArray){//QJsonValue遍历每一个JsonObject
            if(value.isObject()){//如果是json对象
                QString cityName = value["cityName"].toString();//获取城市名
                QString cityId = value["cityCode"].toString();//获取城市id
                mapCity[cityName] = cityId;//存入map中
            }
        }
    }else{
        QMessageBox errMsg;//如果json有误，执行该提示框
        errMsg.setWindowTitle("Err");
        errMsg.setText(QString("打开json文件失败"));
        errMsg.setStyleSheet("QPushButton {color:black}");
        errMsg.setStandardButtons(QMessageBox::Ok);
        errMsg.exec();
    }

}

void Widget::mapImgSet()//设置内容对应的icon图片
{
    mapImg["多云"] = ":/res/duoyun.svg";
    mapImg["晴"] = ":/res/weather_icon_sun.svg";
    mapImg["大雨"] = ":/res/dayu.svg";
    mapImg["大雪"] = ":/res/daxue.svg";
    mapImg["小雨"] = ":/res/xiaoyv.svg";
    mapImg["雨夹雪"] = ":/res/yvjiaxue.svg";
    mapImg["中雪"] = ":/res/zhongxue.svg";
    mapImg["雷阵雨"] = ":/res/leidian.svg";
    mapImg["阴"] = ":/res/yun.svg";
    mapImg["霾"] = ":/res/wuqi.svg";
}

void Widget::parseWeatherJsonDate(QByteArray data)//获取七日天气,设置今日天气情况数据
{
    QJsonDocument qJsonDocument = QJsonDocument::fromJson(data);//创建QJsonDocument对象接收Json数据
    if(!qJsonDocument.isNull() && qJsonDocument.isObject()){//判断是否为空以及是否是JsonObject
        QJsonObject qJsonObject = qJsonDocument.object();//获取QJsonObject对象
        if(qJsonObject.contains("data") && qJsonObject["data"].isArray()){

            QJsonArray qJsonArray = qJsonObject["data"].toArray();
            QString update_time = qJsonObject["update_time"].toString();
            QString city = qJsonObject["city"].toString();

            for(int i = 0;i < qJsonArray.size();i++){//循环将七天天气情况输入到day类中
                QJsonObject obj = qJsonArray[i].toObject();
                qDebug()<<obj["wea"]<<obj["date"];
                QString week1 = obj["week"].toString();
                QString date1 = obj["date"].toString();
                weaDays[i].week = week1;
                weaDays[i].date = date1;
                QString wea1 = obj["wea"].toString();
                weaDays[i].wea = wea1;
                QString tem_1 = obj["tem"].toString();
                weaDays[i].tem = tem_1;
                QString tem1_1 = obj["tem1"].toString();
                QString tem2_1 = obj["tem2"].toString();
                weaDays[i].tem1 = tem1_1;
                weaDays[i].tem2 = tem2_1;
                QString win1 = obj["win"][0].toString();
                weaDays[i].win = win1;
                QString win_speed1 = obj["win_speed"].toString();
                weaDays[i].win_speed = win_speed1;
                QString humidity1 = obj["humidity"].toString();
                weaDays[i].humidity = humidity1;
                QString air_pm251 = obj["air"].toString();
                weaDays[i].air_pm25 = air_pm251;
                QString air_level1 = obj["air_level"].toString();
                weaDays[i].air_level = air_level1;
                QString air_tips1 = obj["air_tips"].toString();
                weaDays[i].air_tips = air_tips1;
            }

            //设置控件数据
            ui->label_Date->setText(weaDays[0].date+" "+weaDays[0].week);//设置日期

            ui->label_WeatherTpye->setText(weaDays[0].wea);//设置当前天气
            ui->labelWeatherIcon->setPixmap(mapImg[weaDays[0].wea]);//设置当前天气图片

            ui->label_time->setText("数据更新时间："+update_time);//设置数据更新时间

            ui->label_WeatherCurrentTemp->setText(weaDays[0].tem+"℃");//设置当前温度

            ui->label_WeatherTemp->setText(weaDays[0].tem2+"-"+weaDays[0].tem1+"℃");//设置最高最低温度

            ui->label_FxTitle->setText(weaDays[0].win);//设置风向

            ui->label_FxType->setText(weaDays[0].win_speed);//设置风速

            ui->label_SdType->setText(weaDays[0].humidity);//设置湿度

            ui->label_Type_5->setText(weaDays[0].air_pm25);//设置pm2.5指数

            ui->label_Type_6->setText(weaDays[0].air_level);//设置空气质量

            ui->labelGanMao->setText(weaDays[0].air_tips);//设置指南

            ui->label_location->setText(city);//设置城市位置

            sevenDaysDate();//设置底部七日天气

            ui->widget_TempH->update();//画折线
            ui->widget_TempL->update();
        }

    }
}

void Widget::sevenDaysDate()//设置底部七日天气数据,多文本实现
{
    for(int i = 0;i <= 5;i++){
        int j = i+1;//符合label的命名格式1-6
        //多文本循环实现对每个label_day设置文本
        QString label_day = QString("label_day%1").arg(j);
        QLabel *qLabelDay = findChild<QLabel*>(label_day);
        qLabelDay->setText(weaDays[i].week);
        //多文本循环实现对每个label_date设置文本
        QString label_date = QString("label_date%1").arg(j);
        QLabel *qLabelDate = findChild<QLabel*>(label_date);
        qLabelDate->setText(weaDays[i].date.mid(5,2)+"."+weaDays[i].date.mid(8,2));//提取日期10.11这种格式
        //多文本循环实现对每个label_wea设置文本
        QString label_wea = QString("label_wea%1").arg(j);
        QLabel *qLabelWea = findChild<QLabel*>(label_wea);
        qLabelWea->setText(weaDays[i].wea);
        //多文本循环实现对每个label_weaicon设置图片
        QString label_weaicon = QString("label_weaicon%1").arg(j);
        QLabel *qLabelWeaIcon = findChild<QLabel*>(label_weaicon);
        qLabelWeaIcon->setPixmap(mapImg[weaDays[i].wea]);
        //多文本循环实现对每个label_AirQ设置文本
        QString label_AirQ = QString("label_AirQ%1").arg(j);
        QLabel *qLabelAirQ = findChild<QLabel*>(label_AirQ);
        qLabelAirQ->setText(weaDays[i].air_level);
        if(weaDays[i].air_level=="优"){//根据空气质量修改背景色以及格式
            qLabelAirQ->setStyleSheet("QLabel {color: rgb(255, 255, 255);background-color: rgb(0, 202, 40);border-radius:10px }");
        }else if(weaDays[i].air_level=="良"){
            qLabelAirQ->setStyleSheet("QLabel {color: rgb(255, 255, 255);background-color: rgb(255, 182, 79);border-radius:10px }");
        }else if(weaDays[i].air_level=="轻度污染"){
            qLabelAirQ->setStyleSheet("QLabel {color: rgb(255, 255, 255);background-color: rgb(255, 108, 35);border-radius:10px }");
        }else{
            qLabelAirQ->setStyleSheet("QLabel {color: rgb(255, 255, 255);background-color: red;border-radius:10px }");
        }

        //多文本循环实现对每个label_win设置文本
        QString label_win = QString("label_win%1").arg(j);
        QLabel *qLabelWin = findChild<QLabel*>(label_win);
        qLabelWin->setText(weaDays[i].win);
        //多文本循环实现对每个label_winsp设置文本
        QString label_winsp = QString("label_winsp%1").arg(j);
        QLabel *qLabelWinSp = findChild<QLabel*>(label_winsp);
        qLabelWinSp->setText(weaDays[i].win_speed);
    }

    ui->label_day1->setText("今天");
    ui->label_day2->setText("明天");
}

bool Widget::eventFilter(QObject *watched, QEvent *event)//绘制折线图过滤器
{
    if(watched == ui->widget_TempH && event->type() == QEvent::Paint){//如果观察对象是改widget且事件类型是paint类型
        paintLineH();//执行高温绘制函数
        return true;//事件已被处理
    }
    if(watched == ui->widget_TempL && event->type() == QEvent::Paint){
        paintLineL();
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

void Widget::paintLineH()//高温绘制函数
{

    QPainter qPainter(ui->widget_TempH);
    qPainter.setBrush(Qt::yellow);//画笔及颜色
    int mid = ui->widget_TempH->height()/2;//找到widgetTempH的半高线
    int ave = 0,sum = 0;
    for(int i =0;i<6;i++){
        sum += weaDays[i].tem1.toInt();
    }
    ave = sum/6;//计算高温平均值
    QPoint point[6];
    for(int i = 0;i < 6;i++){//画出六个点的位置
        QString labelAQ = QString("label_AirQ%1").arg(i+1);//找到上面空气质量控件，以他们的中线为横坐标基准
        QLabel *labelAirQ = findChild<QLabel *>(labelAQ);//多控件处理
        point[i].setX(labelAirQ->x() + labelAirQ->width()/2);//设置每个点的横坐标
        int offset = (weaDays[i].tem1.toInt()-ave)*2;//配置偏差值，为当天温度减去平均温度,比如26-20=6 20是平均温度，6是偏差值，所以直接让mid减去6就得到纵坐标，是向上的，因为Y越小，位置越高，和坐标轴相反
        qDebug()<<offset;
        point[i].setY(mid - offset);//设置Y位置
        qPainter.drawEllipse(QPoint(point[i]),2,2);

        QPoint tempP;
        tempP.setX(point[i].x()-7);
        tempP.setY(point[i].y()-7);
        qPainter.drawText(tempP,weaDays[i].tem1+"℃");
    }
    for(int i = 0;i < 5;i++){//画线
        qPainter.drawLine(point[i],point[i+1]);
    }
}

void Widget::paintLineL()//低温绘制函数
{
    QPainter qPainter(ui->widget_TempL);
    qPainter.setBrush(Qt::blue);//画笔及颜色
    int mid = ui->widget_TempL->height()/2;//找到widgetTempL的半高线
    int ave = 0,sum = 0;
    for(int i =0;i<6;i++){
        sum += weaDays[i].tem2.toInt();
    }
    ave = sum/6;//计算高温平均值
    QPoint point[6];
    for(int i = 0;i < 6;i++){//画出六个点的位置
        QString labelAQ = QString("label_AirQ%1").arg(i+1);//找到上面空气质量控件，以他们的中线为横坐标基准
        QLabel *labelAirQ = findChild<QLabel *>(labelAQ);//多控件处理
        point[i].setX(labelAirQ->x() + labelAirQ->width()/2);//设置每个点的横坐标
        int offset = (weaDays[i].tem2.toInt()-ave)*2;//配置偏差值，为当天温度减去平均温度,比如26-20=6 20是平均温度，6是偏差值，所以直接让mid减去6就得到纵坐标，是向上的，因为Y越小，位置越高，和坐标轴相反
        qDebug()<<offset;
        point[i].setY(mid - offset);//设置Y位置
        qPainter.drawEllipse(QPoint(point[i]),2,2);

        QPoint tempP;
        tempP.setX(point[i].x()-7);
        tempP.setY(point[i].y()-7);
        qPainter.drawText(tempP,weaDays[i].tem2+"℃");
    }
    for(int i = 0;i < 5;i++){//画线
        qPainter.drawLine(point[i],point[i+1]);
    }
}

void Widget::on_pushButton_clicked()//搜索点击按钮
{
    if(ui->lineEdit_Search->text()==""){//输入为空则提示
        qDebug()<<"searchErr";
    }else{
        getCityId(ui->lineEdit_Search->text());//执行搜索函数
    }
}

