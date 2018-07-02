#include "downyiji.h"



downyiji::downyiji( QObject *parent ) : QObject(parent)
{
    downing = false;
    reply = NULL;
    tostop = false;
    errored = 0;
    ua = "Mozilla/5.0 (Linux; Android 4.0.4; Galaxy Nexus Build/IMM76B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/46.0.2490.76 Mobile Safari/537.36";

}



void downyiji:: seturl(QString Url , QString Path){
    url = Url;//
    path = Path;//就是最终存放地址
    if(path.at(path.length()-1)!='/') path.append('/');

    qDebug()<<"path"<<path;
    init();    //url和path赋值后才能初始化
}


//void downyiji::setrefer(QString Referer){
//    referer = Referer;
//}

void downyiji::setua(QString user_agent){
    ua = user_agent;
}


void downyiji::start(){

    if((now == all)&&(all!=0)) {
        qDebug()<<now<<"/"<<all<<"the task is a fined work";
        downing = false;
        emit finished(url);
        return;
    }
    QDir().mkpath(path);//生产文件夹
    downing = true;
    if( list.isEmpty() && dmzjlist.isEmpty() ){
        getpagesinfo(url);
    }else{
        if(now >-1&& now <list.length()){
            if(dmzjlist.isEmpty()) getimginfo(list.at(now));
            else downimg(dmzjlist.at(now));
        }
        else qDebug()<<"now out of list";
    }
    qDebug()<<"downyiji::start() started";
}

void downyiji::restart(){
    now = 0;
    all = 0;
    list.clear();
    dmzjlist.clear();
    //这里别用init,因为init会读配置文件
    start();
}


void downyiji::stop(){
    stop_nosignal();
    emit stoped();
}

void downyiji::stop_nosignal(){
    tostop = true;
    if(reply!=NULL) reply->abort();
    savecfg();
    tostop = false;
}


void downyiji::init(){
    //init 里不能情况url 和 path!
    if(reply!=NULL) {
        reply = NULL;
        reply->deleteLater();
    }
    //初始数值
    now = 0;
    all = 0;
    list.clear();
    dmzjlist.clear();
    tostop = false;
    errored = 0;
    downing = false;
    readcfg();
    QString inf = QString().setNum(now)+"/"+QString().setNum(all);//now可以为0，all不需要加1
    emit info( inf ,url);
}

void downyiji::getpagesinfo(QString url){
    if(tostop) return;
    if(url.indexOf("dmzj") != -1 ) url.replace("http://" , "https://");
    QNetworkRequest request;
    request.setRawHeader(QByteArray("User-Agent"), ua.toLatin1());
    request.setRawHeader(QByteArray("Referer"), url.toLatin1());
    request.setUrl(QUrl(url));
    reply = manager.get(request);
    if(url.indexOf("fzdm")!=-1)    connect(reply,SIGNAL(finished()),this,SLOT(getpages()));
    else      connect(reply,SIGNAL(finished()),this,SLOT(getdmzjimgsurl()));
    qDebug()<<"getpagesinfo started";
}

void  downyiji::getimginfo(QString pageurl){
    if(tostop) return;
//    cookiejar = new QNetworkCookieJar;
//    manager.setCookieJar(cookiejar);
    if(pageurl.indexOf("dmzj") != -1 ) pageurl.replace("http://" , "https://");
    QNetworkRequest request;
    request.setRawHeader(QByteArray("User-Agent"), ua.toLatin1());
    request.setRawHeader(QByteArray("Referer"), url.toLatin1());
    request.setUrl(QUrl(pageurl));
    reply = manager.get(request);
    connect(reply,SIGNAL(finished()),this,SLOT(getimgurl()));
    qDebug()<<"getimginfo started";

}

void downyiji::downimg(QString imgurl){
    if(tostop) return;
    QNetworkRequest request;
    request.setUrl(QUrl(imgurl));
    request.setRawHeader(QByteArray("User-Agent"), ua.toLatin1());
    request.setRawHeader(QByteArray("Referer"), url.toLatin1());
     reply = manager.get(request);
    connect(reply,SIGNAL(finished()),this,SLOT(getimg()));
    qDebug()<<"downimg started";

}

