/* Orbitals.c */
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
#include "GlobalOrb.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Utils/Constants.h"
#include "../Spectrum/DOS.h"
#include "GeomDraw.h"
#include "GLArea.h"
#include "UtilsOrb.h"
#include "Basis.h"
#include "GeomOrbXYZ.h"
#include "AtomicOrbitals.h"
#include "StatusOrb.h"
#include "Orbitals.h"
/*
#include "OrbitalsDalton.h"
*/
#include "OrbitalsGamess.h"
#include "OrbitalsMolpro.h"
#include "OrbitalsQChem.h"
#include "OrbitalsNWChem.h"
#include "OrbitalsMopac.h"
#include "OrbitalsOrca.h"
#include "OrbitalsNBO.h"
#include "wfx.h"

#define WIDTHSCR 0.56

static gint TypeSelOrbtmp = 1;
static gint NumSelOrbtmp = -1;
void create_iso_orbitals();
static gboolean sphericalBasis = FALSE;
static GtkWidget* winList = NULL;
/********************************************************************************/
static void resetViewOrbCoef(GtkWidget* textWidget);
/************************************************************************************************************/
static gint getNumSelectOrb(gint numRow)
{
	gint n = -1;
	gint i;
	if(numRow<0) return -1;
	for(i=0;i<NOrb;i++)
	{
  		if(TypeSelOrb == 1 && strcmp(SymAlphaOrbitals[i],"DELETED")) n++;
		else if( TypeSelOrb != 1 && strcmp(SymBetaOrbitals[i],"DELETED")) n++;
		if(n==numRow) return i;
	}
	return -1;
}
/************************************************************************************************************/
static gint getNumSelectRow(gint numOrb)
{
	gint numRow = -1;
	gint i;
	for(i=0;i<NOrb;i++)
	{
  		if(TypeSelOrb == 1 && strcmp(SymAlphaOrbitals[i],"DELETED")) numRow++;
		else if(TypeSelOrb != 1 && strcmp(SymBetaOrbitals[i],"DELETED")) numRow++;
		if(i==numOrb) return numRow;
	}
	return -1;
}
/************************************************************************************************************/
static void destroyWinsList(GtkWidget *win)
{
	if(GTK_IS_WIDGET(win)) delete_child(win);
	if(GTK_IS_WIDGET(win)) gtk_widget_destroy(win);
	winList  = NULL;
}
/************************************************************************************************************/
void destroy_win_list()
{
	if(!winList) return;
	destroyWinsList(winList);
}
/********************************************************************************/
static void selectRow(GtkWidget* list, gint row)
{
	GtkWidget* textWidget = NULL;
	GtkTreePath *path;
	gchar* tmp = NULL;

	if(row<0) return;
	tmp = g_strdup_printf("%d",row);
	path = gtk_tree_path_new_from_string  (tmp);
	g_free(tmp);
	if(!list) return;
	gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (list)), path);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW (list), path, NULL, FALSE,0.5,0.5);
	gtk_tree_path_free(path);
	textWidget = g_object_get_data(G_OBJECT (list), "CoefOrbWidget");
	if(textWidget) resetViewOrbCoef(textWidget);
}
/********************************************************************************/
void free_orbitals()
{
	gint i;
	if(EnerAlphaOrbitals == EnerBetaOrbitals)
	{
		if(EnerAlphaOrbitals)
			g_free(EnerAlphaOrbitals);
		EnerAlphaOrbitals = NULL;
		EnerBetaOrbitals = NULL;
	}
	else
	{
		if(EnerAlphaOrbitals)
			g_free(EnerAlphaOrbitals);
		EnerAlphaOrbitals = NULL;
		if(EnerBetaOrbitals)
			g_free(EnerBetaOrbitals);
		EnerBetaOrbitals = NULL;
	}
	if(OccAlphaOrbitals == OccBetaOrbitals)
	{
		if(OccAlphaOrbitals)
			g_free(OccAlphaOrbitals);
		OccAlphaOrbitals = NULL;
		OccBetaOrbitals = NULL;
	}
	else
	{
		if(OccAlphaOrbitals)
			g_free(OccAlphaOrbitals);
		OccAlphaOrbitals = NULL;
		if(OccBetaOrbitals)
			g_free(OccBetaOrbitals);
		OccBetaOrbitals = NULL;

	}
	if(SymAlphaOrbitals == SymBetaOrbitals)
	{
		if(SymAlphaOrbitals)
		{
			for(i=0;i<NAlphaOrb;i++)
				if(SymAlphaOrbitals[i])
					g_free(SymAlphaOrbitals[i]);
			g_free(SymAlphaOrbitals);
		}
		SymAlphaOrbitals = NULL;
		SymBetaOrbitals = NULL;
	}
	else
	{
		if(SymAlphaOrbitals)
		{
			for(i=0;i<NAlphaOrb;i++)
				if(SymAlphaOrbitals[i])
					g_free(SymAlphaOrbitals[i]);
			g_free(SymAlphaOrbitals);
		}
		SymAlphaOrbitals = NULL;
		if(SymBetaOrbitals)
		{
			for(i=0;i<NBetaOrb;i++)
				if(SymBetaOrbitals[i])
					g_free(SymBetaOrbitals[i]);
			g_free(SymBetaOrbitals);
		}
		SymBetaOrbitals = NULL;
	}

	if(CoefAlphaOrbitals == CoefBetaOrbitals)
	{
		if(CoefAlphaOrbitals)
		{
			for(i=0;i<NOrb;i++)
				if(CoefAlphaOrbitals[i])
					g_free(CoefAlphaOrbitals[i]);
			g_free(CoefAlphaOrbitals);
		}
		CoefAlphaOrbitals  = NULL;
		CoefBetaOrbitals  = NULL;
	}
	else
	{
		if(CoefAlphaOrbitals)
		{
			for(i=0;i<NOrb;i++)
				if(CoefAlphaOrbitals[i])
					g_free(CoefAlphaOrbitals[i]);
			g_free(CoefAlphaOrbitals);
		}
		CoefAlphaOrbitals  = NULL;
		if(CoefBetaOrbitals)
		{
			for(i=0;i<NOrb;i++)
				if(CoefBetaOrbitals[i])
					g_free(CoefBetaOrbitals[i]);
			g_free(CoefBetaOrbitals);
		}
		CoefBetaOrbitals = NULL;
	}
	set_status_label_info(_("Mol. Orb."),_("Nothing"));
	NOrb = 0;
	NAOrb = 0;
}
/********************************************************************************/
static void applyiso(GtkWidget *Win,gpointer data)
{
	GtkWidget* Entry =(GtkWidget*)g_object_get_data(G_OBJECT (Win), "Entry");
	gchar* temp;
	gdouble isovalue;
	
	temp = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry))); 
	delete_first_spaces(temp);
	delete_last_spaces(temp);
	if(this_is_a_real(temp)) isovalue = atof(temp);
	else
	{
		GtkWidget* message =Message(_("Error : one entry is not a float "),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		return;
	}
	if(fabs(isovalue)>fabs(limits.MinMax[1][3]) && fabs(isovalue)>fabs(limits.MinMax[0][3]))
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Error : The isovalue  value should between %lf and %lf"),fabs(limits.MinMax[1][3]),fabs(limits.MinMax[0][3]));
		GtkWidget* message = Message(buffer,_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		return;
	}
	/*
	if(isovalue<limits.MinMax[0][3])
	{
		GtkWidget* message = Message("Error :  The minimal value should be smaller than the minimal value ",_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		return;
	}
	*/

	delete_child(Win);
	Define_Iso(fabs(isovalue));
	glarea_rafresh(GLArea);
}

