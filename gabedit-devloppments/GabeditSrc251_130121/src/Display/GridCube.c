/* GridCube.c */
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
#include "../Display/GlobalOrb.h"
#include "../Display/StatusOrb.h"
#include "../Display/UtilsOrb.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Constants.h"
#include "../Display/GLArea.h"
#include "../Display/AtomicOrbitals.h"
#include "../Display/Orbitals.h"
#include "../Display/ColorMap.h"
#include "../Display/GeomOrbXYZ.h"
#include "../Display/BondsOrb.h"

typedef enum
{
  GABEDIT_CUBE_GABEDIT,
  GABEDIT_CUBE_GAUSS_ORB,
  GABEDIT_CUBE_GAUSS_DEN,
  GABEDIT_CUBE_GAUSS_GRAD,
  GABEDIT_CUBE_GAUSS_LAP,
  GABEDIT_CUBE_GAUSS_NGRAD,
  GABEDIT_CUBE_GAUSS_POT,
  GABEDIT_CUBE_MOLPRO_ORB1,
  GABEDIT_CUBE_MOLPRO_ORBN,
  GABEDIT_CUBE_MOLPRO_DEN,
  GABEDIT_CUBE_MOLPRO_DEN_GRAD,
  GABEDIT_CUBE_MOLPRO_LAPDEN,
  GABEDIT_CUBE_MOLPRO_LAPLAP
} GabEditTypeCube;

typedef gboolean  (*FuncGetVal)(FILE*,gint,gint,gint,gdouble []);

Grid* get_grid_from_gauss_molpro_cube_file(gint typefile,FILE* file,gint num,gint n,gint N[],
		gdouble XYZ0[3],gdouble X[3],gdouble Y[3],gdouble Z[3]);

