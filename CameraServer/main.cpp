#include <QCoreApplication>
#include <httpd.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Httpd *httpd = new Httpd();
    httpd->start();
    
    return a.exec();
}