/********************************************************************************/
void applygrid(GtkWidget *Win,gpointer data)
{
	GtkWidget** entriestmp = NULL;
	G_CONST_RETURN gchar* temp;
	gchar* dump;
	gint i;
	gint j;
	GridLimits limitstmp;
	gint NumPointstmp[3];
	GtkWidget *entries[3][6];
	gdouble V[3][3];

	if(GTK_IS_WIDGET(Win))
	{
		entriestmp = (GtkWidget **)g_object_get_data(G_OBJECT (Win), "Entries");
	}
	else return;

	if(entriestmp==NULL) return;

	for(i=0;i<3;i++)
	for(j=0;j<6;j++)
		entries[i][j] = entriestmp[i*6+j];
	
	for(i=0;i<3;i++)
	{
		for(j=3;j<5;j++)
		{
        		temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][j])); 
			dump = NULL;
			if(temp && strlen(temp)>0)
			{
				dump = g_strdup(temp);
				delete_first_spaces(dump);
				delete_last_spaces(dump);
			}

			if(dump && strlen(dump)>0 && this_is_a_real(dump))
			{
				limitstmp.MinMax[j-3][i] = atof(dump);
			}
			else
			{
				GtkWidget* message = Message(_("Error : one entry is not a float "),_("Error"),TRUE);
  				gtk_window_set_modal (GTK_WINDOW (message), TRUE);
				return;
			}
			if(dump) g_free(dump);
		}
        	temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][5])); 
		NumPointstmp[i] = atoi(temp);
		if(NumPointstmp[i] <=2)
		{
			GtkWidget* message = Message(_("Error : The number of points should be > 2. "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		
	}

	for(i=0;i<3;i++)
	{
		if( limitstmp.MinMax[0][i]> limitstmp.MinMax[1][i])
		{
			GtkWidget* message = Message(_("Error :  The minimal value should be smaller than the maximal value "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
	}
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
		{
			V[i][j] = 0;
        		temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][j])); 
			dump = NULL;
			if(temp && strlen(temp)>0)
			{
				dump = g_strdup(temp);
				delete_first_spaces(dump);
				delete_last_spaces(dump);
			}

			if(dump && strlen(dump)>0 && this_is_a_real(dump))
			{
				V[i][j] = atof(dump);
			}
			else
			{
				GtkWidget* message = Message(_("Error : one entry is not a float "),_("Error"),TRUE);
  				gtk_window_set_modal (GTK_WINDOW (message), TRUE);
				return;
			}
			if(dump) g_free(dump);
		}
	}
        
	for(i=0;i<3;i++)
	{
		gdouble norm = 0.0;
		for(j=0;j<3;j++)
			norm += V[i][j]*V[i][j];
		if(fabs(norm)<1e-8)
		{
			GtkWidget* message = Message(_("Error : the norm is equal to 0 "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		for(j=0;j<3;j++)
			V[i][j] /= sqrt(norm);
	}
	for(j=0;j<3;j++) originOfCube[j] = 0;
	for(j=0;j<3;j++) firstDirection[j] = V[0][j];
	for(j=0;j<3;j++) secondDirection[j] = V[1][j];
	for(j=0;j<3;j++) thirdDirection[j] = V[2][j];

	for(i=0;i<3;i++)
	{
		NumPoints[i] =NumPointstmp[i] ; 
		for(j=0;j<2;j++)
			limits.MinMax[j][i] =limitstmp.MinMax[j][i]; 
	}
	/* for(i=0;i<3;i++) printf("%f %f %d\n",limits.MinMax[0][i], limits.MinMax[1][i], NumPoints[i]); */


	delete_child(Win);
	TypeSelOrb = TypeSelOrbtmp;
 	NumSelOrb = NumSelOrbtmp;
	/* printf("Define_Grid\n");*/
	Define_Grid();
	/* printf("create_iso_orbitals\n");*/
	create_iso_orbitals();
}
/********************************************************************************/
static void applycutoff(GtkWidget *Win,gpointer data)
{
	GtkWidget* entry =(GtkWidget*)g_object_get_data(G_OBJECT (Win), "Entry");
  	GtkWidget *alphalist  = g_object_get_data(G_OBJECT (Win), "AlphaList");
  	GtkWidget *betalist  = g_object_get_data(G_OBJECT (Win), "BetaList");
	gchar* temp;
	gdouble cutoff;
	gint i,j;
	
	temp = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry))); 
	cutoff =fabs(atof(temp));
	for(j=0;j<NAlphaOrb;j++)
		for(i=0;i<NAOrb;i++)
			if(fabs(CoefAlphaOrbitals[j][i])<cutoff) CoefAlphaOrbitals[j][i] = 0.0;
	for(j=0;j<NBetaOrb;j++)
		for(i=0;i<NAOrb;i++)
			if(fabs(CoefBetaOrbitals[j][i])<cutoff) CoefBetaOrbitals[j][i] = 0.0;
	delete_child(Win);
	if(alphalist) 
	{
		GtkWidget* textWidget = g_object_get_data(G_OBJECT (alphalist), "CoefOrbWidget");
		if(textWidget) resetViewOrbCoef(textWidget);
	}
	if(betalist) 
	{
		GtkWidget* textWidget = g_object_get_data(G_OBJECT (betalist), "CoefOrbWidget");
		if(textWidget) resetViewOrbCoef(textWidget);
	}
}
/********************************************************************************/
static void cut_ceof_orbitals_win(GtkWidget* parentWin)
{
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *entry;
	GtkWidget *table;

	GtkWidget *Win;
	GtkWidget *hbox;
	GtkWidget *vboxall;
	GtkWidget *vboxwin;
	GtkWidget *button;
  	GtkWidget *alphalist  = g_object_get_data(G_OBJECT (parentWin), "AlphaList");
  	GtkWidget *betalist  = g_object_get_data(G_OBJECT (parentWin), "BetaList");


	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),_("Cutoff molecular orbitals coefficients"));
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 2);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"cutoff coef");

	vboxall = create_vbox(Win);
	vboxwin = vboxall;
	frame = gtk_frame_new (NULL);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	table = gtk_table_new(1,3,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);

	add_label_at_table(table,"Cut off value",0,0,GTK_JUSTIFY_LEFT);
	add_label_at_table(table,":",0,1,GTK_JUSTIFY_LEFT);
	entry= gtk_entry_new ();
	add_widget_table(table,entry,0,2);
	gtk_entry_set_text(GTK_ENTRY( entry),"1e-6");
	gtk_widget_show_all(frame);
	g_object_set_data(G_OBJECT (Win), "Entry",entry);
  

  	g_object_set_data(G_OBJECT (Win), "AlphaList",alphalist);
  	g_object_set_data(G_OBJECT (Win), "BetaList",betalist);
	/* buttons box */
	hbox = create_hbox_false(vboxwin);
	gtk_widget_realize(Win);

	button = create_button(Win,_("Cancel"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, GTK_OBJECT(Win));
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(Win));
	gtk_widget_show (button);

	button = create_button(Win,_("OK"));
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)applycutoff,GTK_OBJECT(Win));
  	g_signal_connect_swapped(G_OBJECT (entry), "activate", (GCallback) gtk_button_clicked, GTK_OBJECT (button));
  

	/* Show all */
	gtk_widget_show_all (Win);
}
/*************************************************************************************************/
static void resetViewOrbCoef(GtkWidget* textWidget)
{
	gchar buffer[BSIZE];
       	gchar buffer1[20];
	gint nchar;
	gint l;
	gchar *XYZ[]={"x","y","z"};
	gint j = NumSelOrbtmp;
	gint i;
	gint nC = 0;
	gdouble** M = CoefAlphaOrbitals;
	GtkTreeSelection *select;
	GtkTreeIter iter;
	GtkTreeModel* model = NULL;
	GtkListStore *store = NULL;
	G_CONST_RETURN gchar* entryText = NULL;
	gdouble filter = 0.0;
	GtkWidget* entry = NULL;

	if(NumSelOrbtmp<0) return;

	if(!textWidget) return;
	if(!AOrb && !SAOrb ) return;
	if(!GTK_IS_TREE_VIEW(textWidget)) return;
  
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(textWidget));
	if(!model) return;
	store = GTK_LIST_STORE(model);
	if(!store) return;
	gtk_list_store_clear(store);
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (textWidget));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_MULTIPLE);


	if(TypeSelOrbtmp != 1) M = CoefBetaOrbitals;

	nchar=gabedit_text_get_length(GABEDIT_TEXT(textWidget));
	gabedit_text_set_point(GABEDIT_TEXT(textWidget),0);
	gabedit_text_forward_delete(GABEDIT_TEXT(textWidget),nchar);
	gabedit_text_set_point(GABEDIT_TEXT(textWidget),0);

	entry = g_object_get_data(G_OBJECT (textWidget), "EntryFilter");
	if(GTK_IS_WIDGET(entry)) 
	{
		entryText = gtk_entry_get_text(GTK_ENTRY(entry));
		filter = fabs(atof(entryText));
	}

	for( i=0;i<NAOrb;i++)
	{
		if(fabs(M[j][i])<filter) continue;
		gtk_list_store_append(store, &iter);
		if(AOrb) nC = AOrb[i].NumCenter;
		else if(SAOrb) nC = SAOrb[i].NumCenter;
 		sprintf(buffer,"%s[%d]",GeomOrb[nC].Symb,nC+1);
		gtk_list_store_set (store, &iter, 0, buffer, -1);

		l=0;
		if(AOrb) for(j=0;j<3;j++) l += AOrb[i].Gtf[0].l[j];
		else if(SAOrb) for(j=0;j<3;j++) l += SAOrb[i].Stf[0].l[j];

		sprintf(buffer1,"%c",GetSymmetry(l));
 		for(j=0;j<3;j++)
 		{
			gint ll = 0;
			if(AOrb ) ll = AOrb[i].Gtf[0].l[j];
			else if(SAOrb) ll = SAOrb[i].Stf[0].l[j];
	 		switch(ll)
	 		{
	 			case 0: break;
	 			case 1: sprintf(buffer1+strlen(buffer1),"%s",XYZ[j]); break;
	 			default :
					if(AOrb)
				     	sprintf(buffer1+strlen(buffer1),"%s%d",XYZ[j],AOrb[i].Gtf[0].l[j]);
					else if(SAOrb) 
				     	sprintf(buffer1+strlen(buffer1),"%s%d",XYZ[j],SAOrb[i].Stf[0].l[j]);
	 		}
 		}
		sprintf(buffer,"%-12s",buffer1);
		gtk_list_store_set (store, &iter, 1, buffer, -1);

		j = NumSelOrbtmp;
		sprintf(buffer,"%+9.6f",M[j][i]);
		gtk_list_store_set (store, &iter, 2, buffer, -1);
	}
}
/*************************************************************************************************/
static void eventDispatcher(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{

	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreeModel *model;
	gint* type;
	GtkWidget *gtklist = widget;
  	GtkWidget *otherlist = NULL;

	if (!event) return;
        if (((GdkEventButton *) event)->button != 1) return; 

	type = (gint*)g_object_get_data(G_OBJECT (widget), "Type");
	TypeSelOrbtmp = *type;

  	otherlist = g_object_get_data(G_OBJECT (gtklist), "OtherList");
	if(otherlist)
		gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (otherlist)));

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
			NumSelOrbtmp = getNumSelectOrb(atoi(gtk_tree_path_to_string(path)));
			gtk_tree_model_get_iter (model, &iter, path);
			gtk_tree_path_free(path);
 			if ((GdkEventButton *) event && ((GdkEventButton *) event)->type==GDK_2BUTTON_PRESS)
			{
				GtkWidget* button = g_object_get_data(G_OBJECT (widget), "ButtonOk");
				gtk_button_clicked (GTK_BUTTON (button));
			}
		}
		else NumSelOrbtmp = -1;
	}
	GTK_WIDGET_GET_CLASS(widget)->button_press_event(widget, event);
	if(NumSelOrbtmp>=0)
	{
		GtkWidget* textWidget = g_object_get_data(G_OBJECT (gtklist), "CoefOrbWidget");
		if(textWidget) resetViewOrbCoef(textWidget);
	}
}
/********************************************************************************/
GtkWidget* create_gtk_list_orbitals(gint N,gdouble* Energies,gdouble* Occ,gchar** sym, gint* widall)
{
	gint i;
	gint j;
	GtkWidget* gtklist = NULL;
	gint *Width = NULL;
	gint NlistTitle = 4;
	gchar* Titles[] = {"Nr","Energy","Occ.","Sym."};
	gchar* List[4];
	GtkListStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *select;
	GtkTreeIter iter;
	GType* types;
  
	Width = g_malloc(NlistTitle*sizeof(gint));

	for (j=0;j<NlistTitle;j++) Width[j] = strlen(Titles[j]);

	types = g_malloc(NlistTitle*sizeof(GType));
	for (i=0;i<NlistTitle;i++) types[i] = G_TYPE_STRING;
  	store = gtk_list_store_newv (NlistTitle, types);
	g_free(types);
	model = GTK_TREE_MODEL (store);


	Width[0] = (gint)(Width[0]*10);
	Width[1] = (gint)(Width[1]*12);
	Width[2] = (gint)(Width[2]*8);
	Width[3] = (gint)(Width[3]*14);

	*widall = 0;
	for (j=0;j<NlistTitle;j++) *widall += Width[j];
	*widall += 80;

	gtklist = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (gtklist), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (gtklist), TRUE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW (gtklist), FALSE);

	for (i=0;i<NlistTitle;i++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, Titles[i]);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, TRUE);
		gtk_tree_view_column_set_min_width(column, Width[i]);
		gtk_tree_view_column_set_attributes (column, renderer, "text", i, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (gtklist), column);
	}
  	g_free( Width);
  
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (gtklist));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);

	for(i=0;i<N;i++)
	{
		if(strcmp(sym[i],"DELETED")==0)continue;
		List[0] = g_strdup_printf("%i",i+1);
		List[1] = g_strdup_printf("%lf",Energies[i]);
		List[2] = g_strdup_printf("%lf",Occ[i]);
		List[3] = g_strdup(sym[i]);

		gtk_list_store_append(store, &iter);
		for(j=0;j<4;j++) gtk_list_store_set (store, &iter, j, List[j], -1);

		for(j=0;j<4;j++) g_free(List[j]);
	}
	g_signal_connect(gtklist, "button_press_event", G_CALLBACK(eventDispatcher), NULL);

	return gtklist;
 
}
/********************************************************************************/
GtkWidget* create_gtk_list_coef_orbitals(gint* widall)
{
	gint i;
	gint j;
	GtkWidget* gtklist = NULL;
	gint *Width = NULL;
	gint NlistTitle = 3;
	gchar* titles[] = {"Atom","Type","Coeff."};
	GtkListStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GType* types;
  
	Width = g_malloc(NlistTitle*sizeof(gint));

	for (j=0;j<NlistTitle;j++) Width[j] = strlen(titles[j]);

	types = g_malloc(NlistTitle*sizeof(GType));
	for (i=0;i<NlistTitle;i++) types[i] = G_TYPE_STRING;
  	store = gtk_list_store_newv (NlistTitle, types);
	g_free(types);
	model = GTK_TREE_MODEL (store);


	Width[0] = (gint)(Width[0]*5);
	Width[1] = (gint)(Width[1]*5);
	Width[2] = (gint)(Width[2]*5);

	*widall = 0;
	for (j=0;j<NlistTitle;j++) *widall += Width[j];
	*widall += 10;

	gtklist = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (gtklist), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (gtklist), TRUE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW (gtklist), FALSE);

	for (i=0;i<NlistTitle;i++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, titles[i]);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, TRUE);
		gtk_tree_view_column_set_min_width(column, Width[i]);
		gtk_tree_view_column_set_attributes (column, renderer, "text", i, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (gtklist), column);
	}
  	g_free( Width);
  
	return gtklist;
 
}
/************************************************************************************************************/
static void changedEntryFilterCoef(GtkWidget *textWidget, gpointer data)
{
	if(NumSelOrbtmp<0) return;
	if(textWidget) resetViewOrbCoef(textWidget);
}

