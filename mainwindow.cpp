#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //down = new downyiji(this);
    //this->setLayout(ui->verticalLayout);
    //down->seturl("http://manhua.fzdm.com/132/121/","/home/sheng/1");
    //down->start();

    ui->setupUi(this);
    user_agent = "Mozilla/5.0 (Linux; Android 4.0.4; Galaxy Nexus Build/IMM76B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/46.0.2490.76 Mobile Safari/537.36";
    downpath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)+"/WeiManhua/";
    cfgpath =  QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/WeiManhua/";
    QDir().mkpath(cfgpath);
    //qDebug()<<"https://images.dmzj.com/f/[C84]\u8299\u5170\u60f3\u8981\u53bb\u6b7b\/\u5168\u4e00\u8bdd_1510753564/00000018.jpg";
    this->setCentralWidget(ui->verticalLayoutWidget);
    ui->all_list->setLayout(ui->verticalLayout_4);
    ui->downed->setLayout(ui->verticalLayout_3);
    ui->downing->setLayout(ui->verticalLayout_2);
    ui->setting->setLayout(ui->verticalLayout_5);
    ui->browser->setLayout(ui->verticalLayout_6);
    ui->start->setText("全部开始");
    ui->stop->setText("全部暂停");
    manystop = false;
    checking = false ;
    mverror = 0 ;
    initlist = true;//读取下载列表时给addtask用的
    init();
    readcfg();
    ui->tabWidget->setCurrentIndex(0);
    ui->browser->setBackgroundRole(QPalette::Background);
    ui->browser->setAutoFillBackground(true);
     jar = new mycookiejar();
     ui->webView->installEventFilter(this);//注册
     ui->webView->page()->networkAccessManager()->setCookieJar(jar);
    ui->webView->settings()->setUserStyleSheetUrl(fzdmcss);
     ui->webView->settings()->enablePersistentStorage(cfgpath);
     ui->webView->settings()->setAttribute(QWebSettings::LocalStorageEnabled,true);
    ui-> webView->settings()->setLocalStoragePath(cfgpath);
    connect( ui-> webView ,SIGNAL(loadFinished(bool)),this,SLOT(savecookie()));
    readcookie();

    ui->webView->load(QUrl("http://manhua.fzdm.com/"));
    readlist(true);//读取正在下载列表
    readlist(false);//读取已完成列表
    initlist = false;//下载列表读完后就没用了
}

MainWindow::~MainWindow()
{
    savecfg();
    savelist(true);
    savelist(false);
    delete ui;
}

void MainWindow::init(){
    QFile file(cfgpath+"dmzj.css");
    if(file.exists()) dmzjcss = dmzjcss.fromLocalFile(cfgpath+"dmzj.css");
    else dmzjcss.setUrl("qrc:/css/dmzj.css");

    file.setFileName(cfgpath+"fzdm.css");
    if(file.exists()) fzdmcss = fzdmcss.fromLocalFile(cfgpath+"fzdm.css");
    else fzdmcss.setUrl("qrc:/css/fzdm.css");

    for(int i=0;i < 2;i++){
        downyiji  *down =new downyiji(this);
        connect(down,SIGNAL(error(QString&)),this,SLOT(error(QString&)));
        connect(down,SIGNAL(stoped()),this,SLOT(downstoped()));
        connect(down,SIGNAL(finished(QString&)),this,SLOT(downfinished(QString&)));
        connect(down,SIGNAL(info(QString,QString&)),this,SLOT(downinfo(QString,QString&)));
        downloader.append(down);
    }
    qDebug()<<"downloader inited";
}

void MainWindow::on_analyze_clicked()
{
    QString u = ui->lineEdit->text();
    if(u.isEmpty()) return;
    reg.setPattern("(?<=[^:])//+");
    u = u.replace(reg,"/");
    if(u.indexOf("http")==-1)
    {
        u="http://"+u;
    }
    //先加"http://"
    if(u.indexOf("fzdm")!=-1)
    {
        u = u.replace("www.fzdm.com/manhua","manhua.fzdm.com");
        if(u.at(u.length()-1)!='/') u.append('/');
        reg.setPattern("http://[^/]+?/[^/]+?/");
        u = reg.match(u).captured();
        if(u.isEmpty()) return;
    }
    //qDebug()<<u;
    else if(u.indexOf("www.dmzj.com/view/")!=-1||
       u.indexOf("www.dmzj.com/info/")!=-1 ){
        u = u.remove(QRegularExpression(".html.*"));
        u = "http://m.dmzj.com/info/"+u.split("/").at(4)+".html";
    }
    else if(u.indexOf(QRegularExpression("http://manhua.dmzj.com/[^/]+/{0,1}"))!=-1 ){
        u = "http://m.dmzj.com/info/"+u.split("/").at(3)+".html";
    }
    getlistinfo( u );
    ui->lineEdit->setText(u);
}

