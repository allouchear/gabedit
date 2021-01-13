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
#include "../NetWork/Rsh.h"
#include "../NetWork/Ssh.h"

void remote_command (char *fout,char *ferr,const char* cmd, const char *hostname,const char* username,const char *password)
{
  if(fileopen.netWorkProtocol == GABEDIT_NETWORK_FTP_RSH)
  {
	  rsh(fout,ferr,cmd,hostname,username,password);
  }
  else
  {
	  ssh(fout,ferr,cmd,hostname,username,password);
  }
}