/********************************************************************************/
GtkWidget* create_alpha_beta_lists(GtkWidget *noteBook, gint Type)
{
	GtkWidget *frame;
	GtkWidget *scr;
	GtkWidget *hbox;
	GtkWidget *gtklist;
	gint i;
	gint N;
	gdouble* Energies;
	gdouble* Occ;
	gchar** sym;
	static gint alphaType = 1;
	static gint betaType = 2;
	gint widall = 0;
	gchar* labelAlpha = "Alpha Orbitals";
	gchar* labelBeta = "Beta Orbitals";
	gchar* label = labelAlpha;
	GtkWidget *labelOnglet = NULL;
	GtkWidget *labelMenu = NULL;
	GtkWidget *textWidget = NULL;
	GtkWidget *table = NULL;
	GtkWidget *entry = NULL;
	GtkWidget *vbox = NULL;
	GtkWidget *hseparator = gtk_hseparator_new ();

	N = NAlphaOrb;
	if(Type != 1) N = NBetaOrb;
	Energies = g_malloc(N*sizeof(gdouble));
	Occ = g_malloc(N*sizeof(gdouble));
	sym = g_malloc(N*sizeof(gchar*));

	if(Type == 1)
	{
		label = labelAlpha;
		for(i=0;i<N;i++)
		{	
			Energies[i] = EnerAlphaOrbitals[i];
			Occ[i] = OccAlphaOrbitals[i];
			sym[i] = g_strdup(SymAlphaOrbitals[i]);
		}
		gtklist = create_gtk_list_orbitals(N,Energies,Occ,sym,&widall);
		g_object_set_data(G_OBJECT (gtklist), "Type",&alphaType);
	}
	else
	{
		label = labelBeta;
		for(i=0;i<N;i++)
		{
			Energies[i] = EnerBetaOrbitals[i];
			Occ[i] = OccBetaOrbitals[i];
			sym[i] = g_strdup(SymBetaOrbitals[i]);
		}
		gtklist = create_gtk_list_orbitals(N,Energies,Occ,sym,&widall);
		g_object_set_data(G_OBJECT (gtklist), "Type",&betaType);
	}

  	frame = gtk_frame_new (NULL);
  	gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
  	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	labelOnglet = gtk_label_new(label);
	labelMenu = gtk_label_new(label);
	gtk_notebook_append_page_menu(GTK_NOTEBOOK(noteBook),frame, labelOnglet, labelMenu);
  	gtk_widget_show (frame);


  	vbox = gtk_vbox_new (FALSE, 0);
  	gtk_widget_show (vbox);
  	gtk_container_add (GTK_CONTAINER (frame), vbox);

	hbox = gtk_hbox_new (TRUE, 0);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

  	scr=gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_set_size_request(scr,widall,(gint)(ScreenHeightD*WIDTHSCR));
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  	gtk_box_pack_start(GTK_BOX (hbox), scr,TRUE, TRUE, 2);
  	gtk_container_add(GTK_CONTAINER(scr),gtklist);
	set_base_style(gtklist,55000,55000,55000);

  	vbox = gtk_vbox_new (FALSE, 0);
  	gtk_widget_show (vbox);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

	textWidget = create_gtk_list_coef_orbitals(&widall);
  	scr=gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_set_size_request(scr,widall,(gint)(ScreenHeightD*WIDTHSCR));
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  	gtk_box_pack_start(GTK_BOX (vbox), scr,TRUE, TRUE, 2);
  	gtk_container_add(GTK_CONTAINER(scr),textWidget);
	g_object_set_data(G_OBJECT (gtklist), "CoefOrbWidget",textWidget);
	gtk_widget_set_size_request(textWidget,widall,(gint)(ScreenHeightD*WIDTHSCR));

	gtk_widget_show (hseparator);
	gtk_box_pack_start (GTK_BOX (vbox), hseparator, FALSE, FALSE, 1);

	table = gtk_table_new(1,3,FALSE);
	add_label_at_table(table,"Filter",0,0,GTK_JUSTIFY_LEFT);
	add_label_at_table(table,":",0,1,GTK_JUSTIFY_LEFT);
	entry = gtk_entry_new ();
	gtk_table_attach(GTK_TABLE(table),entry,2,3,0,1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
	gtk_entry_set_text(GTK_ENTRY( entry),"0.0");
  	gtk_box_pack_start(GTK_BOX (vbox), table,FALSE, FALSE, 2);
	g_object_set_data(G_OBJECT (textWidget), "EntryFilter",entry);
	g_signal_connect_swapped(G_OBJECT(entry),"changed", G_CALLBACK(changedEntryFilterCoef),textWidget);


	for(i=0;i<N;i++)
		g_free(sym[i]);
	g_free(Energies);
	g_free(Occ);
	g_free(sym);

	return gtklist;

}
/********************************************************************************/
static void getiso(GtkWidget *button,gpointer data)
{
	GtkWidget* Entry =(GtkWidget*)g_object_get_data(G_OBJECT (button), "Entry");
	GtkWidget* EntryPercent =(GtkWidget*)g_object_get_data(G_OBJECT (button), "EntryPercent");
	gchar* temp;
	gdouble isovalue = 0.1;
	gdouble percent = 100;
	gboolean square = TRUE;
	gdouble precision =1e-6;
	
	temp = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryPercent))); 
	delete_first_spaces(temp);
	delete_last_spaces(temp);
	if(this_is_a_real(temp)) percent = fabs(atof(temp));
	else
	{
		GtkWidget* message =Message(_("Error : one entry is not a float "),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		if(temp) g_free(temp);
		return;
	}
	if(temp) g_free(temp);
	if(percent<0) percent = 0;
	if(percent>100) percent = 100;

	if(
		   TypeGrid == GABEDIT_TYPEGRID_ELFSAVIN
		|| TypeGrid == GABEDIT_TYPEGRID_ELFBECKE
		|| TypeGrid == GABEDIT_TYPEGRID_FEDELECTROPHILIC
		|| TypeGrid == GABEDIT_TYPEGRID_FEDNUCLEOPHILIC
		|| TypeGrid == GABEDIT_TYPEGRID_FEDRADICAL
		|| TypeGrid == GABEDIT_TYPEGRID_EDENSITY
	) square = FALSE;

	if(!compute_isovalue_percent_from_grid(grid, square, percent, precision, &isovalue)) return;
	temp = g_strdup_printf("%f",isovalue);
	gtk_entry_set_text(GTK_ENTRY(Entry),temp); 
	if(temp) g_free(temp);
}
/********************************************************************************/
GtkWidget *create_iso_frame( GtkWidget *vboxall,gchar* title)
{
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *Entry;
	GtkWidget *EntryPercent;
	gushort i;
	gushort j;
	GtkWidget *Table;
	gdouble v;
  	GtkWidget* button;
#define NLIGNES   3
#define NCOLUMNS  3
	gchar      *strlabels[NLIGNES][NCOLUMNS];
	
	strlabels[0][0] = g_strdup(_(" Min "));
	strlabels[1][0] = g_strdup(_(" Max "));
	strlabels[2][0] = g_strdup(_(" Value "));
	strlabels[0][1] = g_strdup(" : ");
	strlabels[1][1] = g_strdup(" : ");
	strlabels[2][1] = g_strdup(" : ");

	if(fabs(limits.MinMax[0][3])>1e6) strlabels[0][2] = g_strdup_printf(" %e ",limits.MinMax[0][3]);
	else strlabels[0][2] = g_strdup_printf(" %lf ",limits.MinMax[0][3]);

	if(fabs(limits.MinMax[1][3])>1e6) strlabels[1][2] = g_strdup_printf(" %e ",limits.MinMax[1][3]);
	else strlabels[1][2] = g_strdup_printf(" %lf ",limits.MinMax[1][3]);
	v = limits.MinMax[1][3]/4;
	if(v>0.2 && fabs(limits.MinMax[1][3])>0.01 && fabs(limits.MinMax[0][3])<0.01) v= 0.01;
	if(TypeGrid == GABEDIT_TYPEGRID_SAS) v = 0;
	if(TypeGrid == GABEDIT_TYPEGRID_ELFSAVIN) v = 0.8;
	if(TypeGrid == GABEDIT_TYPEGRID_ELFBECKE) v = 0.8;
	if(TypeGrid == GABEDIT_TYPEGRID_FEDELECTROPHILIC) v *= 3;
	if(TypeGrid == GABEDIT_TYPEGRID_FEDNUCLEOPHILIC) v *= 3;
	if(TypeGrid == GABEDIT_TYPEGRID_FEDRADICAL) v *= 3;
	if(TypeGrid == GABEDIT_TYPEGRID_NCI) v = 0.1;
		
	strlabels[2][2] = g_strdup_printf("%lf",v);

	frame = gtk_frame_new (title);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	Table = gtk_table_new(3,3,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),Table);

	for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			if(i+j != 4)
				add_label_at_table(Table,strlabels[i][j],i,(gushort)j,GTK_JUSTIFY_LEFT);
	Entry= gtk_entry_new ();
	add_widget_table(Table,Entry,(gushort)2,(gushort)2);
	gtk_entry_set_text(GTK_ENTRY( Entry),strlabels[2][2]);

	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
			g_free(strlabels[i][j]);
	}
	gtk_box_pack_start (GTK_BOX (vboxframe), gtk_hseparator_new (), TRUE, TRUE, 0);
	Table = gtk_table_new(1,3,FALSE);
	gtk_box_pack_start (GTK_BOX (vboxframe), Table, TRUE, TRUE, 0);
	EntryPercent= gtk_entry_new ();
	gtk_widget_set_size_request(EntryPercent,60,-1);
	add_widget_table(Table,EntryPercent,0,0);
	gtk_entry_set_text(GTK_ENTRY( EntryPercent),"99");
	g_object_set_data(G_OBJECT (frame), "EntryPercent",EntryPercent);
	add_label_at_table(Table,"%",0,1,GTK_JUSTIFY_LEFT);
    	button = gtk_button_new_with_label("  Get Isovalue  ");
	g_object_set_data(G_OBJECT (frame), "ButtonGet",button);
	gtk_table_attach(GTK_TABLE(Table),button,2,3,0,1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);

	gtk_widget_show_all(frame);
	g_object_set_data(G_OBJECT (frame), "Entry",Entry);

	g_object_set_data(G_OBJECT (button), "EntryPercent",EntryPercent);
	g_object_set_data(G_OBJECT (button), "Entry",Entry);
  	g_signal_connect(G_OBJECT(button), "clicked",(GCallback)getiso,GTK_OBJECT(Entry));
  
  	return frame;
