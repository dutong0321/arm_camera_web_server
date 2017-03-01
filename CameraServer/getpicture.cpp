#include "getpicture.h"

GetPicture::GetPicture()
{
    camera = new Camera();
    count = 0;
}

// 反转字符串
void GetPicture::strrevCon(char* str,char* temp)
{
    int j,i = 0;
    while(temp[i] != '\0')
    {
        i++;
    }
    str[i] = '\0';
    for(j = 0;j < i;j++)
    {
        str[j] = temp[i - 1 - j];
    }
}

// 将数字转换为图片名
// 将以前生成的图片删除 -50
void GetPicture::convertJpgChar(long long count)
{
    int i = 0,num = 0;
    long long delCount = count - 50;
    while(1)
    {
        num = count % 10;
        count /= 10;
        tempJpgChar[i++] = num + 48;
        if(count == 0)
            break;
    }
    tempJpgChar[i] = '\0';
    strrevCon(jpgChar,tempJpgChar);
    strcat(jpgChar,".jpg");
    if(delCount < 0)
        return;
    i = 0;
    while(1)
    {
        num = delCount % 10;
        delCount /= 10;
        tempJpgChar[i++] = num + 48;
        if(delCount == 0)
            break;
    }
    tempJpgChar[i] = '\0';
    strrevCon(delJpgChar,tempJpgChar);
    strcat(delJpgChar,".jpg");
    remove(delJpgChar);
}

void GetPicture::run()
{
    struct timeval tvafter,tvpre;
    camera->camera_init(640, 480);
    gettimeofday(&tvpre,NULL);
    while(1)
    {
        gettimeofday(&tvafter,NULL);
        while((tvafter.tv_sec - tvpre.tv_sec) * 1000 + (tvafter.tv_usec - tvpre.tv_usec) / 1000 < 300)
            gettimeofday(&tvafter,NULL);
        memcpy(&tvpre,&tvafter,sizeof(struct timeval));
        convertJpgChar(count++);
        camera->capture(jpgChar);
        printf("C-->%s\n",jpgChar);
    }
}
