#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "tool.h"
#include "player.h"
#include "client.h"
#include "serverinfosearcher.h"
#include <QJsonObject>
#include "player.h"
#include "playerwidget.h"
namespace Ui {
class MainWindow;
}

class PlayThread;
class MainWindow : public QMainWindow
{
    Q_OBJECT
    typedef struct configure{
        QString server_name;
        int dev_id;
        QString sig_ip;
        int sig_port;
        QString ntp_ip;
        int ntp_port;
        QJsonValue cams_cfg;
    }configture_t;
    configture_t cfg;
public:
    friend class PlayThread;
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void open_device(QString ip)
    {
        clt.connect_to_server(ip);
        connect(&clt,SIGNAL(signal_get_config_done(bool,QByteArray)),this,
                SLOT(slot_open_config(bool,QByteArray)));
        connect(&clt,SIGNAL(get_ret(QByteArray)),this,
                SLOT(get_server_msg(QByteArray)));
        connect(&clt,SIGNAL(connect_done()),this,SLOT(clt_ready()));
    }

    void cfg_2_obj(QJsonObject &obj)
    {

        obj["device_name"]=cfg.server_name;
        obj["deviceID"]=cfg.dev_id;
        obj["signal_machine_ip"]=cfg.sig_ip;
        obj["signal_machine_port"]=cfg.sig_port;
        obj["ntp_ip"]=cfg.ntp_ip;
        obj["ntp_port"]=cfg.ntp_port;
        obj["cameras"]=cfg.cams_cfg;
    }

    void obj_2_cfg(QJsonObject obj)
    {

        cfg.server_name=obj["device_name"].toString();
        cfg.dev_id=obj["deviceID"].toInt();
        cfg.sig_ip=obj["signal_machine_ip"].toString();
        cfg.sig_port= obj["signal_machine_port"].toInt();
        cfg.ntp_ip=obj["ntp_ip"].toString();
        cfg.ntp_port=obj["ntp_port"].toInt();
        cfg.cams_cfg=obj["cameras"];
    }
    void play()//this will be call in helper thread
    {
        QJsonArray cams=  cfg.cams_cfg.toArray();
        qDebug()<<"----sssss--------";
        foreach (Player *p, players) {
            QObjectList ls=ui->groupBox_picture->children();
            qDebug()<<ls.size()<<"------------";
            delete p;
        }
        players.clear();
        foreach (QJsonValue v, cams) {
            players.append(new Player(v));
            emit add_picture(players.last());
            players.last()->start();

        }
//        foreach (Player *p, players) {
//           if( p->wait()){
//               prt(info,"stop a thread");
//           }
//        }

    }
    void  play_start();

private slots:
    void get_server_msg(QByteArray ba)
    {
        ui->textEdit_output->clear();
        ui->textEdit_output->setText(ba.toStdString().data());
    }

    void rect_changed(QList <QPoint> lst)
    {
        prt(info,"new points");
        QJsonArray ps;

        foreach (QPoint p, lst) {
            prt(info,"(%d %d)",p.x(),p.y());
            QJsonObject v;
            v["x"]=p.x();
            v["y"]=p.y();
            ps.append(v);
        }
        QJsonObject o;
        o["type"]=6;
        o["cam_index"]=1;
        QJsonObject obj;
        obj["selected_alg"]="pvd_c4";

        QJsonObject o1;
        o1["detect_area"]=ps;

        obj["pvd_c4"]=o1;
        //   obj
        o["alg"]=obj;
        QJsonDocument doc(o);
        QByteArray ba=doc.toJson();

        ui->textEdit_output->setText(ba);

    }

    void try_add_picture(Player *p)
    {

        QList <QPoint> l=p->get_v();
        if(l.size()!=4){
            prt(info,"error points");
        }
        PlayerWidget *pw=new PlayerWidget(l);
        ui->groupBox_picture->layout()->addWidget(pw);
        connect(pw,SIGNAL(selected(PlayerWidget*)),this,SLOT(picture_selected(PlayerWidget*)));

        p->set_widget(pw);
        //    update_pic();
    }
    void picture_selected(PlayerWidget *wgt)
    {
        flag1++;
        bool select=false;
        if(flag1%2)
            select=true;
        else
            select=false;
        foreach (Player *p, players) {

            PlayerWidget *pp= (PlayerWidget*)p->get_widget();


            if(pp==wgt){
                prt(info," select");
                if(select){
                    pp->set_show(true);
                    connect(&data_rcv,SIGNAL(send_rst(QByteArray)),pp,SLOT(set_layout_data(QByteArray)));
                    connect(pp,SIGNAL(rect_changed(QList<QPoint>)),this,SLOT(rect_changed(QList<QPoint>)));
                    clt.focus_camera(players.indexOf(p)+1);
                }else{
                    pp->set_show(false);
                    disconnect(pp,SIGNAL(rect_changed(QList<QPoint>)),this,SLOT(rect_changed(QList<QPoint>)));

                    clt.disfocus_camera(players.indexOf(p)+1);
                    disconnect(&data_rcv,SIGNAL(send_rst(QByteArray)),pp,SLOT(set_layout_data(QByteArray)));
                }
                pp->show();
            }else{

                //  prt(info,"not select");
                if(select)
                {
                    pp->hide();
                }else{
                    pp->show();
                }
            }
        }
    }

    void slot_open_config(bool ,QByteArray ba)
    {
        prt(debug,"get %d  bytes",ba.size());
        QJsonDocument doc=QJsonDocument::fromJson(ba);
        QJsonObject obj=doc.object();
        obj_2_cfg(obj);
        QString str(ba);
        ui->textEdit_config->setText(str);
        play_start();
    }
    void clt_ready()
    {
        prt(info,"--->  conect ok");
        clt.get_config();
    }

    void on_groupBox_picture_clicked();

    void on_groupBox_picture_clicked(bool checked);
    void set_full()
    {
    }
    void on_pushButton_clicked();
    void set_ip(QString ip)
    {
        prt(info,"find %s",ip.toStdString().data());
        ui->comboBox->addItem(ip);
        ui->comboBox->setEnabled(true);
    }
    void on_comboBox_currentIndexChanged(int index);

    void on_comboBox_activated(int index);

    void on_pushButton_save_clicked();

    void test()
    {
        while(1){
            prt(info,"ssss");
        }
        foreach (Player *p, players) {
            delete p;
        }
        players.clear();
        foreach (QJsonValue v,  cfg.cams_cfg.toArray()) {
            players.append(new Player(v));
        }
    }
    void on_pushButton_insert_clicked();

    void on_pushButton_delete_clicked();

signals:
    void add_picture(Player *);

private:
    Ui::MainWindow *ui;
    int flg;
    //Player *ply;
    ServerInfoSearcher searcher;
    Client clt;
    QList <Player *>players;
    // Tmp *player_starter;
    QThread starter_thread;
    PlayThread *pt;
    int flag1;
    ProcessedDataReciver data_rcv;
};

class PlayThread:public QObject{
    Q_OBJECT
public:
    PlayThread(MainWindow *w)
    {
        mw=w;
    }
public slots:
    void play()
    {

        Qt::HANDLE handle=QThread::currentThreadId();
        mw->play();
    }
private:
    MainWindow *mw;


};

#endif // MAINWINDOW_H
