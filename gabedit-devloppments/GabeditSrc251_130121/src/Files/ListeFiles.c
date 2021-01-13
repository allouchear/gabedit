/* ListeFiles.c */
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
#include "../Common/Global.h"

#ifdef G_OS_WIN32
#include <winsock.h>
#else
#include <unistd.h>
#endif /* G_OS_WIN32 */


#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/ResultsAnalise.h"
#include "../Geometry/EnergiesCurves.h"
#include "../Common/Windows.h"
#include "../Geometry/GeomZmatrix.h"
#include "../Common/Run.h"
#include "../Common/Status.h"
#include "../Molcas/MolcasVariables.h"
#include "../Molcas/MolcasGateWay.h"
#include "../../pixmaps/DeMonMini.xpm"
#include "../../pixmaps/GamessMini.xpm"
#include "../../pixmaps/FireFlyMini.xpm"
#include "../../pixmaps/Gaussian.xpm"
#include "../../pixmaps/MolproMini.xpm"
#include "../../pixmaps/MolcasMini.xpm"
#include "../../pixmaps/MPQCMini.xpm"
#include "../../pixmaps/NWChemMini.xpm"
#include "../../pixmaps/PsicodeMini.xpm"
#include "../../pixmaps/OrcaMini.xpm"
#include "../../pixmaps/QChemMini.xpm"
#include "../../pixmaps/MopacMini.xpm"
#include "../../pixmaps/GabeditMini.xpm"
#include "../../pixmaps/Book_close.xpm"
#include "../../pixmaps/Page.xpm"

#include "../Common/StockIcons.h"
#include "../Files/ListeFiles.h"

static GdkPixbuf *demonPixbuf = NULL;
static GdkPixbuf *gamessPixbuf = NULL;
static GdkPixbuf *gaussianPixbuf = NULL;
static GdkPixbuf *molcasPixbuf = NULL;
static GdkPixbuf *molproPixbuf = NULL;
static GdkPixbuf *mpqcPixbuf = NULL;
static GdkPixbuf *fireflyPixbuf = NULL;
static GdkPixbuf *nwchemPixbuf = NULL;
static GdkPixbuf *psicodePixbuf = NULL;
static GdkPixbuf *orcaPixbuf = NULL;
static GdkPixbuf *qchemPixbuf = NULL;
static GdkPixbuf *mopacPixbuf = NULL;
static GdkPixbuf *gabeditPixbuf = NULL;
static GdkPixbuf *bookPixbuf = NULL;
static GdkPixbuf *pagePixbuf = NULL;

typedef enum
{
  LIST_PIXBUF = 0,
  LIST_NAME,
  LIST_NODE_FOREGROUND_COLOR, /* this column is not visible */
  LIST_DATA /* column for data, this column is not visible */
}ListColumnsTypes;


static DataTree** AllFiles=NULL;
static int Nfiles = 0;
static GtkWidget *checkbutton[NBNOD];
/*static  GtkTooltips *GroupeHelp = NULL;*/
static gboolean popupshow = FALSE;

#define NL 3
static GtkWidget *LocalLabel[NL];
#define NR 3
static GtkWidget *RemoteLabel[NR];
static gchar selectedRow[100] = "-1";

/********************************************************************************/
static GtkTreeIter* CreeNoeud(GtkTreeView* , gchar *);
void CreeFeuille(GtkWidget *treeView, GtkTreeIter *parent,gchar* projectname,gchar*datafile,gchar* localdir,
				  gchar* remotehost,gchar* remoteuser,gchar* remotepass,gchar* remotedir,gint itype, gchar* command, GabEditNetWork netWorkProtocol);
static void AddFeuille(GtkTreeView *treeView, GtkTreeIter *parent, DataTree *pfd);
static void tree_data_destroy (DataTree* pfd);
static void  create_window_list_to_clear();
static void clear_one_project();
static void create_set_dialogue_window();
static DataTree* tree_data_new (gchar *projectname,gchar* datafile,gchar* localdir,
		gchar* remotehost,gchar* remoteuser,gchar* remotepass,gchar* remotedir,gint itype, gchar* command, GabEditNetWork netWorkProtocol);
static void create_remote_frame_popup(GtkWidget *hbox,DataTree* data);
static void create_local_frame_popup(GtkWidget *hbox,DataTree* data);
static void create_info_win();

/********************************************************************************/
static void set_pixbuf()
{
	if(!demonPixbuf) demonPixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) demon_mini_xpm);
	if(!gamessPixbuf) gamessPixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) gamess_mini_xpm);
	if(!gaussianPixbuf) gaussianPixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) gaussian_xpm);
	if(!molcasPixbuf) molcasPixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) molcas_mini_xpm);
	if(!molproPixbuf) molproPixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) molpro_mini_xpm);
	if(!mpqcPixbuf) mpqcPixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) mpqc_mini_xpm);
	if(!nwchemPixbuf) nwchemPixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) nwchem_mini_xpm);
	if(!psicodePixbuf) psicodePixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) psicode_mini_xpm);
	if(!orcaPixbuf) orcaPixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) orca_mini_xpm);
	if(!qchemPixbuf) qchemPixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) qchem_mini_xpm);
	if(!mopacPixbuf) mopacPixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) mopac_mini_xpm);
	if(!gabeditPixbuf) gabeditPixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) gabedit_mini_xpm);
	if(!fireflyPixbuf) fireflyPixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) firefly_mini_xpm);
	if(!bookPixbuf) bookPixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) book_close_xpm);
	if(!pagePixbuf) pagePixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) page_xpm);
}
/********************************************************************************/
static void set_sensitive_option(GtkUIManager *manager, gchar* path, gboolean sensitive)
{
	GtkWidget *wid = gtk_ui_manager_get_widget (manager, path);
	if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid, sensitive);
}
/**********************************************************************************/
static gboolean show_menu_popup(GtkUIManager *manager, guint button, guint32 time)
{
	GtkWidget *menu = gtk_ui_manager_get_widget (manager, "/MenuRecentProjects");
	if (GTK_IS_MENU (menu)) 
	{
		gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, button, time);
		return TRUE;
	}
	return FALSE;
}
/*********************************************************************************************************************/
static void activate_action (GtkAction *action)
{
	const gchar *name = gtk_action_get_name (action);
	if(!strcmp(name, "Info")) create_info_win();
	else if(!strcmp(name, "Cut")) clear_one_project();
	else if(!strcmp(name, "Set")) create_set_dialogue_window();
	else if(!strcmp(name, "ClearNodes")) create_window_list_to_clear(); 
	else if(!strcmp(name, "Save")) parse_liste_files(); 
}
/*--------------------------------------------------------------------*/
static GtkActionEntry gtkActionEntries[] =
{
	{"Info", GABEDIT_STOCK_INFO, N_("_Info"), NULL, "Info", G_CALLBACK (activate_action) },
	{"Cut", GABEDIT_STOCK_CUT, N_("_Cut"), NULL, "Cut", G_CALLBACK (activate_action) },
	{"Set", NULL, "_Set", NULL, N_("Set"), G_CALLBACK (activate_action) },
	{"ClearNodes", NULL, N_("Clear _nodes"), NULL, "Clear nodes", G_CALLBACK (activate_action) },
	{"Save", GABEDIT_STOCK_SAVE, N_("_Save list of projects"), NULL, "Save list of projects", G_CALLBACK (activate_action) },
};
static guint numberOfGtkActionEntries = G_N_ELEMENTS (gtkActionEntries);
/********************************************************************************/
/* XML description of the menus for the test app.  The parser understands
 * a subset of the Bonobo UI XML format, and uses GMarkup for parsing */
