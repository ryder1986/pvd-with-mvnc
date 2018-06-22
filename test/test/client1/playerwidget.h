#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include <QObject>
#include <QOpenGLWidget>
#include <QPainter>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QMouseEvent>
#include "tool.h"
#include "pvd.h"
#include <QDebug>
#include "opencv2/core/core.hpp"
#include  <QGroupBox>
#include <datamanager.h>
using namespace cv;
class LayoutPainter{
public:
    typedef struct process_result{
        int width;
        int height;
        bool exist;
        int count;
        int front_count;
        int back_count;
        int other_count;
        int duration;
        vector <Rect> rects;
    }m_result;
    int count_begin_symbol(QByteArray ba)
    {
        char *tmp=ba.data();
        int sz=ba.size();
        int ret=0;
        int i;
        for( i=0;i<sz;i++){
            if(tmp[i]=='{'){
                ret++;
            }
        }
        return ret;
    }
    int count_end_symbol(QByteArray ba)
    {
        char *tmp=ba.data();
        int sz=ba.size();
        int ret=0;
        int i;
        for( i=0;i<sz;i++){
            if(tmp[i]=='}'){
                ret++;
            }
        }
    }
    bool try_get_obj_buf(QByteArray src,QByteArray &dst)
    {
        int ret=false;
        int stack=0;
        char *p_tmp=src.data();
        bool flg=false;
        //    bool flg_real_end=false;
        //char *p_start=src.data();
        dst.clear();
        dst.append(src);
        int i;
        if(count_begin_symbol(src)>0){
            for(i=0;i<src.size();i++){
                if(p_tmp[i]=='{')
                {
                    stack++;
                    flg=true;
                }
                if(p_tmp[i]=='}')
                    stack--;


                if(stack==0&&flg)
                {

                    break;
                }

            }
            if(i<src.size()){
                ret=true;
                if(src[i+1]=='\n')
                    dst.truncate(i+2);
                else
                    dst.truncate(i+i);
            }
        }
        return ret;
    }

    //dst:a sting which contain a compelete json object
    //src:a slice ofstream buffer
    //tmp_msg:last slice of buffer(maybe begining of json string)
    bool get_valid_buf(QByteArray &src,QByteArray &dst)
    {
        if(try_get_obj_buf(src,dst)){
            src.remove(0,dst.size());
            return true;
        }
        return false;

    }
    LayoutPainter(  QList <QPoint> ps):pns(ps),picture_w(640),picture_h(480)
    {
        index=-1;
        data.clear();
        picked=false;

        poly_num=4;//rectangles
        channel_num=pns.size()/poly_num;
        //    p=new QPainter(par);
    }

