/* AxisGL.c */
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
#include "../Geometry/GeomGlobal.h"
#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
#include "../Utils/Constants.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/UtilsGL.h"
#include "../Utils/Utils.h"
#include "../Common/Windows.h"
#include "GLArea.h"
#include "../Display/UtilsOrb.h"

#define Deg_Rad 180.0/PI

/************************************************************************/
typedef struct _AxisDef
{
 gboolean show;
 gboolean negative;
 gdouble origin[3];
 gdouble radius;
 gdouble scal;
 gdouble xColor[3];
 gdouble yColor[3];
 gdouble zColor[3];
}AxisDef;

static AxisDef axis;
/************************************************************************/
void getAxisProperties(gboolean* show, gboolean* negative, gdouble origin[], gdouble* radius, gdouble* scal, gdouble xColor[], gdouble yColor[], gdouble zColor[])
{
	gint i;
	*show = axis.show;
	*negative = axis.negative;
	for(i=0;i<3;i++) origin [i] = axis.origin[i];
	*radius = axis.radius;
	*scal = axis.scal;
	for(i=0;i<3;i++)
	{
		xColor[i] = axis.xColor[i]; 
		yColor[i] = axis.yColor[i]; 
		zColor[i] = axis.zColor[i]; 
	}
}
/************************************************************************/
void initAxis()
{
	axis.show = FALSE;
	axis.negative = FALSE;
	axis.origin[0] = 0;
	axis.origin[1] = 0;
	axis.origin[2] = 0;
	axis.radius = 0.25;
	axis.scal = 5;
	axis.xColor[0] = 1.0; 
	axis.xColor[1] = 0.0; 
	axis.xColor[2] = 0.0; 
	axis.yColor[0] = 0.0; 
	axis.yColor[1] = 1.0; 
	axis.yColor[2] = 0.0; 
	axis.zColor[0] = 0.0; 
	axis.zColor[1] = 1.0; 
	axis.zColor[2] = 1.0; 
}
/******************************************************************/
void save_axis_properties()
{
	gchar *axesfile;
	FILE *file;

	axesfile = g_strdup_printf("%s%saxes",gabedit_directory(),G_DIR_SEPARATOR_S);

	file = fopen(axesfile, "w");

 	fprintf(file,"%d\n",axis.show);
 	fprintf(file,"%d\n",axis.negative);
 	fprintf(file,"%lf %lf %lf\n",axis.origin[0],axis.origin[1],axis.origin[2]);
 	fprintf(file,"%lf\n",axis.radius);
 	fprintf(file,"%lf\n",axis.scal);
 	fprintf(file,"%lf %lf %lf\n",axis.xColor[0],axis.xColor[1],axis.xColor[2]);
 	fprintf(file,"%lf %lf %lf\n",axis.yColor[0],axis.yColor[1],axis.yColor[2]);
 	fprintf(file,"%lf %lf %lf\n",axis.zColor[0],axis.zColor[1],axis.zColor[2]);

	fclose(file);

	g_free(axesfile);
}
/******************************************************************/
void read_axis_properties()
{
	gchar *axesfile;
	FILE *file;
	gint n;

	initAxis();
	axesfile = g_strdup_printf("%s%saxes",gabedit_directory(),G_DIR_SEPARATOR_S);

	file = fopen(axesfile, "rb");
	if(!file) return;

 	n = fscanf(file,"%d\n",&axis.show);
	if(n != 1) { initAxis(); return ; fclose(file); g_free(axesfile);}
 	n = fscanf(file,"%d\n",&axis.negative);
	if(n != 1) { initAxis(); return ; fclose(file); g_free(axesfile);}
 	n = fscanf(file,"%lf %lf %lf\n",&axis.origin[0],&axis.origin[1],&axis.origin[2]);
	if(n != 3) { initAxis(); return ; fclose(file); g_free(axesfile);}
 	n = fscanf(file,"%lf\n",&axis.radius);
	if(n != 1) { initAxis(); return ; fclose(file); g_free(axesfile);}
 	n = fscanf(file,"%lf\n",&axis.scal);
	if(n != 1) { initAxis(); return ; fclose(file); g_free(axesfile);}
 	n = fscanf(file,"%lf %lf %lf\n",&axis.xColor[0],&axis.xColor[1],&axis.xColor[2]);
	if(n != 3) { initAxis(); return ; fclose(file); g_free(axesfile);}
 	n = fscanf(file,"%lf %lf %lf\n",&axis.yColor[0],&axis.yColor[1],&axis.yColor[2]);
	if(n != 3) { initAxis(); return ; fclose(file); g_free(axesfile);}
 	n = fscanf(file,"%lf %lf %lf\n",&axis.zColor[0],&axis.zColor[1],&axis.zColor[2]);
	if(n != 3) { initAxis(); return ; fclose(file); g_free(axesfile);}

	fclose(file);

	g_free(axesfile);
}
/******************************************************************/
static void set_axis(GtkWidget* fp,gpointer data)
{
	GtkWidget** entrys = (GtkWidget**)data;
	gdouble  *vX = g_object_get_data(G_OBJECT (fp), "XColor");
	gdouble  *vY = g_object_get_data(G_OBJECT (fp), "YColor");
	gdouble  *vZ = g_object_get_data(G_OBJECT (fp), "ZColor");
	gboolean  *negative = g_object_get_data(G_OBJECT (fp), "Negative");
	G_CONST_RETURN gchar* tentry;
	gint i;

	tentry = gtk_entry_get_text(GTK_ENTRY(entrys[0]));
	axis.scal = atof(tentry);
	if(axis.scal<1e-6) axis.scal = 1.0;
	for(i=1;i<4;i++)
	{
		tentry = gtk_entry_get_text(GTK_ENTRY(entrys[i]));
		axis.origin[i-1] = atof(tentry)*ANG_TO_BOHR;
	}

	tentry = gtk_entry_get_text(GTK_ENTRY(entrys[4]));
	axis.radius = atof(tentry)*ANG_TO_BOHR;

	if(axis.radius<1e-6) axis.radius = 0.1;

	for(i=0;i<3;i++)
	{
		 axis.xColor[i] = vX[i];
		 axis.yColor[i] = vY[i];
		 axis.zColor[i] = vZ[i];
	}
	axis.negative = *negative;
	rafresh_window_orb();
}
/******************************************************************/
static void set_negative(GtkWidget* button,gpointer data)
{
	gboolean  *negative= g_object_get_data(G_OBJECT (button), "Negative");
	*negative = !(*negative);
}
/******************************************************************/
static void set_axis_color(GtkColorSelection *Sel,gpointer *d)
{
	GdkColor color;
	gdouble  *v = g_object_get_data(G_OBJECT (Sel), "Color");

	gtk_color_selection_get_current_color(Sel, &color);
	
	v[0] =color.red/65535.0;
	v[1] =color.green/65535.0;
	v[2] =color.blue/65535.0;
}
/******************************************************************/
static void set_axis_button_color(GtkObject *button,gpointer *data)
{
	GtkStyle *style = g_object_get_data(G_OBJECT (button), "Style");
	gdouble  *v = g_object_get_data(G_OBJECT (button), "Color");
	GtkWidget *OldButton  = g_object_get_data(G_OBJECT (button), "Button");
  	style =  gtk_style_copy(style); 
  	style->bg[0].red=(gushort)(v[0]*65535.0);
  	style->bg[0].green=(gushort)(v[1]*65535.0);
  	style->bg[0].blue=(gushort)(v[2]*65535.0);
	gtk_widget_set_style(OldButton, style);
}
/******************************************************************/
static void open_color_dlg_axis(GtkWidget *button,gpointer data)
{

	GtkColorSelectionDialog *colorDlg;
  	GtkStyle* style = g_object_get_data(G_OBJECT (button), "Style");
  	GtkWidget *win = g_object_get_data(G_OBJECT (button), "Win");
	gdouble* v = g_object_get_data(G_OBJECT (button), "Color");;
	GdkColor color;

	colorDlg = (GtkColorSelectionDialog *)gtk_color_selection_dialog_new("Set Axis Color");
	color.red = (gushort)(v[0]*65535);
	color.green = (gushort)(v[1]*65535);
	color.blue = (gushort)(v[2]*65535);
	gtk_color_selection_set_current_color (GTK_COLOR_SELECTION (colorDlg->colorsel), &color);
	gtk_color_selection_set_current_color (GTK_COLOR_SELECTION (colorDlg->colorsel), &color);
	gtk_window_set_transient_for(GTK_WINDOW(colorDlg),GTK_WINDOW(win));
        gtk_window_set_position(GTK_WINDOW(colorDlg),GTK_WIN_POS_CENTER);
  	gtk_window_set_modal (GTK_WINDOW (colorDlg), TRUE);
 	g_signal_connect(G_OBJECT(colorDlg), "delete_event",(GCallback)destroy_button_windows,NULL);
  	g_signal_connect(G_OBJECT(colorDlg), "delete_event",G_CALLBACK(gtk_widget_destroy),NULL);

  	g_object_set_data(G_OBJECT (colorDlg->colorsel), "Color", v);
  	gtk_widget_hide(colorDlg->help_button);
	g_signal_connect_swapped(G_OBJECT(colorDlg->ok_button),"clicked",
		(GCallback)set_axis_color,GTK_OBJECT(colorDlg->colorsel));

  	g_object_set_data(G_OBJECT (colorDlg->ok_button), "Color", v);
  	g_object_set_data(G_OBJECT (colorDlg->ok_button), "Button", button);
  	g_object_set_data(G_OBJECT (colorDlg->ok_button), "Style", style);
	g_signal_connect(G_OBJECT(colorDlg->ok_button),"clicked", (GCallback)set_axis_button_color,NULL);

  	g_signal_connect_swapped(G_OBJECT(colorDlg->ok_button), "clicked",
		(GCallback)destroy_button_windows,GTK_OBJECT(colorDlg));
	g_signal_connect_swapped(G_OBJECT(colorDlg->ok_button),"clicked",
		(GCallback)gtk_widget_destroy,GTK_OBJECT(colorDlg));

  	g_signal_connect_swapped(G_OBJECT(colorDlg->cancel_button), "clicked",
		(GCallback)destroy_button_windows,GTK_OBJECT(colorDlg));
	g_signal_connect_swapped(G_OBJECT(colorDlg->cancel_button),"clicked",
		(GCallback)gtk_widget_destroy,GTK_OBJECT(colorDlg));

  	add_button_windows(" Set Color ",GTK_WIDGET(colorDlg));
	gtk_widget_show(GTK_WIDGET(colorDlg));

}
/*********************************************************************/
void set_axis_dialog ()
{
  GtkWidget *fp;
  GtkWidget *frame;
  GtkWidget *vboxall;
  GtkWidget *vboxframe;
  GtkWidget *hbox;
  GtkWidget *button;
  GtkStyle *style;
  static GtkWidget* entrys[5];
  static gdouble vX[3];
  static gdouble vY[3];
  static gdouble vZ[3];
  static gboolean negative;
  gchar* tlabel[5]={"Factor","X Origin (Ang)","Y Origin (Ang)","Z Origin (Ang)","Radius"};
  gint i;
  GtkWidget* table;

  /* principal Window */
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(fp),TRUE);
  gtk_window_set_title(GTK_WINDOW(fp),_("Set the axis parameters"));
  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);

  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (fp), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(PrincipalWindow));

  g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)gtk_widget_destroy,NULL);

  vboxall = create_vbox(fp);
  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  table = gtk_table_new(9,3,FALSE);
  gtk_box_pack_start(GTK_BOX(vboxframe), table,TRUE,TRUE,0);

  for(i=0;i<5;i++)
  {
	add_label_table(table,tlabel[i],(gushort)i,0);
	add_label_table(table," : ",(gushort)i,1); 

	entrys[i] = gtk_entry_new ();
	gtk_table_attach(GTK_TABLE(table),entrys[i],2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);

	if(i!=0)
	{
		if(i==4)
		{
			gchar* t = g_strdup_printf("%lf",axis.radius*BOHR_TO_ANG);
			gtk_entry_set_text(GTK_ENTRY(entrys[i]),t);
			g_free(t);
		}
		else
		{
			gchar* t = g_strdup_printf("%lf",axis.origin[i-1]*BOHR_TO_ANG);
			gtk_entry_set_text(GTK_ENTRY(entrys[i]),t);
			g_free(t);
		}
	}
	else
	{
		gchar* t = g_strdup_printf("%lf",axis.scal);
		gtk_entry_set_text(GTK_ENTRY(entrys[i]),t);
		g_free(t);
	}
  }

  i = 5;
  add_label_table(table,_("Color for the X axis"),(gushort)i,0);
  add_label_table(table," : ",(gushort)i,1); 
  style = gtk_widget_get_style(fp);
  button = gtk_button_new_with_label(" ");
  style =  gtk_style_copy(style); 
  style->bg[0].red=axis.xColor[0]*65535.0;
  style->bg[0].green=axis.xColor[1]*65535.0;
  style->bg[0].blue=axis.xColor[2]*65535.0;

  vX[0] = axis.xColor[0];
  vX[1] = axis.xColor[1];
  vX[2] = axis.xColor[2];

  gtk_widget_set_style(button, style );
  gtk_table_attach(GTK_TABLE(table),button,2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
  gtk_widget_show (button);

  g_object_set_data(G_OBJECT (button), "Style", style);
  g_object_set_data(G_OBJECT (button), "Win", fp);
  g_object_set_data(G_OBJECT (button), "Color", vX);
  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)open_color_dlg_axis, NULL);

  i++;
  add_label_table(table,_("Color for the Y axis"),(gushort)i,0);
  add_label_table(table," : ",(gushort)i,1); 
  style = gtk_widget_get_style(fp);
  button = gtk_button_new_with_label(" ");
  style =  gtk_style_copy(style); 
  style->bg[0].red=axis.yColor[0]*65535.0;
  style->bg[0].green=axis.yColor[1]*65535.0;
  style->bg[0].blue=axis.yColor[2]*65535.0;

  vY[0] = axis.yColor[0];
  vY[1] = axis.yColor[1];
  vY[2] = axis.yColor[2];

  gtk_widget_set_style(button, style );
  gtk_table_attach(GTK_TABLE(table),button,2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
  gtk_widget_show (button);

  g_object_set_data(G_OBJECT (button), "Style", style);
  g_object_set_data(G_OBJECT (button), "Win", fp);
  g_object_set_data(G_OBJECT (button), "Color", vY);
  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)open_color_dlg_axis, NULL);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vboxframe), hbox, FALSE, FALSE, 1);

  i++;
  add_label_table(table,_("Color for the Z axis"),(gushort)i,0);
  add_label_table(table," : ",(gushort)i,1); 
  style = gtk_widget_get_style(fp);
  button = gtk_button_new_with_label(" ");
  style =  gtk_style_copy(style); 
  style->bg[0].red=axis.zColor[0]*65535.0;
  style->bg[0].green=axis.zColor[1]*65535.0;
  style->bg[0].blue=axis.zColor[2]*65535.0;

  vZ[0] = axis.zColor[0];
  vZ[1] = axis.zColor[1];
  vZ[2] = axis.zColor[2];

  gtk_widget_set_style(button, style );
  gtk_widget_set_style(button, style );
  gtk_table_attach(GTK_TABLE(table),button,2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
  gtk_widget_show (button);
  g_object_set_data(G_OBJECT (button), "Style", style);
  g_object_set_data(G_OBJECT (button), "Win", fp);
  g_object_set_data(G_OBJECT (button), "Color", vZ);
  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)open_color_dlg_axis, NULL);

  i++;
  button = gtk_check_button_new_with_label (_("Show the negative part of the axes"));
  negative = axis.negative;
  g_object_set_data(G_OBJECT (button), "Negative", &negative);
  gtk_table_attach(GTK_TABLE(table),button,0,3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), negative);
  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)set_negative, NULL);

  hbox = create_hbox(vboxall);

  button = create_button(PrincipalWindow,"Close");
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(fp));
  gtk_widget_show (button);

  button = create_button(PrincipalWindow,"Apply");
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_object_set_data(G_OBJECT (button), "XColor", vX);
  g_object_set_data(G_OBJECT (button), "YColor", vY);
  g_object_set_data(G_OBJECT (button), "ZColor", vZ);
  g_object_set_data(G_OBJECT (button), "Negative", &negative);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(set_axis),(gpointer)entrys);
  gtk_widget_show (button);

  button = create_button(PrincipalWindow,"OK");
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_object_set_data(G_OBJECT (button), "XColor", vX);
  g_object_set_data(G_OBJECT (button), "YColor", vY);
  g_object_set_data(G_OBJECT (button), "ZColor", vZ);
  g_object_set_data(G_OBJECT (button), "Negative", &negative);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(set_axis),(gpointer)entrys);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(fp));
  gtk_widget_show (button);


   
  gtk_widget_show_all(fp);
}
/************************************************************************/
gboolean testShowAxis()
{
	return axis.show;
}
/************************************************************************/
void showAxis()
{
	axis.show = TRUE;
}
/************************************************************************/
void hideAxis()
{
	axis.show = FALSE;
}
/************************************************************************/
static void rotatedVector(V3d v)
{
	V3d vz={0.0,0.0,1.0};
	V3d	vert;
	gdouble angle;


	v3d_cross(vz,v,vert);
	angle = acos(v3d_dot(vz,v)/v3d_length(v))*Deg_Rad;
	  
	if(fabs(angle)<1e-6)
		return;
	if(fabs(angle-180)<1e-6)
		glRotated(angle, 1.0, 0.0, 0.0);
	else
	glRotated(angle, vert[0],vert[1],vert[2]);

}
/************************************************************************/
static void drawPrism(GLdouble radius,V3d Base1Pos,V3d Base2Pos)
{
		V3d Direction;
		double lengt;
		GLUquadricObj *obj;
		glPushMatrix();
		glTranslated(Base1Pos[0],Base1Pos[1],Base1Pos[2]);
		Direction[0] = Base2Pos[0]-Base1Pos[0];
		Direction[1] = Base2Pos[1]-Base1Pos[1];
		Direction[2] = Base2Pos[2]-Base1Pos[2];
		lengt = v3d_length(Direction);

		rotatedVector(Direction);
		obj = gluNewQuadric();
		gluQuadricNormals(obj, GL_SMOOTH);
		gluQuadricDrawStyle(obj, GLU_FILL);
		gluCylinder (obj,radius,radius/5,lengt,10,10);
		gluDeleteQuadric(obj);
		glPopMatrix(); 
}

