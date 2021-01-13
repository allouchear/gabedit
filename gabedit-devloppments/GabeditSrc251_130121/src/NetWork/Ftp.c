/*Ftp.c */
/**********************************************************************************************************
Copyright (c) 2002-2013 Abdul-Rahman Allouche. All rights reserved

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the Gabedit), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions
  of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
************************************************************************************************************/

/****************************************************************
*								*
*	Files Transfer between local host and a remote host	*
*		 ftp Client for a local Windows 95/98 system	*
*		 rcp Client for a local unix/Linux system	*
*								*
****************************************************************/

#include "../../Config.h"
#include <glib.h> /* definition of G_OS_WIN32 if windows */
#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Common/Status.h"

#ifndef BUFSIZ
#define BUFSIZ 8192
#endif
#ifdef G_OS_WIN32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>
#include <fcntl.h>
#include <io.h>
#include <direct.h>
#else /* G_OS_WIN32 */

#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h> 
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#endif /* G_OS_WIN32 */


/*****************************************/
char *HostName = NULL;
char *UserName = NULL;
char *PassWord = NULL;
char *home;
static int data = -1;
static struct    sockaddr_in remote_addr;
static int cinput = -1; 
static int coutput = -1; 
static FILE* FileErr = NULL;
static FILE* FileOut = NULL;
static long FileSize = 0;