static const gchar *uiMenuInfo =
"  <popup name=\"MenuRecentProjects\">\n"
"    <separator name=\"sepMenuPopNew\" />\n"
"    <menuitem name=\"Info\" action=\"Info\" />\n"
"    <menuitem name=\"Cut\" action=\"Cut\" />\n"
"    <menuitem name=\"Set\" action=\"Set\" />\n"
"    <separator name=\"sepMenuPopNodes\" />\n"
"    <menuitem name=\"ClearNodes\" action=\"ClearNodes\" />\n"
"    <separator name=\"sepMenuPopSave\" />\n"
"    <menuitem name=\"Save\" action=\"Save\" />\n"
"  </popup>\n"
;
/*******************************************************************************************************************************/
static GtkUIManager *create_menu(GtkWidget* win)
{
	GtkActionGroup *actionGroup = NULL;
	GtkUIManager *manager = NULL;
	GError *error = NULL;

  	manager = gtk_ui_manager_new ();
  	g_signal_connect_swapped (win, "destroy", G_CALLBACK (g_object_unref), manager);

	actionGroup = gtk_action_group_new ("GabeditListOfProject");
	gtk_action_group_set_translation_domain(actionGroup,GETTEXT_PACKAGE);
	gtk_action_group_add_actions (actionGroup, gtkActionEntries, numberOfGtkActionEntries, NULL);

  	gtk_ui_manager_insert_action_group (manager, actionGroup, 0);

  	gtk_window_add_accel_group (GTK_WINDOW (win), gtk_ui_manager_get_accel_group (manager));
	if (!gtk_ui_manager_add_ui_from_string (manager, uiMenuInfo, -1, &error))
	{
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}
	return manager;
}
/********************************************************************************/
static void set_fileopen(DataTree* data)
{
	if(fileopen.projectname) g_free(fileopen.projectname);
	if(fileopen.localdir) g_free(fileopen.localdir);
	if(fileopen.datafile) g_free(fileopen.datafile);
	if(fileopen.outputfile) g_free(fileopen.outputfile);
	if(fileopen.logfile) g_free(fileopen.logfile);
	if(fileopen.moldenfile) g_free(fileopen.moldenfile);

	fileopen.projectname=g_strdup(data->projectname);
	fileopen.datafile=g_strdup(data->datafile);
	fileopen.localdir=g_strdup(data->localdir);

	if(data->itype == PROG_IS_MOLCAS)
	{
 		fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  		/* fileopen.moldenfile=g_strdup_printf("%s.molden",fileopen.projectname);*/
  		fileopen.moldenfile=g_strdup_printf("'%s.*.molden'", fileopen.projectname);
	}
	else if(data->itype == PROG_IS_MOLPRO)
	{
 		fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("%s.molden",fileopen.projectname);
	}
	else if(data->itype == PROG_IS_GAUSS)
	{
 		fileopen.outputfile = g_strdup_printf("%s.log",fileopen.projectname);
 		fileopen.logfile = g_strdup_printf("%s.log",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("%s.log",fileopen.projectname);
	}
	else if(data->itype == PROG_IS_MPQC)
	{
 		fileopen.outputfile = g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile = g_strdup_printf("%s.out",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);
	}
	else if(data->itype == PROG_IS_ORCA)
	{
 		fileopen.outputfile = g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile = g_strdup_printf("%s.out",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);
	}
	else if(data->itype == PROG_IS_QCHEM)
	{
 		fileopen.outputfile = g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile = g_strdup_printf("%s.out",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);
	}
	else if(data->itype == PROG_IS_NWCHEM)
	{
 		fileopen.outputfile = g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile = g_strdup_printf("%s.out",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);
	}
	else if(data->itype == PROG_IS_PSICODE)
	{
 		fileopen.outputfile = g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile = g_strdup_printf("%s.out",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);
	}
	else if(data->itype == PROG_IS_MOPAC)
	{
 		fileopen.outputfile = g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile = g_strdup_printf("%s.aux",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);
	}
	else if(data->itype == PROG_IS_FIREFLY)
	{
 		fileopen.outputfile = g_strdup_printf("%s.log",fileopen.projectname);
 		fileopen.logfile = g_strdup_printf("%s.log",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("%s.log",fileopen.projectname);
	}
	else if(data->itype == PROG_IS_DEMON)
	{
 		fileopen.outputfile = g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile = g_strdup_printf("%s.out",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);
	}
	else if(data->itype == PROG_IS_GAMESS)
	{
 		fileopen.outputfile = g_strdup_printf("%s.log",fileopen.projectname);
 		fileopen.logfile = g_strdup_printf("%s.log",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("%s.log",fileopen.projectname);
	}
	else
	{
 		fileopen.outputfile = g_strdup_printf("unknown");
 		fileopen.logfile = g_strdup_printf("unknown");
 		fileopen.moldenfile = g_strdup_printf("unknown");
	}

	fileopen.remotehost = g_strdup(data->remotehost);
	fileopen.remoteuser = g_strdup(data->remoteuser);
	fileopen.remotepass = g_strdup(data->remotepass);
	fileopen.remotedir = g_strdup(data->remotedir);
  	fileopen.command=g_strdup(data->command);
	fileopen.netWorkProtocol = data->netWorkProtocol;


	if(lastdirectory) g_free(lastdirectory);
	lastdirectory = g_strdup(fileopen.localdir);
}
/********************************************************************************/
static void set_properties(GtkWidget *Win,gpointer data)
{  
	DataTree*  newdata = NULL;
	DataTree*  olddata = (DataTree*)(g_object_get_data(G_OBJECT(Win),"Data")); 
	GtkWidget **entrys = (GtkWidget **)(g_object_get_data(G_OBJECT(Win),"Entries"));
	GtkWidget **buttons =(GtkWidget **)(g_object_get_data(G_OBJECT(Win),"Buttons"));

	GtkWidget * buttonSsh = g_object_get_data(G_OBJECT(Win),"ButtonSsh");
	/*
	GtkWidget * buttonFtpRsh = g_object_get_data(G_OBJECT(Win),"ButtonFtpRsh");
	*/

	gchar* remotehost = NULL;
	gchar* remoteuser = NULL;
	gchar* remotepass = NULL;
	gchar* remotedir  = NULL;
	GabEditNetWork netWorkProtocol ;


	if (GTK_TOGGLE_BUTTON (buttons[0])->active)
	{
		remotehost = g_strdup("");
		remoteuser = g_strdup("");
		remotepass  = g_strdup("");
		remotedir  = g_strdup("");
	}
	else
	{
		remotehost = g_strdup(gtk_entry_get_text(GTK_ENTRY(entrys[0])));
		remoteuser = g_strdup(gtk_entry_get_text(GTK_ENTRY(entrys[1])));
		remotepass = g_strdup(gtk_entry_get_text(GTK_ENTRY(entrys[2])));
		remotedir  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entrys[3])));
	}
	if (GTK_TOGGLE_BUTTON (buttonSsh)->active) netWorkProtocol = GABEDIT_NETWORK_SSH;
	else netWorkProtocol = GABEDIT_NETWORK_FTP_RSH;

	
   	newdata = tree_data_new (
			olddata->projectname,olddata->datafile,olddata->localdir,
			remotehost,remoteuser,remotepass,remotedir,
			olddata->itype, olddata->command, netWorkProtocol);
	if(remotehost) g_free(remotehost);
	if(remoteuser) g_free(remoteuser);
	if(remotepass) g_free(remotepass);
	if(remotedir) g_free(remotedir);
	
	clear_one_project();
	AddFeuille(GTK_TREE_VIEW(treeViewProjects),noeud[newdata->itype],newdata);
   	if(Nfiles==0)
      		AllFiles = g_malloc(sizeof(DataTree*));
    	else
      		AllFiles = (DataTree**)g_realloc(AllFiles,(Nfiles+1)*sizeof(DataTree*));
    Nfiles++;
    AllFiles[Nfiles-1] = newdata;
	if(  strcmp(fileopen.projectname,newdata->projectname)==0 
	  && strcmp(fileopen.localdir,newdata->localdir)==0 
	  )
	{
		set_fileopen(newdata);
 		change_all_labels();
	}
}
/********************************************************************************/
static void set_password_visibility(GtkWidget *button,gpointer data)
{
	GtkWidget * entryPassWord = g_object_get_data(G_OBJECT (button), "EntryPassWord");
	GtkWidget * label1PassWord = g_object_get_data(G_OBJECT (button), "Label1PassWord");
	GtkWidget * label2PassWord = g_object_get_data(G_OBJECT (button), "Label2PassWord");

	if (GTK_TOGGLE_BUTTON (button)->active)
	{
		gboolean Ok = FALSE;
		if(data != NULL ) Ok = TRUE;

		gtk_widget_set_sensitive(entryPassWord, Ok);
		gtk_widget_set_sensitive(label1PassWord, Ok);
		gtk_widget_set_sensitive(label2PassWord, Ok);
	}
}
/********************************************************************************/
static void set_frame_remote_sensitive(GtkWidget *button,gpointer data)
{
	GtkWidget *FrameRemote = GTK_WIDGET(g_object_get_data(G_OBJECT(button),"FrameRemote"));
	GtkWidget *FrameNetWork = GTK_WIDGET(g_object_get_data(G_OBJECT(button),"FrameNetWork"));
	if (GTK_TOGGLE_BUTTON (button)->active)
	{
		gboolean Ok = FALSE;
		if(data != NULL ) Ok = TRUE;

		gtk_widget_set_sensitive(FrameRemote, Ok);
		gtk_widget_set_sensitive(FrameNetWork, Ok);
			
	}
}
/********************************************************************************/
static void changed_user(GtkWidget *combo,gpointer data)
{
	GtkWidget **entry = (GtkWidget **)data;
	gint nlistdir = 1;
	G_CONST_RETURN gchar *hostname;
	G_CONST_RETURN gchar *username;
	gint numhost = -1;
	gint numuser = -1;
	GtkWidget* combodir = NULL;
	gint i;
	gint j;
        GList *glist = NULL;

        if(!this_is_an_object((GtkObject*)entry[0]))
		return;
        if(!this_is_an_object((GtkObject*)entry[1]))
		return;
        if(!this_is_an_object((GtkObject*)entry[3]))
		return;

	hostname = gtk_entry_get_text(GTK_ENTRY(entry[0]));

	username = gtk_entry_get_text(GTK_ENTRY(entry[1]));

	combodir = g_object_get_data(G_OBJECT (entry[3]), "Combo");

        if(!combodir)
		return;

	if(recenthosts.nhosts>0)
  	{
		for(i=0;i<recenthosts.nhosts;i++)
  			if(strcmp(hostname,recenthosts.hosts[i].hostname) == 0)
			{
				numhost = i;
				for(j=0;j<recenthosts.hosts[numhost].nusers;j++)
  					if(strcmp(username,recenthosts.hosts[i].users[j].username) == 0)
					{
						numuser = j;
						break;
					}
				
				break;
			}
		if(numhost<0)
			return;
		if(numuser<0)
			return;

        	nlistdir = recenthosts.hosts[numhost].users[numuser].ndirs;
		for(i=0;i<nlistdir;i++)
  			glist = g_list_append(glist,recenthosts.hosts[numhost].users[numuser].dirs[i]);
  	}
  	else
		return;


        gtk_combo_box_entry_set_popdown_strings( combodir, glist) ;


	g_list_free(glist);
}
/********************************************************************************/
static void changed_host(GtkWidget *combo,gpointer data)
{
	GtkWidget **entry = (GtkWidget **)data;
	gint nlistuser = 1;
	G_CONST_RETURN gchar *hostname;
	gint numhost = -1;
	GtkWidget* combouser = NULL;
	gint i;
        GList *glist = NULL;

	hostname = gtk_entry_get_text(GTK_ENTRY(entry[0]));
        if(!this_is_an_object((GtkObject*)entry[1]))
		return;
	combouser = g_object_get_data(G_OBJECT (entry[1]), "Combo");
        if(!combouser)
		return;
	if(recenthosts.nhosts>0)
  	{
		for(i=0;i<recenthosts.nhosts;i++)
  			if(strcmp(hostname,recenthosts.hosts[i].hostname) == 0)
			{
				numhost = i;
				break;
			}
		if(numhost<0)
			return;

        	nlistuser = recenthosts.hosts[numhost].nusers;
		for(i=0;i<recenthosts.hosts[numhost].nusers;i++)
		glist = g_list_append(glist,recenthosts.hosts[numhost].users[i].username);
  	}
  	else
		return;


  	for (i=0;i<nlistuser;i++)
        	gtk_combo_box_entry_set_popdown_strings( combouser, glist) ;


	g_list_free(glist);
}
/********************************************************************************/
static void create_info_win()
{
	gchar *title = g_strdup_printf(_("Properties of project"));
	GtkWidget *fp;
	GtkWidget *vboxwin;
	GtkWidget *hbox = NULL;
	GtkWidget *button = NULL;
	GtkTreeIter node;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeViewProjects));
   	DataTree* data = NULL;

	if(atoi(selectedRow)<0) return;
	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);

	if(!data) return;

	fp = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  	gtk_window_set_title(GTK_WINDOW(fp),title);
  	gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  	gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Fenetre));

	vboxwin = create_vbox(fp);
        gtk_widget_realize(fp);
  	init_child(fp,gtk_widget_destroy,_(" Prop. of project "));
	g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)destroy_children,NULL);

	hbox = gtk_hbox_new(0,FALSE);
	gtk_box_pack_start (GTK_BOX(vboxwin),hbox , TRUE, TRUE, 2);
        gtk_widget_show(hbox);
	create_local_frame_popup(hbox,data);
	create_remote_frame_popup(hbox,data);
   	create_hseparator(vboxwin);
	/* OK bouton box */
	hbox = create_hbox(vboxwin);
  	gtk_box_set_homogeneous (GTK_BOX(hbox), FALSE);
	button = create_button(fp,"OK");
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, FALSE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);

	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)destroy_children,GTK_OBJECT(fp));
        gtk_widget_show_all(fp);
}
/********************************************************************************/
static void create_set_dialogue_window()
{
	GtkWidget *fp;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget *vboxwin;
	GtkWidget *hbox;
	GtkWidget *button;
	GtkWidget **entrys;
	GtkWidget **buttons;
	GtkWidget *FrameRemote = NULL;
	GtkWidget *FrameNetWork = NULL;
	gchar *title = g_strdup_printf(_("Set properties of project"));
	GtkWidget *ButtonFtpRsh;
	GtkWidget *ButtonSsh;
	GtkWidget *label1PassWord;
	GtkWidget *label2PassWord;

	GtkTreeIter node;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeViewProjects));
   	DataTree* data = NULL;

	if(atoi(selectedRow)<0) return;
	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);

	if(!data) return;

	entrys = g_malloc(4*sizeof(GtkWidget *));
	buttons = g_malloc(2*sizeof(GtkWidget *));

  	/* Principal Window */
  	fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  	gtk_window_set_title(GTK_WINDOW(fp),title);
  	gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  	gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Fenetre));

  	gtk_widget_realize(fp);
  	init_child(fp,gtk_widget_destroy,_(" Prop. of project "));
	g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)destroy_children,NULL);

	gtk_container_set_border_width (GTK_CONTAINER (fp), 5);
	vboxall = create_vbox(fp);
	vboxwin = vboxall;

	frame = gtk_frame_new (NULL);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_add(GTK_CONTAINER(vboxall),frame);
	gtk_widget_show (frame);

	vboxall = create_vbox(frame);

	hbox = create_hbox(vboxall);

	/* server frame */
	{
		GtkWidget *Table;
		GtkWidget *vboxframe;

		frame = gtk_frame_new (_("Server"));
		gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
		gtk_box_pack_start (GTK_BOX( hbox), frame, TRUE, TRUE, 2);
		gtk_widget_show (frame);

		vboxframe = gtk_vbox_new (TRUE, 0);
		gtk_widget_show (vboxframe);
		gtk_container_add (GTK_CONTAINER (frame), vboxframe);

		Table = gtk_table_new(1,2,TRUE);
		gtk_container_add(GTK_CONTAINER(vboxframe),Table);

		buttons[0]  = gtk_radio_button_new_with_label( NULL,_("Local"));

		gtk_table_attach(GTK_TABLE(Table), buttons[0] ,0,1,0,1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);

		buttons[1]  = gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttons[0])),"Remote host "); 
		gtk_table_attach(GTK_TABLE(Table), buttons[1] ,1,2,0,1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);
	}

	/* local frame */
	{
		GtkWidget* vboxframe;
		GtkWidget* Table;
		gchar* t = NULL;


  		frame = gtk_frame_new (_("Local"));
  		gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  		gtk_container_add (GTK_CONTAINER (vboxall), frame);
  		gtk_widget_show (frame);

  		vboxframe = create_vbox(frame);
  		Table = gtk_table_new(2,3,FALSE);
  		gtk_container_add(GTK_CONTAINER(vboxframe),Table);

		add_label_table(Table,_("Local Directory "),0,0);
		add_label_table(Table,":",0,1);
		add_label_table(Table,data->localdir,0,2);

  		if(
		data->itype == PROG_IS_GAUSS || data->itype == PROG_IS_MOLCAS || 
		data->itype == PROG_IS_MOLPRO || data->itype == PROG_IS_MPQC
		|| data->itype == PROG_IS_ORCA 
		|| data->itype == PROG_IS_QCHEM 
		|| data->itype == PROG_IS_NWCHEM 
		|| data->itype == PROG_IS_PSICODE 
		|| data->itype == PROG_IS_MOPAC 
		|| data->itype == PROG_IS_GAMESS 
		|| data->itype == PROG_IS_DEMON 
		|| data->itype == PROG_IS_FIREFLY 
		)
			add_label_table(Table,_("Files "),1,0);
  		else
			add_label_table(Table,_("File "),1,0);

  		switch(data->itype)
  		{
			case PROG_IS_DEMON :
			t = g_strdup_printf("%s, %s.out",data->datafile,data->projectname);
			break;

			case PROG_IS_GAMESS :
			t = g_strdup_printf("%s, %s.log",data->datafile,data->projectname);
			break;

			case PROG_IS_GAUSS :
			t = g_strdup_printf("%s, %s.log",data->datafile,data->projectname);
			break;

			case PROG_IS_MOLCAS : 
			t = g_strdup_printf("%s, %s.out, %s.log, %s.molden",data->datafile,data->projectname,data->projectname,data->projectname);
			break;

			case PROG_IS_MOLPRO : 
			t = g_strdup_printf("%s, %s.out, %s.log, %s.molden",data->datafile,data->projectname,data->projectname,data->projectname);
			break;

			case PROG_IS_MPQC : 
			t = g_strdup_printf("%s, %s.out, %s.log, %s.molden",data->datafile,data->projectname,data->projectname,data->projectname);
			break;

			case PROG_IS_FIREFLY :
			t = g_strdup_printf("%s, %s.log",data->datafile,data->projectname);
			break;

			case PROG_IS_ORCA : 
			t = g_strdup_printf("%s, %s.out, %s.out, %s.out",data->datafile,data->projectname,data->projectname,data->projectname);
			break;
			case PROG_IS_QCHEM : 
			t = g_strdup_printf("%s, %s.out, %s.out, %s.out",data->datafile,data->projectname,data->projectname,data->projectname);
			break;
			case PROG_IS_NWCHEM : 
			t = g_strdup_printf("%s, %s.out, %s.out, %s.out",data->datafile,data->projectname,data->projectname,data->projectname);
			break;

			case PROG_IS_PSICODE : 
			t = g_strdup_printf("%s, %s.out, %s.out, %s.out",data->datafile,data->projectname,data->projectname,data->projectname);
			break;

			case PROG_IS_MOPAC : 
			t = g_strdup_printf("%s, %s.out, %s.aux, %s.out",data->datafile,data->projectname,data->projectname,data->projectname);
			break;

			case GABEDIT_TYPENODE_GABEDIT :
			case GABEDIT_TYPENODE_XYZ :
			case GABEDIT_TYPENODE_MOL2 :
			case GABEDIT_TYPENODE_TINKER :
			case GABEDIT_TYPENODE_PDB :
			case GABEDIT_TYPENODE_GZMAT :
			case GABEDIT_TYPENODE_MZMAT :
			case GABEDIT_TYPENODE_HIN :
			t = g_strdup_printf("%s",data->datafile);
			break;

			default : 
			t = g_strdup_printf("%s",data->projectname);
	
  		}
		add_label_table(Table,":",1,1);
		add_label_table(Table,t,1,2);
		g_free(t);
	}

	/* network protocol frame */
	{
		GtkWidget *frame;
		GtkWidget *vboxframe;
		GtkWidget *table = gtk_table_new(1,2,FALSE);
		gchar ftprsh[] = N_("FTP and rsh protocols");
		gchar ssh[]    = N_("ssh/scp protocols    ");

		frame = gtk_frame_new (_("NetWork protocols"));
		gtk_widget_show (frame);
		gtk_box_pack_start (GTK_BOX (vboxall), frame, TRUE, TRUE, 0);

		vboxframe = gtk_vbox_new (TRUE, 0);
		gtk_widget_show (vboxframe);
		gtk_container_add (GTK_CONTAINER (frame), vboxframe);

		gtk_box_pack_start (GTK_BOX (vboxframe), table, TRUE, TRUE, 0);

		ButtonFtpRsh = gtk_radio_button_new_with_label( NULL,ftprsh);
		gtk_table_attach(GTK_TABLE(table),ButtonFtpRsh,0,1,0,1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);
		gtk_widget_show (ButtonFtpRsh);

		ButtonSsh = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonFtpRsh)), ssh); 
		gtk_table_attach(GTK_TABLE(table),ButtonSsh,1,2,0,1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);
		gtk_widget_show (ButtonSsh);
		FrameNetWork = frame;


	}
	/* remote frame */
	{
		GtkWidget *combo;
		GtkWidget *vboxframe;
#define NLABEL 4
		gchar	*LabelLeft[NLABEL];
		gushort i;
		GtkWidget *Table;
		gchar *tlisthost[NHOSTMAX];
		gchar *tlistuser[NHOSTMAX];
		gchar *tlistdir[NHOSTMAX];
		gint nlisthost = 1;
		gint nlistuser = 1;
		gint nlistdir  = 1;

		if(recenthosts.nhosts>0)
		{
			nlisthost = recenthosts.nhosts;
			for(i=0;i<nlisthost;i++)
				tlisthost[i] = g_strdup(recenthosts.hosts[i].hostname);
			nlistuser = recenthosts.hosts[0].nusers;
			for(i=0;i<recenthosts.hosts[0].nusers;i++)
				tlistuser[i] = g_strdup(recenthosts.hosts[0].users[i].username);
			nlistdir = recenthosts.hosts[0].users[0].ndirs;
			for(i=0;i<recenthosts.hosts[0].users[0].ndirs;i++)
				tlistdir[i] = g_strdup(recenthosts.hosts[0].users[0].dirs[i]);
		}
		else
		{
			tlisthost[0] = g_strdup("hostname");
			tlistuser[0] = g_strdup("login");
			tlistdir[0] = g_strdup("");
		}

		LabelLeft[0] = g_strdup(_("Host name"));
		LabelLeft[1] = g_strdup(_("Login"));
		LabelLeft[2] = g_strdup(_("Password"));
		LabelLeft[3] = g_strdup(_("Working Directory"));

		frame = gtk_frame_new (_("Remote host"));
		FrameRemote = frame;
		gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
		gtk_container_add (GTK_CONTAINER (vboxall), frame);
		gtk_widget_show (frame);

		vboxframe = create_vbox(frame);
		Table = gtk_table_new(3,3,FALSE);
		gtk_container_add(GTK_CONTAINER(vboxframe),Table);

		i = 0;
		add_label_table(Table,LabelLeft[i],(gushort)(i),0);
		add_label_table(Table,":",(gushort)(i),1);
		combo = create_combo_box_entry(tlisthost,nlisthost,TRUE,-1,-1);
		gtk_table_attach(GTK_TABLE(Table),combo,2,3,i,i+1,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
		gtk_widget_show (combo);
		entrys[i] = GTK_BIN(combo)->child;
		g_object_set_data(G_OBJECT (entrys[i]), "Combo",combo);
		g_signal_connect(G_OBJECT(GTK_COMBO_BOX(combo)), "changed",G_CALLBACK(changed_host),entrys);

		i = 1;
		add_label_table(Table,LabelLeft[i],(gushort)(i),0);
		add_label_table(Table,":",(gushort)(i),1);
		combo = create_combo_box_entry(tlistuser,nlistuser,TRUE,-1,-1);
		add_widget_table(Table,combo,(gushort)(i),2);
		entrys[i] = GTK_BIN(combo)->child;
		g_object_set_data(G_OBJECT (entrys[i]), "Combo",combo);
		g_signal_connect(G_OBJECT(GTK_COMBO_BOX(combo)), "changed",G_CALLBACK(changed_user),entrys);

		i = 2;
		label1PassWord = add_label_table(Table,LabelLeft[i],(gushort)(i),0);
		label2PassWord = add_label_table(Table,":",(gushort)(i),1);
		entrys[i] = gtk_entry_new ();
		gtk_entry_set_visibility(GTK_ENTRY (entrys[i]),FALSE);
		add_widget_table(Table,entrys[i],(gushort)(i),2);

		i = 3;
		add_label_table(Table,LabelLeft[i],(gushort)(i),0);
		add_label_table(Table,":",(gushort)(i),1);
		combo = create_combo_box_entry(tlistdir,nlistdir,TRUE,-1,-1);
		add_widget_table(Table,combo,(gushort)(i),2);
		entrys[i] = GTK_BIN(combo)->child;
		g_object_set_data(G_OBJECT (entrys[i]), "Combo",combo);

		gtk_widget_show_all(frame);
		
		if(data->remotehost)
			gtk_entry_set_text(GTK_ENTRY(entrys[0]),data->remotehost);
		if(data->remoteuser)
			gtk_entry_set_text(GTK_ENTRY(entrys[1]),data->remoteuser);
		if(data->remotepass)
			gtk_entry_set_text(GTK_ENTRY(entrys[2]),data->remotepass);
		if(data->remotedir)
			gtk_entry_set_text(GTK_ENTRY(entrys[3]),data->remotedir);

		for (i=0;i<NLABEL;i++)
			g_free(LabelLeft[i]);
		for (i=0;i<nlisthost;i++)
			g_free(tlisthost[i]);
		for (i=0;i<nlistuser;i++)
			g_free(tlistuser[i]);
		for (i=0;i<nlistdir;i++)
			g_free(tlistdir[i]);

	}

	g_object_set_data(G_OBJECT (ButtonFtpRsh), "EntryPassWord", entrys[2]);
	g_object_set_data(G_OBJECT (ButtonFtpRsh), "Label1PassWord", label1PassWord);
	g_object_set_data(G_OBJECT (ButtonFtpRsh), "Label2PassWord", label2PassWord);
	g_signal_connect(G_OBJECT(ButtonFtpRsh), "clicked",G_CALLBACK(set_password_visibility),ButtonFtpRsh);

	g_object_set_data(G_OBJECT (ButtonSsh), "EntryPassWord", entrys[2]);
	g_object_set_data(G_OBJECT (ButtonSsh), "Label1PassWord", label1PassWord);
	g_object_set_data(G_OBJECT (ButtonSsh), "Label2PassWord", label2PassWord);
	g_signal_connect(G_OBJECT(ButtonSsh), "clicked",G_CALLBACK(set_password_visibility),NULL);
	

	/* OK and Cancel boutons box */
	hbox = create_hbox(vboxwin);

	button = create_button(fp,_("Cancel"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)destroy_children,GTK_OBJECT(fp));
	gtk_widget_show (button);

	button = create_button(fp,"OK");
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_object_set_data(G_OBJECT(fp),"Entries",entrys);
	g_object_set_data(G_OBJECT(fp),"Buttons",buttons);
	g_object_set_data(G_OBJECT(fp),"Data",data);
	g_object_set_data(G_OBJECT(fp),"ButtonSsh",ButtonSsh);
	g_object_set_data(G_OBJECT(fp),"ButtonFtpRsh",ButtonFtpRsh);

	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(set_properties),(gpointer)fp);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)destroy_children,GTK_OBJECT(fp));


	g_object_set_data(G_OBJECT(buttons[0]),"FrameRemote",FrameRemote);
	g_object_set_data(G_OBJECT(buttons[1]),"FrameRemote",FrameRemote);
	g_object_set_data(G_OBJECT(buttons[0]),"FrameNetWork",FrameNetWork);
	g_object_set_data(G_OBJECT(buttons[1]),"FrameNetWork",FrameNetWork);
	g_signal_connect(G_OBJECT(buttons[0]), "clicked",G_CALLBACK(set_frame_remote_sensitive),NULL);
	g_signal_connect(G_OBJECT(buttons[1]), "clicked",G_CALLBACK(set_frame_remote_sensitive),frame);
