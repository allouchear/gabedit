/* GridQChem.c */
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
static void get_grid_from_qchem_file(gchar* fileName, gint numOfGrid)
{
	FILE* file = FOpen(fileName, "rb");
	gint i;
	gint j;
	gint k;
	gdouble x;
	gdouble y;
	gdouble z;
	gdouble v;
    	gboolean beg = TRUE;
	gdouble scal;
	gint n;
	gint len = BSIZE;
	gchar buffer[BSIZE];
	gboolean Ok = TRUE;

	CancelCalcul = FALSE;
	if(!file)
	{
		sprintf(buffer,_("I can not open %s file\n"),fileName);
		Message(buffer,_("Error"),TRUE);
		return;
	}

	progress_orb(0,GABEDIT_PROGORB_READGRID,TRUE);
	scal = (gdouble)101/grid->N[2];

 
	while(!feof(file))
	{
		if(!fgets(buffer,len,file)) break;
		if(strstr(buffer,"X") && strstr(buffer,"Y") && strstr(buffer,"Z"))
		{
			Ok = TRUE;
			break;
		}
	}
	if(!Ok) 
	{
		Message(_("Sorry, I can not read grid from this file"),_("Error"),TRUE);
		grid = free_grid(grid);
		return;
	}

	n = -1;
	for(k=0;k<grid->N[0];k++)
	{
	for(j=0;j<grid->N[1];j++)
	{
	for(i=0;i<grid->N[2];i++)
	{
				if(!fgets(buffer,len,file))
				{
					Message(_("Sorry, I can not read grid from this file"),_("Error"),TRUE);
					grid = free_grid(grid);
					return;
				}
				if( 3 != sscanf(buffer,"%lf %lf %lf",&x, &y, &z) ) 
				{
					Message(_("Sorry, I can not read grid from this file"),_("Error"),TRUE);
					grid = free_grid(grid);
					return;
				}
				v = atof(&buffer[13*3+(numOfGrid-1)*13]);
				/* printf("x = %lf y = %lf z = %lf v  %e\n",x,y,z,v);*/
		
				n++;
				grid->point[k][j][i].C[0] = x;
				grid->point[k][j][i].C[1] = y;
				grid->point[k][j][i].C[2] = z;
				grid->point[k][j][i].C[3] = v;
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
	progress_orb(0,GABEDIT_PROGORB_READGRID,TRUE);
	return;
}
/********************************************************************************/
static gchar** get_list_of_grids(gchar* fileName, gint *nE)
{
	FILE* file = FOpen(fileName, "rb");
	gboolean Ok = TRUE;
	gint len = BSIZE;
	gchar buffer[BSIZE];
	gchar** gridsList = NULL;
	gint n;
	gint i;
	gint j;

	*nE = 0;
	CancelCalcul = FALSE;
	if(!file)
	{
		sprintf(buffer,_("I can not open %s file\n, I read list of grids from this file"),fileName);
		Message(buffer,_("Error"),TRUE);
		return NULL;
	}
	while(!feof(file))
	{
		if(!fgets(buffer,len,file)) return NULL;
		if(strstr(buffer,"X") && strstr(buffer,"Y") && strstr(buffer,"Z"))
		{
			Ok = TRUE;
			break;
		}
	}
	if(!Ok) return NULL;
	delete_last_spaces(buffer);
	n = (strlen(buffer)+1)/13-3;
	if(n<1) return NULL;

	gridsList = g_malloc((n+1)*sizeof(gchar*));
	for(i=0;i<n;i++)
	{
		gridsList[i] = g_malloc((13+1)*sizeof(gchar));
		for(j=0;j<13;j++)
			gridsList[i][j] = buffer[13*3+i*13+j];
		gridsList[i][13] = '\0';
	}

	gridsList[n] = NULL;


	*nE = n;
	return gridsList;

}
/********************************************************************************/
static gboolean read_qchem_grid_limits(FILE* file)
{
	gboolean Ok = TRUE;
	gint len = BSIZE;
	gchar buffer[BSIZE];
	gint i;
	GridLimits limits;
	gint N[3];
	gdouble min[3]; 
	gdouble max[3];

	Ok = set_position_label(file,"$plots",buffer, len);
	if(!Ok) Ok = set_position_label(file,"$PLOTS",buffer, len);
	if(!Ok) return FALSE;
	/* comments */
	if(!fgets(buffer,len,file)) return FALSE;
	/* x limits */
	if(!fgets(buffer,len,file)) return FALSE;
	if( 3!= sscanf(buffer,"%d %lf %lf",&N[0],&min[0],&max[0]) ) return FALSE;
	/* y limits */
	if(!fgets(buffer,len,file)) return FALSE;
	if( 3!= sscanf(buffer,"%d %lf %lf",&N[1],&min[1],&max[1]) ) return FALSE;
	/* z limits */
	if(!fgets(buffer,len,file)) return FALSE;
	if( 3!= sscanf(buffer,"%d %lf %lf",&N[2],&min[2],&max[2]) ) return FALSE;

	if(N[0]<1 || N[1]<1 || N[2]<1 ) return FALSE;
	

 	for(i=0;i<3;i++)
	{
   		limits.MinMax[0][i] = min[i];
   		limits.MinMax[1][i] = max[i];
	}

	grid = grid_point_alloc(N,limits);

	return TRUE;
}
/**************************************************************/
static gboolean read_qchem_geometry(FILE* file)
{
	gboolean Ok = TRUE;
	gint len = BSIZE;
	gchar buffer[BSIZE];
	gchar buffer1[BSIZE];
	gint k;
	gint j;
	gint ii;

	Ok = set_position_label(file,"Standard Nuclear Orientation",buffer, len);
	if(!Ok) return FALSE;
	nCenters = 0;
	if(Ok)
	{
		if(!fgets(buffer,len,file)) return FALSE; /* I     Atom         X            Y            Z */
		if(!fgets(buffer,len,file)) return FALSE; /* -------------------------------- */
	}
	else return FALSE;
	j = 0;
  	while(!feof(file) )
  	{
		if(!fgets(buffer,len,file)) break;
		if(strstr(buffer,"--------------------")) break;
    		j++;
    		if(GeomOrb==NULL) GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    		else GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));
		if(5 != sscanf(buffer,"%d %s %lf %lf %lf",&ii,buffer1, &GeomOrb[j-1].C[0], &GeomOrb[j-1].C[1], &GeomOrb[j-1].C[2]))
		{
			nCenters = j;
			g_free(GeomOrb);
			return FALSE;
		}
		GeomOrb[j-1].C[0] *= ANG_TO_BOHR;
		GeomOrb[j-1].C[1] *= ANG_TO_BOHR;
		GeomOrb[j-1].C[2] *= ANG_TO_BOHR;
		for(k=0; k<(gint)strlen(buffer1);k++)
		{
			if(testi(buffer1[k])) buffer1[k] =' ';
		}
		delete_all_spaces(buffer1);
		GeomOrb[j-1].Symb = g_strdup(buffer1);
		GeomOrb[j-1].Prop = prop_atom_get(GeomOrb[j-1].Symb);
		GeomOrb[j-1].partialCharge = 0.0;
		GeomOrb[j-1].variable = TRUE;
		GeomOrb[j-1].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j-1].Symb);
  	}
	nCenters = j;
	if(j<1) Ok = FALSE;
	return Ok;
}
/**************************************************************/
static gboolean read_qchemout_file(gchar* filename)
{
	FILE* file = FOpen(filename, "rb");
	gchar* tmp;
	gboolean Ok = TRUE;
	gchar buffer[BSIZE];

	CancelCalcul = FALSE;
	if(!file)
	{
		sprintf(buffer,_("I can not open %s file\n, I read geometry and grid limits from this file"),filename);
		Message(buffer,_("Error"),TRUE);
		return FALSE;
	}

	free_data_all();
	tmp = get_name_file(filename);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),_("QChem output file"));
	nCenters = 0;
	/* read geometry */
	Ok = read_qchem_geometry(file);
	if(!Ok)
	{
		sprintf(buffer,_("I can not read geometry from %s file"),filename);
		Message(buffer,_("Error"),TRUE);
		fclose(file);
		return FALSE;
	}
	fseek(file, 0L, SEEK_SET);
	if(Ok) Ok = read_qchem_grid_limits(file);
	if(!Ok)
	{
		sprintf(buffer,_("I can not read grid limits from %s file"),filename);
		Message(buffer,_("Error"),TRUE);
		fclose(file);
		return FALSE;
	}
	if(Ok) init_atomic_orbitals();

	buildBondsOrb();
	RebuildGeomD = TRUE;
	glarea_rafresh(GLArea);

	if(nCenters>0) set_status_label_info(_("Geometry"),_("Ok"));
	/*printf("N = %d %d %d\n",N[0],N[1],N[2]);*/

	CancelCalcul = FALSE;

	fclose(file);
	return TRUE;
}
/********************************************************************************/
GtkWidget *create_grids_list_frame( GtkWidget *vboxall,gchar** gridsList,gint n, gchar* fileName)
{
	GtkWidget *frame;
	GtkWidget *combo;
	GtkWidget *vboxframe;
	GtkWidget *entry;
	GtkWidget *table;

	frame = gtk_frame_new (_("Selection your grid"));
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	table = gtk_table_new(1,3,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);

	add_label_table(table,_(" Grid Name "),0,0);
	add_label_table(table,":",0,1);
	combo = create_combo_box_entry(gridsList,n,TRUE,-1,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,4,0,1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);
	gtk_widget_show (combo);
	entry = GTK_BIN(combo)->child;
	gtk_widget_set_sensitive(entry,FALSE); 
        g_object_set_data(G_OBJECT (entry), "FileName",fileName);
        g_object_set_data(G_OBJECT (entry), "Combo",combo);
	g_object_set_data(G_OBJECT (entry), "N", GINT_TO_POINTER(n));
	g_object_set_data(G_OBJECT (entry), "GridsList", gridsList);

	g_object_set_data(G_OBJECT (frame), "Entry", entry);

  	return entry;
}
/********************************************************************************/
static void apply_read_grid(GtkWidget *button,gpointer data)
{
	GtkWidget *entry = (GtkWidget *)data;
	GtkWidget *Win = g_object_get_data (G_OBJECT (button), "Window");
	gchar** gridsList = NULL;
	gint n = 0; 
	gchar* fileName = NULL;
	G_CONST_RETURN gchar* t;
	gint i;
	gint numOfGrid = 1;

        if(!this_is_an_object((GtkObject*)entry)) return;

	n = GPOINTER_TO_INT(g_object_get_data(G_OBJECT (entry), "N"));
	gridsList = g_object_get_data(G_OBJECT (entry), "GridsList");

	t = gtk_entry_get_text(GTK_ENTRY(entry));

	numOfGrid = 0;
	for(i=0;i<n;i++)
	{
		if(!strcmp(t,gridsList[i]))
		{
			numOfGrid = i+1;
			break;
		}
	}

	fileName = (gchar*)g_object_get_data(G_OBJECT (entry), "FileName");
	/* printf("numOfGrid=%d\n",numOfGrid);*/
	if( numOfGrid>=1)
	{
		get_grid_from_qchem_file(fileName, numOfGrid);
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
	}
  	delete_child(Win);
}
/********************************************************************************/
void create_window_list_grids(GtkWidget *w, gchar** gridsList,gint n, gchar* fileName)
{
	GtkWidget *fp;
	GtkWidget *sep;
	GtkWidget *vboxall;
	GtkWidget *vboxwin;
	GtkWidget *hbox;
	GtkWidget *button;
	GtkWidget *entry;

	/* Principal Window */
	fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(fp),_("Grids available"));
	gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (fp), TRUE);

	gtk_widget_realize(fp);

	add_glarea_child(fp," Grid List ");


	gtk_container_set_border_width (GTK_CONTAINER (fp), 5);
	vboxall = create_vbox(fp);
	vboxwin = vboxall;

	entry = create_grids_list_frame(vboxall, gridsList, n, fileName);

	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vboxwin), sep, FALSE, FALSE, 2);
	gtk_widget_show(sep);

	/* boutons box */
	hbox = create_hbox(vboxwin);
	gtk_widget_realize(fp);

	button = create_button(fp,"Cancel");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 5);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(fp));
	gtk_widget_show (button);

	button = create_button(fp,"OK");
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 5);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_object_set_data(G_OBJECT (button), "Window", fp);
	g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(apply_read_grid),(gpointer)entry);
  

	/* Show all */
	gtk_widget_show_all(fp);
}
/**************************************************************/
static gboolean read_qchemgrid_file(gchar* fileName)
{
	gint n;
	gchar** gridsList = get_list_of_grids(fileName, &n);
	/* printf("n=%d\n",n);*/
	if( n==1)
	{
		get_grid_from_qchem_file(fileName, 1);
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
	}
	else create_window_list_grids(NULL, gridsList, n, fileName);
	return TRUE;
}
/********************************************************************************/
void load_qchemgrid_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *fileName;
 	gchar *fileNameOut;
	gboolean rg = FALSE;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	fileNameOut = g_strdup_printf("%s.out",get_suffix_name_file(fileName));
	TypeGrid = GABEDIT_TYPEGRID_ORBITAL;
	rg = read_qchemout_file(fileNameOut);
	if(rg) read_qchemgrid_file(fileName);
}
/********************************************************************************/
