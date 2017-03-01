#ifndef HTTPD_H
#define HTTPD_H
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <cstring>
#include <errno.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <linux/fb.h>
#include <QThread>
#include "getpicture.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif
#define SERVER_PORT 80

class Httpd : public QThread
{
    Q_OBJECT
public:
    Httpd();
    int TIMEOUT;

protected:
    void run();

private:
    char copybuf[16384];
    int KEY_QUIT;
    char referrer[128];
    int content_length;
    GetPicture* getPicture;
    int PrintHeader(FILE *f, int content_type);
    int DoJpeg(FILE *f, char *name);
    int ParseReq(FILE *f, char *r);
    int HandleConnect(int fd);
    int copy(FILE *read_f, FILE *write_f);
};

#endif // HTTPD_H