#undef NLIGNES
#undef NCOLUMNS
}
/********************************************************************************/
static void reset_new_surface()
{
	newSurface = FALSE;
}
/********************************************************************************/
void create_iso_orbitals()
{
  GtkWidget *Win;
  GtkWidget *frame;
  GtkWidget *hbox;
  GtkWidget *vboxall;
  GtkWidget *vboxwin;
  GtkWidget *button;
  GtkWidget* Entry;


  /* printf("Begin create_iso_orbitals\n"); */
  if(!grid )
  {
	  if( !CancelCalcul)
	  	Message(_("Grid not defined "),_("Error"),TRUE);
	  return;
  }
  /* Principal Window */
  Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(Win),_("Calculations of isosurfaces for an orbital"));
  gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
  gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
  gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
  gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

  add_glarea_child(Win,"Iso surface ");
  g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)reset_new_surface,NULL);

  vboxall = create_vbox(Win);
  vboxwin = vboxall;
  frame = create_iso_frame(vboxall,_("Iso-Value"));
  Entry = (GtkWidget*) g_object_get_data(G_OBJECT (frame), "Entry");
  g_object_set_data(G_OBJECT (Win), "Entry",Entry);
   

  /* buttons box */
  hbox = create_hbox_false(vboxwin);
  gtk_widget_realize(Win);

  button = create_button(Win,_("Cancel"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, GTK_OBJECT(Win));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(Win));
  gtk_widget_show (button);

  button = create_button(Win,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)applyiso,GTK_OBJECT(Win));
  g_signal_connect_swapped(G_OBJECT (Entry), "activate", (GCallback) gtk_button_clicked, GTK_OBJECT (button));
  

  /* Show all */
  gtk_widget_show_all (Win);
}
/********************************************************************************/
void create_grid_orbitals()
{
  TypeGrid = GABEDIT_TYPEGRID_ORBITAL;
  create_grid("Calculations of grid for an orbital");
}
/********************************************************************************/
static void create_alpha_dos(GtkWidget *parentWindow,gpointer data)
{
	gint n  = NAlphaOrb;
	createDOSSpectrumFromEnergiesTable(parentWindow, EnerAlphaOrbitals,  n);
}
/********************************************************************************/
static void create_beta_dos(GtkWidget *parentWindow,gpointer data)
{
	gint n  = NBetaOrb;
	createDOSSpectrumFromEnergiesTable(parentWindow, EnerBetaOrbitals,  n);
}
/********************************************************************************/
static void create_all_dos(GtkWidget *parentWindow,gpointer data)
{
	gint i;
	gint n  = NAlphaOrb + NBetaOrb;
	gdouble *energies = NULL;
	if(n<1)return;
	if(!EnerAlphaOrbitals && NAlphaOrb>0) return;
	if(!EnerBetaOrbitals && NBetaOrb>0) return;
	energies = g_malloc(n*sizeof(gdouble));
	for(i=0;i<NAlphaOrb;i++) energies[i] = EnerAlphaOrbitals[i];
	for(i=0;i<NBetaOrb;i++) energies[i+NAlphaOrb] = EnerBetaOrbitals[i];
	createDOSSpectrumFromEnergiesTable(parentWindow, energies,  n);
	g_free(energies);
}
/********************************************************************************/
void create_list_orbitals()
{
  GtkWidget *Win;
  GtkWidget *frame;
  GtkWidget *hbox;
  GtkWidget *vboxall;
  GtkWidget *vboxwin;
  GtkWidget *button;
  GtkWidget *noteBook = gtk_notebook_new();
  GtkWidget *alphalist = NULL;
  GtkWidget *betalist = NULL;

  if(NAOrb<1)
  {
	  Message(_("Sorry, Please load a file beforee\n"),_("Error"),TRUE);
	  return;
  }

  if(winList) destroyWinsList(winList);

  /* Principal Window */
  Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(Win),_("Orbitals"));
  gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
  gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
  gtk_window_set_default_size (GTK_WINDOW(Win),-1,(gint)(ScreenHeightD*0.69));
  gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
  /* gtk_window_set_modal (GTK_WINDOW (Win), TRUE);*/

  add_glarea_child(Win,"Orbitals List ");
  g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)destroyWinsList,NULL);

  vboxall = create_vbox(Win);
  vboxwin = vboxall;

  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_add(GTK_CONTAINER(vboxall),frame);
  gtk_widget_show (frame);
  vboxall = create_vbox(frame);
  gtk_box_pack_start (GTK_BOX (vboxall), noteBook, TRUE, TRUE, 0); 
  alphalist = create_alpha_beta_lists(noteBook, 1);
  betalist = create_alpha_beta_lists(noteBook, 2);
  g_object_set_data(G_OBJECT (alphalist), "OtherList",betalist);
  g_object_set_data(G_OBJECT (betalist), "OtherList",alphalist);

  TypeSelOrbtmp = TypeSelOrb;
  NumSelOrbtmp = NumSelOrb;

  gtk_widget_show_all(vboxall);

  /* buttons box */
  hbox = create_hbox_false(vboxwin);
  gtk_widget_realize(Win);

  button = create_button(Win,_("Close"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 2);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)destroyWinsList, GTK_OBJECT(Win));
  gtk_widget_show (button);

  button = create_button(Win,_("Cut MO Coef."));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 2);
  gtk_widget_show (button);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)cut_ceof_orbitals_win,GTK_OBJECT(Win));

  button = create_button(Win,_("DOS Alpha"));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 2);
  gtk_widget_show (button);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)create_alpha_dos,GTK_OBJECT(Win));

  button = create_button(Win,_("DOS Beta"));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 2);
  gtk_widget_show (button);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)create_beta_dos,GTK_OBJECT(Win));

  button = create_button(Win,_("DOS All"));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 2);
  gtk_widget_show (button);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)create_all_dos,GTK_OBJECT(Win));

  button = create_button(Win,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 2);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)create_grid_orbitals,GTK_OBJECT(Win));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)destroyWinsList, GTK_OBJECT(Win));

  g_object_set_data(G_OBJECT (alphalist), "ButtonOk",button);
  g_object_set_data(G_OBJECT (betalist), "ButtonOk",button);
  g_object_set_data(G_OBJECT (Win), "AlphaList",alphalist);
  g_object_set_data(G_OBJECT (Win), "BetaList",betalist);
  


  gtk_widget_show (Win);
  /* fit_windows_position(PrincipalWindow, Win);*/
  /* Show all */

  gtk_widget_show_all (Win);

  if(TypeSelOrb == 1 && NumSelOrb > -1)
  {
  	gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (betalist)));
	selectRow(alphalist,getNumSelectRow(NumSelOrb));
  }
  if(TypeSelOrb == 2 && NumSelOrb > -1)
  {
  	gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (alphalist)));
	selectRow(betalist,getNumSelectRow(NumSelOrb));  
  }
  winList = Win;
}
/********************************************************************************/
gchar* get_sym_orb(gchar *allstr)
{
	gchar* begin = NULL;
	gchar* end = NULL;
	begin = strstr(allstr,"(");
	if(begin)
	{
		end = strstr(begin+1,")");
		if(end)
		{
			gint l = end-begin;
			gint i;
			gchar *sym;
			if(l<=0)
				return g_strdup(_("Unknown"));
			sym = g_malloc(l*sizeof(gchar));
			for(i=0;i<l-1;i++)
				sym[i] = begin[i+1];
			sym[l-1] = '\0';
			return sym;
		}
		else
			return g_strdup(_("Unknown"));
	}
	else 
		return g_strdup(_("Unknown"));

}
/********************************************************************************/
static void save_geometry_gabedit_format(FILE* file)
{
	gint j;
	fprintf(file,"[Atoms] Angs\n");
	for(j=0;j<nCenters;j++)
	{
		fprintf(file,"%s %d %d %lf %lf %lf\n",GeomOrb[j].Symb,j+1,(gint)GeomOrb[j].Prop.atomicNumber,
				BOHR_TO_ANG*GeomOrb[j].C[0],BOHR_TO_ANG*GeomOrb[j].C[1],BOHR_TO_ANG*GeomOrb[j].C[2]);
	}
}
/********************************************************************************/
static void save_mo_orbitals_gabedit_format(FILE* file)
{
	gint i;
	gint j;
	fprintf(file,"[MO]\n");
	for(j=0;j<NAlphaOrb;j++)
	{

		fprintf(file," Ene= %lf\n",EnerAlphaOrbitals[j]);
		fprintf(file," Spin= Alpha\n");
		fprintf(file," Occup= %lf\n",OccAlphaOrbitals[j]);
		fprintf(file," Sym= %s\n",SymAlphaOrbitals[j]);
		for(i=0;i<NAOrb;i++)
			fprintf(file,"     %d    %lf\n",i+1, CoefAlphaOrbitals[j][i]);
	}
	for(j=0;j<NBetaOrb;j++)
	{

		fprintf(file," Ene= %lf\n",EnerBetaOrbitals[j]);
		fprintf(file," Spin= Beta\n");
		fprintf(file," Occup= %lf\n",OccBetaOrbitals[j]);
		fprintf(file," Sym= %s\n",SymBetaOrbitals[j]);
		for(i=0;i<NAOrb;i++)
			fprintf(file,"     %d    %lf\n",i+1, CoefBetaOrbitals[j][i]);
	}
	fprintf(file,"\n");
}
/********************************************************************************/
static void save_gabedit_orbitals(gchar* FileName)
{
 	FILE *file;
 	file = FOpen(FileName, "w");
	if(file == NULL)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not create '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return;
	}
	if(sphericalBasis)
		fprintf(file,"[Gabedit Format] Sphe\n");
	else
		fprintf(file,"[Gabedit Format]\n");
	save_geometry_gabedit_format(file);
	save_basis_gabedit_format(file);
	save_mo_orbitals_gabedit_format(file);
	save_ao_orbitals_gabedit_format(file);
	fclose(file);
}
/********************************************************************************/
gboolean read_last_orbitals_in_gaussian_file(gchar *fileName,gint itype)
{
 	gchar *t;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	FILE *fd;
 	gint taille=BSIZE;
 	gint i;
 	gint numorb;
 	gchar *pdest = NULL;
	gint NumOrb[5];
	gchar SymOrb[5][10];
	gdouble EnerOrb[5];
	gchar *dump1= g_malloc(20);
	gchar *dump2= g_malloc(20);
	gint ncart;
	gint n;
	/* gint k;*/
	gdouble **CoefOrbitals;
	gdouble *EnerOrbitals;
	gchar **SymOrbitals;
	gchar* tmp = NULL;
	gint NOcc = 0;
	gint nReadOrb = 0;
	gint nR = 0;
	
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return FALSE;
 	}

 	t=g_malloc(taille);
 	fd = FOpen(fileName, "rb");
 	if(fd ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
  		return FALSE;
 	}
 	for(i=0;i<5;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(char));
  
	CoefOrbitals = CreateTable2(NOrb);
	EnerOrbitals = g_malloc(NOrb*sizeof(gdouble));
	SymOrbitals = g_malloc(NOrb*sizeof(gchar*));

 	numorb =1;
 	do 
 	{
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
			switch(itype)
			{
			case 1 :
          			pdest = strstr( t, "Alpha Molecular Orbital Coefficients" );
					break;
			case 2 :
          			pdest = strstr( t, "Beta Molecular Orbital Coefficients" );
					break;
			case 3: 
          			pdest = strstr( t, "Molecular Orbital Coefficients" );
					break;
			case 4: 
          			pdest = strstr( t, "Natural Orbital Coefficients" );
					break;
			}
	 		if ( pdest != NULL )
	  		{
                		numorb++;
                		OK = TRUE;
	  			break;
	  		}
        	}
 		if(!OK && (numorb == 1) )
		{
			if(itype==2 || itype==4)
			{
				gchar buffer[BSIZE];
				sprintf(buffer,_("Sorry,  I can not read orbitals from '%s' file\n"),fileName);
  				Message(buffer,_("Error"),TRUE);
			}
			FreeTable2(CoefOrbitals,NOrb);
			g_free(EnerOrbitals);
			g_free(SymOrbitals);
			return FALSE;
    	}
 		if(!OK)
		{
			/* Debug("End of read \n");*/
 			fclose(fd);
 			g_free(t);
 			for(i=0;i<5;i++)
				g_free(AtomCoord[i]);

			switch(itype)
			{
			case 1 : 
				CoefAlphaOrbitals = CoefOrbitals;
				EnerAlphaOrbitals = EnerOrbitals;
				
				SymAlphaOrbitals = SymOrbitals;

				OccAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
				for(i=0;i<NOcc;i++)
					OccAlphaOrbitals[i] = 1.0;
				for(i=NOcc;i<NOrb;i++)
					OccAlphaOrbitals[i] = 0.0;

				NAlphaOcc = NOcc;
				NAlphaOrb = nReadOrb;
				break;
			case 2 : 
				CoefBetaOrbitals = CoefOrbitals;
				EnerBetaOrbitals = EnerOrbitals;
				SymBetaOrbitals = SymOrbitals;

				OccBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));
				for(i=0;i<NOcc;i++)
					OccBetaOrbitals[i] = 1.0;
				for(i=NOcc;i<NOrb;i++)
					OccBetaOrbitals[i] = 0.0;

				NBetaOcc = NOcc;
				NBetaOrb = nReadOrb;
				break;
			case 3 : 
				CoefAlphaOrbitals = CoefOrbitals;
				EnerAlphaOrbitals = EnerOrbitals;
				SymAlphaOrbitals = SymOrbitals;
				OccAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
				for(i=0;i<NOcc;i++)
					OccAlphaOrbitals[i] = 1.0;
				for(i=NOcc;i<NOrb;i++)
					OccAlphaOrbitals[i] = 0.0;

				CoefBetaOrbitals = CoefOrbitals;
				EnerBetaOrbitals = EnerOrbitals;
				OccBetaOrbitals = OccAlphaOrbitals;
				SymBetaOrbitals = SymOrbitals;
				NAlphaOcc = NOcc;
				NBetaOcc = NOcc;
				NAlphaOrb = nReadOrb;
				NBetaOrb = nReadOrb;
				break;
			case 4 : 
				CoefAlphaOrbitals = CoefOrbitals;
				EnerAlphaOrbitals = EnerOrbitals;
				SymAlphaOrbitals = SymOrbitals;
				OccAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
				for(i=0;i<NOcc;i++)
					OccAlphaOrbitals[i] = 1.0;
				for(i=NOcc;i<NOrb;i++)
					OccAlphaOrbitals[i] = 0.0;

				CoefBetaOrbitals = CoefOrbitals;
				EnerBetaOrbitals = EnerOrbitals;
				OccBetaOrbitals = OccAlphaOrbitals;
				SymBetaOrbitals = SymOrbitals;
				NAlphaOcc = NOcc;
				NBetaOcc = NOcc;
				NAlphaOrb = nReadOrb;
				NBetaOrb = nReadOrb;
				break;
			}
			return TRUE;
    		}

		NOcc = 0;
  		ncart=NOrb/5;
		if(NOrb%5!=0) ncart++;
		nReadOrb = 0;
		nR = 5;
		for(n=0;n<ncart && nR==5;n++)
		{
    			{ char* e = fgets(t,taille,fd);}
			nR = sscanf(t,"%d %d %d %d %d",&NumOrb[0],&NumOrb[1],&NumOrb[2],&NumOrb[3],&NumOrb[4]);
			nReadOrb += nR;
			for(i=0;i<nR;i++) NumOrb[i]--;
			for(i=nR;i<5;i++) NumOrb[i]=NOrb-1;
			if(nR==0) break;

			if(itype<4)
			{
    				{ char* e = fgets(t,taille,fd);}
			/* Debug("%d %d %d %d %d\n",NumOrb[0],NumOrb[1],NumOrb[2],NumOrb[3],NumOrb[4]);*/
				sscanf(t,"%s %s %s %s %s",SymOrb[0],SymOrb[1],SymOrb[2],SymOrb[3],SymOrb[4]);
			/* Debug("%s %s %s %s %s\n",SymOrb[0],SymOrb[1],SymOrb[2],SymOrb[3],SymOrb[4]);*/
				for(i=0;i<nR;i++)
                			if(strstr(SymOrb[i],"O"))
						NOcc++;
				for(i=0;i<nR;i++)
			   		SymOrbitals[NumOrb[i]] = get_sym_orb(SymOrb[i]);
			}
			else
			{
				for(i=0;i<nR;i++)
			   		SymOrbitals[NumOrb[i]] = g_strdup("UNK");
			}

			
    			{ char* e = fgets(t,taille,fd);}
			sscanf(t,"%s %s %lf %lf %lf %lf %lf",dump1,dump2,
				&EnerOrb[0], &EnerOrb[1], &EnerOrb[2], &EnerOrb[3], &EnerOrb[4]);

			for(i=0;i<nR;i++)
                        	EnerOrbitals[NumOrb[i]] = EnerOrb[i];
			if(itype>3)
			{
				for(i=0;i<nR;i++)
                			if(EnerOrb[i]>0)
						NOcc++;
			}

			for(i=0;i<NOrb;i++)
			{
    				{ char* e = fgets(t,taille,fd);}
				tmp = t + 20;
				/* k = sscanf(tmp,"%lf %lf %lf %lf %lf",&CoefOrbitals[NumOrb[0]][i],&CoefOrbitals[NumOrb[1]][i],*/
				sscanf(tmp,"%lf %lf %lf %lf %lf",&CoefOrbitals[NumOrb[0]][i],&CoefOrbitals[NumOrb[1]][i],
				&CoefOrbitals[NumOrb[2]][i],&CoefOrbitals[NumOrb[3]][i],&CoefOrbitals[NumOrb[4]][i]);
			}
		}
		/*
		Debug("End ncart\n");
		Debug("nR = %d\n",nR);
		*/
		if(nR!=5)
		{
			for(i=nReadOrb;i<NOrb;i++)
			   		SymOrbitals[i] = g_strdup("DELETE");
		}
		
		/*
		if(NOrb%5 !=0 && nR==5)
		{
		  nReadOrb += NOrb%5;
		  switch(NOrb%5)
		  {
			case 1:
	  			fgets(t,taille,fd);
				sscanf(t,"%d",&NumOrb[0]);
				for(i=0;i<1;i++)
					NumOrb[i]--;
				if(itype<4)
				{
	  			fgets(t,taille,fd);
				sscanf(t,"%s",SymOrb[0]);
				if(strstr(SymOrb[0],"O"))
						NOcc++;
				SymOrbitals[NumOrb[0]] = get_sym_orb(SymOrb[0]);
				}
				else
				SymOrbitals[NumOrb[0]] = g_strdup("UNK");

	  			fgets(t,taille,fd);
				sscanf(t,"%s %s %lf ",dump1,dump2,&EnerOrb[0]);
                        	EnerOrbitals[NumOrb[0]] = EnerOrb[0];
				if(itype>3)
				{
					for(i=0;i<1;i++)
                				if(EnerOrb[i]>0)
						NOcc++;
				}
				for(i=0;i<NOrb;i++)
				{
	  				fgets(t,taille,fd);
					tmp = t + 20;
					k = sscanf(tmp,"%lf ",&CoefOrbitals[NumOrb[0]][i]);
				}
				break;

			case 2:
	  			fgets(t,taille,fd);
				sscanf(t,"%d %d",&NumOrb[0],&NumOrb[1]);
				for(i=0;i<5;i++)
					NumOrb[i]--;

				if(itype<4)
				{
	  			fgets(t,taille,fd);
				sscanf(t,"%s %s",SymOrb[0],SymOrb[1]);
				for(i=0;i<2;i++)
					if(strstr(SymOrb[i],"O"))
						NOcc++;
				for(i=0;i<2;i++)
					SymOrbitals[NumOrb[i]] = get_sym_orb(SymOrb[i]);
				}
				else
				{
					for(i=0;i<2;i++)
						SymOrbitals[NumOrb[i]] = g_utf8_strup("UNK");
				}

	  			fgets(t,taille,fd);
				sscanf(t,"%s %s %lf %lf",dump1,dump2,&EnerOrb[0], &EnerOrb[1]);
				for(i=0;i<2;i++)
                        		EnerOrbitals[NumOrb[i]] = EnerOrb[i];
				if(itype>3)
				{
					for(i=0;i<2;i++)
                				if(EnerOrb[i]>0)
						NOcc++;
				}
				for(i=0;i<NOrb;i++)
				{
	  				fgets(t,taille,fd);
					tmp = t + 20;
					k = sscanf(tmp,"%lf %lf ",&CoefOrbitals[NumOrb[0]][i],&CoefOrbitals[NumOrb[1]][i]);
				}
				break;
			case 3:
	  			fgets(t,taille,fd);
				sscanf(t,"%d %d %d",&NumOrb[0],&NumOrb[1],&NumOrb[2]);
				for(i=0;i<5;i++)
					NumOrb[i]--;
				if(itype<4)
				{
	  			fgets(t,taille,fd);
				sscanf(t,"%s %s %s",SymOrb[0],SymOrb[1],SymOrb[2]);
				for(i=0;i<3;i++)
					if(strstr(SymOrb[i],"O"))
						NOcc++;
				for(i=0;i<3;i++)
					SymOrbitals[NumOrb[i]] = get_sym_orb(SymOrb[i]);
				}
				else
				{
					for(i=0;i<3;i++)
						SymOrbitals[NumOrb[i]] = g_utf8_strup("UNK");
				}

	  			fgets(t,taille,fd);
				sscanf(t,"%s %s %lf %lf %lf ",dump1,dump2,&EnerOrb[0], &EnerOrb[1], &EnerOrb[2]);
				for(i=0;i<3;i++)
                        		EnerOrbitals[NumOrb[i]] = EnerOrb[i];
				if(itype>3)
				{
					for(i=0;i<3;i++)
                				if(EnerOrb[i]>0)
						NOcc++;
				}
				for(i=0;i<NOrb;i++)
				{
	  				fgets(t,taille,fd);
					tmp = t + 20;
					k = sscanf(tmp,"%lf %lf %lf ",&CoefOrbitals[NumOrb[0]][i],&CoefOrbitals[NumOrb[1]][i],&CoefOrbitals[NumOrb[2]][i]);
				}
				break;
			case 4:
	  			fgets(t,taille,fd);
				sscanf(t,"%d %d %d %d",&NumOrb[0],&NumOrb[1],&NumOrb[2],&NumOrb[3]);
				for(i=0;i<5;i++)
					NumOrb[i]--;
				if(itype<4)
				{
	  			fgets(t,taille,fd);
				sscanf(t,"%s %s %s %s",SymOrb[0],SymOrb[1],SymOrb[2],SymOrb[3]);
				for(i=0;i<4;i++)
					if(strstr(SymOrb[i],"O"))
						NOcc++;
				for(i=0;i<4;i++)
					SymOrbitals[NumOrb[i]] = get_sym_orb(SymOrb[i]);
				}
				else
				{
				for(i=0;i<4;i++)
					SymOrbitals[NumOrb[i]] = g_strdup("UNK");
				}

	  			fgets(t,taille,fd);
				sscanf(t,"%s %s %lf %lf %lf %lf",dump1,dump2,
					&EnerOrb[0], &EnerOrb[1], &EnerOrb[2], &EnerOrb[3]);
				for(i=0;i<4;i++)
                        		EnerOrbitals[NumOrb[i]] = EnerOrb[i];
				if(itype>3)
				{
					for(i=0;i<4;i++)
                				if(EnerOrb[i]>0)
						NOcc++;
				}
				for(i=0;i<NOrb;i++)
				{
	  				fgets(t,taille,fd);
					tmp = t + 20;
					k = sscanf(tmp,"%lf %lf %lf %lf",
					&CoefOrbitals[NumOrb[0]][i],&CoefOrbitals[NumOrb[1]][i],&CoefOrbitals[NumOrb[2]][i],&CoefOrbitals[NumOrb[3]][i]);
					
				}
		  }
			
		}
	*/
 	}while(!feof(fd));

	/* Debug("End of read \n"); */
 	fclose(fd);
 	g_free(t);
 	for(i=0;i<5;i++)
		g_free(AtomCoord[i]);

	CoefAlphaOrbitals = CoefOrbitals;
	EnerAlphaOrbitals = EnerOrbitals;
	return TRUE;
}
/********************************************************************************/
gboolean read_orbitals_in_gabedit_or_molden_file(gchar *fileName,gint itype)
{
 	gchar *t;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	FILE *fd;
 	gint taille=BSIZE;
 	gint i;
 	gint numorb;
 	gchar *pdest;
	gint n = 0;
	gdouble **CoefOrbitals;
	gdouble *EnerOrbitals;
	gdouble *OccOrbitals;
	gchar **SymOrbitals;
	gint NOcc = 0;
	gint idump;

	
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return FALSE;
 	}

 	t=g_malloc(taille);
 	fd = FOpen(fileName, "rb");
 	if(fd ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
  		return FALSE;
 	}

 	for(i=0;i<5;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(char));
  
	CoefOrbitals = CreateTable2(NOrb);
	EnerOrbitals = g_malloc(NOrb*sizeof(gdouble));
	OccOrbitals = g_malloc(NOrb*sizeof(gdouble));
	SymOrbitals = g_malloc(NOrb*sizeof(gchar*));

	/* printf(" NAOrb = %d\n",NOrb);*/

 	numorb =1;
 	do 
 	{
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
          		pdest = strstr( t, "[MO]" );
	 		if ( pdest != NULL )
	  		{
                		numorb++;
                		OK = TRUE;
	  			break;
	  		}
        	}
 		if(!OK && (numorb == 1) )
		{
			gchar buffer[BSIZE];
			sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  			Message(buffer,_("Error"),TRUE);
			FreeTable2(CoefOrbitals,NOrb);
			g_free(EnerOrbitals);
			g_free(SymOrbitals);
			return FALSE;
    		}
 		if(!OK) goto end;

		n = -1;
		while(!feof(fd))
		{
			gdouble e = 0.0;
			gdouble o = 0.0;
			gchar* begin = NULL;
			gchar* spin = NULL;
			gchar sym[BSIZE];
			gboolean begincoef = FALSE;
			
			/* 
			 * Ene=      -2.5353 
			 * Spin= Alpha 
			 * Occup=   2.000000
			 */
			sym[0] = '\0';
			while(!feof(fd) && !begincoef)
			{
    				{ char* e = fgets(t,taille,fd);}
				if( this_is_a_backspace(t))
				{
					begincoef = FALSE;
					break;
				}
				if(atoi(t) != 0)
				{
					begincoef = TRUE;
					break;
				}
				begin = strstr(t,"=")+1;
				if(strstr(t,"Ene")!= 0) e = atof(begin);
				if( strstr(t,"Occ") != 0) o = atof(begin);
				if( strstr(t,"Spin") != 0) spin = g_strdup(begin);
				if( strstr(t,"Sym") != 0) sscanf(begin,"%s",sym);
			}
			if(!begincoef || feof(fd)) break;
			if(!spin) continue;
			if(strstr(spin,"Alpha") && itype == 2) continue;
			if(strstr(spin,"Beta") && itype == 1) continue;

			n++;
			EnerOrbitals[n] = e;
			if(sym[0] != '\0') SymOrbitals[n] = g_strdup(sym);
			else SymOrbitals[n] = g_strdup(_("Unknown"));

			OccOrbitals[n] = o;

			if(o>0) NOcc++;

			sscanf(t,"%d %lf",&idump,&CoefOrbitals[n][0]);
			for(i=1;i<NAOrb;i++)
			{
    				{ char* e = fgets(t,taille,fd);}
				sscanf(t,"%d %lf",&idump,&CoefOrbitals[n][i]);
			}
			if(n == NOrb-1) OK = FALSE;
		}
		
 	}while(!feof(fd));

