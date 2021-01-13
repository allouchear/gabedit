/* Status.c */
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
#include "Orbitals.h"
#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"

static GtkWidget *ProgressBar = NULL;
static	GtkWidget* Status[2][4]; 
static	GtkWidget *StatusProgress = NULL;
static	GtkWidget *button = NULL;
static  GtkWidget *handleboxStatus = NULL;
static GtkWidget *tableProgressBar = NULL;
/********************************************************************************/
void show_handlebox_status(gboolean show)
{
	if(!handleboxStatus) return;
	gtk_widget_hide(handleboxStatus);
	if(show) gtk_widget_show(handleboxStatus);
}
/********************************************************************************/
void show_progress_bar(gboolean show)
{
	if(!tableProgressBar) return;
	gtk_widget_hide(tableProgressBar);
	if(show) gtk_widget_show(tableProgressBar);
}
/********************************************************************************/
void set_button_sensitive(gboolean sens)
{
	if(this_is_an_object((GtkObject*)button))
		gtk_widget_set_sensitive(button, sens); 

}
/********************************************************************************/
void setTextInProgress(gchar* t)
{

	guint idStatus = 0;
	idStatus= gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusProgress),"Testing");
	gtk_statusbar_pop(GTK_STATUSBAR(StatusProgress),idStatus);
	gtk_statusbar_push(GTK_STATUSBAR(StatusProgress),idStatus, t);
    	while( gtk_events_pending() ) gtk_main_iteration();
}
/********************************************************************************/
gint progress_orb_txt(gdouble scal,gchar* str,gboolean reset)
{

	gdouble new_val;
	guint idStatus = 0;

	gtk_widget_set_sensitive(button, FALSE); 
	idStatus= gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusProgress),"Testing");
	gtk_statusbar_pop(GTK_STATUSBAR(StatusProgress),idStatus);
	if(reset)
	{
		gtk_widget_show(ProgressBar);
		new_val = 0;
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (ProgressBar), new_val);
		gtk_widget_set_sensitive(button, FALSE); 

		gtk_statusbar_pop(GTK_STATUSBAR(StatusProgress),idStatus);
		gtk_statusbar_push(GTK_STATUSBAR(StatusProgress),idStatus, str);
		while( gtk_events_pending() ) gtk_main_iteration();
		return TRUE;
	}
	else if(scal>0)
	{
    		new_val = gtk_progress_bar_get_fraction( GTK_PROGRESS_BAR(ProgressBar) ) + scal;
		if (new_val > 1) new_val = 1;
	}
	else
	{
    		new_val = gtk_progress_bar_get_fraction( GTK_PROGRESS_BAR(ProgressBar) ) - scal;
		if (new_val > 1) new_val = 1;
		if (new_val <0 ) new_val = 0;
	}
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (ProgressBar), new_val);
	gtk_widget_set_sensitive(button, TRUE); 

	gtk_statusbar_pop(GTK_STATUSBAR(StatusProgress),idStatus);
	gtk_statusbar_push(GTK_STATUSBAR(StatusProgress),idStatus, str);
	while( gtk_events_pending() ) gtk_main_iteration();

	return TRUE;
}
/********************************************************************************/
gint progress_orb(gdouble scal,GabEditTypeProgressOrb  type,gboolean reset)
{

	gdouble new_val;
	gchar *t = NULL;
	guint idStatus = 0;

	if(reset)
	{
		gtk_widget_show(ProgressBar);
		new_val = 0;

    		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (ProgressBar), new_val);

		gtk_widget_set_sensitive(button, FALSE); 
		idStatus= gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusProgress),"Testing");
		gtk_statusbar_pop(GTK_STATUSBAR(StatusProgress),idStatus);

		while( gtk_events_pending() ) gtk_main_iteration();

		return TRUE;
	}
	else
		if(scal>0)
		{
    			new_val = gtk_progress_bar_get_fraction( GTK_PROGRESS_BAR(ProgressBar) ) + scal;
			if (new_val > 1) new_val = 1;
		}
		else
		{
    			new_val = gtk_progress_bar_get_fraction( GTK_PROGRESS_BAR(ProgressBar) ) - scal;
			if (new_val > 1) new_val = 1;
			if (new_val <0 ) new_val = 0;
		}

		while( gtk_events_pending() ) gtk_main_iteration();

    /* Set the new value */
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (ProgressBar), new_val);

	switch(type)
	{

	case GABEDIT_PROGORB_UNK : 
		t = g_strdup_printf(_(" %.0f%%"),new_val*100);
		break;

	case  GABEDIT_PROGORB_READGEOM: 
		t = g_strdup_printf(_(" Geometry Reading : %.0f%%"),new_val*100);
		break;
	case  GABEDIT_PROGORB_SAVEGEOM: 
		t = g_strdup_printf(_(" Recording of the geometry : %.0f%%"),new_val*100);
		break;
	case  GABEDIT_PROGORB_COMPINTEG: 
		t = g_strdup_printf(_(" Computing of an integral : %.0f%%"),new_val*100);
		break;
	case GABEDIT_PROGORB_COMPGRID :
		if(TypeGrid == GABEDIT_TYPEGRID_EDENSITY)
		t = g_strdup_printf(_(" Computing of the electronic density grid : %.0f%%"),new_val*100);
		else
		if(TypeGrid == GABEDIT_TYPEGRID_ORBITAL)
		t = g_strdup_printf(_(" Grid computing for an orbital : %.0f%%"),new_val*100);
		else
		if(TypeGrid == GABEDIT_TYPEGRID_ELFBECKE || TypeGrid == GABEDIT_TYPEGRID_ELFSAVIN)
		t = g_strdup_printf(_(" Grid computing for the ELF : %.0f%%"),new_val*100);
		else
		if(TypeGrid == GABEDIT_TYPEGRID_FEDELECTROPHILIC)
		t = g_strdup_printf(_(" Grid computing for the Electro. susceptibility : %.0f%%"),new_val*100);
		else
		if(TypeGrid == GABEDIT_TYPEGRID_FEDNUCLEOPHILIC)
		t = g_strdup_printf(_(" Grid computing for the Nucleo. susceptibility : %.0f%%"),new_val*100);
		else
		if(TypeGrid == GABEDIT_TYPEGRID_FEDRADICAL)
		t = g_strdup_printf(_(" Grid computing for the Radical. susceptibility : %.0f%%"),new_val*100);
		else
		t = g_strdup_printf(_(" Grid Computing : %.0f%%"),new_val*100);
		break;
	case GABEDIT_PROGORB_SCALEGRID :
		t = g_strdup_printf(_(" Scale grid : %.0f%%"),new_val*100);
		break;
	case GABEDIT_PROGORB_SUBSGRID :
		t = g_strdup_printf(_(" Subtract grid : %.0f%%"),new_val*100);
		break;
	case GABEDIT_PROGORB_COMPISOSURFACE :
		t = g_strdup_printf(_(" Isosurface Computing : %.0f%%"),new_val*100);
		break;
	case GABEDIT_PROGORB_SAVEGRID :
		t = g_strdup_printf(_(" Save Grid : %.0f%%"),new_val*100);
		break;
	case GABEDIT_PROGORB_READGRID :
		t = g_strdup_printf(_(" Grid reading : %.0f%%"),new_val*100);
		break;
	case GABEDIT_PROGORB_MAPGRID :
		t = g_strdup_printf(_(" Grid mapping : %.0f%%"),new_val*100);
		break;
	case GABEDIT_PROGORB_COMPLAPGRID :
		t = g_strdup_printf(_(" Computing of the laplacian of the grid : %.0f%%"),new_val*100);
		break;
	case GABEDIT_PROGORB_COMPGRADGRID :
		t = g_strdup_printf(_(" Computing of the gradient of the grid : %.0f%%"),new_val*100);
		break;
	case GABEDIT_PROGORB_COMPNCIGRID :
		t = g_strdup_printf(_(" Computing NCI from the density grid : %.0f%%"),new_val*100);
		break;
	case GABEDIT_PROGORB_COMPL2GRID :
		t = g_strdup_printf(_(" Computing sign of middle eigenvalue hessian * grid : %.0f%%"),new_val*100);
		break;
	case GABEDIT_PROGORB_SCANFILEGRID :
		t = g_strdup_printf(_(" Scan file for get the total orbitals number ....."));
		break;
	case GABEDIT_PROGORB_COMPMEPGRID :
		t = g_strdup_printf(_(" Computing of the MEP : %.0f%%"),new_val*100);
		break;
	case GABEDIT_PROGORB_COMPMULTIPOL :
		t = g_strdup_printf(_(" Multipole Computing : %.0f%%"),new_val*100);
		break;
	}
	if(type!= GABEDIT_PROGORB_SCANFILEGRID || (type==GABEDIT_PROGORB_SCANFILEGRID && scal==0))
	{
		idStatus= gtk_statusbar_get_context_id(GTK_STATUSBAR(StatusProgress),"Testing");
		gtk_statusbar_pop(GTK_STATUSBAR(StatusProgress),idStatus);
		gtk_statusbar_push(GTK_STATUSBAR(StatusProgress),idStatus, t);
	}
	if(
		type==GABEDIT_PROGORB_READGRID || 
		type==GABEDIT_PROGORB_COMPGRID || 
		type==GABEDIT_PROGORB_COMPINTEG || 
		type == GABEDIT_PROGORB_SCANFILEGRID || 
		type==GABEDIT_PROGORB_COMPMULTIPOL ||
		type==GABEDIT_PROGORB_COMPLAPGRID ||
		type==GABEDIT_PROGORB_COMPGRADGRID ||
		type==GABEDIT_PROGORB_COMPNCIGRID ||
		type==GABEDIT_PROGORB_COMPL2GRID ||
		type==GABEDIT_PROGORB_COMPMEPGRID
	)
		gtk_widget_set_sensitive(button, TRUE); 

    g_free(t);
    while( gtk_events_pending() ) gtk_main_iteration();

    return TRUE;
}
/***********************************************************/
void set_status_label_info(gchar* type,gchar* txt)
{
	gchar* tlabels[2][4]={{_("File name"),_("File type"),_("Geometry"),_("Mol. Orb.")},
			    {_("At. Orb."),_("Grid"),_("IsoSurface")," "}};
	guint idStatus = 0;
	gint i;
	gint j;

	for(i=0;i<2;i++)
		for(j=0;j<4;j++)
		if(strcmp(type,tlabels[i][j])==0)
		{
			gchar*t = g_strdup_printf(" %s : %s ",tlabels[i][j],txt);
			idStatus= gtk_statusbar_get_context_id(GTK_STATUSBAR(Status[i][j]),"Testing");
			gtk_statusbar_pop(GTK_STATUSBAR(Status[i][j]),idStatus);
			gtk_statusbar_push(GTK_STATUSBAR(Status[i][j]),idStatus, t);
			g_free(t);
			break;
		}
	progress_orb(0,GABEDIT_PROGORB_READGEOM,TRUE);
        while( gtk_events_pending() )
          gtk_main_iteration();
}
/***********************************************************/
void cancel_calcul(GtkWidget* win,gpointer data)  
{
	CancelCalcul = TRUE;
}
/***********************************************************/
GtkWidget *create_progress_bar_orb(GtkWidget *box)
{
	GtkWidget *pbar;
	GtkWidget *table;

	table = gtk_table_new(1,4,FALSE);
	
	gtk_box_pack_start (GTK_BOX(box), table, FALSE, TRUE, 2);
	gtk_widget_show (table);

	StatusProgress = gtk_statusbar_new();
	gtk_widget_show(StatusProgress);
	gtk_table_attach(GTK_TABLE(table),StatusProgress,0,2,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
    	pbar = gtk_progress_bar_new ();
    	gtk_widget_show(pbar);

	gtk_table_attach(GTK_TABLE(table),pbar,2,3,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			1,1);
	/*
	gtk_progress_bar_set_bar_style(GTK_PROGRESS_BAR(pbar),GTK_PROGRESS_DISCRETE);
	gtk_progress_bar_set_discrete_blocks(GTK_PROGRESS_BAR(pbar),15);
	*/

  	button = gtk_button_new_with_label(_("Cancel"));
	gtk_table_attach(GTK_TABLE(table),button,3,4,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			1,1);
  	gtk_widget_show_all (button);
	gtk_widget_set_sensitive(button, FALSE); 
	g_signal_connect(G_OBJECT(button), "clicked", (GCallback)cancel_calcul,NULL);  
	tableProgressBar = table;

	return pbar;
}
/********************************************************************************/
void create_status_bar_orb(GtkWidget* box)
{
  	GtkWidget *handlebox;
	GtkWidget* table; 
	gchar* label[2][4]={{_(" File name : Nothing"),_(" File type : Nothing"),_(" Geometry : Nothing"),_(" Mol. Orb. : Nothing")},
			    {_(" Atomic orb. : Nothing"),_(" Grid : Nothing"),_(" Iso Surface : Nothing"),_(" Right mouse button for popup menu.")}};
	guint idStatus = 0;
	gint i;
	gint j;

  	handlebox = gtk_handle_box_new ();
  	gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(handlebox),GTK_SHADOW_NONE);    
	gtk_handle_box_set_handle_position  (GTK_HANDLE_BOX(handlebox),GTK_POS_LEFT);
  	gtk_widget_show (handlebox);
  	gtk_box_pack_start (GTK_BOX (box), handlebox, FALSE, FALSE, 0);
	table = gtk_table_new(2,4,FALSE);

	gtk_container_add (GTK_CONTAINER (handlebox), table);
	/* Mode Status */
	for(i=0;i<2;i++)
		for(j=0;j<4;j++)
		{
			Status[i][j] = gtk_statusbar_new();
			gtk_widget_show(Status[i][j]);
			gtk_table_attach(GTK_TABLE(table),Status[i][j],j,j+1,i,i+1,
					(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
					(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
					1,1);
			idStatus= gtk_statusbar_get_context_id(GTK_STATUSBAR(Status[i][j]),"Testing");
			gtk_statusbar_pop(GTK_STATUSBAR(Status[i][j]),idStatus);
			gtk_statusbar_push(GTK_STATUSBAR(Status[i][j]),idStatus, label[i][j]);
		}


	gtk_widget_show(table);
	handleboxStatus = handlebox;
}
/********************************************************************************/
void create_status_progress_bar_orb(GtkWidget* vbox)
{
	ProgressBar = create_progress_bar_orb(vbox);
}
/********************************************************************************/