void MainWindow::getlistinfo(QString url)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    reply = manager.get(request);
    if(url.indexOf("dmzj")!=-1)
        connect(reply,SIGNAL(finished()),this,SLOT(getdmzjlist()));
    else connect(reply,SIGNAL(finished()),this,SLOT(getlist()));

    qDebug()<<"getlistinfo started";
}

void MainWindow::getlist(){
    //需要找到漫画名，然后列表里去除漫画名字

    if(reply->hasRawHeader("Location")){
        reply->deleteLater();
        QString url(reply->rawHeader("Location"));
        getlistinfo( url)  ;
        ui->lineEdit->setText(url);
        return;
    }
    QString html = reply->readAll();
    QString url = reply->url().toString();
    reply->deleteLater();
    QRegularExpressionMatch matched;
    //qDebug()<<html;
    reg.setPattern("book_name.+[\"']");
   matched  = reg.match(html);
   if(matched.hasMatch())
   {
       qDebug()<<"列表";
       //booklist.clear();
       //bookurl = "";
       QString name = matched.captured();
       reg.setPattern("(?<=content=[\"']).+?(?=[\"'])");
       matched = reg.match(name);
       if(matched.hasMatch()) name = matched.captured();
       else name = url;
       name = name.remove("/");//因为有可能做路径
       qDebug()<<name;
       ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->all_list),name);
       reg.setPattern("id=\"mhlistad\"[\\s\\S]+</li>");
       matched = reg.match(html);
       QString match ;
       if(matched.hasMatch())       match = matched.captured();
       else         return;
       match = match.remove(name);
       //qDebug()<<match;
       reg.setPattern("(?<=href=['\"]).+?['\"].+?['\"].+?(?=[\"'])");
       QRegularExpressionMatchIterator  matchs=  reg.globalMatch(match);

       if(matchs.hasNext()){
           ui->alllist->clear();
           QString tmp1;
           QString tmp2;
           QListWidgetItem *item ;
           while(matchs.hasNext()){
               QRegularExpressionMatch i = matchs.next();
               tmp1 = "";
               tmp2 = "";
               reg.setPattern("\".+\"");
               tmp1 = i.captured().split(reg).at(0);//href
               tmp2 = i.captured().split(reg).at(1);//title
               item = new QListWidgetItem(ui->alllist);
               item->setText(tmp2);//name
               item->setToolTip(tmp2);
               item->setWhatsThis(url + tmp1);//url
               item->setSizeHint(QSize(100,24));
               item->setTextAlignment(Qt::AlignCenter);
               ui->alllist->addItem(item);
           }
           ui->tabWidget->setCurrentWidget(ui->all_list);
           ui->tabWidget->tabBar()->setVisible(true);
       }
   }
   else{
       //下载单集
       reg.setPattern("og:title.+[\"']");
      matched  = reg.match(html);
      if(matched.hasMatch())
      {
          qDebug()<<"单集";
          QString name = matched.captured();
          qDebug()<<name;
          reg.setPattern("(?<=content=[\"']).+?(?=[\"'])");
          matched = reg.match(name);
          if(matched.hasMatch()) name = matched.captured();
          else name = url;
          name = name.remove("/");//因为有可能做路径
          Taskitem task;
          task.name = name;
          task.path = downpath + name+"/";
          task.url = url;
          task.ua = user_agent;
          qDebug()<<task.name<<task.url<<task.path;

          addtask(task ,true);
          checkfin();
      }
   }

}