end:
	/* Debug("End of read \n");*/
 	fclose(fd);
 	g_free(t);
	for(i=0;i<5;i++)
		g_free(AtomCoord[i]);

	switch(itype)
	{
		case 1 : 
			CoefAlphaOrbitals = CoefOrbitals;
			EnerAlphaOrbitals = EnerOrbitals;
			OccAlphaOrbitals = OccOrbitals;
			SymAlphaOrbitals = SymOrbitals;
			NAlphaOcc = NOcc;
			NAlphaOrb = n+1;
			break;
		case 2 : 
			CoefBetaOrbitals = CoefOrbitals;
			EnerBetaOrbitals = EnerOrbitals;
			OccBetaOrbitals = OccOrbitals;
			SymBetaOrbitals = SymOrbitals;
			NBetaOcc = NOcc;
			NBetaOrb = n+1;
			break;
		case 3 : 
			CoefAlphaOrbitals = CoefOrbitals;
			EnerAlphaOrbitals = EnerOrbitals;
			OccAlphaOrbitals = OccOrbitals;
			SymAlphaOrbitals = SymOrbitals;
			CoefBetaOrbitals = CoefOrbitals;
			EnerBetaOrbitals = EnerOrbitals;
			OccBetaOrbitals = OccOrbitals;
			SymBetaOrbitals = SymOrbitals;
			NAlphaOcc = NOcc;
			NBetaOcc = NOcc;
			NAlphaOrb = n+1;
			NBetaOrb = n+1;
			break;
		}
		if(n<0)
			return FALSE;
		else
			return TRUE;
}

