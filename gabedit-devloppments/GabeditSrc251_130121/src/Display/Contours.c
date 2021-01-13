/* Contours.c */
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
#include "GlobalOrb.h"
#include "Orbitals.h"
#include "GLArea.h"
#include "UtilsOrb.h"
#include "../Utils/UtilsInterface.h"
#include "Contours.h"
#include "../Utils/Utils.h"

static void reset_limits_values(GtkWidget *Win,gpointer data);
static gboolean linear = TRUE;

/**************************************************************/
PointsContour** contour_point_alloc(gint N[],gint i0, gint i1)
{
	PointsContour** points;
	gint i;
	gint j;
  	
	points = g_malloc(N[i0]*sizeof(PointsContour*));
	for(i=0;i< N[i0] ;i++)
	{
		points[i] = g_malloc(N[i1]*sizeof(PointsContour));
		for( j = 0; j<N[i1];j++)
		{
			points[i][j].N = 0;
			points[i][j].point = NULL;
		}
	}
		
	return points;
}
/**************************************************************/
void contour_point_free(Contours contours)
{
	gint i;
	gint j;
  	
	if(!contours.pointscontour) return;
	for(i=0;i<contours.N[0];i++)
	{

		if(contours.pointscontour[i])
		{
			for( j = 0; j<contours.N[1];j++)
			{
				/* printf("N = %d \n",contours.pointscontour[i][j].N);*/
				if(contours.pointscontour[i][j].N>0 && contours.pointscontour[i][j].point) 
					g_free(contours.pointscontour[i][j].point);
			}
			g_free(contours.pointscontour[i]);
		}
	}
	g_free(contours.pointscontour);
		
}
/**************************************************************/
void set_contour_point(PointsContour** pointscontour,Grid* plansgrid,gdouble value,gint i0,gint i1,gint numplan)
{
	gint i;
	gint j;
	gint k;
	gint c;
	Point5 t[4];
	gdouble u1;
	gdouble u2;
	gdouble v1;
	gdouble v2;
	gint ip = numplan;
	gint ix=0,iy=0,iz=0;
	gint ix1=0,iy1=0,iz1=0;
	gint ix2=0,iy2=0,iz2=0;
	gint ix3=0,iy3=0,iz3=0;
	gint ix4=0,iy4=0,iz4=0;

	for( i = 0; i<plansgrid->N[i0]-1;i++)
	{
		for( j = 0; j<plansgrid->N[i1]-1;j++)
		{
			switch(i0)
			{
			case 0: 
				ix = i;
				ix1 = ix2 = ix + 1;
				ix4 = ix3 = ix;
				switch(i1)
				{
				case 1 : iy = j; iz = ip; 
					 iy1 = iy4 =  iy; iy2 = iy3 = iy+1 ;
					 iz1 = iz2 = iz3 = iz4 = iz;
					 break;
				case 2 : iy = ip;iz = j;
					 iy1 = iy2 = iy3 = iy4 = iy;
					 iz1 = iz4 = iz; iz2 = iz3 =  iz+1 ; 
					 break;
				}
				break;
			case 1: iy = i;
				iy1 = iy2 =  iy + 1;
				iy3 = iy4 = iy ;
				switch(i1)
				{
				case 0 : ix = j; iz = ip;
					 iz1 = iz2 = iz3 = iz4 = iz;
					 ix1 = ix4 =  ix; ix2 = ix3 = ix+1 ;
					 break;
				case 2 : ix = ip;iz = j;
					 ix1 = ix2 = ix3 = ix4 = ix;
					 iz1 = iz4 =  iz; iz2 = iz3 = iz+1 ;
					 break;
				}
				break;
			case 2: iz = i;
				iz1 = iz2 =  iz + 1;
				iz3 = iz4 = iz ;
				switch(i1)
				{
				case 0 : ix = j; iy = ip;
					 iy1 = iy2 = iy3 = iy4 = iy;
					 ix1 = ix4 =  ix; ix2 = ix3 = ix+1 ;
					 break;
				case 1 : ix = ip;iy = j;
					 ix1 = ix2 = ix3 = ix4 = ix;
					 iy1 = iy4 =  iy; iy2 = iy3 = iy+1 ;
					 break;
				}
				break;
			}
        
			v1 = plansgrid->point[ix][iy][iz].C[3]-value,
			v2 = plansgrid->point[ix1][iy1][iz1].C[3]-value;

			k = -1;
			if( v1*v2 <0 )
			{
				k++;
				for(c=0;c<3;c++)
				{
					u1 = plansgrid->point[ix][iy][iz].C[c];
					u2 = plansgrid->point[ix1][iy1][iz1].C[c];
					if(v1==0)
					 	t[k].C[c] = u1;
					else
						t[k].C[c] = u1 - v1 *(u2-u1)/(v2-v1);
				}
				t[k].C[3] =value;
			}
			v1 = v2;
			v2 = plansgrid->point[ix2][iy2][iz2].C[3]-value;
			if( v1*v2 <0 )
			{
				k++;
				for(c=0;c<3;c++)
				{
					u1 = plansgrid->point[ix1][iy1][iz1].C[c];
					u2 = plansgrid->point[ix2][iy2][iz2].C[c];
					if(v1==0)
					 	t[k].C[c] = u1;
					else
						t[k].C[c] = u1 - v1 *(u2-u1)/(v2-v1);
				}
				t[k].C[3] =value;
			}
			v1 = v2;
			v2 = plansgrid->point[ix3][iy3][iz3].C[3]-value;
			if( v1*v2 <0 )
			{
				k++;
				for(c=0;c<3;c++)
				{
					u1 = plansgrid->point[ix2][iy2][iz2].C[c];
					u2 = plansgrid->point[ix3][iy3][iz3].C[c];
					if(v1==0)
					 	t[k].C[c] = u1;
					else
						t[k].C[c] = u1 - v1 *(u2-u1)/(v2-v1);
				}
				t[k].C[3] =value;
			}
			v1 = v2;
			v2 = plansgrid->point[ix4][iy4][iz4].C[3]-value;
			if( v1*v2 <0 )
			{
				k++;
				for(c=0;c<3;c++)
				{
					u1 = plansgrid->point[ix3][iy3][iz3].C[c];
					u2 = plansgrid->point[ix4][iy4][iz4].C[c];
					if(v1==0)
					 	t[k].C[c] = u1;
					else
						t[k].C[c] = u1 - v1 *(u2-u1)/(v2-v1);
				}
				t[k].C[3] =value;
			}
			pointscontour[i][j].N = k+1;
			pointscontour[i][j].point = NULL;
			if(pointscontour[i][j].N>0)
			{
				pointscontour[i][j].point = g_malloc(pointscontour[i][j].N*sizeof(Point5));
				for(k=0;k<pointscontour[i][j].N;k++)
				{
					for(c=0;c<4;c++)
				   		pointscontour[i][j].point[k].C[c] = t[k].C[c];
				}
			}
		}
	}

}
/**************************************************************/
Contours get_contours(Grid* plansgrid,gdouble value,gint i0,gint i1,gint numplan)
{
	Contours contours;
	PointsContour** pointscontour = contour_point_alloc(plansgrid->N,i0,i1);
	set_contour_point(pointscontour,plansgrid,value,i0,i1,numplan);
	contours.N[0] = plansgrid->N[i0];
	contours.N[1] = plansgrid->N[i1];
	contours.pointscontour = pointscontour;
	return contours;
}
/********************************************************************************/
void create_contours_surface_one_plane(gint type, gdouble value)
{
	gint i0 = 0;
	gint i1 = 1;
	gdouble gap = 0;
	gint numplane = 0;

	switch(type)
	{
		case 0 : i0 = 1;i1 = 2;break; /* plane YZ */
		case 1 : i0 = 0;i1 = 2;break; /* plane XZ */
		case 2 : i0 = 0;i1 = 1;break; /* plane XY */
	}
	for(numplane=0;numplane<grid->N[type];numplane++)
	{
		gdouble* values = g_malloc(sizeof(gdouble));
		*values = value;
		set_contours_values(1, values, i0, i1, numplane, gap);
		glarea_rafresh(GLArea);
	}
}
/********************************************************************************/
void create_contours_surface(gboolean first, gboolean second, gboolean third, gdouble value)
{
	if(first) create_contours_surface_one_plane(0, value);
	if(second) create_contours_surface_one_plane(1, value);
	if(third) create_contours_surface_one_plane(2, value);
}
/********************************************************************************/
static void apply_contours_isosurface(GtkWidget *Win,gpointer data)
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
		gchar buffer[1024];
		sprintf(buffer,_("Error : The isovalue  value should between %lf and %lf"),fabs(limits.MinMax[1][3]),fabs(limits.MinMax[0][3]));
		GtkWidget* message = Message(buffer,_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		return;
	}

	delete_child(Win);
	create_contours_surface(TRUE, TRUE, FALSE, atof(temp));
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
	) square = FALSE;

	if(!compute_isovalue_percent_from_grid(grid, square, percent, precision, &isovalue)) return;
	temp = g_strdup_printf("%f",isovalue);
	gtk_entry_set_text(GTK_ENTRY(Entry),temp); 
	if(temp) g_free(temp);
}
/********************************************************************************/
static GtkWidget *create_iso_frame( GtkWidget *vboxall,gchar* title)
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
void create_contours_isosurface()
{
  GtkWidget *Win;
  GtkWidget *frame;
  GtkWidget *hbox;
  GtkWidget *vboxall;
  GtkWidget *vboxwin;
  GtkWidget *button;
  GtkWidget* Entry;


  if(!grid )
  {
	  if( !CancelCalcul)
	  	Message(_("Grid not defined "),_("Error"),TRUE);
	  return;
  }
  /* Principal Window */
  Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(Win),_("Draw contours for an isosurface"));
  gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
  gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
  gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
  gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

  add_glarea_child(Win,"Iso surface ");

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
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_contours_isosurface,GTK_OBJECT(Win));
  g_signal_connect_swapped(G_OBJECT (Entry), "activate", (GCallback) gtk_button_clicked, GTK_OBJECT (button));
  

  /* Show all */
  gtk_widget_show_all (Win);
}
/********************************************************************************/
void apply_contours(GtkWidget *Win,gpointer data)
{
	GtkWidget** Entries =(GtkWidget**)g_object_get_data(G_OBJECT (Win), "Entries");
	gint type = *((gint*)g_object_get_data(G_OBJECT (Win), "Type"));
	guint handel_id = *((guint*)g_object_get_data(G_OBJECT (Entries[0]), "HandelId"));
	GtkObject* Combo =(GtkObject*)g_object_get_data(G_OBJECT (Entries[0]), "ComboList");
	G_CONST_RETURN gchar* temp;
	gint i;
	gint N;
	gdouble min;
	gdouble max;
	gint i0=0;
	gint i1=1;
	gint numplane = -1;
	gdouble* values = NULL;
	gdouble step = 0;
	gint pvalue = 0;
	gdouble gap = 0;
	
        temp	= gtk_entry_get_text(GTK_ENTRY(Entries[0])); 
	pvalue = atoi(temp);

	N = get_number_of_point(Entries[1]);
	if(N<=0) return;

        if(!get_a_float(Entries[2],&min, _("Error : The minimal value should be float."))) return;
        if(!get_a_float(Entries[3],&max,_("Error : The maximal value should be float."))) return;
	if( max<=min)
	{
		GtkWidget* message = Message(_("Error :  The minimal value should be smaller than the maximal value "),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		return;
	}
        if(!get_a_float(Entries[4],&gap,_("Error : The projection value should be float."))) return;
	numplane = pvalue-1;
	if(numplane <0 || numplane>=grid->N[type]) numplane = grid->N[type]/2;
	switch(type)
	{
		case 0 : i0 = 1;i1 = 2;break; /* plane YZ */
		case 1 : i0 = 0;i1 = 2;break; /* plane XZ */
		case 2 : i0 = 0;i1 = 1;break; /* plane XY */
	}
	/* Debug("N = %d\n",N);*/

	values = g_malloc(N*sizeof(gdouble));

	if(linear)
	{
		step = (max-min)/(N-1);
		for(i=0;i<N;i++) values[i] = min + i*step;
	}
	else
	{
		gdouble e = exp(1.0);
		step = (1.0)/(N-1);
		for(i=0;i<N;i++) values[i] = min+(max-min)*log(step*i*(e-1)+1);
	}
	/* Debug("Begin set_contours_values\n");*/
	set_contours_values(N, values, i0, i1, numplane, gap);
	/* Debug("End set_contours_values\n");*/
	
	g_signal_handler_disconnect(G_OBJECT(Combo), handel_id);

  	delete_child(Win);
  	/* gtk_widget_destroy(Win);*/
}
/********************************************************************************/
static void reset_limits_values(GtkWidget *Win,gpointer data)
{

	GtkWidget** Entries;
	GtkWidget* LabelMin;
	GtkWidget* LabelMax;
	gint type;
	gint ix=0,iy=0,iz=0;
	gint i,j;
	gint i0=0;
	gint i1=1;
	gint numplane = -1;
	gdouble min = 0;
	gdouble max = 0;
	gint pvalue = 0;
	G_CONST_RETURN gchar* temp;
	
	if(!this_is_an_object((GtkObject*)Win))
		return; 

	Entries =(GtkWidget**)g_object_get_data(G_OBJECT (Win), "Entries");
	type = *((gint*)g_object_get_data(G_OBJECT (Win), "Type"));
	if(!Entries)
		return; 
	
	for(i=0;i<4;i++)
		if(!this_is_an_object((GtkObject*)Entries[i]))
			return; 
	    
	LabelMin =(GtkWidget*)g_object_get_data(G_OBJECT (Win), "LabelMin");
	if(!this_is_an_object((GtkObject*)LabelMin))
		return; 

	LabelMax =(GtkWidget*)g_object_get_data(G_OBJECT (Win), "LabelMax");
	if(!this_is_an_object((GtkObject*)LabelMax))
		return; 

	


	temp = gtk_entry_get_text(GTK_ENTRY(Entries[0])); 
	pvalue = atoi(temp);

	switch(type)
	{
		case 0 : i0 = 1;i1 = 2;break; /* plane YZ */
		case 1 : i0 = 0;i1 = 2;break; /* plane XZ */
		case 2 : i0 = 0;i1 = 1;break; /* plane XY */
	}
	numplane = pvalue-1;
	if(numplane<0 || numplane>=grid->N[type]) numplane = grid->N[type]/2;
	for(i=0;i<grid->N[i0];i++)
		for(j=0;j<grid->N[i1];j++)
		{
			switch(type)
			{
				case 0 : ix = numplane;iy = i;iz = j;break;
				case 1 : ix = i;iy = numplane;iz = j;break;
				case 2 : ix = i;iy = j;iz = numplane;break;
			}
			if(i==0 && j==0)
			{
				min = grid->point[ix][iy][iz].C[3];
				max = grid->point[ix][iy][iz].C[3];
			}
			else
			{
				if(min>grid->point[ix][iy][iz].C[3])
					min = grid->point[ix][iy][iz].C[3];
				if(max<grid->point[ix][iy][iz].C[3])
					max = grid->point[ix][iy][iz].C[3];

			}
		}
	temp = g_strdup_printf("%lf",min);
	gtk_entry_set_text(GTK_ENTRY(Entries[2]),temp);
	/*g_free(temp);*/
	temp = g_strdup_printf(" >= %lf ",min);
	gtk_label_set_text(GTK_LABEL(LabelMin),temp);
	/*g_free(temp);*/
	if(fabs(max-min)<1e-5) max +=1e-4;
	temp = g_strdup_printf("%lf",max);
	gtk_entry_set_text(GTK_ENTRY(Entries[3]),temp);
	/*g_free(temp);*/
	temp = g_strdup_printf(" <= %lf ",max);
	gtk_label_set_text(GTK_LABEL(LabelMax),temp);
	/*g_free(temp);*/
}
/********************************************************************************/
static void linearSelected(GtkWidget *widget)
{
	linear = TRUE;
}
/********************************************************************************/
static void logSelected(GtkWidget *widget)
{
	linear = FALSE;
}
/********************************************************************************/
GtkWidget *create_contours_frame( GtkWidget *vboxall,gchar* title,gint type)
{
#define NLIGNES   8
#define NCOLUMNS  4
	GtkWidget *frame;
	GtkWidget *combo;
	GtkWidget *Label;
	GtkWidget *LabelMin = NULL;
	GtkWidget *LabelMax = NULL;
	GtkWidget *vboxframe;
	GtkWidget *hseparator;
	GtkWidget *linearButton;
	GtkWidget *logButton;
	static guint handel_id = 0;
	GtkWidget **Entries = (GtkWidget **)g_malloc(5*sizeof(GtkWidget *));
	gushort i;
	gushort j;
	GtkWidget *Table;
	gchar** listvalues;
	gint ix=0,iy=0,iz=0;
	gdouble min = 0;
	gdouble max = 0;
	static gint itype = 0;
	gchar      *strlabels[NLIGNES][NCOLUMNS];
	
	itype = type;
	strlabels[0][0] = g_strdup(_(" Plane number "));
	listvalues = g_malloc(grid->N[type]*sizeof(gchar*));
	for(i=0;i<grid->N[type];i++)
	{
		switch(type)
		{
			case 0 : ix = i;iy = 0;iz = 0;break;
			case 1 : ix = 0;iy = i;iz = 0;break;
			case 2 : ix = 0;iy = 0;iz = i;break;
		}
		listvalues[i] = g_strdup_printf("%d",i+1);
		if(i==0)
		{
			min = grid->point[ix][iy][iz].C[3];
			max = grid->point[ix][iy][iz].C[3];
		}
		else
		{
			if(min>grid->point[ix][iy][iz].C[3])
				min = grid->point[ix][iy][iz].C[3];
			if(max<grid->point[ix][iy][iz].C[3])
				max = grid->point[ix][iy][iz].C[3];

		}
	}
	strlabels[1][0] = g_strdup(_(" Numbre of contours "));
	strlabels[2][0] = g_strdup(_(" Min iso-value "));
	strlabels[3][0] = g_strdup(_(" Max iso-value "));
	strlabels[4][0] = NULL;
	strlabels[5][0] = NULL;
	strlabels[6][0] = NULL;
	strlabels[7][0] = g_strdup(_(" Projection "));

	strlabels[0][1] = g_strdup(" : ");
	strlabels[1][1] = g_strdup(" : ");
	strlabels[2][1] = g_strdup(" : ");
	strlabels[3][1] = g_strdup(" : ");
	strlabels[4][1] = NULL;
	strlabels[5][1] = NULL;
	strlabels[6][1] = NULL;
	strlabels[7][1] = g_strdup(" : ");

	strlabels[0][2] = g_strdup("  ");
	strlabels[1][2] = g_strdup("10");
	strlabels[2][2] = g_strdup_printf("%lf",9*min/10);
	strlabels[3][2] = g_strdup_printf("%lf",9*max/10);
	strlabels[4][2] = NULL;
	strlabels[5][2] = NULL;
	strlabels[6][2] = NULL;
	strlabels[7][2] = g_strdup("0.0");

	strlabels[0][3] = g_strdup(" ");
	strlabels[1][3] = g_strdup(" ");
	strlabels[2][3] = g_strdup_printf(" >= %lf ",min);
	strlabels[3][3] = g_strdup_printf(" <= %lf ",max);
	strlabels[4][3] = NULL;
	strlabels[5][3] = NULL;
	strlabels[6][3] = NULL;
	strlabels[7][3] = g_strdup(_(" left if <0, right if >0. and nothing if 0"));

	frame = gtk_frame_new (title);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	Table = gtk_table_new(NLIGNES+1,NCOLUMNS,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),Table);

	for(i=0;i<NLIGNES-2;i++)
		for(j=0;j<NCOLUMNS;j++)
			if(j != 2)
			{
				Label =	add_label_at_table(Table,strlabels[i][j],i,(gushort)j,GTK_JUSTIFY_LEFT);
				if(i==2 && j == 3)
					LabelMin = Label;
				if(i==3 && j == 3)
					LabelMax = Label;
			}
	
	i = 7;
	for(j=0;j<NCOLUMNS;j++)
			if(j != 2)
				Label =	add_label_at_table(Table,strlabels[i][j],i,(gushort)j,GTK_JUSTIFY_LEFT);

	combo = create_combo_box_entry(listvalues,grid->N[type], FALSE,-1,-1);
	Entries[0] = GTK_BIN(combo)->child;
	add_widget_table(Table,combo,(gushort)0,(gushort)2);
	Entries[1] = gtk_entry_new ();
	add_widget_table(Table,Entries[1],(gushort)1,(gushort)2);
	gtk_entry_set_text(GTK_ENTRY(Entries[1]),strlabels[1][2]);
	Entries[2] = gtk_entry_new ();
	add_widget_table(Table,Entries[2],(gushort)2,(gushort)2);
	gtk_entry_set_text(GTK_ENTRY(Entries[2]),strlabels[2][2]);
	Entries[3] = gtk_entry_new ();
	add_widget_table(Table,Entries[3],(gushort)3,(gushort)2);
	gtk_entry_set_text(GTK_ENTRY(Entries[3]),strlabels[3][2]);
	
	Entries[4] = gtk_entry_new ();
	add_widget_table(Table,Entries[4],(gushort)7,(gushort)2);
	gtk_entry_set_text(GTK_ENTRY(Entries[4]),strlabels[7][2]);

	
	i=4;
	hseparator = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(Table),hseparator,0,4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  3,3);

	i=5;
	linearButton = gtk_radio_button_new_with_label( NULL,_("Linear scale"));
	add_widget_table(Table, linearButton,i,0);
	logButton = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (linearButton)), _("logarithmic scale")); 
	gtk_table_attach(GTK_TABLE(Table), logButton,1,3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
  	g_signal_connect (G_OBJECT(linearButton), "toggled", G_CALLBACK (linearSelected), NULL);  
  	g_signal_connect (G_OBJECT(logButton), "toggled", G_CALLBACK (logSelected), NULL);  

	if(linear)
	{
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (linearButton), TRUE);
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (logButton), FALSE);
	}
	else
	{
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (linearButton), FALSE);
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (logButton), TRUE);
	}

	i=6;
	hseparator = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(Table),hseparator,0,4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  3,3);

	g_object_set_data(G_OBJECT (frame), "Entries",Entries);
	g_object_set_data(G_OBJECT (GTK_OBJECT(GTK_COMBO_BOX(combo))), "Entries",Entries);
	g_object_set_data(G_OBJECT (GTK_OBJECT(GTK_COMBO_BOX(combo))), "LabelMin",LabelMin);
	g_object_set_data(G_OBJECT (GTK_OBJECT(GTK_COMBO_BOX(combo))), "LabelMax",LabelMax);
	g_object_set_data(G_OBJECT (GTK_OBJECT(GTK_COMBO_BOX(combo))), "Type",&itype);
	handel_id = g_signal_connect(G_OBJECT(GTK_COMBO_BOX(combo)), "changed",G_CALLBACK(reset_limits_values),NULL);
	g_object_set_data(G_OBJECT(Entries[0]), "HandelId",&handel_id);
	g_object_set_data(G_OBJECT(Entries[0]), "ComboList",GTK_OBJECT(GTK_COMBO_BOX(combo)));
	gtk_entry_set_text(GTK_ENTRY(Entries[0]),listvalues[grid->N[type]/2]);

	for(i=0;i<NLIGNES;i++)
	{
		for(j=0;j<NCOLUMNS;j++)
			if(strlabels[i][j])
			g_free(strlabels[i][j]);
	}
	for(i=0;i<grid->N[type];i++)
	{
		if(listvalues[i])
		       g_free(listvalues[i]);
	}
	g_free(listvalues);

	gtk_widget_show_all(frame);
  
  	return frame;
}