void downyiji::checkfin(){
    QString inf ="  【 "+ QString().setNum(now)+"/"+QString().setNum(all)+" 】";//在now=all时刚好完成
    emit info( inf ,url);
    if(now<all)
    {
        int listall;
        listall = list.length()+dmzjlist.length();//如无意外其中一个为0，费事判断了。
        if(now>-1&&now<listall)
        {
            QString url;
            if(dmzjlist.isEmpty()){
                url = list.at(now);
                getimginfo(url);
            }
            else {
                url = dmzjlist.at(now);
                downimg(url);
            }
            qDebug()<<"nextimg:"<<(now+1)<<"/"<<all;
        }
    }
    else {
        if(errored!=0){
            qDebug()<<"down finished,but has error";
            inf = inf + " 缺 " + QString().setNum( errored )+ " 张图片！[E]";
            emit info( inf ,url);
            emit error(url);
        }
        else{
            downing = false;
            emit finished(url);
        }
        qDebug()<<"down finished";
    }
    savecfg();
}

void downyiji::readcfg(){
    QString name = path+"cfg";
    qDebug()<<"cfg:"<<name;
//    file = new QFile(name);
//   if( file->open(QIODevice::ReadOnly))
//   {
//        QDataStream in;
//        in.setDevice(file);
//        QString u,p;
//        in>>u>>p>>now>>all>>list>>ua;//>>referer>>ua;//相当于忽略cfg的u和p，使用传递进来的u和p
//        if(url.isEmpty()) url = u;//假如上级保存的u有问题
//        if(path.isEmpty()) path = p;//同上。
//        if(ua.isEmpty()) ua =     ua = "Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko (bigrats web browser  0.4.7.9r)";
//        qDebug()<<"readcfg:"<<url<<path<<now<<all<<list<<ua;//<<referer<<ua;
//        file->close();
//   }
   QSettings settings(name,QSettings::IniFormat);
   QString u,p;
   u = settings.value("url", "").toString();
   p = settings.value("path","").toString();
   if(url.isEmpty()) url = u;//假如上级保存的u有问题
   if(path.isEmpty()) path = p;//同上。
   now = settings.value("now",0).toInt();
   all = settings.value("all",0).toInt();
   list = settings.value("list",list).toStringList();
   ua = settings.value("ua",ua).toString();
   dmzjlist  = settings.value("dmzjlist",dmzjlist).toStringList();
   qDebug()<<"readcfged"<<url<<path<<now<<all;
   //init 里初始化了数值
}

void downyiji::savecfg(){
    QString name = path+"cfg";
//    file = new QFile(name);
//   if( file->open(QIODevice::WriteOnly))
//   {
//        QDataStream out;
//        out.setDevice(file);
//        out<<url<<path<<now<<all<<list<<ua;

//        file->close();
//   }
   QSettings settings(name,QSettings::IniFormat);
   settings.setValue("url", url);
   settings.setValue("path", path);
   settings.setValue("now",now);
   settings.setValue("all",all);
   settings.setValue("list",list);
   settings.setValue("ua",ua);
   settings.setValue("dmzjlsit",dmzjlist);
   qDebug()<<"saved-ownloader-cfg";
}

void  downyiji::getpages(){
    qDebug()<<"downyiji::getpages";
    if(reply->error()!=QNetworkReply::NoError) {
        downing = false;
        savecfg();
        if(!tostop) emit error(url);
        qDebug()<<"getpages() error";
        return;
    }
    QString html = reply->readAll();
   // qDebug()<<html;
    reply->deleteLater();
    reply = NULL;
    reg.setPattern("\"navigation\">[\\s\\S]+?</div>");
    QString match =  reg.match(html).captured();
    //qDebug()<<"match"<<match;
    reg.setPattern("(?<=href=['\"]).+?(?=[\"'])");
    QRegularExpressionMatchIterator  matchs=  reg.globalMatch(match);
      while(matchs.hasNext()){
        QRegularExpressionMatch i = matchs.next();
        QString tmp = i.captured();
        if(tmp.indexOf(QString(".."))!=-1) continue;
        if(list.indexOf(url+i.captured())==-1)        list.append(url + i.captured());//低效疯狂对比，只加入不存在的
       // qDebug() <<i.captured();
    }
  if(html.indexOf("最后一页了")!=-1){
    now = 0;
    all = list.length();
    qDebug()<<"getpagesinfo end"<<list;
    qDebug()<<"now/all:"<<now<<"/"<<all;
    savecfg();
    checkfin();
  }else{
    qDebug()<<"list.last():"<<list.last();
    qDebug()<<"查找最后一页";
      getpagesinfo(list.last());//不断翻页，找到最后一页
  }
}

