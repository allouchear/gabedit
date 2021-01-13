/* GridAdf.c */
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
#include "StatusOrb.h"
#include "UtilsOrb.h"
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
	gint symmetryNumber;
	gint orbitalNumber;
}DataRow;

static gint numberOfSymmetries = 0;
static gchar** labelsSymmetry = NULL;
static gint* numberOfOrbitalsBySymmetry = NULL;
static gdouble** occupationsBySymmetry = NULL;
static gdouble** energiesBySymmetry = NULL;
static gint selectedRow = -1;
static	gint N[3];
static	gdouble XYZ0[3]={0.0,0.0,0.0};
static	gdouble    X[3]={0.0,0.0,0.0};
static	gdouble    Y[3]={0.0,0.0,0.0};
static	gdouble    Z[3]={0.0,0.0,0.0};
static  gchar adfFileName[2048];

/**************************************************************/
static void free_adf_orb()
{
	gint i;
	if(numberOfSymmetries<1)
		return;
	if(labelsSymmetry)
	{
		for(i=0;i<numberOfSymmetries;i++)
		{
			if(labelsSymmetry[i])
				g_free(labelsSymmetry[i]);
		}
		g_free(labelsSymmetry);
	}
	labelsSymmetry = NULL;

	if(numberOfOrbitalsBySymmetry)
		g_free(numberOfOrbitalsBySymmetry);
	numberOfOrbitalsBySymmetry = NULL;

	if(occupationsBySymmetry)
	{
		for(i=0;i<numberOfSymmetries;i++)
		{
			if(occupationsBySymmetry[i])
				g_free(occupationsBySymmetry[i]);
		}
		g_free(occupationsBySymmetry);
	}
	occupationsBySymmetry  = NULL;

	if(energiesBySymmetry)
	{
		for(i=0;i<numberOfSymmetries;i++)
		{
			if(energiesBySymmetry[i])
				g_free(energiesBySymmetry[i]);
		}
		g_free(energiesBySymmetry);
	}
	energiesBySymmetry = NULL;

}
/********************************************************************************/
static gboolean get_values_from_adf_file(FILE* file,gdouble V[])
{
	gint len = BSIZE;
	gchar t[BSIZE];
	gint k = 0;

	while(!feof(file))
	{
		if(!fgets(t,len,file))  /* Nx, X0,X1,X2 */   
		{
			Message(_("I can not read adf file\n"),_("Error"),TRUE);
			g_free(V);
			return FALSE;
		}
		if(sscanf(t,"%lf %lf %lf", &V[k], &V[k+1], &V[k+2])!=3)
		{
	                break;
		}
		k += 3;
	}
	return TRUE;
}
/********************************************************************************/
static gboolean set_position(FILE* file,gchar* label,gint orbitalNumber)
{
	gint len = BSIZE;
	gchar buffer[BSIZE];
	while(!feof(file))
	{
		if(!fgets(buffer,len,file))
			return FALSE;
		if(strstr(buffer,label))
		{
			if(!fgets(buffer,len,file))
				return FALSE;
			if(atoi(buffer)==orbitalNumber+1)
			{
				if(!fgets(buffer,len,file))return FALSE; /* Number of points */
				return TRUE;
			}
		}
	}
	return FALSE;
}
/********************************************************************************/
static void get_grid_from_adf_file(FILE* file,gchar* label,gint orbitalNumber)
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

	progress_orb(0,GABEDIT_PROGORB_READGRID,TRUE);
	scal = (gdouble)1.01/grid->N[0];

	/*printf("N = %d %d %d\n",N[0],N[1],N[2]);*/
 
	V = g_malloc((N[0]*N[1]*N[2]+6)*sizeof(gdouble));
	if(!set_position(file,label,orbitalNumber))
	{
		Message(_("Sorry, I can not read this orbital"),_("Error"),TRUE);
		grid = free_grid(grid);
		return;
	}
	if(!get_values_from_adf_file(file,V))
		return;
	n = -1;
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
				/*printf("%lf %lf %lf %lf\n",x,y,z,v);*/
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
static void read_orbital(GtkWidget *Win,gpointer user_data)
{
	DataRow* data = NULL;
	GtkWidget* gtklist = GTK_WIDGET(user_data);
	gchar buffer[BSIZE];
	FILE* file = FOpen(adfFileName, "rb");
	GtkTreeIter node;
	GtkTreeModel *model = NULL;
	gchar pathString[100];

	if(!file)
	{
		sprintf(buffer,_("Sorry, i can not open \"%s\" file"),adfFileName);
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

	sprintf(buffer,"SCF_%s",labelsSymmetry[data->symmetryNumber]);
	get_grid_from_adf_file(file,buffer,data->orbitalNumber);
	
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
	gint k;
	gint l;
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


	i=-1;
	for(k=0;k<numberOfSymmetries;k++)
	for(l=0;l<numberOfOrbitalsBySymmetry[k];l++)
	{
		DataRow* data = g_malloc(sizeof(DataRow));
		i++;
		List[0] = g_strdup_printf("%i",i+1);
		List[1] = g_strdup_printf("%lf",energiesBySymmetry[k][l]);
		List[2] = g_strdup_printf("%lf",occupationsBySymmetry[k][l]);
		List[3] = g_strdup(labelsSymmetry[k]);
		data->symmetryNumber = k;
		data->orbitalNumber = l;
		gtk_list_store_append(store, &iter);
		for(j=0;j<4;j++) gtk_list_store_set (store, &iter, j, List[j], -1);
		gtk_list_store_set (store, &iter, 4, data, -1);
		for(j=0;j<4;j++) g_free(List[j]);
	}
	g_signal_connect(gtklist, "button_press_event", G_CALLBACK(eventDispatcher), NULL);

 return gtklist;
 
}
/********************************************************************************/
static void create_list_adf_orbitals()
{
  GtkWidget *Win;
  GtkWidget *frame;
  GtkWidget *hbox;
  GtkWidget *vboxall;
  GtkWidget *vboxframe;
  GtkWidget *button;
  GtkWidget *gtklist;
  GtkWidget *scr;

  if(numberOfSymmetries<1)
  {
	  Message(_("Sorry, Please load a file beforee\n"),_("Error"),TRUE);
	  return;
  }

  /* Principal Window */
  Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(Win),_("Orbitals"));
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
/**************************************************************/
static gdouble* read_one_table(FILE* file,gint* n)
{
	gint len = BSIZE;
	gchar buffer[BSIZE];
	gdouble* table = NULL;
	gdouble V[3];
	gint i,k,l,krest;

	*n = -1;

	if(!fgets(buffer,len,file))return NULL;

	if(1 != sscanf(buffer,"%d",n))
	{
		*n = -1;
		return NULL;
	}
	if(*n<1)
		return NULL;
	/*printf("*n = %d\n",*n);*/
	table  = g_malloc(*n* sizeof(gdouble));

	k = *n/3;
	krest = *n % 3;
	i=0;
	for(l=0;l<k;l++)
	{
		if(!fgets(buffer,len,file))return table;
		if(3 != sscanf(buffer,"%lf %lf %lf",&V[0],&V[1],&V[2]))
		{
			g_free(table);
			return NULL;
		}
		table[i] = V[0];
		table[i+1] = V[1];
		table[i+2] = V[2];
		i += 3;
	}
	if(krest>0)
	{
		if(!fgets(buffer,len,file))return table;
		l = sscanf(buffer,"%lf %lf %lf",&V[0],&V[1],&V[2]);
		if(l != krest)
		{
			*n = -1;
			g_free(table);
			return NULL;
		}
		for(l=0;l<krest;l++)
			table[i+l] = V[l];
	}

	/*printf("Table = \n ");*/
	/*
	for(l=0;l<*n;l++)
		printf("%lf  ",table[l]);
	printf("\n");
	*/

	return table;
}
/**************************************************************/
static gboolean read_info_orbitals(FILE* file)
{
	gint len = BSIZE;
	gchar buffer[BSIZE];
	gchar buffer1[BSIZE];
	gint i;
	gboolean Ok = TRUE;
	gint nOcc, nEner;

	numberOfOrbitalsBySymmetry = g_malloc(numberOfSymmetries*sizeof(gint));
	occupationsBySymmetry = g_malloc(numberOfSymmetries*sizeof(gdouble*));
	energiesBySymmetry = g_malloc(numberOfSymmetries*sizeof(gdouble*));
	for(i=0;i<numberOfSymmetries;i++)
	{
		numberOfOrbitalsBySymmetry[i] = 0;
		occupationsBySymmetry[i] = NULL;
		energiesBySymmetry[i] = NULL;
	}

	for(i=0;i<numberOfSymmetries;i++)
	{
		sprintf(buffer1,"SCF_%s",labelsSymmetry[i]);
		nOcc = 0;
		nEner = 0;
		while(!feof(file) && Ok)
		{
			if(!fgets(buffer,len,file))return FALSE;
			if(strstr(buffer,buffer1))
			{
				if(!fgets(buffer,len,file))return FALSE;
				if(strstr(buffer,"Occupations"))
				{
					occupationsBySymmetry[i] = read_one_table(file,&nOcc);
					if(nOcc<1)
					{
						occupationsBySymmetry[i]=NULL;
						numberOfOrbitalsBySymmetry[i] = 0;
						break;
					}
				
				}
				if(strstr(buffer,"Eigenvalues"))
				{
					energiesBySymmetry[i] = read_one_table(file,&nEner);
					/*
					if(nEner<1)
						return FALSE;
						*/
				}
				
			}
			if(nOcc != 0 && nEner !=0)
			{
				if(nOcc != nEner)
					return FALSE;
				numberOfOrbitalsBySymmetry[i] = nOcc;
				break; /* New Symmetry */
			}
		}
	}
	return TRUE;
}
/**************************************************************/
static gboolean read_labels_symmetry(FILE* file)
{
	gint len = BSIZE;
	gchar buffer[BSIZE];
	gchar buffer1[BSIZE];
	gint i;

	if(!fgets(buffer,len,file))return FALSE;
	if(numberOfSymmetries<1)
		return FALSE;
	labelsSymmetry = g_malloc(numberOfSymmetries*sizeof(gchar*));
	for(i=0;i<numberOfSymmetries;i++)
		labelsSymmetry[i] = NULL;

	
	for(i=0;i<numberOfSymmetries;i++)
	{
		if(!fgets(buffer,len,file))return FALSE;
		if(1 != sscanf(buffer,"%s",buffer1))
			return FALSE;
		labelsSymmetry[i] = g_strdup(buffer1);
		if(!fgets(buffer,len,file))return FALSE;
		/*printf("%s\n",labelsSymmetry[i]);*/
	}
	return TRUE;
}
/**************************************************************/
static gboolean read_vector(FILE* file,gdouble V[])
{
	gint len = BSIZE;
	gchar buffer[BSIZE];
	if(!fgets(buffer,len,file))return FALSE;
	if(!fgets(buffer,len,file))return FALSE;
	if(3 != sscanf(buffer,"%lf %lf %lf",&V[0],&V[1],&V[2]))
		return FALSE;
	return TRUE;
}
/**************************************************************/
static gint read_one_integer(FILE* file)
{
	gint len = BSIZE;
	gchar buffer[BSIZE];
	gint n;
	if(!fgets(buffer,len,file))return 0;
	if(!fgets(buffer,len,file))return 0;
	if(1 != sscanf(buffer,"%d",&n))
			return 0;
	return n;
}
/**************************************************************/
static gboolean read_atoms_number(FILE* file)
{
	gint len = BSIZE;
	gchar buffer[BSIZE];
	if(!fgets(buffer,len,file))return FALSE;
	if(!fgets(buffer,len,file))return FALSE;
	nCenters = atoi(buffer);
	/*printf("nCenters=%d\n",nCenters);*/
	if(nCenters>0)
	{
		GeomOrb=g_malloc(nCenters*sizeof(TypeGeomOrb));
		return TRUE;
	}
	return FALSE;
}
/**************************************************************/
static gboolean read_atoms_labels(FILE* file)
{
	gint len = BSIZE;
	gchar buffer[BSIZE];
	gchar buffer1[BSIZE];
	gint i;

	/*printf("je suis dans read atoms labels\n");*/
	if(!fgets(buffer,len,file))return FALSE;

	if(nCenters<1)
		return FALSE;

	for(i=0;i<nCenters;i++)
	{

		if(!fgets(buffer,len,file))return FALSE;
		if(1 != sscanf(buffer,"%s",buffer1))
			return FALSE;
		if(!fgets(buffer,len,file))return FALSE; /* dum line*/

		GeomOrb[i].Symb = g_strdup(buffer1);
		/*printf("%s\n",GeomOrb[i].Symb);*/
		GeomOrb[i].Prop = prop_atom_get(GeomOrb[i].Symb);
		GeomOrb[i].partialCharge = 0.0;
		GeomOrb[i].variable = TRUE;
		GeomOrb[i].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[i].Symb);
	}
	return TRUE;
}
/**************************************************************/
static gboolean read_atoms_coordinates(FILE* file)
{
	gint len = BSIZE;
	gchar buffer[BSIZE];
	gint i;

	/*printf("je suis dans read atoms\n");*/
	if(!fgets(buffer,len,file))return FALSE;

	if(nCenters<1)
		return FALSE;

	for(i=0;i<nCenters;i++)
	{
		if(!fgets(buffer,len,file))return FALSE;
		if(3 != sscanf(buffer,"%lf %lf %lf",&GeomOrb[i].C[0],&GeomOrb[i].C[1],&GeomOrb[i].C[2]))
			return FALSE;
		/*printf("%lf %lf %lf\n",GeomOrb[i].C[0],GeomOrb[i].C[1],GeomOrb[i].C[2]);*/
	}
	return TRUE;
}
/**************************************************************/
static gboolean read_adf_grid_limits(FILE* file, gint N[],gdouble XYZ0[], gdouble X[], gdouble Y[],gdouble Z[])
{
	gboolean Ok = TRUE;
	gint len = BSIZE;
	gchar buffer[BSIZE];

	while(!feof(file) && Ok)
	{
		if(!fgets(buffer,len,file))
		{
			Ok = FALSE;
			break;
		}
		if (strstr(buffer,"nr of points x"))   
		{
			N[0] = read_one_integer(file);
			if(N[0]<1)
			{
				Ok = FALSE;
				break;
			}
		}
		if (strstr(buffer,"nr of points y"))   
		{
			N[1] =  read_one_integer(file);
			if(N[1]<1)
			{
				Ok = FALSE;
				break;
			}
		}
		if (strstr(buffer,"nr of points z"))
		{
			N[2] =  read_one_integer(file);
			if(N[2]<1)
			{
				Ok = FALSE;
				break;
			}
		}
		if (strstr(buffer,"Start_point"))
		{
			if(!read_vector(file,XYZ0))
			{
				Ok = FALSE;
				break;
			}
			/* printf("XYZ0 = %lf %lf %lf\n",XYZ0[0],XYZ0[1],XYZ0[2]);*/
		}
		if (strstr(buffer,"x-vector"))
		{
			if(!read_vector(file,X))
			{
				Ok = FALSE;
				break;
			}
			/*printf("X = %lf %lf %lf\n",X[0],X[1],X[2]);*/
		}
		if (strstr(buffer,"y-vector"))
		{
			if(!read_vector(file,Y))
			{
				Ok = FALSE;
				break;
			}
			/*printf("Y = %lf %lf %lf\n",Y[0],Y[1],Y[2]);*/
		}
		if (strstr(buffer,"z-vector"))
		{
			if(!read_vector(file,Z))
			{
				Ok = FALSE;
				break;
			}
			/*printf("Z = %lf %lf %lf\n",Z[0],Z[1],Z[2]);*/
		}
		if (strstr(buffer,"nr of symmetries"))
		{
			free_adf_orb();
			numberOfSymmetries = read_one_integer(file);
			if(numberOfSymmetries<0)
			{
				Ok = FALSE;
				break;
			}
			/*printf("numberOfSymmetries %d\n",numberOfSymmetries);*/
		}
		if (strstr(buffer,"labels"))
		{
			Ok = read_labels_symmetry(file);
			if(!Ok)
				break;
			Ok = read_info_orbitals(file);
		}
		if(!fgets(buffer,len,file))
		{
			Ok = FALSE;
			break;
		}
		if(!strstr(buffer,"Grid"))
			break;

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
static gboolean read_adf_geometry(FILE* file)
{
	gboolean Ok = TRUE;
	gint len = BSIZE;
	gchar buffer[BSIZE];

	while(!feof(file) && Ok)
	{
		if(!fgets(buffer,len,file))
		{
			Ok = FALSE;
			break;
		}
		if (strstr(buffer,"nnuc"))   
			if(!read_atoms_number(file))
			{
				Ok = FALSE;
				break;
			}
		/*if (strstr(buffer,"qtch"))   read_atoms_charges(file);*/
		if (strstr(buffer,"labels"))
			if(!read_atoms_labels(file))
			{
				Ok = FALSE;
				break;
			}
		if (strstr(buffer,"xyznuc"))
			if(!read_atoms_coordinates(file))
			{
				Ok = FALSE;
				break;
			}
	}
	return Ok;
}
/**************************************************************/
static void read_adf_file(gchar* filename)
{
	FILE* file = FOpen(filename, "rb");
	gchar* tmp;
	gint len = BSIZE;
	gchar buffer[BSIZE];
	gboolean Ok = TRUE;
	/*
	gint N[3];
	gdouble XYZ0[3];
	gdouble X[3];
	gdouble Y[3];
	gdouble Z[3];
	*/

	CancelCalcul = FALSE;
	if(!file)
	{
		Message(_("I can not open file\n"),_("Error"),TRUE);
		return ;
	}

	free_data_all();
	tmp = get_name_file(filename);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),_("ADF Formatted file(41)"));
	nCenters = 0;
	while(!feof(file) && Ok)
	{
		if(!fgets(buffer,len,file))
		{
			Ok = FALSE;
			break;
		}
		if(strstr(buffer,"Geometry"))
		{
			/*printf("Begin Geometry read\n");*/
			Ok = read_adf_geometry(file);
		}
		if(strstr(buffer,"Grid"))
		{
			Ok = read_adf_grid_limits(file,N,XYZ0,X,Y,Z);
			/*printf("Ok = %d\n",Ok);*/
		}

	}

	if(nCenters>0)
	{
		/*printf("C'est OK\n");*/
		buildBondsOrb();
		RebuildGeomD = TRUE;
		glarea_rafresh(GLArea);
		init_atomic_orbitals();
		set_status_label_info(_("Geometry"),_("Ok"));
	}
	/*printf("N = %d %d %d\n",N[0],N[1],N[2]);*/

	set_status_label_info(_("Grid"),_("Nothing"));
	CancelCalcul = FALSE;

	fclose(file);
}
/********************************************************************************/
void load_adf_file_orbitals(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	sprintf(adfFileName,"%s",FileName);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	TypeGrid = GABEDIT_TYPEGRID_ORBITAL;
	read_adf_file(FileName);
	create_list_adf_orbitals();
}
/********************************************************************************/
