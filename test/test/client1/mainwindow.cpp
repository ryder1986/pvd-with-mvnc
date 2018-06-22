#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
     running=false;
    focus_index=0;
    mode=DISPLAY_NORMAL_MODE;
    picked=false;
    ui->setupUi(this);
    src=new VideoSource("rtsp://192.168.1.97:554/av0_1");
    connect(&tmr,SIGNAL(timeout()),this,SLOT(timeup()));
    connect(&searcher,SIGNAL(find_ip(QString)),this,SLOT(ip_found(QString)));
    connect(&clt,SIGNAL(get_ret(QByteArray)),this,SLOT(server_msg(QByteArray)));
    connect(&clt,SIGNAL(send_done(QByteArray)),this,SLOT(sended(QByteArray)));
    connect(&clt,SIGNAL(signal_get_config_done(bool,string)),this,
            SLOT(open_config(bool,string)));

    connect(&rcvr,SIGNAL(send_rst(QByteArray)),this,
            SLOT(set_layout(QByteArray)));

    tmr.start(1);

}

MainWindow::~MainWindow()
{
    delete src;
    delete ui;
}

void MainWindow::on_pushButton_search_clicked()
{
    ui->comboBox->clear();
    searcher.search_device();
}

void MainWindow::on_comboBox_activated(const QString &ip)
{
 //   qDebug()<<arg1;
    clt.connect_to_server(ip);

}

void MainWindow::on_pushButton_clear_clicked()
{
    ui->textEdit_recv->clear();
    ui->textEdit_send->clear();
}

void MainWindow::on_pushButton_send_clicked()
{
    clt.send_msg(ui->textEdit_send->toPlainText().toUtf8());
}

void MainWindow::on_pushButton_getconfig_clicked()
{
   clt.get_config();
}

void MainWindow::on_pushButton_setconfig_clicked()
{
   clt.set_config(dm.get().data());
}

void MainWindow::on_pushButton_addcam_clicked()
{
   QString url= ui->lineEdit_addcam->text();

   clt.add_camera(url,dm.get_cams().size()+1);
}


void MainWindow::on_pushButton_del_clicked()
{
    clt.del_camera(ui->lineEdit_delcam->text().toInt());

}
