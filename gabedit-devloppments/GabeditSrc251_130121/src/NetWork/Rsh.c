/* Rsh.c */
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
*                                                               *
*          Execute a shell command in remote host               *
*          rsh - Remote Shell Client for Windows 95/98 system   *
*          rsh - Remote Shell Client using rsh system           *
*                command for a unix/Linux system                *
*                                                               *
****************************************************************/
#include "../../Config.h"
#include <glib.h> /* definition of G_OS_WIN32 if windows */
#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"

#ifdef G_OS_WIN32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>
#include <fcntl.h>
#include <io.h>

/* local socket address structure */
struct sockaddr_in anaddr;

/* server socket address structure */
struct sockaddr_in saddr;

/* the local rsh port; determined dynamically */
u_short rshPort;

/* the local rsh port for client FileErr output */
u_short rshErrPort;

/* the remote rsh port; basically, the 'shell' port from services */
u_short rshSPort;

/* the rsh protocol ("tcp") */
u_short rshProto;

/* the rsh client socket for outgoing connections */
SOCKET rshClient=INVALID_SOCKET;

/* the rsh client socket for FileErr input */
SOCKET rshClientErr=INVALID_SOCKET;

char userName[64]; 
char cmd[4096];

/* set when a connection has been detected on the FileErr channel */
int FileErrFlag=0; 

/* socket options variables */
int on=1;
struct linger linger;
static FILE* FileErr = NULL;
static FILE* FileOut = NULL;

/********************************************************************
*	winsockError :displays the current Winsock error in text format *
********************************************************************/
void winsockError()
{
	int errCode;
    
	fprintf(FileErr, "Winsock error: ");

    errCode=WSAGetLastError();
    switch(errCode)
    {
        case WSAENETDOWN:
            fprintf(FileErr, "The network subsystem has failed.\n");
            break;
        case WSAEINTR:
            fprintf(FileErr, "A blocking call was cancelled.  This can be caused by\n1) a short response time, or\n2) User interrupts the process.\n");
            break;
        case WSAEINPROGRESS:
            fprintf(FileErr, "A blocking call is in progress.\n");
            break;
        case WSAENOBUFS:
            fprintf(FileErr, "No buffer space is available.\n");
            break;
        case WSAENOTSOCK:
            fprintf(FileErr, "Invalid socket descriptor.\n");
            break;
        case WSAEADDRINUSE:
            fprintf(FileErr, "The specified address is already in use.\n");
            break;
        case WSAEADDRNOTAVAIL:
            fprintf(FileErr, "The specified address is not available\nfrom the local machine.\n");
            break;
        case WSAECONNREFUSED:
            fprintf(FileErr, "The connection attempt was refused.\n");
            break;
        case WSAEINVAL:
            fprintf(FileErr, "The socket is not bound to an address.\n");
            break;
        case WSAEISCONN:
            fprintf(FileErr, "The socket is already connected.\n");
            break;
        case WSAEMFILE:
            fprintf(FileErr, "The maximum number of sockets has exceeded.\n");
            break;
        case WSAENETUNREACH:
            fprintf(FileErr, "Network cannot be reached from this host at this time.\n");
            break;
        case WSAETIMEDOUT:
            fprintf(FileErr, "Attempt to connect timed out without establishing a connection.\n");
            break;
        case WSAENOTCONN:
            fprintf(FileErr, "The socket is not connected.\n");
            break;
        case WSAESHUTDOWN:
            fprintf(FileErr, "The socket has been shut down.\n");
            break;
        case WSAECONNABORTED:
            fprintf(FileErr, "The virtual circuit was aborted due to timeout or other failure.\n");
            break;
        case WSAECONNRESET:
            fprintf(FileErr, "The virtual circuit was reset by the remote side.\n");
            break;
        case WSAEACCES:
            fprintf(FileErr, "The requested address is a broadcast address.\n");
            break;
        case WSAENETRESET:
            fprintf(FileErr, "The connection must be reset.\n");
            break;
        case WSAHOST_NOT_FOUND:
            fprintf(FileErr, "Authoritative Answer Host is not found.\n");
            break;
        default:
			fprintf(FileErr, "%d.\n", errCode);
            break;
    }
}

/************************************************************************
*  error : display an error message and possibly the last Winsock error *
*************************************************************************/
void error (char* message, int ex)
{
    fprintf(FileErr,"%s\n",message);
/*	Message(message,"Error",TRUE);*/
    winsockError();
    if(ex)
    {
        WSACleanup();
        return;
    }
}

/************************************************************************
* rresvport : the windows hack of rresvport;							*
* bind a socket to a reserved port using the given protocol, if any		*
************************************************************************/
int rresvport (u_short* alport, int sProto)
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
/****************************************************
* send to the server the assembled command buffer	*
*****************************************************/ 
void sendcommand (const char* buff, int bufflen, SOCKET rshClient,int flag)
{
    if(send(rshClient, buff, bufflen, flag) < bufflen)
        error("Error sending command.", 0);
}