void MainWindow::getdmzjlist(){
    //需要找到漫画名，然后列表里去除漫画名字

    if(reply->hasRawHeader("Location")){
        reply->deleteLater();
        QString url(reply->rawHeader("Location"));
        getlistinfo( url)  ;
        ui->lineEdit->setText(url);
        return;
    }
    QString html = reply->readAll();
    QString url = reply->url().toString();
    reply->deleteLater();
    QRegularExpressionMatch matched;
    qDebug()<<html;
    reg.setPattern("(?<=comicName\">).+?(?=<)");
   matched  = reg.match(html);
   qDebug()<<"dmzj:"<<matched.captured();
   if(matched.hasMatch())
   {
        QString name = matched.captured();
        if(name.isEmpty())  name = url;
        name = name.remove("/");//因为有可能做路径
        ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->all_list),name);
        reg.setPattern("(?<=initIntroData\\().+?(?=\\);)");//[......]
        matched = reg.match(html);
        QString match ;
        if(matched.hasMatch())       match = matched.captured();
        else         return;
        QJsonParseError json_error;
        QJsonDocument parse_doucment = QJsonDocument::fromJson(match.toLatin1(), &json_error);

        if(json_error.error == QJsonParseError::NoError)
        {
            ui->alllist->clear();
            QJsonObject obj;
            QJsonValue vale;
            QJsonArray arr2;
            int id, mhid ;
            QString text;
            QListWidgetItem *item ;

            QJsonArray arr = parse_doucment.array();
            for(int i=0;i<arr.count();i++)
            {
                obj = arr.at(i).toObject();
                vale = obj.take("data");
                arr2 = vale.toArray();
                for(int n=0;n<arr2.count();n++)
                {
                 obj = arr2.at(n).toObject();
                 vale = obj.value("id");
                 id = vale.toInt();
                 vale = obj.value("comic_id");
                 mhid = vale.toInt();
                 vale = obj.value("chapter_name");
                 text = vale.toString();
                 //qDebug()<<vale.toInt();
                 item = new QListWidgetItem(ui->alllist);
                 item->setText(text);//name
                 item->setToolTip(text);
                 item->setWhatsThis("http://m.dmzj.com/view/" + QString().setNum( mhid )+ "/" +QString().setNum( id)+".html");//url
                 item->setSizeHint(QSize(100,24));
                 item->setTextAlignment(Qt::AlignCenter);
                 ui->alllist->addItem(item);
                }
            }
            ui->tabWidget->setCurrentWidget(ui->all_list);
            ui->tabWidget->tabBar()->setVisible(true);
        }
        else    qDebug()<<"json eeror";

   }
   else{
       //下载单集
       if(html.indexOf("mReader.initData(")==-1) return;//不是漫画网页
       reg.setPattern("(?<=<title>).+?(?=-[^-]+-[^-]+)");
      matched  = reg.match(html);
      if(matched.hasMatch())
      {
          QString name = matched.captured();
          qDebug()<<"单集"<<name;
          if(name.isEmpty())   name = url;
          name = name.remove("/");//因为有可能做路径
          Taskitem task;
          task.name = name;
          task.path = downpath + name+"/";
          task.url = url;
          task.ua = user_agent;
         qDebug()<<task.name<<task.url<<task.path;
          addtask(task,true);
          checkfin();
      }
   }

}

void MainWindow::addtask(Taskitem t,bool saveinglist)
{
    if((findingitem(t.url)!=NULL)||(findfinitem(t.url)!=NULL)) return;//如果已经存在就跳过
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(QSize(0,40));
    item->setData(Qt::UserRole,QVariant().fromValue(t));
    item->setText(t.name+ t.text);
    item->setToolTip(t.url);
    if(initlist){
        if(t.text.indexOf("[E]")!=-1)
        {
            item->setWhatsThis("error");
            item->setIcon(QIcon(":/icon/err.png"));
        }
        else {
            item->setWhatsThis("stop");
            item->setIcon(QIcon(":/icon/stop.png"));
        }
    }
    else {
        item->setWhatsThis("ready");
        item->setIcon(QIcon(":/icon/ready.png"));
    }
    if(saveinglist) ui->inglist->addItem(item);
    else {
        item->setIcon(QIcon(":/icon/fin.png"));
        item->setWhatsThis("fin");
        //ui->finlist->addItem(item); //新的放后面
        ui->finlist->insertItem(0,item); //新的放前面
    }


}