#ifdef G_OS_WIN32
	gtk_widget_set_sensitive(buttons[0], FALSE);
#endif
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttons[1]), TRUE);

  	if(data->netWorkProtocol== GABEDIT_NETWORK_FTP_RSH) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonFtpRsh), TRUE);
	else gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonSsh), TRUE);

  	/* Show all */
  	gtk_widget_show_all(fp);
}
/********************************************************************************/
void set_label_infos_file()
{
 	gtk_label_set_text(GTK_LABEL(LocalLabel[2]),fileopen.localdir);
	if(fileopen.remotehost)
 		gtk_label_set_text(GTK_LABEL(RemoteLabel[0]),fileopen.remotehost);
	else
 		gtk_label_set_text(GTK_LABEL(RemoteLabel[0])," ");
	if(fileopen.remoteuser)
 		gtk_label_set_text(GTK_LABEL(RemoteLabel[1]),fileopen.remoteuser);
	else
 		gtk_label_set_text(GTK_LABEL(RemoteLabel[1])," ");
	if(fileopen.remotedir)
 		gtk_label_set_text(GTK_LABEL(RemoteLabel[2]),fileopen.remotedir);
	else
 		gtk_label_set_text(GTK_LABEL(RemoteLabel[2])," ");

        if(fileopen.remotepass && !this_is_a_backspace(fileopen.remotepass))
 		gtk_entry_set_text(GTK_ENTRY(ResultEntryPass),fileopen.remotepass);
	else
 		gtk_entry_set_text(GTK_ENTRY(ResultEntryPass),"");

	if(
		fileopen.remotehost && !this_is_a_backspace(fileopen.remotehost) &&
		fileopen.remoteuser && !this_is_a_backspace(fileopen.remoteuser) 
	)
	{

		set_sensitive_remote_frame(TRUE);
	}
	else
		set_sensitive_remote_frame(FALSE);

	
}
/********************************************************************************/
static void create_remote_frame_popup(GtkWidget *hbox,DataTree* data)
{
  GtkWidget *frame;
  GtkWidget *Label;
  GtkWidget *vboxframe;
  gchar      *LabelLeft[4];
  gushort i;
  GtkWidget *Table;
  gchar* t = NULL;

  LabelLeft[0] = g_strdup(_("Host"));
  LabelLeft[1] = g_strdup(_("Login"));
  LabelLeft[2] = g_strdup(_("Directory"));


  if(data->itype == PROG_IS_GAUSS 
    || data->itype == PROG_IS_MOLCAS 
    || data->itype == PROG_IS_MOLPRO 
    || data->itype == PROG_IS_MPQC 
    || data->itype == PROG_IS_GAMESS 
    || data->itype == PROG_IS_FIREFLY 
    || data->itype == PROG_IS_DEMON
    || data->itype == PROG_IS_QCHEM 
    ||  data->itype == PROG_IS_NWCHEM 
    ||  data->itype == PROG_IS_PSICODE  
    || data->itype == PROG_IS_MOPAC  
    || data->itype == PROG_IS_ORCA )
  	LabelLeft[3] = g_strdup(_("Files"));
  else
  	LabelLeft[3] = g_strdup(_("File"));

  frame = gtk_frame_new (_("Remote"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_box_pack_start (GTK_BOX(hbox),frame, TRUE, TRUE, 1);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  Table = gtk_table_new(4,3,FALSE);
  gtk_container_add(GTK_CONTAINER(vboxframe),Table);

  Label = add_label_table(Table,LabelLeft[0],0,0);
  add_label_table(Table,":",0,1);
  add_label_table(Table,data->remotehost,0,2);

  Label = add_label_table(Table,LabelLeft[1],1,0);
  add_label_table(Table,":",1,1);
  if(data->remoteuser && strlen(data->remoteuser)<1 && this_is_a_backspace(data->remoteuser))
  {
  	add_label_table(Table,data->remoteuser,1,2);
  }
  else
  {
  	gchar* tmp = NULL;
  	if(data->netWorkProtocol== GABEDIT_NETWORK_FTP_RSH) tmp = g_strdup_printf(_("%s  [Using rsh/ftp Protocol]"), data->remoteuser);
  	else tmp = g_strdup_printf(_("%s  [Using ssh Protocol]"),data->remoteuser);
  	add_label_table(Table,tmp,1,2);
	g_free(tmp);

  }

  Label = add_label_table(Table,LabelLeft[2],2,0);
  add_label_table(Table,":",2,1);
  add_label_table(Table,data->remotedir,2,2);

  if(!data->remotehost ||  this_is_a_backspace(data->remotehost))
  t = g_strdup("");
  else
  switch(data->itype)
  {
	case PROG_IS_DEMON :
		t = g_strdup_printf("%s, %s.out",data->datafile,data->projectname);
		break;

	case PROG_IS_GAMESS :
		t = g_strdup_printf("%s, %s.log",data->datafile,data->projectname);
		break;

	case PROG_IS_GAUSS :
		t = g_strdup_printf("%s, %s.log",data->datafile,data->projectname);
		break;

	case PROG_IS_MOLCAS : 
		t = g_strdup_printf("%s, %s.out, %s.log, %s.molden",data->datafile,data->projectname,data->projectname,data->projectname);
		break;

	case PROG_IS_MOLPRO : 
		t = g_strdup_printf("%s, %s.out, %s.log, %s.molden",data->datafile,data->projectname,data->projectname,data->projectname);
		break;
	case PROG_IS_MPQC : 
		t = g_strdup_printf("%s, %s.out, %s.log, %s.molden",data->datafile,data->projectname,data->projectname,data->projectname);
		break;

	case PROG_IS_FIREFLY :
		t = g_strdup_printf("%s, %s.log",data->datafile,data->projectname);
		break;
	case PROG_IS_ORCA : 
		t = g_strdup_printf("%s, %s.out",data->datafile,data->projectname);
		break;

	case PROG_IS_QCHEM : 
		t = g_strdup_printf("%s, %s.out",data->datafile,data->projectname);
		break;

	case PROG_IS_NWCHEM : 
		t = g_strdup_printf("%s, %s.out",data->datafile,data->projectname);
		break;

	case PROG_IS_PSICODE : 
		t = g_strdup_printf("%s, %s.out",data->datafile,data->projectname);
		break;


	case PROG_IS_MOPAC : 
		t = g_strdup_printf("%s, %s.out, %s.aux",data->datafile,data->projectname,data->projectname);
		break;

	case GABEDIT_TYPENODE_GABEDIT :
	case GABEDIT_TYPENODE_XYZ :
	case GABEDIT_TYPENODE_MOL2 :
	case GABEDIT_TYPENODE_TINKER :
	case GABEDIT_TYPENODE_PDB :
	case GABEDIT_TYPENODE_GZMAT :
	case GABEDIT_TYPENODE_MZMAT :
	case GABEDIT_TYPENODE_HIN :
		t = g_strdup_printf("%s",data->datafile);
		break;
	default : 
		t = g_strdup_printf("%s",data->projectname);
	
  }

  Label = add_label_table(Table,LabelLeft[3],3,0);
  add_label_table(Table,":",3,1);
  add_label_table(Table,t,3,2);
  g_free(t);

  for (i=0;i<4;i++)
        g_free(LabelLeft[i]);
}
/********************************************************************************/
static void create_local_frame_popup(GtkWidget *hbox,DataTree* data)
{
  GtkWidget *frame;
  GtkWidget *Label;
  GtkWidget *vboxframe;
  gchar      *LabelLeft[4];
  gushort i;
  GtkWidget *Table;
  gchar localhost[100];
  G_CONST_RETURN gchar *localuser = get_local_user();
  gchar *t;

#ifdef G_OS_WIN32
	winsockCheck(stderr);
#endif
  gethostname(localhost,100);
  LabelLeft[0] = g_strdup(_("Host"));
  LabelLeft[1] = g_strdup(_("Login"));
  LabelLeft[2] = g_strdup(_("Directory"));
  if(data->itype == PROG_IS_GAUSS 
    || data->itype == PROG_IS_MOLCAS 
    || data->itype == PROG_IS_MOLPRO 
    || data->itype == PROG_IS_MPQC  
    || data->itype == PROG_IS_GAMESS 
    || data->itype == PROG_IS_DEMON
    || data->itype == PROG_IS_FIREFLY 
    || data->itype == PROG_IS_QCHEM 
    || data->itype == PROG_IS_NWCHEM 
    || data->itype == PROG_IS_PSICODE 
    || data->itype == PROG_IS_MOPAC 
    || data->itype == PROG_IS_ORCA )
  	LabelLeft[3] = g_strdup(_("Files"));
  else
  	LabelLeft[3] = g_strdup(_("File"));

  frame = gtk_frame_new (_("Local"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_box_pack_start (GTK_BOX(hbox),frame, TRUE, TRUE, 1);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  Table = gtk_table_new(4,3,FALSE);
  gtk_container_add(GTK_CONTAINER(vboxframe),Table);

  Label = add_label_table(Table,LabelLeft[0],0,0);
  add_label_table(Table,":",0,1);
  add_label_table(Table,localhost,0,2);

  Label = add_label_table(Table,LabelLeft[1],1,0);
  add_label_table(Table,":",1,1);
  add_label_table(Table,localuser,1,2);

  Label = add_label_table(Table,LabelLeft[2],2,0);
  add_label_table(Table,":",2,1);
  add_label_table(Table,data->localdir,2,2);


  switch(data->itype)
  {
	case PROG_IS_DEMON :
		t = g_strdup_printf("%s, %s.out",data->datafile,data->projectname);
		break;

	case PROG_IS_GAMESS :
		t = g_strdup_printf("%s, %s.log",data->datafile,data->projectname);
		break;

	case PROG_IS_GAUSS :
		t = g_strdup_printf("%s, %s.log",data->datafile,data->projectname);
		break;

	case PROG_IS_MOLCAS : 
		t = g_strdup_printf("%s, %s.out, %s.log, %s.molden",data->datafile,data->projectname,data->projectname,data->projectname);
		break;

	case PROG_IS_MOLPRO : 
		t = g_strdup_printf("%s, %s.out, %s.log, %s.molden",data->datafile,data->projectname,data->projectname,data->projectname);
		break;
	case PROG_IS_MPQC : 
		t = g_strdup_printf("%s, %s.out, %s.log, %s.molden",data->datafile,data->projectname,data->projectname,data->projectname);
		break;

	case PROG_IS_FIREFLY :
		t = g_strdup_printf("%s, %s.log",data->datafile,data->projectname);
		break;

	case PROG_IS_ORCA : 
		t = g_strdup_printf("%s, %s.out",data->datafile,data->projectname);
		break;

	case PROG_IS_QCHEM : 
		t = g_strdup_printf("%s, %s.out",data->datafile,data->projectname);
		break;

	case PROG_IS_NWCHEM : 
		t = g_strdup_printf("%s, %s.out",data->datafile,data->projectname);
		break;

	case PROG_IS_PSICODE : 
		t = g_strdup_printf("%s, %s.out",data->datafile,data->projectname);
		break;


	case PROG_IS_MOPAC : 
		t = g_strdup_printf("%s, %s.out, %s.aux",data->datafile,data->projectname, data->projectname);
		break;
	case GABEDIT_TYPENODE_GABEDIT :
	case GABEDIT_TYPENODE_XYZ :
	case GABEDIT_TYPENODE_MOL2 :
	case GABEDIT_TYPENODE_TINKER :
	case GABEDIT_TYPENODE_PDB :
	case GABEDIT_TYPENODE_GZMAT :
	case GABEDIT_TYPENODE_MZMAT :
	case GABEDIT_TYPENODE_HIN :
		t = g_strdup_printf("%s",data->datafile);
		break;
	default : 
		t = g_strdup_printf("%s",data->datafile);
	
  }

  Label = add_label_table(Table,LabelLeft[3],3,0);
  add_label_table(Table,":",3,1);
  add_label_table(Table,t,3,2);
  g_free(t);

  for (i=0;i<4;i++)
        g_free(LabelLeft[i]);
}
/********************************************************************************/
static gint destroy_popup(gpointer data)
{
        gtk_widget_hide(GTK_WIDGET(data));
        gtk_object_destroy(GTK_OBJECT(data));
 	popupshow = FALSE;
        return FALSE;
}
/********************************************************************************/
static void create_popupwin(DataTree* data)
{
	GtkWidget *MainFrame;
	GtkWidget *hbox = gtk_hbox_new(0,FALSE);
	
 	popupshow = TRUE;

	MainFrame = gtk_window_new (GTK_WINDOW_POPUP);

	/* center it on the screen*/
        gtk_window_set_position(GTK_WINDOW (MainFrame), GTK_WIN_POS_MOUSE);

	/* set up key and mound button press to hide splash screen*/

        gtk_widget_add_events(MainFrame,
                              GDK_BUTTON_PRESS_MASK|
                              GDK_BUTTON_RELEASE_MASK|
                              GDK_KEY_PRESS_MASK);

        g_signal_connect(G_OBJECT(MainFrame),"button_release_event",
                G_CALLBACK(destroy_popup),NULL);
        g_signal_connect(G_OBJECT(MainFrame),"key_press_event",
                G_CALLBACK(gtk_widget_destroy),NULL);
        gtk_widget_realize(MainFrame);

        gtk_widget_show(hbox);
        gtk_container_add (GTK_CONTAINER (MainFrame), hbox);
	create_local_frame_popup(hbox,data);
	create_remote_frame_popup(hbox,data);
        gtk_widget_show_all(MainFrame);

	/* force it to draw now.*/
	/* gdk_flush();*/

	/* go into main loop, processing events.*/
        while(gtk_events_pending())
                gtk_main_iteration();
	/* after 3 seconds, destroy the splash screen.*/
    /*    gtk_timeout_add( 5000, destroy_popup, MainFrame );*/
}
/********************************************************************************/
static void create_remote_frame(GtkWidget *hbox)
{
  GtkWidget *frame;
  GtkWidget *Label;
  GtkWidget *vboxframe;
  gchar      *LabelLeft[NR];
  gushort i;
  GtkWidget *Table;

  LabelLeft[0] = g_strdup(_("Host"));
  LabelLeft[1] = g_strdup(_("Login"));
  LabelLeft[2] = g_strdup(_("Directory"));

  frame = gtk_frame_new (_("Remote"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_box_pack_start (GTK_BOX(hbox),frame, TRUE, TRUE, 1);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  Table = gtk_table_new(3,3,FALSE);
  gtk_container_add(GTK_CONTAINER(vboxframe),Table);

  Label = add_label_table(Table,LabelLeft[0],0,0);
  add_label_table(Table,":",0,1);
  RemoteLabel[0] = add_label_table(Table,"",0,2);

  Label = add_label_table(Table,LabelLeft[1],1,0);
  add_label_table(Table,":",1,1);
  RemoteLabel[1] = add_label_table(Table,"",1,2);

  Label = add_label_table(Table,LabelLeft[2],2,0);
  add_label_table(Table,":",2,1);
  RemoteLabel[2] = add_label_table(Table,"",2,2);



  for (i=0;i<NR;i++)
        g_free(LabelLeft[i]);
}
/********************************************************************************/
void resetFontLabelErrOut()
{
  	GtkWidget* labelOngletOut = g_object_get_data(G_OBJECT (Fenetre), "LabelOngletOutput");
  	GtkWidget* labelOngletErr = g_object_get_data(G_OBJECT (Fenetre), "LabelOngletError");
	PangoFontDescription *font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);

	/********/
	return;

	  GtkStyle* style;
	  if(!font_desc)return;
	  if(labelOngletOut)
	  {
	  	style = gtk_style_copy(gtk_widget_get_style(labelOngletOut)); 
		if(!style) return;
	  	if(font_desc) style->font_desc = font_desc;
	  	gtk_widget_set_style(labelOngletOut, style );
	  }
	  if(labelOngletErr)
	  {
	  	style = gtk_style_copy(gtk_widget_get_style(labelOngletErr)); 
		if(!style) return;
	  	if(font_desc) style->font_desc = font_desc;
	  	gtk_widget_set_style(labelOngletErr, style );
	  }
}
/********************************************************************************/
static void AddNotebookPage(GtkWidget* NoteBook,char *label,GtkWidget **TextP)
{
  GtkWidget *scrolledwindow;
  GtkWidget *Frame;
  GtkWidget *LabelOnglet;
  GtkWidget *LabelMenu;
  GtkWidget *vboxframe = NULL;
  

  Frame = gtk_frame_new(NULL);
  gtk_container_set_border_width(GTK_CONTAINER(Frame), 2);

  LabelOnglet = gtk_label_new(label);
  gtk_widget_show(LabelOnglet);
  if(strstr(label,_("Output")))
  {
	  GtkStyle* style = gtk_style_copy( gtk_widget_get_default_style ()); 
	  /*
	  PangoFontDescription *font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);
	  */

          style->fg[0].red=0;
          style->fg[0].green=0;
          style->fg[0].blue=65535;
	  /*
	  if(font_desc) style->font_desc = font_desc;
	  */
	  gtk_widget_set_style(LabelOnglet, style );
  	  vboxframe = create_vbox(Frame);
  	  g_object_set_data(G_OBJECT (Fenetre), "LabelOngletOutput", LabelOnglet);
  }
  else
  if(strstr(label,_("Error")))
  {
	  /*
	  PangoFontDescription *font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);
	  */
	  GtkStyle* style = gtk_style_copy(LabelOnglet->style); 
          style->fg[0].red=65535;
          style->fg[0].green=0;
          style->fg[0].blue=0;
	  /*
	  if(font_desc) style->font_desc = font_desc;
	  */
	  gtk_widget_set_style(LabelOnglet, style );
  	  g_object_set_data(G_OBJECT (Fenetre), "LabelOngletError", LabelOnglet);
  }
  LabelMenu = gtk_label_new(label);
  gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),Frame,LabelOnglet, LabelMenu);

  g_object_set_data(G_OBJECT (Frame), "Frame", Frame);

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  if(strstr(label,_("Output")))
  {
  	  gtk_box_pack_start (GTK_BOX(vboxframe),scrolledwindow, TRUE, TRUE, 1);
  	  create_status_progress_connection_bar(vboxframe);
  }
  else
  gtk_container_add (GTK_CONTAINER (Frame), scrolledwindow);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  *TextP = gabedit_text_new ();
  set_tab_size (*TextP, 8);
  /* gabedit_text_set_word_wrap (GABEDIT_TEXT(*TextP), FALSE);*/
  gtk_widget_show (*TextP);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), *TextP);
}
/*****************************************************************************************/
static void create_local_frame(GtkWidget *hbox)
{
  GtkWidget *frame;
  GtkWidget *Label;
  GtkWidget *vboxframe;
  gchar      *LabelLeft[NL];
  gushort i;
  GtkWidget *Table;
  gchar localhost[100];
  G_CONST_RETURN gchar *localuser = get_local_user();

#ifdef G_OS_WIN32
	winsockCheck(stderr);
#endif
  gethostname(localhost,100);
  LabelLeft[0] = g_strdup(_("Host"));
  LabelLeft[1] = g_strdup(_("Login"));
  LabelLeft[2] = g_strdup(_("Directory"));

  frame = gtk_frame_new (_("Local"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_box_pack_start (GTK_BOX(hbox),frame, TRUE, TRUE, 1);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  Table = gtk_table_new(3,3,FALSE);
  gtk_container_add(GTK_CONTAINER(vboxframe),Table);

  Label = add_label_table(Table,LabelLeft[0],0,0);
  add_label_table(Table,":",0,1);
  LocalLabel[0] = add_label_table(Table,localhost,0,2);

  Label = add_label_table(Table,LabelLeft[1],1,0);
  add_label_table(Table,":",1,1);
  LocalLabel[1] = add_label_table(Table,localuser,1,2);

  Label = add_label_table(Table,LabelLeft[2],2,0);
  add_label_table(Table,":",2,1);
  LocalLabel[2] = add_label_table(Table," ",2,2);

  for (i=0;i<NL;i++)
        g_free(LabelLeft[i]);
}
/********************************************************************************/
static void AddNotebookPageFiles(GtkWidget* NoteBook,char *label)
{
  GtkWidget *vboxframe;
  GtkWidget *hbox;
  GtkWidget *LabelOnglet;
  GtkWidget *LabelMenu;


  vboxframe = gtk_vbox_new(0,FALSE);

  LabelOnglet = gtk_label_new(label);
  LabelMenu = gtk_label_new(label);
  gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),
                                vboxframe,
                                LabelOnglet, LabelMenu);
/*
  vboxframe = gtk_vbox_new(0,FALSE);
  gtk_container_add (GTK_CONTAINER (Frame),vboxframe);
*/

  hbox = gtk_hbox_new(TRUE,1);
  gtk_container_add(GTK_CONTAINER(vboxframe),hbox);
  create_local_frame(hbox);
  create_remote_frame(hbox);

}
/********************************************************************************/
void cree_files_out_err_notebook(GtkWidget* box)
{
  GtkWidget *NoteBook;

  NoteBook = gtk_notebook_new();
  gtk_notebook_set_show_border(GTK_NOTEBOOK(NoteBook),TRUE);
  gtk_notebook_popup_enable(GTK_NOTEBOOK(NoteBook));
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(NoteBook),GTK_POS_LEFT);
  gtk_notebook_set_scrollable(GTK_NOTEBOOK(NoteBook), TRUE); 
  gtk_widget_show(NoteBook);
  gtk_box_pack_start(GTK_BOX (box), NoteBook,FALSE, TRUE, 0);

  AddNotebookPageFiles(NoteBook,_(" Location "));
  
  AddNotebookPage(NoteBook,_(" Output "),&TextOutput);
  gabedit_text_set_editable (GABEDIT_TEXT (TextOutput), FALSE);
  set_font (TextOutput,FontsStyleResult.fontname);
  set_base_style(TextOutput,FontsStyleResult.BaseColor.red ,FontsStyleResult.BaseColor.green ,FontsStyleResult.BaseColor.blue);
  set_text_style(TextOutput,FontsStyleResult.TextColor.red ,FontsStyleResult.TextColor.green ,FontsStyleResult.TextColor.blue);

  AddNotebookPage(NoteBook,_(" Error "),&TextError);
  gabedit_text_set_editable (GABEDIT_TEXT (TextError), FALSE);
  set_font (TextError,FontsStyleResult.fontname);
  set_base_style(TextError,FontsStyleResult.BaseColor.red ,FontsStyleResult.BaseColor.green ,FontsStyleResult.BaseColor.blue);
  set_text_style(TextError,FontsStyleResult.TextColor.red ,FontsStyleResult.TextColor.green ,FontsStyleResult.TextColor.blue);
  gtk_widget_show(NoteBook);
  hide_progress_connection();
  NoteBookInfo = NoteBook;

}
/********************************************************************************/
static void save_one_line(FILE* fd,gchar* st)
{
	if(!st || strlen(st)<1)
	{
		fprintf(fd," \n");
		return;
	}
	str_delete_n(st);
	if(strlen(st)<1)
	{
		fprintf(fd," \n");
		return;
	}
	fprintf(fd,"%s\n",st);
}
/********************************************************************************/
void parse_liste_files()
{
 	gint i;
 	FILE *fd;
 	gchar *outfile= g_strdup_printf("%s%slistfiles",gabedit_directory(), G_DIR_SEPARATOR_S);
	gboolean *toprint = g_malloc(Nfiles*sizeof(gboolean));
	gint Nf[NBNOD];
	gint NfAll=0;
	gboolean expanded[NBNOD];
	gchar* pathString;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeViewProjects));

	if(AllFiles == NULL) return;


	for(i=0;i<NBNOD;i++)
	{
 		expanded[i] =  FALSE;
		if (noeud[i] == NULL) continue;
		pathString = g_strdup_printf("%d", i);
		if(gtk_tree_model_get_iter_from_string (model, &iter, pathString) == TRUE)
		{
			GtkTreePath *path = gtk_tree_path_new_from_string  (pathString);
 			expanded[i] =  gtk_tree_view_row_expanded(GTK_TREE_VIEW(treeViewProjects), path);
			gtk_tree_path_free(path);
		}
		g_free(pathString);
       }

        if(!AllFiles)
        {
 		fd = FOpen(outfile, "w");
        	if(fd)
		{
			fprintf(fd,"0\n");
		}
    		fclose(fd);
		return;
	}
	for(i=0;i<NBNOD;i++)
	{
		Nf[i] = 0;
	}

	for(i=Nfiles-1;i>=0;i--)
	{
		if(Nf[AllFiles[i]->itype]<=20)
		{
			Nf[AllFiles[i]->itype]++;
			NfAll++;
			toprint[i] = TRUE;
		}
		else
			toprint[i] = FALSE;
	}

 	fd = FOpen(outfile, "w");
        if(fd)
	{
		fprintf(fd,"%d\n",NfAll);
		for(i=0;i<Nfiles;i++)
			if(toprint[i])
			{
				fprintf(fd,"%d\n",AllFiles[i]->itype);
				save_one_line(fd,AllFiles[i]->projectname);
				save_one_line(fd,AllFiles[i]->datafile);
				save_one_line(fd,AllFiles[i]->localdir);
				save_one_line(fd,AllFiles[i]->remotehost);
				save_one_line(fd,AllFiles[i]->remoteuser);
				save_one_line(fd,AllFiles[i]->remotedir);
				save_one_line(fd,AllFiles[i]->command);

				if(AllFiles[i]->netWorkProtocol == GABEDIT_NETWORK_FTP_RSH) fprintf(fd,"0\n");
				else fprintf(fd,"1\n");
			}
		
		for(i=0;i<NBNOD;i++)
			fprintf(fd,"%d\n",(gint)expanded[i]);
    		fclose(fd);
	}
	g_free(outfile);
	g_free(toprint);
}
/********************************************************************************/
static void get_one_line(FILE* fd,gchar st[])
{
	static gint taille = BSIZE;
	gchar t[BSIZE];

    	{ char* e = fgets(t,taille,fd);}
	strcpy(st,t);
	str_delete_n(st);
}
/********************************************************************************/
static void add_liste_files()
{
	gint i;
 	FILE *fd;
	gint taille = BSIZE;
	gchar *t=g_malloc(BSIZE*sizeof(gchar));
	gint itype;
	gchar* projectname = g_malloc(BSIZE*sizeof(gchar));
	gchar* datafile = g_malloc(BSIZE*sizeof(gchar));
	gchar* localdir = g_malloc(BSIZE*sizeof(gchar));
	gchar* remotehost = g_malloc(BSIZE*sizeof(gchar));
	gchar* remoteuser = g_malloc(BSIZE*sizeof(gchar));
	gchar* remotedir = g_malloc(BSIZE*sizeof(gchar));
	gchar* command = g_malloc(BSIZE*sizeof(gchar));
 	gchar *outfile= g_strdup_printf("%s%slistfiles",gabedit_directory(), G_DIR_SEPARATOR_S);
	gint Nf =0;
	gboolean expanded;
	GabEditNetWork netWorkProtocol;
	gint iNet = 0;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeViewProjects));
	GtkTreeIter iter;
        