void  downyiji::getimgurl( ){
    qDebug()<<"downyiji::getimgurl";
    if(reply->error()!=QNetworkReply::NoError) {
        haserror();
        return;
    }
    QString html = reply->readAll();
    //qDebug()<<html;
    //QList<QNetworkCookie> cookies =manager.cookieJar()->cookiesForUrl(reply->url());
    //QStringList cook;

//    for (int i = 0; i < cookies.count(); ++i)
//                 cook.append(cookies.at(i).toRawForm());//转为qstringlist
//    reg.setPattern("picHost.+");
//    qDebug()<<cook;
//    QString mhss = cook.at(cook.indexOf(reg));
     QString mhss ="";//不搞cookie了，直接为空；
    reply->deleteLater();
    reply = NULL;
    html.replace(" ","");
    reg.setPattern("(?<=mhurl=['\"]).+?(?=[\"'])");
    QString mhurl =  reg.match(html).captured();

    reg.setPattern("(?<=mhss=['\"]).+?(?=[\"'])");
     if(mhss=="") mhss= reg.match(html).captured();
     if(mhurl.indexOf("2015")!=-1||mhurl.indexOf("2016")!=-1||mhurl.indexOf("2017")!=-1||mhurl.indexOf("2018")!=-1){
     }else{
     mhss=mhss.replace("p1","p0");
     };
     QString mhpicurl="http://"+mhss+"/"+mhurl;
     if(mhurl.indexOf("http")!=-1){
     mhpicurl=mhurl;
     }
     qDebug()<<"img url:"<<mhpicurl;

     downimg(mhpicurl);

}

void  downyiji::getdmzjimgsurl( ){
    qDebug()<<"downyiji::getdmzjimgsurl( )";
    if(reply->error()!=QNetworkReply::NoError) {
        haserror();
        return;
    }
    QString html = reply->readAll();
    reg.setPattern("(?<=\"page_url\":)\\[.+?\\]");
    QRegularExpressionMatch match;
    match = reg.match(html);
    if(!match.hasMatch()) {
        downing = false;
        savecfg();
        emit error(url);
        qDebug()<<"getimgurl() error"<<html;
        return;
    }
    QString data = match.captured();
    QJsonDocument document;
    QJsonParseError json_error;
    document = QJsonDocument::fromJson(data.toLatin1(),&json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        QJsonArray arr = document.array();
        for(int i=0;i<arr.count();i++)
        {
            data = arr.at(i).toString();
            dmzjlist.append(data);
            qDebug()<<"url:"<<data;
        }
        now = 0;
        all = dmzjlist.length();
        downimg(dmzjlist.first());
    }
    else {
        qDebug()<<"json error";
    }

}
void  downyiji::getimg( ){
    qDebug()<<"downyiji::getimg:isstop?"<<tostop;
    if( reply->error()!=QNetworkReply::NoError) {
        qDebug()<<"downyiji::getimg";
        haserror();
        return;
    }
    QString ffm = reply->url().toString().split('.').last();
    QString name = path+QString().setNum( now )  + "." +  ffm;
    qDebug()<<"getimg file name:"<<name;
    file = new QFile(name);
    if(!file->exists())  {
        file->open(QIODevice::WriteOnly);
        file->write(reply->readAll());
    }
    file->close();//命名前先关闭
    QMimeType type = QMimeDatabase().mimeTypeForData(file);
    QString fm = type.preferredSuffix();
    if(fm != ffm){
        name = path+QString().setNum( now )  + "." +  fm;
        if(!file->rename(name)){
            if(QFile(name).remove()){
                if(!file->rename(name))   file->remove();
            }
            else file->remove();
        }
        qDebug()<<"getimg:rename"<<name;
    }
    now++;
    //下一页
    reply->deleteLater();
    qDebug()<<"getimg end";

    checkfin();
}

QString downyiji::isruning(){
    if( downing )   return url;
    else return QString();
}

void downyiji::haserror(){
    downing = false;
    savecfg();
    if(!tostop){
        //emit error(url);
        qDebug()<<"reply has error"<<now;
        errored++;
        now++;//跳过这张，下一页
        checkfin();
    }
    reply->deleteLater();
}
