#include <QCoreApplication>
#include "filedatabase.h"
#include "server.h"
#include "pvd.h"

int main(int argc, char *argv[])
{
   // Tool1::set_debug_level(Tool1::DEBUG_LEVEL::FATAL);
  //  Tool1::set_label(Tool1::LABEL_SELECTION::BOTH);
    QCoreApplication a(argc, argv);
    ProcessedDataSender *s=ProcessedDataSender::get_instance();
    FileDatabase db(Pvd::get_instance().config_file);
    Server svr(&db);
    svr.start();

    return a.exec();
}
