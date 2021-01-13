/* Ssh.c */
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
*          plink - Remote Shell Client for Windows     system   *
*          ssh - Remote Shell Client using ssh system           *
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

static gchar*	cmdGlobal = NULL;
static gchar*	foutGlobal = NULL;
static gchar*	ferrGlobal = NULL;
static gchar*	userNameGlobal = NULL;
static gchar*	rhostGlobal = NULL;
static gchar*	passWordGlobal = NULL;

static long clientThread ()
{
	gchar *command;
#ifndef G_OS_WIN32
	FILE* f = stdout;
#endif
	/*
	FILE* FileOut =freopen(foutGlobal, "w", stdout);

    */
	FILE* FileErr =freopen(ferrGlobal, "w", stderr);
	gchar* fileouttmp = 
		g_strdup_printf("%s%stmp%sfouttmp",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	


	command = g_strdup_printf(
					"%s -pw %s %s@%s %s > \"%s\"",
					plinkCommand,passWordGlobal,
					userNameGlobal,rhostGlobal,cmdGlobal
					,fileouttmp);

	/* printf("%s\n",command);*/
	system(command);
	g_free(command);

	command = g_strdup_printf("copy \"%s\" \"%s\"",fileouttmp,foutGlobal);
	system(command);
	g_free(command);
	g_free(fileouttmp);

	/*
	fclose(FileOut);
	*/
	fclose(FileErr);
	
	
	
	return 0;
}

/********************************************************
* ssh : main processing routine; connect to server,		*
* pass command line and wait for results				*
*********************************************************/
void ssh (char *fout,char *ferr,const char* cmd, 
		  const char *rhost,const char* userName,
		  const char* password 
		  )
{
	HANDLE threadHnd;
	DWORD threadID;
	int i;

	cmdGlobal = g_strdup(cmd);
	foutGlobal = g_strdup(fout);
	ferrGlobal = g_strdup(ferr);
	userNameGlobal = g_strdup(userName);
	rhostGlobal = g_strdup(rhost);
	cmdGlobal = g_strdup(cmd);
	passWordGlobal = g_strdup(password);

	if(strstr(cmdGlobal,"&"))
	for(i=0;i<(int)strlen(cmdGlobal);i++)
	{
		if(cmdGlobal[i]=='&')
			cmdGlobal[i] = ' ';
	}
	/*
	printf("cmd = %s\n",cmdGlobal);
	printf("fout = %s\n",foutGlobal);
	printf("ferr = %s\n",ferrGlobal);
	*/
	
	unlink(fout);
	unlink(ferr);

	threadHnd=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)clientThread,
			(LPVOID)0, 0, (LPDWORD)&threadID);
	if(!threadHnd)
	{
			FILE* FileErr = FOpen(ferr,"w");
			fprintf(FileErr,_("Cannot start client thread...\n"));
			fclose(FileErr);
			goto end;
	}
	/*printf("End start client thread cmd = %s\n",cmd);*/

	if(!strstr(cmd,"&"))
	if(threadHnd)
	{
		DWORD exitCode=0;
		GetExitCodeThread(threadHnd, &exitCode);
		while(exitCode==STILL_ACTIVE)
		{
			Sleep(50);
			GetExitCodeThread(threadHnd, &exitCode);
		}
		CloseHandle(threadHnd);
	}
	if(strstr(cmd,"&") && threadHnd)
		Sleep(500);

	WSACleanup();

end :
	return;
}
#else /* G_SO_WIN32 */
/********************************************************
* rsh : main processing routine; connect to server,	*
* pass command line and wait for results				*
*********************************************************/
#include <stdlib.h>
#include <unistd.h>

void ssh (char *fout,char *ferr,const char* cmd, 
		  const char *rhost,const char* userName,
		  const char* password 
		  )
{
	gchar *command;

	command = g_strdup_printf(
					"sh -c \"sh -c 'ssh -l%s %s %s' >%s 2>%s\"",
					userName,rhost,cmd,
					fout,ferr);

	unlink(fout);
	unlink(ferr);
	{int ierr = system(command);}
	g_free(command);
}
#endif /* G_SO_WIN32 */
/*********************************************************/

