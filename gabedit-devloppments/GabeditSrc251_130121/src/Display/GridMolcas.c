/* GridMolcas.c */
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
#include <ctype.h>
#include "GlobalOrb.h"
#include "StatusOrb.h"
#include "UtilsOrb.h"
#include "GeomOrbXYZ.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Constants.h"
#include "GLArea.h"
#include "AtomicOrbitals.h"
#include "Orbitals.h"
#include "BondsOrb.h"

typedef struct _DataRow
{
	gint numGrid;
	gint totalNumberOfGrids;
}DataRow;

static gint numberOfOrbitals = 0;
static gint totalNumberOfGrids = 0;
static gchar** labelsSymmetry = NULL;
static gdouble* occupations = NULL;
static gdouble* energies = NULL;
static gint* numGridOfOrbitals = NULL;
static gint selectedRow = -1;
static	gint N[3];
static	gdouble XYZ0[3]={0.0,0.0,0.0};
static	gdouble    X[3]={0.0,0.0,0.0};
static	gdouble    Y[3]={0.0,0.0,0.0};
static	gdouble    Z[3]={0.0,0.0,0.0};
static  gchar molcasgridFileName[2048];
static gdouble version = 0.0;

/**************************************************************/
static void free_molcasgrid_orb()
{
	gint i;
	if(numberOfOrbitals<1) return;
	if(labelsSymmetry)
	{
		for(i=0;i<numberOfOrbitals;i++)
			if(labelsSymmetry[i]) g_free(labelsSymmetry[i]);
		g_free(labelsSymmetry);
	}
	labelsSymmetry = NULL;

	if(occupations) g_free(occupations);
	occupations  = NULL;

	if(energies) g_free(energies);
	energies = NULL;

	if(numGridOfOrbitals) g_free(numGridOfOrbitals);
	numGridOfOrbitals = NULL;

}
/**************************************************************/
static gint read_one_block(FILE* file,gdouble* block, gint nMaxBlocK, gint nOld)
{
	gint len = BSIZE;
	gchar buffer[BSIZE];
	gint n = nOld;

	while(!feof(file))
	{
		if(!fgets(buffer,len,file))return n;
		/* printf("%s",buffer);*/
		if(strstr(buffer,"Title")) return n;
		if(1 != sscanf(buffer,"%lf",&block[n])) return 0;
		n++;
		if(nMaxBlocK<=n)break;
	}
	return n;
}
/**************************************************************/
static gint advance_one_block(FILE* file)
{
	gint len = BSIZE;
	gchar buffer[BSIZE];

	while(!feof(file))
	{
		if(!fgets(buffer,len,file)) return 0;
		if(strstr(buffer,"Title")) return 1;
	}
	return -1;
}
/********************************************************************************/
static gboolean get_values_from_molcasgrid_file(FILE* file,gdouble V[], gint nMaxBlock, gint numGrid)
{
	gint k = 0;
	gint kOld = 0;
	gint nG = 1;

	while(!feof(file))
	{
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_READGRID,TRUE);
			return FALSE;
		}
		if(nG==numGrid)
		{
			kOld = k;
			/* printf("numGrid=%d\n",numGrid);*/
			k = read_one_block(file,V,  nMaxBlock, kOld);
			/* printf("k=%d\n",k);*/
			if(k>=nMaxBlock)break;
		}
		else
			advance_one_block(file);
		nG++;
		if(nG>totalNumberOfGrids) nG-=totalNumberOfGrids;
	}
	if(k==nMaxBlock)
		return TRUE;

	return FALSE;
}
/********************************************************************************/
static gboolean set_position_label(FILE* file,gchar* label,gchar* buffer, gint len)
{
	while(!feof(file))
	{
		if(!fgets(buffer,len,file))
			return FALSE;
		if(strstr(buffer,label))
				return TRUE;
	}
	return FALSE;
}
/********************************************************************************/
static void get_grid_from_molcasgrid_file(FILE* file,gint numOfGrid)
{
	gint i;
	gint j;
	gint k;
	gdouble x;
	gdouble y;
	gdouble z;
	gdouble v;
    	gboolean beg = TRUE;
	gdouble scal;
	gdouble* V;
	gint n;
	gint nMaxBlock = 0;
	gint len = BSIZE;
	gchar buffer[BSIZE];

	progress_orb(0,GABEDIT_PROGORB_READGRID,TRUE);
	scal = (gdouble)101/grid->N[0];

 
	nMaxBlock = N[0]*N[1]*N[2];
	V = g_malloc((nMaxBlock+6)*sizeof(gdouble));
	if(!set_position_label(file,"Title", buffer, len))
	{
		Message(_("Sorry, I can not read grid from this file"),_("Error"),TRUE);
		grid = free_grid(grid);
		return;
	}
	if(!get_values_from_molcasgrid_file(file,V, nMaxBlock, numOfGrid))
	{
		Message(_("Sorry, I can not read grid from this file"),_("Error"),TRUE);
		grid = free_grid(grid);
		return;
	}
	n = -1;
	/* Grid Molcas 7 */
	if(fabs(version)>1e-8)
	for(i=0;i<grid->N[0];i++)
	{
	for(j=0;j<grid->N[1];j++)
	{
	for(k=0;k<grid->N[2];k++)
	{
				x = XYZ0[0] + i*X[0] + j*X[1] +  k*X[2]; 
				y = XYZ0[1] + i*Y[0] + j*Y[1] +  k*Y[2]; 
				z = XYZ0[2] + i*Z[0] + j*Z[1] +  k*Z[2]; 
		
				n++;
				v = V[n];
				grid->point[i][j][k].C[0] = x;
				grid->point[i][j][k].C[1] = y;
				grid->point[i][j][k].C[2] = z;
				grid->point[i][j][k].C[3] = v;
				if(beg)
				{
					beg = FALSE;
        				grid->limits.MinMax[0][3] =  v;
        				grid->limits.MinMax[1][3] =  v;
				}
                		else
				{
        				if(grid->limits.MinMax[0][3]>v)
        					grid->limits.MinMax[0][3] =  v;
        				if(grid->limits.MinMax[1][3]<v)
        					grid->limits.MinMax[1][3] =  v;
				}
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_READGRID,TRUE);
			break;
		}

		progress_orb(scal,GABEDIT_PROGORB_READGRID,FALSE);
	}
	/* M2Msi Molcas 6 */
	else
	for(k=0;k<grid->N[2];k++)
	{
	for(j=0;j<grid->N[1];j++)
	{
	for(i=0;i<grid->N[0];i++)
	{
				x = XYZ0[0] + i*X[0] + j*X[1] +  k*X[2]; 
				y = XYZ0[1] + i*Y[0] + j*Y[1] +  k*Y[2]; 
				z = XYZ0[2] + i*Z[0] + j*Z[1] +  k*Z[2]; 
		
				n++;
				v = V[n];
				grid->point[i][j][k].C[0] = x;
				grid->point[i][j][k].C[1] = y;
				grid->point[i][j][k].C[2] = z;
				grid->point[i][j][k].C[3] = v;
				if(beg)
				{
					beg = FALSE;
        				grid->limits.MinMax[0][3] =  v;
        				grid->limits.MinMax[1][3] =  v;
				}
                		else
				{
        				if(grid->limits.MinMax[0][3]>v)
        					grid->limits.MinMax[0][3] =  v;
        				if(grid->limits.MinMax[1][3]<v)
        					grid->limits.MinMax[1][3] =  v;
				}
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_READGRID,TRUE);
			break;
		}

		progress_orb(scal,GABEDIT_PROGORB_READGRID,FALSE);
	}

	if(CancelCalcul)
	{
		grid = free_grid(grid);
	}
	g_free(V);
	progress_orb(0,GABEDIT_PROGORB_READGRID,TRUE);
	return;
}
/********************************************************************************/
static void read_density(gint numOfGrid)
{
	FILE* file = FOpen(molcasgridFileName, "rb");
	gchar buffer[BSIZE];
	if(!file)
	{
		sprintf(buffer,_("Sorry, i can not open \"%s\" file"),molcasgridFileName);
		grid = free_grid(grid);
		Message(buffer,_("Error"),TRUE);
		return;
	}

	get_grid_from_molcasgrid_file(file,numOfGrid);
	
	if(grid)
	{
        	limits = grid->limits;
		create_iso_orbitals();
		set_status_label_info(_("Grid"),_("Ok"));
	}
	else
	{
		set_status_label_info(_("Grid"),_("Nothing"));
		CancelCalcul = FALSE;
	}

	fclose(file);
}
/********************************************************************************/
static gint get_num_density(FILE* file, gchar* buffer, gint len)
{
	gint numOfGrid = 0;
	while(!feof(file))
	{
		if(!fgets(buffer,len,file))
			return 0;
		if(strstr(buffer,"GridName="))
		{
			numOfGrid++;
			if(strstr(buffer,"Density"))
				return numOfGrid;
		}
	}
	return 0;
}
/********************************************************************************/
static void read_orbital(GtkWidget *Win,gpointer user_data)
{
	DataRow* data = NULL;
	GtkWidget* gtklist = GTK_WIDGET(user_data);
	gchar buffer[BSIZE];
	FILE* file = FOpen(molcasgridFileName, "rb");
	GtkTreeIter node;
	GtkTreeModel *model = NULL;
	gchar pathString[100];

	if(!file)
	{
		sprintf(buffer,_("Sorry, I can not open \"%s\" file"),molcasgridFileName);
		grid = free_grid(grid);
		Message(buffer,_("Error"),TRUE);
		return;
	}

	if(selectedRow<0) return;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(gtklist));
	sprintf(pathString,"%d",selectedRow);
	if(!gtk_tree_model_get_iter_from_string (model, &node, pathString)) return;
	gtk_tree_model_get (model, &node, 4, &data, -1);

	if(!data) return;

	get_grid_from_molcasgrid_file(file,data->numGrid);
	
	if(grid)
	{
        	limits = grid->limits;
		create_iso_orbitals();
		set_status_label_info(_("Grid"),_("Ok"));
	}
	else
	{
		set_status_label_info(_("Grid"),_("Nothing"));
		CancelCalcul = FALSE;
	}

	fclose(file);
}
/*************************************************************************************************/
static void eventDispatcher(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{

	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (!event) return;
        if (((GdkEventButton *) event)->button != 1) return; 

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
 			selectedRow = atoi(gtk_tree_path_to_string(path));
			gtk_tree_model_get_iter (model, &iter, path);
			gtk_tree_path_free(path);
 			if ((GdkEventButton *) event && ((GdkEventButton *) event)->type==GDK_2BUTTON_PRESS)
			{
				GtkWidget* button = g_object_get_data(G_OBJECT (widget), "ButtonOk");
				gtk_button_clicked (GTK_BUTTON (button));
			}
		}
		else selectedRow = -1;
	}
	GTK_WIDGET_GET_CLASS(widget)->button_press_event(widget, event);
}
/********************************************************************************/
static GtkWidget* create_gtk_list_orbitals()
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
	GType* types = NULL;

	types = g_malloc((NlistTitle+1)*sizeof(GType));
	for (i=0;i<NlistTitle;i++) types[i] = G_TYPE_STRING;
	types[NlistTitle] = G_TYPE_POINTER;
  	store = gtk_list_store_newv (NlistTitle+1, types);
	g_free(types);
	model = GTK_TREE_MODEL (store);

	Width = g_malloc(NlistTitle*sizeof(gint));

	for (j=0;j<NlistTitle;j++) Width[j] = strlen(Titles[j]);

	Width[0] = (gint)(Width[0]*20);
	Width[1] = (gint)(Width[1]*20);
	Width[2] = (gint)(Width[2]*20);
	Width[3] = (gint)(Width[3]*20);

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

	for(i=0;i<numberOfOrbitals;i++)
	{
		DataRow* data = g_malloc(sizeof(DataRow));
		List[0] = g_strdup_printf("%i",i+1);
		List[1] = g_strdup_printf("%lf",energies[i]);
		List[2] = g_strdup_printf("%lf",occupations[i]);
		List[3] = g_strdup(labelsSymmetry[i]);
		data->numGrid = numGridOfOrbitals[i];
		data->totalNumberOfGrids = totalNumberOfGrids;
		gtk_list_store_append(store, &iter);
		for(j=0;j<4;j++) gtk_list_store_set (store, &iter, j, List[j], -1);
		gtk_list_store_set (store, &iter, 4, data, -1);
		for(j=0;j<4;j++) g_free(List[j]);
	}

	g_signal_connect(gtklist, "button_press_event", G_CALLBACK(eventDispatcher), NULL);
	return gtklist;
}
/********************************************************************************/
static void create_list_molcasgrid_orbitals()
{
  GtkWidget *Win;
  GtkWidget *frame;
  GtkWidget *hbox;
  GtkWidget *vboxall;
  GtkWidget *vboxframe;
  GtkWidget *button;
  GtkWidget *gtklist;
  GtkWidget *scr;

  if(numberOfOrbitals<1)
  {
	  Message(_("Sorry, Please load a file beforee\n"),_("Error"),TRUE);
	  return;
  }
  selectedRow=0;
  /* printf("Norb = %d\n",numberOfOrbitals);*/

  /* Principal Window */
  Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(Win),"Orbitals");
  gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
  gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
  gtk_widget_set_size_request (GTK_WIDGET(Win), 3*ScreenWidthD/10,(gint)(ScreenHeightD*0.69));
  gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
  gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

  add_glarea_child(Win,"Orbitals List ");

  vboxall = create_vbox(Win);

  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_add(GTK_CONTAINER(vboxall),frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

  gtklist = create_gtk_list_orbitals();
  scr=gtk_scrolled_window_new(NULL,NULL);
  gtk_box_pack_start(GTK_BOX (vboxframe), scr,TRUE, TRUE, 2);
  gtk_container_add(GTK_CONTAINER(scr),gtklist);
  set_base_style(gtklist,50000,50000,50000);

  gtk_widget_show_all (vboxall);
  /* buttons box */
  hbox = create_hbox_false(vboxall);
  gtk_widget_realize(Win);

  button = create_button(Win,"Cancel");
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, GTK_OBJECT(Win));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(Win));
  gtk_widget_show (button);

  button = create_button(Win,"OK");
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);
  g_signal_connect(G_OBJECT(button), "clicked",(GCallback)read_orbital,gtklist);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, GTK_OBJECT(Win));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(Win));
  g_object_set_data(G_OBJECT (gtklist), "ButtonOk",button);



  gtk_widget_show_now(Win);

  /* fit_windows_position(PrincipalWindow, Win);*/

  gtk_widget_show_all (Win);
}
/******************************************************************************/
static gint read_one_integer(FILE* file, gchar* label, gchar* buffer, gint len)
{
	while(!feof(file))
	{
		if(!fgets(buffer,len,file))
			return -1;
		if(strstr(buffer,label))
		{
			gchar* pos = strstr(buffer,"=")+1;
			gint n = -1;
			if( pos && 1 == sscanf(pos,"%d",&n))
				return n;
			else
				return -1;
		}
	}
	return 0;
}
/**************************************************************/
static gboolean read_info_orbitals(FILE* file)
{
	gint len = BSIZE;
	gchar buffer[BSIZE];
	gint i;
	gint iGrid = 0;

	fseek(file, 0L, SEEK_SET);
	numberOfOrbitals = read_one_integer(file,"N_of_MO=",buffer,len);
	if(numberOfOrbitals<1) return FALSE;

	occupations = g_malloc(numberOfOrbitals*sizeof(gdouble));
	energies = g_malloc(numberOfOrbitals*sizeof(gdouble));
	numGridOfOrbitals =  g_malloc(numberOfOrbitals*sizeof(gint));
	labelsSymmetry = g_malloc(numberOfOrbitals*sizeof(gchar*));

	fseek(file, 0L, SEEK_SET);
	totalNumberOfGrids = read_one_integer(file,"N_of_Grids=",buffer,len);
	if(totalNumberOfGrids<1) return FALSE;

	for(i=0;i<numberOfOrbitals;i++)
	{
		occupations[i] = 0;
		energies[i] = 0;
		numGridOfOrbitals[i] = 0;
		labelsSymmetry[i] = g_strdup("DELETED");
	}
	fseek(file, 0L, SEEK_SET);
	totalNumberOfGrids = read_one_integer(file,"N_of_Grids=",buffer,len);
	if(totalNumberOfGrids<1) return FALSE;
	iGrid = 0;
	i = -1;
	while(!feof(file))
	{
		if(!fgets(buffer,len,file))
		{
			free_molcasgrid_orb();
			return FALSE;
		}

		if(strstr(buffer,"GridName="))
		{
			gchar* pos = strstr(buffer,"=")+1;
			gdouble nt, et;
			gchar st[BSIZE];
			iGrid++;
			if(iGrid>totalNumberOfGrids)break;
			if( pos && 3 == sscanf(pos,"%s %lf %lf",st, &nt, &et))
			{
				i++;
				labelsSymmetry[i] = g_strdup(st);
				energies[i] = et;
				numGridOfOrbitals[i] = iGrid;
				pos = strstr(buffer,"(")+1;
				if(pos) sscanf(pos,"%lf",&occupations[i]);
				/* this is a orbitals */
			}
			if(iGrid>=totalNumberOfGrids)break;
		}
	}
	{
		for(i=0;i<numberOfOrbitals;i++)
		{
			if(!strcmp(labelsSymmetry[i],"DELETED")) 
			{
				g_free(labelsSymmetry[i]); 
				labelsSymmetry[i] = NULL;
			}
		}
		for(i=0;i<numberOfOrbitals;i++)
		{
			if(labelsSymmetry[i]==NULL) break;
		}
		if(i==0)
		{
			printf("Number of orbitals = 0\n");
			return FALSE;
		}
		if(i!=numberOfOrbitals)
		{
			numberOfOrbitals = i;
			occupations = g_realloc(occupations, numberOfOrbitals*sizeof(gdouble));
			energies = g_realloc(energies, numberOfOrbitals*sizeof(gdouble));
			numGridOfOrbitals =  g_realloc(numGridOfOrbitals,numberOfOrbitals*sizeof(gint));
			labelsSymmetry = g_realloc(labelsSymmetry,numberOfOrbitals*sizeof(gchar*));
		}
	}

	return TRUE;
}
/************************************************************************************************************/
static gboolean read_molcasgrid_grid_limits(FILE* file, gint N[],gdouble XYZ0[], gdouble X[], gdouble Y[],gdouble Z[], gdouble*version)
{
	gboolean Ok = TRUE;
	gint len = BSIZE;
	gchar buffer[BSIZE];
	gint i;
	gint nAll = 0;
	*version = 0;

	while(!feof(file) && Ok)
	{
		if(!fgets(buffer,len,file))
		{
			Ok = FALSE;
			break;
		}
		if (strstr(buffer,"VERSION="))   
		{
			gchar* pos = strstr(buffer,"=")+1;
			if(pos) *version = atof(pos);
		}
		if (strstr(buffer,"Net="))   
		{
			gchar* pos = strstr(buffer,"=")+1;
			if(pos)
			{
				gint n = sscanf(pos,"%d %d %d",&N[0],&N[1],&N[2]);
				if(n !=3  || N[0]<1 || N[2]<1 || N[2]<1)
				{
				       	Ok = FALSE;
					break;
				}
				N[0]++;
				N[1]++;
				N[2]++;
				nAll++;
			}
			else
			{
				Ok = FALSE;
				break;
			}
		}
		if (strstr(buffer,"Origin="))   
		{
			gchar* pos = strstr(buffer,"=")+1;
			if(pos)
			{
				sscanf(pos,"%lf %lf %lf",&XYZ0[0],&XYZ0[1],&XYZ0[2]);
				nAll++;
			}
			else
			{
				Ok = FALSE;
				break;
			}
		}
		if (strstr(buffer,"Axis_1="))   
		{
			gchar* pos = strstr(buffer,"=")+1;
			if(pos)
			{
				gint n = sscanf(pos,"%lf %lf %lf",&X[0],&X[1],&X[2]);
				if(n !=3  || N[0]<2 || N[2]<2 || N[2]<2)
				{
				       	Ok = FALSE;
					break;
				}
				else
				{
					nAll++;
					for(i=0;i<3;i++)
						X[i] /= N[i]-1;
				}
			}
			else
			{
				Ok = FALSE;
				break;
			}
		}
		if (strstr(buffer,"Axis_2="))   
		{
			gchar* pos = strstr(buffer,"=")+1;
			if(pos)
			{
				gint n = sscanf(pos,"%lf %lf %lf",&Y[0],&Y[1],&Y[2]);
				if(n !=3  || N[0]<2 || N[2]<2 || N[2]<2)
				{
				       	Ok = FALSE;
					break;
				}
				else
				{
					nAll++;
					for(i=0;i<3;i++)
						Y[i] /= N[i]-1;
				}
			}
			else
			{
				Ok = FALSE;
				break;
			}
		}
		if (strstr(buffer,"Axis_3="))   
		{
			gchar* pos = strstr(buffer,"=")+1;
			if(pos)
			{
				gint n = sscanf(pos,"%lf %lf %lf",&Z[0],&Z[1],&Z[2]);
				if(n !=3  || N[0]<2 || N[2]<2 || N[2]<2)
				{
				       	Ok = FALSE;
					break;
				}
				else
				{
					nAll++;
					for(i=0;i<3;i++)
						Z[i] /= N[i]-1;
				}
			}
			else
			{
				Ok = FALSE;
				break;
			}
		}
		if(nAll==5) break;
		if(strstr(buffer,"Title")) break;

	}
	if(Ok)
	{
		GridLimits limits;
		gint i;

  		for(i=0;i<3;i++)
   			limits.MinMax[0][i] = XYZ0[i];

		limits.MinMax[1][0] = XYZ0[0] + (N[0]-1)*X[0] + (N[1]-1)*X[1] +  (N[2]-1)*X[2];
		limits.MinMax[1][1] = XYZ0[1] + (N[0]-1)*Y[0] + (N[1]-1)*Y[1] +  (N[2]-1)*Y[2];
		limits.MinMax[1][2] = XYZ0[2] + (N[0]-1)*Z[0] + (N[1]-1)*Z[1] +  (N[2]-1)*Z[2];

		grid = grid_point_alloc(N,limits);

	}
	return Ok;
}
/**************************************************************/
static gboolean read_molcasgrid_geometry(FILE* file)
{
	gboolean Ok = TRUE;
	gint len = BSIZE;
	gchar buffer[BSIZE];
	gchar buffer1[BSIZE];
	gchar* pos;
	gint i;
	gint j;

	Ok = set_position_label(file,"Natom=",buffer, len);
	nCenters = 0;
	if(Ok)
	{
		pos = strstr(buffer,"=")+1;
		nCenters = atoi(pos);
		if(nCenters<1)Ok = FALSE;
	}
	if(Ok)
	{
		GeomOrb=g_malloc(nCenters*sizeof(TypeGeomOrb));
		for( i=0; i<nCenters; i++)
		{
			if(!fgets(buffer,len,file))
				return FALSE;
			if(4 != sscanf(buffer,"%s %lf %lf %lf",buffer1, &GeomOrb[i].C[0], &GeomOrb[i].C[1], &GeomOrb[i].C[2]))
			{
				g_free(GeomOrb);
				return FALSE;
			}
			for(j=0; j<(gint)strlen(buffer1);j++)
			{
				if(testi(buffer1[j])) buffer1[j] =' ';
			}
			delete_all_spaces(buffer1);
			if(strlen(buffer1)>1) buffer1[1] = tolower(buffer1[1]);
			if(strlen(buffer1)>2) buffer1[2] = tolower(buffer1[2]);
			GeomOrb[i].Symb = g_strdup(buffer1);
			GeomOrb[i].Prop = prop_atom_get(GeomOrb[i].Symb);
			GeomOrb[i].partialCharge = 0.0;
			GeomOrb[i].variable = TRUE;
			GeomOrb[i].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[i].Symb);
		}
	}
	return Ok;
}
/**************************************************************/
static void read_molcasgrid_orbitals_file(gchar* filename)
{
	FILE* file = FOpen(filename, "rb");
	gchar* tmp;
	gchar buffer[BSIZE];
	gboolean Ok = TRUE;

	CancelCalcul = FALSE;
	if(!file)
	{
		sprintf(buffer,_("I can not open %s file"),filename);
		Message(buffer,_("Error"),TRUE);
		return ;
	}

	free_data_all();
	tmp = get_name_file(filename);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),_("Molcas grid formatted file"));
	nCenters = 0;
	
	Ok = read_molcasgrid_geometry(file);
	if(Ok)
	{
		init_atomic_orbitals();
		Ok =  read_info_orbitals(file);
		if(Ok)
		{
			fseek(file, 0L, SEEK_SET);
			Ok = read_molcasgrid_grid_limits(file, N, XYZ0, X, Y, Z,&version);
			if(!Ok)
			{
				free_data_all();
				Message(_("Sorry, I can not grid information from thi file\n"),_("Error"),TRUE);
			}
		}
		else
		{
			free_data_all();
			Message(_("Sorry, No grid available in this file\n"),_("Error"),TRUE);
			Ok = FALSE;
		}
	}

	buildBondsOrb();
	RebuildGeomD = TRUE;
	glarea_rafresh(GLArea);
	if(Ok) init_atomic_orbitals();

	if(nCenters>0) set_status_label_info(_("Geometry"),_("Ok"));
	/*printf("N = %d %d %d\n",N[0],N[1],N[2]);*/

	if(Ok) set_status_label_info(_("Grid"),_("Ok"));
	else set_status_label_info(_("Grid"),_("Nothing"));

	CancelCalcul = FALSE;

	fclose(file);
}
/**************************************************************/
static void read_molcasgrid_density_file(gchar* filename)
{
	FILE* file = FOpen(filename, "rb");
	gchar* tmp;
	gint len = BSIZE;
	gchar buffer[BSIZE];
	gboolean Ok = TRUE;
	gint nMO = 0;
	gint numGridDensity = 0;

	CancelCalcul = FALSE;
	if(!file)
	{
		sprintf(buffer,_("I can not open %s file"),filename);
		Message(buffer,_("Error"),TRUE);
		return ;
	}

	free_data_all();
	tmp = get_name_file(filename);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),_("Molcas grid formatted file"));
	nCenters = 0;
	/* read geometry */
	Ok = read_molcasgrid_geometry(file);
	if(Ok)
	{
		init_atomic_orbitals();
		fseek(file, 0L, SEEK_SET);
		nMO = read_one_integer(file,"N_of_MO=",buffer,len);
		fseek(file, 0L, SEEK_SET);
		totalNumberOfGrids = read_one_integer(file,"N_of_Grids=",buffer,len);
		if(totalNumberOfGrids>1)
		{
			fseek(file, 0L, SEEK_SET);
			Ok = read_molcasgrid_grid_limits(file, N, XYZ0, X, Y, Z,&version);
			if(!Ok)
			{
				free_data_all();
				Message(_("Sorry, I can not grid information from thi file\n"),_("Error"),TRUE);
			}
		}
		else
		{
			free_data_all();
			Message(_("Sorry, No grid available in this file\n"),_("Error"),TRUE);
			Ok = FALSE;
		}
	}
	if(Ok)
	{
		fseek(file, 0L, SEEK_SET);
		numGridDensity = get_num_density(file,buffer,len); 
		if(numGridDensity<1)
		{
			free_data_all();
			Message(_("Sorry, No density available in this file\n"),_("Error"),TRUE);
        		Ok = FALSE;
		}
		
	}

	buildBondsOrb();
	RebuildGeomD = TRUE;
	glarea_rafresh(GLArea);
	if(Ok) init_atomic_orbitals();

	if(nCenters>0) set_status_label_info(_("Geometry"),_("Ok"));
	/*printf("N = %d %d %d\n",N[0],N[1],N[2]);*/

	if(Ok) set_status_label_info(_("Grid"),_("Ok"));
	else set_status_label_info(_("Grid"),_("Nothing"));

	CancelCalcul = FALSE;

	fseek(file, 0L, SEEK_SET);
	fclose(file);

	if(Ok) read_density(numGridDensity);

}
/********************************************************************************/
void load_molcasgrid_file_orbitals(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	sprintf(molcasgridFileName,"%s",FileName);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	TypeGrid = GABEDIT_TYPEGRID_ORBITAL;
	read_molcasgrid_orbitals_file(FileName);
	create_list_molcasgrid_orbitals();
}
/********************************************************************************/
void load_molcasgrid_file_density(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	sprintf(molcasgridFileName,"%s",FileName);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	TypeGrid = GABEDIT_TYPEGRID_EDENSITY;
	read_molcasgrid_density_file(FileName);
}
/********************************************************************************/
