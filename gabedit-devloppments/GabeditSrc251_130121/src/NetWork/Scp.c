/* Scp.c */
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
*          Scp command                                          *
*          pscp - Remote Shell Client for Windows system        *
*          scp  - Remote Shell Client using ssh system          *
*                command for a unix/Linux system                *
*                                                               *
****************************************************************/
#include "../../Config.h"
#include <glib.h> /* definition of G_OS_WIN32 if windows */
#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Common/Status.h"


#ifdef G_OS_WIN32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>
#include <fcntl.h>
#include <io.h>


static gchar*	cmdGlobal = NULL;
static gchar*	foutGlobal = NULL;
static gchar*	ferrGlobal = NULL;

/********************************************************/
static gchar* createUnixFile(gchar* winFile)
{
	gchar *unixFile =  g_strdup_printf("%s%stmp%sunix.txt",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	FILE* FileUnix = NULL;
	FILE* FileWin = NULL;
	char c;

	FileUnix = FOpen(unixFile,"wb");
	if(!FileUnix)
	{
		g_free(unixFile);
		return NULL;
	}

	FileWin = FOpen(winFile,"r");
	if(!FileWin)
	{
		g_free(unixFile);
		return NULL;
	}
	while (!feof(FileWin) && (c = getc(FileWin)) != EOF)
	{
		if(c == '\r')
			continue;
		if(c == '\n')
		{
			/*putc('\r', FileUnix);*/
			putc('\n', FileUnix);
		}
		else
			putc(c, FileUnix);
	}
	fclose(FileWin);
	fclose(FileUnix);

	return unixFile;

}
/********************************************************/
static long clientThread ()
{
	if(system(cmdGlobal)==0)
	{
		FILE* FileOut = FOpen(foutGlobal,"w");
		fprintf(FileOut,"Ok\n");
		fclose(FileOut);
		return 0;
	}
	else
	{
		FILE* FileErr = FOpen(ferrGlobal,"w");
		FILE* FileOut = FOpen(foutGlobal,"w");
		fprintf(FileErr,_("Sorry, I Cannot get file at remot host...\n"));
		fprintf(FileOut,_("Sorry, I Cannot get file at remot host...\n"));
		fclose(FileErr);
		fclose(FileOut);
		return 1;
	}
}

/********************************************************
* scp : main processing routine; connect to server,		*
* pass command line and wait for results				*
*********************************************************/
int run_scp(gchar* fout,gchar* ferr,char* cmd, gchar* message)
{
	HANDLE threadHnd;
	DWORD threadID;
	
	FILE* FileOut;
	FILE* FileErr;


	if(cmdGlobal)
		g_free(cmdGlobal);

	cmdGlobal = g_strdup(cmd);

	show_progress_connection();
	progress_connection(0,message,FALSE);

	threadHnd=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)clientThread,
			(LPVOID)0, 0, (LPDWORD)&threadID);
	if(!threadHnd)
	{
			FILE* FileErr = FOpen(ferr,"w");
			fprintf(FileErr,_("Cannot start client thread...\n"));
			fclose(FileErr);
			hide_progress_connection();
			progress_connection(0," ",TRUE);
			WSACleanup();
			return 1;
	}
	/*printf("End start client thread\n");*/

	FileErr = FOpen(ferr,"w");
	FileOut = FOpen(fout,"w");
	if(threadHnd)
	{
		DWORD exitCode=0;
		GetExitCodeThread(threadHnd, &exitCode);
		while(exitCode==STILL_ACTIVE)
		{
			Sleep(50);
			GetExitCodeThread(threadHnd, &exitCode);
        		while(gtk_events_pending())
                		gtk_main_iteration();
			if(stopDownLoad)
			{
				fprintf(FileOut,_("Download Stopped\n"));
				break;
			}
		}
		CloseHandle(threadHnd);
	}

	fclose(FileErr);
	fclose(FileOut);

	hide_progress_connection();
	progress_connection(0," ",TRUE);

	Sleep(500);

	WSACleanup();

	return 0;
}

/******************************************************/
int scp_put_file(gchar* fout,gchar* ferr,
			 char* filename,char* localdir,char* remotedir,
			 char *hostname,char *username,char* password)
{
	gchar* winFile = g_strdup_printf("%s\\%s",localdir,filename);
	gchar* unixFile = createUnixFile(winFile);
	

	if(!unixFile)
	{
			FILE* FileErr = FOpen(ferr,"w");
			fprintf(FileErr,_("Sorry, I Cannot put file at remot host...\n"));
			fclose(FileErr);
			g_free(winFile);
			return 1;
	}

	cmdGlobal = g_strdup_printf(
		"%s -pw %s \"%s\" %s@%s:%s/%s",
		pscpCommand,
		password,
		unixFile,
		username,hostname,remotedir,filename);

	g_free(winFile);
	
	unlink(fout);
	unlink(ferr);
	if(system(cmdGlobal)!=0)
	{
			FILE* FileErr = FOpen(ferr,"w");
			fprintf(FileErr,_("Sorry, I Cannot put %s file at remot host...\n"),filename);
			fclose(FileErr);
			unlink(unixFile);
			g_free(unixFile);
			return 1;
	}
	/*
	return run_scp(fout,ferr,cmdGlobal);
	*/
	unlink(unixFile);
	g_free(unixFile);
	return 0;
}
/******************************************************/
int scp_get_file(gchar* fout,gchar* ferr,
			 char* filename,char* localdir,char* remotedir,
			 char *hostname,char *username,char* password)
{
	gchar* message;
	gchar* cmd = NULL;

	foutGlobal = g_strdup(fout);
	ferrGlobal = g_strdup(ferr);

	message = g_strdup_printf( _("Get \"%s/%s\" file from \"%s\" host..."),
					remotedir,filename,hostname
				);

	cmd = g_strdup_printf(
		"%s -pw %s %s@%s:%s/%s \"%s\\%s\"",
		pscpCommand,
		password,
		username,hostname,
		remotedir,filename,
		localdir,filename
		);

	unlink(fout);
	unlink(ferr);

	return run_scp(fout,ferr,cmd,message);

}
#else /* G_OS_WIN32 */
/********************************************************
* ssh : main processing routine; connect to server,	*
* pass command line and wait for results				*
*********************************************************/
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <glib.h>
#include <pthread.h>