/**************************************************************/
void create_contours(gchar* title,gint type)
{
  GtkWidget *Win;
  GtkWidget *frame;
  GtkWidget *hbox;
  GtkWidget *vboxall;
  GtkWidget *vboxwin;
  GtkWidget *button;
  GtkWidget** Entries;
  static gint itype;


  if(!grid)
  {
	  if(!CancelCalcul) Message(_("Grid not defined "),_("Error"),TRUE);
	  return;
  }
  itype = type;
  /* Principal Window */
  Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(Win),title);
  gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
  gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
  gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
  gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

  add_glarea_child(Win," Contours ");

  vboxall = create_vbox(Win);
  vboxwin = vboxall;

  frame = create_contours_frame(vboxall,"Contours",type);
  Entries = (GtkWidget**) g_object_get_data(G_OBJECT (frame), "Entries");
  g_object_set_data(G_OBJECT (Win), "Entries",Entries);
  g_object_set_data(G_OBJECT (Win), "Type",&itype);

  /* buttons box */
  hbox = create_hbox_false(vboxwin);
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
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_contours,GTK_OBJECT(Win));
  

  /* Show all */
  gtk_widget_show_all (Win);
}
/********************************************************************************/
void apply_contours_plane(GtkWidget *Win,gpointer data)
{
			
	GtkWidget** Entries =(GtkWidget**)g_object_get_data(G_OBJECT (Win), "Entries");
	gint N;
	gdouble minv;
	gdouble maxv;
	gdouble gap;
	
	N = get_number_of_point(Entries[0]);
	if(N<=0) return;

        if(!get_a_float(Entries[1],&minv,_("Error : The minimal value should be float."))) return;
        if(!get_a_float(Entries[2],&maxv,_("Error : The maximal value should be float."))) return;
        if(!get_a_float(Entries[3],&gap,_("Error : The projection value should be float."))) return;
	if( maxv<=minv)
	{
		GtkWidget* message = Message(_("Error :  The minimal value should be smaller than the maximal value "),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		return;
	}

	/* Debug("N = %d min = %lf max = %lf\n",N,minv,maxv);*/
 	set_contours_values_from_plane(minv,maxv,N,gap,linear);
	/*Debug("N = %d min = %lf max = %lf\n",N,minv,maxv);*/
	glarea_rafresh(GLArea);
  	delete_child(Win);
}
/********************************************************************************/
static void destroyWin(GtkWidget *Win)
{
	gtk_widget_destroy(Win);
	reDrawContoursPlane = TRUE;
}
/********************************************************************************/
GtkWidget *create_contours_frame_plane( GtkWidget *vboxall,gchar* title)
{
#define NLIGNESP   7
#define NCOLUMNSP  4
	GtkWidget *frame;
	GtkWidget *Label;
	GtkWidget *vboxframe;
	GtkWidget *hseparator;
	GtkWidget *linearButton;
	GtkWidget *logButton;
	GtkWidget **Entries = (GtkWidget **)g_malloc(4*sizeof(GtkWidget *));
	gushort i;
	gushort j;
	GtkWidget *Table;
	gdouble min = 0;
	gdouble max = 0;
	gchar      *strlabels[NLIGNESP][NCOLUMNSP];

	for(i=0;i<gridPlaneForContours->N[0];i++)
		for(j=0;j<gridPlaneForContours->N[1];j++)
		{
			if( i==0 && j == 0)
			{
				min = gridPlaneForContours->point[i][j][0].C[3];
				max = gridPlaneForContours->point[i][j][0].C[3];
			}
			else
			{
				if(min>gridPlaneForContours->point[i][j][0].C[3])
					min = gridPlaneForContours->point[i][j][0].C[3]; 
				if(max<gridPlaneForContours->point[i][j][0].C[3])
					max = gridPlaneForContours->point[i][j][0].C[3]; 
			}
		}
	
	strlabels[0][0] = g_strdup(" Numbre of contours ");
	strlabels[1][0] = g_strdup(" Min iso-value ");
	strlabels[2][0] = g_strdup(" Max iso-value ");
	strlabels[3][0] = NULL;
	strlabels[4][0] = NULL;
	strlabels[5][0] = NULL;
	strlabels[6][0] = g_strdup(" Projection ");

	strlabels[0][1] = g_strdup(" : ");
	strlabels[1][1] = g_strdup(" : ");
	strlabels[2][1] = g_strdup(" : ");
	strlabels[3][1] = NULL;
	strlabels[4][1] = NULL;
	strlabels[5][1] = NULL;
	strlabels[6][1] = g_strdup(" : ");

	strlabels[0][2] = g_strdup("10");
	strlabels[1][2] = g_strdup_printf("%lf",9*min/10);
	strlabels[2][2] = g_strdup_printf("%lf",9*max/10);
	strlabels[3][2] = NULL;
	strlabels[4][2] = NULL;
	strlabels[5][2] = NULL;
	strlabels[6][2] = g_strdup("0.0");

	strlabels[0][3] = g_strdup("");
	strlabels[1][3] = g_strdup_printf(" >= %lf ",min);
	strlabels[2][3] = g_strdup_printf(" <= %lf ",max);
	strlabels[3][3] = NULL;
	strlabels[4][3] = NULL;
	strlabels[5][3] = NULL;
	strlabels[6][3] = g_strdup(" left if <0, right if >0. and nothing if 0");


	frame = gtk_frame_new (title);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	Table = gtk_table_new(NLIGNESP,NCOLUMNSP,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),Table);

	
	for(i=0;i<3;i++)
		for(j=0;j<NCOLUMNSP;j++)
			if(j != 2)
			{
				Label =	add_label_at_table(Table,strlabels[i][j],(gushort)i,(gushort)j,GTK_JUSTIFY_LEFT);
			}
	i = 6;
	for(j=0;j<NCOLUMNSP;j++)
		if(j != 2)
		{
			Label =	add_label_at_table(Table,strlabels[i][j],(gushort)i,(gushort)j,GTK_JUSTIFY_LEFT);
		}
	for(i=0;i<3;i++)
	{
		Entries[i] = gtk_entry_new ();
		add_widget_table(Table,Entries[i],(gushort)i,(gushort)2);
		gtk_entry_set_text(GTK_ENTRY(Entries[i]),strlabels[i][2]);
	}
	i = 6;
	Entries[i-3] = gtk_entry_new ();
	add_widget_table(Table,Entries[i-3],(gushort)i,(gushort)2);
	gtk_entry_set_text(GTK_ENTRY(Entries[i-3]),strlabels[i][2]);

	i= 3;
	hseparator = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(Table),hseparator,0,4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  3,3);

	i=4;
	linearButton = gtk_radio_button_new_with_label( NULL,_("Linear scale"));
	add_widget_table(Table, linearButton,i,0);
	logButton = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (linearButton)), _("logarithmic scale")); 
	gtk_table_attach(GTK_TABLE(Table), logButton,1,3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
  	g_signal_connect (G_OBJECT(linearButton), "toggled", G_CALLBACK (linearSelected), NULL);  
  	g_signal_connect (G_OBJECT(logButton), "toggled", G_CALLBACK (logSelected), NULL);  

	if(linear)
	{
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (linearButton), TRUE);
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (logButton), FALSE);
	}
	else
	{
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (linearButton), FALSE);
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (logButton), TRUE);
	}

	i= 5;
	hseparator = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(Table),hseparator,0,4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  3,3);
	
	g_object_set_data(G_OBJECT (frame), "Entries",Entries);
	for(i=0;i<NLIGNESP;i++)
	{
		for(j=0;j<NCOLUMNSP;j++)
			if(strlabels[i][j])
				g_free(strlabels[i][j]);
	}
	gtk_widget_show_all(frame);
  
  	return frame;
}
/********************************************************************************/
void create_contours_plane(gchar* title)
{
  GtkWidget *Win;
  GtkWidget *frame;
  GtkWidget *hbox;
  GtkWidget *vboxall;
  GtkWidget *vboxwin;
  GtkWidget *button;
  GtkWidget** Entries;


  /* Debug("Creation de la fenetre contour\n");*/
  if(!gridPlaneForContours)
  {
	  Message(_("Grid in plane is not defined "),_("Error"),TRUE);
	  return;
  }
  /* Principal Window */
  Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(Win),title);
  gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
  gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
  gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
  gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

  /* add_glarea_child(Win," Plan Contours ");*/
  add_child(PrincipalWindow,Win,destroyWin,"Plane Contours.");
  g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)delete_child,NULL);

  vboxall = create_vbox(Win);
  vboxwin = vboxall;

  frame = create_contours_frame_plane(vboxall,"Contours");
  Entries = (GtkWidget**) g_object_get_data(G_OBJECT (frame), "Entries");
  g_object_set_data(G_OBJECT (Win), "Entries",Entries);

  /* buttons box */
  hbox = create_hbox_false(vboxwin);
  gtk_widget_realize(Win);

  button = create_button(Win,_("Cancel"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, GTK_OBJECT(Win));
  gtk_widget_show (button);

  button = create_button(Win,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_contours_plane,GTK_OBJECT(Win));
  

  /* Show all */
  gtk_widget_show_all (Win);
}
/********************************************************************************/