/********************************************************************************/
void read_gauss_orbitals(gchar* FileName)
{
	gint typefile;
	gint typebasis;
	gboolean Ok = FALSE;
	gchar *t = NULL;


	typefile =get_type_file_orb(FileName);
	if(typefile==GABEDIT_TYPEFILE_UNKNOWN) return;
	if(typefile != GABEDIT_TYPEFILE_GAUSSIAN)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not read this format in '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return ;
	}
	free_data_all();
	t = get_name_file(FileName);
	set_status_label_info(_("File name"),t);
	g_free(t);
	set_status_label_info(_("File type"),"Gaussian");
	set_status_label_info(_("Mol. Orb."),_("Reading"));
	
	free_orbitals();	
 	if(!gl_read_gaussn_file_geomi(FileName,-1))
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}

	set_status_label_info(_("Mol. Orb."),_("Reading"));
 	InitializeAll();
 	if(!DefineBasisType(FileName))
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}

	typebasis =get_type_basis_in_gaussian_file(FileName);

	if(typebasis == 1)
	{
 		DefineSphericalBasis();
		sphericalBasis = TRUE;
	}
	else
	{
 		DefineCartBasis();
		sphericalBasis = FALSE;
	}
 	
 	/* PrintAllBasis();*/
 	NormaliseAllBasis();
 	/* PrintAllBasis();*/
 	DefineNOccs();


	Ok = read_last_orbitals_in_gaussian_file(FileName,1);
	if(Ok)
	{
		Ok = read_last_orbitals_in_gaussian_file(FileName,2);
	}
	else
	{
		Ok = read_last_orbitals_in_gaussian_file(FileName,3);
	}
	if(!Ok)
		Ok = read_last_orbitals_in_gaussian_file(FileName,4); /* Natural Orbitals */
	if(Ok)
	{
		/* PrintAllOrb(CoefAlphaOrbitals);*/
		set_status_label_info(_("Mol. Orb."),_("Ok"));
		glarea_rafresh(GLArea); /* for geometry*/
		NumSelOrb = NAlphaOcc-1;
		create_list_orbitals();
	}
	else
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
	}

} 
/********************************************************************************/
gboolean read_orbitals_from_fchk_gaussian_file(gchar* fileName)
{
	FILE* file;
	file = FOpen(fileName, "rb");
	gdouble* coefsAlpha = NULL;
	gdouble* coefsBeta = NULL;
	gdouble* energiesAlpha = NULL;
	gdouble* energiesBeta = NULL;
	gint nA = 0;
	gint nAOcc = 0;
	gint nB = 0;
	gint nBOcc = 0;
	gint nBasis;
	gint n,i,k,ib;
	if(file ==NULL)
	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  		return FALSE;
	}
	nAOcc = get_one_int_from_fchk_gaussian_file(file,"Number of alpha electrons ");
	nBOcc = get_one_int_from_fchk_gaussian_file(file,"Number of beta electrons ");