    ~LayoutPainter()
    {
        //     delete p;
    }
    void unpack_rst1(m_result &rst,QByteArray ba)
    {
        DataPacket pkt(string(ba.data()));
        JsonValue data= pkt.get_value("rt_data");
        DataPacket pkt_ch(data);
        vector<JsonValue> jv=pkt_ch.array_value();
        foreach (JsonValue v, jv) {
            JsonValue result_v=DataPacket(v).get_value("result");
            JsonValue result_rects=DataPacket(result_v).get_value("rects");
            vector<  JsonValue >rcts_v=DataPacket(result_rects).array_value();


            foreach (JsonValue v, rcts_v) {
                DataPacket p(v);
                int w= p.get_int("w");
                int h= p.get_int("h");
                int x= p.get_int("x");
                int y= p.get_int("y");
                rst.rects.push_back(Rect(x,y,w,h));
            }


            rst.width= DataPacket(result_v).get_int("width");
            rst.height= DataPacket(result_v).get_int("height");
        }

    }
    void unpack_rst(m_result &rst,QByteArray ba)
    {
        //    rst.rects
        DataPacket pkt(string(ba.data()));
        vector <JsonValue> rs=pkt.get_array("rects");
        foreach (JsonValue v, rs) {
            DataPacket p(v);
            int w= p.get_int("w");
            int h= p.get_int("h");
            int x= p.get_int("x");
            int y= p.get_int("y");
            rst.rects.push_back(Rect(x,y,w,h));
        }

        rst.width= pkt.get_int("width");
        rst.height= pkt.get_int("height");

        //        // QJsonObject obj=pkt.data();
        //        //   qDebug()<<"------start pkt--------";
        //        //    qDebug()<<ba;
        //        //  qDebug()<<"---end pkt----";
        //        printf("===>%s<===",ba.data());
        //        fflush(NULL);
        //        rst.width= pkt.get_value("width").toInt();
        //        rst.height= pkt.get_value("height").toInt();
        //        rst.exist= pkt.get_value("exist").toInt();
        //        rst.count= pkt.get_value("count").toInt();
        //        rst.front_count= pkt.get_value("front_count").toInt();
        //        rst.back_count= pkt.get_value("back_count").toInt();
        //        rst.other_count= pkt.get_value("other_count").toInt();
        //        rst.duration= pkt.get_value("duration").toInt();

        //        QJsonArray rects=pkt.get_value("rects").toArray();
        //        foreach (QJsonValue v, rects) {
        //            //   QJsonObject rc;
        //            DataPacket pkt1(v.toObject());
        //            Rect rct1;
        //            rct1.x=pkt1.get_value("x").toInt();
        //            rct1.y=pkt1.get_value("y").toInt();
        //            rct1.width=pkt1.get_value("w").toInt();
        //            rct1.height=pkt1.get_value("h").toInt();
        //            rst.rects.push_back(rct1);
        //        }
        // rst.rects

    }
    QByteArray tmp_msg;
    void paint(QPainter *test)
    {
        m_result rst;
        window_w=test->window().width();
        window_h=test->window().height();

        if(!data.isEmpty()){


#if 1
            QBrush brush1(QColor(255,0,0,111));
            QPen pen(brush1,10);
            test->setPen(pen);

            //unpack_rst(rst,data);
            unpack_rst1(rst,data);
            alg_w=rst.width;
            alg_h=rst.height;
            if(rst.rects.size()){
                prt(info,"sz %d (%d)",rst.rects.size(),rst.rects[0].x);

            }
            foreach (Rect r, rst.rects) {
                test->drawRect(QRect(r.x*window_w/alg_w,r.y*window_h/alg_h,r.width*window_w/alg_w,r.height*window_h/alg_h));
            }
#else
            QBrush brush1(QColor(255,0,0,111));
            QPen pen(brush1,10);
            test->setPen(pen);




            QByteArray valid_buf;
            valid_buf.clear();
            tmp_msg.append(data);
            while(get_valid_buf(tmp_msg,valid_buf)) {
                unpack_rst(rst,valid_buf);
                foreach (Rect r, rst.rects) {
                    test->drawRect(QRect(r.x,r.y,r.width,r.height));
                }

            }


            return ;
#endif

            //  test->drawRect(r);

            //  prt(info,"%d",rst.rects.size());

        }

        //       bool active=test->isActive();
        if(!data.isEmpty()){
            QString str(data.toStdString().data());
            //  prt(info,"-->%s",str.toStdString().data());


            //        QString str(rst.data());
            QStringList list=str.split(":");
            QStringList l;

#if 0
            QBrush blue_brush_trans(QColor(111,111,111,111));
            // blue_brush_trans.setStyle(Qt::BrushStyle);
            test->setBrush(blue_brush_trans);
#else
            QBrush brush1(QColor(255,0,0,111));
            QPen pen(brush1,10);
            test->setPen(pen);

#endif
            window_w=test->window().width();
            window_h=test->window().height();

            foreach (QString s, list) {
                l=s.split(',');
                QRect r;

                if(l.size()==4){
                    r.setRect(l[0].toInt()*window_w/alg_w,l[1].toInt()*window_h/alg_h,l[2].toInt()*window_w/alg_w, l[3].toInt()*window_h/alg_h);
                    //              r.setRect(l[0].toInt()*window_w/960*2,l[1].toInt()*window_h/540*2,l[2].toInt()*window_w/960*2, l[3].toInt()*window_h/540*2);

                    //                    // rcts.append(r);
                    //           test->isActive();
                    test->drawRect(r);
                }else
                    if(l.size()==2){
                        alg_w=l[0].toInt();
                        alg_h=l[1].toInt();
                        //   prt(info,"(get %d %d)",wid,hei);
                        //                        wi_ori=wid;
                        //                        he_ori=hei;
                        //                        wi_ori=640;
                        //                        he_ori=480;
                    }

            }


            data.clear();
        }
    }
    bool try_pick(QPoint point)
    {
        bool ret=false;
        int i=0;
        if(pns_now.size()==channel_num*poly_num)
            for (i=0;i<pns_now.size();i++) {
                if(abs(point.x()-pns_now[i].x())<10&&abs(point.y()-pns_now[i].y())<10){
                    prt(info,"%d slect",i+1);
                    picked=true;
                    ret=true;
                    index=i;
                }
            }
        return ret;
    }