/*********************************************************************/
static void CloseSocket(int s)
{
#ifndef  G_OS_WIN32
	close(s);
#else
       closesocket(s);
#endif 
	
}
/*********************************************************************/
int ReadMsg(int cin, char *szBuffer, int len)
{
  	int ret;
 
	if( (ret=recv(cin,szBuffer,len,0)) <= 0)
       		return 0;
 
	if(szBuffer[strlen(szBuffer)-2]=='\r')
		szBuffer[strlen(szBuffer)-2]=' ';
	return ret;
}
/*********************************************************************/
int SendMsg(int cout, char *szBuffer, int len)
{
   	if( send(cout,szBuffer,len,0) <= 0)
       		return 0;
   	return 1;
}      
/************************************************
* put_file_to_data : put file in data socket	*
*************************************************/
long put_file_to_data(int fout,char* filename)
{
	FILE *fin = NULL;
	long bytes = 0;
	char c;
	char buf[2];

	buf[1] = '\0';

    if( (fin = FOpen(filename,"r")) != NULL)
	{
		while (!feof(fin) && (c = getc(fin)) != EOF)
		{
			if(c == '\n')
			{
				buf[0] ='\r';
 				SendMsg(fout,buf,1);
			}
			buf[0] =c;
 			SendMsg(fout,buf,1);
			bytes++;
		}
		fclose(fin);

		return bytes;
	}
	return 0;
}
/****************************************************
* get_file_from_data : get file from data socket	*
****************************************************/
long get_file_from_data(int din,char* filename)
{
	FILE *fout = NULL;
	long bytes = 0;
	char buf[2];
	long step = FileSize/100;
	gchar* str = g_strdup_printf(_("Get \"%s\" File from \"%s\" host : %%p%%%%"),filename,HostName);
	buf[1] = '\0';

        if( (fout = FOpen(filename,"w")) != NULL && !stopDownLoad)
	{
		while (ReadMsg(din,buf, 1)>0 &&  buf[0] != EOF && !stopDownLoad)
		{
			if(buf[0] != '\r')
 				putc(buf[0], fout);
			bytes++;
			if(bytes%step == 0)
			{
				progress_connection((gdouble)0.01,str,FALSE);
			}
		}
		fclose(fout);
		g_free(str);
		return bytes;
	}
	return 0;
}
/****************************************
* get_reply : get replay of ftp server	*
****************************************/
int get_reply()
{
    char reply_string[BUFSIZ];
                                 

	memset (reply_string, 0, BUFSIZ);
#ifdef G_OS_WIN32
	Sleep(50);
#endif
	for (;;)
	{
		if(ReadMsg(cinput,reply_string, sizeof(reply_string))<=0)
		{
			fprintf(FileErr,_("Error in get_reply\n"));
			fflush(FileErr);
			return 1;
		};
		fprintf(FileOut,"%s",reply_string);
		fflush(FileOut);
		if(strstr(reply_string,"incorrect") || strstr(reply_string,"failed"))
		{
			fprintf(FileErr, "Login failed.\n");
			return 1;
		}
		if(strstr(reply_string,"No such file or directory"))
		{
			fprintf(FileErr, "No such file or directory\n");
			return 2;
		}
		if(strstr(reply_string,"command not understood"))
		{
			fprintf(FileErr, "command not understood\n");
			return 3;
		}

		return 0;
	}
}
/************************************************
* ftp_command : execute command in ftp connection	*
************************************************/
int ftp_command(char *fmt,...)
{
	va_list ap ;
        char cmd_string[BUFSIZ];

	memset (cmd_string, 0, BUFSIZ);
	if (coutput<0)
	{
		fprintf (FileErr,"No control connection for command\n");
		return -1;
	}

	va_start(ap,fmt);
	vsprintf(cmd_string, fmt, ap);
	va_end(ap);
	strcat(cmd_string,"\r\n");
 	SendMsg(cinput,cmd_string,strlen(cmd_string));
	return get_reply() ;
}
/************************************************************
* data_connection : new socket for data						*
************************************************************/
int data_connection(char *mode,char *cmd,char *filename)
{
	struct  sockaddr_in data_addr;
	struct  sockaddr_in from_addr;
        int s;
	char *pos;
	char *pos1;
	int   datain = -1; 
#ifdef G_OS_WIN32
        gint  len = sizeof (data_addr);
#else
        socklen_t len = sizeof (data_addr);
#endif

	memset((char *)&data_addr,0, sizeof (data_addr));
	data_addr.sin_addr.s_addr = INADDR_ANY;;
	data_addr.sin_family = AF_INET;
 
	if (data != -1)
               close (data); 
 	data = socket(AF_INET, SOCK_STREAM,  IPPROTO_TCP);
 	if (getsockname(cinput, (struct sockaddr *)&data_addr, &len) < 0)
	{
		fprintf(FileErr,"ftp: getsockname");
		return -1;
        }   

 	data_addr.sin_port =  0; 

	if (bind(data, (struct sockaddr *)&data_addr, sizeof (data_addr)) < 0)
	{
		fprintf(FileErr,"ftp: bind");
		return -1;
	}      
 	if (getsockname(data, (struct sockaddr *)&data_addr, &len) < 0) 
	{
		fprintf(FileErr,"ftp: getsockname");
		return -1;
    	}   

	if (listen(data, 1) < 0)
	{
		fprintf(FileErr,"ftp: listen"); 
		return -1;
	}

	pos = (char *) &data_addr.sin_addr;
	pos1 = (char *) &data_addr.sin_port;
/*
	printf ("PORT %d,%d,%d,%d,%d,%d\r\n",
			pos[0] & 0xff, pos[1] & 0xff, pos[2] & 0xff,
			pos[3] & 0xff, pos1[0] & 0xff,
			pos1[1] & 0xff);   
*/
	if( ftp_command ( "PORT %d,%d,%d,%d,%d,%d",
				pos[0] & 0xff, pos[1] & 0xff, pos[2] & 0xff,
				pos[3] & 0xff, pos1[0] & 0xff,
				pos1[1] & 0xff
			)
	  != 0)
		return -1;   

	if(ftp_command(cmd,filename) != 0)
		return -1;

	if( (datain = data)<0)
	{
		fprintf(FileErr,"ftp: fdopen");
		return -1;
	}

	s = accept(datain, (struct sockaddr *) &from_addr, &len);

	if (s < 0)
	{
		fprintf(FileErr,"ftp: accept");
  		CloseSocket(data);
		data = -1;
		return -1;
	}

  	CloseSocket(data);
	data = s;
    return data;
}          