void MainWindow::addtask(QList<Taskitem> list ,bool saveinglist)
{
    for(int i=0;i<list.length();i++)
    {
        Taskitem t = list.at(i);
        if(!t.path.isEmpty()) addtask(t , saveinglist);
       // qDebug()<<t.name<<i<<"/"<<list.length();
    }
//    savelist(true);
}

downyiji* MainWindow::finddownloader( QString url){
    qDebug()<<"findownloader:"<<url;
    for(int i=0;i<downloader.count();i++){
        downyiji* down = downloader.at(i);
        qDebug()<<"downloader:"<<i;
        if( (down->isruning()==url) ) return down;
    }
    return NULL;
}


void MainWindow::checkfin(){
    if(checking) return;
    checking = true;
    int all = ui->inglist->count();
    if(all==0) return;
    QListWidgetItem* item;
    for(int i=0;i<all;i++)
    {
        item = ui->inglist->item(i);
        if(item->whatsThis()=="ready"){
            downyiji  *down =NULL;
            down = finddownloader();
            //qDebug()<<down;
            if(down == NULL ) break;//别return，还要执行下面
            qDebug()<<"is ready,down it";
            downitem(item,down);
        }
    }
    //为什么要在这里移动item到finlist呢？
    //一是因为downitem()函数里极有可能触发finished，
    //二是因为上面我是直接操作inglist，
    //因此在触发了finished时,inglist的长度是会变化的，会导致上面的for循环出错（脑补，没验证，但应该没错）
    //所以只能在遍历完inglist，设好ing标记，再进行移动，以尽可能保证没事。
    //下面的for循环其实也有这个问题，但是我可以直接在这个函数里在删除项目时自减1解决。
    int old = all;
    for(int i=0;i<all;i++)
    {
        item = ui->inglist->item(i);
        if(item->whatsThis()=="fin"||(mverror==-1&&item->whatsThis()=="error")){
            int r = ui->inglist->row(item);
            item = ui->inglist->takeItem(r);
            //ui->finlist->addItem(item); //新的放后面
            ui->finlist->insertItem(0,item); //新的放前面
            all--;
            i--;
            if(mverror==-1&&item->whatsThis()=="error") mverror = 0;
            qDebug()<<"is fin,move to finlist";
        }
    }
    if(all != old){
        //列表已变化
        savelist(true);
        savelist(false);
    }
    checking = false;
    qDebug()<<"checkfin ed";

}



void MainWindow::on_finlist_itemDoubleClicked(QListWidgetItem *item)
{
    QVariant var = item->data(Qt::UserRole);
    Taskitem i = var.value<Taskitem>();
    QString cmd=  QApplication::applicationDirPath()+"/picture";
    if(QFile(cmd).exists())
    {
        qDebug()<<"findoubleclicked"<<cmd;
        QProcess pro;
        QStringList arg;
        arg<<i.path;
        pro.startDetached(cmd , arg);
    }
    else {
        on_folder_clicked();
    }
    //打开漫画
}

void MainWindow::on_inglist_itemDoubleClicked(QListWidgetItem *item)
{
    if(item->whatsThis()=="stop"||item->whatsThis()=="error") {
        item->setWhatsThis("ready");
        item->setIcon(QIcon(":/icon/ready.png"));
        checkfin();
    }
    else if(item->whatsThis()=="ready" || item->whatsThis()=="ing")
    {

        if(item->whatsThis()=="ing")
        {
            QVariant var = item->data(Qt::UserRole);
            Taskitem i = var.value<Taskitem>();
            downyiji  *down = NULL;
            down = finddownloader(i.url);
            if(down!=NULL) down->stop();

        }
        item->setWhatsThis("stop");
        item->setIcon(QIcon(":/icon/stop.png"));
    }
}

void MainWindow::downitem(QListWidgetItem *item,downyiji* down){
    qDebug()<<"down task start";
    QString tmp = item->text();
    QVariant var = item->data(Qt::UserRole);
    Taskitem i = var.value<Taskitem>();
    down->seturl(i.url,i.path);//这里会重置text，所以要先保存
    down->setua(user_agent);
    item->setWhatsThis("ing");
    item->setIcon(QIcon(":/icon/dl.png"));
    qDebug()<<"down task started"<<i.name;

    if(tmp.indexOf("[E]")!=-1) down->restart();
    else down->start();
}