    bool try_move(QPoint point)
    {
        bool ret=false;
        if(picked){
            ret=true;
            pns_now[index]=point;
            set_points();
        }
        return ret;
    }
    QList <QPoint> points()
    {
        return pns;
    }

    bool try_put()
    {
        bool ret=false;
        if(picked){
            set_points();
            picked=false;
            index=-1;
            ret=true;
        }
        return ret;
    }
    int get_index()
    {
        return index;
    }

    QPoint get_points()
    {
        int i=0;
        //  pns_now.reserve(4);
        pns_now.clear();
        if(pns.size()>0){
            for(i=0;i<pns.size();i++)
                pns_now.append((QPoint(pns[i].x()*window_w/picture_w,pns[i].y()*window_h/picture_h)));
            //          pns_now.append((QPoint(pns[i].x()*window_w/640,pns[i].y()*window_h/480)));
        }
    }
    void set_points()
    {
        int i=0;
        if(pns_now.size()>0){
            for(i=0;i<pns_now.size();i++)
                pns[i]=QPoint(pns_now[i].x()*picture_w/window_w,pns_now[i].y()*picture_h/window_h);
            //    pns[i]=QPoint(pns_now[i].x()*wi_ori/window_w,pns_now[i].y()*he_ori/window_h);
            //   pns[i]=QPoint(pns_now[i].x()*640/window_w,pns_now[i].y()*480/window_h);
        }
    }


    void paint_rect1(QPainter *pt,int w,int h)
    {
        picture_w=w;
        picture_h=h;
        window_w=pt->window().width();
        window_h=pt->window().height();
        get_points();
        if(1){
            //if(pns.size()==4){
            //  prt(info,"ps ok");

            //            QBrush brush1(QColor(0,0,222));
            //            QPen pen(brush1,5);
            //            pt->setPen(pen);

            pt->setPen( QPen (QBrush (QColor(0,0,222)),5));

            QPointF p[poly_num*channel_num];
            //            p[0]=pns[0];
            //            p[1]=pns[1];
            //            p[2]=pns[2];
            //            p[3]=pns[3];


            //            p[0]=get_point(pns[0],w,h);
            //            p[1]=get_point(pns[1],w,h);
            //            p[2]=get_point(pns[2],w,h);
            //            p[3]=get_point(pns[3],w,h);

            get_points();
            for(int j=0;j<channel_num;j++){
                for(int i=0;i<poly_num;i++){

                    p[i+poly_num*j]=pns_now[i+poly_num*j];
                    pt->drawEllipse(p[i+poly_num*j],10,10);
                    if(picked&&(i+j*poly_num)==index){

                        pt->save();
                        pt->setPen( QPen (QBrush (QColor(222,0,0)),5));
                        pt->drawEllipse(p[i+poly_num*j],10,10);
                        pt->restore();
                    }

                }
                pt->setPen( QPen (QBrush (QColor(0,0,222)),5));
                pt->drawPolygon((QPointF *)(p+j*poly_num),poly_num);
            }



            //            p[0].setX(p[0].x()*w/wi); p[0].setY(p[0].y()*h/he);
            //            p[1].setX(p[1].x()*w/wi); p[1].setY(p[1].y()*h/he);
            //            p[2].setX(p[2].x()*w/wi); p[2].setY(p[2].y()*h/he);
            //            p[3].setX(p[3].x()*w/wi); p[3].setY(p[3].y()*h/he);

            //  pt->drawPolyline((QPointF *)p,4);

        }else{
            prt(info,"ps err");
        }
    }

