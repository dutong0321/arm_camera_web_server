/* httpd.c:  A very simple http server
 * Copyfight (C) 2003      Zou jian guo <ah_zou@163.com>
 * Copyright (C) 2000  	   Lineo, Inc.  (www.lineo.com)
 * Copyright (c) 1997-1999 D. Jeff Dionne <jeff@lineo.ca>
 * Copyright (c) 1998      Kenneth Albanowski <kjahds@kjahds.com>
 * Copyright (c) 1999      Nick Brok <nick@nbrok.iaehv.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
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
#include "pthread.h"

int KEY_QUIT=0;
int TIMEOUT=30;

#ifndef O_BINARY
#define O_BINARY 0
#endif

char referrer[128];
long long count = 0;
char jpgChar[20],tempJpgChar[20],delJpgChar[20];
int content_length;

#define SERVER_PORT 80
void capture(char *filename);

int PrintHeader(FILE *f, int content_type)
{
	alarm(TIMEOUT);
	fprintf(f,"HTTP/1.0 200 OK\n");
	switch (content_type)
	{
	case 't':
		fprintf(f,"Content-type: text/plain\n");
		break;
	case 'j':
		fprintf(f,"Content-type: image/jpeg\n");
		break;
	case 'h':
		fprintf(f,"Content-type: text/html\n");
		break;
	}
	fprintf(f,"Server: uClinux-httpd 0.2.2\n");
	fprintf(f,"Expires: 0\n");
	fprintf(f,"\n");
	alarm(0);
	return(0);
}

int DoJpeg(FILE *f, char *name)
{
	char *buf;
	FILE * infile;

    printf("S<--%s\n",name);
	if (!(infile = fopen(name, "r")))
	{
		alarm(TIMEOUT);
		fprintf(stderr, "Unable to open JPEG file %s, %d\n", name, errno);
		fflush(f);
		alarm(0);
		return -1;
	}

	PrintHeader(f,'j');


	copy(infile,f); /* prints the page */

	alarm(TIMEOUT);
	fclose(infile);
	alarm(0);

	return 0;
}

int ParseReq(FILE *f, char *r)
{
	char *bp;
	struct stat stbuf;
	char * arg;
	char * c;
	int e;
	int raw;

	while(*(++r) != ' ');  /*skip non-white space*/
	while(isspace(*r))
		r++;

	while (*r == '/')
		r++;
	bp = r;
	while(*r && (*(r) != ' '))
		r++;
	arg = 0;
	*r = 0;
	c = bp;

	
	if (c[0] == 0x20)
	{
		c[0]='.';
		c[1]='\0';
	}
	if(c[0] == '\0') strcat(c,".");
	if (c && !stat(c, &stbuf))
	{
		if (S_ISDIR(stbuf.st_mode))
		{
			PrintHeader(f,'h');
			alarm(TIMEOUT);
			fprintf(f, "<html><head><meta http-equiv=\"refresh\" content=\"120\"><meta http-equiv=\"Pragma\" content=\"no-cache\"><meta http-equiv=\"Cache-Control\" content=\"no-cache\"> <meta http-equiv=\"Ecpires\" content=\"0\"> <meta charset=\"UTF-8\"><title>videoPlayer</title><script>var count = ");
			fprintf(f,"%lld",count-2);
			fprintf(f, ";setInterval(function() {var element = document.getElementById('myimage');element.src = count + \".jpg\";count++;}, 300);</script></head>\n");
			fprintf(f, "<body><img id=\"myimage\" src=\"\" /></body></html>\n");
			alarm(0);
		}
		else if (!strcmp(r - 4, ".jpg") || !strcmp(r - 5, ".jpeg"))
			DoJpeg(f,c);
	}
	else
	{
		PrintHeader(f,'h');
		alarm(TIMEOUT);
		fprintf(f, "<html><head><title>404 File Not Found</title></head>\n");
		fprintf(f, "<body>The requested URL was not found on this server</body></html>\n");
		alarm(0);
	}
	return 0;
}

void sigalrm(int signo)
{
	exit(0);
}

int HandleConnect(int fd)
{
	FILE *f;

	char buf[160];
	char buf1[160];

	f = fdopen(fd,"a+");
	if (!f)
	{
		fprintf(stderr, "httpd: Unable to open httpd input fd, error %d\n", errno);
		alarm(TIMEOUT);
		close(fd);
		alarm(0);
		return 0;
	}
	setbuf(f, 0);

	alarm(TIMEOUT);

	if (!fgets(buf, 150, f))
	{
		fprintf(stderr, "httpd: Error reading connection, error %d\n", errno);
		fclose(f);
		alarm(0);
		return 0;
	}

	alarm(0);
	referrer[0] = '\0';
	content_length = -1;

	alarm(TIMEOUT);
	while (fgets(buf1, 150, f) && (strlen(buf1) > 2))
	{
		alarm(TIMEOUT);
		if (!strncasecmp(buf1, "Referer:", 8))
		{
			char * c = buf1+8;
			while (isspace(*c))
				c++;
			strcpy(referrer, c);
		}
		else if (!strncasecmp(buf1, "Referrer:", 9))
		{
			char * c = buf1+9;
			while (isspace(*c))
				c++;
			strcpy(referrer, c);
		}
		else if (!strncasecmp(buf1, "Content-length:", 15))
		{
			content_length = atoi(buf1+15);
		}
	}
	
	alarm(0);
	if (ferror(f))
	{
		fprintf(stderr, "http: Error continuing reading connection, error %d\n", errno);
		fclose(f);
		return 0;
	}

	ParseReq(f, buf);

	alarm(TIMEOUT);
	fflush(f);
	fclose(f);
	alarm(0);
	return 1;
}

// 反转字符串
void strrevCon(char* str,char* temp)
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
void convertJpgChar(long long count)
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

void* getPicture(void* data)
{
	struct timeval tvafter,tvpre;
	camera_init(640, 480);
	gettimeofday(&tvpre,NULL);
	while(1)
	{
		gettimeofday(&tvafter,NULL);
		while((tvafter.tv_sec - tvpre.tv_sec) * 1000 + (tvafter.tv_usec - tvpre.tv_usec) / 1000 < 300)
			gettimeofday(&tvafter,NULL);
		memcpy(&tvpre,&tvafter,sizeof(struct timeval));
		convertJpgChar(count++);
		capture(jpgChar);
        printf("C-->%s\n",jpgChar);
	}
}

int main(int argc, char *argv[])
{
	int fd, s;
	int len;
	volatile int true = 1;
	struct sockaddr_in ec;
	struct sockaddr_in server_sockaddr;
	pthread_t th_key;
	void * retval;


	signal(SIGCHLD, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, sigalrm);

	//chroot(HTTPD_DOCUMENT_ROOT);
	printf("starting httpd...\n");

	if((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		perror("Unable to obtain network");
		exit(1);
	}

	if((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (void *)&true,
	               sizeof(true))) == -1)
	{
		perror("setsockopt failed");
		exit(1);
	}

	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(SERVER_PORT);
	server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(s, (struct sockaddr *)&server_sockaddr,
	        sizeof(server_sockaddr)) == -1)
	{
		perror("Unable to bind socket");
		exit(1);
	}

	if(listen(s, 8*3) == -1)
	{
		perror("Unable to listen");
		exit(4);
	}


	pthread_create(&th_key, NULL, getPicture, 0);
	while (1)
	{

		len = sizeof(ec);
		if((fd = accept(s, (void *)&ec, &len)) == -1)
		{
			exit(5);
			close(s);
		}
		HandleConnect(fd);

	}
	pthread_join(th_key, &retval);
}