void MainWindow::savecfg(){
    QSettings settings("WeiManhua","cfg");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.setValue("path",downpath);
    settings.setValue("UA",user_agent);
    qDebug()<<"savecfged";
}

void MainWindow::readcfg(){
    QSettings settings("WeiManhua","cfg");
    downpath = settings.value("path", downpath).toString();
    user_agent = settings.value("UA", user_agent).toString();
    if(downpath.at(downpath.length()-1)!='/') downpath.append('/');
    if(!QFile(downpath).isWritable()) downpath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)+"/WeiManhua/";
    ui->savepath->setText(downpath);
    QSize s = settings.value("size", QSize(880, 600)).toSize();
    int x=QApplication::desktop()->width()/2-s.width()/2;
    int y=QApplication::desktop()->height()/2-s.height()/2;
    setGeometry(x , y , s.width() , s.height() );
    qDebug()<<"readcfged";
}

void MainWindow::savelist(bool saveinglist){
    //inglist
    tasklist.clear();//清空先
    QString  path ;
    QListWidget *alist;
    if(saveinglist){
        path =  cfgpath + "inglist";
        alist = ui->inglist;
    }
    else {
        path =  cfgpath + "finlist";
        alist = ui->finlist;
    }
    QFile file(path);
    file.open(QIODevice::WriteOnly );
    QDataStream data;
    data.setDevice(&file);
    //totasklist
    Taskitem task;
    QListWidgetItem *item;
    for(int i=0;i<alist->count();i++)
    {
        item = alist->item(i);
        QVariant var = item->data(Qt::UserRole);
        task = var.value<Taskitem>();
        tasklist.append(task);
    }
    data<<tasklist;
    qDebug()<<"savelist"<<saveinglist<<tasklist;
    file.close();

}

void MainWindow::readlist(bool saveinglist){
    QString  path ;
    QListWidget *alist;
    if(saveinglist){
        path =  cfgpath + "inglist";
        alist = ui->inglist;
    }
    else {
        path =  cfgpath + "finlist";
        alist = ui->finlist;
    }
    QFile file(path);
   if( file.open(QIODevice::ReadOnly ))
   {
        QDataStream data;
        data.setDevice(&file);
        data>>tasklist;
        qDebug()<<"readlist"<<saveinglist<<tasklist;
        addtask(tasklist,saveinglist);
        file.close();
   }
}

void MainWindow::downinfo(QString text , QString& url){
    QListWidgetItem* item = findingitem(url);
    QVariant var = item->data(Qt::UserRole);
    Taskitem task = var.value<Taskitem>();
    item->setText(task.name+text);
    task.text = text;
    item->setData(Qt::UserRole,QVariant().fromValue(task));
}

void MainWindow::downfinished( QString& url){
    qDebug()<<"a task fined";
    QListWidgetItem* item = findingitem(url);
    item->setIcon(QIcon(":/icon/fin.png"));
    item->setWhatsThis("fin");
    qDebug()<<"a task fined,start next";
    if(!checking&&!manystop ) checkfin();
}

void MainWindow::downstoped(){

     if(!manystop) checkfin();//批量操作时不checkfin，手动check
}

QListWidgetItem* MainWindow::findingitem(QString& url){
    QString t;
    QListWidgetItem* it =NULL;
    for(int i=0;i<ui->inglist->count();i++){
        it = ui->inglist->item(i);
        t = it->toolTip();
        if(t==url) return it;
    }
    return NULL;
}
QListWidgetItem* MainWindow::findfinitem(QString& url){
    QString t;
    QListWidgetItem* it = NULL;
    for(int i=0;i<ui->finlist->count();i++){
        it = ui->finlist->item(i);
        t = it->toolTip();
        if(t==url) return it;
    }
    return NULL;

}

void MainWindow::error(QString& url){
    QListWidgetItem* item = findingitem(url);
    item->setWhatsThis("error");
    item->setIcon(QIcon(":/icon/err.png"));
    mverror =1;
    if(!manystop) checkfin();
}

void MainWindow::on_pushButton_3_clicked()
{
    ui->alllist->selectAll();
}