    void paint_rect(QPainter *pt,int w,int h)
    {
        picture_w=w;
        picture_h=h;
        window_w=pt->window().width();
        window_h=pt->window().height();
        get_points();
        if(pns.size()==4){
            //  prt(info,"ps ok");

            //            QBrush brush1(QColor(0,0,222));
            //            QPen pen(brush1,5);
            //            pt->setPen(pen);

            pt->setPen( QPen (QBrush (QColor(0,0,222)),5));

            QPointF p[4];
            //            p[0]=pns[0];
            //            p[1]=pns[1];
            //            p[2]=pns[2];
            //            p[3]=pns[3];


            //            p[0]=get_point(pns[0],w,h);
            //            p[1]=get_point(pns[1],w,h);
            //            p[2]=get_point(pns[2],w,h);
            //            p[3]=get_point(pns[3],w,h);

            get_points();
            for(int i=0;i<4;i++){

                p[i]=pns_now[i];
                pt->drawEllipse(p[i],10,10);
                if(picked&&i==index){

                    pt->save();
                    pt->setPen( QPen (QBrush (QColor(222,0,0)),5));
                    pt->drawEllipse(p[i],10,10);
                    pt->restore();
                }

            }



            //            p[0].setX(p[0].x()*w/wi); p[0].setY(p[0].y()*h/he);
            //            p[1].setX(p[1].x()*w/wi); p[1].setY(p[1].y()*h/he);
            //            p[2].setX(p[2].x()*w/wi); p[2].setY(p[2].y()*h/he);
            //            p[3].setX(p[3].x()*w/wi); p[3].setY(p[3].y()*h/he);

            //  pt->drawPolyline((QPointF *)p,4);
            pt->setPen( QPen (QBrush (QColor(0,0,222)),5));
            pt->drawPolygon((QPointF *)p,4);
        }else{
            prt(info,"ps err");
        }
    }

    void set_data(QByteArray ba)
    {
        data=ba;
    }

private:
    // QPainter *p;
    QByteArray data;
    QList <QPoint> pns;
    QList <QPoint> pns_now;
    bool picked;
    int index;
    int picture_w;
    int picture_h;
    int alg_w;
    int alg_h;


    int window_w;
    int window_h;

    int channel_num;
    int poly_num;//rectangles
};


#include <datamanager.h>
#include <QMenu>
class PlayerWidget : public QOpenGLWidget
{
    Q_OBJECT

    DataManager *dma;
    int cam_index;
    QAction *action_add_channel;
    QMenu *menu;
public:
    PlayerWidget(QGroupBox *d):show_info(false)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(right_click(QPoint)));

        menu=new QMenu(this);
        action_add_channel=new QAction(this);
        action_add_channel->setText("add channel");
        menu->addAction(action_add_channel);
        connect(action_add_channel,SIGNAL(triggered(bool)),this,SLOT(add_channel(bool)));

    }

    ~PlayerWidget()
    {
        delete action_add_channel;
        delete menu;
        delete pt;
    }
    void set_realtime_data(QByteArray rst)
    {
        pt->set_data(rst);
    }

    void set_data( DataManager *dm,int index)
    {
        cam_index=index;
        dma=dm;
        pt=new LayoutPainter(dm->get_points(index));
        channel_num=0;
        poly_num=4;//rectangles

    }

    PlayerWidget( QList <QPoint> ps):painter(this),show_info(false)
    {


        pt=new LayoutPainter(ps);
        frame_rate=0;
        QTimer *t=new QTimer();
        connect(t,SIGNAL(timeout()),this,SLOT(check_rate()));
        t->start(1000);


        QBrush blue_brush_trans(QColor(0,222,200,255));
        // blue_brush_trans.setStyle(Qt::BrushStyle);
        painter.setBrush(blue_brush_trans);

        QPen p(blue_brush_trans,200);
        painter.setPen(p);

    }



    void set_image(QImage img1)

    {
        lock.lock();
        img=img1;
        lock.unlock();
    }
    void set_title(QString t)
    {
        lock.lock();
        title=t;
        lock.unlock();
    }
    void set_show(bool flg)
    {
        show_info=flg;
    }

