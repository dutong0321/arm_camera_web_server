#include "httpd.h"

int Httpd::copy(FILE *read_f, FILE *write_f)
{
  int n;
  int wrote;
  alarm(TIMEOUT);
  while (n = fread(copybuf,1,sizeof(copybuf),read_f)) {
    alarm(TIMEOUT);
    wrote = fwrite(copybuf,n,1,write_f);
    alarm(TIMEOUT);
    if (wrote < 1)
        return -1;
  }
  alarm(0);
  return 0;
}

Httpd::Httpd()
{
    KEY_QUIT=0;
    TIMEOUT=30;
}

int Httpd::PrintHeader(FILE *f, int content_type)
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

int Httpd::DoJpeg(FILE *f, char *name)
{
	FILE * infile;

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

int Httpd::ParseReq(FILE *f, char *r)
{
	char *bp;
	struct stat stbuf;
	char * arg;
	char * c;

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
            fprintf(f,"%lld",getPicture->count-2);
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

int Httpd::HandleConnect(int fd)
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

void Httpd::run()
{
	int fd, s;
	int len;
    volatile int trueInt;
	struct sockaddr_in ec;
    struct sockaddr_in server_sockaddr;

    trueInt = 1;
	signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
	printf("starting httpd...\n");

	if((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		perror("Unable to obtain network");
		exit(1);
	}
    if((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (void *)&trueInt,
                   sizeof(trueInt))) == -1)
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
    getPicture = new GetPicture();
    getPicture->start();
	while (1)
	{

		len = sizeof(ec);
        if((fd = accept(s, (sockaddr *)&ec,(socklen_t *) &len)) == -1)
		{
			exit(5);
			close(s);
		}
		HandleConnect(fd);

    }
}