void MainWindow::on_todownload_clicked()
{
    ui->todownload->setEnabled(false);
    QList<QListWidgetItem* > list;
    QList<Taskitem> tklist;
    list = ui->alllist->selectedItems();
    QString name = ui->tabWidget->tabText(ui->tabWidget->indexOf(ui->all_list));
    //qDebug()<<"to add tasks";
    for(int i=0;i<list.length();i++)
    {
        QListWidgetItem* item = list.at(i);
        Taskitem task;
        task.name = name+" "+item->toolTip();
        task.path = downpath + name + "/" + item->toolTip()+"/";
        task.url = item->whatsThis();
        task.ua = user_agent;
        //qDebug()<<"down:"<<task.name<<task.url<<task.path;
        tklist.append(task);
      //  qDebug()<<task.name;
    }
    addtask(tklist,true);
    //qDebug()<<"tasks added";
    ui->todownload->setEnabled(true);
    checkfin();
    ui->tabWidget->setCurrentWidget(ui->downing);
}

void MainWindow::on_folder_clicked()
{
    QProcess pro;
    QString cmd = "xdg-open \""+ downpath +"\"";
    if(!ui->finlist->selectedItems().isEmpty()){
        QVariant var = ui->finlist->selectedItems().at(0)->data(Qt::UserRole);
        Taskitem task = var.value<Taskitem>();
        cmd = "xdg-open \""+ task.path +"\"";
    }
    pro.startDetached(cmd);
    qDebug()<<"cmd:"<<cmd;
}

void MainWindow::on_delete1_clicked()
{
    int result;
    result = QMessageBox::question(this,"删除已完成的任务？","是否同时删除文件？","取消","否","是，删除文件");
    if (result!=0) {
        QList<QListWidgetItem* > list = ui->finlist->selectedItems();
        QString cmd ;//= QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.local/share/Trash/files";//手动放回收站无用，文件重复时放不了。
        QProcess pro;
        while(list.length()>0)
        {
            int n = ui->finlist->row(list.at(0));
            ui->finlist->takeItem(n);
            if(result==2) {
                QVariant var = list.at(0)->data(Qt::UserRole);
                Taskitem task = var.value<Taskitem>();
                //如果有这个回收站目录，就放到用户目录的回收站，否则就删除
                QString ppath;
                QStringList list = task.path.split("/");
                ppath = list.last();
                if(ppath.isEmpty()) list.takeLast();
                list.takeLast();
                ppath = list.join('/');
                cmd = "rm -rf \""+ task.path +"\"";
                qDebug()<<cmd<<ppath;
                pro.start(cmd);
                pro.waitForFinished(3000);
                QDir dir(ppath);//自带的删除函数自能删除空文件夹的，用它来判断文件夹是否为空。
                 dir.rmpath(ppath);//删除上级空文件夹
            }
            list.takeFirst();
        }
        savelist(false);
    }
}

void MainWindow::on_inglist_itemSelectionChanged()
{
    if(ui->inglist->selectedItems().count()>0) {
        ui->start->setText("开始");
        ui->stop->setText("暂停");
    }
    else
    {
        ui->start->setText("全部开始");
        ui->stop->setText("全部暂停");
    }

}

void MainWindow::on_start_clicked()
{
    ui->start->setEnabled(false);
    if(ui->inglist->selectedItems().count()>0) {
        QList<QListWidgetItem* > list = ui->inglist->selectedItems();
        for(int i=0;i<list.length();i++)
        {
            //不是ing和ready和fin就设为ready
            if(list.at(i)->whatsThis()!="ing"&&list.at(i)->whatsThis()!="ready"&&list.at(i)->whatsThis()!="fin"){
                list.at(i)->setWhatsThis("ready");
                list.at(i)->setIcon(QIcon(":/icon/ready.png"));
            }
        }
    }
    else
    {
        int all = ui->inglist->count();
        QListWidgetItem *item;
        for(int i=0;i<all;i++)
        {
            //不是ing和ready和fin就设为ready
            item = ui->inglist->item(i);
            if(item->whatsThis()!="fin"&&item->whatsThis()!="ing"&&item->whatsThis()!="ready"){
                item->setWhatsThis("ready");
                item->setIcon(QIcon(":/icon/ready.png"));
            }
        }
    }
    ui->start->setEnabled(true);
    downyiji  *down =NULL;
    down = finddownloader();
    //qDebug()<<down;
    if(down == NULL ) return;
    else checkfin();
}