/****************************************************
*  receive : receive a string from the given socket	*
*****************************************************/
int receive (SOCKET rshClient, char* buff, int blen)
{
    int bufflen;
    int totallen=0;
    do
    {
        bufflen=recv(rshClient, buff+totallen, blen-totallen, 0);
        if(bufflen==SOCKET_ERROR)
            return bufflen;
        totallen+=bufflen;
    } while(bufflen && totallen<blen && buff[bufflen-1]);
    if(!totallen)
        buff[0]=0;
    buff[totallen]=0;
    return totallen;
}

/************************************************
* hostCheck : check the remote host name and	* 
* fill the server address structure				*
*************************************************/
gboolean hostCheck (const char* hostname)
{
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_addr.s_addr=inet_addr(hostname);
	if(saddr.sin_addr.s_addr==(u_long)INADDR_NONE)
	{
		/* 
			we must have gotten a host name instead 
			of an IP address; resolve!
		*/
		struct hostent* hostInfo=gethostbyname(hostname);
		if(!hostInfo)
		{
			error("Invalid hostname!",1);
			return FALSE;
		}
		memcpy((void*)&saddr.sin_addr.s_addr, hostInfo->h_addr, hostInfo->h_length);
	}
	return TRUE;
}

/********************************************************
* initSocket : standard socket initialization procedure	*
*********************************************************/ 
gboolean initSocket ()
{
    /* get port number for rsh */
    struct servent FAR* sp=getservbyname("shell", "tcp");
	LPPROTOENT lpProto;
    if(sp==NULL)
	{
        error("Cannot determine port number for the rsh client.",1);
		return FALSE;
	}
    rshSPort=htons(sp->s_port);

    /* get protocol number for tcp */
    lpProto=getprotobyname("tcp");
    if(!lpProto)
        rshProto=IPPROTO_TCP;
    else
        rshProto=lpProto->p_proto;

    /* create socket */
    rshClient=rresvport(&rshPort, rshProto);
    if(rshClient==INVALID_SOCKET)
	{
        error("Cannot allocate socket for the rsh client.",1);
		return FALSE;
	}
    if(setsockopt(rshClient, SOL_SOCKET, SO_KEEPALIVE, (char*)&on, sizeof(on))<0)
	{
        error("Cannot set SO_KEEPALIVE!\n", 0);
		return FALSE;
	}
    linger.l_onoff=1;
    linger.l_linger=60;
    if(setsockopt(rshClient, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger))<0)
	{
        error("Cannot set SO_LINGER!\n", 0);
		return FALSE;
	}
	return TRUE;
}

/********************************************************************
* openErrSocket : an additional socket is created for FileErr output *
*********************************************************************/
gboolean initErrSocket ()
{
    /* create the new socket and bind it to the client FileErr port */
    rshErrPort=IPPORT_RESERVED-1;
    rshClientErr=rresvport(&rshErrPort,0);
    if(rshClientErr==INVALID_SOCKET)
    {
        error("Cannot create FileErr socket!", 0);
        return FALSE;
    }

    if(setsockopt(rshClientErr, SOL_SOCKET, SO_KEEPALIVE, (char*)&on, sizeof(on))<0)
        error("Cannot set SO_KEEPALIVE!", 0);
    linger.l_onoff=0;
    linger.l_linger=60;
    if(setsockopt(rshClientErr, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger))<0)
        error("Cannot set SO_LINGER!", 0);
	/*  now listen... */
	if(listen(rshClientErr, 5))
		error("Cannot listen!",1);
    return TRUE;
}

/********************************************************
* rsh_command : pass the command string to the rsh server	*
* and retrieve the results								*
*********************************************************/
void rsh_command (const char* cmd, const char* userName)
{
	char cmdbuff[2048];
	int cmdbufflen=0;
	char buff[2048];
    int respbufflen;
    int firstbuf;
	int i = 0;
	
	/*memset(cmdbuff, 0, 2048*sizeof(char));*/
	for(i=0;i<2047;i++)
	{
		cmdbuff[i] = ' ';
	}
	/* local FileErr port */
	sprintf(cmdbuff+cmdbufflen, "%d", rshErrPort); 
	cmdbufflen=strlen(cmdbuff)+1;
	/* remot user */
	strcpy(cmdbuff+cmdbufflen, userName); 
	cmdbufflen+=strlen(userName)+1;
	/* local user */
	strcpy(cmdbuff+cmdbufflen, userName); 
	cmdbufflen+=strlen(userName)+1;
	/* command */
	strcpy(cmdbuff+cmdbufflen, cmd); 
	cmdbufflen+=strlen(cmd)+1;
		
	sendcommand(cmdbuff, cmdbufflen, rshClient,0);
	
    /* for some reason first buffer has nul char at begining */
/*	Debug("End Send command\n");*/
    firstbuf = 1;
	if(!strstr(cmd,"&"))
	while((respbufflen=receive(rshClient, buff, 2047))>0)
	{
		fprintf(FileOut, "%s", buff);
/*		fwrite(buff, respbufflen, 1, FileOut);*/
	}
}

