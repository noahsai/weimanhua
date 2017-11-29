#ifndef DOWNYIJI_H
#define DOWNYIJI_H

#include <QObject>

//#include"QtNetwork/QNetworkCookieJar"
//#include"QtNetwork/QNetworkCookie"
#include"QtNetwork/QNetworkAccessManager"
#include"QtNetwork/QNetworkReply"
#include"QtNetwork/QNetworkRequest"
#include"QRegularExpression"
#include"QRegularExpressionMatch"
#include"QFile"
#include"QDir"
#include"QDataStream"
#include<QSettings>
#include<QJsonArray>
#include<QJsonValue>
#include<QJsonDocument>
#include<QJsonParseError>
#include<QMimeDatabase>
#include<QMimeType>
class downyiji : public QObject
{
    Q_OBJECT
public:
    explicit downyiji(QObject *parent = nullptr    );
    void seturl(QString Url ,QString Path);
    //用不到自定义referer和ua
//    void setrefer(QString Referer);
    void setua(QString user_agent);
    void start();
    void restart();
    void stop();
    void stop_nosignal();
    QString isruning();

signals:
    void info(QString text , QString& url);
    void finished( QString& url);
    void error(QString& url);
    void stoped();

private  slots:
    void getpages();
    void getimgurl( );
    void getdmzjimgsurl();
    void getimg( );
   //void getlist();

private:
    void init();
    void getpagesinfo(QString url);//首页url
    void getimginfo(QString pageurl);//pageurl为某页的url
    void downimg(QString imgurl);
    void checkfin();
    void readcfg();
    void savecfg();
    void haserror();
//    void getlistinfo(QString url);

    QNetworkAccessManager manager;
    QNetworkReply *reply;
    //QNetworkCookieJar* cookiejar;
    QString url;
    QString path;
    QString ua;
    //需要手动重置
    int now;
    int all;
    QStringList list,dmzjlist;
    bool downing;
    bool tostop;
    int errored;
    //====
    QRegularExpression reg;
    QFile *file;

};

#endif // DOWNYIJI_H
