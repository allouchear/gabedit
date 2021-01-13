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

#ifndef __GABEDIT_GLOBAL_H__
#define __GABEDIT_GLOBAL_H__

#include <stdio.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "../Files/GabeditFileChooser.h"
#include "../Common/GabeditType.h"

#define NBNOD GABEDIT_TYPENODE_OTHER + 1

#define PROG_IS_DEMON  GABEDIT_TYPENODE_DEMON
#define PROG_IS_GAMESS  GABEDIT_TYPENODE_GAMESS
#define PROG_IS_GAUSS  GABEDIT_TYPENODE_GAUSSIAN
#define PROG_IS_MOLCAS GABEDIT_TYPENODE_MOLCAS
#define PROG_IS_MOLPRO GABEDIT_TYPENODE_MOLPRO
#define PROG_IS_MPQC  GABEDIT_TYPENODE_MPQC
#define PROG_IS_ORCA  GABEDIT_TYPENODE_ORCA
#define PROG_IS_FIREFLY  GABEDIT_TYPENODE_FIREFLY
#define PROG_IS_QCHEM  GABEDIT_TYPENODE_QCHEM
#define PROG_IS_NWCHEM  GABEDIT_TYPENODE_NWCHEM
#define PROG_IS_PSICODE  GABEDIT_TYPENODE_PSICODE
#define PROG_IS_MOPAC  GABEDIT_TYPENODE_MOPAC
#define PROG_IS_OTHER -1

#define GEOM_IS_XYZ    GABEDIT_TYPENODE_XYZ
#define GEOM_IS_ZMAT   GABEDIT_TYPENODE_GZMAT
#define GEOM_IS_OTHER -1

#define DATA_MOD_YES   1
#define DATA_MOD_NO    0

#define MAJOR_VERSION    2
#define MINOR_VERSION    5
#define MICRO_VERSION    1

/**** Structures *********/
typedef struct _FileOpen
{
 gchar *projectname; /* without .com or .log or .molden */
 gchar *datafile;
 gchar *outputfile;
 gchar *logfile;
 gchar *moldenfile;

 gchar *localhost;
 gchar *localdir;

 gchar *remotehost;
 gchar *remoteuser;
 gchar *remotepass;
 gchar *remotedir;
 gchar *command;
 GabEditNetWork netWorkProtocol;
}FileOpen;

typedef struct _FontsStyle
{
 gchar *fontname;
 GdkColor BaseColor;
 GdkColor TextColor;
}FontsStyle;

typedef struct _WidgetChildren
{
 gint nchildren;
 GtkWidget **children;
 GabeditSignalFunc *destroychildren;
}WidgetChildren;

typedef struct _User
{
 gint ndirs;
 gchar *username;
 gchar *password;
 gchar **dirs;
}User;

typedef struct _Host
{
 gint nusers;
 gchar *hostname;
 User *users;
}Host;
typedef struct _RecentHosts
{
 gint nhosts;
 Host *hosts;
}RecentHosts;

typedef struct _CommandsList
{
	gint numberOfCommands;
	gint numberOfDefaultCommand;
	gchar** commands;
}CommandsList;

typedef struct _CommandsBatch
{
	gint numberOfTypes;
	gchar** types;
	gchar** commandListAll;
	gchar** commandListUser;
	gchar** commandKill;
	gchar** jobIdTitle;
}CommandsBatch;

/**** Global variables *********/
extern   GtkWidget *Fenetre;
extern   GtkWidget *vboxlistfiles;
extern   GtkWidget *vboxtexts;
extern   GtkWidget *vboxmain;
extern   GtkWidget *BarreMenu;
extern   GtkWidget *text;
extern   GtkWidget *treeViewProjects;
extern   GtkTreeIter *noeud[NBNOD];
extern   GtkWidget *NoteBookText;
extern   GtkWidget *NoteBookInfo;
extern   GtkWidget *TextOutput;
extern   GtkWidget *TextError;
extern   GtkWidget *textresult;
extern   GtkWidget *HboxWins;
extern   GtkWidget *FrameWins;
extern   GtkWidget *FrameList;
extern   GtkWidget *Hpaned;
extern   GtkWidget *ResultEntryPass;
extern   GtkWidget *ResultLocalFrame;
extern   GtkWidget *ResultRemoteFrame;
extern   gint iedit;
extern   gint imodif;
extern   gint iframe;
extern   gchar *NameCommandGamess;
extern   gchar *NameCommandGaussian;
extern   gchar *NameCommandMolcas;
extern   gchar *NameCommandMolpro;
extern   gchar *NameCommandMPQC;
extern   gchar *NameCommandFireFly;
extern   gchar *NameCommandQChem;
extern   gchar *NameCommandOrca;
extern   gchar *NameCommandDeMon;
extern   gchar *NameCommandNWChem;
extern   gchar *NameCommandPsicode;
extern   gchar *NameCommandMopac;
extern   gchar *NameCommandPovray;
extern   gint ScreenWidth;
extern   gint ScreenHeight;
extern   FontsStyle FontsStyleData;
extern   FontsStyle FontsStyleResult;
extern   FontsStyle FontsStyleOther;
extern   FontsStyle FontsStyleLabel;

extern   RecentHosts recenthosts;

extern   int iprogram; 
extern   gboolean MeasureIsHide;
extern   FileOpen fileopen;
extern   gchar* lastdirectory;
extern   GabEditNetWork defaultNetWorkProtocol;
extern   gchar* pscpCommand;
extern   gchar* plinkCommand;
extern   gchar* pscpplinkDirectory;
extern   gchar* babelCommand;
extern   gchar* gamessDirectory;
extern   gchar* fireflyDirectory;
extern   gchar* orcaDirectory;
extern   gchar* demonDirectory;
extern   gchar* nwchemDirectory;
extern   gchar* psicodeDirectory;
extern   gchar* mopacDirectory;
extern   gchar* gaussDirectory;
extern   gchar* povrayDirectory;
extern   gchar* openbabelDirectory;
extern   CommandsList demonCommands;
extern   CommandsList gamessCommands;
extern   CommandsList gaussianCommands;
extern   CommandsList molcasCommands;
extern   CommandsList molproCommands;
extern   CommandsList mpqcCommands;
extern   CommandsList orcaCommands;
extern   CommandsList nwchemCommands;
extern   CommandsList psicodeCommands;
extern   CommandsList fireflyCommands;
extern   CommandsList qchemCommands;
extern   CommandsList mopacCommands;
extern   CommandsList povrayCommands;

extern   gchar *NameTypeBatch;
extern   gchar *NameCommandBatchAll;
extern   gchar *NameCommandBatchUser;
extern   gchar *NameCommandBatchKill;
extern   gchar *NamejobIdTitleBatch;
extern   CommandsBatch batchCommands;
extern   OpenGLOptions openGLOptions;
extern   gint colorMapType;
extern   gdouble colorMapColors[3][3];
extern   gdouble alpha_opacity;
extern   gdouble multipole_rank;

#endif /* __GABEDIT_GLOBAL_H__ */