void MainWindow::on_stop_clicked()
{
    //qDebug()<<"================on_stop_clicked():stop==========";
    manystop = true;
    ui->start->setEnabled(false);
    if(ui->inglist->selectedItems().count()>0) {
        QList<QListWidgetItem* > list = ui->inglist->selectedItems();
        QListWidgetItem *item;
        qDebug()<<"inglist selected"<<list.length();
        for(int i=0;i<list.length();i++)
        {
            item = list.at(i);

            if(item->whatsThis()=="ing"){
                QVariant var = item->data(Qt::UserRole);
                Taskitem i = var.value<Taskitem>();
                downyiji  *down = NULL;
                down = finddownloader(i.url);
                if(down!=NULL) down->stop_nosignal();
            }
            else {
                item->setWhatsThis("stop");
                item->setIcon(QIcon(":/icon/stop.png"));
            }
        }
        checkfin();//批量暂停后启动下载
    }
    else
    {
        int all = ui->inglist->count();
        QListWidgetItem *item;

        for(int i=0;i<downloader.length();i++)
        {
            downloader.at(i)->stop_nosignal();
        }
        for(int i=0;i<all;i++)
        {
            item = ui->inglist->item(i);
            item->setWhatsThis("stop");
            item->setIcon(QIcon(":/icon/stop.png"));
        }
    }
    manystop = false;

    ui->start->setEnabled(true);
}

void MainWindow::on_delete2_clicked()
{
    manystop = true;//防止删除时停止任务触发其他要删除的任务下载
    int result;
    result = QMessageBox::question(this,"删除任务？","是否同时删除文件？","取消","否","是，删除文件");
    if (result!=0) {
        QList<QListWidgetItem* > list = ui->inglist->selectedItems();
        QString cmd ;//= QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.local/share/Trash/files";//同上
        QProcess pro;
        if(list.length()>0) on_stop_clicked();//避免触发全部暂停
        while(list.length()>0)
        {
            if(result==2) {
                QVariant var = list.at(0)->data(Qt::UserRole);//因为是即时删除item所以永远都是第一个
                Taskitem task = var.value<Taskitem>();
                QString ppath;
                QStringList list = task.path.split("/");
                ppath = list.last();
                if(ppath.isEmpty()) list.takeLast();
                list.takeLast();
                ppath = list.join('/');
                cmd = "rm -rf \""+ task.path +"\"";
                qDebug()<<cmd;
                pro.startDetached(cmd);
                pro.waitForFinished(3000);
                QDir dir;//自带的删除函数自能删除空文件夹的，用它来判断文件夹是否为空。
                dir.rmpath(ppath);//删除上级空文件夹
            }
            if(list.at(0)->whatsThis()=="ing") on_inglist_itemDoubleClicked(list.at(0));
            int n = ui->inglist->row(list.at(0));
            ui->inglist->takeItem(n);//takeitem还是放在最后吧。
            list.takeFirst();
        }
        savelist(true);
    }
    manystop = false;
}

void MainWindow::on_webView_urlChanged(const QUrl &arg1)
{
   // ui->webView->settings()->setUserStyleSheetUrl(QUrl("qrc:/css/my.css"));
    if(arg1==QUrl("http://manhua.fzdm.com/")) ui->down_in_web->setVisible(false);
    else ui->down_in_web->setVisible(true);
    reg.setPattern("http://m.dmzj.com/info/[^/]+?\\.html");
    if(arg1.toString().indexOf(reg)!=-1){
        //是dmzj列表页面就先保存
        tmpdmzjurl = arg1.toString();
    }
}


//void MainWindow::wheelEvent(QWheelEvent *event){
//    if(ui->tabWidget->currentIndex()==0){
//        if(event->delta() > 0){
//             ui->tabWidget->tabBar()->setVisible(true);
//         }else{
//            ui->tabWidget->tabBar()->setVisible(false);
//         }
//    }
//    else ui->tabWidget->tabBar()->setVisible(true);
//}

