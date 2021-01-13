/*HydrogenBond.c*/
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
#include <math.h>

#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/AtomsProp.h"


#include "../Geometry/GeomGlobal.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Common/Windows.h"
#include "../Display/GLArea.h"
#include "../Common/StockIcons.h"

static gint nAtomsCanConnect = 6;
static gchar** atomsCanConnect = NULL;
static gdouble minDistance = 1.50; /* in Agnstrom */
static gdouble maxDistance = 3.15; /* in Agnstrom */ 
static gdouble minAngle = 145.0;
static gdouble maxAngle = 215.0;
static gint rowSelected = -1;
static	GtkTreeView *treeViewOfAtoms = NULL;

static void rafreshTreeView(GtkTreeView *treeView);
/************************************************************************/
void initHBonds()
{
	nAtomsCanConnect = 6;
	atomsCanConnect = g_malloc(nAtomsCanConnect*sizeof(gchar*));
	atomsCanConnect[0] = g_strdup("N");
	atomsCanConnect[1] = g_strdup("O");
	atomsCanConnect[2] = g_strdup("F");
	atomsCanConnect[3] = g_strdup("Cl");
	atomsCanConnect[4] = g_strdup("Br");
	atomsCanConnect[5] = g_strdup("I");
	minDistance = 1.50; /* in Agnstrom */
	maxDistance = 3.15; /* in Agnstrom */ 
	minAngle = 145.0;
	maxAngle = 215.0;
}
/******************************************************************/
void save_HBonds_properties()
{
	gchar *hbondsfile;
	FILE *file;
	gint i;

	hbondsfile = g_strdup_printf("%s%shbonds",gabedit_directory(),G_DIR_SEPARATOR_S);

	file = FOpen(hbondsfile, "w");
	if(!file) return;

 	fprintf(file,"%f\n",minDistance);
 	fprintf(file,"%f\n",maxDistance);
 	fprintf(file,"%f\n",minAngle);
 	fprintf(file,"%f\n",maxAngle);
 	fprintf(file,"%d\n",nAtomsCanConnect);
	for(i=0;i<nAtomsCanConnect;i++) fprintf(file,"%s\n",atomsCanConnect[i]);
	fclose(file);

	g_free(hbondsfile);
}
/******************************************************************/
void read_HBonds_properties()
{
	gchar *hbondsfile;
	FILE *file;
	gint n;
	gint i;

	initHBonds();
	hbondsfile = g_strdup_printf("%s%shbonds",gabedit_directory(),G_DIR_SEPARATOR_S);

	file = FOpen(hbondsfile, "rb");
	if(!file) return;

 	n = fscanf(file,"%lf\n",&minDistance);
	if(n != 1) { initHBonds(); return ; fclose(file); g_free(hbondsfile);}

 	n = fscanf(file,"%lf\n",&maxDistance);
	if(n != 1) { initHBonds(); return ; fclose(file); g_free(hbondsfile);}

 	n = fscanf(file,"%lf\n",&minAngle);
	if(n != 1) { initHBonds(); return ; fclose(file); g_free(hbondsfile);}
 	n = fscanf(file,"%lf\n",&maxAngle);
	if(n != 1) { initHBonds(); return ; fclose(file); g_free(hbondsfile);}

 	n = fscanf(file,"%d\n",&nAtomsCanConnect);
	if(n != 1 || nAtomsCanConnect<0 ) { initHBonds(); return ; fclose(file); g_free(hbondsfile);}

	for(i=0;i<nAtomsCanConnect;i++)
	{
 		n = fscanf(file,"%s\n",atomsCanConnect[i]);
		if(n != 1) { initHBonds(); return ; fclose(file); g_free(hbondsfile);}
		delete_last_spaces(atomsCanConnect[i]);
		delete_first_spaces(atomsCanConnect[i]);
		str_delete_n(atomsCanConnect[i]);
	}

	fclose(file);

	g_free(hbondsfile);
}
/******************************************************************/
static void set_HBonds(GtkWidget* fp,gpointer data)
{
	GtkWidget** entrys = (GtkWidget**)data;
	G_CONST_RETURN gchar* tentry;
	gchar* t;

	tentry = gtk_entry_get_text(GTK_ENTRY(entrys[0]));
	minDistance = atof(tentry);
	if(minDistance<1e-6)minDistance = 1.5;

	tentry = gtk_entry_get_text(GTK_ENTRY(entrys[1]));
	maxDistance = atof(tentry);
	if(maxDistance<1e-6)maxDistance = 3.5;
	if(maxDistance<=minDistance) maxDistance = minDistance + 1.0;

	tentry = gtk_entry_get_text(GTK_ENTRY(entrys[2]));
	minAngle = atof(tentry);
	if(minAngle<1e-6)minAngle = 145.0;

	tentry = gtk_entry_get_text(GTK_ENTRY(entrys[3]));
	maxAngle = atof(tentry);
	if(maxAngle<1e-6)maxAngle = 215.0;
	if(maxAngle<=minAngle) maxAngle = minAngle + 70.0;

	t = g_strdup_printf("%f",minDistance);	
	gtk_entry_set_text(GTK_ENTRY(entrys[0]),t);
	g_free(t);

	t = g_strdup_printf("%f",maxDistance);	
	gtk_entry_set_text(GTK_ENTRY(entrys[1]),t);
	g_free(t);

	t = g_strdup_printf("%f",minAngle);
	gtk_entry_set_text(GTK_ENTRY(entrys[2]),t);
	g_free(t);

	t = g_strdup_printf("%f",maxAngle);
	gtk_entry_set_text(GTK_ENTRY(entrys[3]),t);
	g_free(t);


	rafresh_window_orb();
        if(GeomDrawingArea != NULL) draw_geometry(NULL,NULL);
}
/******************************************************************/
static void deleteAnAtomDlg()
{

	gint j;
	gint k = rowSelected;

	if(k<0 || k >= nAtomsCanConnect) return;
	if(!atomsCanConnect) return;
	if(nAtomsCanConnect == 1)
	{
		GtkWidget* win;
		win = Message(_("Sorry, You can not delete all atoms"),_("Warning"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(win),TRUE);
		return;
	}

	for(j=k;j<nAtomsCanConnect-1;j++)
	{
		if(atomsCanConnect[j])
		{
			g_free(atomsCanConnect[j]);
			atomsCanConnect[j] = g_strdup(atomsCanConnect[j+1]);
		}
		else break;
	}
	nAtomsCanConnect--;
	atomsCanConnect = g_realloc(atomsCanConnect, nAtomsCanConnect*sizeof(gchar*));

	rafreshTreeView(treeViewOfAtoms);
	return;
}
/********************************************************************************/
static void addAtom(GtkWidget *button,gpointer data)
{
	GtkWidget* winTable = g_object_get_data(G_OBJECT(button),"WinTable");
	gchar* atomToInsert = (gchar*)data;
	gint i;

	gtk_widget_destroy(winTable);
	for(i=0;i<nAtomsCanConnect;i++) if(strcmp(atomToInsert,atomsCanConnect[i])==0) return;

	if(nAtomsCanConnect==0) atomsCanConnect = g_malloc(sizeof(gchar*));
	else atomsCanConnect = g_realloc(atomsCanConnect,(nAtomsCanConnect+1)*sizeof(gchar*));
	atomsCanConnect[nAtomsCanConnect] = g_strdup(atomToInsert);
	nAtomsCanConnect++;
	rafreshTreeView(treeViewOfAtoms);
}
/********************************************************************************/
static void addAnAtomDlg()
{
	GtkWidget* table;
	GtkWidget* button;
	GtkWidget* frame;
	GtkWidget* winTable;
	guint i;
	guint j;
        GtkStyle *button_style;
        GtkStyle *style;


	gchar*** Symb = get_periodic_table();

  winTable = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(winTable),TRUE);
  gtk_window_set_title(GTK_WINDOW(winTable),_("Select your atom"));
  gtk_window_set_default_size (GTK_WINDOW(winTable),(gint)(ScreenWidth*0.5),(gint)(ScreenHeight*0.4));

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

  gtk_container_add(GTK_CONTAINER(winTable),frame);  
  gtk_widget_show (frame);

  table = gtk_table_new(PERIODIC_TABLE_N_ROWS-1,PERIODIC_TABLE_N_COLUMNS,TRUE);
  gtk_container_add(GTK_CONTAINER(frame),table);
  button_style = gtk_widget_get_style(winTable); 
  
  for ( i = 0;i<PERIODIC_TABLE_N_ROWS-1;i++)
	  for ( j = 0;j<PERIODIC_TABLE_N_COLUMNS;j++)
  {
	  if(strcmp(Symb[j][i],"00"))
	  {
	  button = gtk_button_new_with_label(Symb[j][i]);
          style=set_button_style(button_style,button,Symb[j][i]);
          g_signal_connect(G_OBJECT(button), "clicked",
                            (GCallback)addAtom,(gpointer )Symb[j][i]);
	  g_object_set_data(G_OBJECT(button),"WinTable",winTable);
	  gtk_table_attach(GTK_TABLE(table),button,j,j+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	  }

  }
 	
  gtk_widget_show_all(winTable);
  
}
/******************************************************************/
static void setDefaultAtomsDlg()
{
	if(atomsCanConnect)
	{
		gint i;
		for(i=0;i<nAtomsCanConnect;i++)
			if(atomsCanConnect[i]) g_free(atomsCanConnect[i]);
		g_free(atomsCanConnect);
	}
	nAtomsCanConnect = 6;
	atomsCanConnect = g_malloc(nAtomsCanConnect*sizeof(gchar*));
	atomsCanConnect[0] = g_strdup("N");
	atomsCanConnect[1] = g_strdup("O");
	atomsCanConnect[2] = g_strdup("F");
	atomsCanConnect[3] = g_strdup("Cl");
	atomsCanConnect[4] = g_strdup("Br");
	atomsCanConnect[5] = g_strdup("I");
	rafreshTreeView(treeViewOfAtoms);
}
/********************************************************************************/
static void set_sensitive_option(GtkUIManager *manager, gchar* path)
{
	GtkWidget *wid = gtk_ui_manager_get_widget (manager, path);
	gboolean sensitive = TRUE;
  	if(nAtomsCanConnect<2) sensitive = FALSE;
	if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid, sensitive);
}
/**********************************************************************************/
static gboolean show_menu_popup(GtkUIManager *manager, guint button, guint32 time)
{
	GtkWidget *menu = gtk_ui_manager_get_widget (manager, "/MenuHBonds");
	if (GTK_IS_MENU (menu)) 
	{
		set_sensitive_option(manager,"/MenuHBonds/DeleteAtom");
		gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, button, time);
		return TRUE;
	}
	return FALSE;
}
/*********************************************************************************************************************/
static void activate_action (GtkAction *action)
{
	const gchar *name = gtk_action_get_name (action);
	if(!strcmp(name, "DeleteAtom")) deleteAnAtomDlg();
	else if(!strcmp(name, "AddAtom")) addAnAtomDlg();
	else if(!strcmp(name, "DefaultAtoms")) setDefaultAtomsDlg();
}
/*--------------------------------------------------------------------*/
static GtkActionEntry gtkActionEntries[] =
{
	{"DeleteAtom", GABEDIT_STOCK_CUT, N_("_Delete selected atom"), NULL, "Delete selected atom", G_CALLBACK (activate_action) },
	{"AddAtom", GABEDIT_STOCK_NEW, N_("_Add an atom"), NULL, "Add an atom", G_CALLBACK (activate_action) },
	{"DefaultAtoms", NULL, N_("_Default atoms"), NULL, "Default atoms", G_CALLBACK (activate_action) },
};
static guint numberOfGtkActionEntries = G_N_ELEMENTS (gtkActionEntries);
/********************************************************************************/
/* XML description of the menus for the test app.  The parser understands
 * a subset of the Bonobo UI XML format, and uses GMarkup for parsing */
