#ifndef GETPICTURE_H
#define GETPICTURE_H

#include <QObject>
#include <QThread>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include "camera.h"

class GetPicture : public QThread
{
    Q_OBJECT
public:
    volatile long long count;
    GetPicture();
protected:
    void run();
private:
    Camera *camera;
    char jpgChar[20],tempJpgChar[20],delJpgChar[20];
    void strrevCon(char* str,char* temp);
    void convertJpgChar(long long count);
    void* getPicture(void* data);
};

#endif // GETPICTURE_H