extern char **environ;
static int endchild = 0;
static pid_t pidchild = -1;
static pid_t pidparent = -1;

/********************************************************/
static void* clientThread (gpointer data)
{
	char* cmd = (char*)data;
	pidchild = getpid();
	pidparent = getppid();

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	pthread_setcancelstate(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

	pthread_testcancel();
	{int ierr = system(cmd);}
	pthread_testcancel();
	endchild = 1;
	pthread_exit((void*)0);
	return (void*)0;
}
/********************************************************/
int run_scp(gchar* fout,gchar* ferr,char* cmd,char* message)
{
	pthread_attr_t attr;
	pthread_t threadId;
	int ret;
	FILE* FileOut;
	FILE* FileErr;

	/*printf("cmd = %s\n",cmd);*/

	show_progress_connection();
	progress_connection(0,message,FALSE);

        pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setstacksize(&attr,1000);
	endchild = 0;
	ret = pthread_create(&threadId, &attr, clientThread, cmd);
	pthread_detach(threadId);
	if(ret)
	{
		FILE* FileErr = FOpen(ferr,"w");
		fprintf(FileErr,_("Cannot start client thread...\n"));
		fclose(FileErr);
		return 0;
	}
	else

	FileErr = FOpen(ferr,"w");
	FileOut = FOpen(fout,"w");
	while(endchild==0)
	{
        	while(gtk_events_pending())
                	gtk_main_iteration();
		if(stopDownLoad)
		{
			if(pthread_cancel(threadId)!=0)
			{
				progress_connection(0,_("Sorry I can not stop download"),FALSE);
			}
			else
			{
				if(pidchild != -1)
				{
					/*
					char t[BSIZE];
					sprintf(t,"kill %d",pidchild);
					system(t);
					*/
					/*printf("pid child = %d\n",pidchild);*/
				}
				pidchild =-1;
				fprintf(FileOut,_("Download Stopped\n"));
				break;
			}
		}

	}
	fclose(FileErr);
	fclose(FileOut);

	hide_progress_connection();
	progress_connection(0," ",TRUE);
	if(pidparent!=-1)
	{
		/*
		char t[BSIZE];
		sprintf(t,"kill %d",pidparent);
		printf("pid parent = %d\n",pidparent);
		system(t);
		*/
		pidparent =-1;
	}


	return 0;
}
/********************************************************/
int scp_put_file(char* fout,char* ferr,
			 char* filename,char* localdir,char* remotedir,
			 char *hostname,char *username,char* password)
{
	gchar *command;
	gchar* message;

	message = g_strdup_printf( _("Put %s/%s file at %s host..."),
					remotedir,filename,hostname
				);


	show_progress_connection();
	progress_connection(0,message,FALSE);
	
	command = g_strdup_printf(
					"sh -c \"sh -c 'scp %s/%s %s@%s:%s/%s' >%s 2>%s\"",
					localdir,filename,username,hostname,remotedir,filename,
					fout,ferr);
	unlink(fout);
	unlink(ferr);
	if(system(command)==0)
	{
		g_free(command);
		hide_progress_connection();
		progress_connection(0," ",TRUE);
	}
	else
	{
		g_free(command);
		return 1;
	}

	return 0;
}
/********************************************************/
int scp_get_file(char* fout,char* ferr,
			 char* filename,char* localdir,char* remotedir,
			 char *hostname,char *username,char* password)
{
	gchar *command;
	gchar* message;

	message = g_strdup_printf( _("Get \"%s/%s\" file from \"%s\" host..."),
					remotedir,filename,hostname
				);

	command = g_strdup_printf(
					"sh -c \"sh -c 'scp  %s@%s:%s/%s %s/%s' \"",
					username,hostname,
					remotedir,filename,
					localdir,"."
					);
	unlink(fout);
	unlink(ferr);

	if(run_scp(ferr,fout,command,message)==0)
		g_free(command);
	else
	{
		g_free(command);
		return 1;
	}
	return 0;
}
#endif /* G_SO_WIN32 */
/*********************************************************/

