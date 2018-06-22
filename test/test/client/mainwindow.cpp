#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    pt=NULL;
    flg=0;
    ui->setupUi(this);
    //    ply=new Player(ui->groupBox_picture);

    //    ply->setMinimumWidth(300);
    //    ply->setMinimumHeight(300);
    //    ui->groupBox_picture->layout()->addWidget(ply);
    //    ply->start();
    //    connect(ply,SIGNAL(doub_click()),this,SLOT(set_full()));
    //            ui->textEdit_2->hide();
    //            ui->textEdit->hide();
    connect(&searcher,SIGNAL(find_ip(QString)),this,SLOT(set_ip(QString)));
    ui->comboBox->clear();
    ui->comboBox->setEnabled(false);
    connect(this,SIGNAL(add_picture(Player *)),this,SLOT(try_add_picture(Player *)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::play_start()
{

  //  starter_thread.setObjectName("players manager");
    //    player_starter=new Tmp(players,cfg.cams_cfg);
   if(pt)
       delete pt;
    pt=new PlayThread(this);
    pt->moveToThread(&starter_thread);
    connect(&starter_thread,SIGNAL(started()),pt,SLOT(play()));

    starter_thread.start();
}

void MainWindow::on_groupBox_picture_clicked()
{
    prt(info,"click");
}

void MainWindow::on_groupBox_picture_clicked(bool checked)
{
    prt(info,"1");
}

void MainWindow::on_pushButton_clicked()
{
    prt(info,"start search ");
    ui->comboBox->clear();
    ui->comboBox->addItem("");

    searcher.search_device();
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    //    prt(info,"%d selected",index);
    //    QString ip=ui->comboBox->itemText(index);
    //    prt(info,"%s",ip.toStdString().data());
    //    ui->comboBox->setEnabled(false);
    //    open_device(ip);
}

void MainWindow::on_comboBox_activated(int index)
{
    prt(info,"%d selected",index);
    QString ip=ui->comboBox->itemText(index);
    prt(info,"%s",ip.toStdString().data());
    ui->comboBox->setEnabled(false);
    open_device(ip);

}

void MainWindow::on_pushButton_save_clicked()
{

    QByteArray ba=ui->textEdit_config->toPlainText().toUtf8();
    clt.set_config(ba);

}

void MainWindow::on_pushButton_insert_clicked()
{
    QByteArray ba=ui->textEdit_input->toPlainText().toUtf8();
    clt.add_test_server(ba);

}

void MainWindow::on_pushButton_delete_clicked()
{
    // int index= ui->lineEdit_cam_index->text().toInt();
    //  clt.del_camera(index);
    ui->textEdit_input->clear();
    ui->textEdit_output->clear();
}
