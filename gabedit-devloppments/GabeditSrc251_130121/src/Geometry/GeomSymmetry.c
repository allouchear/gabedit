/* GeomSymmetry.c */
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
#include "../Utils/Constants.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Utils/Jacobi.h"
#include "../Utils/Transformation.h"
#include "../Common/Windows.h"

/********************************************************************************/
typedef struct _GeomPAxis
{
 gchar *Symb;
 gdouble C[3];
}GeomPAxis;
/********************************************************************************/
static gdouble Axis[3][3] = {{0.0,0.0,0.0},{0.0,0.0,0.0},{0.0,0.0,0.0}};
static gdouble I[3] = {0.0,0.0,0.0};
static gdouble K[3] = {0.0,0.0,0.0};
static gdouble D[3] = {0.0,0.0,0.0};
static GeomPAxis* geomPAxis = NULL;
static gdouble Masse = 0;
/********************************************************************************/
static void put_geom_in_text_widget(GtkWidget* TextWid)
{
	gchar * t = NULL;
	guint n;
	GdkColor blue;
	GdkColor green;

	blue.red = 0;
	blue.green = 65535;
	blue.blue = 65535;

	green.red = 0;
	green.green = 50000;
	green.blue = 0;

	for(n=0;n<Natoms;n++)
	{
		t = g_strdup_printf("%5s %20.8f %20.8f %20.8f\n",
				geomPAxis[n].Symb,
				geomPAxis[n].C[0]*BOHR_TO_ANG,
				geomPAxis[n].C[1]*BOHR_TO_ANG,
				geomPAxis[n].C[2]*BOHR_TO_ANG
				);
		if(n%2==0)
 		gabedit_text_insert (GABEDIT_TEXT(TextWid), NULL, NULL, &blue,t,-1);   
		else
 		gabedit_text_insert (GABEDIT_TEXT(TextWid), NULL, NULL, &green,t,-1);   
		g_free(t);
	}
}
/********************************************************************************/
static void set_geom_principal_axis()
{
	gdouble **m0 = g_malloc(3*sizeof(gdouble*));
	gdouble** minv;
	gint i,j;
	guint n;

	gdouble A[3];
	gdouble B[3];
	guint k;

	if(Natoms<1)
		return;
	geomPAxis = g_malloc(Natoms*sizeof(GeomPAxis));

	for(i=0;i<3;i++)
		m0[i] = g_malloc(3*sizeof(gdouble));


	for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			m0[i][j] = Axis[j][i];

	minv = Inverse(m0,3,1e-7);
	/*minv = Inverse3(m0);*/

	for(n = 0;n<Natoms;n++)
	{
		A[0] = geometry0[n].X;
		A[1] = geometry0[n].Y;
		A[2] = geometry0[n].Z;

		for(j=0;j<3;j++)
		{
			B[j] = 0.0;
			for(k=0;k<3;k++)
				B[j] += minv[k][j]*A[k];
		}

		for(i=0;i<3;i++)
		geomPAxis[n].C[i] = B[i];
		geomPAxis[n].Symb = g_strdup(geometry0[n].Prop.symbol);
	}
	
	/*
	Debug("\n");
	for(n=0;n<Natoms;n++)
		Debug("%s %20.8f %20.8f %20.8f\n",geomPAxis[n].Symb,geomPAxis[n].C[0],geomPAxis[n].C[1],geomPAxis[n].C[2]);
	Debug("\n");
	Debug("\n");
	*/

	for(i=0;i<3;i++)
		if(minv[i])
			g_free(minv[i]);
	if(minv)
		g_free(minv);

	for(i=0;i<3;i++)
		if(m0[i])
			g_free(m0[i]);
	if(m0)
		g_free(m0);
}
/********************************************************************************/
static void set_rot_matrice(gdouble m[6])
{
	guint i;
	guint ip;
	guint j;
	guint k;
	gdouble a;
	gdouble* XYZ[3];
	gdouble C[3]; /* center of masses*/
	gdouble mt = 0;

	for(i=0;i<3;i++)
	{
		XYZ[i] = g_malloc(Natoms*sizeof(gdouble));
		C[i] = 0.0;
	}


	for(j=0;j<Natoms;j++)
	{
		mt += geometry0[j].Prop.masse;
		XYZ[0][j] = geometry0[j].X;
		XYZ[1][j] = geometry0[j].Y;
		XYZ[2][j] = geometry0[j].Z;
	}
	for(i=0;i<3;i++)
		for(j=0;j<Natoms;j++)
			C[i] += geometry0[j].Prop.masse*XYZ[i][j];
	for(i=0;i<3;i++)
		C[i] /= mt;

	/*
	for(j=0;j<Natoms;j++)
		Debug("%s %20.8f %20.8f %20.8f\n",geometry0[j].Prop.Symb,XYZ[0][j],XYZ[1][j],XYZ[2][j]);
	Debug("\n");
	Debug("Centre of masse : ");
	for(i=0;i<3;i++)
		Debug("%f ",C[i]);
	Debug("\n");
	*/

	for(j=0;j<Natoms;j++)
		for(i=0;i<3;i++)
			XYZ[i][j] -= C[i];
	
	k = 0;
	for(i=0;i<3;i++)
		for(ip=i;ip<3;ip++)
	{
		m[k] = 0.0;

		for(j=0;j<Natoms;j++)
		{
			if(i==ip)
			a = XYZ[(i+1)%3][j]*XYZ[(ip+1)%3][j]
			  + XYZ[(i+2)%3][j]*XYZ[(ip+2)%3][j];
			else
			{
				a =-XYZ[i][j]*XYZ[ip][j];
			}
			m[k] += geometry0[j].Prop.masse*a;
		}
		/*
		Debug("%20.6f ",m[k]);
		if(ip==2)
			Debug("\n");
			*/

		k++;
	}

	for(i=0;i<3;i++)
		g_free(XYZ[i]);
	Masse = mt;


}
/********************************************************************************/
static void set_dipole_to_principal_axis()
{
	gdouble **m0 = g_malloc(3*sizeof(gdouble*));
	gdouble** minv;
	gint i,j;

	gdouble A[3];
	gdouble B[3];
	guint k;

	if(!Ddef)
		return;

	for(i=0;i<3;i++)
		m0[i] = g_malloc(3*sizeof(gdouble));


	for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			m0[i][j] = Axis[j][i];

	minv = Inverse(m0,3,1e-7);

	A[0] =  dipole00[NDIVDIPOLE-1][0] -  dipole00[0][0];
	A[1] =  dipole00[NDIVDIPOLE-1][1] -  dipole00[0][1];
	A[2] =  dipole00[NDIVDIPOLE-1][2] -  dipole00[0][2];
	{
		gdouble M = 0.0;
		gdouble M0 = 0.0;
		for(i=0;i<3;i++)
		{
			M += A[i]*A[i];
			M0 += Dipole.value[i]*Dipole.value[i];
		}
		M = sqrt(M);
		M0 = sqrt(M0);

	for(i=0;i<3;i++)
		A[i] = A[i]*M0/M;
	}
	for(j=0;j<3;j++)
	{
		B[j] = 0.0;
		for(k=0;k<3;k++)
			B[j] += minv[k][j]*A[k];
	}
	/*
	Debug("Dipole in original axis : %20.6f %20.6f %20.6f %f \n",A[0],A[1],A[2],sqrt(A[0]*A[0]+A[1]*A[1]+A[2]*A[2]));
	Debug("Dipole in principal axis : %20.6f %20.6f %20.6f %f \n",B[0],B[1],B[2],sqrt(B[0]*B[0]+B[1]*B[1]+B[2]*B[2]));
	*/


	for(i=0;i<3;i++)
		D[i] = B[i];

	for(i=0;i<3;i++)
		if(minv[i])
			g_free(minv[i]);
	if(minv)
		g_free(minv);

	for(i=0;i<3;i++)
		if(m0[i])
			g_free(m0[i]);
	if(m0)
		g_free(m0);
}
/********************************************************************************/
static void set_rot_constantes()
{
	static gdouble x = 6.62606867/8.0/PI/PI/2.99792458/1.66053873/0.5291772083/0.5291772083*1.0e3;
	gint i;
	/*Debug("Rotational constantes (cm-1): ");*/
	for(i=0;i<3;i++)
	{
		K[i] = -1.0;
		if(fabs(I[i])>1e-10)
			K[i] = x/I[i];
		/*
		if(K[i]<-1)
			Debug(" %20s ","inf");
		else
			Debug(" %20.8f ",K[i]);
		*/
			
	}
	/*
	Debug("\n");
	Debug("Rotational constantes (GHz): ");
	for(i=0;i<3;i++)
	{
		if(K[i]<-1)
			Debug(" %20s ","inf");
		else
			Debug(" %20.8f ",29.9792458*K[i]);
	}
	Debug("\n");
	*/
			

}
/********************************************************************************/
static void set_axis_inertie_rot()
{
	gdouble m[6];
	gdouble** v = g_malloc(3*sizeof(gdouble*));
	gint nrot;
	gint i;
	gint j;
	if(Natoms<1)
		return;

	for(i=0;i<3;i++)
	 v[i] = g_malloc(3*sizeof(gdouble));

	set_rot_matrice(m);
	jacobi(m, 3,I,v,&nrot);

	for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			Axis[j][i] = v[j][i];

	for(i=0;i<3;i++)
		g_free(v[i]);
	g_free(v);

	/*
	for(i=0;i<3;i++)
		Debug("%20.6f ",I[i]);
	Debug("\n");
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
			Debug("%20.6f ",Axis[j][i]);
		Debug("\n");
	}
	*/
	set_rot_constantes();
	set_dipole_to_principal_axis();
}
/********************************************************************************/
static void create_frame_rotation(GtkWidget *Dialogue,GtkWidget *vboxframe)
{
  GtkWidget *vbox;
  GtkWidget *Table;
  GtkWidget *Frame;
  GtkWidget *Label;
  guint i;
  guint j;
  gchar* textlabel[5][5] = { 
				{" "," ",N_("  Axis n 1 "),N_("  Axis n 2  "),N_("  Axis n 3  ")},
				{"I(uam 12C)",":"," "," "," "},
				{"K(cm-1)",":"," "," "," "},
				{"K(Ghz)",":"," "," "," "},
				{N_("Mass "),"=","  ","uam(12C)"," "}
			};
  j = 4;
  textlabel[j][2] = g_strdup_printf("%.8f  ",Masse);

  j = 1;
  for(i=2;i<5;i++)
	textlabel[j][i] = g_strdup_printf("%.8f  ",I[i-2]);
  j = 2;
  for(i=2;i<5;i++)
	  if(K[i-2]>0)
	textlabel[j][i] = g_strdup_printf("%.8f  ",K[i-2]);
  	else
	textlabel[j][i] = g_strdup_printf("%8s  "," ");
  j = 3;
  for(i=2;i<5;i++)
	  if(K[i-2]>0)
	textlabel[j][i] = g_strdup_printf("%.8f  ",29.9792458*K[i-2]);
  	else
	textlabel[j][i] = g_strdup_printf("%8s  "," ");

  Frame = create_frame(Dialogue,vboxframe,_("Inertie Moment/Rotational Constants"));  
  vbox = create_vbox(Frame);
  Table = gtk_table_new(5,5,FALSE);
  gtk_container_add(GTK_CONTAINER(vbox),Table);


  for(j=0;j<5;j++)
  for(i=0;i<5;i++)
  {
	Label = add_label_table(Table,textlabel[j][i],(gushort)j,(gushort)i); 
	if(j== 1)
	set_fg_style(Label,20000,0,0);
	if(j== 2)
	set_fg_style(Label,0,20000,0);
	if(j== 3)
	set_fg_style(Label,0,0,20000);
	gtk_widget_hide(Label);
	gtk_widget_show(Label);
}

  gtk_widget_show_all(Table);
  /*
  for(j=1;j<4;j++)
  for(i=2;i<5;i++)
	g_free(textlabel[j][i]);
	*/
  gtk_widget_show_all(Frame);

}
/********************************************************************************/
static void create_frame_dipole(GtkWidget *Dialogue,GtkWidget *vboxframe)
{
  GtkWidget *vbox;
  GtkWidget *Table;
  GtkWidget *Frame;
  GtkWidget *Label;
  guint i;
  guint j;
  gdouble Module;
  gchar* textlabel[3][6] = { 
				{" "," ",N_("  Axis n 1 "),N_("  Axis n 2  "),N_("  Axis n 3  "),N_(" Module ")},
				{N_("Dipole(au)"),":"," "," "," "," "},
				{N_("Dipole(Debye)"),":"," "," "," "," "},
			};
  Module = sqrt(D[0]*D[0] +  D[1]*D[1] + D[2]*D[2]) ;
  j = 1;
  for(i=2;i<5;i++)
	textlabel[j][i] = g_strdup_printf("%.8f  ",D[i-2]);
  textlabel[j][5] = g_strdup_printf("%.8f  ",Module);

  j = 2;
  for(i=2;i<5;i++)
	textlabel[j][i] = g_strdup_printf("%.8f  ",AUTODEB*D[i-2]);
  textlabel[j][5] = g_strdup_printf("%.8f  ",AUTODEB*Module);

  Frame = create_frame(Dialogue,vboxframe,_("Dipole at principal axis"));  
  vbox = create_vbox(Frame);
  Table = gtk_table_new(3,5,FALSE);
  gtk_container_add(GTK_CONTAINER(vbox),Table);


  for(j=0;j<3;j++)
  for(i=0;i<6;i++)
  {
	Label = add_label_table(Table,textlabel[j][i],(gushort)j,(gushort)i); 
	if(j== 1)
	set_fg_style(Label,20000,0,0);
	if(j== 2)
	set_fg_style(Label,0,20000,0);
	gtk_widget_hide(Label);
	gtk_widget_show(Label);
  }

  gtk_widget_show_all(Table);
  /*
  for(j=1;j<3;j++)
  for(i=2;i<6;i++)
	g_free(textlabel[j][i]);
	*/
  gtk_widget_show_all(Frame);

}
/********************************************************************************/
void create_symmetry_window(GtkWidget* w,guint data)
{
    GtkWidget *Dialogue = NULL;
    GtkWidget *Bouton;
    GtkWidget *frame, *vboxframe;
    gchar * title = NULL;

    if(Natoms<1)
    {
	    Message(N_("Sorry Number of atoms is not positive"),N_("Error"),TRUE);
	    return;
    }
    set_axis_inertie_rot();
    Dialogue = gtk_dialog_new();
    gtk_widget_realize(GTK_WIDGET(Dialogue));
    if(Ddef)
    	title = g_strdup(N_("Rotational constants & Dipole"));
    else
    	title = g_strdup(N_("Rotational constants"));
			
    gtk_window_set_title(GTK_WINDOW(Dialogue),title);

    gtk_window_set_modal (GTK_WINDOW (Dialogue), TRUE);
   gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);

   g_signal_connect(G_OBJECT(Dialogue), "delete_event", (GCallback)destroy_button_windows, NULL);
    g_signal_connect(G_OBJECT(Dialogue), "delete_event", (GCallback)gtk_widget_destroy, NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  create_frame_rotation(Dialogue,vboxframe);
  if(Ddef)
  create_frame_dipole(Dialogue,vboxframe);

  gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Dialogue)->action_area), FALSE);
  
  Bouton = create_button(Dialogue,"OK");
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dialogue)->action_area), Bouton, FALSE, TRUE, 5);  
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)destroy_button_windows, GTK_OBJECT(Dialogue));
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(Dialogue));

    add_button_windows(title,Dialogue);
    gtk_widget_show_all(Dialogue);
    g_free(title);
}
/********************************************************************************/
void create_geometry_paxis_window(GtkWidget* w,guint data)
{
    GtkWidget *Dialogue = NULL;
    GtkWidget *Bouton;
    GtkWidget *frame;
    GtkWidget *TextWid;
    gchar * title = NULL;

    if(Natoms<1)
    {
	    Message(_("Sorry Number of atoms is not positive"),_("Error"),TRUE);
	    return;
    }
    set_axis_inertie_rot();
    set_geom_principal_axis();
    Dialogue = gtk_dialog_new();
    gtk_widget_realize(GTK_WIDGET(Dialogue));
    title = g_strdup(_("Geometry at principal axis"));
			
    gtk_window_set_title(GTK_WINDOW(Dialogue),title);

    gtk_window_set_modal (GTK_WINDOW (Dialogue), TRUE);
   gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);

   g_signal_connect(G_OBJECT(Dialogue), "delete_event", (GCallback)destroy_button_windows, NULL);
    g_signal_connect(G_OBJECT(Dialogue), "delete_event", (GCallback)gtk_widget_destroy, NULL);

  TextWid = create_text_widget(GTK_WIDGET(GTK_DIALOG(Dialogue)->vbox),NULL,&frame);

  gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Dialogue)->action_area), FALSE);
  
  Bouton = create_button(Dialogue,"OK");
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dialogue)->action_area), Bouton, FALSE, TRUE, 5);  
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)destroy_button_windows, GTK_OBJECT(Dialogue));
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(Dialogue));

    add_button_windows(title,Dialogue);
	put_geom_in_text_widget(TextWid);
  gtk_window_set_default_size (GTK_WINDOW(Dialogue), ScreenWidth/3, ScreenHeight/3);
    gtk_widget_show_all(Dialogue);
    g_free(title);
}
