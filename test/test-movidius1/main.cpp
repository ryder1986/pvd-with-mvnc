//#include <QCoreApplication>
#include "movidiusprocessor.h"
int main(int argc, char *argv[])
{
    //QCoreApplication a(argc, argv);
    MovidiusProcessor &p=MovidiusProcessor::get_instance();
    //return a.exec();
    return 1;
}

