#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<downyiji.h>
#include<QListWidgetItem>
#include<QStandardPaths>
#include<QSettings>
#include<QDesktopWidget>
#include<QProcess>
#include<QMessageBox>
#include<QWheelEvent>
#include<QTextCodec>
#include<QJsonObject>
#include<QJsonParseError>
#include<QJsonArray>
#include<QNetworkCookie>
#include<mycookiejar.h>

struct Taskitem
{
    QString name;
    QString  text;
    QString url;
    QString path;
    //并不需要referer和ua，url就是referer，ua内置就够用
//    QString referer;
    QString ua;
};
Q_DECLARE_METATYPE(Taskitem)



 inline QDataStream& operator<<(QDataStream& out, const Taskitem& item)
 {
     out<<item.name<<item.text<<item.url<<item.path;
     return out;
 }

 inline QDataStream& operator>>(QDataStream& in,Taskitem& item)
 {
     in>>item.name>>item.text>>item.url>>item.path;
    return in;
 }

 inline QDebug & operator<<(QDebug& out, const Taskitem& item)
 {
     out<<item.name<<item.text<<item.url<<item.path;
     return out;
 }


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:

private slots:
    void savecookie();
    void readcookie();
    void downinfo(QString text , QString& url);
    void downfinished( QString& url);
    void downstoped();
    void error(QString& url);

    void getlist();
    void getdmzjlist();

    void on_analyze_clicked();

    void on_finlist_itemDoubleClicked(QListWidgetItem *item);

    void on_inglist_itemDoubleClicked(QListWidgetItem *item);

    void on_pushButton_3_clicked();

    void on_todownload_clicked();

    void on_folder_clicked();

    void on_delete1_clicked();

    void on_inglist_itemSelectionChanged();

    void on_start_clicked();

    void on_stop_clicked();

    void on_delete2_clicked();

    void on_webView_urlChanged(const QUrl &arg1);

    void on_backpage_clicked();

    void on_forwardpage_clicked();

    void on_down_in_web_clicked();

    void on_fzdm_clicked();

    void on_dmzj_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_tabWidget_tabBarClicked(int index);

    void on_move_error_clicked();

private:
    Ui::MainWindow *ui;

    void savelist(bool saveinglist);
    void savecfg();//设置
    void readcfg();//设置
    void readlist(bool saveinglist);
    void getlistinfo(QString url);
    void checkfin();
    void addtask(QList<Taskitem>,bool saveinglist);
    void addtask(Taskitem,bool saveinglist);

    void init();
    void downitem(QListWidgetItem* ,downyiji*);
    QListWidgetItem* findingitem(QString& url);
    QListWidgetItem* findfinitem(QString& url);
    downyiji* finddownloader( QString url = QString());

  //  void wheelEvent(QWheelEvent *event);
    bool eventFilter(QObject *, QEvent *);

    QNetworkAccessManager manager,viewmanager;
    QNetworkReply *reply;
    QRegularExpression reg;

    QString downpath,cfgpath;
    QString user_agent;
    QString bookurl;
 //   int booknow,bookall;//记得必要时清空
 //   QStringList booklist;//记得必要时清空

    QList<downyiji *> downloader;
    QList<Taskitem> tasklist;
    bool manystop;
    bool checking;
    bool initlist;
    int mverror;
    QString tmpdmzjurl;
    mycookiejar* jar ;
    QUrl dmzjcss,fzdmcss;
};

#endif // MAINWINDOW_H
