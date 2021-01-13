/* Global.c */
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
#include "Global.h"

  GtkWidget *Fenetre;
  GtkWidget *vboxlistfiles;
  GtkWidget *vboxtexts;
  GtkWidget *vboxmain;
  GtkWidget *BarreMenu;
  GtkWidget *text;
  GtkWidget *treeViewProjects;
  GtkTreeIter *noeud[NBNOD];
  GtkWidget *NoteBookText;
  GtkWidget *NoteBookInfo;
  GtkWidget *TextOutput;
  GtkWidget *TextError;
  GtkWidget *textresult;
  GtkWidget *HboxWins;
  GtkWidget *FrameWins;
  GtkWidget *FrameList;
  GtkWidget *Hpaned;
  GtkWidget *ResultEntryPass;
  GtkWidget *ResultLocalFrame;
  GtkWidget *ResultRemoteFrame;
  gint iedit;
  gint imodif;
  gint iframe;
  gchar *NameCommandGamess;
  gchar *NameCommandGaussian;
  gchar *NameCommandMolcas;
  gchar *NameCommandMolpro;
  gchar *NameCommandMPQC;
  gchar *NameCommandFireFly;
  gchar *NameCommandQChem;
  gchar *NameCommandOrca;
  gchar *NameCommandDeMon;
  gchar *NameCommandNWChem;
  gchar *NameCommandPsicode;
  gchar *NameCommandMopac;
  gchar *NameCommandPovray;
  gint ScreenWidth;
  gint ScreenHeight;
  FontsStyle FontsStyleData;
  FontsStyle FontsStyleResult;
  FontsStyle FontsStyleOther;
  FontsStyle FontsStyleLabel;

  RecentHosts recenthosts;

  int iprogram; 
  gboolean MeasureIsHide;
  FileOpen fileopen;
  gchar* lastdirectory;
  GabEditNetWork defaultNetWorkProtocol;
  gchar* pscpCommand;
  gchar* plinkCommand;
  gchar* pscpplinkDirectory;
  gchar* babelCommand;
  gchar* gamessDirectory;
  gchar* fireflyDirectory;
  gchar* orcaDirectory;
  gchar* demonDirectory;
  gchar* nwchemDirectory;
  gchar* psicodeDirectory;
  gchar* mopacDirectory;
  gchar* gaussDirectory;
  gchar* povrayDirectory;
  gchar* openbabelDirectory;
  CommandsList demonCommands;
  CommandsList gamessCommands;
  CommandsList gaussianCommands;
  CommandsList molcasCommands;
  CommandsList molproCommands;
  CommandsList mpqcCommands;
  CommandsList orcaCommands;
  CommandsList nwchemCommands;
  CommandsList psicodeCommands;
  CommandsList fireflyCommands;
  CommandsList qchemCommands;
  CommandsList mopacCommands;
  CommandsList povrayCommands;

  gchar *NameTypeBatch;
  gchar *NameCommandBatchAll;
  gchar *NameCommandBatchUser;
  gchar *NameCommandBatchKill;
  gchar *NamejobIdTitleBatch;
  CommandsBatch batchCommands;
  OpenGLOptions openGLOptions;
  gint colorMapType;
  gdouble colorMapColors[3][3];
  gdouble alpha_opacity;
  gdouble multipole_rank;
