#include <QCoreApplication>
#include "ipinfo.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    IpInfo ip;
    while(1);
    return a.exec();
}