static const gchar *uiMenuInfo =
"  <popup name=\"MenuHBonds\">\n"
"    <separator name=\"sepMenuPopDelete\" />\n"
"    <menuitem name=\"DeleteAtom\" action=\"DeleteAtom\" />\n"
"    <separator name=\"sepMenuPopAdd\" />\n"
"    <menuitem name=\"AddAtom\" action=\"AddAtom\" />\n"
"    <separator name=\"sepMenuPopDefault\" />\n"
"    <menuitem name=\"DefaultAtoms\" action=\"DefaultAtoms\" />\n"
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

	actionGroup = gtk_action_group_new ("GabeditHBonds");
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
/*******************************************************************************************************************************/
static void rafreshTreeView(GtkTreeView *treeView)
{
	gint i;

        GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(treeView);
        GtkTreeStore *store = GTK_TREE_STORE (model);
  
	gtk_tree_store_clear(store);
        model = GTK_TREE_MODEL (store);

	for(i=0;i<nAtomsCanConnect;i++)
	{
		gchar* string = g_strdup_printf("%s",atomsCanConnect[i]);
        	gtk_tree_store_append (store, &iter, NULL);
        	gtk_tree_store_set (store, &iter, 0, string, -1);
		g_free(string);
	}

	if(nAtomsCanConnect>0)
	{
		GtkTreePath *path;

		rowSelected = 0;
		path = gtk_tree_path_new_from_string  ("0");
		gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeView)), path);
		gtk_tree_path_free(path);
	}
}
/*******************************************************************************************************************************/
static void event_dispatcher2(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{

	GtkTreePath *path;
	gint row = -1;
	if (event->window == gtk_tree_view_get_bin_window (GTK_TREE_VIEW (widget))
	    && !gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget),
					       event->x, event->y, NULL, NULL, NULL, NULL)) {
		gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)));
	}
	if(gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget), event->x, event->y, &path, NULL, NULL, NULL))
	{
		if(path)
		{
			gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)), path);
			row = atoi(gtk_tree_path_to_string(path));
			gtk_tree_path_free(path);
		}
	}

	rowSelected = row;
	if(row<0) return;
  	if (event->type == GDK_BUTTON_PRESS && ((GdkEventButton *) event)->button == 3)
	{
		GdkEventButton *bevent = (GdkEventButton *) event;
		GtkUIManager *manager = GTK_UI_MANAGER(user_data);
		show_menu_popup(manager, bevent->button, bevent->time);
	}
}
/********************************************************************************/
static GtkTreeView* addListOfAtoms(GtkWidget *vbox, GtkUIManager *manager)
{
	GtkWidget *scr;
	gint i;
	gint widall=0;
	gint widths[]={10};
	gint Factor=7;
	gint len = 1;

        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeView *treeView;
	GtkTreeViewColumn *column;
  

	store = gtk_tree_store_new (1,G_TYPE_STRING);
        model = GTK_TREE_MODEL (store);

	for(i=0;i<len;i++) widall+=widths[i];

	widall=widall*Factor+40;

	scr=gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_set_size_request(scr,widall,(gint)(ScreenHeight*0.1));
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 
	gtk_box_pack_start(GTK_BOX (vbox), scr,TRUE, TRUE, 2);

	treeView = (GtkTreeView*)gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeView), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeView), FALSE);
  	gtk_container_add(GTK_CONTAINER(scr),GTK_WIDGET(treeView));

	for (i=0;i<len;i++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, "There atoms can do an hydrogen bonds");
		gtk_tree_view_column_set_min_width(column, widths[i]*Factor);
		gtk_tree_view_column_set_reorderable(column, TRUE);
		{
			GtkWidget*  t = gtk_tree_view_column_get_widget (column);
			if(t) gtk_widget_hide(t);
		}

		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, TRUE);
		gtk_tree_view_column_set_attributes (column, renderer, "text", 0, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (treeView), column);
	}
  
  	set_base_style(GTK_WIDGET(treeView),55000,55000,55000);
	gtk_widget_show (GTK_WIDGET(treeView));

	g_signal_connect (treeView, "button_press_event", G_CALLBACK (event_dispatcher2), manager);

	return treeView;
}
/******************************************************************/
void set_HBonds_dialog (GtkWidget* winParent)
{
	GtkWidget *fp;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget *vboxframe;
	GtkWidget *hbox;
	GtkWidget *button;
	GtkWidget *hseparator;
	GtkWidget *label;
	static GtkWidget* entrys[4];
	gchar* tlabel[4]={N_("Min Distance (Angstroms)"),N_("Max Distance (Angstroms)"),N_("Min angle (degrees)"),N_("Max angle (degrees)")};
	gint i;
	GtkWidget* table;
	gchar* t = NULL;
	GtkUIManager *manager = NULL;

	if(!winParent || !GTK_IS_WIDGET(winParent)) return;

	fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_modal(GTK_WINDOW(fp),TRUE);
	gtk_window_set_title(GTK_WINDOW(fp),_("Set the hydrogen's bonds parameters"));
	gtk_container_set_border_width (GTK_CONTAINER (fp), 5);

	gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
	gtk_window_set_modal (GTK_WINDOW (fp), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(winParent));

	g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)gtk_widget_destroy,NULL);

	vboxall = create_vbox(fp);
	frame = gtk_frame_new (NULL);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	table = gtk_table_new(10,3,FALSE);
	gtk_box_pack_start(GTK_BOX(vboxframe), table,TRUE,TRUE,0);

	for(i=0;i<4;i++)
	{
		add_label_table(table,tlabel[i],(gushort)i,0);
		add_label_table(table," : ",(gushort)i,1); 

		entrys[i] = gtk_entry_new ();
		gtk_table_attach(GTK_TABLE(table),entrys[i],2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
	}
	t = g_strdup_printf("%f",minDistance);
	gtk_entry_set_text(GTK_ENTRY(entrys[0]),t);
	g_free(t);
	t = g_strdup_printf("%f",maxDistance);
	gtk_entry_set_text(GTK_ENTRY(entrys[1]),t);
	g_free(t);
	t = g_strdup_printf("%f",minAngle);
	gtk_entry_set_text(GTK_ENTRY(entrys[2]),t);
	g_free(t);
	t = g_strdup_printf("%f",maxAngle);
	gtk_entry_set_text(GTK_ENTRY(entrys[3]),t);
	g_free(t);

	i = 4;
	hseparator = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),hseparator,0,0+3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
	i = 5;
	hbox = gtk_hbox_new(0,FALSE);
	label = gtk_label_new (_("  There atoms can do an hydrogen bonds.\n  Use right button of mouse to modify this list."));
   	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_table_attach(GTK_TABLE(table),hbox,0,0+3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
	i = 6;
  	hbox = gtk_hbox_new (TRUE, 0);
	gtk_table_attach(GTK_TABLE(table),hbox,0,0+3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
	manager = create_menu(fp);
	treeViewOfAtoms = addListOfAtoms(hbox, manager);

	hbox = create_hbox(vboxall);

	button = create_button(winParent,"OK");
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(set_HBonds),(gpointer)entrys);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(fp));
	gtk_widget_show (button);

	button = create_button(winParent,"Apply");
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(set_HBonds),(gpointer)entrys);
	gtk_widget_show (button);

	button = create_button(winParent,"Close");
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(fp));

	gtk_widget_show (button);
	rafreshTreeView(treeViewOfAtoms);
   
	gtk_widget_show_all(fp);
}
/************************************************************************/
gdouble getMinDistanceHBonds()
{
	return minDistance;
}
/************************************************************************/
gdouble getMaxDistanceHBonds()
{
	return maxDistance;
}
/************************************************************************/
gdouble getMinAngleHBonds()
{
	return minAngle;
}
/************************************************************************/
gdouble getMaxAngleHBonds()
{
	return maxAngle;
}
/************************************************************************/
gboolean atomCanDoHydrogenBond(gchar* symbol)
{
	gint k;
	for(k=0;k<nAtomsCanConnect;k++) if(strcmp(symbol,atomsCanConnect[k])==0) return TRUE;
	return FALSE;
}
/************************************************************************/