/* printf("nBOcc=%d\n",nBOcc);*/
	nBasis = get_one_int_from_fchk_gaussian_file(file,"Number of basis functions  ");
	if(nBasis<1)
	{
  		Message(_("Sorry\nI can not read the number of basis functions"),_("Error"),TRUE);
		fclose(file);
  		return FALSE;
	}
	energiesAlpha = get_array_real_from_fchk_gaussian_file(file, "Alpha Orbital Energies ", &nA);
	if(energiesAlpha)
	{
		coefsAlpha = get_array_real_from_fchk_gaussian_file(file, "Alpha MO coefficients ", &n);
		if(!coefsAlpha || n!=nBasis*nA)
		{
  			Message(_("Sorry\nI can not read the alpha MO coefficients"),_("Error"),TRUE);
			if(energiesAlpha) g_free(energiesAlpha);
			if(coefsAlpha) g_free(coefsAlpha);
			fclose(file);
  			return FALSE;
		}
	}
	rewind(file);
	energiesBeta = get_array_real_from_fchk_gaussian_file(file, "Beta Orbital Energies ", &nB);
	if(energiesBeta)
	{
		coefsBeta = get_array_real_from_fchk_gaussian_file(file, "Beta MO coefficients ", &n);
		if(!coefsBeta || n!=nBasis*nB)
		{
  			Message(_("Sorry\nI can not read the alpha MO coefficients"),_("Error"),TRUE);
			if(energiesBeta) g_free(energiesBeta);
			if(coefsBeta) g_free(coefsBeta);
			fclose(file);
  			return FALSE;
		}
	}
	fclose(file);
	NAlphaOcc = 0;
	NAlphaOrb = 0;
	NBetaOcc = 0;
	NBetaOrb = 0;
	NTotOcc = NOrb;

	if(coefsAlpha && energiesAlpha && nA>0 && nA<=nBasis)
	{
		EnerAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		for(i=0;i<nA;i++) EnerAlphaOrbitals[i] = energiesAlpha[i];
		for(i=nA;i<NOrb;i++) EnerAlphaOrbitals[i] = 0.0;

		CoefAlphaOrbitals = CreateTable2(NOrb);

		k = 0;
		for(i=0;i<nA;i++) 
		for(ib=0;ib<nA;ib++) 
			CoefAlphaOrbitals[i][ib] = coefsAlpha[k++];
		for(i=nA;i<NOrb;i++)
		for(ib=0;ib<nA;ib++) 
			CoefAlphaOrbitals[i][ib] = 0;

		SymAlphaOrbitals = g_malloc(NOrb*sizeof(gchar*));
		for(i=0;i<nA;i++) SymAlphaOrbitals[i] = g_strdup("UNK");
		for(i=nA;i<NOrb;i++) SymAlphaOrbitals[i] = g_strdup("DELETE");

		OccAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		for(i=0;i<nAOcc;i++) OccAlphaOrbitals[i] = 1.0;
		for(i=nAOcc;i<NOrb;i++) OccAlphaOrbitals[i] = 0.0;
		NAlphaOcc = nAOcc;
		NAlphaOrb = nA;
		if(!energiesBeta && !coefsBeta && nB == 0 && nBOcc==nAOcc)
		{
			CoefBetaOrbitals = CoefAlphaOrbitals;
			EnerBetaOrbitals = EnerAlphaOrbitals;
			OccBetaOrbitals = OccAlphaOrbitals;
			SymBetaOrbitals = SymAlphaOrbitals;
			NBetaOcc = nAOcc;
			NBetaOrb = nA;
		}
	}
	if(coefsBeta && energiesBeta && nB>0 && nB<=nBasis)
	{
		EnerBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		for(i=0;i<nB;i++) EnerBetaOrbitals[i] = energiesBeta[i];
		for(i=nB;i<NOrb;i++) EnerBetaOrbitals[i] = 0.0;

		CoefBetaOrbitals = CreateTable2(NOrb);

		k = 0;
		for(i=0;i<nB;i++) 
		for(ib=0;ib<nB;ib++) 
			CoefBetaOrbitals[i][ib] = coefsBeta[k++];
		for(i=nB;i<NOrb;i++)
		for(ib=0;ib<nB;ib++) 
			CoefBetaOrbitals[i][ib] = 0;

		SymBetaOrbitals = g_malloc(NOrb*sizeof(gchar*));
		for(i=0;i<nB;i++) SymBetaOrbitals[i] = g_strdup("UNK");
		for(i=nB;i<NOrb;i++) SymBetaOrbitals[i] = g_strdup("DELETE");

		OccBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		for(i=0;i<nBOcc;i++) OccBetaOrbitals[i] = 1.0;
		for(i=nBOcc;i<NOrb;i++) OccBetaOrbitals[i] = 0.0;
		NBetaOcc = nBOcc;/* bug fixed 03 Sep 2012 */
		NBetaOrb = nB;
		if(!energiesAlpha && !coefsAlpha && nA == 0 && nBOcc==nAOcc)
		{
			CoefAlphaOrbitals = CoefBetaOrbitals;
			EnerAlphaOrbitals = EnerBetaOrbitals;
			OccAlphaOrbitals = OccBetaOrbitals;
			SymAlphaOrbitals = SymBetaOrbitals;
			NAlphaOcc = nAOcc;
			NAlphaOrb = nB;
		}
	}
	if(energiesAlpha) g_free(energiesAlpha);
	if(coefsAlpha) g_free(coefsAlpha);
	if(energiesBeta) g_free(energiesBeta);
	if(coefsBeta) g_free(coefsBeta);
	NTotOcc = NOrb;
	return TRUE;
}
/********************************************************************************/
void read_fchk_gauss_orbitals(gchar* fileName)
{
	gint typefile;
	gchar *t = NULL;


	typefile =get_type_file_orb(fileName);
	if(typefile==GABEDIT_TYPEFILE_UNKNOWN) return;
	if(typefile != GABEDIT_TYPEFILE_GAUSSIAN_FCHK)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not read this format in '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
		return ;
	}
	free_data_all();
	t = get_name_file(fileName);
	set_status_label_info(_("File name"),t);
	g_free(t);
	set_status_label_info(_("File type"),"Gaussian fchk");
	set_status_label_info(_("Mol. Orb."),_("Reading"));
	
	free_orbitals();	
 	if(!gl_read_fchk_gaussn_file_geom(fileName))
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}

	set_status_label_info(_("Mol. Orb."),_("Reading"));
 	InitializeAll();
 	if(!readBasisFromGaussianFChk(fileName))
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}

	sphericalBasis = TRUE; /* Can be also cart or mixed */
 	NormaliseAllBasis();
	if(read_orbitals_from_fchk_gaussian_file(fileName))
	{
		/* PrintAllOrb(CoefAlphaOrbitals);*/
		set_status_label_info(_("Mol. Orb."),_("Ok"));
		glarea_rafresh(GLArea); /* for geometry*/
		NumSelOrb = NAlphaOcc-1;
		create_list_orbitals();
	}
	else
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
	}
} 
/********************************************************************************/
void read_gabedit_orbitals(gchar* FileName)
{
	gint i;
	gint typefile;
	gint typebasis;
	gboolean OkAlpha = FALSE;
	gboolean OkBeta = FALSE;
	gchar* t = NULL;

	typefile =get_type_file_orb(FileName);

	if(typefile==GABEDIT_TYPEFILE_UNKNOWN)
		return;

	if(typefile != GABEDIT_TYPEFILE_GABEDIT)
	{
		Message(_("Sorry, This file is not in Gabedit Format\n"),_("Error"),TRUE);
		return ;
	}
	free_data_all();
	t = get_name_file(FileName);
	set_status_label_info(_("File name"),t);
	g_free(t);
	set_status_label_info(_("File type"),"Gabedit");
	set_status_label_info(_("Mol. Orb."),_("Reading"));
	free_orbitals();	
 	if(!gl_read_gabedit_file_geom(FileName))
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}
 	InitializeAll();
 	if(!DefineGabeditBasisType(FileName))
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}
	
	typebasis =get_type_basis_in_gabedit_file(FileName);
	if(typebasis == 1)
	{
 		DefineSphericalBasis();
		sphericalBasis = TRUE;
	}
	else
	{
 		DefineCartBasis(); 	
		sphericalBasis = FALSE;
	}
 	NormaliseAllBasis();
 	/* PrintAllBasis();*/

 	DefineNOccs();

	OkBeta = read_orbitals_in_gabedit_or_molden_file(FileName,2);/* if beta orbital*/
	OkAlpha = read_orbitals_in_gabedit_or_molden_file(FileName,1);
	if(!OkBeta && OkAlpha)
	{
		CoefBetaOrbitals = CoefAlphaOrbitals;
		EnerBetaOrbitals = EnerAlphaOrbitals;		
		SymBetaOrbitals = SymAlphaOrbitals;
		NBetaOrb = NAlphaOrb;

		OccBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		NBetaOcc = 0;
		for(i=0;i<NBetaOrb;i++)
		{
			if(OccAlphaOrbitals[i]>1.0)
			{
				NBetaOcc++;
				OccBetaOrbitals[i] = OccAlphaOrbitals[i]/2;
				OccAlphaOrbitals[i] = OccBetaOrbitals[i];
			}
			else
				OccBetaOrbitals[i] = 0.0;
		}
		if(NBetaOrb>0)
			OkBeta = TRUE;
	}
	if(OkAlpha || OkBeta)
	{
		read_gabedit_atomic_orbitals(FileName);
		set_status_label_info(_("Mol. Orb."),_("Ok"));
		glarea_rafresh(GLArea); /* for geometry*/
		NumSelOrb = NAlphaOcc-1;
		create_list_orbitals();
	}
	else
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
	}
	DefineType();
}
/********************************************************************************/
void read_molden_orbitals(gchar* FileName)
{
	gint i;
	gint typefile;
	gint typebasis;
	gboolean OkAlpha = FALSE;
	gboolean OkBeta = FALSE;
 	gchar *t = NULL;

	typefile =get_type_file_orb(FileName);

	if(typefile==GABEDIT_TYPEFILE_UNKNOWN)
		return;
	if(typefile != GABEDIT_TYPEFILE_MOLDEN)
	{
		Message(_("Sorry, This file is not in Molden Format\n"),_("Error"),TRUE);
		return ;
	}
	free_data_all();
	t = get_name_file(FileName);
	set_status_label_info(_("File name"),t);
	g_free(t);
	set_status_label_info(_("File type"),"Molden");
	set_status_label_info(_("Mol. Orb."),_("Reading"));

	free_orbitals();	
 	if(!gl_read_molden_file_geom(FileName))
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}

 	InitializeAll();
 	if(!DefineMoldenBasisType(FileName))
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}
	/*
 	Debug("End define Basis Type NAOrb = %d \n",NAOrb);
 	PrintBasis();
 	Debug("End Print Basis\n"); 
	*/
	
	typebasis =get_type_basis_in_molden_file(FileName);
	/* printf("typebasis = %d\n",typebasis);*/
	if(typebasis == 1)
	{
 		DefineSphericalBasis();
		sphericalBasis = TRUE;
	}
	else
	{
 		DefineCartBasis();
		sphericalBasis = FALSE;
	}
 	
	/* Debug("End DefineBasis\n");*/

 	/* PrintAllBasis();*/

 	NormaliseAllBasis();
	/* Debug("Basis after normalisation\n");*/
 	/* PrintAllBasis();*/
 	/* Debug("Spherical basis(Norb=%d)\n",NAOrb);*/
 	DefineNOccs();


	OkBeta = read_orbitals_in_gabedit_or_molden_file(FileName,2);/* if beta orbital*/
	/* Debug("Ok = %d\n",Ok);*/
	OkAlpha = read_orbitals_in_gabedit_or_molden_file(FileName,1);
	if(!OkBeta && OkAlpha)
	{
		CoefBetaOrbitals = CoefAlphaOrbitals;
		EnerBetaOrbitals = EnerAlphaOrbitals;		
		SymBetaOrbitals = SymAlphaOrbitals;
		NBetaOrb = NAlphaOrb;

		OccBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		NBetaOcc = 0;
		for(i=0;i<NBetaOrb;i++)
		{
			if(OccAlphaOrbitals[i]>1.0)
			{
				NBetaOcc++;
				OccBetaOrbitals[i] = OccAlphaOrbitals[i]/2;
				OccAlphaOrbitals[i] = OccBetaOrbitals[i];
			}
			else
				OccBetaOrbitals[i] = 0.0;
		}
		if(NBetaOrb>0)
			OkBeta = TRUE;
	}
	/* Debug("NA = %d\n",NAlphaOcc);*/
	/* Debug("NB = %d\n",NBetaOcc);*/
	if(OkBeta || OkAlpha)
	{
		/*
		PrintAllOrb(CoefAlphaOrbitals);
		Debug("End Orb\n");
		*/
		set_status_label_info(_("Mol. Orb."),_("Ok"));
		glarea_rafresh(GLArea); /* for geometry */
		NumSelOrb = NAlphaOcc-1;
		/* Debug("NumSelOrb = %d\n",NumSelOrb);*/
		create_list_orbitals();
	}
	else
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
	}
	DefineType();

}
/********************************************************************************/
void read_wfx_orbitals(gchar* fileName)
{
	gint typefile;
	gchar *t = NULL;


	typefile =get_type_file_orb(fileName);
	if(typefile==GABEDIT_TYPEFILE_UNKNOWN) return;
	if(typefile != GABEDIT_TYPEFILE_WFX)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I cannot read this format in '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
		return ;
	}
	free_data_all();
	t = get_name_file(fileName);
	set_status_label_info(_("File name"),t);
	g_free(t);
	set_status_label_info(_("File type"),"Gaussian fchk");
	set_status_label_info(_("Mol. Orb."),_("Reading"));
	
	free_orbitals();	
 	if(!gl_read_wfx_file_geom(fileName))
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}

	set_status_label_info(_("Mol. Orb."),_("Reading"));
 	InitializeAll();
 	if(!readBasisFromWFX(fileName))
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}

	sphericalBasis = FALSE; /* Can be also cart or mixed */
 	//NormaliseAllBasis();
	if(read_orbitals_from_wfx_file(fileName))
	{
		/* PrintAllOrb(CoefAlphaOrbitals);*/
		set_status_label_info(_("Mol. Orb."),_("Ok"));
		glarea_rafresh(GLArea); /* for geometry*/
		NumSelOrb = NAlphaOcc-1;
		create_list_orbitals();
	}
	else
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
	}
	DefineType();
} 
/********************************************************************************/
void read_dalton_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id)
{
 	/* gchar *FileName;*/

	if(response_id != GTK_RESPONSE_OK) return;
 	/* FileName = gabedit_file_chooser_get_current_file(SelecFile);*/
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
 	/* read_dalton_orbitals(FileName);*/
} 
/********************************************************************************/
void read_gamess_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
 	read_gamess_orbitals(FileName);
} 
/********************************************************************************/
void read_nbo_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
 	read_nbo_orbitals(FileName);
} 
/********************************************************************************/
/********************************************************************************/
void read_orca_orbitals_sel_2mkl(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *fileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
	read_orca_orbitals_using_orca_2mkl(fileName);
} 
/********************************************************************************/
void read_orca_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *fileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
	read_orca_orbitals_from_output_file(fileName);
} 
/********************************************************************************/
void read_qchem_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
 	read_qchem_orbitals(FileName);
} 
/********************************************************************************/
void read_nwchem_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
 	read_nwchem_orbitals(FileName);
} 
/********************************************************************************/
void read_mopac_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
 	read_mopac_orbitals(FileName);
} 
/********************************************************************************/
void read_gauss_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	add_objects_for_new_grid();
 	read_gauss_orbitals(FileName);
} 
/********************************************************************************/
void read_fchk_gauss_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
 	read_fchk_gauss_orbitals(FileName);
} 
/********************************************************************************/
void read_molpro_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	add_objects_for_new_grid();
 	read_molpro_orbitals(FileName);
	sphericalBasis = TRUE;
} 
/********************************************************************************/
void read_gabedit_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	add_objects_for_new_grid();
 	read_gabedit_orbitals(FileName);
} 
/********************************************************************************/
void save_gabedit_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

 	save_gabedit_orbitals(FileName);
} 
/********************************************************************************/
void save_wfx_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *fileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	export_to_wfx_file(fileName, -1, 0, 1);
} 
/********************************************************************************/
void read_molden_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	add_objects_for_new_grid();
 	read_molden_orbitals(FileName);
} 
/********************************************************************************/
void read_wfx_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	add_objects_for_new_grid();
 	read_wfx_orbitals(FileName);
} 

