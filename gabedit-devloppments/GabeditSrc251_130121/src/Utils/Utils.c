/* Utils.c */
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


#include "../../Config.h"
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/Vector3d.h"
#include "../Geometry/GeomGlobal.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Geometry/ResultsAnalise.h"
#include "../Geometry/EnergiesCurves.h"
#include "../Common/TextEdit.h"
#include "../Common/Preferences.h"
#include "../Common/Run.h"
#include "../Display/GLArea.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/AxesGeomGL.h"
#include "../Utils/HydrogenBond.h"
#ifdef G_OS_WIN32
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>
#include <fcntl.h>
#include <io.h>
#else /* G_OS_WIN32 */
#include <stdarg.h> 
#include <pwd.h>
#include <unistd.h> 
#include <sys/times.h>
#endif /* G_OS_WIN32 */

#define DebugFlag 0
#define Debug1Flag 0

void create_color_surfaces_file();
void read_color_surfaces_file();
void initAxis();
void save_axis_properties();
void read_axis_properties();
void save_principal_axis_properties();
void read_principal_axis_properties();
void initPrincipalAxisGL();


#define BBSIZE 10240
/********************************************************************************/
#ifndef G_OS_WIN32
#define TIMER_TICK      60
static clock_t it;
static struct tms itt;
void timing(double* cpu,double *sys)
{
	it=times(&itt);
	*cpu=(double) itt.tms_utime / (double) TIMER_TICK;
	*sys=(double) itt.tms_stime / (double) TIMER_TICK;
}
#endif
#ifdef G_OS_WIN32
void addUnitDisk(FILE* file, G_CONST_RETURN gchar* name)
{
	if(name && strlen(name)>1 && name[1]==':')
		fprintf(file,"%c%c\n", name[0],name[1]);
}
#endif
#ifdef G_OS_WIN32
/************************************************************************
*  error : display an error message and possibly the last Winsock error *
*************************************************************************/
gboolean winsockCheck(FILE* FileErr)
{
    	WORD wVersionRequested;
    	WSADATA wsaData;
    	int err;
    	wVersionRequested = MAKEWORD( 1, 1 );
    	err = WSAStartup( wVersionRequested, &wsaData );
    	if ( err != 0 )
	{
		fprintf(FileErr,"Unsupported version of winsock.dll!\n");
		return FALSE;
	}

    	if ( LOBYTE( wsaData.wVersion ) != 1 || HIBYTE( wsaData.wVersion ) != 1 )
	{
        	fprintf(FileErr,"Unsupported version of winsock.dll!\n");
		return FALSE;
	}

	return TRUE;
}
#endif /* G_OS_WIN32 */
/********************************************************************************/
FILE* FOpen(const gchar *fileutf8, const gchar* type)
{
	FILE* file;
#ifdef G_OS_WIN32
		/* gchar* filename = g_filename_to_utf8(fileutf8);*/
		gchar* filename = g_strdup(fileutf8);
		file = fopen(filename,type);
		if(file) set_last_directory(fileutf8);
		return file;
#else
		file = fopen(fileutf8,type);
		if(file) set_last_directory(fileutf8);
		return file;
#endif
}   
/********************************************************************************/
static void free_commands_list(CommandsList* list)
{
	gint i;
	if(!list)
		return;

	if(list->numberOfCommands<1)
	{
  		list->numberOfCommands = 0;
  		list->numberOfDefaultCommand = 0;
		list->commands = NULL;
		return;
	}
	if(list->commands)
	{
		for(i=0;i<list->numberOfCommands;i++)
			if(list->commands[i])
				g_free(list->commands[i]);
		g_free(list->commands);
	}
  	list->numberOfCommands = 0;
  	list->numberOfDefaultCommand = 0;
	list->commands = NULL;
}
/********************************************************************************/
void free_gamess_commands()
{
	free_commands_list(&gaussianCommands);
}
/********************************************************************************/
void free_gaussian_commands()
{
	free_commands_list(&gaussianCommands);
}
/********************************************************************************/
void free_molcas_commands()
{
	free_commands_list(&molcasCommands);
}
/********************************************************************************/
void free_molpro_commands()
{
	free_commands_list(&molproCommands);
}
/********************************************************************************/
void free_mpqc_commands()
{
	free_commands_list(&mpqcCommands);
}
/********************************************************************************/
void free_nwchem_commands()
{
	free_commands_list(&nwchemCommands);
}
/********************************************************************************/
void free_psicode_commands()
{
	free_commands_list(&psicodeCommands);
}
/********************************************************************************/
void free_orca_commands()
{
	free_commands_list(&orcaCommands);
}
/********************************************************************************/
void free_demon_commands()
{
	free_commands_list(&demonCommands);
}
/********************************************************************************/
void free_firefly_commands()
{
	free_commands_list(&fireflyCommands);
}
/********************************************************************************/
void free_qchem_commands()
{
	free_commands_list(&qchemCommands);
}
/********************************************************************************/
void free_mopac_commands()
{
	free_commands_list(&mopacCommands);
}
/********************************************************************************/
void free_povray_commands()
{
	free_commands_list(&povrayCommands);
}
/********************************************************************************/
gchar* get_time_str()
{
	gchar* str=NULL;
	time_t t;
	struct tm* ts;

	t = time(NULL);
	ts = localtime(&t);
	str = asctime (ts);
	return str;
}
/********************************************************************************/
gdouble get_multipole_rank()
{
  return multipole_rank;
}
/********************************************************************************/
gboolean this_is_a_backspace(gchar *st)
{
        gint i;
        for(i=0;i<(gint)strlen(st);i++)
        	if(st[i] != ' ' && st[i] !='\n' && st[i] !='\r')
                	return FALSE;
        return TRUE;
}   
/********************************************************************************/
void changeDInE(gchar *st)
{
        gint i;
	gint l = 0;
	if(!st) return;
	l = strlen(st);
        for(i=0;i<l;i++)
       		if(st[i] == 'D' || st[i] =='d') 
			st[i]='e';
}   
/********************************************************************************/
void  set_file_open(gchar* remotehost,gchar* remoteuser,gchar* remotedir, GabEditNetWork netWorkProtocol)
{
  gchar localhost[100];

#ifdef G_OS_WIN32
	winsockCheck(stderr);
#endif
  gethostname(localhost,100);

  if(!fileopen.localhost)
  	fileopen.localhost = g_strdup(localhost);

  if(!fileopen.localdir)
  	fileopen.localdir = get_name_dir(fileopen.projectname);

  if(fileopen.remotehost)
	g_free(fileopen.remotehost);
  if(fileopen.remoteuser)
	g_free(fileopen.remoteuser);
  if(fileopen.remotedir)
	g_free(fileopen.remotedir);
  fileopen.remotehost = g_strdup(remotehost);
  fileopen.remoteuser = g_strdup(remotehost);
  fileopen.remotedir = g_strdup(remotedir);
  fileopen.netWorkProtocol = netWorkProtocol;
}
/********************************************************************************/
void filegets(gchar *temp,FILE* fd)
{
	gchar t[BBSIZE];
        gint taille = BBSIZE;
 	gint k = 0;
 	gint i;
 
    	if(!feof(fd)) { char* e = fgets(t,taille,fd);}
	for(i=0;i<taille;i++)
 	{
  		if(t[i] =='\n')
			break;
  		if(t[i] != ' ' && t[i] !='\n')
  		{
   			temp[k] = t[i];
   			k++;
  		}
 	}
 	temp[k] = '\0';
}
/********************************************************************************/
gboolean  this_is_an_object(GtkObject *obj)
{  
	return GTK_IS_OBJECT(obj);
}
/********************************************************************************/
gboolean  add_dir_to_user(User* user, const gchar* dir)
{  
	gint i;
	if(user->ndirs == 0)
	{
		user->ndirs = 1;
		user->dirs = g_malloc(sizeof(gchar*));
		user->dirs[0] = g_strdup(dir);
	}
	else
	{
		for(i=0;i<user->ndirs;i++)
			if(strcmp(dir,user->dirs[i]) == 0)
				return FALSE;

		user->dirs = g_realloc(user->dirs,(user->ndirs+1)*sizeof(gchar*));
		for(i=user->ndirs;i>0;i--)
			user->dirs[i] = user->dirs[i-1];
		user->dirs[0] = g_strdup(dir);
		if(user->ndirs>=NHOSTMAX)
		{
/*			g_free(user->dirs[user->ndirs]);*/
			user->dirs = g_realloc(user->dirs,(user->ndirs)*sizeof(gchar*));
		}
		else
			(user->ndirs)++;
	}
	return TRUE;
}
/********************************************************************************/
gboolean add_user_to_host(Host* host,const gchar *username, const gchar* password, const gchar *dir)
{  
	gint i;
	if(host->nusers == 0)
	{
		host->nusers = 1;
		host->users = g_malloc(sizeof(User));
		host->users[0].username = g_strdup(username);
		if(password)
			host->users[0].password = g_strdup(password);
		else
			host->users[0].password = NULL;
		host->users[0].ndirs = 0;
  		add_dir_to_user(&host->users[0] ,dir);
	}
	else
	{
		for(i=0;i<host->nusers;i++)
			if(strcmp(username,host->users[i].username) == 0)
			{
				return  add_dir_to_user(&host->users[i],dir);
			}
	
		host->users = g_realloc(host->users,(host->nusers+1)*sizeof(User));
		for(i=host->nusers;i>0;i--)
			host->users[i] = host->users[i-1];

		host->users[0].ndirs = 0;
		host->users[0].username = g_strdup(username);
		if(password)
			host->users[0].password = g_strdup(password);
		else
			host->users[0].password = NULL;
  		add_dir_to_user(&host->users[0] ,dir);
		if(host->nusers>=NHOSTMAX)
		{
			for(i=0;i<host->users[host->nusers].ndirs;i++)
			{
				g_free(host->users[host->nusers].dirs[i]);
			}
			g_free(host->users[host->nusers].dirs);
/*			g_free(host->users[host->nusers]);*/
			host->users = g_realloc(host->users,(host->nusers)*sizeof(User));
		}
		else
			(host->nusers)++;
	}
	return TRUE;
}
/********************************************************************************/
void add_host(const gchar *hostname, const gchar* username, const gchar* password, const gchar* dir)
{  
  gint i;
  gint j;

  if(recenthosts.nhosts == 0)
  {
	recenthosts.hosts = g_malloc(sizeof(Host));
 	recenthosts.hosts[0].hostname = g_strdup(hostname);
 	recenthosts.hosts[0].nusers = 0;
 	add_user_to_host(&recenthosts.hosts[0],username,password,dir);
	recenthosts.nhosts = 1;
  }
  else
  {
	for(i=0;i<recenthosts.nhosts;i++)
		if(strcmp(hostname,recenthosts.hosts[i].hostname) == 0)
		{
 			add_user_to_host(&recenthosts.hosts[i],username,password,dir);
			return;
		}

	recenthosts.hosts = g_realloc(recenthosts.hosts , (recenthosts.nhosts+1)*sizeof(Host));
	for(i=recenthosts.nhosts;i>0;i--)
		recenthosts.hosts[i] = recenthosts.hosts[i-1];
 	recenthosts.hosts[0].hostname = g_strdup(hostname);
 	recenthosts.hosts[0].nusers = 0;
 	add_user_to_host(&recenthosts.hosts[0],username,password,dir);
	if(recenthosts.nhosts>=NHOSTMAX)
	{
		for(i=0;i<recenthosts.hosts[recenthosts.nhosts].nusers;i++)
		{
			for(j=0;j<recenthosts.hosts[recenthosts.nhosts].users[i].ndirs;j++)
				g_free(recenthosts.hosts[recenthosts.nhosts].users[i].dirs[j]);
			g_free(recenthosts.hosts[recenthosts.nhosts].users[i].dirs);
		}
		g_free(recenthosts.hosts[recenthosts.nhosts].users);
		recenthosts.hosts = g_realloc(recenthosts.hosts ,(recenthosts.nhosts)*sizeof(Host));
	}
	else
		recenthosts.nhosts++;
	
  }
}
/********************************************************************************/
G_CONST_RETURN gchar *get_local_user()
{  
	

#ifdef G_OS_WIN32
	return NULL;
#else
  	struct passwd *pw;
  	const static gchar* localuser = NULL;

	if(localuser) return localuser;

  	localuser = g_getenv("USER");
  	if( (localuser == NULL) || ((pw = getpwnam(localuser)) && (pw->pw_uid != getuid())) )
  	{
		if ( (pw = getpwuid(getuid())))
			localuser = g_strdup(pw->pw_name);
		else
			localuser = NULL;
  	}
  return localuser;
#endif
}
/*************************************************************************************/
void Waiting(gdouble tsecond)
{
        GTimer *timer;
        gdouble elaps;
        gulong m ;
	
        timer =g_timer_new( );
	g_timer_start( timer );
	g_timer_reset( timer );
        do{
		elaps = g_timer_elapsed( timer,&m);
        }while(elaps<tsecond);
 	g_timer_destroy(timer);
}
/*************************************************************************************/
void Debug(char *fmt,...)
{
	va_list ap;
        if(DebugFlag)
		return;

	va_start(ap,fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
}
/********************************************************************************/
gchar* get_line_chars(gchar c,gint n)
{
	gint i;
	gchar *line = NULL;

	if(n<1)
		return line;
	line = g_malloc((n+1)*sizeof(gchar));
	for(i=0;i<n;i++)
		line[i] = c;
	line[n] = '\0';

	return line;
	
}
/********************************************************************************/
gchar* cat_file(gchar* namefile,gboolean tabulation)
{
 gchar *t = NULL;
 gchar *tsrt = NULL;
 FILE *fd;
 gchar *dump = NULL;


 t=g_malloc(BBSIZE*sizeof(gchar));

 fd = FOpen(namefile, "rb");
 if(fd)
 {
  	while(!feof(fd))
  	{
    		if(!fgets(t,BBSIZE, fd)) break;
                dump = tsrt;
		if(!tsrt)
		{
			if(tabulation)
				tsrt = g_strdup_printf("\t%s",t);
			else
				tsrt = g_strdup_printf("%s",t);
		}
		else
		{
			if(tabulation)
				tsrt = g_strdup_printf("%s\t%s",tsrt,t);
			else
				tsrt = g_strdup_printf("%s%s",tsrt,t);
			g_free(dump);
			dump = NULL;
		}
  	}
 	fclose(fd);
	unlink (namefile);
 }
 else
 {
   tsrt = NULL;
 }
 g_free(t);
 t = tsrt;
 if(t)
 {
 	tsrt = g_locale_to_utf8(t,-1,NULL,NULL,NULL);
	g_free(t);
 }

 return tsrt;
  
}
/*************************************************************************************/
gchar *run_command(gchar *command)
{
 gchar *t;
 gchar *terr = NULL;
 FILE *fd;
 gchar *temp;
 gchar *outfile= g_strdup_printf("%s%stmp%soutfile",gabedit_directory(), G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S);
 gchar *errfile= g_strdup_printf("%s%stmp%serrfile",gabedit_directory(), G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S);
 gchar *dump;
 gint taille = BBSIZE;

 temp = g_strdup_printf("sh -c '%s >%s 2>%s'",command,outfile,errfile);
 {int it = system(temp);}

 t=g_malloc(taille);

 fd = FOpen(errfile, "rb");
 if(fd)
 {
  	while(!feof(fd))
  	{
    		if(!fgets(t,taille, fd))
			break;
                dump = terr;
		if(!terr)
			terr = g_strdup_printf("%s",t);
		else
		{
			terr = g_strdup_printf("%s%s",terr,t);
			g_free(dump);
		}
  	}
 	fclose(fd);
	unlink (errfile);
 }
 else
   terr = NULL;

 fd = FOpen(outfile, "rb");
 if(fd)
 {
	unlink (outfile);
 }

 g_free(t);
 g_free(temp);
 g_free(outfile);
 g_free(errfile);

 return terr;
}
/********************************************************************************/
#ifdef G_OS_WIN32

void createProcessWin32(char* myChildProcess)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	printf(_("Command = %s\n"),myChildProcess);
	/* Start the child process. */
	if( !CreateProcess( NULL,   /* No module name (use command line). */
		TEXT(myChildProcess), /* Command line. */
	                     NULL,      /* Process handle not inheritable. */
	                     NULL,      /* Thread handle not inheritable. */
	                    FALSE,      /* Set handle inheritance to FALSE. */
	                    0,          /* No creation flags. */
	                   NULL,        /* Use parent's environment block. */
	                   NULL,        /* Use parent's starting directory. */
	                   &si,         /* Pointer to STARTUPINFO structure.*/
	                  &pi )         /* Pointer to PROCESS_INFORMATION structure.*/
                        ) 
	{
		gchar buffer[BBSIZE];
		sprintf(buffer,_("CreateProcess failed (%d)"),(int)GetLastError());
        	Message(buffer, _("Error"), TRUE);
		return;
    }

        /* Wait until child process exits.*/
        /* WaitForSingleObject( pi.hProcess, INFINITE );*/

        /* Close process and thread handles. */
       CloseHandle( pi.hProcess );
       CloseHandle( pi.hThread );
}