/********************************************
* login : put user name and password name	*
********************************************/
int login(char* host,char *user,char *pass)
{
	int n = 0;


	n = ftp_command("USER %s", user);

	if (n == 0)
	{
		n = ftp_command("PASS %s", pass);
	}
	else
		return -1;

	if (n != 0)
	{
		fflush(FileErr);
		return -1;
	}
	return 0;
}
#ifdef G_OS_WIN32
/************************************************
* hostCheck : check the remote host name and	* 
* fill the server address structure				*
* Window version								*
*************************************************/
static gboolean hostCheck (const char* hostname)
{
	HostName = NULL;
	memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_addr.s_addr=inet_addr(hostname);
	remote_addr.sin_family = AF_INET;
	if(remote_addr.sin_addr.s_addr==(u_long)INADDR_NONE)
	{
		/* 
			we must have gotten a host name instead 
			of an IP address; resolve!
		*/
		struct hostent* hostInfo=gethostbyname(hostname);
		if(!hostInfo)
		{
			fprintf(FileErr,"Invalid hostname!");
			return FALSE;
		}
		remote_addr.sin_family = hostInfo->h_addrtype;
		memcpy((void*)&remote_addr.sin_addr.s_addr, hostInfo->h_addr, hostInfo->h_length);
	}
	HostName = g_strdup(hostname);

	return TRUE;
}
/************************************************************************
* rresvport : the windows hack of rresvport;							*
* bind a socket to a reserved port using the given protocol, if any		*
* Window version														*
************************************************************************/
static int rresvport (u_short* alport, int sProto)
{
    struct sockaddr_in sin;
    int s;

    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=INADDR_ANY;
    s=socket(AF_INET, SOCK_STREAM, sProto);
    if(s<0)
        return -1;

    for((*alport)=IPPORT_RESERVED-1; (*alport)>IPPORT_RESERVED/2; (*alport)--)
    {
        sin.sin_port=htons((u_short)(*alport));
        if(bind(s, (struct sockaddr*)&sin, sizeof(sin))==0)
            return s;
        if(WSAGetLastError()!=WSAEADDRINUSE)
            break;
    }
    /* ran out of available ports or weird error; shouldn't happen too often...*/
    closesocket(s);
    return -1;
}
/********************************************************
* initSocket : standard socket initialization procedure	*
* Window version					*
*********************************************************/ 
static int initSocket ()
{
    /* get port number for ftp */
    struct servent FAR* sp=getservbyname("ftp", "tcp");
	LPPROTOENT lpProto;
	SOCKET ftpClient;
	/* the local rsh port; determined dynamically */
	u_short ftpPort;

	/* the rsh protocol ("tcp") */
	u_short ftpProto;
	int on=1;

	struct linger linger;
	/* struct hostent *hp = NULL;*/

    if(sp==NULL)
	{
        fprintf(FileErr, "Cannot determine port number for the ftp client.");
		return INVALID_SOCKET;
	}
	remote_addr.sin_port = sp->s_port;

    /* get protocol number for tcp */
    lpProto=getprotobyname("tcp");
    if(!lpProto)
        ftpProto = IPPROTO_TCP;
    else
        ftpProto = lpProto->p_proto;

    /* create socket */
    ftpClient=rresvport(&ftpPort, ftpProto);
    if(ftpClient==INVALID_SOCKET)
	{
        fprintf(FileErr, "Cannot allocate socket for the ftp client.");
		return INVALID_SOCKET;
	}
    if(setsockopt(ftpClient, SOL_SOCKET, SO_KEEPALIVE, (char*)&on, sizeof(on))<0)
	{
        fprintf(FileErr, "Cannot set SO_KEEPALIVE!\n");
		return INVALID_SOCKET;
	}
    linger.l_onoff=1;
    linger.l_linger=60;
    if(setsockopt(ftpClient, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger))<0)
	{
        fprintf(FileErr,"Cannot set SO_LINGER!\n");
		return INVALID_SOCKET;
	}
	return ftpClient;
}
#else
/************************************************
* hostCheck : check the remote host name and	* 
* fill the server address structure				*
* Unix version									*
*************************************************/
static gboolean hostCheck (const char* hostname)
{
	struct hostent *hp = NULL;
	memset((char *)&remote_addr, 0,sizeof (remote_addr));
	remote_addr.sin_addr.s_addr = inet_addr(hostname);

	if (remote_addr.sin_addr.s_addr != (u_long)INADDR_NONE)
	{
		remote_addr.sin_family = AF_INET;
		HostName = g_strdup(hostname);
	}
	else
	{
		hp = gethostbyname(hostname);
		if (!hp)
		{
			fprintf(FileErr, "ftp: unknown host %s ", hostname);
			return FALSE;
		}
		remote_addr.sin_family = hp->h_addrtype;
		memcpy(&remote_addr.sin_addr,hp->h_addr_list[0],hp->h_length);
		HostName = g_strdup(hp->h_name);
	}
	return TRUE;
}
/********************************************************
* initSocket : standard socket initialization procedure	*
* Unix version											*
*********************************************************/ 
static int initSocket ()
{
	struct servent *sp;
	int s;

	s = socket(remote_addr.sin_family, SOCK_STREAM, 0);

	if (s < 0) {
		fprintf(FileErr,"ftp: socket");
		return -1;
	}
        sp = getservbyname("ftp", "tcp");

	remote_addr.sin_port = sp->s_port;

	return s;
}
#endif /*G_OS_WIN32*/
/********************************************
* tcpopen : open tcp connection				*
********************************************/
int tcpopen(char *host)
{

	int s;
#ifdef G_OS_WIN32
        gint  len;
#else
        socklen_t len;
#endif

#ifdef G_OS_WIN32
	if( !winsockCheck(FileErr) )
		return -1;
#endif
	if( !hostCheck(host) )
		return -1;
	if((s = initSocket())<0)
		return -1;

	while (connect(s, (struct sockaddr *)&remote_addr, sizeof (remote_addr)) < 0)
	{
		fprintf(FileErr,"ftp: connection impossible");
		return -1;
	}
	len = sizeof (remote_addr);
	if (getsockname(s, (struct sockaddr *)&remote_addr, &len) < 0)
	{
		fprintf(FileErr,"ftp: getsockname");
  		CloseSocket(s);
		return -1;
	}

	cinput = s;
	coutput = s;
	if (cinput<0 || coutput <0)
	{
		if(s>=0)
  			CloseSocket(s);
		return -1;
	}
   	return get_reply();
}
/********************************************************************************/
void  ftp_disconnect()
{
	if (data != -1)
  		CloseSocket(data);
	data = -1;
	if(coutput)
#ifndef G_OS_WIN32
 		ftp_command("QUIT");
#endif
	if(FileOut)
		fclose(FileOut);
        FileOut = NULL;
	if(FileErr)
        	fclose(FileErr);
        FileErr = NULL; 
	if(HostName)
		g_free(HostName);
	HostName = NULL;
	if(UserName)
		g_free(UserName);
	UserName = NULL;
	if(PassWord)
		g_free(PassWord);
	PassWord = NULL;
#ifdef G_OS_WIN32
		if(cinput)
  			CloseSocket(cinput);
		if(coutput)
  			CloseSocket(coutput);
		shutdown(coutput, 2);

#else
		if(cinput)
  			CloseSocket(cinput);
#endif

	cinput = -1;
	coutput = -1;
#ifdef G_OS_WIN32
	WSACleanup();
#endif
}
/********************************************************************************/
int  ftp_connection(char *hostname,char *username,char* password)
{
   int code = 0;
   if(tcpopen(hostname) == 0)
   {
   	if((code = login(hostname,username,password))!=0)
	{
		return code;
	}
	UserName = g_strdup(username);
	PassWord = g_strdup(password);
	code = ftp_command("PWD");
	if(code != 0)
		return code;
 	code = ftp_command("TYPE %s", "A");
	if(code != 0)
		return code;
	return 0;
  }
  return -1;


}
/*********************************************************************/
static long get_file_size(char* filename)
{
        char cmd_string[BUFSIZ];
    	char reply_string[BUFSIZ];
	long size;
	int i;

	memset (cmd_string, 0, BUFSIZ);
	if (coutput<0)
	{
		fprintf (FileErr,"No control connection for command\n");
		return -1;
	}

	sprintf(cmd_string,"SIZE %s\r\n",filename);
 	ftp_command("TYPE %s", "A");
 	SendMsg(cinput,cmd_string,strlen(cmd_string));

                                 

	memset (reply_string, 0, BUFSIZ);
#ifdef G_OS_WIN32
	Sleep(50);
#endif
	if(ReadMsg(cinput,reply_string, sizeof(reply_string))<=0)
	{
		fprintf(FileErr,"Error in get_file_size\n");
		fflush(FileErr);
		return -1;
	}
	if(strstr(reply_string,"No such file or directory"))
	{
		fprintf(FileErr, "No such file or directory\n");
		return -1;
	}
	if(strstr(reply_string,"command not understood"))
	{
		fprintf(FileErr, "command not understood\n");
		return -1;
	}
	if(strstr(reply_string,":"))
	{
		sscanf(strstr(reply_string,":")+1,"%ld",&size);

	}
	else
		sscanf(reply_string,"%d %ld",&i,&size);
	return size;
}
/*********************************************************************/
int ftp_get_file(char* fout,char* ferr,char* filename,char* localdir,char* remotedir,
			 char *hostname,char *username,char* password)
{
	int finput = -1;
	int code = 0;
	long bytes = 0;
	gchar* str = g_strdup_printf(_("Get \"%s\" File from remote host : %%p%%"),filename);

	FileOut = FOpen(fout,"w");
	FileErr = FOpen(ferr,"w");

	if( !FileOut  || !FileErr)
	{
		return -1;
	}
	
	show_progress_connection();
	progress_connection(0,_("Connecting...."),FALSE);
	if( !cinput || !coutput || !HostName || strcmp(HostName,hostname)  || !UserName || strcmp(UserName,username) )
	{
		if( (code = ftp_connection(hostname,username,password)) != 0)
			goto closeall;
	}

	if( (code = chdir(localdir)) != 0)
		goto closeall;
	if( (code = ftp_command("CWD %s",remotedir)) != 0)
		goto closeall;
	
	progress_connection(0,"Get File size ",FALSE);
	FileSize = get_file_size(filename);
	/*
	if( (code = ftp_command("SIZE %s",filename)) != 0)
		goto closeall;
		*/
	progress_connection(0,"Data connecting....",FALSE);
 	if( (finput = data_connection("r","RETR %s",filename))<0)
	{
		code = -1;
		goto closeall;
	}
	progress_connection(0,str,FALSE);
 	if( (bytes = get_file_from_data(finput,filename)) == 0)
	{
		code = -1;
		goto closeall;
	}

	if (data != -1)
	{
		CloseSocket(data);
#ifdef G_OS_WIN32
		shutdown(data, 2);
#endif
	}

	data = -1;
	finput = -1;
	get_reply();
	fprintf (FileOut,"    %ld bytes received.\n",bytes);

closeall :
	g_free(str);
	FileSize = 0;
	hide_progress_connection();
	progress_connection(0," ",TRUE);
	if(finput>=0)
  		CloseSocket(finput);
	finput = -1;
	ftp_disconnect();
	return code;
}
/******************************************************/
int ftp_put_file(char* fout,char* ferr,char* filename,char* localdir,char* remotedir,
			 char *hostname,char *username,char* password)
{
	int foutput = -1;
	int code = 0;
	long bytes = 0;

	stopDownLoad = FALSE;
	FileOut = FOpen(fout,"w");
	FileErr = FOpen(ferr,"w");
	if( !FileOut  || !FileErr)
	{
		return -1;
	}

	if( !cinput || !coutput || !HostName || strcmp(HostName,hostname)  || !UserName || strcmp(UserName,username) )
	{
		if( (code = ftp_connection(hostname,username,password)) != 0)
			goto closeall;
	}

	if( (code = chdir(localdir)) != 0)
		goto closeall;
	if( (code = ftp_command("CWD %s",remotedir)) != 0)
		goto closeall;
	if( (foutput = data_connection("w","STOR %s",filename))<0)
	{
		code = -1;
		goto closeall;
	}
 	if( (bytes = put_file_to_data(foutput,filename))<0)
	{
		code = -1;
		goto closeall;
	}

	if(foutput>=0)
  		CloseSocket(foutput);

	foutput = -1;
	get_reply();
	fprintf (FileOut,"    %ld bytes sent.\n",bytes);

closeall :
	if(foutput>=0)
  		CloseSocket(foutput);
	ftp_disconnect();
	return code;

}
/******************************************************/