/*******************************************************/
gboolean read_orbitals(gchar* fileName)
{
	GabEditTypeFile fileType = get_type_file(fileName);
	if(fileType == GABEDIT_TYPEFILE_GAMESS) read_gamess_orbitals(fileName);
	else if(fileType == GABEDIT_TYPEFILE_FIREFLY) read_gamess_orbitals(fileName);
	else if(fileType == GABEDIT_TYPEFILE_GAUSSIAN) read_gauss_orbitals(fileName);
	else if(fileType == GABEDIT_TYPEFILE_GAUSSIAN_FCHK) read_fchk_gauss_orbitals(fileName);
	else if(fileType == GABEDIT_TYPEFILE_MOLPRO) read_molpro_orbitals(fileName);
	else if(fileType == GABEDIT_TYPEFILE_MOPAC) read_mopac_orbitals(fileName);
	else if(fileType == GABEDIT_TYPEFILE_MOPAC_AUX) read_mopac_orbitals(fileName);
	else if(fileType == GABEDIT_TYPEFILE_ORCA) read_orca_orbitals_from_output_file(fileName);
	else if(fileType == GABEDIT_TYPEFILE_NWCHEM) read_nwchem_orbitals(fileName);
	else if(fileType == GABEDIT_TYPEFILE_QCHEM) read_qchem_orbitals(fileName);
	else if(fileType == GABEDIT_TYPEFILE_GABEDIT) read_gabedit_orbitals(fileName);
	else if(fileType == GABEDIT_TYPEFILE_MOLDEN) read_molden_orbitals(fileName);
	else if(fileType == GABEDIT_TYPEFILE_UNKNOWN) 
	{
		Message(
			_("Sorry, I cannot find the type of your file\n")
			," Error ",TRUE);
		return FALSE;
	}
	return TRUE;
}
/*************************************************************************/
void read_orbitals_sel(GabeditFileChooser *selecFile, gint response_id)
{
	gchar *fileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(selecFile);
	gtk_widget_hide(GTK_WIDGET(selecFile));
	while( gtk_events_pending() ) gtk_main_iteration();
 
	add_objects_for_new_grid();
 	read_orbitals(fileName);
}
