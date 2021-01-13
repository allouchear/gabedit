/*FilesTransfert.c */
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
#include "../NetWork/Ftp.h"
#include "../NetWork/Scp.h"

/*********************************************************************/
int get_file(char* fout,char* ferr,char* filename,char* localdir,char* remotedir,
			 char *hostname,char *username,char* password)
{
  if(fileopen.netWorkProtocol == GABEDIT_NETWORK_FTP_RSH)
  {
	  return ftp_get_file(fout,ferr,filename,localdir,remotedir,hostname,username,password);
  }
  else
  {
	  return scp_get_file(fout,ferr,filename,localdir,remotedir,hostname,username,password);
  }
}
/******************************************************/
int put_file(char* fout,char* ferr,char* filename,char* localdir,char* remotedir,
			 char *hostname,char *username,char* password)
{
  if(fileopen.netWorkProtocol == GABEDIT_NETWORK_FTP_RSH)
  {
	  return ftp_put_file(fout,ferr,filename,localdir,remotedir,hostname,username,password);
  }
  else
  {
	  return scp_put_file(fout,ferr,filename,localdir,remotedir,hostname,username,password);
  }
}