protected:
    void paintEvent(QPaintEvent *)
    {
#if 1
        lock.lock();
        // QThread::msleep(2000);
        //            frame_rate++;
        QPainter painter(this);
        if(!img.isNull()){
            //     prt(info,"%d %d",img.byteCount(),img.bytesPerLine());
            painter.drawImage(QRect(0,0,this->width(),this->height()),img);
            //      bool active=painter.isActive();
            //    pt->paint(&painter);

            pt->paint_rect1(&painter,img.width(),img.height());
        }
        if(show_info){
            pt->paint(&painter);
        }
        lock.unlock();
        //        qDebug()<<"paint";
#else
        // if(img1.width()>0){
        if(1){
            //            QThread::msleep(10);
            lock.lock();
            // QThread::msleep(2000);
            //            frame_rate++;
            QPainter painter(this);
            if(!img.isNull()){
                //     prt(info,"%d %d",img.byteCount(),img.bytesPerLine());
                painter.drawImage(QRect(0,0,this->width(),this->height()),img);
                //      bool active=painter.isActive();
                pt->paint(&painter);


            }

            if(show_info){
                pt->paint_rect(&painter,img.width(),img.height());
            }

            //            painter.drawText(QPointF(111,111),title);

            //#if 0
            //            QBrush blue_brush_trans(QColor(0,222,200,255));
            //            painter.setBrush(blue_brush_trans);

            //            //   painter.drawRect(0,0,this->width(),this->height());
            //            painter.drawRect(100,100,300,300);
            //#endif
            lock.unlock();
        }
#endif
    }
    void  initializeGL()
    {

    }

public slots:
    void add_channel(bool)
    {
        QList<QPoint> ps;
        ps.append(QPoint(0,0));
        ps.append(QPoint(0,0));
        ps.append(QPoint(0,0));
        ps.append(QPoint(0,0));
        dma->add_channel(ps,cam_index);
  //      emit data_changed();
        emit alg_changed(cam_index);
    }
    void del_channel(bool)
    {

        dma->del_channel((pt->get_index()+1)/4,cam_index);
      //  emit data_changed();
        emit alg_changed(cam_index);
    }
    void right_click(QPoint p)
    {
        prt(info,"right clik at %d %d",p.x(),p.y());

        menu->exec(mapToGlobal(p));
    }

    //    void set_layout_data(QByteArray data)
    //    {
    //        //    lock.lock();
    //        pt->set_data(data);
    //        //    lock.unlock();
    //    }

    void check_rate()
    {
        //frame_rate++;
        //     prt(info,"frame rate :%d ",frame_rate);
        frame_rate=0;
    }

    void mouseDoubleClickEvent(QMouseEvent *)
    {
        emit selected(this);
    }
    void mousePressEvent(QMouseEvent *e)
    {

        if(e->button()==Qt::MouseButton::RightButton){
            QAction *action_del_channel;

            if(pt->try_pick(e->pos())){
                action_del_channel=new QAction(this);
                action_del_channel->setText("del channel");
                connect(action_del_channel,SIGNAL(triggered(bool)),this,SLOT(del_channel(bool)));
                menu->addAction(action_del_channel);
            }
            prt(info,"right press");
        }

        // if(e->button()==Qt::MouseButton::LeftButton){
        if(1){

            prt(info,"left press");
            (pt->try_pick(e->pos()));
        }


    }

    void mouseReleaseEvent(QMouseEvent *e )
    {
        if(pt->try_put()){

            dma->set_points(pt->points(),cam_index);
            //emit data_changed();
            emit alg_changed(cam_index);

        }
    }
    void mouseMoveEvent(QMouseEvent *e)
    {
        QList <QPoint>l;
        if(pt->try_move(e->pos())){
            l=pt->points();
        }
    }


signals:
    void selected(PlayerWidget *w);
    void data_changed();
    void alg_changed(int index);
private:
    QImage img;
    QString title;
    QMutex lock;
    int frame_rate;
    LayoutPainter *pt;
    QPainter painter;
    QList <QPoint> area_v;
    bool show_info;
    int channel_num;
    int poly_num;
    //  QList <QPoint> points;
    //    QByteArray ba;
};

#endif // PLAYERWIDGET_H
