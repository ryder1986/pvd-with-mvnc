#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QDebug>
#include "ui_mainwindow.h"
#include "videosource.h"
#include <QTimer>
#include "serverinfosearcher.h"
#include "client.h"
#include "datamanager.h"
namespace Ui {
class MainWindow;
}
class PlayerBox:public PlayerWidget
{
    Q_OBJECT
    int cam_index;

public:
    PlayerBox(string url,DataManager *dm,int index):PlayerWidget(NULL)
    {

        cam_index=index;
        this->set_data(dm,index);
        connect(&tmr,SIGNAL(timeout()),this,SLOT(play_a_frame()));
        tmr.start(1);
        src=new VideoSource(url);
    }
public slots:
    void  play_a_frame()
    {
        Mat rgb_frame;
        Mat bgr_frame;
        QImage img1;
        bool ret=src->get_frame(bgr_frame);
        if(ret){
            cvtColor(bgr_frame,rgb_frame,CV_BGR2RGB);
            img1=QImage((const uchar*)(rgb_frame.data),
                        rgb_frame.cols,rgb_frame.rows,
                        QImage::Format_RGB888);

            if(this){
                img1.bits();
                this->set_image(img1);
                this->set_title(src->get_url().data());
                if(frame_rate%3==0)
                    this->update();
                frame_rate++;
            }

        }

    }
private:
    VideoSource *src;
    QTimer tmr;
    int frame_rate;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum{
        DISPLAY_NORMAL_MODE,
        DISPLAY_MAX_MODE
    };
    bool running;
    DataManager dm;
    vector <PlayerBox *> players;
    ProcessedDataReciver rcvr;
    int mode;
    int focus_index;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void picture_selected(PlayerWidget *w)
    {

        //   vector <PlayerBox *>::iterator ir=std::find(players.front(),players.end(),w);
        int sz=players.size();
        int i;
        for( i=0;i<sz;i++){
            if(players[i]==w)
                break;
        }
        if(i<sz){
            prt(info,"%d selected",i+1);
        }
        //  vector<PlayerBox*>::iterator it=find(players.front(),players.end(),w);
        int pos=0;
        for (pos=0;pos<sz;pos++)
        {
            if(players[pos]==w){
                break;
            }
        }
        focus_index=pos;

        if(mode==DISPLAY_NORMAL_MODE){
            foreach (PlayerWidget *w, players) {
                if(players[i]!=w)
                    w->hide();
                else{
                    clt.focus_camera(pos+1);
                }
            }

            mode=DISPLAY_MAX_MODE;
            PlayerWidget *w= players[focus_index];
            w->set_show(true);
        }else{
            foreach (PlayerWidget *w, players) {
                w->show();
                //   clt.disfocus_camera();
            }

            for (pos=0;pos<sz;pos++)
            {
                clt.disfocus_camera(pos+1);
                PlayerWidget *w= players[focus_index];
                w->set_show(false);
            }
            mode=DISPLAY_NORMAL_MODE;
        }

    }

    void config_changed()
    {
        clt.set_config(dm.get().data());
    }
    void set_layout(QByteArray rst)
    {
//        prt(info,"%s",rst.data());
        if(running){
            PlayerWidget *w= players[focus_index];

            w->set_realtime_data(rst);
        }
    }

    void open_config(bool ss,string cfg)
    {
        // ui->groupBox_picturebox->layout()->removeWidget(ui->widget_picture);
        ui->widget_picture->hide();
        running=true;
        dm.set(cfg);
        vector <string> urls=dm.get_cams();
        int index=0;
        for(int i=0;i<players.size();i++){
            ui->groupBox_picturebox->layout()->removeWidget(players[i]);
        }
        players.clear();
        foreach (string url, urls) {
            index++;
            PlayerBox *b=new PlayerBox(url,&dm,index);
            players.push_back(b);
            ui->groupBox_picturebox->layout()->addWidget(b);
            connect(b,SIGNAL(data_changed()),this,SLOT(config_changed()));
            connect(b,SIGNAL(alg_changed(int)),this,SLOT(modify_alg(int)));
            connect(b,SIGNAL(selected(PlayerWidget*)),this,SLOT(picture_selected(PlayerWidget*)));
        }
    }

    void timeup()
    {

    }

    void mouseMoveEvent(QMouseEvent *e)
    {
        if(picked){
            this->ui->groupBox_textbox->setFixedWidth(e->pos().x()-ui->groupBox_textbox->pos().x());
        }
    }
    void mousePressEvent(QMouseEvent *e)
    {
        picked=true;
    }
    void mouseReleaseEvent(QMouseEvent *e)
    {
        picked=false;
    }

private slots:
    void modify_alg(int index)
    {
        clt.set_alg( dm.get_alg(index).data(),index);
    }

    void on_pushButton_search_clicked();
    void ip_found(QString ip)
    {
        ui->comboBox->addItem(ip);
    }
    void server_msg(QByteArray ba)
    {
        ui->textEdit_recv->setPlainText(ba.data());
    }
    void sended(QByteArray ba)
    {
        ui->textEdit_send->setPlainText(ba.data());
    }

    void on_comboBox_activated(const QString &arg1);

    void on_pushButton_clear_clicked();

    void on_pushButton_send_clicked();

    void on_pushButton_getconfig_clicked();

    void on_pushButton_setconfig_clicked();

    void on_pushButton_addcam_clicked();

    void on_pushButton_del_clicked();

private:
    Ui::MainWindow *ui;
    bool picked;
    VideoSource *src;
    QTimer tmr;
    int frame_rate;
    ServerInfoSearcher searcher;
    Client clt;
};

#endif // MAINWINDOW_H