#ifdef G_OS_WIN32
 	fd = FOpen(outfile, "r");
#else
 	fd = FOpen(outfile, "rb");
#endif
        if(fd)
	{
    		{ char* e = fgets(t,BSIZE,fd);}
		Nf = atoi(t);
		Nfiles = 0;
		if(Nf>0)
		{
			for(i=0;i<Nf;i++)
			{
				projectname[0] = '\0';
				datafile[0] = '\0';
				localdir[0] = '\0';
				remotehost[0] = '\0';
				remoteuser[0] = '\0';
				remotedir[0] = '\0';

    				{ char* e = fgets(t,BSIZE,fd);}
				sscanf(t,"%d",&itype);
				get_one_line(fd,projectname);
				get_one_line(fd,datafile);
				get_one_line(fd,localdir);
				get_one_line(fd,remotehost);
				get_one_line(fd,remoteuser);
				get_one_line(fd,remotedir);
				get_one_line(fd,command);
    				{ char* e = fgets(t,BSIZE,fd);}
				sscanf(t,"%d",&iNet);
				if(iNet==0) netWorkProtocol = GABEDIT_NETWORK_FTP_RSH;
				else netWorkProtocol = GABEDIT_NETWORK_SSH;

 				CreeFeuille(treeViewProjects, noeud[itype],projectname,datafile,localdir,remotehost,remoteuser,"",remotedir,itype, command, netWorkProtocol);
			}
		}
		for(i=0;i<NBNOD;i++)
		{
			if(fgets(t,taille,fd))
				expanded =(gboolean)atoi(t);
			else
				expanded =FALSE;
			if(expanded)
			{
				gchar* pathString = g_strdup_printf("%d", i);
				if(gtk_tree_model_get_iter_from_string (model, &iter, pathString) == TRUE)
				{
					GtkTreePath *path = gtk_tree_path_new_from_string  (pathString);
					gtk_tree_view_expand_to_path(GTK_TREE_VIEW(treeViewProjects), path);
					gtk_tree_path_free(path);
				}
				g_free(pathString);
			}
		}

    	fclose(fd);
	}
	g_free(projectname);
	g_free(datafile);
	g_free(localdir);
	g_free(remotehost);
	g_free(remoteuser);
	g_free(remotedir);
	g_free(command);
	g_free(outfile);
	g_free(t);
}
/********************************************************************************/
static void get_doc_no_add_list(GtkWidget *wid, gpointer d)
{
	DataTree* data = (DataTree*)d;
	gchar *t;
	FILE *fd = NULL;
	guint nchar;
	guint taille=BSIZE;
        
	gchar *NomFichier = NULL;
	NomFichier =  g_strdup_printf("%s%s%s",data->localdir,G_DIR_SEPARATOR_S,data->datafile);

	if ((!NomFichier) || (strcmp(NomFichier,"") == 0)) return ;

	t=g_malloc(taille*sizeof(gchar));
	fd = FOpen(NomFichier, "rb");
	if(fd == NULL)
	{
		//fprintf(stderr,"\nSorry, I can not open file :\n %s\n",NomFichier);
 		g_free(t);
		t = g_strdup_printf(_("\nSorry, I can not open file :\n %s\n"),NomFichier);
		Message(t,"ERROR",TRUE);
		g_free(t);
		return;
	}

	iprogram = get_type_of_program(fd);
	//fprintf(stderr,"\niprog=%d\n",iprogram);

	nchar=gabedit_text_get_length(GABEDIT_TEXT(text));
	gtk_notebook_set_current_page((GtkNotebook*)NoteBookText,0);
	gabedit_text_set_point(GABEDIT_TEXT(text),0);
	gabedit_text_forward_delete(GABEDIT_TEXT(text),nchar);
	while(1)
	{
		nchar = fread(t, 1, taille, fd);
		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, t,nchar);
		if(nchar<taille) break;
	}
	fclose(fd);
	g_free(t);
	gabedit_text_set_point(GABEDIT_TEXT(text),0);

	set_fileopen(data);


	if(iprogram == PROG_IS_MOLPRO ) read_geom_in_molpro_input(NomFichier);
	else if(iprogram == PROG_IS_GAUSS) read_geom_in_gauss_input(NomFichier);
	else if(iprogram == PROG_IS_DEMON) read_geom_in_demon_input(NomFichier);// TO CHANGE
	else if(iprogram == PROG_IS_GAMESS) read_geom_in_gamess_input(NomFichier);
	else if(iprogram == PROG_IS_FIREFLY) read_geom_in_gamess_input(NomFichier);
	else if(iprogram == PROG_IS_MOLCAS)
	{
		setMolcasVariablesFromInputFile(NomFichier);
		read_geom_in_molcas_input(NomFichier);
	}
	else if(iprogram == PROG_IS_MPQC) read_geom_in_mpqc_input(NomFichier);
	else if(iprogram == PROG_IS_QCHEM) read_geom_in_qchem_input(NomFichier);
	else if(iprogram == PROG_IS_NWCHEM) read_geom_in_nwchem_input(NomFichier);
	else if(iprogram == PROG_IS_PSICODE) read_geom_in_psicode_input(NomFichier);
	else if(iprogram == PROG_IS_ORCA) read_geom_in_orca_input(NomFichier);
	else if(iprogram == PROG_IS_MOPAC) read_geom_in_mopac_input(NomFichier);

