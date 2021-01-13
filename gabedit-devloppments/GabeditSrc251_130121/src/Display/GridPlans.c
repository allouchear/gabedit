/* GridPlans.c */
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
#include <gtk/gtk.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "../Display/GlobalOrb.h"
#include "../Display/Orbitals.h"
#include "../Display/UtilsOrb.h"
#include "../Utils/UtilsInterface.h"
#include "../Display/GridPlans.h"
#include "../Display/Contours.h"
#include "../Display/PlanesMapped.h"
#include "../Utils/Utils.h"
#include "../Display/StatusOrb.h"
#include "../Display/GLArea.h"

/**************************************************************/
Plane get_plane(Point5 C, Point5 V, gdouble *len, gint *N)
{
	Plane plane;
	gint i;

	for(i=0;i<3;i++)
	{
		plane.Center.C[i] = C.C[i];
		plane.Vector.C[i] = V.C[i];
	}
	plane.len[0] = len[0];
	plane.len[1] = len[1];

	plane.N[0] = N[0];
	plane.N[1] = N[1];

	plane.point = g_malloc( plane.N[0]*sizeof(Point5*));
	for(i=0;i< plane.N[0] ;i++)
		plane.point[i] = g_malloc(plane.N[1]*sizeof(Point5));

	return plane;
	
}
/**************************************************************/
Plane* free_plane(Plane *plane)
{
	gint i;

	for(i=0;i< plane->N[0] ;i++)
		if(plane->point[i])
			g_free(plane->point[i]);
	if(plane->point)
		g_free(plane->point);
	plane->point = NULL;

	return plane;
	
}
/**************************************************************/
void set_vector_plane(Plane *plane)
{
	gint i;
	gint j;
	gdouble n;
	/* Normalize Vector */
	n = 0.0;
	for(i=0;i<3;i++)
		n += plane->Vector.C[i]*plane->Vector.C[i];
	n = sqrt(n);
	for(i=0;i<3;i++)
		plane->Vector.C[i] /= n;
	j = 0;
	for(i=1;i<3;i++)
		if(plane->Vector.C[i]>plane->Vector.C[j])
			j = i;

	plane->V[0].C[j] = 0.0;
	plane->V[0].C[(j+1)%3] = 1;
	plane->V[0].C[(j+2)%3] = 1;

	n = 0.0;
	for(i=0;i<3;i++)
	n += 	plane->Vector.C[i]*plane->V[0].C[i];

	if(plane->Vector.C[j] != 0)
	plane->V[0].C[j] = -n/plane->Vector.C[j];
	else
	{
		for(i=0;i<3;i++)
			plane->V[0].C[i] = 1;
		plane->V[0].C[(j+1)%3] = 1.0;
	}
	/*Debug("Vector plane = %lf %lf %lf \n", plane->V[0].C[0], plane->V[0].C[1] , plane->V[0].C[2]  );*/
	
	n = 0.0;
	for(i=0;i<3;i++)
		n += plane->V[0].C[i]*plane->V[0].C[i];
	n = sqrt(n);
	for(i=0;i<3;i++)
		plane->V[0].C[i] /= n;
	for(i=0;i<3;i++)
		plane->V[1].C[i] = plane->Vector.C[(i+1)%3]*plane->V[0].C[(i+2)%3]- plane->Vector.C[(i+2)%3]*plane->V[0].C[(i+1)%3];
}
/**************************************************************/
void set_points_plane(Plane *plane)
{
	gint i;
	gint j;
	gint k;
	gdouble step0 = plane->len[0]/(plane->N[0]-1);
	gdouble step1 = plane->len[1]/(plane->N[1]-1);
	gdouble pos0 = -plane->len[0]/2;
	gdouble pos1 = -plane->len[1]/2;

	for(i=0;i<plane->N[0];i++)
		for(j=0;j<plane->N[1];j++)
		{
			for(k=0;k<3;k++)
				plane->point[i][j].C[k] = plane->Center.C[k]
					+plane->V[0].C[k]*(pos0+i*step0)
					+plane->V[1].C[k]*(pos1+j*step1);
		}
	plane->limits[0][0] =  plane->point[0][0].C[0]; /* xmin */
	plane->limits[0][1] =  plane->point[0][0].C[0]; /* xmax */
	plane->limits[1][0] =  plane->point[0][0].C[0]; /* ymin */
	plane->limits[1][1] =  plane->point[0][0].C[0]; /* ymax */
	plane->limits[2][0] =  plane->point[0][0].C[0]; /* zmin */
	plane->limits[2][1] =  plane->point[0][0].C[0]; /* zmax */
	for(i=0;i<plane->N[0];i++)
		for(j=0;j<plane->N[1];j++)
			for(k=0;k<3;k++)
			{
				if(plane->limits[k][0]> plane->point[i][j].C[0] )
					plane->limits[k][0] =  plane->point[i][j].C[0];
				if(plane->limits[k][1]< plane->point[i][j].C[0] )
					plane->limits[k][1] =  plane->point[i][j].C[0];
			}
}
/**************************************************************/
void print_vector_plane(Plane *plane)
{
	gint i;
	gint j;
	gint k;
	printf("Plane properties\n");
	printf("Center :");
	for(i=0;i<3;i++)
		printf(" %lf ",plane->Center.C[i]);
	printf("\n ");

	printf("Vector :");
	for(i=0;i<3;i++)
		printf(" %lf ",plane->Vector.C[i]);
	printf("\n ");

	printf("V0 :");
	for(i=0;i<3;i++)
		printf(" %lf ",plane->V[0].C[i]);
	printf("\n ");
	printf("V1 :");
	for(i=0;i<3;i++)
		printf(" %lf ",plane->V[1].C[i]);
	printf("\n ");
	printf("Grid :\n");
	for(i=0;i<plane->N[0];i++)
		for(j=0;j<plane->N[1];j++)
		{
			printf(" %d %d  : ",i,j);
			for(k=0;k<3;k++)
				printf(" %lf ",plane->point[i][j].C[k]); 
			printf("\n ");
		}
	printf("Xlimits : %lf %lf\n",plane->limits[0][0],plane->limits[0][1]);
	printf("Ylimits : %lf %lf\n",plane->limits[1][0],plane->limits[1][1]);
	printf("Zlimits : %lf %lf\n",plane->limits[2][0],plane->limits[2][1]);
}
/**************************************************************/
Grid* plane_grid_point_alloc(Plane *plane,GridLimits limits)
{
	Grid* planegrid = g_malloc(sizeof(Grid));
	gint i,j;
  	
	planegrid->N[0] = plane->N[0];
	planegrid->N[1] = plane->N[1];
	planegrid->N[2] = 1;
	planegrid->point = g_malloc( planegrid->N[0]*sizeof(Point5**));
	for(i=0;i< planegrid->N[0] ;i++)
	{
		planegrid->point[i] = g_malloc(planegrid->N[1]*sizeof(Point5*));
		for(j=0;j< planegrid->N[1] ;j++)
			planegrid->point[i][j] = g_malloc(planegrid->N[2]*sizeof(Point5));
	}

	for(i=0;i<2;i++)
		for(j=0;j<3;j++)
			planegrid->limits.MinMax[i][j] = limits.MinMax[i][j];
	return planegrid;
}
/**************************************************************/
Grid* grid_point_free(Grid* planegrid)
{
	gint i,j;
	if(!planegrid)
		return NULL;
	for(i=0;i< planegrid->N[0] ;i++)
	{
		for(j=0;j< planegrid->N[1] ;j++)
			g_free(planegrid->point[i][j]);
		g_free(planegrid->point[i]);
	}
	g_free(planegrid->point);
	g_free(planegrid);
	planegrid=NULL;
	return planegrid;
}
/**************************************************************/
Grid* define_planegrid_point(Plane *plane,Func3d func)
{
	Grid* planegrid;
	GridLimits limits;
	gint i;
	gint j;
	gint k;
	gdouble x;
	gdouble y;
	gdouble z;
	gdouble v;
    gboolean beg = TRUE;

	limits.MinMax[0][0] = plane->limits[0][0];
	limits.MinMax[1][0] = plane->limits[0][1];
	limits.MinMax[0][1] = plane->limits[1][0];
	limits.MinMax[1][1] = plane->limits[1][1];
	limits.MinMax[0][2] = plane->limits[2][0];
	limits.MinMax[1][2] = plane->limits[2][1];

	planegrid = plane_grid_point_alloc(plane,limits);

	
	for(i=0;i<planegrid->N[0];i++)
	{
		for(j=0;j<planegrid->N[1];j++)
		{
			x = plane->point[i][j].C[0];
			y = plane->point[i][j].C[1];
			z = plane->point[i][j].C[2];
			for(k=0;k<planegrid->N[2];k++)
			{
				v = func( x, y, z,NumSelOrb);
	
				planegrid->point[i][j][k].C[0] = x;
				planegrid->point[i][j][k].C[1] = y;
				planegrid->point[i][j][k].C[2] = z;
				planegrid->point[i][j][k].C[3] = v;
				if(beg)
				{
					beg = FALSE;
        			planegrid->limits.MinMax[0][2] =  z;
        			planegrid->limits.MinMax[1][2] =  z;
        			planegrid->limits.MinMax[0][3] =  v;
        			planegrid->limits.MinMax[1][3] =  v;
				}
                else
				{
        			if(planegrid->limits.MinMax[0][2]>z)
        				planegrid->limits.MinMax[0][2] =  z;
        			if(planegrid->limits.MinMax[1][2]<z)
        				planegrid->limits.MinMax[1][2] =  z;
        			if(planegrid->limits.MinMax[0][3]>v)
        				planegrid->limits.MinMax[0][3] =  v;
        			if(planegrid->limits.MinMax[1][3]<v)
        				planegrid->limits.MinMax[1][3] =  v;
				}
			}
		}
	}

	/* Debug("Vlimits = %lf %lf \n", planegrid->limits.MinMax[0][3] , planegrid->limits.MinMax[1][3] ); */
	return planegrid;
}
/**************************************************************/
Grid* define_plane_grid(Point5 C, Point5 V, gdouble *len, gint *N)
{
	Grid* planegrid;
	Plane plane;
  	if(!GeomOrb)
  	{
	  	Message(_("Sorry, Please load a file beforee\n"),_("Error"),TRUE);
	  	return NULL;
  	}
  	if(!CoefAlphaOrbitals)
  	{
	  	Message(_("Sorry, Please load the MO beforee\n"),_("Error"),TRUE);
	  	return NULL;
  	}

  	if(!AOAvailable &&(TypeGrid == GABEDIT_TYPEGRID_DDENSITY || TypeGrid == GABEDIT_TYPEGRID_ADENSITY))
  	{
	  	Message(_("Sorry, No atomic orbitals available.\nPlease use a gabedit file for load : \n"
		  "Geometry, Molecular and Atomic Orbitals\n"),_("Error"),TRUE);
	  	return NULL;
  	}

	plane =  get_plane(C,V,len,N);
 	set_vector_plane(&plane);
 	set_points_plane(&plane);
 	/*print_vector_plane(&plane);*/

	set_status_label_info("Grid","Computing for contours");

	switch(TypeGrid)
	{
		case GABEDIT_TYPEGRID_ORBITAL :
			planegrid = define_planegrid_point(&plane,get_value_orbital);
			break;
		case GABEDIT_TYPEGRID_EDENSITY :
			planegrid = define_planegrid_point(&plane,get_value_electronic_density);
			break;
		case GABEDIT_TYPEGRID_DDENSITY :
			planegrid = define_planegrid_point(&plane,get_value_electronic_density_bonds);
			break;
		case GABEDIT_TYPEGRID_ADENSITY :
			planegrid = define_planegrid_point(&plane,get_value_electronic_density_atomic);
			break;
		case GABEDIT_TYPEGRID_SDENSITY :
			planegrid = define_planegrid_point(&plane,get_value_spin_density);
			break;
		default : return NULL;

	}
 	/*print_grid_point(planegrid);*/
	set_status_label_info("Grid","Ok");
	free_plane(&plane);
	return planegrid;
}
/********************************************************************************/
void apply_planegrid_center_vector(GtkWidget *Win,gpointer data)
{
			
	GtkWidget** Entries =(GtkWidget**)g_object_get_data(G_OBJECT (Win), "Entries");
  	gchar* type = g_object_get_data(G_OBJECT (Win), "Type");
	gint i;
	gint N[2];
	gdouble len[2];
	Point5 C;
	Point5 V;
	
	N[0] = get_number_of_point(Entries[8]);
	if(N[0]<=0) return;
	N[1] = get_number_of_point(Entries[9]);
	if(N[1]<=0) return;
	if(!get_a_float(Entries[6],&len[0],_("Error : The length of plane should be a float."))) return;
	if(len[0]<=0)
	{
		Message(_("Error : The length of plane should be positive. "),_("Error"),TRUE);
		return;
	}
	if(!get_a_float(Entries[7],&len[1],_(" Error : The length of plane should be a float."))) return;
	if(len[1]<=0)
	{
		Message(_("Error : The length of plane should be positive. "),_("Error"),TRUE);
		return;
	}
	for(i=0;i<5;i+=2)
	{	
        	if(!get_a_float(Entries[i],&C.C[i/2],_("Error : A non float value in a float area."))) return;
	}
	for(i=1;i<6;i+=2)
	{	
        	if(!get_a_float(Entries[i],&V.C[i/2],_("Error : A non float value in a float area."))) return;
	}
	if( sqrt(V.C[0] * V.C[0] + V.C[1] * V.C[1] + V.C[2] * V.C[2])<1e-6)
	{
		Message(_("Error : The length of normal Vector should be > 1e-6. "),_("Error"),TRUE);
		return;
	}
	/* Debug("V = %lf %lf %lf \n",V.C[0] , V.C[1] , V.C[2]); */
	if(type)
	{
		if(strstr(type,"Maps"))
		{
			gridPlaneForPlanesMapped = define_plane_grid(C,V,len,N);
			create_maps_plane(_("Definition of Maps"));
			reDrawPlaneMappedPlane = FALSE;
		}
		else
		{
			gridPlaneForContours = define_plane_grid(C,V,len,N);
			create_contours_plane(_("Definition of contours"));
			reDrawContoursPlane = FALSE;
		}
	}
  	delete_child(Win);
}
/********************************************************************************/
GtkWidget *create_planegrid_frame_center_vector( GtkWidget *vboxall,gchar* title)
{
#define NLIGNES   7
#define NCOLUMNS  4
	GtkWidget *frame;
	GtkWidget *Label;
	GtkWidget *vboxframe;
	GtkWidget *hseparator;
	GtkWidget **Entries = (GtkWidget **)g_malloc(10*sizeof(GtkWidget *));
	gushort i;
	gushort j;
	gushort k;
	GtkWidget *Table;
	gchar     *strlabels[NLIGNES][NCOLUMNS];
	
	strlabels[0][0] = g_strdup("  ");
	strlabels[0][1] = g_strdup("  ");
	strlabels[0][2] = g_strdup(_("Center of plan"));
	strlabels[0][3] = g_strdup(_("Vector perpendicular to plan"));

	strlabels[1][0] = g_strdup(" X ");
	strlabels[1][1] = g_strdup(" : ");
	strlabels[1][2] = g_strdup("0.0");
	strlabels[1][3] = g_strdup("1.0");

	strlabels[2][0] = g_strdup(" Y ");
	strlabels[2][1] = g_strdup(" : ");
	strlabels[2][2] = g_strdup("0.0");
	strlabels[2][3] = g_strdup("1.0");

	strlabels[3][0] = g_strdup(" Z ");
	strlabels[3][1] = g_strdup(" : ");
	strlabels[3][2] = g_strdup("0.0");
	strlabels[3][3] = g_strdup("1.0");

	strlabels[4][0] = g_strdup("  ");
	strlabels[4][1] = g_strdup("  ");
	strlabels[4][2] = g_strdup(_("First  direction"));
	strlabels[4][3] = g_strdup(_("Second direction"));

	strlabels[5][0] = g_strdup(_(" Length "));
	strlabels[5][1] = g_strdup(" : ");
	strlabels[5][2] = g_strdup("10.0");
	strlabels[5][3] = g_strdup("10.0");

	strlabels[6][0] = g_strdup(_(" N points "));
	strlabels[6][1] = g_strdup(" : ");
	strlabels[6][2] = g_strdup("40");
	strlabels[6][3] = g_strdup("40");


	frame = gtk_frame_new (title);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);

	Table = gtk_table_new(NLIGNES+1,NCOLUMNS,FALSE);
	gtk_box_pack_start (GTK_BOX(vboxframe), Table, TRUE, TRUE, 1); 

	for(i=0;i<4;i++)
		for(j=0;j<NCOLUMNS;j++)
			if(i<1 || j<2)
			{
				Label =	add_label_at_table(Table,strlabels[i][j],i,(gushort)j,GTK_JUSTIFY_LEFT);
			}
	k = 0;
	for(i=1;i<4;i++)
		for(j=2;j<NCOLUMNS;j++)
			{
				Entries[k] = gtk_entry_new ();
				add_widget_table(Table,Entries[k],(gushort)i,(gushort)j);
				gtk_entry_set_text(GTK_ENTRY(Entries[k]),strlabels[i][j]);
				k++;
			}
	hseparator = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(Table),hseparator,0,4,4,5,
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  3,3);

	for(i=5;i<NLIGNES+1;i++)
		for(j=0;j<NCOLUMNS;j++)
			if(i<6 || j<2)
			{
				Label =	add_label_at_table(Table,strlabels[i-1][j],(gushort)i,(gushort)j,GTK_JUSTIFY_LEFT);
			}
	
	for(i=6;i<NLIGNES+1;i++)
		for(j=2;j<NCOLUMNS;j++)
			{
				Entries[k] = gtk_entry_new ();
				add_widget_table(Table,Entries[k],(gushort)i,(gushort)j);
				gtk_entry_set_text(GTK_ENTRY(Entries[k]),strlabels[i-1][j]);
				k++;
			}
	
	g_object_set_data(G_OBJECT (frame), "Entries",Entries);
	for(i=0;i<NLIGNES;i++)
	{
		for(j=0;j<NCOLUMNS;j++)
		{
			g_free(strlabels[i][j]);
		}
	}
	gtk_widget_show_all(frame);
  
  	return frame;
}
/********************************************************************************/
void create_planeGrid_center_vector(gchar* type)
{
  GtkWidget *Win;
  GtkWidget *frame;
  GtkWidget *hbox;
  GtkWidget *vboxall;
  GtkWidget *vboxwin;
  GtkWidget *button;
  GtkWidget** Entries;
  gchar* planeFor[] ={_("Contours"),_("Maps")};
  gint itype = 0;

  if(!GeomOrb)
  {
  	Message(_("Sorry, Please load a file beforee\n"),_("Error"),TRUE);
  	return;
 }
 if(!CoefAlphaOrbitals)
 {
  	Message(_("Sorry, Please load the MO beforee\n"),_("Error"),TRUE);
  	return;
 }
 if(strstr(type,planeFor[1])) itype = 1;


  /* Principal Window */
  Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(Win),_("Definition of plane by a Point and a Vector"));
  gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
  gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
  gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
  gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

  add_glarea_child(Win,_("Plane of contours "));

  vboxall = create_vbox(Win);
  vboxwin = vboxall;

  frame = create_planegrid_frame_center_vector(vboxall,_("Definition of plan"));
  Entries = (GtkWidget**) g_object_get_data(G_OBJECT (frame), "Entries");
  g_object_set_data(G_OBJECT (Win), "Entries",Entries);

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
  g_object_set_data(G_OBJECT (Win), "Type",planeFor[itype]);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_planegrid_center_vector,GTK_OBJECT(Win));


  /* Show all */
  gtk_widget_show_all (Win);
}
/********************************************************************************/