bool MainWindow::eventFilter(QObject *object, QEvent *event){
    if( event->type() == QEvent::Wheel){
        //qDebug()<<"eventfilter:Wheel";
        QWheelEvent* e = static_cast<QWheelEvent*>(event);//强制转型
        if(ui->tabWidget->currentIndex()==0){
            if(e->delta() > 0){
                 ui->tabWidget->tabBar()->setVisible(true);
             }else{
                ui->tabWidget->tabBar()->setVisible(false);
             }
        }
    }
    return false;//还需要原目标处理
    //event处理后返回true表示已经处理该事件，返回false则qt会将event发给原目标
}

void MainWindow::on_backpage_clicked()
{
    ui->webView->back();

}

void MainWindow::on_forwardpage_clicked()
{
    ui->webView->forward();

}

void MainWindow::on_down_in_web_clicked()
{
    QString url = ui->webView->url().toString();
    if(url.indexOf("fzdm")!=-1){
        reg.setPattern("http://[^/]+?/[^/]+?/");
        url = reg.match(url).captured();
        if(url.isEmpty()) return;
    }
    else if(ui->webView->url().toString().indexOf(QRegularExpression("http://m.dmzj.com/view/[^/]+/[^/]+.html"))!=-1){
        url = tmpdmzjurl;
        //如果是在DMZJ看漫画界面就用保存的列表url
    }
    getlistinfo(url);
}

void MainWindow::on_fzdm_clicked()
{
    ui->webView->load(QUrl("http://manhua.fzdm.com"));
    ui->webView->settings()->setUserStyleSheetUrl(fzdmcss);
}

void MainWindow::on_dmzj_clicked()
{
    ui->webView->load(QUrl("https://m.dmzj.com/latest.html"));
    ui->webView->settings()->setUserStyleSheetUrl(dmzjcss);


}

void MainWindow::on_pushButton_clicked()
{
    ui->savepath->setText(downpath);
    ui->useragent->clear();
}

void MainWindow::on_pushButton_2_clicked()
{
    if(!ui->savepath->text().isEmpty()){
        downpath = ui->savepath->text();
        if(downpath.at(downpath.length()-1)!='/') downpath.append('/');
        if(!QFile(downpath).isWritable()) downpath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)+"/WeiManhua/";
        ui->savepath->setText(downpath);
    }
    if(!ui->useragent->text().isEmpty())    user_agent = ui->useragent->text();
}

void MainWindow::on_tabWidget_tabBarClicked(int index)
{
    ui->alllist->clearSelection();
    ui->inglist->clearSelection();
    ui->finlist->clearSelection();
}

void MainWindow::on_move_error_clicked()
{
    if(mverror>0){
        mverror = -1;
        checkfin();
    }

}



void MainWindow::savecookie()
{
  //  jar =  webview.page()->networkAccessManager()->cookieJar();
   QList  <QNetworkCookie  >   list =jar->getallCookies();
 //  qDebug()<<list.length()<<list;
   QFile file(cfgpath+"cookie.save");
   file.open(QIODevice::WriteOnly);
   QTextStream  data(&file);
   foreach (QNetworkCookie one, list) {
       data<<one.toRawForm()<<endl;
   }
   //data<<list;
   file.close();
}

void MainWindow::readcookie()
{
   QFile file(cfgpath+"cookie.save");
  if( file.open(QIODevice::ReadOnly)){
        QTextStream  data(&file);
        QString t;
        QStringList list;
        while(!data.atEnd())
        {
            t = data.readLine();
            list = t.split("; ");
            QNetworkCookie cookie;
            for(int i=0;i<list.length();i++){
                QString name = list.at(i);
                 QString value;
                QStringList l = name.split("=");
                for(int j =0;j<l.length();j++){
                    if(j==0) name = l.at(0);
                    if(j==1) value  = l.at(1);
                }
                if(name=="domain") cookie.setDomain(value);
                else    if(name=="expires") cookie.setExpirationDate(QDateTime().fromString(value));
                else      if(name=="path") cookie.setPath(value);
                else  if(name == "HttpOnly") cookie.setHttpOnly( true);
               else {
                    cookie.setName( name.toLatin1());
                     cookie.setValue( value.toLatin1());
                //   qDebug()<<"zidingyi"<<name<<value<<cookie;
                }
            }
         //   qDebug()<<cookie<<endl;
            jar->insertCookie(cookie);
          //  qDebug()<<"readcookie:"<<list;
        }
        file.close();
    }
}