/*	printf("iprogram=%d nwchem=%d orca=%d\n",iprogram,PROG_IS_NWCHEM,PROG_IS_ORCA);*/

	data_modify(FALSE);

	if(GeomConvIsOpen) find_energy_all(NULL,NULL);
}
/********************************************************************************/
static void select_row(DataTree* data)
{
        gchar *t;
        gchar *allname = NULL;

        if (!data) return;

        switch(data->itype)
        {
        case GABEDIT_TYPENODE_DEMON:
        case GABEDIT_TYPENODE_GAMESS:
        case GABEDIT_TYPENODE_GAUSSIAN:
        case GABEDIT_TYPENODE_MOLCAS:
        case GABEDIT_TYPENODE_MOLPRO:
        case GABEDIT_TYPENODE_MPQC:
        case GABEDIT_TYPENODE_FIREFLY:
        case GABEDIT_TYPENODE_ORCA:
        case GABEDIT_TYPENODE_QCHEM:
        case GABEDIT_TYPENODE_NWCHEM:
        case GABEDIT_TYPENODE_PSICODE:
        case GABEDIT_TYPENODE_MOPAC:
        case NBNOD-1:
 	        if(imodif == DATA_MOD_YES)
            	{
				t = g_strdup_printf(_("\nThe \"%s\" file has been modified.\n\n"),get_name_file(fileopen.datafile));
				t = g_strdup_printf(_(" %sIf you continue, you lose what you have changed.\n\n"),t);
				t = g_strdup_printf(_(" %sYou want to continue?\n"),t);
				Continue_YesNo(get_doc_no_add_list, data,t);
				g_free(t);
            	}
            	else
        		get_doc_no_add_list(NULL,data);
		break;
        case GABEDIT_TYPENODE_GABEDIT:
				MethodeGeom = GEOM_IS_XYZ;
				allname = g_strdup_printf("%s%s%s",data->localdir,G_DIR_SEPARATOR_S,data->datafile);
  				read_gabedit_file_no_add_list(allname);
				set_last_directory(allname);
				g_free(allname);
		break;
        case GABEDIT_TYPENODE_XYZ:
				MethodeGeom = GEOM_IS_XYZ;
				allname = g_strdup_printf("%s%s%s",data->localdir,G_DIR_SEPARATOR_S,data->datafile);
  				read_XYZ_file_no_add_list(allname);
				set_last_directory(allname);
				g_free(allname);
		break;
        case GABEDIT_TYPENODE_MOL2:
				MethodeGeom = GEOM_IS_XYZ;
				allname = g_strdup_printf("%s%s%s",data->localdir,G_DIR_SEPARATOR_S,data->datafile);
  				read_mol2_tinker_file_no_add_list(allname,"MOL2");
				set_last_directory(allname);
				g_free(allname);
		break;
        case GABEDIT_TYPENODE_TINKER:
				MethodeGeom = GEOM_IS_XYZ;
				allname = g_strdup_printf("%s%s%s",data->localdir,G_DIR_SEPARATOR_S,data->datafile);
  				read_mol2_tinker_file_no_add_list(allname,"TINKER");
				set_last_directory(allname);
				g_free(allname);
		break;
        case GABEDIT_TYPENODE_PDB:
				MethodeGeom = GEOM_IS_XYZ;
				allname = g_strdup_printf("%s%s%s",data->localdir,G_DIR_SEPARATOR_S,data->datafile);
  				read_pdb_file_no_add_list(allname);
				set_last_directory(allname);
				g_free(allname);
		break;
    	case GABEDIT_TYPENODE_GZMAT:
                	MethodeGeom = GEOM_IS_ZMAT;
			allname = g_strdup_printf("%s%s%s",data->localdir,G_DIR_SEPARATOR_S,data->datafile);
  			read_ZMatrix_file_no_add_list(allname);
			set_last_directory(allname);
			g_free(allname);
			break;
    	case GABEDIT_TYPENODE_MZMAT:
                	MethodeGeom = GEOM_IS_ZMAT;
			allname = g_strdup_printf("%s%s%s",data->localdir,G_DIR_SEPARATOR_S,data->datafile);
  			read_ZMatrix_mopac_file_no_add_list(allname);
			set_last_directory(allname);
			g_free(allname);
			break;
    	case GABEDIT_TYPENODE_HIN:
                	MethodeGeom = GEOM_IS_XYZ;
			allname = g_strdup_printf("%s%s%s",data->localdir,G_DIR_SEPARATOR_S,data->datafile);
  			read_hin_file_no_add_list(allname);
			set_last_directory(allname);
			g_free(allname);
        }
}
/********************************************************************************/
static void event_dispatcher(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (!event) return;
 	if(popupshow) return;
	if (event->window == gtk_tree_view_get_bin_window (GTK_TREE_VIEW (widget))
	    && !gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget), event->x, event->y, NULL, NULL, NULL, NULL)) {
		gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)));
	}
	if(gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget), event->x, event->y, &path, NULL, NULL, NULL))
	{
		if(path)
		{
			model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
			gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)), path);
			sprintf(selectedRow ,"%s", gtk_tree_path_to_string(path));
			gtk_tree_model_get_iter (model, &iter, path);
			gtk_tree_path_free(path);
        		if (event->type == GDK_2BUTTON_PRESS &&  ((GdkEventButton *) event)->button == 1)
        		{
   				DataTree* data = NULL;
				gtk_tree_model_get (model, &iter, LIST_DATA, &data, -1);
				if(data) select_row(data);
			}
			else
  			if (event->type == GDK_BUTTON_PRESS && ((GdkEventButton *) event)->button == 2)
			{
   				DataTree* data = NULL;
				gtk_tree_model_get (model, &iter, LIST_DATA, &data, -1);
				if(data) create_popupwin(data);
			}
			else
  			if (event->type == GDK_BUTTON_PRESS && ((GdkEventButton *) event)->button == 3)
			{
				GdkEventButton *bevent = (GdkEventButton *) event;
  				GtkUIManager *manager = GTK_UI_MANAGER(user_data);
				if(atoi(selectedRow) < 0)
				{
					set_sensitive_option(manager, "/MenuRecentProjects/Cut", FALSE);
					set_sensitive_option(manager, "/MenuRecentProjects/Set", FALSE);
					set_sensitive_option(manager, "/MenuRecentProjects/Info", FALSE);
				}
				else
				{
   					DataTree* data = NULL;
					gtk_tree_model_get (model, &iter, LIST_DATA, &data, -1);
					if(data)
					{
						set_sensitive_option(manager, "/MenuRecentProjects/Cut", TRUE);
						set_sensitive_option(manager, "/MenuRecentProjects/Set", TRUE);
						set_sensitive_option(manager, "/MenuRecentProjects/Info", TRUE);
					}
					else
					{
						set_sensitive_option(manager, "/MenuRecentProjects/Cut", FALSE);
						set_sensitive_option(manager, "/MenuRecentProjects/Set", FALSE);
						set_sensitive_option(manager, "/MenuRecentProjects/Info", FALSE);
					}
				}
				show_menu_popup(manager, bevent->button, bevent->time);
			}
		}
	}
	GTK_WIDGET_GET_CLASS(widget)->button_press_event(widget, event);
}
/********************************************************************************/
static void tree_clear_all()
{
	gint i;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeViewProjects));
	GtkTreeStore *store = GTK_TREE_STORE (model);

	if(AllFiles == NULL) return;

	gtk_tree_store_clear(store);

	for(i=0;i<Nfiles;i++) tree_data_destroy (AllFiles[i]);

	g_free(AllFiles);
	AllFiles = NULL;
	Nfiles = 0;

	noeud[GABEDIT_TYPENODE_DEMON]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"DeMon");
	noeud[GABEDIT_TYPENODE_FIREFLY]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"FireFly");
	noeud[GABEDIT_TYPENODE_GAMESS]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Gamess");
	noeud[GABEDIT_TYPENODE_GAUSSIAN]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Gaussian");
	noeud[GABEDIT_TYPENODE_MOLCAS]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Molcas");
	noeud[GABEDIT_TYPENODE_MOLPRO]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Molpro");
	noeud[GABEDIT_TYPENODE_MOPAC]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Mopac");
	noeud[GABEDIT_TYPENODE_MPQC]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"MPQC");
	noeud[GABEDIT_TYPENODE_NWCHEM]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"NWChem");
	noeud[GABEDIT_TYPENODE_PSICODE]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Psicode");
	noeud[GABEDIT_TYPENODE_ORCA]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"ORCA");
	noeud[GABEDIT_TYPENODE_QCHEM]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Q-Chem");
	noeud[GABEDIT_TYPENODE_GABEDIT]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Gabedit");
	noeud[GABEDIT_TYPENODE_XYZ]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"XYZ");
	noeud[GABEDIT_TYPENODE_MOL2]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Mol2");
	noeud[GABEDIT_TYPENODE_PDB]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"PDB");
	noeud[GABEDIT_TYPENODE_TINKER]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Tinker");
	noeud[GABEDIT_TYPENODE_GZMAT]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Gauss zmat");
	noeud[GABEDIT_TYPENODE_MZMAT]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Mopac zmat");
	noeud[GABEDIT_TYPENODE_HIN]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Hyperchem");
	noeud[GABEDIT_TYPENODE_OTHER]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Others");
}
/********************************************************************************/
static void tree_data_destroy_one_node (gint inode)
{
    gint ifdel = 0;
    gint ifn = 0;
    gint i;
    DataTree** tmp=NULL;
    

    if(AllFiles == NULL)
	return;

     for(i=0;i<Nfiles;i++)
     {
     	if(AllFiles[i]->itype == inode)
     	{
		ifdel++;
     		if(AllFiles[i]->projectname)
	 		g_free(AllFiles[i]->projectname);
     		if(AllFiles[i]->localdir)
	 		g_free(AllFiles[i]->localdir);
     		if(AllFiles[i]->remotehost)
	 		g_free(AllFiles[i]->remotehost);
     		if(AllFiles[i]->remoteuser)
	 		g_free(AllFiles[i]->remoteuser);
     		if(AllFiles[i]->remotepass)
	 		g_free(AllFiles[i]->remotepass);
     		if(AllFiles[i]->remotedir)
	 		g_free(AllFiles[i]->remotedir);
     		if(AllFiles[i]->command)
	 		g_free(AllFiles[i]->command);
      		AllFiles[i]->noeud=NULL;
      		AllFiles[i]->projectname=g_strdup("DELETED");
	}
      }

      if(Nfiles>ifdel)
      	tmp = g_malloc((Nfiles-ifdel)*sizeof(DataTree*));
      else
      {
      	Nfiles = 0;
      	if(AllFiles)
		g_free(AllFiles);
      	AllFiles = NULL;
      	return;
      }
      ifn =0;
      for(i=0;i<Nfiles;i++)
      {
     	if(AllFiles[i]->itype != inode)
     	{
         tmp[ifn]=AllFiles[i];
         ifn++;
        }
       }


      Nfiles = ifn;
      if(AllFiles)
      	g_free(AllFiles);
      AllFiles = tmp;

}
/********************************************************************************/
static void tree_clear_one(gint in)
{
    	gint i;
	gboolean expanded[NBNOD];
	gchar* pathString;
	GtkTreeIter iter;

	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeViewProjects));
	GtkTreeStore *store = GTK_TREE_STORE (model);

	if(AllFiles == NULL) return;


	for(i=0;i<NBNOD;i++)
	{
 		expanded[i] =  FALSE;
		if (noeud[i] == NULL) continue;
		pathString = g_strdup_printf("%d", i);
		if(gtk_tree_model_get_iter_from_string (model, &iter, pathString) == TRUE)
		{
			GtkTreePath *path = gtk_tree_path_new_from_string  (pathString);
 			expanded[i] =  gtk_tree_view_row_expanded(GTK_TREE_VIEW(treeViewProjects), path);
			gtk_tree_path_free(path);
		}
		g_free(pathString);
       }

	gtk_tree_store_clear(store);

	tree_data_destroy_one_node (in);

	noeud[GABEDIT_TYPENODE_DEMON]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"DeMon");
	noeud[GABEDIT_TYPENODE_FIREFLY]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"FireFly");
	noeud[GABEDIT_TYPENODE_GAMESS]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Gamess");
	noeud[GABEDIT_TYPENODE_GAUSSIAN]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Gaussian");
	noeud[GABEDIT_TYPENODE_MOLCAS]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Molcas");
	noeud[GABEDIT_TYPENODE_MOLPRO]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Molpro");
	noeud[GABEDIT_TYPENODE_MOPAC]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Mopac");
	noeud[GABEDIT_TYPENODE_MPQC]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"MPQC");
	noeud[GABEDIT_TYPENODE_NWCHEM]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"NWChem");
	noeud[GABEDIT_TYPENODE_PSICODE]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Psicode");
	noeud[GABEDIT_TYPENODE_ORCA]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"ORCA");
	noeud[GABEDIT_TYPENODE_QCHEM]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Q-Chem");
	noeud[GABEDIT_TYPENODE_GABEDIT]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Gabedit");
	noeud[GABEDIT_TYPENODE_XYZ]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"XYZ");
	noeud[GABEDIT_TYPENODE_MOL2]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Mol2");
	noeud[GABEDIT_TYPENODE_PDB]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"PDB");
	noeud[GABEDIT_TYPENODE_TINKER]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Tinker");
	noeud[GABEDIT_TYPENODE_GZMAT]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Gauss zmat");
	noeud[GABEDIT_TYPENODE_MZMAT]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Mopac zmat");
	noeud[GABEDIT_TYPENODE_HIN]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Hyperchem");
	noeud[GABEDIT_TYPENODE_OTHER]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Others");

	for(i=0;i<Nfiles;i++) AddFeuille(GTK_TREE_VIEW(treeViewProjects),noeud[AllFiles[i]->itype],AllFiles[i]);

	for(i=0;i<NBNOD;i++)
	if(expanded[i])
	{
		gchar* pathString = g_strdup_printf("%d", i);
		if(gtk_tree_model_get_iter_from_string (model, &iter, pathString) == TRUE)
		{
			GtkTreePath *path = gtk_tree_path_new_from_string  (pathString);
			gtk_tree_view_expand_to_path(GTK_TREE_VIEW(treeViewProjects), path);
			gtk_tree_path_free(path);
		}
		g_free(pathString);
	}
}
/********************************************************************************/
static GtkTreeIter *tree_clear(GtkTreeIter *parent,gint ifile)
{
    	int i;
    	int in = 0;
	gboolean expanded[NBNOD];
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeViewProjects));
	gchar* pathString;
	GtkTreeStore *store = GTK_TREE_STORE (model);
	GtkTreeIter iter;

	for(i=0;i<NBNOD;i++)
	{
 		expanded[i] =  FALSE;
		if (noeud[i] == NULL) continue;
		pathString = g_strdup_printf("%d", i);
		if(gtk_tree_model_get_iter_from_string (model, &iter, pathString) == TRUE)
		{
			GtkTreePath *path = gtk_tree_path_new_from_string  (pathString);
 			expanded[i] =  gtk_tree_view_row_expanded(GTK_TREE_VIEW(treeViewProjects), path);
			gtk_tree_path_free(path);
		}
		g_free(pathString);
       }

	for(i=0;i<NBNOD;i++)
    		if(noeud[i]==parent)
        	{
			in=i;  
			break;
        	}
	if(AllFiles == NULL) return parent;

	gtk_tree_store_clear(store);

    	tree_data_destroy (AllFiles[ifile]);

	noeud[GABEDIT_TYPENODE_DEMON]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"DeMon");
	noeud[GABEDIT_TYPENODE_FIREFLY]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"FireFly");
	noeud[GABEDIT_TYPENODE_GAMESS]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Gamess");
	noeud[GABEDIT_TYPENODE_GAUSSIAN]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Gaussian");
	noeud[GABEDIT_TYPENODE_MOLCAS]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Molcas");
	noeud[GABEDIT_TYPENODE_MOLPRO]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Molpro");
	noeud[GABEDIT_TYPENODE_MOPAC]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Mopac");
	noeud[GABEDIT_TYPENODE_MPQC]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"MPQC");
	noeud[GABEDIT_TYPENODE_NWCHEM]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"NWChem");
	noeud[GABEDIT_TYPENODE_PSICODE]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Psicode");
	noeud[GABEDIT_TYPENODE_ORCA]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"ORCA");
	noeud[GABEDIT_TYPENODE_QCHEM]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Q-Chem");
	noeud[GABEDIT_TYPENODE_GABEDIT]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Gabedit");
	noeud[GABEDIT_TYPENODE_XYZ]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"XYZ");
	noeud[GABEDIT_TYPENODE_MOL2]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Mol2");
	noeud[GABEDIT_TYPENODE_PDB]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"PDB");
	noeud[GABEDIT_TYPENODE_TINKER]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Tinker");
	noeud[GABEDIT_TYPENODE_GZMAT]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Gauss zmat");
	noeud[GABEDIT_TYPENODE_MZMAT]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Mopac zmat");
	noeud[GABEDIT_TYPENODE_HIN]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Hyperchem");
	noeud[GABEDIT_TYPENODE_OTHER]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Others");

    	for(i=0;i<Nfiles;i++) AddFeuille(GTK_TREE_VIEW(treeViewProjects),noeud[AllFiles[i]->itype],AllFiles[i]);

	for(i=0;i<NBNOD;i++)
	if(expanded[i])
	{
		gchar* pathString = g_strdup_printf("%d", i);
		if(gtk_tree_model_get_iter_from_string (model, &iter, pathString) == TRUE)
		{
			GtkTreePath *path = gtk_tree_path_new_from_string  (pathString);
			gtk_tree_view_expand_to_path(GTK_TREE_VIEW(treeViewProjects), path);
			gtk_tree_path_free(path);
		}
		g_free(pathString);
	}

	return noeud[in];
    
}
/********************************************************************************/
static void tree_data_destroy (DataTree* pfd)
{
    int ifn=0;
    int i;
    DataTree** tmp=NULL;
    

    if(AllFiles == NULL)
	return;
    if(pfd == NULL)
	return;

     if(pfd->projectname) g_free(pfd->projectname);
     if(pfd->localdir) g_free(pfd->localdir);
     if(pfd->remotehost) g_free(pfd->remotehost);
     if(pfd->remoteuser) g_free(pfd->remoteuser);
     if(pfd->remotepass) g_free(pfd->remotepass);
     if(pfd->remotedir) g_free(pfd->remotedir);
     if(pfd->command) g_free(pfd->command);

      pfd->noeud=NULL;
      pfd->projectname=g_strdup("DELETED");

      if(Nfiles>1) tmp = g_malloc((Nfiles-1)*sizeof(DataTree*));
      else
      {
      	Nfiles--;
      	if(AllFiles) g_free(AllFiles);
      	AllFiles = NULL;
      	return;
      }
      ifn =0;
      for(i=0;i<Nfiles;i++)
        if(AllFiles[i] != pfd)
        {
         	tmp[ifn]=AllFiles[i];
         	ifn++;
        }

      Nfiles--;

      if(AllFiles) g_free(AllFiles);
      AllFiles = tmp;

}
/********************************************************************************/
static DataTree* tree_data_new (gchar *projectname,gchar* datafile,gchar* localdir,
		gchar* remotehost,gchar* remoteuser,gchar* remotepass,gchar* remotedir,gint itype, gchar* command, GabEditNetWork netWorkProtocol)
{
    DataTree *pfd;

    pfd = g_malloc (sizeof (DataTree));
 
     pfd->projectname = g_strdup(projectname);
     pfd->datafile = g_strdup(datafile);
     pfd->localdir = g_strdup(localdir);
     if(remotehost) pfd->remotehost = g_strdup(remotehost);
     else pfd->remotehost = g_strdup("");

     if(remoteuser) pfd->remoteuser = g_strdup(remoteuser);
     else pfd->remoteuser = g_strdup("");

     if(remotepass) pfd->remotepass = g_strdup(remotepass);
     else pfd->remotepass = g_strdup("");

     if(remotedir) pfd->remotedir = g_strdup(remotedir);
     else pfd->remotedir = g_strdup("");

     if(command) pfd->command = g_strdup(command);
     else pfd->command = g_strdup("");

     pfd->itype =itype;
     pfd->netWorkProtocol = netWorkProtocol;

     return pfd;
}
/********************************************************************************/
static GtkTreeIter* CreeNoeud(GtkTreeView *treeView,gchar *text)
{
	gchar* t = g_strdup(text);
	GtkTreeIter* node = g_malloc(sizeof(GtkTreeIter));
	GtkTreeModel *model;
        GtkTreeStore *store;
	GdkColor nodeColor;

	nodeColor.red = 0;
	nodeColor.green = 0;
	nodeColor.blue = 65535;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
        store = GTK_TREE_STORE (model);

	gtk_tree_store_append(store, node, NULL);
    	uppercase(t);
       	gtk_tree_store_set (store, node, LIST_NAME, t, -1);
	if(strstr(t,"FIREFLY")) gtk_tree_store_set (store, node, LIST_PIXBUF, fireflyPixbuf, -1);
	else
	if(strstr(t,"GAMESS")) gtk_tree_store_set (store, node, LIST_PIXBUF, gamessPixbuf, -1);
	else
	if(strstr(t,"DEMON")) gtk_tree_store_set (store, node, LIST_PIXBUF, demonPixbuf, -1);
	else
	if(strstr(t,"GAUSSIAN")) gtk_tree_store_set (store, node, LIST_PIXBUF, gaussianPixbuf, -1);
	else
	if(strstr(t,"MOLCAS")) gtk_tree_store_set (store, node, LIST_PIXBUF, molcasPixbuf, -1);
	else
	if(strstr(t,"MOLPRO")) gtk_tree_store_set (store, node, LIST_PIXBUF, molproPixbuf, -1);
	else
	if(strstr(t,"MPQC")) gtk_tree_store_set (store, node, LIST_PIXBUF, mpqcPixbuf, -1);
	else
	if(strstr(t,"NWCHEM")) gtk_tree_store_set (store, node, LIST_PIXBUF, nwchemPixbuf, -1);
	else
	if(strstr(t,"PSICODE")) gtk_tree_store_set (store, node, LIST_PIXBUF, psicodePixbuf, -1);
	else
	if(strstr(t,"ORCA")) gtk_tree_store_set (store, node, LIST_PIXBUF, orcaPixbuf, -1);
	else
	if(strstr(t,"Q-CHEM")) gtk_tree_store_set (store, node, LIST_PIXBUF, qchemPixbuf, -1);
	else
	if(strstr(t,"MOPAC Z")) gtk_tree_store_set (store, node, LIST_PIXBUF, bookPixbuf, -1);
	else
	if(strstr(t,"MOPAC")) gtk_tree_store_set (store, node, LIST_PIXBUF, mopacPixbuf, -1);
	else
	if(strstr(t,"GABEDIT")) gtk_tree_store_set (store, node, LIST_PIXBUF, gabeditPixbuf, -1);
	else
		gtk_tree_store_set (store, node, LIST_PIXBUF, bookPixbuf, -1);

	gtk_tree_store_set (store, node, LIST_NODE_FOREGROUND_COLOR, &nodeColor, -1);
       	gtk_tree_store_set (store, node, LIST_DATA, NULL, -1);
	g_free(t);
	return node;
}
/********************************************************************************/
static int file_in_list(gchar *projectname,gchar*datafile,gchar* localdir,gint itype)
{
  int i;
  for(i=0;i<Nfiles;i++)
  if( 	!strcmp(localdir,AllFiles[i]->localdir) && 
	!strcmp(projectname,AllFiles[i]->projectname) && 
	!strcmp(datafile,AllFiles[i]->datafile) && 
	itype == AllFiles[i]->itype) 
	{
		if(AllFiles[i]->remotehost)
			g_free(AllFiles[i]->remotehost);
		if(fileopen.remotehost)
			AllFiles[i]->remotehost = g_strdup(fileopen.remotehost);
		else
			AllFiles[i]->remotehost =NULL;
		if(fileopen.remoteuser)
			AllFiles[i]->remoteuser = g_strdup(fileopen.remoteuser);
		else
			AllFiles[i]->remoteuser =g_strdup("");
		if(fileopen.remotepass)
			AllFiles[i]->remotepass = g_strdup(fileopen.remotepass);
		else
			AllFiles[i]->remotepass =g_strdup("");
		if(fileopen.remotedir)
			AllFiles[i]->remotedir = g_strdup(fileopen.remotedir);
		else
			AllFiles[i]->remotedir =g_strdup("");
		if(fileopen.command)
			AllFiles[i]->command = g_strdup(fileopen.command);
		else
			AllFiles[i]->command =g_strdup("");
    		return i;
	}
  return -1;
}
/********************************************************************************/
static void AddFeuille(GtkTreeView *treeView, GtkTreeIter *parent,DataTree *pfd)
{
	GtkTreeIter feuille;
	gchar *text;
	GtkTreeModel *model;
        GtkTreeStore *store;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
        store = GTK_TREE_STORE (model);

	text = g_strdup(pfd->projectname);

	gtk_tree_store_append(store, &feuille, parent);
	gtk_tree_store_set (store, &feuille, LIST_PIXBUF, pagePixbuf, -1);
       	gtk_tree_store_set (store, &feuille, LIST_NAME, text, -1);
	pfd->noeud = &feuille;
       	gtk_tree_store_set (store, &feuille, LIST_DATA, pfd, -1);

	g_free(text);
}
/********************************************************************************/
void CreeFeuille(GtkWidget *treeView, GtkTreeIter *parent,gchar* projectname,gchar*datafile,gchar* localdir,
				  gchar* remotehost,gchar* remoteuser,gchar* remotepass,gchar* remotedir,gint itype, gchar* command, GabEditNetWork netWorkProtocol)
{
	DataTree* pfd;
	int ifile =  file_in_list(projectname,datafile,localdir,itype);

	if( itype<0 || itype>=NBNOD) return;
	if( ifile != -1 )
	{
		if(AllFiles[ifile]->itype==iprogram) return;
		else parent=tree_clear(parent,ifile);
	}
	pfd = tree_data_new (projectname,datafile,localdir,remotehost,remoteuser,remotepass,remotedir,itype, command, netWorkProtocol);

	AddFeuille(GTK_TREE_VIEW(treeView),parent,pfd);

	if(Nfiles==0) AllFiles = g_malloc(sizeof(DataTree*));
	else AllFiles = (DataTree**)g_realloc(AllFiles,(Nfiles+1)*sizeof(DataTree*));
	Nfiles++;
	AllFiles[Nfiles-1] = pfd;
}
/********************************************************************************/
static void clear_one_project()
{
	int ifile =  -1;
	GtkTreeIter node;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeViewProjects));
   	DataTree* data = NULL;

	if(atoi(selectedRow)<0) return;
	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);
		
        if(!data) return;
  	if( data->itype<0 || data->itype>=NBNOD) return;
	ifile =  file_in_list(data->projectname, data->datafile, data->localdir, data->itype);

  	if( ifile != -1 )
  	{
		GtkTreeIter* parent;
		parent = noeud[data->itype];
       		parent = tree_clear(parent,ifile);
  	}
}
/********************************************************************************/
static void to_clear_lists(GtkWidget *wid,gpointer data)
{
  gint i;
  gint k = 0;
  for(i=0;i<NBNOD;i++)
    		if (GTK_TOGGLE_BUTTON (checkbutton[i])->active) 
			k++;
  if(k == NBNOD)
 	tree_clear_all();
  else
  if(k != 0)
  	for(i=0;i<NBNOD;i++)
    		if (GTK_TOGGLE_BUTTON (checkbutton[i])->active) 
			tree_clear_one(i);
	
}
/********************************************************************************/
static void select_all_buttons(GtkWidget *wid,gpointer data)
{
  gint i;
  for(i=0;i<NBNOD;i++)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton[i]), TRUE);
}
/********************************************************************************/
static void unselect_all_buttons(GtkWidget *wid,gpointer data)
{
  gint i;
  for(i=0;i<NBNOD;i++)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton[i]), FALSE);
}
/********************************************************************************/
static void  create_window_list_to_clear()
{
  guint i;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *Dialogue;
  GtkWidget *button;
  gchar * title = N_("Clear list of projects");
  gchar *buttonlabel[NBNOD]={
  		"deMon2k list",
  		"FireFly list",
  		"Gamess list",
  		"Gaussian list",
  		"Molcas list",
  		"Molpro list",
  		"Mopac list",
  		"MPQC list",
  		"NWChem list",
  		"Psicode list",
  		"Orca list",
  		"Q-Chem list",
  		"Gabedit list",
  		"xyz list",
  		"Mol2 list",
  		"PDB list",
  		"Tinker list",
  		"Gaussian z-matrix list",
  		"Mopac z-matrix list",
  		"HyperChem (hin) list",
  		"Other files list",
  		};
  GtkWidget *ButtonSelAll;
  GtkWidget *ButtonUnSelAll;

  for(i=0;i<NBNOD;i++)
	checkbutton[i]=NULL;

  Dialogue = gtk_dialog_new();
  gtk_widget_realize(GTK_WIDGET(Dialogue));
  gtk_window_set_title(GTK_WINDOW(Dialogue),title);
  gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(Fenetre));
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);

  g_signal_connect(G_OBJECT(Dialogue),"delete_event",(GCallback)destroy_button_windows,NULL);
  g_signal_connect(G_OBJECT(Dialogue),"delete_event",(GCallback)gtk_widget_destroy,NULL);

  frame = gtk_frame_new (title);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG(Dialogue)->vbox), frame, TRUE, TRUE, 8);
  gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

  vbox = gtk_vbox_new (TRUE, 0);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  /* Select all and unselect all */
   hbox =  gtk_hbox_new (TRUE, 0);
   gtk_widget_show (hbox);
   gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

   ButtonUnSelAll = gtk_radio_button_new_with_label( NULL,_(" Unselect all") );
   gtk_box_pack_end (GTK_BOX (hbox), ButtonUnSelAll, TRUE, TRUE, 0);
   gtk_widget_show (ButtonUnSelAll);

   ButtonSelAll = gtk_radio_button_new_with_label(
                       gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonUnSelAll)),
                       _("Select all ")); 
   gtk_box_pack_start (GTK_BOX (hbox), ButtonSelAll, TRUE, TRUE, 0);
   gtk_widget_show (ButtonSelAll);
   create_hseparator(vbox);

  for(i=0;i<NBNOD;i++)
  {
  	checkbutton[i] = gtk_check_button_new_with_label (buttonlabel[i]);
  	gtk_widget_show (checkbutton[i]);
  	gtk_box_pack_start (GTK_BOX (vbox), checkbutton[i], FALSE, FALSE, 0);
  }
  g_signal_connect(G_OBJECT(ButtonSelAll), "clicked",(GCallback)select_all_buttons,NULL);
  g_signal_connect(G_OBJECT(ButtonUnSelAll), "clicked",(GCallback)unselect_all_buttons,NULL);
  gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Dialogue)->action_area), FALSE);

  /* The CANCEL button */
  button = create_button(Dialogue,_("Cancel"));
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dialogue)->action_area), button, FALSE, TRUE, 5);  
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)destroy_button_windows,GTK_OBJECT(Dialogue));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(Dialogue));

    /* The OK button */
  button = create_button(Dialogue,"OK");
  gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Dialogue)->action_area), button, FALSE, TRUE, 5);  
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  g_signal_connect(G_OBJECT(button), "clicked",(GCallback)to_clear_lists,NULL);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)destroy_button_windows,GTK_OBJECT(Dialogue));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(Dialogue));


  if(atoi(selectedRow)>-1)
  {
  	gint i;
	GtkTreeIter node;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeViewProjects));
	gtk_tree_model_get_iter_from_string (model, &node, selectedRow);
	GtkTreePath* path1 = gtk_tree_model_get_path(model, &node);
	GtkTreePath* path2;

	gboolean Ok = FALSE;
  	for(i=0;i<NBNOD;i++)
  	{
		path2 = gtk_tree_model_get_path(model, noeud[i]);
		if(gtk_tree_path_compare(path1, path2)==0)
		{
    			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton[i]), TRUE);
			Ok = TRUE;
			break;
		}
		gtk_tree_path_free(path2);
  	}
	gtk_tree_path_free(path1);
	if(!Ok)
	{
		DataTree* p;
		gtk_tree_model_get (model, &node, LIST_DATA, &p, -1);
		if(p)
		{
  			if( p->itype>= 0 && p->itype<NBNOD)
    				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton[p->itype]), TRUE);
		}
	}
  }

   add_button_windows(title,Dialogue);
   gtk_widget_show_all(Dialogue);

}
/********************************************************************************/
void ListeFiles(GtkWidget* vbox)
{                                  
	GtkWidget *Scr;
	GtkUIManager *manager = NULL;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	set_pixbuf();

	Scr = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (Scr), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 
	gtk_container_add(GTK_CONTAINER(vbox), Scr);      
	gtk_widget_set_size_request(Scr, (gint)(ScreenHeight*0.15), (gint)(ScreenHeight*0.1));
	FrameList = vbox;

	store = gtk_tree_store_new (4, GDK_TYPE_PIXBUF, G_TYPE_STRING,  GDK_TYPE_COLOR, G_TYPE_POINTER);
        model = GTK_TREE_MODEL (store);

	treeViewProjects = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeViewProjects), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeViewProjects), TRUE);

	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Recent Projects"));
	gtk_tree_view_column_set_reorderable(column, TRUE);

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute (column, renderer, "pixbuf", LIST_PIXBUF);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer, "text", LIST_NAME);
	gtk_tree_view_column_add_attribute (column, renderer, "foreground_gdk", LIST_NODE_FOREGROUND_COLOR);

	gtk_tree_view_append_column (GTK_TREE_VIEW (treeViewProjects), column);

	gtk_container_add(GTK_CONTAINER(Scr), treeViewProjects);
  
	noeud[GABEDIT_TYPENODE_DEMON]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"DeMon");
	noeud[GABEDIT_TYPENODE_FIREFLY]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"FireFly");
	noeud[GABEDIT_TYPENODE_GAMESS]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Gamess");
	noeud[GABEDIT_TYPENODE_GAUSSIAN]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Gaussian");
	noeud[GABEDIT_TYPENODE_MOLCAS]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Molcas");
	noeud[GABEDIT_TYPENODE_MOLPRO]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Molpro");
	noeud[GABEDIT_TYPENODE_MOPAC]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Mopac");
	noeud[GABEDIT_TYPENODE_MPQC]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"MPQC");
	noeud[GABEDIT_TYPENODE_NWCHEM]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"NWChem");
	noeud[GABEDIT_TYPENODE_PSICODE]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Psicode");
	noeud[GABEDIT_TYPENODE_ORCA]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"ORCA");
	noeud[GABEDIT_TYPENODE_QCHEM]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Q-Chem");
	noeud[GABEDIT_TYPENODE_GABEDIT]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Gabedit");
	noeud[GABEDIT_TYPENODE_XYZ]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"XYZ");
	noeud[GABEDIT_TYPENODE_MOL2]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Mol2");
	noeud[GABEDIT_TYPENODE_PDB]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"PDB");
	noeud[GABEDIT_TYPENODE_TINKER]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Tinker");
	noeud[GABEDIT_TYPENODE_GZMAT]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Gauss zmat");
	noeud[GABEDIT_TYPENODE_MZMAT]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Mopac zmat");
	noeud[GABEDIT_TYPENODE_HIN]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Hyperchem");
	noeud[GABEDIT_TYPENODE_OTHER]=CreeNoeud(GTK_TREE_VIEW(treeViewProjects),"Others");

	add_liste_files();

	manager = create_menu(Fenetre);
	g_signal_connect(treeViewProjects, "button_press_event", G_CALLBACK(event_dispatcher), manager);
}
/********************************************************************************/