/***************************************************************************************************************/
static void drawPrismColor(GLdouble radius,V3d Base1Pos,V3d Base2Pos, V4d Specular,V4d Diffuse,V4d Ambiant)
{
	glMaterialdv(GL_FRONT_AND_BACK,GL_SPECULAR,Specular);
	glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse);
	glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Ambiant);
	glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,50);
	drawPrism(radius,Base1Pos,Base2Pos);
}
/***************************************************************************************************************/
static void drawAxis(V3d vector, GLdouble radius,V3d origin, V4d specular,V4d diffuse,V4d ambiant)
{
	V3d top;
	V3d bottom;
	V3d center;
	double lengt;
	gint i;
	V4d diffuseFleche;
	V4d ambiantFleche;

	bottom[0] = origin[0];
	bottom[1] = origin[1];
	bottom[2] = origin[2];
	if(axis.negative)
	{
		bottom[0] -= vector[0];
		bottom[1] -= vector[1];
		bottom[2] -= vector[2];
	}


	top[0] = origin[0] + vector[0];
	top[1] = origin[1] + vector[1];
	top[2] = origin[2] + vector[2];

	lengt = v3d_length(vector);

	if(radius<0.1) radius = 0.1;

	if(axis.negative) lengt *=2;

	center[0] = top[0];
	center[1] = top[1];
	center[2] = top[2];

	top[0] += (top[0]-bottom[0])/lengt*2*radius;
	top[1] += (top[1]-bottom[1])/lengt*2*radius;
	top[2] += (top[2]-bottom[2])/lengt*2*radius;


	Cylinder_Draw_Color(radius/2,bottom,center,specular,diffuse,ambiant);
	for(i=0;i<3;i++)
	{
		diffuseFleche[i] = diffuse[i] *0.6;
		ambiantFleche[i] = ambiant[i] *0.6;
	}
	diffuseFleche[3] = diffuse[3];
	ambiantFleche[3] = ambiant[3];

	drawPrismColor(radius/1.5,center,top,specular,diffuseFleche,ambiantFleche);
}
/************************************************************************/
GLuint axisGenList(GLuint axisList)
{
	V4d specular = {1.0f,1.0f,1.0f,1.0f};
	V4d xDiffuse = {1.0f,1.0f,1.0f,1.0f};
	V4d xAmbiant = {1.0f,1.0f,1.0f,1.0f};
	V4d yDiffuse = {1.0f,1.0f,1.0f,1.0f};
	V4d yAmbiant = {1.0f,1.0f,1.0f,1.0f};
	V4d zDiffuse = {1.0f,1.0f,1.0f,1.0f};
	V4d zAmbiant = {1.0f,1.0f,1.0f,1.0f};
	V3d vectorX  = {1,0,0};
	V3d vectorY  = {0,1,0};
	V3d vectorZ  = {0,0,1};
	GLdouble radius = axis.radius;
	gint i;

	for(i=0;i<3;i++)
	{
		xDiffuse[i] = axis.xColor[i];
		yDiffuse[i] = axis.yColor[i];
		zDiffuse[i] = axis.zColor[i];

		xAmbiant[i] = xDiffuse[i]/10;
		yAmbiant[i] = yDiffuse[i]/10;
		zAmbiant[i] = zDiffuse[i]/10;
	}
	xDiffuse[3] = 1;
	yDiffuse[3] = 1;
	zDiffuse[3] = 1;

	for(i=0;i<3;i++)
	{
		vectorX[i] *= axis.scal;
		vectorY[i] *= axis.scal;
		vectorZ[i] *= axis.scal;
	}
	

	if (glIsList(axisList) == GL_TRUE) glDeleteLists(axisList,1);

        axisList = glGenLists(1);
	glNewList(axisList, GL_COMPILE);
	drawAxis(vectorX, radius, axis.origin,  specular, xDiffuse, xAmbiant);
	drawAxis(vectorY, radius, axis.origin,  specular, yDiffuse, yAmbiant);
	drawAxis(vectorZ, radius, axis.origin,  specular, zDiffuse, zAmbiant);
	glEndList();
	return axisList;
}
/************************************************************************/
void axisShowList(GLuint axisList)
{
	if(TypeGeom==GABEDIT_TYPEGEOM_NO) return;

	if(!axis.show) return;

	if (glIsList(axisList) == GL_TRUE) glCallList(axisList);

}
/************************************************************************/