/**************************************************************************/
static void applyRestrictionCube()
{
	gint i, j, k;
	gboolean beg = TRUE;
	gdouble v;
	gdouble scal;
	gint N[3];
	Grid* newGrid;
	GridLimits newLimits;
	gint c;
	gint ii, jj, kk;

	if(grid->N[0]<3) return;
	if(grid->N[1]<3) return;
	if(grid->N[2]<3) return;

	N[0] = (grid->N[0]-1)/2+1;
	N[1] = (grid->N[1]-1)/2+1;
	N[2] = (grid->N[2]-1)/2+1;


	newLimits = grid->limits;
  	for(c=0;c<3;c++)
   		newLimits.MinMax[0][c] = grid->point[0][0][0].C[c];

	i = (grid->N[0]-1)-(grid->N[0]-1)%2;
	j = (grid->N[1]-1)-(grid->N[1]-1)%2;
	k = (grid->N[2]-1)-(grid->N[2]-1)%2;
  	for(c=0;c<3;c++)
   		newLimits.MinMax[1][c] = grid->point[i][j][k].C[c];

	newGrid = grid_point_alloc(N,newLimits);

	progress_orb(0,GABEDIT_PROGORB_SCALEGRID,TRUE);
	scal = (gdouble)1.01/grid->N[0]*2;
	for(i=0;i<grid->N[0];i+=2)
	{
		ii = i/2;
		for(j=0;j<grid->N[1];j+=2)
		{
			jj = j/2;
			for(k=0;k<grid->N[2];k+=2)
			{
				kk = k/2;
				for( c=0;c<4;c++) newGrid->point[ii][jj][kk].C[c] = grid->point[i][j][k].C[c];
				v = grid->point[i][j][k].C[3];
				if(beg)
				{
					beg = FALSE;
        				newGrid->limits.MinMax[0][3] =  v;
        				newGrid->limits.MinMax[1][3] =  v;
				}
                		else
				{
        				if(newGrid->limits.MinMax[0][3]>v)
        					newGrid->limits.MinMax[0][3] =  v;
        				if(newGrid->limits.MinMax[1][3]<v)
        					newGrid->limits.MinMax[1][3] =  v;
				}
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_SCALEGRID,TRUE);
			break;
		}
		progress_orb(scal,GABEDIT_PROGORB_SCALEGRID,FALSE);
	}
	progress_orb(0,GABEDIT_PROGORB_SCALEGRID,TRUE);
	if(CancelCalcul) newGrid = free_grid(newGrid);
	else
	{
		grid = free_grid(grid);
		grid = newGrid;
		limits = grid->limits;
	}
}
/**************************************************************************/
static void applySquareCube()
{
	gint i, j, k;
	gboolean beg = TRUE;
	gdouble v;
	gdouble scal;

	TypeGrid = GABEDIT_TYPEGRID_EDENSITY;
	progress_orb(0,GABEDIT_PROGORB_SCALEGRID,TRUE);
	scal = (gdouble)1.01/grid->N[0];
	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				grid->point[i][j][k].C[3] = grid->point[i][j][k].C[3]*grid->point[i][j][k].C[3];
				v = grid->point[i][j][k].C[3];
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
			progress_orb(0,GABEDIT_PROGORB_SCALEGRID,TRUE);
			break;
		}
		progress_orb(scal,GABEDIT_PROGORB_SCALEGRID,FALSE);
	}
	progress_orb(0,GABEDIT_PROGORB_SCALEGRID,TRUE);
	if(CancelCalcul) grid = free_grid(grid);
	else
	{
		limits = grid->limits;
	}
}
/********************************************************************************/
void square_cube()
{
	applySquareCube();
	create_iso_orbitals();
}
/********************************************************************************/
void restriction_cube()
{
	applyRestrictionCube();
	create_iso_orbitals();
}
/**************************************************************************/
void scale_cube_file(gdouble factor)
{
	gint i, j, k;
	gboolean beg = TRUE;
	gdouble v;
	gdouble scal;

	progress_orb(0,GABEDIT_PROGORB_SCALEGRID,TRUE);
	scal = (gdouble)1.01/grid->N[0];
	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				grid->point[i][j][k].C[3] *= factor;
				v = grid->point[i][j][k].C[3];
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
			progress_orb(0,GABEDIT_PROGORB_SCALEGRID,TRUE);
			break;
		}
		progress_orb(scal,GABEDIT_PROGORB_SCALEGRID,FALSE);
	}
	progress_orb(0,GABEDIT_PROGORB_SCALEGRID,TRUE);
	if(CancelCalcul) grid = free_grid(grid);
	else
	{
		limits = grid->limits;
	}
}
/********************************************************************************/
static void applyScale(GtkWidget *Win,gpointer data)
{
	GtkWidget* entry =(GtkWidget*)g_object_get_data(G_OBJECT (Win), "Entry");
	gchar* temp;
	gdouble factor;
	
	temp = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry))); 
	delete_first_spaces(temp);
	delete_last_spaces(temp);
	if(this_is_a_real(temp)) factor = atof(temp);
	else
	{
		GtkWidget* message =Message(_("Error : your value is not a float "),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		if(temp) g_free(temp);
		return;
	}
	if(temp) g_free(temp);
	scale_cube_file(factor);
	create_iso_orbitals();

	delete_child(Win);
	glarea_rafresh(GLArea);
}
/********************************************************************************/
void create_scale_dlg()
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *vboxall;
	GtkWidget *vboxwin;
	GtkWidget *button;
	GtkWidget* entry;
	GtkWidget* vboxframe;
	GtkWidget* table;

	if(!grid )
	{
		if( !CancelCalcul) Message(_("Sorry, Grid is not defined "),_("Error"),TRUE);
		return;
	}
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),_("Scale grid"));
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,_("Scale grid"));

	vboxall = create_vbox(Win);
	vboxwin = vboxall;
	frame = gtk_frame_new (NULL);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	table = gtk_table_new(1,3,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);
	add_label_table(table,_("Factor"),0,0);
	add_label_table(table,":",0,1);

	entry= gtk_entry_new ();
	add_widget_table(table,entry,0,2);
	gtk_entry_set_text(GTK_ENTRY(entry),"-1");

	gtk_widget_show_all(frame);
	g_object_set_data(G_OBJECT (Win), "Entry",entry);
   

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
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)applyScale,GTK_OBJECT(Win));
  

	gtk_widget_show_all (Win);
}
/**************************************************************************/
void subtract_cube_file(gchar* filename)
{
	FILE* file = FOpen(filename, "rb");
	gint len = BSIZE;
	gchar t[BSIZE];
	gint Natoms;
	gdouble XYZ0[3];
	gint N[3];
	gdouble X[3];
	gdouble Y[3];
	gdouble Z[3];
	Grid* tmpGrid;
	gdouble xh, yh, zh;
	gdouble scal;
	gint i, j, k;
	gboolean beg = TRUE;
	gdouble v;

	if(!file)
	{
		sprintf(t,_("Sorry, I can not open %s file"),filename);
		Message(t,_("Error"),TRUE);
		return ;
	}

	if(!fgets(t,len,file))  /* Title */   
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return ;
	}
	if(!fgets(t,len,file))  /* Type */   
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return;
	}
	if(!fgets(t,len,file))  /* NAtoms, X-Origin, Y-Origin, Z-Origin */   
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return;
	}
	if(sscanf(t,"%d %lf %lf %lf",&Natoms,&XYZ0[0],&XYZ0[1],&XYZ0[2])!=4)
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return;
	};
	Natoms = abs(Natoms);
	if(Natoms != nCenters)
		Message(_("The number of atoms in cube file is not equal to default value"),_("Warning"),TRUE);
	for(i=0;i<3;i++)
		if(fabs(XYZ0[i]-grid->point[0][0][0].C[i])>1e-6) 
		{
			Message(_("Sorry, probleme with origin of cube"),_("Error"),TRUE);
			fclose(file);
			return;
		}
	if(!fgets(t,len,file))  /* Nx, X0,Y0,Z0 */   
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return;
	}
	if(sscanf(t,"%d %lf %lf %lf",&N[0],&X[0],&Y[0],&Z[0])!=4)
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return;
	};
	xh = grid->point[1][0][0].C[0]-grid->point[0][0][0].C[0];
	yh = grid->point[0][1][0].C[0]-grid->point[0][0][0].C[0];
	zh = grid->point[0][0][1].C[0]-grid->point[0][0][0].C[0];
	if(N[0] != grid->N[0])
	{
		Message(_("Sorry, problem with number of points at x direction"),_("Error"),TRUE);
		return;
	}
	if(fabs(xh-X[0])>1e-6 || fabs(yh-Y[0])>1e-6 || fabs(zh-Z[0])>1e-6)
	{
		Message(_("Sorry, problem with X vecteor"),_("Error"),TRUE);
		return;
	}

	if(!fgets(t,len,file))  /* Ny, X1,Y1,Z1 */   
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return;
	}
	if(sscanf(t,"%d %lf %lf %lf",&N[1],&X[1],&Y[1],&Z[1])!=4)
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return;
	};
	xh = grid->point[1][0][0].C[1]-grid->point[0][0][0].C[1];
	yh = grid->point[0][1][0].C[1]-grid->point[0][0][0].C[1];
	zh = grid->point[0][0][1].C[1]-grid->point[0][0][0].C[1];
	if(N[1] != grid->N[1])
	{
		Message(_("Sorry, problem with number of points at y direction"),_("Error"),TRUE);
		return;
	}
	if(fabs(xh-X[1])>1e-6 || fabs(yh-Y[1])>1e-6 || fabs(zh-Z[1])>1e-6)
	{
		Message(_("Sorry, problem with Y vecteor"),_("Error"),TRUE);
		return;
	}
	if(!fgets(t,len,file))  /* Nz, X2, Y2,Z2 */   
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return;
	}
	if(sscanf(t,"%d %lf %lf %lf",&N[2],&X[2],&Y[2],&Z[2])!=4)
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return;
	};
	xh = grid->point[1][0][0].C[2]-grid->point[0][0][0].C[2];
	yh = grid->point[0][1][0].C[2]-grid->point[0][0][0].C[2];
	zh = grid->point[0][0][1].C[2]-grid->point[0][0][0].C[2];
	if(N[1] != grid->N[1])
	{
		Message(_("Sorry, problem with number of points at z direction"),_("Error"),TRUE);
		return;
	}
	if(fabs(xh-X[2])>1e-6 || fabs(yh-Y[2])>1e-6 || fabs(zh-Z[2])>1e-6)
	{
		Message(_("Sorry, problem with Z vecteor"),_("Error"),TRUE);
		return;
	}
	j=-1;
	while(!feof(file) && (j+1<(gint)Natoms))
	{
		j++;
		if(!fgets(t,len,file))
		{
			sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
			Message(t,_("Error"),TRUE);
			return;
		}
		/* printf("%s\n",t);*/
	}

	tmpGrid = get_grid_from_gauss_molpro_cube_file(0,file,1,1,N,XYZ0,X,Y,Z);
	if(!tmpGrid)
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		return;
	}

	progress_orb(0,GABEDIT_PROGORB_SUBSGRID,TRUE);
	scal = (gdouble)1.01/grid->N[0];
	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				grid->point[i][j][k].C[3] -= tmpGrid->point[i][j][k].C[3];
				v = grid->point[i][j][k].C[3];
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
			progress_orb(0,GABEDIT_PROGORB_SUBSGRID,TRUE);
			break;
		}
		progress_orb(scal,GABEDIT_PROGORB_SUBSGRID,FALSE);
	}
	progress_orb(0,GABEDIT_PROGORB_SUBSGRID,TRUE);
	free_grid(tmpGrid);
	fclose(file);
	limits = grid->limits;
	create_iso_orbitals();
}
/**************************************************************************/
void mapping_cube_by_an_other_cube(Grid* tmpGrid)
{
	gdouble scal;
	gint i, j, k;
	ColorMap* colorMap = NULL;

	if(!tmpGrid) return;

	progress_orb(0,GABEDIT_PROGORB_MAPGRID,TRUE);
	scal = (gdouble)1.01/grid->N[0];
	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				grid->point[i][j][k].C[4] = tmpGrid->point[i][j][k].C[3];
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_MAPGRID,TRUE);
			break;
		}
		progress_orb(scal,GABEDIT_PROGORB_MAPGRID,FALSE);
	}
	progress_orb(0,GABEDIT_PROGORB_MAPGRID,TRUE);
	limits = grid->limits;
	grid->mapped = TRUE;
	RebuildSurf = TRUE;
	colorMap = new_colorMap_fromGrid(grid);
	{
		GtkWidget* handleBoxColorMapGrid = g_object_get_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapGrid ");
		if(handleBoxColorMapGrid)
		{
			GtkWidget* entryLeft  = g_object_get_data(G_OBJECT(handleBoxColorMapGrid), "EntryLeft");
			GtkWidget* entryRight = g_object_get_data(G_OBJECT(handleBoxColorMapGrid), "EntryRight");
			GtkWidget* darea      = g_object_get_data(G_OBJECT(handleBoxColorMapGrid), "DrawingArea");

			g_object_set_data(G_OBJECT(handleBoxColorMapGrid),"ColorMap", colorMap);
			g_object_set_data(G_OBJECT(entryLeft),"ColorMap", colorMap);
			g_object_set_data(G_OBJECT(entryRight),"ColorMap", colorMap);
			g_object_set_data(G_OBJECT(darea),"ColorMap", colorMap);
			color_map_show(handleBoxColorMapGrid);
		}
		else
		{
			printf("Error handleBoxColorMapGrid ==NULL\n");
		}
	}
	if(TypeGrid == GABEDIT_TYPEGRID_SAS)
	{
		Define_Iso(0.0);
		glarea_rafresh(GLArea);
	}
	else create_iso_orbitals();
}
/**************************************************************************/
void read_mapping_cube_file(gchar* filename)
{
	FILE* file = FOpen(filename, "rb");
	gint len = BSIZE;
	gchar t[BSIZE];
	gint Natoms;
	gdouble XYZ0[3];
	gint N[3];
	gdouble X[3];
	gdouble Y[3];
	gdouble Z[3];
	Grid* tmpGrid;
	gdouble xh, yh, zh;
	gint i, j;

	if(!file)
	{
		sprintf(t,_("Sorry, I can not open %s file"),filename);
		Message(t,_("Error"),TRUE);
		return ;
	}

	if(!fgets(t,len,file))  /* Title */   
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return ;
	}
	if(!fgets(t,len,file))  /* Type */   
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return;
	}
	if(!fgets(t,len,file))  /* NAtoms, X-Origin, Y-Origin, Z-Origin */   
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return;
	}
	if(sscanf(t,"%d %lf %lf %lf",&Natoms,&XYZ0[0],&XYZ0[1],&XYZ0[2])!=4)
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return;
	};
	Natoms = abs(Natoms);
	if(Natoms != nCenters)
		Message(_("The number of atoms in cube file is not equal to default value"),_("Warning"),TRUE);
	for(i=0;i<3;i++)
		if(fabs(XYZ0[i]-grid->point[0][0][0].C[i])>1e-6) 
		{
			Message(_("Sorry, probleme with origin of cube"),_("Error"),TRUE);
			fclose(file);
			return;
		}
	if(!fgets(t,len,file))  /* Nx, X0,Y0,Z0 */   
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return;
	}
	if(sscanf(t,"%d %lf %lf %lf",&N[0],&X[0],&Y[0],&Z[0])!=4)
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return;
	};
	xh = grid->point[1][0][0].C[0]-grid->point[0][0][0].C[0];
	yh = grid->point[0][1][0].C[0]-grid->point[0][0][0].C[0];
	zh = grid->point[0][0][1].C[0]-grid->point[0][0][0].C[0];
	if(N[0] != grid->N[0])
	{
		Message(_("Sorry, problem with number of points at x direction"),_("Error"),TRUE);
		return;
	}
	if(fabs(xh-X[0])>1e-6 || fabs(yh-Y[0])>1e-6 || fabs(zh-Z[0])>1e-6)
	{
		Message(_("Sorry, problem with X vecteor"),_("Error"),TRUE);
		return;
	}

	if(!fgets(t,len,file))  /* Ny, X1,Y1,Z1 */   
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return;
	}
	if(sscanf(t,"%d %lf %lf %lf",&N[1],&X[1],&Y[1],&Z[1])!=4)
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return;
	};
	xh = grid->point[1][0][0].C[1]-grid->point[0][0][0].C[1];
	yh = grid->point[0][1][0].C[1]-grid->point[0][0][0].C[1];
	zh = grid->point[0][0][1].C[1]-grid->point[0][0][0].C[1];
	if(N[1] != grid->N[1])
	{
		Message(_("Sorry, problem with number of points at y direction"),_("Error"),TRUE);
		return;
	}
	if(fabs(xh-X[1])>1e-6 || fabs(yh-Y[1])>1e-6 || fabs(zh-Z[1])>1e-6)
	{
		Message(_("Sorry, problem with Y vecteor"),_("Error"),TRUE);
		return;
	}
	if(!fgets(t,len,file))  /* Nz, X2, Y2,Z2 */   
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return;
	}
	if(sscanf(t,"%d %lf %lf %lf",&N[2],&X[2],&Y[2],&Z[2])!=4)
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		fclose(file);
		return;
	};
	xh = grid->point[1][0][0].C[2]-grid->point[0][0][0].C[2];
	yh = grid->point[0][1][0].C[2]-grid->point[0][0][0].C[2];
	zh = grid->point[0][0][1].C[2]-grid->point[0][0][0].C[2];
	if(N[1] != grid->N[1])
	{
		Message(_("Sorry, problem with number of points at z direction"),_("Error"),TRUE);
		return;
	}
	if(fabs(xh-X[2])>1e-6 || fabs(yh-Y[2])>1e-6 || fabs(zh-Z[2])>1e-6)
	{
		Message(_("Sorry, problem with Z vecteor"),_("Error"),TRUE);
		return;
	}
	j=-1;
	while(!feof(file) && (j+1<(gint)Natoms))
	{
		j++;
		if(!fgets(t,len,file))
		{
			sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
			Message(t,_("Error"),TRUE);
			return;
		}
		/* printf("%s\n",t);*/
	}

	tmpGrid = get_grid_from_gauss_molpro_cube_file(0,file,1,1,N,XYZ0,X,Y,Z);
	fclose(file);
	if(!tmpGrid)
	{
		sprintf(t,_("Sorry, I can not read cube from %s file"),filename);
		Message(t,_("Error"),TRUE);
		return;
	}

	mapping_cube_by_an_other_cube(tmpGrid);
	free_grid(tmpGrid);
}
/********************************************************************************/
void save_grid_gabedit_cube_file(gchar* filename)
{
	FILE* file = FOpen(filename, "w");
	gint i, j, k;
	gdouble scal;
	gdouble dum;
	gdouble xh, yh, zh;

	fprintf(file,"Grid file generated by Gabedit\n");
	fprintf(file,"Density\n");
	fprintf(file,"%d %0.14le %0.14le %0.14le\n",nCenters,grid->point[0][0][0].C[0],grid->point[0][0][0].C[1],grid->point[0][0][0].C[2]);
	xh = grid->point[1][0][0].C[0]-grid->point[0][0][0].C[0];
	yh = grid->point[0][1][0].C[0]-grid->point[0][0][0].C[0];
	zh = grid->point[0][0][1].C[0]-grid->point[0][0][0].C[0];
	fprintf(file,"%d %0.14le %0.14le %0.14le\n",grid->N[0],xh, yh, zh);

	xh = grid->point[1][0][0].C[1]-grid->point[0][0][0].C[1];
	yh = grid->point[0][1][0].C[1]-grid->point[0][0][0].C[1];
	zh = grid->point[0][0][1].C[1]-grid->point[0][0][0].C[1];
	fprintf(file,"%d %0.14le %0.14le %0.14le\n",grid->N[1],xh, yh, zh);

	xh = grid->point[1][0][0].C[2]-grid->point[0][0][0].C[2];
	yh = grid->point[0][1][0].C[2]-grid->point[0][0][0].C[2];
	zh = grid->point[0][0][1].C[2]-grid->point[0][0][0].C[2];
	fprintf(file,"%d %0.14le %0.14le %0.14le\n",grid->N[2],xh, yh, zh);

	set_status_label_info(_("Geometry"),_("Writing..."));

	progress_orb(0,GABEDIT_PROGORB_SAVEGEOM,TRUE);
	scal = (gdouble)1.01/nCenters;

	dum = 0.0;
	for(j=0; j<(gint)nCenters; j++)
	{
		progress_orb(scal,GABEDIT_PROGORB_SAVEGEOM,FALSE);
		fprintf(file,"%d %0.14le %0.14le %0.14le %0.14le\n",(gint)GeomOrb[j].Prop.atomicNumber,dum,GeomOrb[j].C[0],GeomOrb[j].C[1],GeomOrb[j].C[2]);
	}
	set_status_label_info(_("Geometry"),_("Ok"));

	progress_orb(0,GABEDIT_PROGORB_SAVEGRID,TRUE);
	scal = (gdouble)1.01/grid->N[0];
 
	set_status_label_info(_("Grid"),_("Writing..."));
	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				fprintf(file,"%0.14le ",grid->point[i][j][k].C[3]);
				if((k+1)%6==0) fprintf(file,"\n");
			}
			if(grid->N[2]%6 !=0) fprintf(file,"\n");
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_SAVEGRID,TRUE);
			break;
		}

		progress_orb(scal,GABEDIT_PROGORB_SAVEGRID,FALSE);
	}
	progress_orb(0,GABEDIT_PROGORB_SAVEGRID,TRUE);
	set_status_label_info(_("Grid"),_("Ok"));
	CancelCalcul = FALSE;
	fclose(file);
}
/************************************************************************/
gint get_orbitals_number_from_molpro_cube_file(FILE* file,gint N[])
{
	gint len = BSIZE;
	gchar t[BSIZE];
	long int k = 0;
	gint norbs;
	gint nval = 6;
	/*
	gdouble V[6];
	*/

	/*Debug("Begin scan orbitals molpro cube file \n");*/
	progress_orb(0,GABEDIT_PROGORB_SCANFILEGRID,TRUE);
	progress_orb(0,GABEDIT_PROGORB_SCANFILEGRID,FALSE);
	while(!feof(file) && !CancelCalcul)
	{
		if(!fgets(t,len,file))  /* Nx, X0,X1,X2 */   
			break;

		/* Debug("t = %s",t);*/

		nval = (strlen(t)+1)/13;
		/* nval = numb_of_string_by_row(t);*/
		/*
		nval =sscanf(t,"%lf %lf %lf %lf %lf %lf",
			&V[0], &V[1], &V[2],
			&V[3], &V[4], &V[5]
			);
		*/
		k += nval;
		/* Debug("k=%d\n",k);*/
		if(k%N[2]==0)
			progress_orb(-0.1,GABEDIT_PROGORB_SCANFILEGRID,FALSE);
	}
	if(!CancelCalcul)
	{
		/* Debug("End scan orbitals molpro cube file %ld\n",k);*/
		norbs = k/((long int)N[0]*(long int)N[1]*(long int)N[2]);
	}
	else
		norbs = 0;

	progress_orb(0,GABEDIT_PROGORB_SCANFILEGRID,TRUE);
	return norbs;
}
/****************************************************************************************************************/
gboolean get_values_from_gauss_molpro_cube_density_file(FILE* file,gint numblock,gint nblocks,gint N2,gdouble V[])
{
	gint len = BSIZE;
	gchar t[BSIZE];
	gint k = 0;
	gint i;
	gint n = nblocks*N2;
	gdouble* tmpV = g_malloc((nblocks*N2+6)*sizeof(gdouble));
	gint nval = 6;

	/*Debug("Begin read den n = %d\n",n);*/
	while(!feof(file))
	{
		if(!fgets(t,len,file))  /* Nx, X0,X1,X2 */   
		{
			Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
			g_free(tmpV);
			return FALSE;
		}
		/* Debug("t = %s",t);*/
		nval =sscanf(t,"%lf %lf %lf %lf %lf %lf",
			&tmpV[k], &tmpV[k+1], &tmpV[k+2],
			&tmpV[k+3], &tmpV[k+4], &tmpV[k+5]
			);

		if(k+5>=n-1)
			break;
		k += nval;
		/* Debug("k=%d\n",k);*/
	}
	/* Debug("End read den\n");*/
	k=(numblock-1)*N2;
	for(i=0;i<N2;i++)
	{
		V[i] = tmpV[i+k];
	}
	g_free(tmpV);
	return TRUE;
}
/********************************************************************************/
gboolean get_values_from_gauss_molpro_cube_orbitals_file(FILE* file,gint numorb,gint norbs,gint N2,gdouble V[])
{
	gint len = BSIZE;
	gchar t[BSIZE];
	gint k = 0;
	gint i;
	gint n = norbs*N2;
	gdouble* tmpV = g_malloc((norbs*N2+6)*sizeof(gdouble));

	/* Debug("Begin read orb n = %d\n",n);*/
	while(!feof(file))
	{
		if(!fgets(t,len,file))  /* Nx, X0,X1,X2 */   
		{
			Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
			g_free(tmpV);
			return FALSE;
		}
		/* Debug("t = %s",t);*/
		if(sscanf(t,"%lf %lf %lf %lf %lf %lf",
			&tmpV[k], &tmpV[k+1], &tmpV[k+2],
			&tmpV[k+3], &tmpV[k+4], &tmpV[k+5]
			)!=6)
		{
			if(k+5>=n-1)
	                        break;
			/* Debug("Error t =%s ",t);*/
			Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
			g_free(tmpV);
			return FALSE;
		}
		if(k+5>=n-1)
			break;
		k += 6;
		/* Debug("k=%d\n",k);*/
	}
	/* Debug("End read orb\n");*/
	k=numorb-1;
	for(i=0;i<N2;i++)
	{
		V[i] = tmpV[i+k];
		k+=norbs-1;
	}
	g_free(tmpV);
	return TRUE;
}
/********************************************************************************/
/* 
 * typefile = 0 =>  cube orbitals file 
 * typefile = 1 =>  cube density file 
*/
Grid* get_grid_from_gauss_molpro_cube_file(gint typefile,FILE* file,gint num,gint n,gint N[],
		gdouble XYZ0[3],gdouble X[3],gdouble Y[3],gdouble Z[3])
{
	Grid* grid;
	gint i;
	gint j;
	gint k;
	gdouble x;
	gdouble y;
	gdouble z;
	gdouble v;
    	gboolean beg = TRUE;
	gdouble scal;
	GridLimits limits;
	gdouble* V;
	FuncGetVal Func;

	if(typefile==0)
		Func = get_values_from_gauss_molpro_cube_orbitals_file;
	else
		Func = get_values_from_gauss_molpro_cube_density_file;

  	for(i=0;i<3;i++)
   		limits.MinMax[0][i] = XYZ0[i];

	limits.MinMax[1][0] = XYZ0[0] + (N[0]-1)*X[0] + (N[1]-1)*X[1] +  (N[2]-1)*X[2];
	limits.MinMax[1][1] = XYZ0[1] + (N[0]-1)*Y[0] + (N[1]-1)*Y[1] +  (N[2]-1)*Y[2];
	limits.MinMax[1][2] = XYZ0[2] + (N[0]-1)*Z[0] + (N[1]-1)*Z[1] +  (N[2]-1)*Z[2];

	grid = grid_point_alloc(N,limits);

	progress_orb(0,GABEDIT_PROGORB_READGRID,TRUE);
	scal = (gdouble)1.01/grid->N[0];
 
	V = g_malloc((N[2]+6)*sizeof(gdouble));
	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			if(!Func(file,num,n,N[2],V))
			{
				/* Debug("Error Free Grid\n");*/
				grid = free_grid(grid);
				/* Debug("ENd Free Grid\n");*/
				return NULL;
			}
			/* Debug("\n");*/
			for(k=0;k<grid->N[2];k++)
			{
				x = XYZ0[0] + i*X[0] + j*X[1] +  k*X[2]; 
				y = XYZ0[1] + i*Y[0] + j*Y[1] +  k*Y[2]; 
				z = XYZ0[2] + i*Z[0] + j*Z[1] +  k*Z[2]; 

				v = V[k];
				/* Debug("%lf %lf %lf %lf \n",x,y,z,v);*/
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
	return grid;
}
/**************************************************************/
gboolean read_geometry_from_gauss_cube_file(FILE* file,gint Natoms)
{
	gchar *AtomCoord[3];
	gint len = BSIZE;
	gchar t[BSIZE];
	gint i;
	gint j;
	gint N;
	gdouble dum;
	gdouble scal;
	gboolean OK = TRUE;

	for(i=0;i<3;i++)
		AtomCoord[i]=g_malloc(len*sizeof(gchar));


	nCenters = Natoms;
	GeomOrb=g_malloc(nCenters*sizeof(TypeGeomOrb));

	set_status_label_info(_("Geometry"),_("Reading"));

	progress_orb(0,GABEDIT_PROGORB_READGEOM,TRUE);
	scal = (gdouble)1.01/nCenters;

	j=-1;
	while(!feof(file) && (j<(gint)nCenters))
	{
		j++;
		if(j>=(gint)nCenters)
			break;
		progress_orb(scal,GABEDIT_PROGORB_READGEOM,FALSE);
		if(!fgets(t,len,file))
		{
			OK = FALSE;
			break;
		}
		if(5 != sscanf(t,"%d %lf %s %s %s",&N,&dum,AtomCoord[0],AtomCoord[1],AtomCoord[2]))
		{
			OK = FALSE;
			break;
		}

		GeomOrb[j].Symb=symb_atom_get((guint)N);
		/* Debug("Symb = %s  ", GeomOrb[j].Symb);*/

		for(i=0;i<3;i++)
		{
			/* GeomOrb[j].C[i] = atof(ang_to_bohr(AtomCoord[i]));*/
			GeomOrb[j].C[i] = atof(AtomCoord[i]);
			/* Debug("%lf  ", GeomOrb[j].C[i]);*/
		}
		/* Debug("\n");*/
		 GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb[j].partialCharge = 0.0;
		GeomOrb[j].variable = TRUE;
		GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
	}

	for(i=0;i<3;i++)
		g_free(AtomCoord[i]);
	if(OK)
	{
		buildBondsOrb();
		RebuildGeomD = TRUE;
		if(this_is_a_new_geometry()) free_objects_all();
		glarea_rafresh(GLArea);
		init_atomic_orbitals();
		set_status_label_info(_("Geometry"),_("Ok"));
	}
	else
	{
		free_data_all();
		set_status_label_info(_("Geometry"),_("Nothing"));
	}
	return OK;
}
/**************************************************************/
gboolean read_grid_limits_from_gauss_cube_file(FILE* file,gint N[],gdouble X[3],gdouble Y[3],gdouble Z[3])
{
	gint len = BSIZE;
	gchar t[BSIZE];
	if(!fgets(t,len,file))  /* Nx, X0,Y0,Z0 */   
	{
		Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
		fclose(file);
		return FALSE;
	}
	if(sscanf(t,"%d %lf %lf %lf",&N[0],&X[0],&Y[0],&Z[0])!=4)
	{
		Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
		fclose(file);
		return FALSE;
	};
	if(!fgets(t,len,file))  /* Ny, X1,Y1,Z1 */   
	{
		Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
		fclose(file);
		return FALSE;
	}
	if(sscanf(t,"%d %lf %lf %lf",&N[1],&X[1],&Y[1],&Z[1])!=4)
	{
		Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
		fclose(file);
		return FALSE;
	};
	if(!fgets(t,len,file))  /* Nz, X2, Y2,Z2 */   
	{
		Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
		fclose(file);
		return FALSE;
	}
	if(sscanf(t,"%d %lf %lf %lf",&N[2],&X[2],&Y[2],&Z[2])!=4)
	{
		Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
		fclose(file);
		return FALSE;
	};
	return TRUE;

}
/**************************************************************/
gint* read_numorbs_from_gauss_cube_file(FILE* file)
{
	gint len = BSIZE;
	gchar t[BSIZE];
	gchar** allints;
	gint norbs;
	gint* numorbs = NULL;
	gint i;
	gint k;

	if(!fgets(t,len,file)) 
	{
		Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
		return NULL;
	}
 	allints =gab_split (t);
	norbs = atoi(allints[0]);
	if(norbs<=0)
	{
		if(allints)
			g_strfreev(allints);
		return NULL;
	}
	numorbs = g_malloc((norbs+1)*sizeof(gint));
	numorbs[0] = norbs;
	for(i=1;i<=norbs && allints[i]!=NULL;i++)
		numorbs[i] = atoi(allints[i]);

	g_strfreev(allints);
	if(i==(norbs+1))
	{
		return numorbs;
	}

	k = i;

	/* Debug("k=%d\n",k);*/

	while(!feof(file))
	{
		if(!fgets(t,len,file)) 
		{
			Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
			return NULL;
		}
		/* Debug("t =%s\n",t);*/
 		allints =gab_split (t);
		for(i=0;i+k<=norbs && allints[i]!=NULL;i++)
			numorbs[i+k] = atoi(allints[i]);

		g_strfreev(allints);
		if(i+k==(norbs+1))
		{
			return numorbs;
		}
		k = k+i;
	}
	return numorbs;
}
/**********************************************************************************************************/
static GtkWidget *create_orbitals_number_frame( GtkWidget *vboxall,GtkWidget **entry,gint norbs)
{
  GtkWidget *frame;
  GtkWidget *combo;
  GtkWidget *vboxframe;
#define NL 2
  gchar      *LabelLeft[NL];
  gchar      **tlistnum=NULL;
  gushort i;
  GtkWidget *Table;

  LabelLeft[0] = g_strdup(_("Total number of orbitals"));
  LabelLeft[1] = g_strdup(_("Orbital number"));

  tlistnum  = g_malloc(norbs*sizeof(gchar*));
  for(i=0;i<norbs;i++)
  {
	  tlistnum[i] = g_strdup_printf("%d",i+1);
  }

  frame = gtk_frame_new (_("Selection of orbital number"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  Table = gtk_table_new(2,3,FALSE);
  gtk_container_add(GTK_CONTAINER(vboxframe),Table);

	add_label_table(Table,LabelLeft[0],0,0);
	add_label_table(Table,":",0,1);
	add_label_table(Table,tlistnum[norbs-1],0,2);

	add_label_table(Table,LabelLeft[1],1,0);
	add_label_table(Table,":",1,1);
	combo = create_combo_box_entry(tlistnum,norbs,TRUE,-1,-1);
	add_widget_table(Table,combo,1,2);
	entry[0] = GTK_BIN(combo)->child;
	gtk_widget_set_sensitive(entry[0],FALSE); 

	gtk_widget_show_all(frame);
  
  	for (i=0;i<NL;i++)
        	g_free(LabelLeft[i]);
  	for (i=0;i<norbs;i++)
        	g_free(tlistnum[i]);
	g_free(tlistnum);

  	return frame;
}
/**************************************************************************/
gboolean read_gauss_molpro_cube_orbitals_file(gchar* filename,gint numorb,gint Norbs,gint typefile, gboolean showisowin)
{
	FILE* file = FOpen(filename, "rb");
	gchar* tmp;
	gint len = BSIZE;
	gchar t[BSIZE];
	gint Natoms;
	gdouble XYZ0[3];
	gint type;
	gint N[3];
	gdouble X[3];
	gdouble Y[3];
	gdouble Z[3];
	gint* numorbs = NULL;
	gint norbs = Norbs;

	/* Debug("Filename  = %s\n",filename);*/
	if(!file)
	{
		Message(_("I can not open file\n"),_("Error"),TRUE);
		return FALSE;
	}

	free_data_all();
	tmp = get_name_file(filename);
	set_status_label_info(_("File Name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Gaussian Cube");

	if(!fgets(t,len,file))  /* Title */   
	{
		Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		fclose(file);
		return FALSE;
	}
	/* Debug("t = %s\n",t);*/
	if(!fgets(t,len,file))  /* Type */   
	{
		Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		fclose(file);
		return FALSE;
	}
	/* Debug("t = %s\n",t);*/
	if(!fgets(t,len,file))  /* NAtoms, X-Origin, Y-Origin, Z-Origin */   
	{
		Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		fclose(file);
		return FALSE;
	}
	/* Debug("t = %s\n",t);*/
	if(sscanf(t,"%d %lf %lf %lf",&Natoms,&XYZ0[0],&XYZ0[1],&XYZ0[2])!=4)
	{
		Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		fclose(file);
		return FALSE;
	};
	if(Natoms<0)
		type = 0;
	else
		type = 1;
	Natoms = abs(Natoms);

	/* Debug("t = %s\n",t);*/
	/* Debug("read limits %s\n");*/
	if(!read_grid_limits_from_gauss_cube_file(file,N,X,Y,Z))
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		return FALSE;
	}

	/* Debug("read geometry %s\n");*/
	if(!read_geometry_from_gauss_cube_file(file,Natoms))
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		return FALSE;
	}
	/* Debug("Natoms = %d\n",Natoms);*/
	/* Debug("X = %lf %lf %lf \n",X[0],X[1],X[2]);*/
	/* Debug("Y = %lf %lf %lf \n",Y[0],Y[1],Y[2]);*/
	/* Debug("Z = %lf %lf %lf \n",Z[0],Z[1],Z[2]);*/

	if(typefile==0)
	{
		numorbs = read_numorbs_from_gauss_cube_file(file);
		/* Debug("End read_numorbs_from_gauss_cube_file\n");*/
		if(numorbs)
		{
			norbs = numorbs[0];
			g_free(numorbs);
		}
	}

	if(numorb>norbs)
		numorb = norbs;
	set_status_label_info("Grid","Reading...");
	if(typefile==0) grid = get_grid_from_gauss_molpro_cube_file(0,file,numorb,norbs,N,XYZ0,X,Y,Z);
	else grid = get_grid_from_gauss_molpro_cube_file(1,file,numorb,norbs,N,XYZ0,X,Y,Z);
	if(grid)
	{
        	limits = grid->limits;
		if(showisowin) create_iso_orbitals();
		set_status_label_info("Grid","Ok");
	}
	else
	{
		set_status_label_info("Grid","Nothing");
		CancelCalcul = FALSE;
	}

	fclose(file);
	return (grid!=NULL);
}
/********************************************************************************/
static void apply_read_orb(GtkWidget *button,gpointer data)
{
	GtkWidget **entry = (GtkWidget **)data;
	GtkWidget *Win = g_object_get_data(G_OBJECT (button), "Window");
	gchar* FileName =(gchar*)g_object_get_data(G_OBJECT (button), "FileName");
	gint* Norbs =(gint*)g_object_get_data(G_OBJECT (button), "Norbs");
	gint* TypeFile =(gint*)g_object_get_data(G_OBJECT (button), "TypeFile");
	G_CONST_RETURN gchar* t;
	gint numorb;

        if(!this_is_an_object((GtkObject*)entry[0]))
		return;

	t = gtk_entry_get_text(GTK_ENTRY(entry[0]));

	numorb = atoi(t);
	if(numorb>*Norbs || numorb<=0)
	{
		Message(_("Error detected in gabedit\n Please contat the author for this bug"),_("Error"),TRUE);
		g_free(FileName);
		g_free(Norbs);
  		delete_child(Win);
		return;
	}
  	delete_child(Win);
	read_gauss_molpro_cube_orbitals_file(FileName,numorb,*Norbs,*TypeFile,TRUE);
	g_free(FileName);
	g_free(Norbs);
	g_free(TypeFile);
}
/********************************************************************************/
/* 
 * typefile = 0 => Gaussian
 * typefile = 1 => Molpro
*/
static void create_window_list_orbitals_numbers(GtkWidget *w,gint norbs,gchar* filename,gint typefile)
{
  GtkWidget *fp;
  GtkWidget *sep;
  GtkWidget *frame;
  GtkWidget *vboxall;
  GtkWidget *vboxwin;
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget **entry;
  gchar *title = _("Orbitals available ");
  gint* Norbs=g_malloc(sizeof(gint));
  gint* TypeFile=g_malloc(sizeof(gint));
  gchar* FileName = g_strdup(filename);

  *Norbs = norbs;
  *TypeFile = typefile;

  entry=g_malloc(sizeof(GtkWidget *));

  /* Principal Window */
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(fp),title);
  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(PrincipalWindow));
  gtk_window_set_modal (GTK_WINDOW (fp), TRUE);

  gtk_widget_realize(fp);

  add_glarea_child(fp," Orb. List ");


  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);
  vboxall = create_vbox(fp);
  vboxwin = vboxall;

  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_add(GTK_CONTAINER(vboxall),frame);
  gtk_widget_show (frame);

  vboxall = create_vbox(frame);

  /* Debug("Creation of orbitals numbers frame\n");*/

  frame = create_orbitals_number_frame(vboxall,entry,norbs); 

  sep = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(vboxwin), sep, FALSE, FALSE, 2);
  gtk_widget_show(sep);

  hbox = create_hbox(vboxwin);
  gtk_widget_realize(fp);

  button = create_button(fp,_("Cancel"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 5);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(fp));
  gtk_widget_show (button);

  button = create_button(fp,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 5);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);
  g_object_set_data(G_OBJECT (button), "Window", fp);
  g_object_set_data(G_OBJECT (button), "FileName", FileName);
  g_object_set_data(G_OBJECT (button), "Norbs", Norbs);
  g_object_set_data(G_OBJECT (button), "TypeFile", TypeFile);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(apply_read_orb),(gpointer)entry);
  

  gtk_widget_show_all(fp);
}
/**************************************************************/
void read_gauss_molpro_cube_file(GabEditTypeCube typefile,gchar* filename)
{
	FILE* file = FOpen(filename, "rb");
	gchar* tmp;
	gint len = BSIZE;
	gchar t[BSIZE];
	gint Natoms;
	gdouble XYZ0[3];
	gint type;
	gint N[3];
	gdouble X[3];
	gdouble Y[3];
	gdouble Z[3];
	gint* numorbs = NULL;
	gint norbs = 1;

	CancelCalcul = FALSE;
	
	if(!file)
	{
		Message(_("I can not open file\n"),_("Error"),TRUE);
		return ;
	}

	free_data_all();
	tmp = get_name_file(filename);
	set_status_label_info(_("File Name"),tmp);
	g_free(tmp);
	switch(typefile)
	{
  		case GABEDIT_CUBE_GABEDIT : 
				set_status_label_info(_("File type"),"Gabedit Cube");
				break;
  		case GABEDIT_CUBE_GAUSS_ORB : 
  		case GABEDIT_CUBE_GAUSS_DEN : 
  		case GABEDIT_CUBE_GAUSS_GRAD : 
  		case GABEDIT_CUBE_GAUSS_LAP : 
  		case GABEDIT_CUBE_GAUSS_NGRAD : 
  		case GABEDIT_CUBE_GAUSS_POT : 
				set_status_label_info(_("File type"),"Gaussian Cube");
				break;
  		case GABEDIT_CUBE_MOLPRO_ORB1 : 
  		case GABEDIT_CUBE_MOLPRO_ORBN : 
  		case GABEDIT_CUBE_MOLPRO_DEN : 
  		case GABEDIT_CUBE_MOLPRO_DEN_GRAD : 
  		case GABEDIT_CUBE_MOLPRO_LAPDEN : 
		case GABEDIT_CUBE_MOLPRO_LAPLAP :
				set_status_label_info(_("File type"),"Molpro Cube");
				break;
	}

	if(!fgets(t,len,file))  /* Title */   
	{
		Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		fclose(file);
		return ;
	}
	/* Debug("t = %s\n",t);*/
	if(!fgets(t,len,file))  /* Type */   
	{
		Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		fclose(file);
		return;
	}
	/* Debug("t = %s\n",t);*/
	if(!fgets(t,len,file))  /* NAtoms, X-Origin, Y-Origin, Z-Origin */   
	{
		Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		fclose(file);
		return;
	}
	/* Debug("t = %s\n",t);*/
	if(sscanf(t,"%d %lf %lf %lf",&Natoms,&XYZ0[0],&XYZ0[1],&XYZ0[2])!=4)
	{
		Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		fclose(file);
		return;
	};
	if(Natoms<0)
		type = 0;
	else
		type = 1;
	Natoms = abs(Natoms);

	/* Debug("t = %s\n",t);*/
	/* Debug("read limits %s\n");*/
	if(!read_grid_limits_from_gauss_cube_file(file,N,X,Y,Z))
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		return;
	}

	/* Debug("read geometry %s\n");*/
	if(!read_geometry_from_gauss_cube_file(file,Natoms))
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		return;
	}
	/* Debug("Natoms = %d\n",Natoms);*/
	/* Debug("X = %lf %lf %lf \n",X[0],X[1],X[2]);*/
	/* Debug("Y = %lf %lf %lf \n",Y[0],Y[1],Y[2]);*/
	/* Debug("Z = %lf %lf %lf \n",Z[0],Z[1],Z[2]);*/
	if(type==0 && typefile ==GABEDIT_CUBE_GAUSS_ORB)
	{
		if(typefile !=  GABEDIT_CUBE_GAUSS_ORB)
		{
			TypeGrid = GABEDIT_TYPEGRID_ORBITAL;
			Message(_("Warning : This file is a orbitals cube file\nI read The first orbital\n"),_("Warning"),TRUE);
		}

		numorbs = read_numorbs_from_gauss_cube_file(file);
		if(numorbs)
		{
			norbs = numorbs[0];
			g_free(numorbs);
		}
		if(norbs<=1)
		{
			set_status_label_info("Grid","Reading...");
			grid = get_grid_from_gauss_molpro_cube_file(0,file,norbs,norbs,N,XYZ0,X,Y,Z);
		}
		else
		{
			/* Debug("Creation of window orbitals list norbs = %d \n",norbs);*/
  			create_window_list_orbitals_numbers(NULL,norbs,filename,0);
			/* Debug("End Creation of window orbitals list\n");*/
		}
	}
	else 
	if(type==0 && typefile ==GABEDIT_CUBE_MOLPRO_ORBN)
	{
		norbs = get_orbitals_number_from_molpro_cube_file(file,N);
		if(norbs==0)
		{
			grid = NULL;
			if(!CancelCalcul)
				Message(_("Sorry this is not a orbitals molpro cube file\n"),_("Error"),TRUE);
			CancelCalcul = FALSE;

		}
		else
		if(norbs==1)
		{
			Message(_("One orbital detected in this file\n"),_("Warning"),TRUE);
			set_status_label_info("Grid","Reading...");
			grid = get_grid_from_gauss_molpro_cube_file(0,file,norbs,norbs,N,XYZ0,X,Y,Z);
		}
		else
		{
			/* Debug("Creation of window orbitals list norbs = %d \n",norbs);*/
  			create_window_list_orbitals_numbers(NULL,norbs,filename,1);
			/* Debug("End Creation of window orbitals list\n");*/
		}

		/* Debug("Total number of orbitals = %d ",norbs);*/
		
	}
	else
	{
		set_status_label_info("Grid","Reading...");
		switch(typefile)
		{
			case GABEDIT_CUBE_GAUSS_ORB:
				Message(_("Error : This file is not a orbitals cube file\n"),_("Error"),TRUE);
				grid = NULL;
				break;
			case GABEDIT_CUBE_GAUSS_DEN:
				grid = get_grid_from_gauss_molpro_cube_file(1,file,1,1,N,XYZ0,X,Y,Z);
			      	break;
			case GABEDIT_CUBE_GAUSS_GRAD:
				grid = get_grid_from_gauss_molpro_cube_file(1,file,1,4,N,XYZ0,X,Y,Z);
			      	break;
			case GABEDIT_CUBE_GAUSS_LAP:
				grid = get_grid_from_gauss_molpro_cube_file(1,file,1,1,N,XYZ0,X,Y,Z);
			      	break;
			case GABEDIT_CUBE_GAUSS_NGRAD:
				grid = get_grid_from_gauss_molpro_cube_file(1,file,1,1,N,XYZ0,X,Y,Z);
			      	break;
			case GABEDIT_CUBE_GAUSS_POT:
				grid = get_grid_from_gauss_molpro_cube_file(1,file,1,1,N,XYZ0,X,Y,Z);
			      	break;
			case GABEDIT_CUBE_MOLPRO_ORB1:
				grid = get_grid_from_gauss_molpro_cube_file(0,file,1,1,N,XYZ0,X,Y,Z);
			      	break;
			case GABEDIT_CUBE_GABEDIT:
				grid = get_grid_from_gauss_molpro_cube_file(0,file,1,1,N,XYZ0,X,Y,Z);
			      	break;
			case GABEDIT_CUBE_MOLPRO_ORBN:
				grid = NULL;
				break;
			case GABEDIT_CUBE_MOLPRO_DEN:
				grid = get_grid_from_gauss_molpro_cube_file(1,file,1,1,N,XYZ0,X,Y,Z);
			      	break;
			case GABEDIT_CUBE_MOLPRO_DEN_GRAD:
				grid = get_grid_from_gauss_molpro_cube_file(1,file,1,4,N,XYZ0,X,Y,Z);
			      	break;
			case GABEDIT_CUBE_MOLPRO_LAPDEN:
				grid = get_grid_from_gauss_molpro_cube_file(1,file,1,5,N,XYZ0,X,Y,Z);
			      	break;
			case GABEDIT_CUBE_MOLPRO_LAPLAP:
				grid = get_grid_from_gauss_molpro_cube_file(1,file,5,5,N,XYZ0,X,Y,Z);
			      	break;
		}

	}
	if(grid)
	{
        	limits = grid->limits;
		create_iso_orbitals();
		set_status_label_info("Grid","Ok");
	}
	else
	{
		set_status_label_info("Grid","Nothing");
		CancelCalcul = FALSE;
	}

	fclose(file);
}
/********************************************************************************/
void load_cube_gauss_orbitals_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	TypeGrid = GABEDIT_TYPEGRID_ORBITAL;
	add_objects_for_new_grid();
	read_gauss_molpro_cube_file(GABEDIT_CUBE_GAUSS_ORB,FileName);
}
/********************************************************************************/
void load_cube_gauss_density_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	TypeGrid = GABEDIT_TYPEGRID_EDENSITY;
	add_objects_for_new_grid();
	read_gauss_molpro_cube_file(GABEDIT_CUBE_GAUSS_DEN,FileName);
}
/********************************************************************************/
void load_cube_gauss_gradient_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	TypeGrid = GABEDIT_TYPEGRID_EDENSITY;
	add_objects_for_new_grid();
	read_gauss_molpro_cube_file(GABEDIT_CUBE_GAUSS_GRAD,FileName);
}
/********************************************************************************/
void load_cube_gauss_laplacian_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	TypeGrid = GABEDIT_TYPEGRID_ORBITAL;
	add_objects_for_new_grid();
	read_gauss_molpro_cube_file(GABEDIT_CUBE_GAUSS_LAP,FileName);
}
/********************************************************************************/
void load_cube_gauss_normgrad_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	TypeGrid = GABEDIT_TYPEGRID_EDENSITY;
	add_objects_for_new_grid();
	read_gauss_molpro_cube_file(GABEDIT_CUBE_GAUSS_NGRAD,FileName);
}
/********************************************************************************/
void load_cube_gauss_potential_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	TypeGrid = GABEDIT_TYPEGRID_ORBITAL;
	add_objects_for_new_grid();
	read_gauss_molpro_cube_file(GABEDIT_CUBE_GAUSS_POT,FileName);
}
/********************************************************************************/
void load_cube_molpro_orbital_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	TypeGrid = GABEDIT_TYPEGRID_ORBITAL;
	add_objects_for_new_grid();
	read_gauss_molpro_cube_file(GABEDIT_CUBE_MOLPRO_ORB1,FileName);
}
/********************************************************************************/
void load_cube_molpro_orbitals_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	TypeGrid = GABEDIT_TYPEGRID_ORBITAL;
	add_objects_for_new_grid();
	read_gauss_molpro_cube_file(GABEDIT_CUBE_MOLPRO_ORBN,FileName);
}
/********************************************************************************/
void load_cube_molpro_density_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	TypeGrid = GABEDIT_TYPEGRID_EDENSITY;
	add_objects_for_new_grid();
	read_gauss_molpro_cube_file(GABEDIT_CUBE_MOLPRO_DEN,FileName);
}
/********************************************************************************/
void load_cube_molpro_density_gradient_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	TypeGrid = GABEDIT_TYPEGRID_EDENSITY;
	add_objects_for_new_grid();
	read_gauss_molpro_cube_file(GABEDIT_CUBE_MOLPRO_DEN_GRAD,FileName);
}
/********************************************************************************/
void load_cube_molpro_laplacian_density_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	TypeGrid = GABEDIT_TYPEGRID_EDENSITY;
	add_objects_for_new_grid();
	read_gauss_molpro_cube_file(GABEDIT_CUBE_MOLPRO_LAPDEN,FileName);
}
/********************************************************************************/
void load_cube_molpro_laplacian_laplacian_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	TypeGrid = GABEDIT_TYPEGRID_ORBITAL;
	add_objects_for_new_grid();
	read_gauss_molpro_cube_file(GABEDIT_CUBE_MOLPRO_LAPLAP,FileName);
}
/********************************************************************************/
void load_cube_gabedit_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	TypeGrid = GABEDIT_TYPEGRID_ORBITAL;
	add_objects_for_new_grid();
	read_gauss_molpro_cube_file(GABEDIT_CUBE_GABEDIT,FileName);
}
/********************************************************************************/
void subtract_cube(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(!grid)
	{
		Message(_("Sorry, you have not a default grid"),_("Error"),TRUE);
		return;
	}

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	TypeGrid = GABEDIT_TYPEGRID_ORBITAL;
	subtract_cube_file(FileName);
}
/********************************************************************************/
void mapping_cube(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(!grid)
	{
		Message(_("Sorry, you have not a default grid"),_("Error"),TRUE);
		return;
	}

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();

	if(TypeGrid != GABEDIT_TYPEGRID_SAS)
		TypeGrid = GABEDIT_TYPEGRID_ORBITAL;
	read_mapping_cube_file(FileName);
}
/********************************************************************************/
void save_cube_gabedit_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(!grid)
	{
		Message(_("Sorry, you have not a default grid"),_("Error"),TRUE);
		return;
	}

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();
	save_grid_gabedit_cube_file(FileName);
}
/********************************************************************************/
void mapping_with_mep(gint N[],GridLimits limits, PoissonSolverMethod psMethod)
{
	Grid* mep = NULL;
	GabEditTypeGrid oldTypeGrid = TypeGrid;

	if (psMethod==GABEDIT_EXACT)
		mep = compute_mep_grid_exact(N, limits);
	else if(psMethod != GABEDIT_UNK)
		mep = solve_poisson_equation_from_orbitals(N,limits, psMethod);
	else
		mep = compute_mep_grid_using_multipol_from_orbitals(N, limits, get_multipole_rank());

	TypeGrid = oldTypeGrid;
	if(!mep) return;
	mapping_cube_by_an_other_cube(mep);
	free_grid(mep);
}
/********************************************************************************/
void mapping_with_mep_from_multipol(gint lmax)
{
	Grid* mep = NULL;
	GabEditTypeGrid oldTypeGrid = TypeGrid;

	mep = compute_mep_grid_using_multipol_from_orbitals(grid->N, grid->limits, lmax);

	TypeGrid = oldTypeGrid;
	if(!mep) return;
	mapping_cube_by_an_other_cube(mep);
	free_grid(mep);
}
/********************************************************************************/
void mapping_with_mep_from_charges()
{
	Grid* mep = NULL;
	GabEditTypeGrid oldTypeGrid = TypeGrid;
	gdouble s= GetSumAbsCharges();

	if(s<1e-6) 
	{
		Message(_("Sorry, All partial charges are null\n"),_("Error"),TRUE);
		return;
	}

	mep = compute_mep_grid_using_partial_charges_cube_grid(grid);

	TypeGrid = oldTypeGrid;
	if(!mep) return;
	mapping_cube_by_an_other_cube(mep);
	free_grid(mep);
}
/********************************************************************************/
void mapping_with_fed(gint n)
{
	Grid* mep = NULL;
	GabEditTypeGrid oldTypeGrid = TypeGrid;

	if(n==0) TypeGrid = GABEDIT_TYPEGRID_FEDELECTROPHILIC;
	else if(n==2) TypeGrid = GABEDIT_TYPEGRID_FEDNUCLEOPHILIC;
	else TypeGrid = GABEDIT_TYPEGRID_FEDRADICAL;

	mep = compute_fed_grid_using_cube_grid(grid, n);

	TypeGrid = oldTypeGrid;
	if(!mep) return;
	mapping_cube_by_an_other_cube(mep);
	free_grid(mep);
}
/********************************************************************************/
static Grid* get_grid_from_dx_file(FILE* file, gint N[], gdouble XYZ0[3],gdouble X[3],gdouble Y[3],gdouble Z[3])
{
	Grid* grid;
	gint i;
	gint j;
	gint k;
	gdouble x;
	gdouble y;
	gdouble z;
	gdouble v;
    	gboolean beg = TRUE;
	gdouble scal;
	GridLimits limits;
	gboolean Ok = FALSE;
	gchar t[BSIZE];
	gint len = BSIZE;

	while(!Ok)
	{
		if(!fgets(t,len,file)) return NULL;
		if(strstr(t,"class array"))
		{
			Ok = TRUE;
			break;
		}
	}
	if(!Ok) return NULL;

  	for(i=0;i<3;i++)
   		limits.MinMax[0][i] = XYZ0[i];

	limits.MinMax[1][0] = XYZ0[0] + (N[0]-1)*X[0] + (N[1]-1)*X[1] +  (N[2]-1)*X[2];
	limits.MinMax[1][1] = XYZ0[1] + (N[0]-1)*Y[0] + (N[1]-1)*Y[1] +  (N[2]-1)*Y[2];
	limits.MinMax[1][2] = XYZ0[2] + (N[0]-1)*Z[0] + (N[1]-1)*Z[1] +  (N[2]-1)*Z[2];

	grid = grid_point_alloc(N,limits);

	progress_orb(0,GABEDIT_PROGORB_READGRID,TRUE);
	scal = (gdouble)1.01/grid->N[0];
 
	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				while(!feof(file))
				{
				   if(fgets(t,len,file)) 
					   if(1==sscanf(t,"%lf", &v))break;
				}
				x = XYZ0[0] + i*X[0] + j*X[1] +  k*X[2]; 
				y = XYZ0[1] + i*Y[0] + j*Y[1] +  k*Y[2]; 
				z = XYZ0[2] + i*Z[0] + j*Z[1] +  k*Z[2]; 

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
	progress_orb(0,GABEDIT_PROGORB_READGRID,TRUE);
	return grid;
}
/**************************************************************/
gboolean read_dx_grid_file(gchar* filename, gboolean showisowin)
{
	FILE* file = FOpen(filename, "rb");
	gchar* tmp;
	gint len = BSIZE;
	gchar t[BSIZE];
	gdouble XYZ0[3];
	gint N[3];
	gdouble X[3];
	gdouble Y[3];
	gdouble Z[3];
	gboolean Ok = FALSE;
	gchar* p;
	gchar dum[100];

	CancelCalcul = FALSE;
	
	if(!file)
	{
		Message(_("I can not open file\n"),_("Error"),TRUE);
		return FALSE;
	}

	free_data_all();
	tmp = get_name_file(filename);
	set_status_label_info(_("File Name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"dx grid");

	while(!Ok)
	{
		if(!fgets(t,len,file))
		{
			Message(_("I can not read grid from this file\n"),_("Error"),TRUE);
			set_status_label_info(_("File name"),_("Nothing"));
			set_status_label_info(_("File type"),_("Nothing"));
			fclose(file);
			return FALSE;
		}
		if(strstr(t,"gridpositions counts"))
		{
			Ok = TRUE;
			break;
		}
	}
	if(!Ok)
	{
		Message(_("I can not read grid from this file\n"),_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		fclose(file);
		return FALSE;
	}
	p = strstr(t,"gridpositions counts")+strlen("gridpositions counts");

	if(3!=sscanf(p,"%d %d %d",&N[0], &N[1], &N[2]))
	{
		Message(_("I can not read cube from this file\n"),_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		fclose(file);
		return FALSE;
	}
	if(!fgets(t,len,file) || sscanf(t,"%s %lf %lf %lf",dum,&XYZ0[0],&XYZ0[1],&XYZ0[2])!=4)
	{
		Message(_("I can not read grid from this file\n"),_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		fclose(file);
		return FALSE;
	}
	if(!fgets(t,len,file) || sscanf(t,"%s %lf %lf %lf",dum,&X[0],&X[1],&X[2])!=4)
	{
		Message(_("I can not read grid from this file\n"),_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		fclose(file);
		return FALSE;
	}
	if(!fgets(t,len,file) || sscanf(t,"%s %lf %lf %lf",dum,&Y[0],&Y[1],&Y[2])!=4)
	{
		Message(_("I can not read grid from this file\n"),_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		fclose(file);
		return FALSE;
	}
	if(!fgets(t,len,file) || sscanf(t,"%s %lf %lf %lf",dum,&Z[0],&Z[1],&Z[2])!=4)
	{
		Message(_("I can not read grid from this file\n"),_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		fclose(file);
		return FALSE;
	}

	grid = get_grid_from_dx_file(file,N,XYZ0,X,Y,Z);
	if(grid)
	{
        	limits = grid->limits;
		if(showisowin) create_iso_orbitals();
		set_status_label_info("Grid","Ok");
	}
	else
	{
		set_status_label_info("Grid","Nothing");
		CancelCalcul = FALSE;
	}

	fclose(file);
	return (grid!=NULL);
}
/********************************************************************************/
void load_dx_grid_file(GabeditFileChooser *selFile, gint response_id)
{
 	gchar *fileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(selFile);
	gtk_widget_hide(GTK_WIDGET(selFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	TypeGrid = GABEDIT_TYPEGRID_ORBITAL;
	add_objects_for_new_grid();
	read_dx_grid_file(fileName,TRUE);
}
/********************************************************************************/
void compute_integral_all_space()
{
	gdouble integ;
	gchar* result = NULL;

	if(compute_integrale_from_grid_all_space(grid,&integ))
	result = g_strdup_printf("Integral = %0.12lf",integ);
	else
	result = g_strdup_printf("Canceled? !\n If not see your terminal ");

	if(result && !CancelCalcul)
	{
		GtkWidget* message = MessageTxt(result,_("Result"));
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		gtk_window_set_transient_for(GTK_WINDOW(message),GTK_WINDOW(PrincipalWindow));
	}
	if(CancelCalcul) CancelCalcul = FALSE;
}
/********************************************************************************/
void cut_cube(gint dir, gboolean left)
{
	gint i, j, k;
	gboolean beg = TRUE;
	gdouble v;
	gdouble scal;
	gint N[3];
	Grid* newGrid;
	GridLimits newLimits;
	gint c;
	gint ii, jj, kk;
	gint min[3];
	gint max[3];

	if(grid->N[0]<3) return;
	if(grid->N[1]<3) return;
	if(grid->N[2]<3) return;

	for(i=0;i<=2;i++) N[i] = grid->N[i];
	N[dir] = (grid->N[dir]+1)/2;

	newLimits = grid->limits;

	for(i=0;i<=2;i++)
	{
        	min[i] = 0;
		max[i] = N[i]-1;
		if(left && dir==i)
		{
        		min[i] = N[i]-1;
			if(grid->N[i]%2==0) min[i]++;
			max[i] = grid->N[i]-1;
		}
	}

  	for(c=0;c<3;c++) newLimits.MinMax[0][c] = grid->point[min[0]][min[1]][min[2]].C[c];
  	for(c=0;c<3;c++) newLimits.MinMax[1][c] = grid->point[max[0]][max[1]][max[2]].C[c];

	newGrid = grid_point_alloc(N,newLimits);

	progress_orb(0,GABEDIT_PROGORB_SCALEGRID,TRUE);


	scal = (gdouble)1.01/(max[0]-min[0]+1);
	for(i=min[0];i<=max[0];i++)
	{
		ii=i-min[0];
		for(j=min[1];j<=max[1];j++)
		{
			jj=j-min[1];
			for(k=min[2];k<=max[2];k++)
			{
				kk=k-min[2];
				for( c=0;c<4;c++) newGrid->point[ii][jj][kk].C[c] = grid->point[i][j][k].C[c];
				v = grid->point[i][j][k].C[3];
				if(beg)
				{
					beg = FALSE;
        				newGrid->limits.MinMax[0][3] =  v;
        				newGrid->limits.MinMax[1][3] =  v;
				}
                		else
				{
        				if(newGrid->limits.MinMax[0][3]>v)
        					newGrid->limits.MinMax[0][3] =  v;
        				if(newGrid->limits.MinMax[1][3]<v)
        					newGrid->limits.MinMax[1][3] =  v;
				}
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_SCALEGRID,TRUE);
			break;
		}
		progress_orb(scal,GABEDIT_PROGORB_SCALEGRID,FALSE);
	}
	progress_orb(0,GABEDIT_PROGORB_SCALEGRID,TRUE);
	if(CancelCalcul) newGrid = free_grid(newGrid);
	else
	{
		grid = free_grid(grid);
		grid = newGrid;
		limits = grid->limits;
	}
}
/********************************************************************************/
void applyCutLeft0(GtkWidget *Win,gpointer data)
{
	cut_cube(0,TRUE);
	create_iso_orbitals();
	glarea_rafresh(GLArea);
}
/********************************************************************************/
void applyCutRight0(GtkWidget *Win,gpointer data)
{
	cut_cube(0,FALSE);
	create_iso_orbitals();
	glarea_rafresh(GLArea);
}
/********************************************************************************/
void applyCutLeft1(GtkWidget *Win,gpointer data)
{
	cut_cube(1,TRUE);
	create_iso_orbitals();
	glarea_rafresh(GLArea);
}
/********************************************************************************/
void applyCutRight1(GtkWidget *Win,gpointer data)
{
	cut_cube(1,FALSE);
	create_iso_orbitals();
	glarea_rafresh(GLArea);
}
/********************************************************************************/
void applyCutLeft2(GtkWidget *Win,gpointer data)
{
	cut_cube(2,TRUE);
	create_iso_orbitals();
	glarea_rafresh(GLArea);
}
/********************************************************************************/
void applyCutRight2(GtkWidget *Win,gpointer data)
{
	cut_cube(2,FALSE);
	create_iso_orbitals();
	glarea_rafresh(GLArea);
}
/********************************************************************************/