#endif
/********************************************************************************/
void run_local_command(gchar *outfile,gchar *errfile,gchar* command,gboolean under)
{
	gchar *temp;
	gint ierr = 0;

	unlink (outfile);
	unlink (errfile);

	if(under)
	{
#ifdef G_OS_WIN32
		temp = g_strdup_printf("%s >%s 2>%s ",command, outfile, errfile);
		createProcessWin32(temp);

#else
		if(strstr(command,">"))
 		temp = g_strdup_printf("sh -c '%s 2>%s&'",command, errfile);
		else
 		temp = g_strdup_printf("sh -c '%s >%s 2>%s&'",command, outfile, errfile);
		ierr = system(temp);
#endif
 		
		Waiting(0.5);
	}
	else
	{
#ifdef G_OS_WIN32
		temp = g_strdup_printf("%s >%s 2>%s",command,outfile,errfile);
#else
 		temp = g_strdup_printf("sh -c '%s >%s 2>%s'",command,outfile,errfile);
#endif
 		ierr = system(temp);
	}

	g_free(temp);
}
/*************************************************************************************/
const gchar *gabedit_directory(void)
{
  static gchar *gabedit_dir = NULL;
  gchar *home_dir;
  gchar *home_dir_sep;
#ifdef G_OS_WIN32
  gchar* Version_S = g_strdup_printf("%d%d%d",MAJOR_VERSION,MINOR_VERSION,MICRO_VERSION);
#else
  gchar* Version_S = g_strdup_printf("%d.%d.%d",MAJOR_VERSION,MINOR_VERSION,MICRO_VERSION);
#endif

  if (gabedit_dir != NULL)
    return gabedit_dir;

  home_dir = g_strdup(g_get_home_dir());

  if (home_dir != NULL && home_dir[strlen(home_dir)-1] != G_DIR_SEPARATOR)
    home_dir_sep = G_DIR_SEPARATOR_S;
  else
    home_dir_sep = "";

  if (!home_dir)
  {
#ifdef G_OS_WIN32
		home_dir = g_strdup("C:");
#else  /* G_OS_WIN32 */
		home_dir = g_strdup("/tmp");
#endif /* G_OS_WIN32 */
		home_dir_sep = G_DIR_SEPARATOR_S;
  }
#ifdef G_OS_WIN32
  gabedit_dir = g_strconcat(home_dir,home_dir_sep,"gabedit",Version_S,NULL);
#else
  gabedit_dir = g_strconcat(home_dir,home_dir_sep,".gabedit-",Version_S,NULL);
#endif

  g_free(Version_S);
  return gabedit_dir;
}
/*************************************************************************************/
void DeleteLastChar(gchar *str)
{
        str[strlen(str)-1]='\0';
}
/*************************************************************************************/
gchar *get_dir_file_name(G_CONST_RETURN gchar* dirname, G_CONST_RETURN gchar* filename)
{
   gchar *name = NULL;

   name = g_strdup_printf("%s%s%s",dirname,G_DIR_SEPARATOR_S,filename); 
   
   return name;
}
/*************************************************************************************/
gchar *get_name_dir(const gchar* allname)
{
   gchar *name;
   name = g_path_get_dirname(allname);
   if(strcmp(name,".")==0) 
   {
	   g_free(name);
	   name = g_strdup(g_get_current_dir());
   }
   
  return name;
}
/*************************************************************************************/
gchar *get_filename_without_ext(const gchar* allname)
{
   gchar *filename= NULL;
   gchar *temp= NULL;
   gint len=0;
   gint i;
   gchar* name = NULL;

   if(!allname || strlen(allname)<1) return g_strdup("error");
   temp = g_strdup(allname);
   filename= g_strdup(allname);
   len=strlen(filename);

   for(i=len;i>0;i--)
	if(temp[i]=='.')
	{
		temp[i] = '\0';
		break;
	}
   name = g_strdup_printf("%s",temp);
   if(temp) g_free(temp);
   if(filename) g_free(filename);
  return name;
}
/*************************************************************************************/
gchar *get_suffix_name_file(const gchar* allname)
{
   gchar *filename= g_path_get_basename(allname);
   gchar *dirname= g_path_get_dirname(allname);
   gchar *temp= g_strdup(filename);
   gint len=strlen(filename);
   gint i;
   gchar* name = NULL;

   if(!allname || strlen(allname)<1) return g_strdup("error");
   filename= g_path_get_basename(allname);
   dirname= g_path_get_dirname(allname);
   temp= g_strdup(filename);
   len=strlen(filename);

   for(i=len;i>0;i--)
	if(temp[i]=='.')
	{
		temp[i] = '\0';
		break;
	}
   name = g_strdup_printf("%s%s%s",dirname,G_DIR_SEPARATOR_S,temp);
   if(temp) g_free(temp);
   if(dirname) g_free(dirname);
   if(filename) g_free(filename);

   if(strcmp(name,".")==0) name = g_strdup(g_get_current_dir());
   
  return name;
}
/*************************************************************************************/
gchar *get_name_file(const gchar* allname)
{
   gchar *name= g_path_get_basename(allname);
	/*
   gchar *name=g_strdup(allname);
   gint i;
   gint len=strlen(allname);
   gint islash=0;

   for(i=len;i>0;i--)
   if(allname[i]==G_DIR_SEPARATOR)
   {
     islash=i+1;
     break;
   }
   if(islash>0)
   {
	if(name)
		g_free(name);
   	name=g_malloc(len-islash+2);
   	for(i=islash;i<=len;i++)
    		name[i-islash]=allname[i];
   	name[len-islash+1]='\0';
   }
   */
   
  return name;
}
/*************************************************************************************/
Point get_produit_vectoriel(Point V1,Point V2)
{
   Point PV;

   PV.C[0]= V1.C[1]*V2.C[2]-V1.C[2]*V2.C[1];
   PV.C[1]= V1.C[2]*V2.C[0]-V1.C[0]*V2.C[2];
   PV.C[2]= V1.C[0]*V2.C[1]-V1.C[1]*V2.C[0];

  return PV;
} 
/*************************************************************************************/
gchar *get_distance_points(Point P1,Point P2,gboolean f3)
{
   gchar *distance;
   gdouble Distance;
   guint i;

   Distance = 0.0;
   for(i=0;i<3;i++)
 	Distance += (P1.C[i]- P2.C[i])*(P1.C[i]- P2.C[i]);
	
   Distance = sqrt(Distance)*BOHR_TO_ANG;
   if(f3)
   	distance = g_strdup_printf("%7.3lf",Distance);
   else
   	distance = g_strdup_printf("%0.20lf",Distance);
  return distance;
} 
/*************************************************************************************/
gdouble get_module(Point V)
{
   gdouble Module;
   guint i;

   Module = 0.0;
   for(i=0;i<3;i++)
 	Module += V.C[i]*V.C[i];

  return sqrt(Module);
	
}
/*************************************************************************************/
gdouble get_scalaire(Point V1,Point V2)
{
   gdouble Scalaire;
   guint i;

   Scalaire = 0.0;
   for(i=0;i<3;i++)
 	Scalaire += V1.C[i]*V2.C[i];

  return Scalaire;
	
}
/*************************************************************************************/
gchar *get_angle_vectors(Point V1,Point V2)
{
   gchar *angle;
   gdouble Angle;
   gdouble modv1v2 = get_module(V1)*get_module(V2);
 
 
   if(fabs(modv1v2)>1e-14 )
   {
        Angle = get_scalaire(V1,V2)/modv1v2;
/*	Debug("Pscal = %f\n",Angle);*/
	if(Angle<=-1)
        	return g_strdup("180.0");
	if(Angle>=1)
        	return g_strdup_printf("0.0");

        Angle = acos(Angle)/DEG_TO_RAD;
/*	Debug("Angle = %f\n",Angle);*/
        angle = g_strdup_printf("%0.20lf",Angle);
   }
   else
        angle = g_strdup_printf("ERROR");   

  return angle;
} 
/*************************************************************************************/
void add_fonts_in_file(FILE *fd,FontsStyle fontsstyle)
{

 if(fontsstyle.fontname[strlen(fontsstyle.fontname)-1] !='\n')
 	fprintf(fd,"%s\n",fontsstyle.fontname);
 else
 	fprintf(fd,"%s",fontsstyle.fontname);

 fprintf(fd,"%d\n",fontsstyle.BaseColor.red);
 fprintf(fd,"%d\n",fontsstyle.BaseColor.green);
 fprintf(fd,"%d\n",fontsstyle.BaseColor.blue);

 fprintf(fd,"%d\n",fontsstyle.TextColor.red);
 fprintf(fd,"%d\n",fontsstyle.TextColor.green);
 fprintf(fd,"%d\n",fontsstyle.TextColor.blue);
}
/*************************************************************************************/
void create_hosts_file()
{
 gchar *hostsfile;
 FILE *fd;
 gint i;
 gint j;
 gint k;

 hostsfile = g_strdup_printf("%s%shosts",gabedit_directory(),G_DIR_SEPARATOR_S);

 fd = FOpen(hostsfile, "w");
 if(fd)
 {
	fprintf(fd,"%d\n",recenthosts.nhosts);
	for(i=0;i<recenthosts.nhosts;i++)
	{
		fprintf(fd,"%s\n",recenthosts.hosts[i].hostname);
		fprintf(fd,"%d\n",recenthosts.hosts[i].nusers);
		for(j=0;j<recenthosts.hosts[i].nusers;j++)
		{
			fprintf(fd,"%s\n",recenthosts.hosts[i].users[j].username);
			fprintf(fd,"%d\n",recenthosts.hosts[i].users[j].ndirs);
			for(k=0;k<recenthosts.hosts[i].users[j].ndirs;k++)
			{
				fprintf(fd,"%s\n",recenthosts.hosts[i].users[j].dirs[k]);
			}
		}
	}
 	fclose(fd);
 }

 g_free(hostsfile);
}
/*************************************************************************************/
void create_fonts_file()
{
 gchar *fontsfile;
 FILE *fd;

 fontsfile = g_strdup_printf("%s%sfonts",gabedit_directory(),G_DIR_SEPARATOR_S);

 fd = FOpen(fontsfile, "w");

 add_fonts_in_file(fd,FontsStyleData);
 add_fonts_in_file(fd,FontsStyleResult);
 add_fonts_in_file(fd,FontsStyleLabel);
 add_fonts_in_file(fd,FontsStyleOther);

 fclose(fd);
 g_free(fontsfile);
}
/*************************************************************************************/
static gboolean readOneDir(FILE* file, char* tag, char** pDirectory)
{
	gboolean ok=FALSE;
	gchar t[BSIZE];
	rewind(file);
	while(!feof(file))
	{
 		if(!fgets(t,BSIZE,file)) break;
		if(strstr(t,tag)) { ok = TRUE; break;}
	}
	if(ok)
 	if(fgets(t,BSIZE,file))
	{
 		*pDirectory = g_strdup(t);
		str_delete_n(*pDirectory);
		delete_last_spaces(*pDirectory);
		delete_first_spaces(*pDirectory);
#ifdef G_OS_WIN32
		{
		gchar t[BBSIZE];
		sprintf(t,"%s;%s",*pDirectory,g_getenv("PATH"));
		if(strlen(t)>1) g_setenv("PATH",t,TRUE);
		}
#endif
 		if(!fgets(t,BSIZE,file)) ok=FALSE;
	}
	return ok;
}
/*************************************************************************************/
static gboolean readCommandsOneSoft(FILE* file, char* tag, char** pCommand, CommandsList* commandsList, void (*free_func)(), char* defCommand)
{
	int i;
	gboolean ok=FALSE;
	gchar t[BSIZE];
	rewind(file);
	while(!feof(file))
	{
 		if(!fgets(t,BSIZE,file)) break;
		if(strstr(t,tag)) { ok = TRUE; break;}
	}
	if(!ok) return ok;

 	if(fgets(t,BSIZE,file))
	{
 		*pCommand = g_strdup(t);
		str_delete_n(*pCommand);
		delete_last_spaces(*pCommand);
		delete_first_spaces(*pCommand);
	}
	if(commandsList==NULL) return ok;

 	if(fgets(t,BSIZE,file) && atoi(t)>0)
	{
		if(free_func) free_func();
		commandsList->numberOfCommands = atoi(t);
		commandsList->commands = g_malloc(commandsList->numberOfCommands*sizeof(gchar*));
		for(i=0;i<commandsList->numberOfCommands;i++) commandsList->commands[i]  = g_strdup(" ");
		for(i=0;i<commandsList->numberOfCommands;i++)
		{
			if(!fgets(t,BSIZE,file) || strstr(t,"End"))
			{
				free_func();
  				commandsList->numberOfCommands = 1;
  				commandsList->numberOfDefaultCommand = 0;
  				commandsList->commands = g_malloc(sizeof(gchar*));
  				commandsList->commands[0] = g_strdup(defCommand);
				return FALSE;
			}
			else
			{
				commandsList->commands[i] = g_strdup(t); 
				str_delete_n(commandsList->commands[i]);
				delete_last_spaces(commandsList->commands[i]);
				delete_first_spaces(commandsList->commands[i]);
			}
		}
	}
 	if(!fgets(t,BSIZE,file)) ok=FALSE; /* End of tag */
	return ok;
}
/*************************************************************************************/
void create_commands_file()
{
	gchar *commandsfile;
	FILE *fd;
	gint i;

	commandsfile = g_strdup_printf("%s%scommands",gabedit_directory(),G_DIR_SEPARATOR_S);

	fd = FOpen(commandsfile, "w");

	fprintf(fd,"Begin Batch\n");

	str_delete_n(NameTypeBatch);
 	fprintf(fd,"%s\n",NameTypeBatch);

	str_delete_n(NameCommandBatchAll);
 	fprintf(fd,"%s\n",NameCommandBatchAll);

	str_delete_n(NameCommandBatchUser);
 	fprintf(fd,"%s\n",NameCommandBatchUser);

	str_delete_n(NameCommandBatchKill);
 	fprintf(fd,"%s\n",NameCommandBatchKill);


	str_delete_n(NamejobIdTitleBatch);
 	fprintf(fd,"%s\n",NamejobIdTitleBatch);

 	fprintf(fd,"%d\n",batchCommands.numberOfTypes);
	for(i=0;i<batchCommands.numberOfTypes;i++)
	{
		str_delete_n(batchCommands.types[i]);
		fprintf(fd,"%s\n",batchCommands.types[i]);
		str_delete_n(batchCommands.commandListAll[i]);
		fprintf(fd,"%s\n",batchCommands.commandListAll[i]);
		str_delete_n(batchCommands.commandListUser[i]);
		fprintf(fd,"%s\n",batchCommands.commandListUser[i]);
		str_delete_n(batchCommands.commandKill[i]);
		fprintf(fd,"%s\n",batchCommands.commandKill[i]);
		str_delete_n(batchCommands.jobIdTitle[i]);
		fprintf(fd,"%s\n",batchCommands.jobIdTitle[i]);
	}
	fprintf(fd,"End\n");
/*-----------------------------------------------------------------------------*/
	fprintf(fd,"Begin DeMon\n");
	str_delete_n(NameCommandDeMon);
	delete_last_spaces(NameCommandDeMon);
	delete_first_spaces(NameCommandDeMon);
 	fprintf(fd,"%s\n",NameCommandDeMon);
 	fprintf(fd,"%d\n",demonCommands.numberOfCommands);
	for(i=0;i<demonCommands.numberOfCommands;i++)
	{
		str_delete_n(demonCommands.commands[i]);
		delete_last_spaces(demonCommands.commands[i]);
		delete_first_spaces(demonCommands.commands[i]);
		fprintf(fd,"%s\n",demonCommands.commands[i]);
	}
	fprintf(fd,"End\n");
/*-----------------------------------------------------------------------------*/
	fprintf(fd,"Begin Gamess\n");
	str_delete_n(NameCommandGamess);
	delete_last_spaces(NameCommandGamess);
	delete_first_spaces(NameCommandGamess);
 	fprintf(fd,"%s\n",NameCommandGamess);
 	fprintf(fd,"%d\n",gamessCommands.numberOfCommands);
	for(i=0;i<gamessCommands.numberOfCommands;i++)
	{
		str_delete_n(gamessCommands.commands[i]);
		delete_last_spaces(gamessCommands.commands[i]);
		delete_first_spaces(gamessCommands.commands[i]);
		fprintf(fd,"%s\n",gamessCommands.commands[i]);
	}
	fprintf(fd,"End\n");

/*-----------------------------------------------------------------------------*/

	fprintf(fd,"Begin Gaussian\n");
	str_delete_n(NameCommandGaussian);
	delete_last_spaces(NameCommandGaussian);
	delete_first_spaces(NameCommandGaussian);
 	fprintf(fd,"%s\n",NameCommandGaussian);
 	fprintf(fd,"%d\n",gaussianCommands.numberOfCommands);
	for(i=0;i<gaussianCommands.numberOfCommands;i++)
	{
		str_delete_n(gaussianCommands.commands[i]);
		delete_last_spaces(gaussianCommands.commands[i]);
		delete_first_spaces(gaussianCommands.commands[i]);
		fprintf(fd,"%s\n",gaussianCommands.commands[i]);
	}
	fprintf(fd,"End\n");

/*-----------------------------------------------------------------------------*/

	fprintf(fd,"Begin Molcas\n");
	str_delete_n(NameCommandMolcas);
	delete_last_spaces(NameCommandMolcas);
	delete_first_spaces(NameCommandMolcas);
 	fprintf(fd,"%s\n",NameCommandMolcas);
 	fprintf(fd,"%d\n",molcasCommands.numberOfCommands);
	for(i=0;i<molcasCommands.numberOfCommands;i++)
	{
		str_delete_n(molcasCommands.commands[i]);
		delete_last_spaces(molcasCommands.commands[i]);
		delete_first_spaces(molcasCommands.commands[i]);
		fprintf(fd,"%s\n",molcasCommands.commands[i]);
	}
	fprintf(fd,"End\n");
/*-----------------------------------------------------------------------------*/

	fprintf(fd,"Begin Molpro\n");
	str_delete_n(NameCommandMolpro);
	delete_last_spaces(NameCommandMolpro);
	delete_first_spaces(NameCommandMolpro);
 	fprintf(fd,"%s\n",NameCommandMolpro);
 	fprintf(fd,"%d\n",molproCommands.numberOfCommands);
	for(i=0;i<molproCommands.numberOfCommands;i++)
	{
		str_delete_n(molproCommands.commands[i]);
		delete_last_spaces(molproCommands.commands[i]);
		delete_first_spaces(molproCommands.commands[i]);
		fprintf(fd,"%s\n",molproCommands.commands[i]);
	}
	fprintf(fd,"End\n");
/*-----------------------------------------------------------------------------*/

	fprintf(fd,"Begin MPQC\n");
	str_delete_n(NameCommandMPQC);
	delete_last_spaces(NameCommandMPQC);
	delete_first_spaces(NameCommandMPQC);
 	fprintf(fd,"%s\n",NameCommandMPQC);
 	fprintf(fd,"%d\n",mpqcCommands.numberOfCommands);
	for(i=0;i<mpqcCommands.numberOfCommands;i++)
	{
		str_delete_n(mpqcCommands.commands[i]);
		delete_last_spaces(mpqcCommands.commands[i]);
		delete_first_spaces(mpqcCommands.commands[i]);
		fprintf(fd,"%s\n",mpqcCommands.commands[i]);
	}
	fprintf(fd,"End\n");
/*-----------------------------------------------------------------------------*/
	fprintf(fd,"Begin NWChem\n");
	str_delete_n(NameCommandNWChem);
	delete_last_spaces(NameCommandNWChem);
	delete_first_spaces(NameCommandNWChem);
 	fprintf(fd,"%s\n",NameCommandNWChem);
 	fprintf(fd,"%d\n",nwchemCommands.numberOfCommands);
	for(i=0;i<nwchemCommands.numberOfCommands;i++)
	{
		str_delete_n(nwchemCommands.commands[i]);
		delete_last_spaces(nwchemCommands.commands[i]);
		delete_first_spaces(nwchemCommands.commands[i]);
		fprintf(fd,"%s\n",nwchemCommands.commands[i]);
	}
	fprintf(fd,"End\n");
/*-----------------------------------------------------------------------------*/
	fprintf(fd,"Begin Psicode\n");
	str_delete_n(NameCommandPsicode);
	delete_last_spaces(NameCommandPsicode);
	delete_first_spaces(NameCommandPsicode);
 	fprintf(fd,"%s\n",NameCommandPsicode);
 	fprintf(fd,"%d\n",psicodeCommands.numberOfCommands);
	for(i=0;i<psicodeCommands.numberOfCommands;i++)
	{
		str_delete_n(psicodeCommands.commands[i]);
		delete_last_spaces(psicodeCommands.commands[i]);
		delete_first_spaces(psicodeCommands.commands[i]);
		fprintf(fd,"%s\n",psicodeCommands.commands[i]);
	}
	fprintf(fd,"End\n");
/*-----------------------------------------------------------------------------*/
	fprintf(fd,"Begin Orca\n");
	str_delete_n(NameCommandOrca);
	delete_last_spaces(NameCommandOrca);
	delete_first_spaces(NameCommandOrca);
 	fprintf(fd,"%s\n",NameCommandOrca);
 	fprintf(fd,"%d\n",orcaCommands.numberOfCommands);
	for(i=0;i<orcaCommands.numberOfCommands;i++)
	{
		str_delete_n(orcaCommands.commands[i]);
		delete_last_spaces(orcaCommands.commands[i]);
		delete_first_spaces(orcaCommands.commands[i]);
		fprintf(fd,"%s\n",orcaCommands.commands[i]);
	}
	fprintf(fd,"End\n");
/*-----------------------------------------------------------------------------*/
	fprintf(fd,"Begin FireFly\n");
	str_delete_n(NameCommandFireFly);
	delete_last_spaces(NameCommandFireFly);
	delete_first_spaces(NameCommandFireFly);
 	fprintf(fd,"%s\n",NameCommandFireFly);
 	fprintf(fd,"%d\n",fireflyCommands.numberOfCommands);
	for(i=0;i<fireflyCommands.numberOfCommands;i++)
	{
		str_delete_n(fireflyCommands.commands[i]);
		delete_last_spaces(fireflyCommands.commands[i]);
		delete_first_spaces(fireflyCommands.commands[i]);
		fprintf(fd,"%s\n",fireflyCommands.commands[i]);
	}
	fprintf(fd,"End\n");
/*-----------------------------------------------------------------------------*/
	fprintf(fd,"Begin QChem\n");
	str_delete_n(NameCommandQChem);
	delete_last_spaces(NameCommandQChem);
	delete_first_spaces(NameCommandQChem);
 	fprintf(fd,"%s\n",NameCommandQChem);
 	fprintf(fd,"%d\n",qchemCommands.numberOfCommands);
	for(i=0;i<qchemCommands.numberOfCommands;i++)
	{
		str_delete_n(qchemCommands.commands[i]);
		delete_last_spaces(qchemCommands.commands[i]);
		delete_first_spaces(qchemCommands.commands[i]);
		fprintf(fd,"%s\n",qchemCommands.commands[i]);
	}
	fprintf(fd,"End\n");
/*-----------------------------------------------------------------------------*/
	fprintf(fd,"Begin Mopac\n");
	str_delete_n(NameCommandMopac);
	delete_last_spaces(NameCommandMopac);
	delete_first_spaces(NameCommandMopac);
 	fprintf(fd,"%s\n",NameCommandMopac);
 	fprintf(fd,"%d\n",mopacCommands.numberOfCommands);
	for(i=0;i<mopacCommands.numberOfCommands;i++)
	{
		str_delete_n(mopacCommands.commands[i]);
		delete_last_spaces(mopacCommands.commands[i]);
		delete_first_spaces(mopacCommands.commands[i]);
		fprintf(fd,"%s\n",mopacCommands.commands[i]);
	}
	fprintf(fd,"End\n");
/*-----------------------------------------------------------------------------*/
	fprintf(fd,"Begin PovRay\n");
	str_delete_n(NameCommandPovray);
	delete_last_spaces(NameCommandPovray);
	delete_first_spaces(NameCommandPovray);
 	fprintf(fd,"%s\n",NameCommandPovray);
 	fprintf(fd,"%d\n",povrayCommands.numberOfCommands);
	for(i=0;i<povrayCommands.numberOfCommands;i++)
	{
		str_delete_n(povrayCommands.commands[i]);
		delete_last_spaces(povrayCommands.commands[i]);
		delete_first_spaces(povrayCommands.commands[i]);
		fprintf(fd,"%s\n",povrayCommands.commands[i]);
	}
	fprintf(fd,"End\n");
/*-----------------------------------------------------------------------------*/
	fprintf(fd,"Begin Babel\n");
	str_delete_n(babelCommand);
	delete_last_spaces(babelCommand);
	delete_first_spaces(babelCommand);
	fprintf(fd,"%s\n",babelCommand);
	fprintf(fd,"End\n");

	fprintf(fd,"Begin DemonDir\n");
	str_delete_n(demonDirectory);
	delete_last_spaces(demonDirectory);
	delete_first_spaces(demonDirectory);
	fprintf(fd,"%s\n",demonDirectory);
	fprintf(fd,"End\n");

	fprintf(fd,"Begin GamessDir\n");
	str_delete_n(gamessDirectory);
	delete_last_spaces(gamessDirectory);
	delete_first_spaces(gamessDirectory);
	fprintf(fd,"%s\n",gamessDirectory);
	fprintf(fd,"End\n");

	fprintf(fd,"Begin NWChemDir\n");
	str_delete_n(nwchemDirectory);
	delete_last_spaces(nwchemDirectory);
	delete_first_spaces(nwchemDirectory);
	fprintf(fd,"%s\n",nwchemDirectory);
	fprintf(fd,"End\n");

	fprintf(fd,"Begin PsicodeDir\n");
	str_delete_n(psicodeDirectory);
	delete_last_spaces(psicodeDirectory);
	delete_first_spaces(psicodeDirectory);
	fprintf(fd,"%s\n",psicodeDirectory);
	fprintf(fd,"End\n");

	fprintf(fd,"Begin OrcaDir\n");
	str_delete_n(orcaDirectory);
	delete_last_spaces(orcaDirectory);
	delete_first_spaces(orcaDirectory);
	fprintf(fd,"%s\n",orcaDirectory);
	fprintf(fd,"End\n");

	fprintf(fd,"Begin FireFlyDir\n");
	str_delete_n(fireflyDirectory);
	delete_last_spaces(fireflyDirectory);
	delete_first_spaces(fireflyDirectory);
	fprintf(fd,"%s\n",fireflyDirectory);
	fprintf(fd,"End\n");

	fprintf(fd,"Begin MopacDir\n");
	str_delete_n(mopacDirectory);
	delete_last_spaces(mopacDirectory);
	delete_first_spaces(mopacDirectory);
	fprintf(fd,"%s\n",mopacDirectory);
	fprintf(fd,"End\n");

	fprintf(fd,"Begin GaussDir\n");
	str_delete_n(gaussDirectory);
	delete_last_spaces(gaussDirectory);
	delete_first_spaces(gaussDirectory);
	fprintf(fd,"%s\n",gaussDirectory);
	fprintf(fd,"End\n");

	fprintf(fd,"Begin PovRayDir\n");
	str_delete_n(povrayDirectory);
	delete_last_spaces(povrayDirectory);
	delete_first_spaces(povrayDirectory);
	fprintf(fd,"%s\n",povrayDirectory);
	fprintf(fd,"End\n");

	fprintf(fd,"Begin OpenBabelDir\n");
	str_delete_n(openbabelDirectory);
	delete_last_spaces(openbabelDirectory);
	delete_first_spaces(openbabelDirectory);
	fprintf(fd,"%s\n",openbabelDirectory);
	fprintf(fd,"End\n");


	fclose(fd);

	g_free(commandsfile);
}
/*************************************************************************************/
void create_network_file()
{
 gchar *networkfile;
 FILE *fd;

 networkfile = g_strdup_printf("%s%snetwork",gabedit_directory(),G_DIR_SEPARATOR_S);

 fd = FOpen(networkfile, "w");

 if(defaultNetWorkProtocol == GABEDIT_NETWORK_FTP_RSH) fprintf(fd,"0\n");
 else fprintf(fd,"1\n");

 fprintf(fd,"%s\n",pscpplinkDirectory);
 fclose(fd);

 g_free(networkfile);
}
/*********************************************************************************************/
gdouble get_alpha_opacity()
{
	return alpha_opacity;
}
/*********************************************************************************************/
void set_alpha_opacity(gdouble a)
{
	alpha_opacity = a;
	if(alpha_opacity>1) alpha_opacity = 1;
	if(alpha_opacity<0) alpha_opacity = 0;
}
/*************************************************************************************/
void create_opengl_file()
{
	gchar *openglfile;
	FILE *fd;

	openglfile = g_strdup_printf("%s%sopengl",gabedit_directory(),G_DIR_SEPARATOR_S);

	fd = FOpen(openglfile, "w");
	if(fd !=NULL)
	{
		fprintf(fd,"%d\n",openGLOptions.activateText);
		fprintf(fd,"%d\n",openGLOptions.rgba);
		fprintf(fd,"%d\n",openGLOptions.doubleBuffer);
		fprintf(fd,"%d\n",openGLOptions.alphaSize);
		fprintf(fd,"%d\n",openGLOptions.depthSize);
		fprintf(fd,"%d\n",openGLOptions.numberOfSubdivisionsCylindre);
		fprintf(fd,"%d\n",openGLOptions.numberOfSubdivisionsSphere);
		fprintf(fd,"%d\n",getOptCol());
		fprintf(fd,"%lf %lf\n",getScaleBall(),getScaleStick());
		fprintf(fd,"%d\n",colorMapType);
		fprintf(fd,"%lf %lf %lf\n",colorMapColors[0][0], colorMapColors[0][1],colorMapColors[0][2]);
		fprintf(fd,"%lf %lf %lf\n",colorMapColors[1][0], colorMapColors[1][1],colorMapColors[1][2]);
		fprintf(fd,"%lf %lf %lf\n",colorMapColors[2][0], colorMapColors[2][1],colorMapColors[2][2]);
		fprintf(fd,"%d\n",getShowOneSurface());
		fprintf(fd,"%lf\n",get_alpha_opacity());
		fclose(fd);
	}
	g_free(openglfile);
}
/*************************************************************************************/
void read_opengl_file()
{
	gchar *openglfile;
	FILE *fd;
	gint optcol = 0;
	gboolean showOneSurface = TRUE;

	openglfile = g_strdup_printf("%s%sopengl",gabedit_directory(),G_DIR_SEPARATOR_S);

	fd = fopen(openglfile, "rb");
	openGLOptions.activateText = 1;
	openGLOptions.rgba = 1;
	openGLOptions.doubleBuffer = 1;
	openGLOptions.alphaSize = 0;
	openGLOptions.depthSize = 1;
	openGLOptions.numberOfSubdivisionsCylindre = 20; 
	openGLOptions.numberOfSubdivisionsSphere = 30; 
	colorMapType =1;
	colorMapColors[0][0] = 1;
	colorMapColors[0][1] = 1;
	colorMapColors[0][2] = 1;
	colorMapColors[1][0] = 1;
	colorMapColors[1][1] = 1;
	colorMapColors[1][2] = 1;
	colorMapColors[2][0] = 1;
	colorMapColors[2][1] = 1;
	colorMapColors[2][2] = 1;
	if(fd !=NULL)
	{
 		guint taille = BBSIZE;
 		gchar t[BBSIZE];
 		if(fgets(t,taille,fd))
			if(sscanf(t,"%d",&openGLOptions.activateText)!=1)
				openGLOptions.activateText = 1;
 		if(fgets(t,taille,fd))
			if(sscanf(t,"%d",&openGLOptions.rgba)!=1)
				openGLOptions.rgba = 1;
 		if(fgets(t,taille,fd))
			if(sscanf(t,"%d",&openGLOptions.doubleBuffer)!=1)
				openGLOptions.doubleBuffer = 1;
 		if(fgets(t,taille,fd))
			if(sscanf(t,"%d",&openGLOptions.alphaSize)!=1)
				openGLOptions.alphaSize = 1;
 		if(fgets(t,taille,fd))
			if(sscanf(t,"%d",&openGLOptions.depthSize)!=1)
				openGLOptions.depthSize = 1;
 		if(fgets(t,taille,fd))
			if(sscanf(t,"%d",&openGLOptions.numberOfSubdivisionsCylindre)!=1)
				openGLOptions.numberOfSubdivisionsCylindre = 10;
 		if(fgets(t,taille,fd))
			if(sscanf(t,"%d",&openGLOptions.numberOfSubdivisionsSphere)!=1)
				openGLOptions.numberOfSubdivisionsSphere = 10;
 		if(fgets(t,taille,fd))
			if(sscanf(t,"%d",&optcol)!=1) optcol = 0;
		setOptCol(optcol);
 		if(fgets(t,taille,fd))
		{
			gdouble b,s;
			if(sscanf(t,"%lf %lf",&b,&s)==2)
			{
				setScaleBall(b);
				setScaleStick(b);
			}
		}
 		if(fgets(t,taille,fd))
			if(sscanf(t,"%d",&colorMapType)!=1) colorMapType =1;
 		if(fgets(t,taille,fd))
		{
			if(sscanf(t,"%lf %lf %lf",&colorMapColors[0][0], &colorMapColors[0][1],&colorMapColors[0][2])!=3)
			{
				colorMapColors[0][0] = 1.0;
				colorMapColors[0][1] = 1.0;
				colorMapColors[0][2] = 1.0;
			}
		}
 		if(fgets(t,taille,fd))
		{
			if(sscanf(t,"%lf %lf %lf",&colorMapColors[1][0], &colorMapColors[1][1],&colorMapColors[1][2])!=3)
			{
				colorMapColors[1][0] = 1.0;
				colorMapColors[1][1] = 1.0;
				colorMapColors[1][2] = 1.0;
			}
		}
 		if(fgets(t,taille,fd))
		{
			if(sscanf(t,"%lf %lf %lf",&colorMapColors[2][0], &colorMapColors[2][1],&colorMapColors[2][2])!=3)
			{
				colorMapColors[2][0] = 1.0;
				colorMapColors[2][1] = 1.0;
				colorMapColors[2][2] = 1.0;
			}
		}
 		if(fgets(t,taille,fd))
			if(sscanf(t,"%d",&showOneSurface)!=1) showOneSurface = 0;
		setShowOneSurface(showOneSurface);
 		if(fgets(t,taille,fd))
		{
			gdouble alpha;
			if(sscanf(t,"%lf",&alpha)==1) set_alpha_opacity(alpha);
		}

		fclose(fd);
	}
	g_free(openglfile);
}
/*************************************************************************************/
void create_ressource_file()
{
	save_atoms_prop();
	create_commands_file();
	create_network_file();
	create_fonts_file();
	create_color_surfaces_file();
	create_opengl_file();
	save_axis_properties();
#ifdef DRAWGEOMGL
	save_axes_geom_properties();
#endif
	save_principal_axis_properties();
	save_HBonds_properties();
	create_drawmolecule_file();
}
/*************************************************************************************/
void read_hosts_file()
{
 gchar *hostsfile;
 FILE *fd;
 gint i;
 gint j;
 gint k;
 gchar t[BBSIZE];
 gint len = BBSIZE;

 hostsfile = g_strdup_printf("%s%shosts",gabedit_directory(),G_DIR_SEPARATOR_S);

 fd = FOpen(hostsfile, "rb");
 if(fd)
 {
    	if(!feof(fd)) { char* e = fgets(t,len,fd);}
	recenthosts.nhosts = atoi(t);
	recenthosts.hosts = g_malloc(recenthosts.nhosts*sizeof(Host));
	for(i=0;i<recenthosts.nhosts;i++)
	{
		filegets(t,fd);recenthosts.hosts[i].hostname = g_strdup(t);
    		if(!feof(fd)) { char* e = fgets(t,len,fd);}
		recenthosts.hosts[i].nusers = atoi(t);
		recenthosts.hosts[i].users = g_malloc(recenthosts.hosts[i].nusers*sizeof(User));
		for(j=0;j<recenthosts.hosts[i].nusers;j++)
		{
			filegets(t,fd);
				recenthosts.hosts[i].users[j].username = g_strdup(t);
				recenthosts.hosts[i].users[j].password = NULL;
    			if(!feof(fd)) { char* e = fgets(t,len,fd);}
			recenthosts.hosts[i].users[j].ndirs = atoi(t);
			recenthosts.hosts[i].users[j].dirs = g_malloc(recenthosts.hosts[i].users[j].ndirs*sizeof(gchar*));
			for(k=0;k<recenthosts.hosts[i].users[j].ndirs;k++)
			{
				filegets(t,fd);recenthosts.hosts[i].users[j].dirs[k] = g_strdup(t);
			}
		}
	}
 	fclose(fd);
 }

 g_free(hostsfile);
}
/*************************************************************************************/
void read_fonts_in_file(FILE *fd,FontsStyle* fontsstyle)
{
	guint taille = BBSIZE;
	gchar *t = NULL;
	gchar *temp = NULL;
	gint i;
	gint k;

	t = g_malloc0(taille*sizeof(gchar));
	temp = g_malloc0(taille*sizeof(gchar));
    	if(!feof(fd)) { char* e = fgets(t,taille,fd);}
 
	k = 0;
	for(i=0;i<(gint)taille;i++)
	{
		if(t[i] =='\n')
		break;
		temp[k++] = t[i];
	}
	temp[k] = '\0';
	g_strchug(temp);
	g_strchomp(temp);

	fontsstyle->fontname= g_strdup(temp);

    	if(!feof(fd)) { char* e = fgets(t,taille,fd);}
	fontsstyle->BaseColor.red =(gushort) atoi(t);
    	if(!feof(fd)) { char* e = fgets(t,taille,fd);}
	fontsstyle->BaseColor.green =(gushort)  atoi(t);
    	if(!feof(fd)) { char* e = fgets(t,taille,fd);}
	fontsstyle->BaseColor.blue = (gushort) atoi(t);
 
    	if(!feof(fd)) { char* e = fgets(t,taille,fd);}
	fontsstyle->TextColor.red = (gushort) atoi(t);
    	if(!feof(fd)) { char* e = fgets(t,taille,fd);}
	fontsstyle->TextColor.green = (gushort) atoi(t);
    	if(!feof(fd)) { char* e = fgets(t,taille,fd);}
	fontsstyle->TextColor.blue = (gushort) atoi(t);                                                                                          
	g_free(t);
	g_free(temp);
}
/*************************************************************************************/
void read_fonts_file()
{
 gchar *fontsfile;
 FILE *fd;

 fontsfile = g_strdup_printf("%s%sfonts",gabedit_directory(),G_DIR_SEPARATOR_S);

 fd = FOpen(fontsfile, "rb");
 if(fd !=NULL)
 {
 	read_fonts_in_file(fd,&FontsStyleData);
 	read_fonts_in_file(fd,&FontsStyleResult);
 	read_fonts_in_file(fd,&FontsStyleLabel);
 	read_fonts_in_file(fd,&FontsStyleOther);
 	fclose(fd);

  	set_font (text,FontsStyleData.fontname);
  	set_base_style(text,FontsStyleData.BaseColor.red ,FontsStyleData.BaseColor.green ,FontsStyleData.BaseColor.blue);
  	set_text_style(text,FontsStyleData.TextColor.red ,FontsStyleData.TextColor.green ,FontsStyleData.TextColor.blue);

  	set_font (textresult,FontsStyleResult.fontname);
  	set_base_style(textresult,FontsStyleResult.BaseColor.red ,FontsStyleResult.BaseColor.green ,FontsStyleResult.BaseColor.blue);
  	set_text_style(textresult,FontsStyleResult.TextColor.red ,FontsStyleResult.TextColor.green ,FontsStyleResult.TextColor.blue);

  	set_font (TextOutput,FontsStyleResult.fontname);
  	set_base_style(TextOutput,FontsStyleResult.BaseColor.red ,FontsStyleResult.BaseColor.green ,FontsStyleResult.BaseColor.blue);
  	set_text_style(TextOutput,FontsStyleResult.TextColor.red ,FontsStyleResult.TextColor.green ,FontsStyleResult.TextColor.blue);

  	set_font (TextError,FontsStyleResult.fontname);
  	set_base_style(TextError,FontsStyleResult.BaseColor.red ,FontsStyleResult.BaseColor.green ,FontsStyleResult.BaseColor.blue);
  	set_text_style(TextError,FontsStyleResult.TextColor.red ,FontsStyleResult.TextColor.green ,FontsStyleResult.TextColor.blue);

 }
 set_font_other (FontsStyleOther.fontname);

}
/********************************************************************************/
void free_batch_commands()
{
	gint i;
	for(i=0;i<batchCommands.numberOfTypes;i++)
	{
		if(batchCommands.types[i])
			g_free(batchCommands.types[i]);
		if(batchCommands.commandListAll[i])
			g_free(batchCommands.commandListAll[i]);
		if(batchCommands.commandListUser[i])
			g_free(batchCommands.commandListUser[i]);
		if(batchCommands.jobIdTitle[i])
			g_free(batchCommands.jobIdTitle[i]);

	}
	if(batchCommands.types)
		g_free(batchCommands.types);
	if(batchCommands.commandListAll)
		g_free(batchCommands.commandListAll);
	if(batchCommands.commandListUser)
		g_free(batchCommands.commandListUser);
	if(batchCommands.jobIdTitle)
		g_free(batchCommands.jobIdTitle);

	if(NameTypeBatch)
		g_free(NameTypeBatch);
	if(NameCommandBatchAll)
		g_free(NameCommandBatchAll);
	if(NameCommandBatchUser)
		g_free(NameCommandBatchUser);
	if(NamejobIdTitleBatch)
		g_free(NamejobIdTitleBatch);
}
/*************************************************************************************/
void read_commands_file()
{
 guint taille = BBSIZE;
 gchar t[BBSIZE];
 gchar *commandsfile;
 FILE *fd;
 gint i;
 gint k;
 gchar *tmp[4] = {NULL,NULL,NULL,NULL};

 commandsfile = g_strdup_printf("%s%scommands",gabedit_directory(),G_DIR_SEPARATOR_S);

 fd = FOpen(commandsfile, "rb");
 if(fd !=NULL)
 {

 	if(fgets(t,taille,fd))
	if(!strstr(t,"Begin Batch"))
	{
		fclose(fd);
		return;
	}
 	if(fgets(t,taille,fd))
	{
		free_batch_commands();

 		NameTypeBatch= g_strdup(t);
		str_delete_n(NameTypeBatch);

		for(k=0;k<4;k++)
		if(fgets(t,taille,fd))
		{
			tmp[k] = g_strdup(t);
			str_delete_n(tmp[k]);
		}
		else
		{
			fclose(fd);
			initialise_batch_commands();
			return;
		}

		NameCommandBatchAll  = tmp[0];
		NameCommandBatchUser = tmp[1];
		NameCommandBatchKill = tmp[2];
		NamejobIdTitleBatch  = tmp[3];
	}
	else
	{
		fclose(fd);
		initialise_batch_commands();
		return;
	}


 	if(fgets(t,taille,fd) && atoi(t)>0)
	{
		batchCommands.numberOfTypes = atoi(t);
		batchCommands.types = g_malloc(batchCommands.numberOfTypes*sizeof(gchar*));
		batchCommands.commandListAll = g_malloc(batchCommands.numberOfTypes*sizeof(gchar*));
		batchCommands.commandListUser = g_malloc(batchCommands.numberOfTypes*sizeof(gchar*));
		batchCommands.jobIdTitle = g_malloc(batchCommands.numberOfTypes*sizeof(gchar*));

		for(i=0;i<batchCommands.numberOfTypes;i++)
		{
			batchCommands.types[i] = g_strdup(" ");
			batchCommands.commandListAll[i] = g_strdup(" ");
			batchCommands.commandListUser[i] = g_strdup(" ");
			batchCommands.jobIdTitle[i] = g_strdup(" ");
		}
		for(i=0;i<batchCommands.numberOfTypes;i++)
		{
			if(!fgets(t,taille,fd) || strstr(t,"End"))
			{
				free_batch_commands();
				fclose(fd);
				initialise_batch_commands();
				return;
			}
			else
			{

 				batchCommands.types[i]= g_strdup(t);
				str_delete_n(batchCommands.types[i]);
				
				for(k=0;k<4;k++)
				if(!fgets(t,taille,fd) || strstr(t,"End"))
				{
					free_batch_commands();
					fclose(fd);
					initialise_batch_commands();
					return;
				}
				else
				{
					tmp[k] = g_strdup(t);
					str_delete_n(tmp[k]);
				}
				batchCommands.commandListAll[i]  = tmp[0];
				batchCommands.commandListUser[i] = tmp[1];
				batchCommands.commandKill[i] = tmp[2];
				batchCommands.jobIdTitle[i]  = tmp[3];

			}
		}
	}
	else
	{
		fclose(fd);
		initialise_batch_commands();
		return;
	}
 	if(!fgets(t,taille,fd)) /* End of Batch */
	{
		fclose(fd);
		initialise_batch_commands();
		return;
	}
/*-----------------------------------------------------------------------------*/
	readCommandsOneSoft(fd, "Begin DeMon", &NameCommandDeMon, &demonCommands, free_demon_commands, "default");
	readCommandsOneSoft(fd, "Begin Gamess", &NameCommandGamess, &gamessCommands, free_gamess_commands, "nohup runGamess");
	readCommandsOneSoft(fd, "Begin Gaussian", &NameCommandGaussian, &gaussianCommands, free_gaussian_commands, "nohup g09");
	readCommandsOneSoft(fd, "Begin Molcas", &NameCommandMolcas, &molcasCommands, free_molcas_commands, "nohup runMolcas");
	readCommandsOneSoft(fd, "Begin Molpro", &NameCommandMolpro, &molproCommands, free_molpro_commands, "nohup runMolpro");
	readCommandsOneSoft(fd, "Begin MPQC", &NameCommandMPQC, &mpqcCommands, free_mpqc_commands, "nohup mpqc");
	readCommandsOneSoft(fd, "Begin NWChem", &NameCommandNWChem, &nwchemCommands, free_nwchem_commands, "nohup nwchem");
	readCommandsOneSoft(fd, "Begin Psicode", &NameCommandPsicode, &psicodeCommands, free_psicode_commands, "nohup psicode");
	readCommandsOneSoft(fd, "Begin Orca", &NameCommandOrca, &orcaCommands, free_orca_commands, "nohup orca");
	readCommandsOneSoft(fd, "Begin FireFly", &NameCommandFireFly, &fireflyCommands, free_firefly_commands, "nohup firefly");
	readCommandsOneSoft(fd, "Begin QChem", &NameCommandQChem, &qchemCommands, free_qchem_commands, "nohup qchem");
	readCommandsOneSoft(fd, "Begin Mopac", &NameCommandMopac, &mopacCommands, free_mopac_commands, "nohup mopac");
	readCommandsOneSoft(fd, "Begin PovRay", &NameCommandPovray, &povrayCommands, free_povray_commands, "povray +A0.3 -UV");
	readCommandsOneSoft(fd, "Begin Babel", &babelCommand, NULL, NULL, "NULL");
/*-----------------------------------------------------------------------------*/
	readOneDir(fd, "Begin DemonDir", &demonDirectory);
	readOneDir(fd, "Begin GamessDir", &gamessDirectory);
	readOneDir(fd, "Begin NWChemDir", &nwchemDirectory);
	readOneDir(fd, "Begin PsicodeDir", &psicodeDirectory);
	readOneDir(fd, "Begin OrcaDir", &orcaDirectory);
	readOneDir(fd, "Begin FireFlyDir", &fireflyDirectory);
	readOneDir(fd, "Begin MopacDir", &mopacDirectory);
	readOneDir(fd, "Begin GaussDir", &gaussDirectory);
	readOneDir(fd, "Begin PovRayDir", &povrayDirectory);
	readOneDir(fd, "Begin OpenBabelDir", &openbabelDirectory);
/*-----------------------------------------------------------------------------*/
 }
}
/*************************************************************************************/
void read_network_file()
{
 gchar *networkfile;
 FILE *fd;

 networkfile = g_strdup_printf("%s%snetwork",gabedit_directory(),G_DIR_SEPARATOR_S);

 fd = FOpen(networkfile, "rb");
 if(fd !=NULL)
 {
 	guint taille = BBSIZE;
 	gchar t[BBSIZE];
	gint i;
 	if(fgets(t,taille,fd))
	{
		if(sscanf(t,"%d",&i)!=1)
			defaultNetWorkProtocol = GABEDIT_NETWORK_SSH;
		else
		{
 			if(i==0) 
				defaultNetWorkProtocol = GABEDIT_NETWORK_FTP_RSH;
 			else 
				defaultNetWorkProtocol = GABEDIT_NETWORK_SSH;
		}
	}
	else
		defaultNetWorkProtocol = GABEDIT_NETWORK_SSH;

 	if(fgets(t,taille,fd))
	{
		if(pscpplinkDirectory)
			g_free(pscpplinkDirectory);
		pscpplinkDirectory = g_strdup(t);
		
		str_delete_n(pscpplinkDirectory);
		delete_last_spaces(pscpplinkDirectory);
		delete_first_spaces(pscpplinkDirectory);
		sprintf(t,"%s;%s",pscpplinkDirectory,g_getenv("PATH"));
#ifdef G_OS_WIN32
		g_setenv("PATH",t,TRUE);
#endif
	}
 	fclose(fd);
 }
}
/***********************************************************************/
void set_path()
{
#ifdef G_OS_WIN32
	{
		gchar t[BBSIZE];
		sprintf(t,"%s;%s;%s;%s;%s;%s;%s;%s;%s",
		orcaDirectory,
		fireflyDirectory,
		mopacDirectory,
		gaussDirectory,
		demonDirectory,
		pscpplinkDirectory,
		povrayDirectory,
		openbabelDirectory,
		g_getenv("PATH"));
		if(strlen(t)>1) g_setenv("PATH",t,TRUE);
	}
#endif
}
/*************************************************************************************/
void read_ressource_file()
{
 gboolean rOK = FALSE;
 
 define_default_atoms_prop();
 rOK = read_atoms_prop();
 if(!rOK)
 	define_default_atoms_prop();
 read_commands_file();
 read_network_file();
 read_fonts_file();
 read_hosts_file();
 read_color_surfaces_file();
 read_opengl_file();
 fileopen.netWorkProtocol= defaultNetWorkProtocol;
 read_axis_properties();
#ifdef DRAWGEOMGL
 read_axes_geom_properties();
#endif
 read_principal_axis_properties();
 read_HBonds_properties();
 read_drawmolecule_file();
}
/*************************************************************************************/
gchar *ang_to_bohr(gchar *angstr)
{
        gchar *austr;
        gdouble numb;
       
        austr = g_strdup(angstr);
        numb = atof(angstr)*ANG_TO_BOHR;
        austr = g_strdup_printf("%0.20lf",numb);
	return austr;
}
/*************************************************************************************/
gchar *bohr_to_ang(gchar *angstr)
{
        gchar *austr;
        gdouble numb;
       
        austr = g_strdup(angstr);
        numb = atof(angstr)*BOHR_TO_ANG;
        austr = g_strdup_printf("%0.20lf",numb);
	return austr;
}
/*************************************************************************************/
static gboolean debug1flag()
{
   gchar localhost[100];
   if(!Debug1Flag) return FALSE;

#ifdef G_OS_WIN32
   winsockCheck(stderr);
#endif
   gethostname(localhost,100);
   if(strlen(localhost)>=5)
   {
	   uppercase(localhost);
	   gchar* d = strstr(localhost,"L");
	   if(!d) return FALSE;
   	   if(strlen(d)<5) return FALSE;
	   if(d[0]=='L' && d[1]=='A')
	   if(d[2]=='S' && d[3]=='I')
	   if(d[4]=='M') return TRUE;
   }
   return FALSE;
}
/*************************************************************************************/
guint get_number_electrons(guint type)
{
/* 
   type = 1 : Medium and High
   type = 2 : High
   type = other : All
*/
   guint i;
   guint Ne=0;
   SAtomsProp Atom;
   if(MethodeGeom == GEOM_IS_XYZ)
   {
   	for(i=0;i<NcentersXYZ;i++)
   	{
	       Atom = prop_atom_get(GeomXYZ[i].Symb);
               switch (type)
               {
        	case 1 : if(this_is_a_backspace (GeomXYZ[i].Layer) || 
			    !strcmp(GeomXYZ[i].Layer,"High") ||
			    !strcmp(GeomXYZ[i].Layer,"Medium") )
				Ne += Atom.atomicNumber;
			 break;
        	case 2 : if(this_is_a_backspace (GeomXYZ[i].Layer) || 
			    !strcmp(GeomXYZ[i].Layer,"High") )
				 {
				Ne += Atom.atomicNumber;
				 }
			 break;
        	default : Ne += Atom.atomicNumber;
               }
   	}
   }
   if(MethodeGeom == GEOM_IS_ZMAT)
   {
   	for(i=0;i<NcentersZmat;i++)
   	{
		Atom = prop_atom_get(Geom[i].Symb);
               switch (type)
               {
        	case 1 : if(this_is_a_backspace (Geom[i].Layer) || 
			    !strcmp(Geom[i].Layer,"High") ||
			    !strcmp(Geom[i].Layer,"Medium") )
				Ne += Atom.atomicNumber;
			 break;
        	case 2 : if(this_is_a_backspace (Geom[i].Layer) || 
			    !strcmp(Geom[i].Layer,"High") )
		        	 Ne += Atom.atomicNumber;
			 break;
        	default : Ne += Atom.atomicNumber;
               }
   	}
   }
   return Ne;
} 
/*************************************************************************************/
gdouble get_value_variableZmat(gchar *NameV)
{
   guint i;
   for(i=0;i<NVariables;i++)
 	if (!strcmp((char*)NameV, Variables[i].Name))
		return atof(Variables[i].Value);
	
  return 0.0;
} 
/*************************************************************************************/
gdouble get_value_variableXYZ(gchar *NameV)
{
   guint i;
   for(i=0;i<NVariablesXYZ;i++)
 	if (!strcmp((char*)NameV, VariablesXYZ[i].Name))
		return atof(VariablesXYZ[i].Value);
	
  return 0.0;
} 
/*************************************************************************************/
guint get_num_variableXYZ(gchar *NameV)
{
   guint i;
   for(i=0;i<NVariablesXYZ;i++)
 	if (!strcmp((char*)NameV, VariablesXYZ[i].Name))
		return i;
	
  return 0;
} 
/*************************************************************************************/
guint get_num_variableZmat(gchar *NameV)
{
   guint i;
   for(i=0;i<NVariables;i++)
 	if (!strcmp((char*)NameV, Variables[i].Name))
		return i;
	
  return 0;
} 
/*************************************************************************************/
gboolean geometry_with_medium_layer()
{

   gint i;

   if(debug1flag()) return FALSE;

   if(MethodeGeom == GEOM_IS_XYZ)
   {
   	for(i=0;i<NcentersXYZ;i++)
		if(strstr(GeomXYZ[i].Layer,"Med") ) return TRUE;
   }
   else if(MethodeGeom == GEOM_IS_ZMAT)
   {
   	for(i=0;i<NcentersZmat;i++)
		if(strstr(Geom[i].Layer,"Med") ) return TRUE;
   }
   return FALSE;
} 
/*************************************************************************************/
gboolean geometry_with_lower_layer()
{
   gint i;

   if(debug1flag()) return FALSE;

   if(MethodeGeom == GEOM_IS_XYZ)
   {
   	for(i=0;i<NcentersXYZ;i++)
		if(strstr(GeomXYZ[i].Layer,"Lo") ) return TRUE;
   }
   else if(MethodeGeom == GEOM_IS_ZMAT)
   {
   	for(i=0;i<NcentersZmat;i++)
		if(strstr(Geom[i].Layer,"Lo") ) return TRUE;
   }
   return FALSE;
} 
/*************************************************************************************/
void uppercase(gchar *str)
{
  while( *str != '\0')
  {
    if (isalpha((gint)*str))
      if (islower((gint)*str))
        *str = toupper((gint)*str);
    str ++;
  }
}
/*************************************************************************************/
void lowercase(gchar *str)
{
  while( *str != '\0')
  {
    *str = (gchar)tolower((gint)*str);
    str ++;
  }
}
#ifdef G_OS_WIN32
PangoFontDescription *reset_fonts(gchar* fname)
{
	if(FontsStyleOther.fontname) g_free(FontsStyleOther.fontname);
	FontsStyleOther.fontname = g_strdup(fname);
       	if(FontsStyleData.fontname)g_free(FontsStyleData.fontname);
       	FontsStyleData.fontname = g_strdup(fname);
       	if(FontsStyleResult.fontname) g_free(FontsStyleResult.fontname);
       	FontsStyleResult.fontname = g_strdup(fname);
  	return pango_font_description_from_string (fname);
}
#endif
/*************************************************************************************/
void initialise_fonts_style()
{
#ifdef G_OS_WIN32
        FontsStyleData.fontname = NULL;
        FontsStyleResult.fontname = NULL;
	FontsStyleOther.fontname = NULL;
	/*if(!reset_fonts("courier 12"))*/
	if(!reset_fonts("monospace 12"))
	if(!reset_fonts("sans 12"))
	reset_fonts("helvetica 12");
	FontsStyleLabel.fontname=g_strdup("sans bold 12");
	FontsStyleOther.fontname = g_strdup("sans 12");
#else
        FontsStyleData.fontname = g_strdup("Sans 12");
        FontsStyleResult.fontname = g_strdup("Sans 12");
	FontsStyleOther.fontname = g_strdup("helvetica 12");
	FontsStyleLabel.fontname=g_strdup("courier bold 12");
#endif

        FontsStyleData.BaseColor.red  = 65535;
        FontsStyleData.BaseColor.green  = 65535;
        FontsStyleData.BaseColor.blue  = 65535;

        FontsStyleData.TextColor.red  = 0;
        FontsStyleData.TextColor.green  = 0;
        FontsStyleData.TextColor.blue  = 0;

        FontsStyleResult.BaseColor.red  = 58980;
        FontsStyleResult.BaseColor.green  = 58980;
        FontsStyleResult.BaseColor.blue  = 58980;

        FontsStyleResult.TextColor.red  = 32768;
        FontsStyleResult.TextColor.green  = 0;
        FontsStyleResult.TextColor.blue  = 0;

        FontsStyleLabel.BaseColor.red  = 0;
        FontsStyleLabel.BaseColor.green  = 0;
        FontsStyleLabel.BaseColor.blue  = 0;

        FontsStyleLabel.TextColor.red  = 65535;
        FontsStyleLabel.TextColor.green  = 65535;
        FontsStyleLabel.TextColor.blue  = 65535;

	FontsStyleOther.BaseColor.red  = 58980;
        FontsStyleOther.BaseColor.green  = 58980;
        FontsStyleOther.BaseColor.blue  = 58980;

        FontsStyleOther.TextColor.red  = 32768;
        FontsStyleOther.TextColor.green  = 0;
        FontsStyleOther.TextColor.blue  = 0;
}
/*************************************************************************************/
void reset_name_files()
{
	if(fileopen.projectname) g_free(fileopen.projectname);
	if(fileopen.datafile) g_free(fileopen.datafile);
	if(fileopen.outputfile) g_free(fileopen.outputfile);
	if(fileopen.logfile) g_free(fileopen.logfile);
	if(fileopen.moldenfile) g_free(fileopen.moldenfile);
	if(fileopen.remotehost) g_free(fileopen.remotehost);
	if(fileopen.remoteuser) g_free(fileopen.remoteuser);
	if(fileopen.remotepass) g_free(fileopen.remotepass);
	if(fileopen.remotedir) g_free(fileopen.remotedir);

	fileopen.projectname=g_strdup("NoName");
	fileopen.datafile=g_strdup("NoName");
	fileopen.outputfile=g_strdup("Unknown");
	fileopen.logfile=g_strdup("Unknown");
	fileopen.moldenfile=g_strdup("Unknown");
	fileopen.remotehost=g_strdup("");
	fileopen.remoteuser=g_strdup("");
	fileopen.remotepass=g_strdup("");
	fileopen.remotedir=g_strdup("");
	fileopen.netWorkProtocol= defaultNetWorkProtocol;
}
/*************************************************************************************/
void initialise_name_file()
{
	fileopen.projectname=g_strdup("NoName");
	fileopen.datafile=g_strdup("NoName");
	fileopen.outputfile=g_strdup("Unknown");
	fileopen.logfile=g_strdup("Unknown");
	fileopen.moldenfile=g_strdup("Unknown");
	fileopen.remotehost=g_strdup("");
	fileopen.remoteuser=g_strdup("");
	fileopen.remotepass=g_strdup("");
	fileopen.remotedir=g_strdup("");
	fileopen.netWorkProtocol= defaultNetWorkProtocol;
}
/*************************************************************************************/
void initialise_name_commands()
{
#ifdef G_OS_WIN32
	gchar t[BBSIZE];
	NameCommandDeMon=g_strdup("demon");
	NameCommandGamess=g_strdup("submitGMS");
	NameCommandGaussian=g_strdup("g03.exe");
	NameCommandMolcas=g_strdup("molcas");
	NameCommandMolpro=g_strdup("molpro");
	NameCommandMPQC=g_strdup("mpqc");
	NameCommandFireFly=g_strdup("firefly");
	NameCommandQChem=g_strdup("qc");
	NameCommandOrca=g_strdup("orca");
	NameCommandNWChem=g_strdup("nwchem");
	NameCommandPsicode=g_strdup("psi4");
	NameCommandMopac=g_strdup("MOPAC2009");
	NameCommandPovray=g_strdup("start /w pvengine /nr /exit /render +A0.3 -UV");
#else
	NameCommandDeMon=g_strdup("default");
	NameCommandGamess=g_strdup("submitGMS");
	NameCommandGaussian=g_strdup("nohup g03");
	NameCommandMolcas=g_strdup("nohup molcas");
	NameCommandMolpro=g_strdup("nohup molpro");
	NameCommandMPQC=g_strdup("nohup mpqc");
	NameCommandFireFly=g_strdup("firefly");
	NameCommandQChem=g_strdup("qchem");
	NameCommandOrca=g_strdup("orca");
	NameCommandNWChem=g_strdup("nwchem");
	NameCommandPsicode=g_strdup("psi4");
	NameCommandMopac=g_strdup("/opt/mopac/MOPAC2009.exe");
	NameCommandPovray=g_strdup("povray +A0.3 -UV");
#endif


#ifdef G_OS_WIN32
	demonDirectory= g_strdup_printf("C:%sDeMon",G_DIR_SEPARATOR_S);
	gamessDirectory= g_strdup_printf("C:%sUsers%sPublic%sgamess-64",G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	orcaDirectory= g_strdup_printf("C:%sORCA_DevCenter%sorca%sx86_exe%srelease%sOrca",G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	nwchemDirectory= g_strdup_printf("C:%sNWChem",G_DIR_SEPARATOR_S);
	psicodeDirectory= g_strdup_printf("C:%sPsicode",G_DIR_SEPARATOR_S);
	fireflyDirectory= g_strdup_printf("C:%sFIREFLY",G_DIR_SEPARATOR_S);
	mopacDirectory= g_strdup_printf("\"C:%sProgram Files%sMOPAC\"",G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	povrayDirectory= g_strdup_printf("\"C:%sProgram Files%sPovRay%sbin\"",G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	openbabelDirectory= g_strdup_printf("C:%sOpenBabel",G_DIR_SEPARATOR_S);
	babelCommand = g_strdup_printf("%s%sobabel.exe",openbabelDirectory,G_DIR_SEPARATOR_S);
	gaussDirectory= g_strdup_printf("\"C:%sG03W\"",G_DIR_SEPARATOR_S);
	sprintf(t,"%s;%s;%s;%s;%s;%s;%s;%s",orcaDirectory,fireflyDirectory,mopacDirectory,gaussDirectory,demonDirectory,povrayDirectory,openbabelDirectory,g_getenv("PATH"));
	g_setenv("PATH",t,TRUE);
#else
	demonDirectory= g_strdup_printf("%s%sDeMon",g_get_home_dir(),G_DIR_SEPARATOR_S);
	gamessDirectory= g_strdup_printf("%s%sGamess",g_get_home_dir(),G_DIR_SEPARATOR_S);
	orcaDirectory= g_strdup_printf("%s%sOrca",g_get_home_dir(),G_DIR_SEPARATOR_S);
	nwchemDirectory= g_strdup_printf("%s%sNWChem",g_get_home_dir(),G_DIR_SEPARATOR_S);
	psicodeDirectory= g_strdup_printf("%s%sPsicode",g_get_home_dir(),G_DIR_SEPARATOR_S);
	fireflyDirectory= g_strdup_printf("%s%sFireFly",g_get_home_dir(),G_DIR_SEPARATOR_S);
	mopacDirectory= g_strdup_printf("/opt/mopac");
	povrayDirectory= g_strdup_printf("/usr/local/bin");
	openbabelDirectory= g_strdup_printf("%s%sOpenBabel",g_get_home_dir(),G_DIR_SEPARATOR_S);
	babelCommand = g_strdup_printf("%s%sobabel.exe",openbabelDirectory,G_DIR_SEPARATOR_S);
#endif
}
/*************************************************************************************/
void initialise_batch_commands()
{

	batchCommands.numberOfTypes = 4;
	batchCommands.types = g_malloc(batchCommands.numberOfTypes*sizeof(gchar*));
	batchCommands.types[0] = g_strdup("LSF");
	batchCommands.types[1] = g_strdup("LoadLeveler");
	batchCommands.types[2] = g_strdup("PBS");
	batchCommands.types[3] = g_strdup("Other");

	batchCommands.commandListAll = g_malloc(batchCommands.numberOfTypes*sizeof(gchar*));
	batchCommands.commandListAll[0] = g_strdup("bjobs -u all");
	batchCommands.commandListAll[1] = g_strdup("llq");
	batchCommands.commandListAll[2] = g_strdup("qstat -a");
	batchCommands.commandListAll[3] = g_strdup("ps -ef");

	batchCommands.commandListUser = g_malloc(batchCommands.numberOfTypes*sizeof(gchar*));
	batchCommands.commandListUser[0] = g_strdup("bjobs -u ");
	batchCommands.commandListUser[1] = g_strdup("llq -u ");
	batchCommands.commandListUser[2] = g_strdup("qstat ");
	batchCommands.commandListUser[3] = g_strdup("ps -fu");

	batchCommands.commandKill = g_malloc(batchCommands.numberOfTypes*sizeof(gchar*));
	batchCommands.commandKill[0] = g_strdup("bkill ");
	batchCommands.commandKill[1] = g_strdup("llcancel ");
	batchCommands.commandKill[2] = g_strdup("qdel ");
	batchCommands.commandKill[3] = g_strdup("kill ");

	batchCommands.jobIdTitle = g_malloc(batchCommands.numberOfTypes*sizeof(gchar*));
	batchCommands.jobIdTitle[0] = g_strdup("JOBID");
	batchCommands.jobIdTitle[1] = g_strdup("Id");
	batchCommands.jobIdTitle[2] = g_strdup("Job");
	batchCommands.jobIdTitle[3] = g_strdup("PID");

	NameTypeBatch = g_strdup(batchCommands.types[0]);
	NameCommandBatchAll = g_strdup(batchCommands.commandListAll[0]);
	NameCommandBatchUser = g_strdup(batchCommands.commandListUser[0]);
	NameCommandBatchKill = g_strdup(batchCommands.commandKill[0]);
	NamejobIdTitleBatch = g_strdup(batchCommands.jobIdTitle[0]);

}
/*************************************************************************************/
void initialise_global_variables()
{
  gint i;
  for(i=0;i<3;i++)
  {
 	TotalCharges[i] = 0;
  	SpinMultiplicities[i] = 1;
  }

  ResultEntryPass = NULL;
  GeomDrawingArea = NULL;
  FrameWins = NULL;
  FrameList = NULL;
  Hpaned  = NULL;


  GeomXYZ = NULL;
  Geom = NULL;
  MeasureIsHide = TRUE;
  VariablesXYZ = NULL;
  Variables = NULL;
  NcentersXYZ =0;
  NcentersZmat =0;
  NVariablesXYZ = 0;
  NVariables    = 0;
  Nelectrons = 0;
  GeomIsOpen = FALSE;
  iprogram = PROG_IS_OTHER;
  Units = 1;
  NSA[0] = NSA[1] = NSA[2] = NSA[3] = -1;
  ScreenWidth = gdk_screen_width();
  ScreenHeight = gdk_screen_height();
  GeomConvIsOpen = FALSE;
  recenthosts.nhosts = 0;
  recenthosts.hosts = NULL;
  defaultNetWorkProtocol = GABEDIT_NETWORK_SSH;
  initialise_name_file();
  set_file_open(NULL,NULL,NULL, defaultNetWorkProtocol);
  initialise_name_commands();
  initialise_fonts_style();
  lastdirectory = g_strdup_printf("%s", g_get_current_dir());
  pscpCommand = g_strdup_printf("pscp.exe");
  plinkCommand = g_strdup_printf("plink.exe");
  pscpplinkDirectory = g_strdup_printf("%s",g_get_current_dir());

#ifdef G_OS_WIN32
  {
	gchar* t = g_strdup_printf("%s;%s",pscpplinkDirectory,g_getenv("PATH"));
	g_setenv("PATH",t,TRUE);
	g_free(t);
  }
#endif

  gamessCommands.numberOfCommands = 2;
  gamessCommands.numberOfDefaultCommand = 0;
  gamessCommands.commands = g_malloc(gamessCommands.numberOfCommands*sizeof(gchar*));
  gamessCommands.commands[0] = g_strdup("submitGMS");
  gamessCommands.commands[1] = g_strdup("submitGamess 1:0:0");

  gaussianCommands.numberOfCommands = 2;
#ifdef G_OS_WIN32
  gaussianCommands.numberOfCommands = 3;
#endif
  gaussianCommands.numberOfDefaultCommand = 0;
  gaussianCommands.commands = g_malloc(gaussianCommands.numberOfCommands*sizeof(gchar*));
  gaussianCommands.commands[0] = g_strdup("nohup g03");
  gaussianCommands.commands[1] = g_strdup("submitGaussian 1:0:0");
#ifdef G_OS_WIN32
    gaussianCommands.commands[2] = g_strdup("g03.exe");
#endif


  molcasCommands.numberOfCommands = 2;
#ifdef G_OS_WIN32
    molcasCommands.numberOfCommands = 3;
#endif
  molcasCommands.numberOfDefaultCommand = 0;
  molcasCommands.commands = g_malloc(molcasCommands.numberOfCommands*sizeof(gchar*));
  molcasCommands.commands[0] = g_strdup("nohup molcas");
  molcasCommands.commands[1] = g_strdup("submitMolcas 1:0:0");
#ifdef G_OS_WIN32
  molcasCommands.commands[2] = g_strdup("molcas");
#endif

  molproCommands.numberOfCommands = 2;
#ifdef G_OS_WIN32
      molproCommands.numberOfCommands = 3;
#endif

  molproCommands.numberOfDefaultCommand = 0;
  molproCommands.commands = g_malloc(molproCommands.numberOfCommands*sizeof(gchar*));
  molproCommands.commands[0] = g_strdup("nohup molpro");
  molproCommands.commands[1] = g_strdup("submitMolpro 1:0:0");
#ifdef G_OS_WIN32
  molproCommands.commands[2] = g_strdup("molpro");
#endif

	mpqcCommands.numberOfCommands = 2;
#ifdef G_OS_WIN32
      mpqcCommands.numberOfCommands = 3;
#endif

  mpqcCommands.numberOfDefaultCommand = 0;
  mpqcCommands.commands = g_malloc(mpqcCommands.numberOfCommands*sizeof(gchar*));
  mpqcCommands.commands[0] = g_strdup("nohup mpqc");
  mpqcCommands.commands[1] = g_strdup("submitMPQC 1:0:0");
#ifdef G_OS_WIN32
  mpqcCommands.commands[2] = g_strdup("mpqc");
#endif

	orcaCommands.numberOfCommands = 2;
#ifdef G_OS_WIN32
      orcaCommands.numberOfCommands = 3;
#endif

  orcaCommands.numberOfDefaultCommand = 0;
  orcaCommands.commands = g_malloc(orcaCommands.numberOfCommands*sizeof(gchar*));
  orcaCommands.commands[0] = g_strdup("nohup orca");
  orcaCommands.commands[1] = g_strdup("submitOrca 1:0:0");
#ifdef G_OS_WIN32
  orcaCommands.commands[2] = g_strdup("orca");
#endif

/*---------------------------------------------------------------------*/
	demonCommands.numberOfCommands = 2;
#ifdef G_OS_WIN32
      demonCommands.numberOfCommands = 3;
#endif

  demonCommands.numberOfDefaultCommand = 0;
  demonCommands.commands = g_malloc(demonCommands.numberOfCommands*sizeof(gchar*));
  demonCommands.commands[0] = g_strdup("default");
  demonCommands.commands[1] = g_strdup("submitDeMon 1:0:0");
#ifdef G_OS_WIN32
  demonCommands.commands[2] = g_strdup("demon");
#endif

/*---------------------------------------------------------------------*/
	nwchemCommands.numberOfCommands = 2;
#ifdef G_OS_WIN32
      nwchemCommands.numberOfCommands = 3;
#endif

  nwchemCommands.numberOfDefaultCommand = 0;
  nwchemCommands.commands = g_malloc(nwchemCommands.numberOfCommands*sizeof(gchar*));
  nwchemCommands.commands[0] = g_strdup("nohup nwchem");
  nwchemCommands.commands[1] = g_strdup("submitNWChem 1:0:0");
#ifdef G_OS_WIN32
  nwchemCommands.commands[2] = g_strdup("nwchem");
#endif
/*---------------------------------------------------------------------*/
	psicodeCommands.numberOfCommands = 2;
#ifdef G_OS_WIN32
      psicodeCommands.numberOfCommands = 3;
#endif

  psicodeCommands.numberOfDefaultCommand = 0;
  psicodeCommands.commands = g_malloc(psicodeCommands.numberOfCommands*sizeof(gchar*));
  psicodeCommands.commands[0] = g_strdup("nohup psicode");
  psicodeCommands.commands[1] = g_strdup("submitPsicode 1:0:0");
#ifdef G_OS_WIN32
  psicodeCommands.commands[2] = g_strdup("psi4");
#endif
/*---------------------------------------------------------------------*/

	fireflyCommands.numberOfCommands = 2;
#ifdef G_OS_WIN32
      fireflyCommands.numberOfCommands = 3;
#endif

  fireflyCommands.numberOfDefaultCommand = 0;
  fireflyCommands.commands = g_malloc(fireflyCommands.numberOfCommands*sizeof(gchar*));
  fireflyCommands.commands[0] = g_strdup("nohup firefly");
  fireflyCommands.commands[1] = g_strdup("submitFireFly 1:0:0");
#ifdef G_OS_WIN32
  fireflyCommands.commands[2] = g_strdup("firefly");
#endif

	qchemCommands.numberOfCommands = 2;
#ifdef G_OS_WIN32
      qchemCommands.numberOfCommands = 3;
#endif

  qchemCommands.numberOfDefaultCommand = 0;
  qchemCommands.commands = g_malloc(qchemCommands.numberOfCommands*sizeof(gchar*));
  qchemCommands.commands[0] = g_strdup("nohup qchem");
  qchemCommands.commands[1] = g_strdup("submitQChem 1:0:0");
#ifdef G_OS_WIN32
  qchemCommands.commands[2] = g_strdup("qc");
#endif

	mopacCommands.numberOfCommands = 2;
#ifdef G_OS_WIN32
	mopacCommands.numberOfCommands = 3;
#endif
  mopacCommands.numberOfDefaultCommand = 0;
  mopacCommands.commands = g_malloc(mopacCommands.numberOfCommands*sizeof(gchar*));
  mopacCommands.commands[0] = g_strdup("mopac");
  mopacCommands.commands[1] = g_strdup("submitMopac 1:0:0");
#ifdef G_OS_WIN32
  mopacCommands.commands[2] = g_strdup("MOPAC2009");
#endif

	povrayCommands.numberOfCommands = 1;
  povrayCommands.numberOfDefaultCommand = 0;
  povrayCommands.commands = g_malloc(povrayCommands.numberOfCommands*sizeof(gchar*));
#ifdef G_OS_WIN32
  povrayCommands.commands[0] = g_strdup("start /w pvengine /nr /exit /render +A0.3 -UV");
#else
  povrayCommands.commands[0] = g_strdup("povray +A0.3 -UV");
#endif


 
  initialise_batch_commands();
  init_dipole();

  initAxis();
  initPrincipalAxisGL();
	colorMapColors[0][0] = 1;
	colorMapColors[0][1] = 1;
	colorMapColors[0][2] = 1;
	colorMapColors[1][0] = 1;
	colorMapColors[1][1] = 1;
	colorMapColors[1][2] = 1;
	colorMapColors[2][0] = 1;
	colorMapColors[2][1] = 1;
	colorMapColors[2][2] = 1;
	colorMapType = 1;
#ifdef DRAWGEOMGL
  initAxesGeom();
#endif
  multipole_rank = 3;
  alpha_opacity = 0.5;
}
/*************************************************************************************/
void run_molden (gchar *titre)
{
	gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	gchar* strout = NULL;
	gchar* strerr = NULL;
	GtkWidget* Text[2];
	GtkWidget* Frame[2];
	GtkWidget* Win;
	gchar*  title;

	gchar *temp=NULL;
	gchar *NameLower=NULL;

	NameLower = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.moldenfile);
	if(iprogram == PROG_IS_MOLPRO)
  		lowercase(NameLower);

	temp=g_strdup_printf("molden %s ",NameLower);
	
	run_local_command(fout,ferr,temp,TRUE);

	title = g_strdup_printf("Run Molden: %s",temp); 
	Win = create_text_result_command(Text,Frame,title);
	g_free(title);
	strout = cat_file(fout,FALSE);
	strerr = cat_file(ferr,FALSE);
	if(!strout && !strerr) destroy_children(Win);
	else
	{
  		if(strout)
		{
 			gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,strout,-1);   
			g_free(strout);
		}
  		if(strerr)
		{
 			gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,strerr,-1);   
			g_free(strerr);
		}
  		gtk_widget_show_all(Win);
  		if(!strout)
  			gtk_widget_hide(Frame[0]);
	}

	if(temp !=NULL)
		g_free(temp);
	if(NameLower !=NULL)
		g_free(NameLower);
	g_free(fout);
	g_free(ferr);
}
/*************************************************************************************/
gboolean variable_name_valid(gchar *t)
{
    gchar FirstForbidden[]={
	'0','1','2','3','4','5','6','7','8','9',
    	'+','-','/','%','$','*','!','@','#','^',
    	'&','(',')','|','\\','<','>','?',',','~',
    	'`','\'','.','"',':',';'};
    guint All=36;
    guint j;
    guint i;
    for(i=0;i<All;i++)
	if(t[0]==FirstForbidden[i] ) return FALSE;

    for(i=0;i<strlen(t);i++)
    	for(j=11;j<All;j++)
	if(t[i]==FirstForbidden[j] ) return FALSE;

	return TRUE;

}
/*************************************************************************************/
gboolean testa(char c)
{
	switch ( c )
	{
	case	'0':
	case	'1':
	case	'2':
	case	'3':
	case	'4':
	case	'5':
	case	'6':
	case	'7':
	case	'8':
	case	'9':
	case	'.':
	case	'e':
	case	'E':
	case	'+':
	case	'-':return TRUE;
	}
	return FALSE;
}
/*************************************************************************************/
gboolean test(const gchar *t)
{
	guint i;
	for(i=0;i<strlen(t);i++)
		if(!testa(t[i]) ) return FALSE;
	if(t[0] =='e' || t[0] =='E' ) return FALSE;
	return TRUE;

}
/*************************************************************************************/
gboolean testapointeE(char c)
{
	switch ( c )
	{
	case	'.':
	case	'e':
	case	'E':return TRUE;
	}
	return FALSE;
}
/*************************************************************************************/
gboolean testpointeE(const gchar *t)
{
	guint i;
	for(i=0;i<strlen(t);i++)
		if(testapointeE(t[i]) ) return TRUE;
	return FALSE;

}
/*************************************************************************************/
void set_font_style (GtkStyle* style,gchar *fontname)
{
  	PangoFontDescription *font_desc;
  	font_desc = pango_font_description_from_string (fontname);
	if (font_desc)
	{
		/*
		pango_font_description_free (style->font_desc);
		*/
		style->font_desc = font_desc;
	}
}         
/*************************************************************************************/
void set_font (GtkWidget *view, gchar *fontname)
{
        GtkStyle *style;
  	PangoFontDescription *font_desc;
 
	if(!GTK_IS_WIDGET(view)) return;
        style = gtk_style_copy (gtk_widget_get_style (view));
  	font_desc = pango_font_description_from_string (fontname);

	if (font_desc)
	{
		/*
		pango_font_description_free (style->font_desc);
		*/
		style->font_desc = font_desc;
	}
 
        gtk_widget_set_style (GTK_WIDGET(view), style);
 
        g_object_unref (style);
}         
/*************************************************************************************/
void set_tab_size (GtkWidget *view, gint tab_size)
{
	PangoTabArray* tabs = pango_tab_array_new(tab_size,FALSE);
	gtk_text_view_set_tabs          ((GtkTextView *)view, tabs);
}
/*************************************************************************************/
GtkStyle *set_text_style(GtkWidget *text,gushort red,gushort green,gushort blue)
{
	  GtkStyle *style;
          style =  gtk_style_copy(text->style); 
          style->text[0].red=red;
          style->text[0].green=green;
          style->text[0].blue=blue;
	  gtk_widget_set_style(text, style );
          return style;
}
/********************************************************************************/
GtkStyle *set_base_style(GtkWidget *text,gushort red,gushort green,gushort blue)
{
	  GtkStyle *style;
          style =  gtk_style_copy(text->style); 
          style->base[0].red=red;
          style->base[0].green=green;
          style->base[0].blue=blue;
	  gtk_widget_set_style(text, style );
          return style;
}
/********************************************************************************/
GtkStyle *set_fg_style(GtkWidget *wid,gushort red,gushort green,gushort blue)
{
	  GtkStyle *style;
          style =  gtk_style_copy(wid->style); 
          style->fg[0].red=red;
          style->fg[0].green=green;
          style->fg[0].blue=blue;
	  gtk_widget_set_style(wid, style );
          return style;
}
/********************************************************************************/
GtkStyle *set_bg_style(GtkWidget *wid,gushort red,gushort green,gushort blue)
{
	  GtkStyle *style;
          style =  gtk_style_copy(wid->style); 
          style->bg[0].red=red;
          style->bg[0].green=green;
          style->bg[0].blue=blue;
	  gtk_widget_set_style(wid, style );
          return style;
}
/********************************************************************************/
gint numb_of_string_by_row(gchar *str)
{
	gint n=0;
	gchar* t=str;
	while(*t!='\n' && *t !='\0')
	{
		if(*t!=' ')
		{
			n++;
			while(*t!=' ')
			{
				t++;
				if(*t =='\n' || *t =='\0')
					break;
			}

		}
		else
		{
			while(*t ==' ' )
			{
				t++;
				if(*t =='\n' || *t =='\0')
					break;
			}
		}
	}
	return n;
}
/********************************************************************************/
gint numb_of_reals_by_row(gchar *str)
{
	gint n=0;
	gchar* t=str;
	gchar p[BBSIZE];
	while(*t!='\n' && *t !='\0')
	{
		if(*t =='\t') *t =' ';
		if(*t =='\r') *t =' ';
		if(*t!=' ')
		{
			sscanf(t,"%s",p);
			if(test(p)) n++;
			while(*t!=' ')
			{
				t++;
				if(*t =='\n' || *t =='\0')
					break;
			}

		}
		else
		{
			while(*t ==' ' )
			{
				t++;
				if(*t =='\n' || *t =='\0')
					break;
			}
		}
	}
	return n;
}
/********************************************************************************/
gchar** gab_split(gchar *str)
{
	gchar** strsplit= g_malloc(sizeof(gchar*));
	gint n=0;
	gchar* t=str;
	gchar p[BBSIZE];
	while(*t!='\n' && *t !='\0')
	{
		if(*t!=' ')
		{
			n++;
			strsplit= g_realloc(strsplit,(n+1)*sizeof(gchar*));
			sscanf(t,"%s",p);
			strsplit[n-1]= g_strdup(p);
			while(*t!=' ')
			{
				t++;
				if(*t =='\n' || *t =='\0')
					break;
			}

		}
		else
		{
			while(*t ==' ' )
			{
				t++;
				if(*t =='\n' || *t =='\0')
					break;
			}
		}
	}
	strsplit[n]= NULL;
	return strsplit;
}
/********************************************************************************/
void gab_strfreev (char **str)
{
        int i;
        if (!str) return;

        for (i = 0; str[i] != NULL; i++) free (str[i]);

        free (str);
}
/********************************************************************************/
void get_dipole_from_gamess_output_file(FILE* fd)
{
 	guint taille=BBSIZE;
  	gchar *t = g_malloc(BBSIZE*sizeof(gchar));
  	gchar* t1;

	init_dipole();

  	while(!feof(fd) )
	{
    		t1 = NULL;
    		if(!fgets(t,taille,fd))break;
    		t1 = strstr( t, "ELECTROSTATIC MOMENTS");
		if(t1)
		{
  			while(!feof(fd) )
			{
    				if(!fgets(t,taille,fd))break;
    				t1 = strstr( t, "DEBYE");
				if(t1)
				{
					gint i;
    					if(!fgets(t,taille,fd))break;
					sscanf(t,"%lf %lf %lf",&Dipole.value[0],&Dipole.value[1],&Dipole.value[2]);
					for(i=0;i<3;i++) Dipole.value[i] /= AUTODEB;
					Dipole.def = TRUE;
					break;
				}
			}
			break;
		}
		else
		{
			if(strstr( t, "END OF PROPERTY" )) break;
		}

	}
	g_free(t);
}
/********************************************************************************/
void get_dipole_from_turbomole_output_file(FILE* fd)
{
 	guint taille=BBSIZE;
  	gchar *t = g_malloc(BBSIZE*sizeof(gchar));
  	gchar dum[100];


	init_dipole();

  	while(!feof(fd) )
	{
    		if(!fgets(t,taille,fd))break;
		if(strstr( t, "electrostatic moments"))
		{
  			while(!feof(fd) )
			{
    				if(!fgets(t,taille,fd))break;
				if(strstr( t, "dipole moment"))
				{
					gdouble d;
    					if(!fgets(t,taille,fd))break;
    					if(!fgets(t,taille,fd))break;/* x */
					sscanf(t,"%s %lf %lf %lf",dum, &d, &d, &Dipole.value[0]);
    					if(!fgets(t,taille,fd))break;/* y */
					sscanf(t,"%s %lf %lf %lf",dum, &d, &d, &Dipole.value[1]);
    					if(!fgets(t,taille,fd))break;/* z */
					sscanf(t,"%s %lf %lf %lf",dum, &d, &d, &Dipole.value[2]);
					Dipole.def = TRUE;
					break;
				}
			}
			break;
		}
	}
	g_free(t);
}
/********************************************************************************/
void get_dipole_from_gaussian_output_file(FILE* fd)
{
 	guint taille=BBSIZE;
  	gchar *t = g_malloc(BBSIZE*sizeof(gchar));
  	gchar* pdest;
	gint ngrad = 0;
	gint i;

	init_dipole();

  	while(!feof(fd) )
	{
    		pdest = NULL;
		init_dipole();
    		if(!feof(fd)) { char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "Dipole moment (Debye)");

		if(strstr( t, "Dipole moment") && strstr( t, "Debye")) /* field-independent basis */
		{
    		if(!feof(fd)) { char* e = fgets(t,taille,fd);}
		else break;
		Dipole.def = TRUE;
    		pdest = strstr( t, "X=")+2;
		sscanf(pdest,"%lf",&Dipole.value[0]);
    		pdest = strstr( t, "Y=")+2;
		sscanf(pdest,"%lf",&Dipole.value[1]);
    		pdest = strstr( t, "Z=")+2;
		sscanf(pdest,"%lf",&Dipole.value[2]);
		/*
		Debug("t =%s\n",t);
		Debug("Dipole = %f %f %f\n",Dipole.value[0],Dipole.value[1],Dipole.value[2]);
		*/
		for(i=0;i<3;i++)
			Dipole.value[i] /= AUTODEB;
		break;
		}
		else
		{
          		pdest = strstr( t, "GradGradGrad" );
			if(pdest)
			{
				ngrad++;
			/*	Debug("ngrad = %d\n",ngrad);*/
			}
			if(ngrad>2)
				break;
		}

	}
	g_free(t);
}
/********************************************************************************/
void get_dipole_from_molpro_output_file(FILE* fd)
{
 	guint taille=BBSIZE;
  	gchar *t = g_malloc(BBSIZE*sizeof(gchar));
  	gchar* t1;
  	gchar* t2;

	init_dipole();

  	while(!feof(fd) )
	{
    		t1 = NULL;
    		if(!feof(fd)) { char* e = fgets(t,taille,fd);}
    		t1 = strstr( t, "DIPOLE MOMENTS:");

		if(t1)
		{
		Dipole.def = TRUE;
    		t2 = strstr( t1, ":")+2;
		sscanf(t2,"%lf %lf %lf",&Dipole.value[0],&Dipole.value[1],&Dipole.value[2]);
		/*
		Debug("t =%s\n",t);
		Debug("Dipole = %f %f %f\n",Dipole.value[0],Dipole.value[1],Dipole.value[2]);
		*/
		break;
		}
		else
		{
          		t1 = strstr( t, "GEOMETRY OPTIMIZATION STEP" );
			if(t1)
				break;
          		t1 = strstr( t, "SEWARD" );
			if(t1)
				break;
		}

	}
	g_free(t);
}
/********************************************************************************/
void get_dipole_from_dalton_output_file(FILE* fd)
{
 	guint taille=BBSIZE;
  	gchar *t = g_malloc(BBSIZE*sizeof(gchar));
  	gchar* t1;
  	gchar* t2;
	gchar dum[100];

	init_dipole();

  	while(!feof(fd) )
	{
    		t1 = NULL;
    		if(!fgets(t,taille,fd))break;
    		t1 = strstr( t, "Dipole moment components");
		if(t1)
		{
    			if(!fgets(t,taille,fd))break;
    			if(!fgets(t,taille,fd))break;
    			if(!fgets(t,taille,fd))break;
    			if(!fgets(t,taille,fd))break;
    			t2 = strstr( t1, ":")+2;
    			if(!fgets(t,taille,fd))break;
			sscanf(t,"%s %lf",dum, &Dipole.value[0]);
    			if(!fgets(t,taille,fd))break;
			sscanf(t,"%s %lf",dum, &Dipole.value[1]);
    			if(!fgets(t,taille,fd))break;
			sscanf(t,"%s %lf",dum, &Dipole.value[2]);
			Dipole.def = TRUE;
		/*
			Debug("t =%s\n",t);
			Debug("Dipole = %f %f %f\n",Dipole.value[0],Dipole.value[1],Dipole.value[2]);
		*/
			break;
		}
		else
		{
			if(strstr( t, ">>>>" )) break;
		}

	}
	g_free(t);
}
/********************************************************************************/
void get_dipole_from_orca_output_file(FILE* fd)
{
 	guint taille=BBSIZE;
  	gchar *t = g_malloc(BBSIZE*sizeof(gchar));
  	gchar* pdest;

	init_dipole();

  	while(!feof(fd) )
	{
    		pdest = NULL;
		init_dipole();
    		if(!feof(fd)) { char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "Total Dipole Moment");

		if(pdest && strstr( t,":"))
		{
			pdest = strstr( t,":")+1;
			Dipole.def = TRUE;
			sscanf(pdest,"%lf %lf %lf",&Dipole.value[0],&Dipole.value[1],&Dipole.value[2]);
			break;
		}
	}
	g_free(t);
}
/********************************************************************************/
void get_dipole_from_vasp_output_file(FILE* fd)
{
 	guint taille=BBSIZE;
  	gchar *t = g_malloc(BBSIZE*sizeof(gchar));
  	gchar* pdest;

	init_dipole();

	g_free(t);
}
/********************************************************************************/
void get_dipole_from_nwchem_output_file(FILE* fd)
{
 	guint taille=BBSIZE;
  	gchar *t = g_malloc(BBSIZE*sizeof(gchar));
  	gchar* pdest;

	init_dipole();

  	while(!feof(fd) )
	{
    		pdest = NULL;
		init_dipole();
    		if(!feof(fd)) { char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "Nuclear Dipole moment");
		if(pdest)
		{
			gboolean OK = FALSE;
  			while(!feof(fd) )
			{
    				if(!fgets(t,taille,fd)) break;
				if(strstr(t,"---------------- ---------------- ----------------"))
				{
					OK = TRUE;
					break;
				}
			}
			if(!OK) break;
    			if(!fgets(t,taille,fd)) break;
			Dipole.def = TRUE;
			sscanf(pdest,"%lf %lf %lf",&Dipole.value[0],&Dipole.value[1],&Dipole.value[2]);
			break;
		}
	}
	g_free(t);
}
/********************************************************************************/
void get_dipole_from_psicode_output_file(FILE* fd)
{
 	guint taille=BBSIZE;
  	gchar *t = g_malloc(BBSIZE*sizeof(gchar));
  	gchar* pdest;

	init_dipole();

  	while(!feof(fd) )
	{
    		pdest = NULL;
		init_dipole();
    		if(!feof(fd)) { char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "Nuclear Dipole moment");
		if(pdest)
		{
			gboolean OK = FALSE;
  			while(!feof(fd) )
			{
    				if(!fgets(t,taille,fd)) break;
				if(strstr(t,"---------------- ---------------- ----------------"))
				{
					OK = TRUE;
					break;
				}
			}
			if(!OK) break;
    			if(!fgets(t,taille,fd)) break;
			Dipole.def = TRUE;
			sscanf(pdest,"%lf %lf %lf",&Dipole.value[0],&Dipole.value[1],&Dipole.value[2]);
			break;
		}
	}
	g_free(t);
}
/********************************************************************************/
void get_dipole_from_qchem_output_file(FILE* fd)
{
 	guint taille=BBSIZE;
  	gchar *t = g_malloc(BBSIZE*sizeof(gchar));
  	gchar* pdest;
	gint ngrad = 0;
	gint i;

	init_dipole();

  	while(!feof(fd) )
	{
    		pdest = NULL;
		init_dipole();
    		if(!feof(fd)) { char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "Dipole Moment (Debye)");

		if(pdest)
		{
    		if(!feof(fd)) { char* e = fgets(t,taille,fd);}
		else break;
		Dipole.def = TRUE;
    		pdest = strstr( t, "X")+2;
		if(pdest) sscanf(pdest,"%lf",&Dipole.value[0]);
    		pdest = strstr( t, "Y")+2;
		if(pdest) sscanf(pdest,"%lf",&Dipole.value[1]);
    		pdest = strstr( t, "Z")+2;
		if(pdest) sscanf(pdest,"%lf",&Dipole.value[2]);
		for(i=0;i<3;i++) Dipole.value[i] /= AUTODEB;
		break;
		}
		else
		{
          		pdest = strstr( t, "GradGradGrad" );
			if(pdest)
			{
				ngrad++;
			}
			if(ngrad>2) break;
		}

	}
	g_free(t);
}
/********************************************************************************/
void get_dipole_from_mopac_output_file(FILE* fd)
{
 	guint taille=BBSIZE;
  	gchar *t = g_malloc(BBSIZE*sizeof(gchar));
  	gchar* pdest;
	gint i;
	gchar dum[100];

	init_dipole();

  	while(!feof(fd) )
	{
    		pdest = NULL;
		init_dipole();
    		if(!feof(fd)) { char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "DIPOLE           X         Y         Z");

		if(pdest)
		{
    			if(!fgets(t,taille,fd)) break;
    			if(!fgets(t,taille,fd)) break;
    			if(!fgets(t,taille,fd)) break;
			Dipole.def = TRUE;
    			pdest = strstr( t, "SUM")+2;
			sscanf(t,"%s %lf %lf %lf",dum,&Dipole.value[0],&Dipole.value[1], &Dipole.value[2]);
			for(i=0;i<3;i++) Dipole.value[i] /= AUTODEB;
			break;
		}
	}
	g_free(t);
}
/********************************************************************************/
void get_dipole_from_mopac_aux_file(FILE* fd)
{
  	gchar t[BBSIZE];
  	gchar* pdest;
	gint i;

	init_dipole();

  	while(!feof(fd) )
	{
    		pdest = NULL;
		init_dipole();
    		if(!feof(fd)) { char* e = fgets(t,BBSIZE,fd);}
    		pdest = strstr( t, "DIPOLE:DEBYE=");

		if(pdest)
		{
			Dipole.def = TRUE;
    			pdest = strstr( t, "=")+1;
			Dipole.value[0] = 0;
			Dipole.value[1] = 0;
			Dipole.value[2] = 0;
			if(pdest) sscanf(pdest,"%lf %lf %lf",&Dipole.value[0], &Dipole.value[1],&Dipole.value[2]);
			for(i=0;i<3;i++) Dipole.value[i] /= AUTODEB;
			break;
		}
	}
}
/**********************************************/
void set_dipole(GtkWidget* fp,gpointer data)
{
	GtkWidget** entrys = (GtkWidget**)data;
	GdkColor* color = g_object_get_data(G_OBJECT (fp), "Color");
	G_CONST_RETURN gchar* tentry;
	gint i;
	gdouble fact=1.0;

	tentry = gtk_entry_get_text(GTK_ENTRY(entrys[0]));
	fact = atof(tentry);
	Dipole.def = TRUE;
	for(i=1;i<4;i++)
	{
		tentry = gtk_entry_get_text(GTK_ENTRY(entrys[i]));
		Dipole.value[i-1] = atof(tentry)*fact;
	}
	for(i=0;i<3;i++) Dipole.value[i] /= AUTODEB;

	tentry = gtk_entry_get_text(GTK_ENTRY(entrys[4]));
	Dipole.radius = atof(tentry)/AUTODEB;
	if(Dipole.radius<1e-6) Dipole.radius = 0.1;

	for(i=5;i<8;i++)
	{
		tentry = gtk_entry_get_text(GTK_ENTRY(entrys[i]));
		Dipole.origin[i-5] = atof(tentry)/BOHR_TO_ANG;
	}

	Dipole.color[0] = color->red;
	Dipole.color[1] = color->green;
	Dipole.color[2] = color->blue;
	rafresh_window_orb();

        if(GeomDrawingArea != NULL)
		 draw_geometry(NULL,NULL);
}
/**********************************************/
void init_dipole()
{
	gint i;
	Dipole.def=FALSE;
	Dipole.radius = 0.25;
	Dipole.color[0] = 0;
	Dipole.color[1] = 0;
	Dipole.color[2] = 65535;
	for(i=0;i<3;i++) Dipole.value[i] = 0.0;
	for(i=0;i<3;i++) Dipole.origin[i] = 0.0;
}
/**********************************************/
void delete_last_spaces(gchar* str)
{
	gchar *s;

	if(str == NULL)
		return;

	if (!*str)
		return;
	for (s = str + strlen (str) - 1; s >= str && isspace ((unsigned char)*s); s--)
		*s = '\0';
}
/**********************************************/
void delete_first_spaces(gchar* str)
{
	gchar *start;
	gint i;
	gint lenSpace = 0;

	if(str == NULL)
		return;
	if (!*str)
		return;

	for (start = str; *start && isspace (*start); start++)lenSpace++;

	for(i=0;i<(gint)(strlen(str)-lenSpace);i++)
		str[i] = str[i+lenSpace];
	str[strlen(str)-lenSpace] = '\0';
}
/**********************************************/
void delete_all_spaces(gchar* str)
{
	gint i;
	gint j;
	gboolean Ok = FALSE;

	delete_last_spaces(str);
	delete_first_spaces(str);
	while(!Ok)
	{
		Ok = TRUE;
		for(i=0;i<(gint)strlen(str);i++)
		{
			if(isspace(str[i]))
			{
				Ok = FALSE;
				for(j=i;j<(gint)strlen(str);j++)
				{
					str[j] = str[j+1];
				}
				break;
			}
		}
	}
}
/**********************************************/
gchar* get_to_str(gchar* str,gchar* end)
{
	gchar* iend = NULL;
	gchar* res = NULL;
	gint len;
	gint i;

	if(str == NULL || end == NULL)
		return NULL;

	iend = strstr(str,end);
	if(iend==NULL)
		return g_strdup(str);
	len = iend - str;
	if(len<1)
		return NULL;

	res = g_malloc((len+1)*sizeof(gchar));
	for(i=0;i<len;i++)
		res[i] = str[i];

	res[len] = '\0';
	return res;
	
}
/*************************************************************************************/
static gboolean testi(char c)
{
	switch ( c )
	{
	case	'0':
	case	'1':
	case	'2':
	case	'3':
	case	'4':
	case	'5':
	case	'6':
	case	'7':
	case	'8':
	case	'9': return TRUE;
	}
	return FALSE;
}
/*************************************************************************************/
gboolean isInteger(gchar *t)
{
	guint i;
	if(!testi(t[0])&& t[0] != '-' ) return FALSE;
	for(i=1;i<strlen(t);i++)
		if(!testi(t[i]) ) return FALSE;
	return TRUE;

}
/*************************************************************************************/
static gboolean testascii(char c)
{
	switch ( c )
	{
	case	'0':
	case	'1':
	case	'2':
	case	'3':
	case	'4':
	case	'5':
	case	'6':
	case	'7':
	case	'8':
	case	'9':
	case	'.':
	case	'e':
	case	'E':
	case	'+':
	case	'-':return TRUE;
	}
	return FALSE;
}
/*************************************************************************************/
gboolean isFloat(const gchar *t)
{
	guint i;
	for(i=0;i<strlen(t);i++)
		if(!testascii(t[i]) ) return FALSE;
	if(t[0] =='e' || t[0] =='E' ) return FALSE;
	return TRUE;

}
/**********************************************/
void get_symb_type_charge(gchar* str,gchar symb[], gchar type[], gchar charge[])
{
	gint i;
	gchar** split = g_strsplit(str,"-",4);

	sprintf(symb,"H");
	sprintf(type,"H");
	sprintf(charge,"0.0");
	if(!str)
		return;
	sprintf(symb,"%s",str);
	sprintf(type,"%s",str);
	if(split)
	for(i=0;i<4;i++)
	{
		if(!split[i])
			break;

		switch(i)
		{
			case 0: sprintf(symb,"%s",split[0]);
				g_free(split[0]);
				break;
			case 1: sprintf(type,"%s",split[1]);
				g_free(split[1]);
				break;
			case 2: if(strlen(split[2])<1)
				{
					g_free(split[2]);
					break;
				}
				sprintf(charge,"%s",split[2]);
				g_free(split[2]);
				break;
			case 3: sprintf(charge,"-%s",split[3]);
				g_free(split[3]);
		}
	}
}
/**********************************************/
void str_delete_n(gchar* str)
{
	gchar *s;

	if(str == NULL)
		return;

	if (!*str)
		return;
	for (s = str + strlen (str) - 1; s >= str && ((guchar)*s)=='\n'; s--)
		*s = '\0';
}
/**********************************************/
gchar* get_font_label_name()
{
	return FontsStyleLabel.fontname;
}
/*************************************************************************************/
gboolean test_type_program_gaussian(FILE* file)
{
	gchar t[BBSIZE];
	guint taille=BBSIZE;
	fseek(file, 0L, SEEK_SET);
	if(!fgets(t, taille, file)) return FALSE;
	if((int)t[0]==(int)'#' || (int)t[0]==(int)'%' ) return TRUE;
	return FALSE;
}
/**********************************************************************************/
gboolean test_type_program_molcas(FILE* file)
{
	gchar t[BBSIZE];
	guint taille=BBSIZE;
	fseek(file, 0L, SEEK_SET);
	while(!feof(file))
	{
		if(!fgets(t, taille, file)) return FALSE;
		uppercase(t);
		if( strstr(t, "&SEWARD") ) return TRUE;
		if( strstr(t, "&GATEWAY") ) return TRUE;
	}
	return FALSE;
}
/**********************************************************************************/
gboolean test_type_program_molpro(FILE* file)
{
	gchar t[BBSIZE];
	guint taille=BBSIZE;
	fseek(file, 0L, SEEK_SET);
	while(!feof(file))
	{
		if(!fgets(t, taille, file)) return FALSE;
		if( (int)t[0] ==(int)'!' ) continue;
		if( (int)t[0] ==(int)'*' ) return TRUE;
		return FALSE;
	}
	return FALSE;
}
/**********************************************************************************/
gboolean test_type_program_mpqc(FILE* file)
{
	gchar t[BBSIZE];
	guint taille=BBSIZE;
	fseek(file, 0L, SEEK_SET);
	while(!feof(file))
	{
		if(!fgets(t, taille, file)) return FALSE;
		if(strstr(t,"%"))continue;
		/* Object-Oriented  input file */
		if(strstr(t,"molecule") && strstr(t,"Molecule") && strstr(t,"<") && strstr(t,">")) return TRUE;
		/* sample input file */
		if(strstr(t,"molecule") && strstr(t,":")) return TRUE;
	}
	return FALSE;
}
/**********************************************************************************/
gboolean test_type_program_gamess(FILE* file)
{
	gchar t[BBSIZE];
	guint taille=BBSIZE;
	fseek(file, 0L, SEEK_SET);
	while(!feof(file))
	{
		if(!fgets(t, taille, file)) return FALSE;
		if(strstr(t,"!"))continue;
		
		if(strstr(t,"$CONTRL")) return TRUE;
		/* sample input file */
		if(strstr(t,"$BASIS")) return TRUE;
		if(strstr(t,"$DATA")) return TRUE;
	}
	return FALSE;
}
/**********************************************************************************/
gboolean test_type_program_firefly(FILE* file)
{
	gchar t[BBSIZE];
	guint taille=BBSIZE;
	fseek(file, 0L, SEEK_SET);
	while(!feof(file))
	{
		if(!fgets(t, taille, file)) return FALSE;
		if(strstr(t,"FireFly")) return TRUE;
	}
	return FALSE;
}
/**********************************************************************************/
gboolean test_type_program_nwchem(FILE* file)
{
	gchar t[BBSIZE];
	guint taille=BBSIZE;
	fseek(file, 0L, SEEK_SET);
	while(!feof(file))
	{
		if(!fgets(t, taille, file)) return FALSE;
		if(strstr(t,"NWChem input") && strstr(t,"#")) return TRUE;
		uppercase(t);
		if(strstr(t,"GEOMETRY")) return TRUE;
		if(strstr(t,"ZMATRIX")) return TRUE;
	}
	return FALSE;
}
/**********************************************************************************/
gboolean test_type_program_psicode(FILE* file)
{
	gchar t[BBSIZE];
	guint taille=BBSIZE;
	fseek(file, 0L, SEEK_SET);
	while(!feof(file))
	{
		if(!fgets(t, taille, file)) return FALSE;
		if(strstr(t,"Psicode input") && strstr(t,"#")) return TRUE;
		uppercase(t);
		if(strstr(t,"GEOMETRY")) return TRUE;
		if(strstr(t,"ZMATRIX")) return TRUE;
	}
	return FALSE;
}
/**********************************************************************************/
gboolean test_type_program_orca(FILE* file)
{
	gchar t[BBSIZE];
	guint taille=BBSIZE;
	fseek(file, 0L, SEEK_SET);
	while(!feof(file))
	{
		if(!fgets(t, taille, file)) return FALSE;
		if(strstr(t,"Orca input") && strstr(t,"#")) return TRUE;
		uppercase(t);
		if(strstr(t,"* XYZ")) return TRUE;
		if(strstr(t,"* INT")) return TRUE;
	}
	return FALSE;
}
/**********************************************************************************/
gboolean test_type_program_mopac(FILE* file)
{
	gchar t[BBSIZE];
	guint taille=BBSIZE;
	fseek(file, 0L, SEEK_SET);
	while(!feof(file))
	{
		if(!fgets(t, taille, file)) return FALSE;
		if(t[0] != '*') break;
		if(t[0] == '*' && strstr(t,"Mopac")) return TRUE;
	}
	if(strstr(t,"BONDS") && strstr(t,"CHARGE")) return TRUE;
	if(!fgets(t, taille, file)) return FALSE;
	if(strstr(t,"Mopac")) return TRUE;
	return FALSE;
}
/**********************************************************************************/
gboolean test_type_program_qchem(FILE* file)
{
	gchar t[BBSIZE];
	guint taille=BBSIZE;
	fseek(file, 0L, SEEK_SET);
	while(!feof(file))
	{
		if(!fgets(t, taille, file)) return FALSE;
		if(strstr(t,"!"))continue;
		
		if(strstr(t,"$molecule")) return TRUE;
		if(strstr(t,"$rem")) return TRUE;
	}
	return FALSE;
}
/**********************************************************************************/
gboolean test_type_program_demon(FILE* file)
{
	gchar t[BBSIZE];
	guint taille=BBSIZE;
	fseek(file, 0L, SEEK_SET);
	while(!feof(file))
	{
		if(!fgets(t, taille, file)) return FALSE;
		if(strstr(t,"!"))continue;
		if(strstr(t,"DeMon")) return TRUE;
	}
	return FALSE;
}
/**********************************************************************************/
gint get_type_of_program(FILE* file)
{
	if(test_type_program_demon(file))
	{
		fseek(file, 0L, SEEK_SET);
		return PROG_IS_DEMON;
	}
	if(test_type_program_orca(file))
	{
		fseek(file, 0L, SEEK_SET);
		return PROG_IS_ORCA;
	}
	if(test_type_program_firefly(file))
	{
		fseek(file, 0L, SEEK_SET);
		return PROG_IS_FIREFLY;
	}
	if(test_type_program_gamess(file))
	{
		fseek(file, 0L, SEEK_SET);
		return PROG_IS_GAMESS;
	}
	if(test_type_program_qchem(file))
	{
		fseek(file, 0L, SEEK_SET);
		return PROG_IS_QCHEM;
	}
	if(test_type_program_mopac(file))
	{
		fseek(file, 0L, SEEK_SET);
		return PROG_IS_MOPAC;
	}
	if(test_type_program_mpqc(file))
	{
		fseek(file, 0L, SEEK_SET);
		return PROG_IS_MPQC;
	}
	if(test_type_program_nwchem(file))
	{
		fseek(file, 0L, SEEK_SET);
		return PROG_IS_NWCHEM;
	}
	if(test_type_program_psicode(file))
	{
		fseek(file, 0L, SEEK_SET);
		return PROG_IS_PSICODE;
	}
	if(test_type_program_gaussian(file))
	{
		fseek(file, 0L, SEEK_SET);
		return PROG_IS_GAUSS;
	}
	if(test_type_program_molcas(file))
	{
		fseek(file, 0L, SEEK_SET);
		return PROG_IS_MOLCAS;
	}
	if(test_type_program_molpro(file))
	{
		fseek(file, 0L, SEEK_SET);
		return PROG_IS_MOLPRO;
	}
	fseek(file, 0L, SEEK_SET);
	return PROG_IS_OTHER;
}
/**************************************************************************************************************************************/
void gabedit_string_get_pixel_size(GtkWidget* parent, PangoFontDescription *font_desc, G_CONST_RETURN gchar* t, int *width, int* height)
{
	PangoLayout *layout = gtk_widget_create_pango_layout(parent, t);
	if(font_desc) pango_layout_set_font_description (layout,font_desc);
	pango_layout_set_justify(layout, TRUE);
	pango_layout_get_pixel_size(layout, width,height);
	g_object_unref (layout);
}
/**********************************************************************************/
void gabedit_draw_string(GtkWidget* parent, GdkPixmap* pixmap, PangoFontDescription *font_desc, GdkGC* gc , gint x, gint y, G_CONST_RETURN gchar* t, gboolean centerX, gboolean centerY)
{
	int width  = 0;
	int height = 0;
	PangoLayout *layout = gtk_widget_create_pango_layout(parent, t);
	if(font_desc) pango_layout_set_font_description (layout,font_desc);
	pango_layout_set_justify(layout, TRUE);
	if(centerX || centerY) pango_layout_get_pixel_size(layout, &width,&height);
	 if(centerX) x -= width/2;
	 if(centerY) y -= height/2;
	gdk_draw_layout (pixmap,gc,x,y,layout);
	g_object_unref (layout);
}
/**********************************************************************************************************************************/
void gabedit_save_image(GtkWidget* widget, gchar *fileName, gchar* type)
{       
	int width;
	int height;
	GError *error = NULL;
	GdkPixbuf  *pixbuf = NULL;

	width =  widget->allocation.width;
	height = widget->allocation.height;
	pixbuf = gdk_pixbuf_get_from_drawable(NULL, widget->window, NULL, 0, 0, 0, 0, width, height);
	/* printf("width = %d height = %d\n",width,height);*/
	if(pixbuf)
	{
		if(!fileName)
		{
			GtkClipboard * clipboard;
			clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
			if(clipboard)
			{
				gtk_clipboard_clear(clipboard);
				gtk_clipboard_set_image(clipboard, pixbuf);
			}
		}
		else 
		{
			if(type && strstr(type,"j") && strstr(type,"g") )
			gdk_pixbuf_save(pixbuf, fileName, type, &error, "quality", "100", NULL);
			else if(type && strstr(type,"png"))
			gdk_pixbuf_save(pixbuf, fileName, type, &error, "compression", "5", NULL);
			else if(type && (strstr(type,"tif") || strstr(type,"tiff")))
			gdk_pixbuf_save(pixbuf, fileName, "tiff", &error, "compression", "1", NULL);
			else
			gdk_pixbuf_save(pixbuf, fileName, type, &error, NULL);
		}
	 	g_object_unref (pixbuf);
	}
	/* else printf("Warnning pixbuf = NULL\n");*/
}
/**********************************************************************************************************************************/
G_CONST_RETURN gchar* get_open_babel_command()
{       
	return babelCommand;
}
/**********************************************************************************************************************************/
gint get_num_orbitals_from_aux_mopac_file(FILE* file, gchar* blockName,  gint* begin, gint* end)
{
	gchar t[BBSIZE];
	*begin = 0;
	*end = 0;
	 while(!feof(file))
	 {
		if(!fgets(t,BBSIZE,file))break;
		if(strstr( t, blockName))
		{
			gchar* pdest = strstr( t, "=")+1;
			gint i = sscanf(pdest,"%d %d",begin,end);
			return i;
		}
	 }
	 return 0;
}
/**********************************************************************************************************************************/
gchar** get_one_block_from_aux_mopac_file(FILE* file, gchar* blockName,  gint* n)
{
	gint nElements = 0;
	gchar** elements = NULL;
	gchar t[BBSIZE];
	 while(!feof(file))
	 {
		if(!fgets(t,BBSIZE,file))break;
		if(strstr( t, blockName))
		{
			gchar* pdest = strstr( t, "[")+1;
			gint i;
			nElements = atoi(pdest);
			if(nElements<1) break;
			else
			{ 
				long int geomposok = ftell(file);
				if(!fgets(t,BBSIZE,file))break;
				if(!strstr(t,"# ")) fseek(file, geomposok, SEEK_SET);
			}

			elements = g_malloc(nElements*sizeof(gchar*));
			for(i=0;i<nElements;i++)
			{
				gint k;
				elements[i] = g_malloc(100*sizeof(gchar));
				k = fscanf(file,"%s",elements[i]);
				if(k<1 || strstr(elements[i],"["))
				{
					if(elements)
					{
						for(i=0;i<nElements;i++)
							if(elements[i]) g_free(elements[i]);
						g_free(elements);
						elements = NULL;
					}
					break;
				}
				else
				{
					if(!strstr(blockName,"ATOM_EL"))
					for(k=0;k<strlen(elements[i]);k++)
					{
						if(elements[i][k]=='D') elements[i][k]='e';
						if(elements[i][k]=='d') elements[i][k]='e';
					}
				}
			}
			break;
		}
	 }
	 *n = nElements;
	 return elements;
}
/**********************************************************************************************************************************/
gchar** free_one_string_table(gchar** table, gint n)
{
	if(table)
	{
		gint i;
		for(i=0;i<n;i++)
			if(table[i]) g_free(table[i]);
		g_free(table);
	}
	return NULL;
}
/********************************************************************************/
gboolean zmat_mopac_irc_output_file(gchar *FileName)
{
 	guint taille=BBSIZE;
  	gchar t[BBSIZE];
 	FILE* fd = FOpen(FileName, "rb");

	if(!fd) return FALSE;
  	while(!feof(fd) )
	{
    		if(!fgets(t,taille,fd)) break;
                 if(strstr(t,"INTRINSIC REACTION COORDINATE"))
		{
			return TRUE;
		}
	}
	return FALSE;
}
/********************************************************************************/
gboolean zmat_mopac_scan_output_file(gchar *FileName)
{
 	guint taille=BBSIZE;
  	gchar t[BBSIZE];
 	FILE* fd = FOpen(FileName, "rb");

	if(!fd) return FALSE;
  	while(!feof(fd) )
	{
    		if(!fgets(t,taille,fd)) break;
		if ( strstr(t,"ATOM")
		 && strstr(t,"CHEMICAL")
		 && strstr(t,"BOND")
		 && strstr(t,"LENGTH")
		 && strstr(t,"ANGLE")
		 && strstr(t,"TWIST")
		 )
		{
			return TRUE;
		}
	}
	return FALSE;
}
/*************************************************************************************/
gchar *get_extenssion_file(const gchar* filename)
{
	gchar *ext = NULL;
	gint len = 0;
	gint i;
	gint p=-1;
	gint lnew = 0;

	if(!filename || strlen(filename)<1) return NULL;
	len=strlen(filename);

	for(i=len;i>0;i--)
	if(filename[i]=='.')
	{
		p=i+1;
		break;
	}
	lnew  = len-p;
	if(p<0 || lnew<=0) return NULL;
	ext= g_malloc((lnew+1)*sizeof(gchar));

	for(i=p;i<len;i++) ext[i-p]= filename[i];
	ext[lnew]= '\0';
	return ext;
}
/**********************************************/
GabEditTypeFile get_type_output_file(gchar* fileName)
{
 	gchar *t;
 	guint taille=BBSIZE;
	GabEditTypeFile ktype = GABEDIT_TYPEFILE_UNKNOWN;
	FILE* file = FOpen(fileName, "rb");

 	if(!file) return ktype;

 	t=g_malloc(taille*sizeof(gchar));

	rewind(file);
    	if(!feof(file)) { char* e = fgets(t,taille,file);}
	uppercase(t);
        if(strstr(t, "ENTERING" )) ktype = GABEDIT_TYPEFILE_GAUSSIAN;
	else if(strstr( t, "[MOLDEN FORMAT]" )) ktype = GABEDIT_TYPEFILE_MOLDEN;
	else if(strstr( t, "[GABEDIT FORMAT]" )) ktype = GABEDIT_TYPEFILE_GABEDIT;
	else if(strstr( t, "GAMESS" )) ktype = GABEDIT_TYPEFILE_GAMESS;
	else if(atoi(t)>0 && !strstr(t,"**********")) ktype = GABEDIT_TYPEFILE_XYZ;
	if( ktype == GABEDIT_TYPEFILE_UNKNOWN)
	{
		while(!feof(file))
		{
    			if(!feof(file)) { char* e = fgets(t,taille,file);}
			if(strstr(t,"PROGRAM SYSTEM MOLPRO"))
			{
				ktype = GABEDIT_TYPEFILE_MOLPRO;
				break;
			}
			if(strstr(t,"GAMESS VERSION") || strstr(t,"PC GAMESS"))
			{
				ktype = GABEDIT_TYPEFILE_GAMESS;
				break;
			}
			if(strstr(t,"PROGRAM deMon2k"))
			{
				ktype = GABEDIT_TYPEFILE_DEMON;
				break;
			}
			if(strstr(t,"GAMESS VERSION") || strstr(t,"PC GAMESS"))
			{
				ktype = GABEDIT_TYPEFILE_GAMESS;
				break;
			}
			if(strstr(t,"Welcome to Q-Chem"))
			{
				ktype = GABEDIT_TYPEFILE_QCHEM;
				break;
			}
			if(strstr(t,"Northwest Computational Chemistry Package"))
			{
				ktype = GABEDIT_TYPEFILE_NWCHEM;
				break;
			}
			if(strstr(t,"TURBOMOLE GmbH"))
			{
				ktype = GABEDIT_TYPEFILE_TURBOMOLE;
				break;
			}
			uppercase(t);
        		if(strstr(t, "ENTERING GAUSSIAN" ))
			{
				ktype = GABEDIT_TYPEFILE_GAUSSIAN;
				break;
			}
			if(strstr( t, "[MOLDEN FORMAT]" ))
			{ 
				ktype = GABEDIT_TYPEFILE_MOLDEN;
				break;
			}
			if(mystrcasestr( t, "<Title>"))
			{ 
				ktype = GABEDIT_TYPEFILE_WFX;
				break;
			}
		}
	}
	rewind(file);
	if( ktype == GABEDIT_TYPEFILE_GAUSSIAN)
	{
		gint iGrad = 0;
  		while(!feof(file) )    
  		{
 			if(!fgets(t, taille, file))break;
			/* fprintf(stderr,"%s\n",t);*/
			
                 	if(strstr(t,"GradGradGradGradGradGradGradGradGrad") )
			{
				iGrad++;
				if(iGrad>1) break;
			}
                 	if(strstr(t,"Scan        ") )
			{
				if(iGrad==1) ktype = GABEDIT_TYPEFILE_GAUSSIAN_SCANOPT;
				/* if(ktype==GABEDIT_TYPEFILE_GAUSSIAN_SCANOPT) fprintf(stderr,"GABEDIT_TYPEFILE_GAUSSIAN_SCANOPT\n");*/
				break;
			}
                 	if(strstr(t,"IRC-IRC-IRC-IRC-IRC-IRC-IRC-IRC-IRC-IRC-IRC-IRC") )
			{
				ktype = GABEDIT_TYPEFILE_GAUSSIAN_IRCOPT;
				break;
			}
                 	if(strstr(t," orientation:") ) break;
		}
	}
	if( ktype != GABEDIT_TYPEFILE_UNKNOWN)
	{
 		g_free(t);
		fclose(file);
		return ktype;
	}
	if( ktype == GABEDIT_TYPEFILE_UNKNOWN)
	{
		rewind	(file);
		while(!feof(file))
		{
    			if(!feof(file)) { char* e = fgets(t,taille,file);}
			if(mystrcasestr( t, "<Title>"))
			{ 
				ktype = GABEDIT_TYPEFILE_WFX;
				break;
			}
		}
	}
	rewind(file);
	if( ktype == GABEDIT_TYPEFILE_UNKNOWN)
	{
		while(!feof(file))
		{
    			if(!feof(file)) { char* e = fgets(t,taille,file);}
			if(strstr(t,"* O   R   C   A *"))
			{
				ktype = GABEDIT_TYPEFILE_ORCA;
				break;
			}
		}
	}
	rewind(file);
	if( ktype == GABEDIT_TYPEFILE_UNKNOWN)
	{
		while(!feof(file))
		{
    			if(!feof(file)) { char* e = fgets(t,taille,file);}
			if(strstr(t,"VASP"))
			{
				ktype = GABEDIT_TYPEFILE_VASPOUTCAR;
				break;
			}
		}
	}
	rewind(file);
	if( ktype == GABEDIT_TYPEFILE_UNKNOWN)
	{
		while(!feof(file))
		{
    			if(!feof(file)) { char* e = fgets(t,taille,file);}
			if(strstr(t,"GAMESS"))
			{
    				if(!feof(file)) { char* e = fgets(t,taille,file);}
				if(strstr(t,"FROM IOWA STATE UNIVERSITY"))
				ktype = GABEDIT_TYPEFILE_GAMESS;
				break;
			}
		}
	}
	rewind(file);
	if( ktype == GABEDIT_TYPEFILE_UNKNOWN)
	{
    		if(!feof(file)) { char* e = fgets(t,taille,file);}
		if(strstr(t,"START OF MOPAC FILE"))
			ktype = GABEDIT_TYPEFILE_MOPAC_AUX;
	}
	rewind(file);
	if( ktype == GABEDIT_TYPEFILE_UNKNOWN)
	{
  		while(!feof(file) )    
  		{
 			if(!fgets(t, taille, file))break;
			if( strstr(t,"VARIABLE") && strstr(t,"FUNCTION")) 
			{
				ktype = GABEDIT_TYPEFILE_MOPAC_SCAN;
				break;
			}
		}
	}
	rewind(file);
	if( ktype == GABEDIT_TYPEFILE_UNKNOWN)
	{
  		while(!feof(file) )    
  		{
 			if(!fgets(t, taille, file))break;
                 	if(strstr(t,"INTRINSIC REACTION COORDINATE") )
			{
				ktype = GABEDIT_TYPEFILE_MOPAC_IRC;
				break;
			}
		}
	}

 	g_free(t);
	fclose(file);
	return ktype;
}
/**********************************************/
GabEditTypeFile get_type_input_file(gchar* fileName)
{
	FILE* file = FOpen(fileName, "rb");
	if(test_type_program_orca(file))
	{
		fseek(file, 0L, SEEK_SET);
		fclose(file);
		return GABEDIT_TYPEFILE_ORCAINPUT;
	}
	if(test_type_program_firefly(file))
	{
		fseek(file, 0L, SEEK_SET);
		fclose(file);
		return GABEDIT_TYPEFILE_FIREFLYINPUT;
	}
	if(test_type_program_gamess(file))
	{
		fseek(file, 0L, SEEK_SET);
		fclose(file);
		return GABEDIT_TYPEFILE_GAMESSINPUT;
	}
	if(test_type_program_qchem(file))
	{
		fseek(file, 0L, SEEK_SET);
		fclose(file);
		return GABEDIT_TYPEFILE_QCHEMINPUT;
	}
	if(test_type_program_mopac(file))
	{
		fseek(file, 0L, SEEK_SET);
		fclose(file);
		return GABEDIT_TYPEFILE_MOPACINPUT;
	}
	if(test_type_program_mpqc(file))
	{
		fseek(file, 0L, SEEK_SET);
		fclose(file);
		return GABEDIT_TYPEFILE_MPQCINPUT;
	}
	if(test_type_program_gaussian(file))
	{
		fseek(file, 0L, SEEK_SET);
		fclose(file);
		return GABEDIT_TYPEFILE_GAUSSIANINPUT;
	}
	if(test_type_program_molcas(file))
	{
		fseek(file, 0L, SEEK_SET);
		fclose(file);
		return GABEDIT_TYPEFILE_MOLCASINPUT;
	}
	if(test_type_program_molpro(file))
	{
		fseek(file, 0L, SEEK_SET);
		fclose(file);
		return GABEDIT_TYPEFILE_MOLPROINPUT;
	}
	fseek(file, 0L, SEEK_SET);
	fclose(file);
	return GABEDIT_TYPEFILE_UNKNOWN;
}
/************************************************/
GabEditTypeFile get_type_file(gchar* filename)
{
	gchar* ext = NULL;
	if(!filename) return GABEDIT_TYPEFILE_UNKNOWN;
	ext = get_extenssion_file(filename);
	if(!ext) return GABEDIT_TYPEFILE_UNKNOWN;
	uppercase(ext);
	if( !strcmp(ext,"INP") || !strcmp(ext,"COM") || !strcmp(ext,"IN") || !strcmp(ext,"MOP") ) 
		return get_type_input_file(filename);
	if( !strcmp(ext,"OUT") || !strcmp(ext,"LOG") ) 
		return get_type_output_file(filename);
	if( !strcmp(ext,"MOL2")) return GABEDIT_TYPEFILE_MOL2;
	if( !strcmp(ext,"XYZ")) return GABEDIT_TYPEFILE_XYZ;
	if( !strcmp(ext,"PDB")) return GABEDIT_TYPEFILE_PDB;
	if( !strcmp(ext,"GZMT")) return GABEDIT_TYPEFILE_GZMAT;
	if( !strcmp(ext,"ZMT")) return GABEDIT_TYPEFILE_MZMAT;
	if( !strcmp(ext,"HIN")) return GABEDIT_TYPEFILE_HIN;
	if( !strcmp(ext,"TNK")) return GABEDIT_TYPEFILE_TINKER;
	if( !strcmp(ext,"GAB")) return GABEDIT_TYPEFILE_GABEDIT;
	if( !strcmp(ext,"MOLDEN")) return GABEDIT_TYPEFILE_MOLDEN;
	if( !strcmp(ext,"MFJ")) return GABEDIT_TYPEFILE_MOBCAL;
	if( !strcmp(ext,"AUX")) return GABEDIT_TYPEFILE_MOPAC_AUX;
	if( !strcmp(ext,"WFX")) return GABEDIT_TYPEFILE_WFX;
	if( !strcmp(ext,"JPG")) return GABEDIT_TYPEFILE_JPEG;
	if( !strcmp(ext,"JPEG")) return GABEDIT_TYPEFILE_JPEG;
	if( !strcmp(ext,"PPM")) return GABEDIT_TYPEFILE_PPM;
	if( !strcmp(ext,"BMP")) return GABEDIT_TYPEFILE_BMP;
	if( !strcmp(ext,"PNG")) return GABEDIT_TYPEFILE_PNG;
	if( !strcmp(ext,"PS")) return GABEDIT_TYPEFILE_PS;
	if( !strcmp(ext,"T41")) return GABEDIT_TYPEFILE_CUBEADF;
	if( !strcmp(ext,"GRID")) return GABEDIT_TYPEFILE_CUBEMOLCAS;
	if( !strcmp(ext,"HF")) return GABEDIT_TYPEFILE_CUBEQCHEM;
	if( !strcmp(ext,"GCUBE")) return GABEDIT_TYPEFILE_CUBEGABEDIT;
	if( !strcmp(ext,"TRJ")) return GABEDIT_TYPEFILE_TRJ;
	if( !strcmp(ext,"TXT")) return GABEDIT_TYPEFILE_TXT;
	if( !strcmp(ext,"IRC")) return GABEDIT_TYPEFILE_GAMESSIRC;
	if( !strcmp(ext,"CUBE") &&  !strcmp(filename,"CUBE")) return GABEDIT_TYPEFILE_CUBEMOLPRO;
	if( !strcmp(ext,"CUBE")) return GABEDIT_TYPEFILE_CUBEGAUSS;
	if( !strcmp(ext,"FCHK")) return GABEDIT_TYPEFILE_GAUSSIAN_FCHK;

	return GABEDIT_TYPEFILE_UNKNOWN;
}
/************************************************/
gchar * mystrcasestr(G_CONST_RETURN gchar *haystack, G_CONST_RETURN gchar *needle)
{
	gchar *i, *startn = 0, *j = 0;
	for (i = (gchar*)haystack; *i; i++)
	{
		if(j)
		{
			if (toupper(*i) == toupper(*j))
			{
				if (!*++j)
				return startn;
			}
			else j = 0;
		}
		else if (toupper(*i) == toupper(*needle))
		{
			j = (gchar*)needle + 1;
			startn = i;
		}
	}
	return 0;
}
/****************************************************************************/
gint get_one_int_from_fchk_gaussian_file(FILE* file, gchar* blockName)
{
	gint ipos = 47;
	gchar t[BBSIZE];
	 while(!feof(file))
	 {
		if(!fgets(t,BBSIZE,file))break;
		if(strstr( t, blockName))
		{
			if(strlen(t)>ipos+1) return atoi(t+ipos);
			return -1;
		}
	 }
	 return -1;
}
/****************************************************************************/
gdouble get_one_real_from_fchk_gaussian_file(FILE* file, gchar* blockName)
{
	gint ipos = 47;
	gchar t[BBSIZE];
	 while(!feof(file))
	 {
		if(!fgets(t,BBSIZE,file))break;
		if(strstr( t, blockName))
		{
			if(strlen(t)>ipos+1) return atof(t+ipos);
			return -1;
		}
	 }
	 return -1;
}
/****************************************************************************/
gint* get_array_int_from_fchk_gaussian_file(FILE* file, gchar* blockName, gint* nElements)
{
	gint ipos = 43;
	gint i;
	gchar t[BBSIZE];
	gint* elements = NULL;
	*nElements = 0;
	 while(!feof(file))
	 {
		if(!fgets(t,BBSIZE,file))break;
		if(strstr( t, blockName))
		{
			if(!(strstr( t, blockName) && strstr(t,"N=") && strlen(strstr(t,"N="))>2)) return elements;
			if(strlen(t)>ipos+1 && t[ipos]!='I') return elements;
			*nElements = atof(strstr(t,"N=")+2);
			if(*nElements<1) return elements;
			elements = g_malloc(*nElements*sizeof(gint));
			for(i=0;i<*nElements;i++)
			{
				if(1!=fscanf(file,"%d",&elements[i])) break;
			}
			if(i!=*nElements)
			{
				*nElements = 0;
				g_free(elements);
				return NULL;
			}
			return elements;
		}
	 }
	 return elements;
}
/****************************************************************************/
gdouble* get_array_real_from_fchk_gaussian_file(FILE* file, gchar* blockName, gint* nElements)
{
	gint ipos = 43;
	gint i;
	gchar t[BBSIZE];
	gdouble* elements = NULL;

	*nElements = 0;
	 while(!feof(file))
	 {
		if(!fgets(t,BBSIZE,file))break;
		if(strstr( t, blockName))
		{
			if(!(strstr( t, blockName) && strstr(t,"N=") && strlen(strstr(t,"N="))>2)) return elements;
			if(strlen(t)>ipos+1 && t[ipos]!='R') return elements;
			*nElements = atof(strstr(t,"N=")+2);
			if(*nElements<1) return elements;
			elements = g_malloc(*nElements*sizeof(gdouble));
			for(i=0;i<*nElements;i++)
				if(1!=fscanf(file,"%lf",&elements[i])) break;
			if(i!=*nElements)
			{
				*nElements = 0;
				g_free(elements);
				return NULL;
			}
			return elements;
		}
	 }
	 return elements;
}
/****************************************************************************/
gchar** get_array_string_from_fchk_gaussian_file(FILE* file, gchar* blockName, gint* nElements)
{
	gint ipos = 43;
	gint i;
	gchar t[BBSIZE];
	gchar** elements = NULL;
	gchar type = ' ';

	*nElements = 0;
	 while(!feof(file))
	 {
		if(!fgets(t,BBSIZE,file))break;
		if(strstr( t, blockName))
		{
			if(!(strstr( t, blockName) && strstr(t,"N=") && strlen(strstr(t,"N="))>2)) return elements;
			if(strlen(t)>ipos+1 && t[ipos]=='C') type = 'C';
			if(strlen(t)>ipos+1 && t[ipos]=='H') type = 'H';
			if(type!='C' && type!='H') return elements;
			*nElements = atof(strstr(t,"N=")+2);
			if(*nElements<1) return elements;
			elements = g_malloc(*nElements*sizeof(gchar*));
			for(i=0;i<*nElements;i++) elements[i] = NULL;
			if(type=='C')
			for(i=0;i<*nElements;i++)
			{
				if(1!=fscanf(file,"%12s",t)) break;
				elements[i] = g_strdup(t);
			}
			else
			for(i=0;i<*nElements;i++)
			{
				if(1!=fscanf(file,"%8s",t)) break;
				elements[i] = g_strdup(t);
			}
			
			if(i!=*nElements)
			{
				*nElements = 0;
				g_free(elements);
				return NULL;
			}
			return elements;
		}
	 }
	 return elements;
}
/*************************************************************************************/
void getvScaleBond(gdouble r, gdouble Center1[], gdouble Center2[], gdouble vScal[])
{
	gint l;
	V3d cros;
	V3d sub;
	V3d C0={0,0,0};
	gdouble C10[3];
	gdouble C20[3];
	gdouble CC1[3];
	gdouble CC2[3];
  	for(l=0;l<3;l++) vScal[l] = r*0.5;
	for(l=0;l<3;l++) CC1[l] = Center1[l];
	for(l=0;l<3;l++) CC2[l] = Center2[l];
	v3d_sub(C0, CC1, C10);
	v3d_sub(C0, CC2, C20);
	v3d_cross(C10, C20, cros);
	v3d_sub(CC1, CC2, sub);
	v3d_cross(cros, sub, vScal);
	if(v3d_dot(vScal,vScal)!=0)
	{
		v3d_normal(vScal);
		v3d_scale(vScal, r*0.5);
	}
	else
	{
        	gdouble d = 0;
		gint j,k;
		/* printf("Warning vScal in getvScaleBond/Utils.c = 0\n");*/
		v3d_normal(sub);
/*      find an orthogonal vector to CC1-CC2 */
		k = 0;
        	for(j=1;j<3;j++) if(fabs(sub[k])>fabs(sub[j])) k = j;
        	for(j=0;j<3;j++) vScal[j] = -sub[k] * sub[j];
        	vScal[k] += 1.0;
		v3d_normal(vScal);
		v3d_scale(vScal, r*0.5);
	}
}
/*************************************************************************************/
void getPositionsRadiusBond3(gdouble r, gdouble Orig[], gdouble Ci[], gdouble Cj[], gdouble C11[], gdouble C12[],  gdouble C21[],  gdouble C22[], gdouble C31[],  gdouble C32[], gdouble radius[], gint type)
{
	gdouble s = 1.8;
	V3d vScal;
	gint k;

	for(k=0;k<3;k++) Ci[k] -= Orig[k];
	for(k=0;k<3;k++) Cj[k] -= Orig[k];

	getvScaleBond(r, Ci, Cj, vScal);
	if(type==0)
	{
		s = 2.8;
		radius[0] = r/4;
		radius[1] = r;
		radius[2] = r/4;
	}
	else
	{
		s = 2.8;
		radius[0] = r/2;
		radius[1] = r/2;
		radius[2] = r/2;
	}
	for(k=0;k<3;k++) C11[k] = Ci[k]-s*vScal[k];
	for(k=0;k<3;k++) C12[k] = Cj[k]-s*vScal[k];
	for(k=0;k<3;k++) C21[k] = Ci[k];
	for(k=0;k<3;k++) C22[k] = Cj[k];
	for(k=0;k<3;k++) C31[k] = Ci[k]+s*vScal[k];
	for(k=0;k<3;k++) C32[k] = Cj[k]+s*vScal[k];

	for(k=0;k<3;k++) Ci[k] += Orig[k];
	for(k=0;k<3;k++) Cj[k] += Orig[k];
	for(k=0;k<3;k++) C11[k] += Orig[k]; 
	for(k=0;k<3;k++) C12[k] += Orig[k]; 
	for(k=0;k<3;k++) C21[k] += Orig[k]; 
	for(k=0;k<3;k++) C22[k] += Orig[k]; 
	for(k=0;k<3;k++) C31[k] += Orig[k]; 
	for(k=0;k<3;k++) C32[k] += Orig[k]; 
}
/*************************************************************************************/
void getPositionsRadiusBond2(gdouble r, gdouble Orig[], gdouble Ci[], gdouble Cj[], gdouble C11[], gdouble C12[],  gdouble C21[],  gdouble C22[], gdouble radius[], gint type)
{
/* type=0=>stick, type=1=>ball&stick */
	gdouble s = 1.5;
	V3d vScal;
	gint k;

	for(k=0;k<3;k++) Ci[k] -= Orig[k];
	for(k=0;k<3;k++) Cj[k] -= Orig[k];
	getvScaleBond(r, Ci, Cj, vScal);
		
	radius[2] = 0;
	if(type==0)
	{
		s = 2.8;
		radius[0] = r/4;
		radius[1] = r;
		for(k=0;k<3;k++) C11[k] = Ci[k]-s*vScal[k];
		for(k=0;k<3;k++) C12[k] = Cj[k]-s*vScal[k];
		for(k=0;k<3;k++) C21[k] = Ci[k];
		for(k=0;k<3;k++) C22[k] = Cj[k];
	}
	else
	{
		s = 1.5;
		radius[0] = r/1.5;
		radius[1] = r/1.5;
		for(k=0;k<3;k++) C11[k] = Ci[k]-s*vScal[k];
		for(k=0;k<3;k++) C12[k] = Cj[k]-s*vScal[k];
		for(k=0;k<3;k++) C21[k] = Ci[k]+s*vScal[k];
		for(k=0;k<3;k++) C22[k] = Cj[k]+s*vScal[k];
	}
	for(k=0;k<3;k++) Ci[k] += Orig[k];
	for(k=0;k<3;k++) Cj[k] += Orig[k];
	for(k=0;k<3;k++) C11[k] += Orig[k]; 
	for(k=0;k<3;k++) C12[k] += Orig[k]; 
	for(k=0;k<3;k++) C21[k] += Orig[k]; 
	for(k=0;k<3;k++) C22[k] += Orig[k]; 
}
/*********************************************************************************************************************/
void getCoefsGradient(gint nBoundary, gdouble xh, gdouble yh, gdouble zh, gdouble* fcx, gdouble* fcy, gdouble* fcz)
{
	gdouble* coefs =  g_malloc((nBoundary)*sizeof(gdouble));
	gdouble xxh = 1.0;
	gdouble yyh = 1.0;
	gdouble zzh = 1.0;
	gint i;
	
	switch(nBoundary)
	{
		case 1:{
				gdouble denom = 2.0;
				gdouble c[] = {-1.0};
				for(i=0;i<nBoundary;i++) coefs[i] = c[i]/denom;
				break;
			}
		case 2:{
				gdouble denom =12.0;
				gdouble c[] = { 1.0, -8.0};
				for(i=0;i<nBoundary;i++) coefs[i] = c[i]/denom;
				break;
			}
		case 3:{
				gdouble denom =60.0;
				gdouble c[] = { -1.0, +9.0, -45.0};
				for(i=0;i<nBoundary;i++) coefs[i] = c[i]/denom;
				break;
			}
		case 4:{
				gdouble denom =840.0;
				gdouble c[] = { 3.0, -32.0, +168.0, -672.0};
				for(i=0;i<nBoundary;i++) coefs[i] = c[i]/denom;
				break;
			}
		case 5:{
				gdouble denom =2520.0 ;
				gdouble c[] = { -2.0, +25.0, -150.0,+600.0, -2100.0};
				for(i=0;i<nBoundary;i++) coefs[i] = c[i]/denom;
				break;
			}
		case 6:{
				gdouble denom =27720.0 ;
				gdouble c[] = { 5.0, -72.0, +495.0, -2200.0, +7425.0, -23760.0};
				for(i=0;i<nBoundary;i++) coefs[i] = c[i]/denom;
				break;
			}
		case 7:{
				gdouble denom =360360.0;
				gdouble c[] = { -15.0, +245.0, -1911.0, +9555.0, -35035.0, +105105.0, -315315.0};
				for(i=0;i<nBoundary;i++) coefs[i] = c[i]/denom;
				break;
			}
		case 8:{
				gdouble denom =720720.0;
				gdouble c[] = { 7.0, -128.0, +1120.0, -6272.0, +25480.0, -81536.0, +224224.0, -640640.0};
				for(i=0;i<nBoundary;i++) coefs[i] = c[i]/denom;
				break;
			}
	}

	xxh = 1.0 / (xh);
	yyh = 1.0 / (yh);
	zzh = 1.0 / (zh);

	for(i=0;i<nBoundary;i++)
	{
		fcx[i] =  xxh * coefs[i];
		fcy[i] =  yyh * coefs[i];
		fcz[i] =  zzh * coefs[i];
	}

	g_free(coefs);
}
/*********************************************************************************************************************************/
void getCoefsLaplacian(gint nBoundary, gdouble xh, gdouble yh, gdouble zh, gdouble* fcx, gdouble* fcy, gdouble* fcz, gdouble* cc)
{
	gdouble* coefs =  g_malloc((nBoundary+1)*sizeof(gdouble));
	gdouble x2h = 1.0 / (xh * xh);
	gdouble y2h = 1.0 / (yh * yh);
	gdouble z2h = 1.0 / (zh * zh);
	gint i;
	
	switch(nBoundary)
	{
		case 1:{
				gdouble c[] = {-2.0, 1.0};
				for(i=0;i<=nBoundary;i++)
					coefs[i] = c[i];
				break;
			}
		case 2:{
				gdouble denom = 12.0;
				gdouble c[] = {-30.0, 16.0, -1.0};
				for(i=0;i<=nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 3:{
				gdouble denom = 180.0;
				gdouble c[] = {-490.0, 270.0,-27.0, 2.0};
				for(i=0;i<=nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 4:{
				gdouble denom = 5040.0;
				gdouble c[] = {-14350.0, 8064.0, -1008.0, 128.0, -9.0};
				for(i=0;i<=nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 5:{
				gdouble denom = 25200.0;
				gdouble c[] = {-73766.0, 42000.0, -6000.0, 1000.0, -125.0, 8.0};
				for(i=0;i<=nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 6:{
				gdouble denom = 831600.0;
			 	gdouble c[] = {-2480478.0,1425600.0,-222750.0,44000.0,-7425.0,864.0,-50.0};
				for(i=0;i<=nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 7:{
				gdouble denom = 75675600.0;
				gdouble c[] = {-228812298.0,132432300.0,-22072050.0,4904900.0,-1003275.0, 160524.0,-17150.0,900.0};
				for(i=0;i<=nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 8:{
				gdouble denom = 302702400.0;
				gdouble c[] = {-924708642.0,538137600.0,-94174080.0,22830080.0,-5350800.0,1053696.0,-156800.0,15360.0,-735.0};
				for(i=0;i<=nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
	}

	*cc = x2h + y2h + z2h;
	*cc *= coefs[0];

	for(i=0;i<=nBoundary;i++)
	{
		fcx[i] =  x2h * coefs[i];
		fcy[i] =  y2h * coefs[i];
		fcz[i] =  z2h * coefs[i];
	}

	g_free(coefs);
}
/*********************************************************************************/
void swapDouble(gdouble* a, gdouble* b)
{
	gdouble c = *a;
	*a = *b;
	*b = c;
}
/****************************************************************************************************************************/
gdouble* newVectorDouble(gint n)
{
	gdouble* v = NULL; 
	if(n<1) return v;
	v = g_malloc(n*sizeof(gdouble));
	return v;
}
/****************************************************************************************************************************/
void initVectorDouble(gdouble* v, gint n, gdouble val)
{
	gint i;
	if(!v) return;
	for(i = 0;i<n; i++)  v[i] = val;
}
/****************************************************************************************************************************/
void freeVectorDouble(gdouble** v)
{
	if(*v) g_free(*v);
	*v= NULL;
}
/****************************************************************************************************************************/
void printVectorDoubleCutOff(gdouble* C, gint n, gdouble cutoff)
{
	gint i;
	if(!C) return;
	for(i=0;i<n;i++) if(fabs(C[i])>=cutoff) printf("%d %20.10f\n",i+1,C[i]);
}
/****************************************************************************************************************************/
gdouble** newMatrixDouble(gint nrows, gint ncolumns)
{
	
	gdouble** M  = NULL;
	if(nrows<1 || ncolumns<1) return M;
	M  = g_malloc(nrows*sizeof(gdouble*));
	gint i;
	for(i = 0;i<nrows; i++) 
		M[i] = g_malloc(ncolumns*sizeof(gdouble));
	return M;
}
/****************************************************************************************************************************/
void freeMatrixDouble(gdouble*** M, gint nrows)
{
	if(*M) 
	{
		gint i;
		for(i = 0;i<nrows; i++) 
			if((*M)[i])g_free((*M)[i]);
	}
	*M= NULL;
}
/****************************************************************************************************************************/
void initMatrixDouble(gdouble** M, gint nrows, gint ncolumns, gdouble val)
{
	gint i,j;
	if(!M) return;
	for(i = 0;i<nrows; i++) 
		for(j = 0;j<ncolumns; j++)  M[i][j]  = val;
}
/****************************************************************************************************************************/
void symmetrizeMatrixDouble(gdouble** M, gint nrows, gint ncolumns, gdouble cutOff)
{
	gint i,j;
	gdouble x,y;
	if(!M) return;
	for(i = 0;i<nrows; i++) 
		for(j = 0;j<i; j++)  
		{
			if(j>ncolumns-1) continue;
			x =  M[i][j];
			y =  M[j][i];
			if(fabs(x)>cutOff && fabs(y)>cutOff)  M[i][j] = M[j][i] = (x+y)/2;
			else if(fabs(x)>cutOff)  M[i][j] = M[j][i] = x;
			else if(fabs(y)>cutOff)  M[i][j] = M[j][i] = y;
			else M[i][j]  = 0.0;
		}
}
/****************************************************************************************************************************/
void printMatrixDouble(gdouble** M, gint nrows, gint ncolumns)
{
	gint i,j;
	for(i = 0;i<nrows; i++) 
	{
		for(j = 0;j<ncolumns; j++) 
      			printf("%f ",M[i][j]);
		printf("\n");
	}
}
/****************************************************************************************************************************/
void printMatrixDoubleCutOff(gdouble** M, gint nrows, gint ncolumns, gdouble cutoff)
{
	gint i,j;
	for(i = 0;i<nrows; i++) 
	{
		for(j = 0;j<ncolumns; j++) 
		if(fabs(M[i][j])>=cutoff)
      			printf("%d %d %20.10f\n",i+1,j+1,M[i][j]);
	}
}
/****************************************************************************************************************************/
gdouble*** newCubeDouble(gint nrows, gint ncolumns, gint nslices)
{
	gdouble*** C  = NULL;
	gint i,j;
	if(nrows<1 || ncolumns<1 || nslices<1) return C;
	C  = g_malloc(nrows*sizeof(gdouble**));
	for(i = 0;i<nrows; i++) 
	{
		C[i] = g_malloc(ncolumns*sizeof(gdouble*));
		for(j = 0;j<ncolumns; j++) 
			C[i][j] = g_malloc(nslices*sizeof(gdouble));
	}
	return C;
}
/****************************************************************************************************************************/
void printCubeDouble(gdouble*** C, gint nrows, gint ncolumns, gint nslices)
{
	gint i,j,k;
	for(i = 0;i<nrows; i++) 
	{
		for(j = 0;j<ncolumns; j++) 
		{
			for(k = 0;k<nslices; k++) 
      				printf("%f ",C[i][j][k]);
			printf("\n");
		}
		printf("\n");
	}
}
/****************************************************************************************************************************/
void printCubeDoubleCutOff(gdouble*** C, gint nrows, gint ncolumns, gint nslices, gdouble cutoff)
{
	gint i,j,k;
	for(i = 0;i<nrows; i++) 
	{
		for(j = 0;j<ncolumns; j++) 
		{
			for(k = 0;k<nslices; k++) 
			if(fabs(C[i][j][k])>=cutoff)
      				printf("%d %d %d %20.10f\n",i+1,j+1,k+1,C[i][j][k]);
		}
	}
}
/****************************************************************************************************************************/
static void symmetrizeCubeDoubleIJ(gdouble*** C, gint nrows, gint ncolumns,  gint nslices, gdouble cutOff)
{
	gint i,j,k;
	gdouble x,y;
	for(i = 0;i<nrows; i++) 
		for(j = 0;j<i; j++)  
		{
			if(j>ncolumns-1) continue;
			for(k = 0;k<nslices; k++)  
			{
				x =  C[i][j][k];
				y =  C[j][i][k];
				if(fabs(x)>cutOff && fabs(y)>cutOff)  C[i][j][k] = C[j][i][k] = (x+y)/2;
				else if(fabs(x)>cutOff)  C[i][j][k] = C[j][i][k] = x;
				else if(fabs(y)>cutOff)  C[i][j][k] = C[j][i][k] = y;
				else C[i][j][k]  = 0.0;
			}
		}
}
/****************************************************************************************************************************/
static void symmetrizeCubeDoubleJK(gdouble*** C, gint nrows, gint ncolumns,  gint nslices, gdouble cutOff)
{
	gint i;
	for(i=0;i<nrows;i++) symmetrizeMatrixDouble(C[i], ncolumns, nslices,  cutOff);
}
/****************************************************************************************************************************/
void symmetrizeCubeDouble(gdouble*** C, gint nrows, gint ncolumns,  gint nslices, gdouble cutOff)
{
	symmetrizeCubeDoubleIJ(C, nrows, ncolumns,  nslices, cutOff);
	symmetrizeCubeDoubleJK(C, nrows, ncolumns,  nslices, cutOff);
	symmetrizeCubeDoubleIJ(C, nrows, ncolumns,  nslices, cutOff);
}
/****************************************************************************************************************************/
void initCubeDouble(gdouble*** C, gint nrows, gint ncolumns, gint nslices, gdouble val)
{
	gint i,j,k;
	if(!C) return;
	for(i = 0;i<nrows; i++) 
		for(j = 0;j<ncolumns; j++) 
			for(k = 0;k<nslices; k++) C[i][j][k] = val;
}
/****************************************************************************************************************************/
void freeCubeDouble(gdouble**** C, gint nrows, gint ncolumns)
{
	if(*C) 
	{
		gint i,j;
		for(i = 0;i<nrows; i++) 
		{
			if((*C)[i])
			for(j = 0;j<ncolumns; j++) 
				if((*C)[i][j]) g_free((*C)[i][j]);
	
			if((*C)[i])g_free((*C)[i]);
		}
	}
	*C= NULL;
}
/****************************************************************************************************************************/
gdouble**** newQuarticDouble(gint nrows, gint ncolumns, gint nslices, gint nl)
{
	gdouble**** C  = NULL;
	gint i,j,k;
	if(nrows<1 || ncolumns<1 || nslices<1) return C;
	C  = g_malloc(nrows*sizeof(gdouble***));
	for(i = 0;i<nrows; i++) 
	{
		C[i] = g_malloc(ncolumns*sizeof(gdouble**));
		for(j = 0;j<ncolumns; j++) 
		{
			C[i][j] = g_malloc(nslices*sizeof(gdouble*));
			for(k = 0;k<nslices; k++) 
			C[i][j][k] = g_malloc(nslices*sizeof(gdouble));
		}
	}
	return C;
}
/****************************************************************************************************************************/
void printQuarticDouble(gdouble**** C, gint nrows, gint ncolumns, gint nslices, gint nl)
{
	gint i,j,k,l;
	for(i = 0;i<nrows; i++) 
	{
		for(j = 0;j<ncolumns; j++) 
		{
			for(k = 0;k<nslices; k++) 
			{
				for(l = 0;l<nl; l++) 
      					printf("%f ",C[i][j][k][l]);
				printf("\n");
			}
		}
		printf("\n");
	}
}
/****************************************************************************************************************************/
void printQuarticDoubleCutOff(gdouble**** C, gint nrows, gint ncolumns, gint nslices, gint nl, gdouble cutoff)
{
	gint i,j,k,l;
	for(i = 0;i<nrows; i++) 
	{
		for(j = 0;j<ncolumns; j++) 
		{
			for(k = 0;k<nslices; k++) 
			{
				for(l = 0;l<nl; l++) 
				if(fabs(C[i][j][k][l])>=cutoff)
      					printf("%d %d %d %d %20.10f\n",i+1,j+1,k+1,l+1,C[i][j][k][l]);
			}
		}
	}
}
/****************************************************************************************************************************/
void initQuarticDouble(gdouble**** C, gint nrows, gint ncolumns, gint nslices, gint nl, gdouble val)
{
	gint i,j,k,l;
	if(!C) return;
	for(i = 0;i<nrows; i++) 
		for(j = 0;j<ncolumns; j++) 
			for(k = 0;k<nslices; k++) 
				for(l = 0;l<nl; l++) 
					C[i][j][k][l] = val;
}
/****************************************************************************************************************************/
void freeQuarticDouble(gdouble***** C, gint nrows, gint ncolumns, gint nl)
{
	if(*C) 
	{
		gint i,j,k;
		for(i = 0;i<nrows; i++) 
		{
			if((*C)[i])
			for(j = 0;j<ncolumns; j++) 
			{
				if((*C)[i][j])
				{
				for(k = 0;k<nl; k++) 
					if((*C)[i][j][k]) g_free((*C)[i][j][k]);
				if((*C)[i][j]) g_free((*C)[i][j]);
				}
			}
			if((*C)[i])g_free((*C)[i]);
		}
		g_free(*C);
	}
	*C= NULL;
}
/****************************************************************************************************************************/
static void symmetrizeQuarticDoubleIJ(gdouble**** Q, gint nrows, gint ncolumns,  gint nslices, gint nq, gdouble cutOff)
{
	gint i,j,k,l;
	gdouble x,y;
	for(i = 0;i<nrows; i++) 
		for(j = 0;j<i; j++)  
		{
			if(j>ncolumns-1) continue;
			for(k = 0;k<nslices; k++)  
			for(l = 0;l<nq; l++)  
			{
				x =  Q[i][j][k][l];
				y =  Q[j][i][k][l];
				if(fabs(x)>cutOff && fabs(y)>cutOff)  Q[i][j][k][l] = Q[j][i][k][l] = (x+y)/2;
				else if(fabs(x)>cutOff)  Q[i][j][k][l] = Q[j][i][k][l] = x;
				else if(fabs(y)>cutOff)  Q[i][j][k][l] = Q[j][i][k][l] = y;
				else Q[i][j][k][l]  = 0.0;
			}
		}
}
/****************************************************************************************************************************/
static void symmetrizeQuarticDoubleJKL(gdouble**** Q, gint nrows, gint ncolumns,  gint nslices, gint nq, gdouble cutOff)
{
	gint i;
	for(i=0;i<nrows;i++) symmetrizeCubeDouble(Q[i], ncolumns, nslices, nq,  cutOff);
}
/****************************************************************************************************************************/
void symmetrizeQuarticDouble(gdouble**** Q, gint nrows, gint ncolumns,  gint nslices, gint nq, gdouble cutOff)
{
	symmetrizeQuarticDoubleIJ(Q, nrows, ncolumns,  nslices, nq, cutOff);
	symmetrizeQuarticDoubleJKL(Q, nrows, ncolumns,  nslices, nq, cutOff);
	symmetrizeQuarticDoubleIJ(Q, nrows, ncolumns,  nslices, nq, cutOff);
}
/****************************************************************************************************************************/
gint* newVectorInt(gint n)
{
	gint* v = NULL;
	if(n<1) return v;
	v = g_malloc(n*sizeof(gint));
	return v;
}
/****************************************************************************************************************************/
void initVectorInt(gint* v, gint n, gint val)
{
	gint i;
	if(!v) return;
	for(i = 0;i<n; i++)  v[i] = val;
}
/****************************************************************************************************************************/
void freeVectorInt(gint** v)
{
	if(*v) g_free(*v);
	*v= NULL;
}
/****************************************************************************************************************************/
gint** newMatrixInt(gint nrows, gint ncolumns)
{
	gint** M  = NULL;
	if(nrows<1 || ncolumns<1) return M;
	M  = g_malloc(nrows*sizeof(gint*));
	gint i;
	for(i = 0;i<nrows; i++) 
		M[i] = g_malloc(ncolumns*sizeof(gint));
	return M;
}
/****************************************************************************************************************************/
void initMatrixInt(gint** M, gint nrows, gint ncolumns, gint val)
{
	gint i,j;
	if(!M) return;
	for(i = 0;i<nrows; i++) 
		for(j = 0;j<ncolumns; j++)  M[i][j]  = val;
}
/****************************************************************************************************************************/
void freeMatrixInt(gint*** M, gint nrows)
{
	if(*M) 
	{
		gint i;
		for(i = 0;i<nrows; i++) 
			if((*M)[i])g_free((*M)[i]);
	}
	*M= NULL;
}
/****************************************************************************************************************************/
gint*** newCubeInt(gint nrows, gint ncolumns, gint nslices)
{
	gint*** C  = NULL;
	gint i,j;
	if(nrows<1 || ncolumns<1 || nslices<1) return C;
	C  = g_malloc(nrows*sizeof(gint**));
	for(i = 0;i<nrows; i++) 
	{
		C[i] = g_malloc(ncolumns*sizeof(gint*));
		for(j = 0;j<ncolumns; j++) 
			C[i][j] = g_malloc(nslices*sizeof(gint));
	}
	return C;
}
/****************************************************************************************************************************/
void initCubeInt(gint*** C, gint nrows, gint ncolumns, gint nslices, gint val)
{
	gint i,j,k;
	if(!C) return;
	for(i = 0;i<nrows; i++) 
		for(j = 0;j<ncolumns; j++) 
			for(k = 0;k<nslices; k++) C[i][j][k] = val;
}
/****************************************************************************************************************************/
void freeCubeInt(gint**** C, gint nrows, gint ncolumns)
{
	if(*C) 
	{
		gint i,j;
		for(i = 0;i<nrows; i++) 
		{
			if((*C)[i])
			for(j = 0;j<ncolumns; j++) 
				if((*C)[i][j]) g_free((*C)[i][j]);
	
			if((*C)[i])g_free((*C)[i]);
		}
	}
	*C= NULL;
}
/****************************************************************************************************************************/
gint**** newQuarticInt(gint nrows, gint ncolumns, gint nslices, gint nl)
{
	gint**** C  = NULL;
	gint i,j,k;
	if(nrows<1 || ncolumns<1 || nslices<1) return C;
	C  = g_malloc(nrows*sizeof(gint***));
	for(i = 0;i<nrows; i++) 
	{
		C[i] = g_malloc(ncolumns*sizeof(gint**));
		for(j = 0;j<ncolumns; j++) 
		{
			C[i][j] = g_malloc(nslices*sizeof(gint*));
			for(k = 0;k<nslices; k++) 
			C[i][j][k] = g_malloc(nslices*sizeof(gint));
		}
	}
	return C;
}
/****************************************************************************************************************************/
void printQuarticInt(gint**** C, gint nrows, gint ncolumns, gint nslices, gint nl)
{
	gint i,j,k,l;
	for(i = 0;i<nrows; i++) 
	{
		for(j = 0;j<ncolumns; j++) 
		{
			for(k = 0;k<nslices; k++) 
			{
				for(l = 0;l<nl; l++) 
      					printf("%d ",C[i][j][k][l]);
				printf("\n");
			}
		}
		printf("\n");
	}
}
/****************************************************************************************************************************/
void initQuarticInt(gint**** C, gint nrows, gint ncolumns, gint nslices, gint nl, gint val)
{
	gint i,j,k,l;
	if(!C) return;
	for(i = 0;i<nrows; i++) 
		for(j = 0;j<ncolumns; j++) 
			for(k = 0;k<nslices; k++) 
				for(l = 0;l<nl; l++) 
					C[i][j][k][l] = val;
}
/****************************************************************************************************************************/
void freeQuarticInt(gint***** C, gint nrows, gint ncolumns, gint nslices)
{
	if(*C) 
	{
		gint i,j,k;
		for(i = 0;i<nrows; i++) 
		{
			if((*C)[i])
			for(j = 0;j<ncolumns; j++) 
			{
				if((*C)[i][j])
				{
				for(k = 0;k<nslices; k++) 
					if((*C)[i][j][k]) g_free((*C)[i][j][k]);
				if((*C)[i][j]) g_free((*C)[i][j]);
				}
			}
			if((*C)[i])g_free((*C)[i]);
		}
		g_free(*C);
	}
	*C= NULL;
}
/****************************************************************************************************************************/
gboolean readOneReal(FILE* file, gchar* tag, double*value)
{
	static gchar *t = NULL; 
	gchar* TAG = NULL;
	gchar* pos;
	if(!tag) return FALSE;
	if(!value) return FALSE;
	if(t==NULL) t = g_malloc(BSIZE*sizeof(gchar));

	TAG = strdup(tag);
	uppercase(TAG);
	rewind(file);

	while(!feof(file))
  	{
    		if(!fgets(t,BSIZE, file)) break;
		deleteFirstSpaces(t);
		if(t[0]=='#') continue;
		uppercase(t);
		pos = strstr(t,TAG);
		if(!pos) continue;
		if(strstr(pos,"=")) 
		{
			pos = strstr(pos,"=")+1;
		}
		else pos += strlen(TAG)+1;
		g_free(TAG);
		if(1==sscanf(pos,"%lf",value)) return TRUE;
		return FALSE;
	}
	g_free(TAG);
	return FALSE;
}
/****************************************************************************************************************************/
gboolean readOneRealFromAFile(gchar* namefile, gchar* tag, double* value)
{
	FILE* file = NULL;
	gboolean res;

	if(!namefile) return FALSE;

	file = fopen(namefile, "rb");
	res = readOneReal(file,tag,value);
	fclose(file);
	return res;
}
/****************************************************************************************************************************/
gboolean readOneInt(FILE* file, gchar* tag, gint*value)
{
	static gchar *t = NULL; 
	gchar* TAG = NULL;
	gchar* pos;
	if(!tag) return FALSE;
	if(!value) return FALSE;
	if(t==NULL) t = g_malloc(BSIZE*sizeof(gchar));

	TAG = strdup(tag);
	uppercase(TAG);
	rewind(file);

	while(!feof(file))
  	{
    		if(!fgets(t,BSIZE, file)) break;
		deleteFirstSpaces(t);
		if(t[0]=='#') continue;
		uppercase(t);
		pos = strstr(t,TAG);
		if(!pos) continue;
		if(strstr(pos,"=")) 
		{
			pos = strstr(pos,"=")+1;
		}
		else pos += strlen(TAG)+1;
		g_free(TAG);
		if(1==sscanf(pos,"%d",value)) return TRUE;
		return FALSE;
	}
	g_free(TAG);
	return FALSE;
}
/****************************************************************************************************************************/
gboolean readOneIntFromAFile(gchar* namefile, gchar* tag, gint* value)
{
	FILE* file = NULL;
	gboolean res;

	if(!namefile) return FALSE;

	file = fopen(namefile, "rb");
	res = readOneInt(file,tag,value);
	fclose(file);
	return res;
}
/****************************************************************************************************************************/
gboolean readOneBoolean(FILE* file, gchar* tag, gboolean*value)
{
	static gchar *t = NULL; 
	gchar* TAG = NULL;
	gchar* pos;
	gchar tmp[100];
	if(!tag) return FALSE;
	if(!value) return FALSE;
	if(t==NULL) t = g_malloc(BSIZE*sizeof(gchar));

	TAG = strdup(tag);
	uppercase(TAG);
	rewind(file);

	while(!feof(file))
  	{
    		if(!fgets(t,BSIZE, file)) break;
		deleteFirstSpaces(t);
		if(t[0]=='#') continue;
		uppercase(t);
		pos = strstr(t,TAG);
		if(!pos) continue;
		if(strstr(pos,"=")) 
		{
			pos = strstr(pos,"=")+1;
		}
		else pos += strlen(TAG)+1;
		g_free(TAG);
		if(1==sscanf(pos,"%s",tmp)) 
		{
			
			if(!strcmp(tmp,"TRUE"))*value = TRUE;
			else *value = FALSE;
			return TRUE;
		}
		return FALSE;
	}
	g_free(TAG);
	return FALSE;
}
/****************************************************************************************************************************/
gboolean readOneBooleanFromAFile(gchar* namefile, gchar* tag, gboolean* value)
{
	FILE* file = NULL;
	gboolean res;

	if(!namefile) return FALSE;

	file = fopen(namefile, "rb");
	res = readOneBoolean(file,tag,value);
	fclose(file);
	return res;
}
/****************************************************************************************************************************/
gboolean readOneStringFromAFile(gchar* namefile, gchar* tag, gint* value)
{
	FILE* file = NULL;
	gboolean res;

	if(!namefile) return FALSE;

	file = fopen(namefile, "rb");
	res = readOneInt(file,tag,value);
	fclose(file);
	return res;
}
/****************************************************************************************************************************/
gboolean readOneString(FILE* file, gchar* tag, gchar**value)
{
	static gchar *t = NULL; 
	static gchar *t2 = NULL; 
	gchar* TAG = NULL;
	gchar* pos;
	if(!tag) return FALSE;
	if(!value) return FALSE;
	if(t==NULL) t = g_malloc(BSIZE*sizeof(gchar));
	if(t2==NULL) t2 = g_malloc((BSIZE+2)*sizeof(gchar));

	TAG = strdup(tag);
	uppercase(TAG);
	rewind(file);

	while(!feof(file))
  	{
    		if(!fgets(t,BSIZE, file)) break;
		deleteFirstSpaces(t);
		if(t[0]=='#') continue;
		sprintf(t2,"%s",t);
		uppercase(t2);
		pos = strstr(t2,TAG);
		if(!pos) continue;
		if(strstr(pos,"=")) 
		{
			pos = strstr(pos,"=")+1;
		}
		else pos += strlen(TAG)+1;
		g_free(TAG);
		if(strlen(pos)>0) 
		{
			gchar* p = t+(gint)(pos-t2);
			if(*value) g_free(*value);
			*value = strdup(p);
			strDeleten(*value);
			deleteFirstSpaces(*value);
			deleteLastSpaces(*value);
			return TRUE;
		}
		return FALSE;
	}
	g_free(TAG);
	return FALSE;
}
/****************************************************************************************************************************/
void strDeleten(gchar* str)
{
        gchar *s;

        if(str == NULL)
                return;

        if (!*str)
                return;
        for (s = str + strlen (str) - 1; s >= str && ((unsigned char)*s)=='\n'; s--)
                *s = '\0';
}
/****************************************************************************************************************************/
void deleteLastSpaces(gchar* str)
{
	gchar *s;

	if(str == NULL)
		return;

	if (!*str)
		return;
	for (s = str + strlen (str) - 1; s >= str && isspace ((unsigned char)*s); s--) *s = '\0';
}
/****************************************************************************************************************************/
void deleteFirstSpaces(gchar* str)
{
	gchar *start;
	gint i;
	gint lenSpace = 0;

	if(str == NULL)
		return;
	if (!*str)
		return;

	for (start = str; *start && isspace (*start); start++)lenSpace++;

	for(i=0;i<(gint)(strlen(str)-lenSpace);i++)
		str[i] = str[i+lenSpace];
	str[strlen(str)-lenSpace] = '\0';
}
/****************************************************************************************************************************/
void deleteAllSpaces(gchar* str)
{
	gint i;
	gint j;
	gboolean Ok = FALSE;

	deleteLastSpaces(str);
	deleteFirstSpaces(str);
	while(!Ok)
	{
		Ok = TRUE;
		for(i=0;i<(gint)strlen(str);i++)
		{
			if(isspace(str[i]))
			{
				Ok = FALSE;
				for(j=i;j<(gint)strlen(str);j++)
				{
					str[j] = str[j+1];
				}
				break;
			}
		}
	}
}
/****************************************************************************************************************************/
gboolean goToStr(FILE* file, gchar* tag)
{
        static gchar *t = NULL;
        gchar* TAG = NULL;
        gchar* pos = NULL;
        int i=0;
        int ii,jj;
        double v;
        int** counter = NULL;
        if(!tag) return FALSE;
        if(t==NULL) t = g_malloc(BSIZE*sizeof(gchar));

        TAG = strdup(tag);
        uppercase(TAG);
        rewind(file);

        while(!feof(file))
        {
                if(!fgets(t,BSIZE, file)) break;
                deleteFirstSpaces(t);
                if(t[0]=='#') continue;
                uppercase(t);
                pos = strstr(t,TAG);
                if(!pos) continue;
                break;
        }
	return pos != NULL;
}
/**********************************************************************************************************************************/
gchar** get_one_block_from_wfx_file(FILE* file, gchar* blockName,  gint* n)
{
	gint nElements = 0;
	gchar** elements = NULL;
	gchar t[BBSIZE];
	long int geomposok = 0;
	gboolean ok = FALSE;
	gint i;
	 while(!feof(file))
	 {
		if(!fgets(t,BBSIZE,file))break;
		if(mystrcasestr( t, blockName))
		{
			geomposok = ftell(file);
			ok= TRUE;
			break;
		}
	}
	if(!ok) return NULL;
	nElements = 0;
	 while(!feof(file))
	 {
		if(!fgets(t,BBSIZE,file)) break;
		if(mystrcasestr( t, blockName)) break;
		nElements++;
	}
	if(nElements<1) return NULL;
	elements = g_malloc(nElements*sizeof(gchar*));
	fseek(file, geomposok, SEEK_SET);
	for(i=0;i<nElements;i++)
	{
			gint k;
			if(!fgets(t,BBSIZE,file))break;
			for(k=0;k<strlen(t);k++) if(t[k]=='\n') t[k]='\0';
			elements[i] = strdup(t);
	}
	*n = nElements;
	return elements;
}
/**********************************************************************************************************************************/
gint* get_one_block_int_from_wfx_file(FILE* file, gchar* blockName,  gint* n)
{
	gint nElements = 0;
	gint* elements = NULL;
	gchar t[BBSIZE];
	long int geomposok = 0;
	gboolean ok = FALSE;
	gchar** allstrs = NULL;
	gint i;
	gint nLines = 0;
	gint k;
	 while(!feof(file))
	 {
		if(!fgets(t,BBSIZE,file))break;
		if(mystrcasestr( t, blockName))
		{
			geomposok = ftell(file);
			ok= TRUE;
			break;
		}
	}
	if(!ok) return NULL;
	nLines = 0;
	 while(!feof(file))
	 {
		if(!fgets(t,BBSIZE,file)) break;
		if(mystrcasestr( t, blockName)) break;
		nLines++;
		allstrs =gab_split (t);
                if(allstrs) for(k=0; allstrs[k]!=NULL; k++) nElements++;
                g_strfreev(allstrs);
                allstrs = NULL;
	}
	if(nLines<1) return NULL;
	if(nElements<1) return NULL;
	elements = g_malloc(nElements*sizeof(gint));
	fseek(file, geomposok, SEEK_SET);
	nElements = 0;
	for(i=0;i<nLines;i++)
	{
		if(!fgets(t,BBSIZE,file))break;
		if(mystrcasestr( t, blockName)) break;
		allstrs =gab_split (t);
                if(allstrs) for(k=0; allstrs[k]!=NULL; k++) 
		{
			elements[nElements] = atof(allstrs[k]);
			nElements++;
		}
                g_strfreev(allstrs);
                allstrs = NULL;
	}
	*n = nElements;
	return elements;
}
/**********************************************************************************************************************************/
gdouble* get_one_block_real_from_wfx_file(FILE* file, gchar* blockName,  gint* n)
{
	gint nElements = 0;
	gdouble* elements = NULL;
	gchar t[BBSIZE];
	long int geomposok = 0;
	gboolean ok = FALSE;
	gchar** allstrs = NULL;
	gint i;
	gint nLines = 0;
	gint k;
	 while(!feof(file))
	 {
		if(!fgets(t,BBSIZE,file))break;
		if(mystrcasestr( t, blockName))
		{
			geomposok = ftell(file);
			ok= TRUE;
			break;
		}
	}
	if(!ok) return NULL;
	nLines = 0;
	 while(!feof(file))
	 {
		if(!fgets(t,BBSIZE,file)) break;
		if(mystrcasestr( t, blockName)) break;
		nLines++;
		allstrs =gab_split (t);
                if(allstrs) for(k=0; allstrs[k]!=NULL; k++) nElements++;
                g_strfreev(allstrs);
                allstrs = NULL;
	}
	if(nLines<1) return NULL;
	if(nElements<1) return NULL;
	elements = g_malloc(nElements*sizeof(gdouble));
	fseek(file, geomposok, SEEK_SET);
	nElements = 0;
	for(i=0;i<nLines;i++)
	{
		if(!fgets(t,BBSIZE,file))break;
		if(mystrcasestr( t, blockName)) break;
		for(k=0;k<strlen(t);k++) if(t[k]=='\n') t[k]='\0';
		for(k=0;k<strlen(t);k++) if(t[k]=='D') t[k]='E';
		for(k=0;k<strlen(t);k++) if(t[k]=='d') t[k]='E';
		allstrs =gab_split (t);
                if(allstrs) for(k=0; allstrs[k]!=NULL; k++) 
		{
			elements[nElements] = atof(allstrs[k]);
			nElements++;
		}
                g_strfreev(allstrs);
                allstrs = NULL;
	}
	*n = nElements;
	return elements;
}
/**********************************************************************************************************************************/
gboolean get_one_int_from_wfx_file(FILE* file, gchar* blockName, gint* n)
{
	gchar t[BBSIZE];
	 while(!feof(file))
	 {
		if(!fgets(t,BBSIZE,file))break;
		if(mystrcasestr( t, blockName))
		{
			if(!fgets(t,BBSIZE,file))break;
			*n = atoi(t);
			return TRUE;
		}
	}
	return FALSE; 
}
/**********************************************************************************************************************************/
gdouble* get_one_orbital_from_wfx_file(FILE* file, gint* n, gint*numOrb)
{
	gint nElements = 0;
	gdouble* elements = NULL;
	gchar t[BBSIZE];
	long int geomposok = 0;
	gboolean ok = FALSE;
	gchar** allstrs = NULL;
	gint i;
	gint nLines = 0;
	gint k;
	if(!get_one_int_from_wfx_file(file, "MO Number", numOrb))
	{
		*n = 0;
		return NULL;
	}
	fgets(t,BBSIZE,file); // <MO Number>
	geomposok = ftell(file);
	nLines = 0;
	 while(!feof(file))
	 {
		if(!fgets(t,BBSIZE,file)) break;
		//fprintf(stderr,"t = %s",t);
		
		if(strstr(t, "<")) break;
		nLines++;
		allstrs =gab_split (t);
                if(allstrs) for(k=0; allstrs[k]!=NULL; k++) nElements++;
                g_strfreev(allstrs);
                allstrs = NULL;
	}
	//fprintf(stderr,"nLines = %d\n",nLines);
	fseek(file, geomposok, SEEK_SET);
	if(nLines<1) return NULL;
	if(nElements<1) return NULL;
	elements = g_malloc(nElements*sizeof(gdouble));
	fseek(file, geomposok, SEEK_SET);
	nElements = 0;
	for(i=0;i<nLines;i++)
	{
		if(!fgets(t,BBSIZE,file))break;
		if(strstr( t, "<")) break;
		for(k=0;k<strlen(t);k++) if(t[k]=='\n') t[k]='\0';
		for(k=0;k<strlen(t);k++) if(t[k]=='D') t[k]='E';
		for(k=0;k<strlen(t);k++) if(t[k]=='d') t[k]='E';
		allstrs =gab_split (t);
                if(allstrs) for(k=0; allstrs[k]!=NULL; k++) 
		{
			elements[nElements] = atof(allstrs[k]);
			nElements++;
		}
                g_strfreev(allstrs);
                allstrs = NULL;
	}
	fseek(file, geomposok, SEEK_SET);

	*n = nElements;
	return elements;
}
/* read all chars from file */
gchar *readFile(gchar *filename)
{
    gchar *fcontent = NULL;
    gint fsize = 0;
    FILE *fp;

    fp = fopen(filename, "rb");
    if(fp) {
        fseek(fp, 0, SEEK_END);
        fsize = ftell(fp);
        rewind(fp);

        fcontent = (gchar*) malloc(sizeof(gchar) * fsize);
        fread(fcontent, 1, fsize, fp);
        fclose(fp);
    }
    return fcontent;
}