/********************************************************
* clientThread : this is the FileErr client thread;		*
* it is started beforee sending the command string		*
* to the server; its purpose is to accept connections	*
* from the server and receive the FileErr output		*
*********************************************************/
long clientThread ()
{
	struct sockaddr anaddr;
    int len=sizeof(anaddr);
	char buff[2048];

    SOCKET rshServer=accept(rshClientErr, (struct sockaddr FAR*)&anaddr, &len);
	if(rshServer==INVALID_SOCKET)
	{
		error("Error connecting to the FileErr server port!", 0);
		return 0;
	}

	FileErrFlag=1; /* mark connection made*/

	while(receive(rshServer, buff, 2047)>0)
        fprintf(FileErr, "%s", buff);
    FileErrFlag=0;
	closesocket(rshClientErr);
    shutdown(rshClientErr, 2);
	closesocket(rshServer);
    shutdown(rshServer, 2);
    

	return 0;
}

/********************************************************
* rsh : main processing routine; connect to server,		*
* pass command line and wait for results				*
*********************************************************/
void rsh (char *fout,char *ferr,const char* cmd,
	       const char *hostname,const char* userName,char *passwod)
{
	HANDLE threadHnd = NULL;

	FileOut = FOpen(fout,"w");
	FileErr = FOpen(ferr,"w");

	/*
	Debug("fout=%s\n",fout);
	Debug("fout=%s\n",ferr);

	Debug("Begin winsockCheck\n");
	*/
	if( !winsockCheck(FileErr) )
		goto closeall;

	/*
	Debug("End winsockCheck\n");

	Debug("Begin hostCeck\n");
	*/
	if( !hostCheck(hostname) )
		goto closeall;

	/*
	Debug("End hostCeck\n");

	Debug("Begin initSocket\n");
	*/
    if( !initSocket() )
		goto closeall;

/*	Debug("End initSocket\n");*/

	if(!initErrSocket())
	{
		error("Cannot create error socket!", 0);
		goto closeall;
	}

	saddr.sin_family=AF_INET;
	saddr.sin_port=rshSPort;
	if(connect(rshClient, (struct sockaddr FAR*)&saddr, sizeof(saddr)))
	{
		error("Cannot connect to RSH port!\n",1);
		goto closeall;
	}
	

	if(rshClientErr!=INVALID_SOCKET)
	{
		DWORD threadID;
		threadHnd=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)clientThread,
			(LPVOID)0, 0, (LPDWORD)&threadID);
		if(!threadHnd)
		{
			error("Cannot start client thread...",1);
			goto closeall;
		}
	}
/*	Debug("cmd = %sg\n",cmd);*/
    rsh_command(cmd, userName);
/*	Debug("End rsh_command\n");*/
	closesocket(rshClient);
    shutdown(rshClient, 2);

	if(!strstr(cmd,"&"))
	if(threadHnd)
	{
		DWORD exitCode=0;
		GetExitCodeThread(threadHnd, &exitCode);
		while(exitCode==STILL_ACTIVE && FileErrFlag)
		{
			Sleep(50);
			GetExitCodeThread(threadHnd, &exitCode);
		}
		CloseHandle(threadHnd);
	}
	if(strstr(cmd,"&") && threadHnd)
	{
		Sleep(500);
		shutdown(rshClientErr, 2);
	}
	
/*	Debug("End rThread\n");*/
	WSACleanup();

closeall :
	if(FileOut)
		fclose(FileOut);
	if(FileErr)
		fclose(FileErr);

}
#else /* G_SO_WIN32 */
/********************************************************
* rsh : main processing routine; connect to server,	*
* pass command line and wait for results				*
*********************************************************/
#include <stdlib.h>

void rsh (char *fout,char *ferr,const char* cmd,
	       const char *hostname,const char* userName,char *passwod)
{
	gchar *commandrcp;
	commandrcp = g_strdup_printf(
					"sh -c \"sh -c 'rsh -l%s %s %s' >%s 2>%s\"",
					userName,hostname,cmd,
					fout,ferr);
/*	Debug("%s\n",commandrcp);*/
	{int ierr = system(commandrcp);}
	g_free(commandrcp);
}
#endif /* G_SO_WIN32 */
/*********************************************************/

