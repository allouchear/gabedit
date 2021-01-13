/* AxesGeomGL.c */
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

#ifdef DRAWGEOMGL
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib/gi18n.h>
#include <glib.h>
#include "../../gl2ps/gl2ps.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "../Common/Global.h"
#include "../Common/GabeditType.h"
#include "../Utils/Constants.h"
#include "../Files/GabeditFileChooser.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsGL.h"
#include "../Geometry/DrawGeomGL.h"
#include "../Common/Windows.h"

/************************************************************************/
typedef struct _AxesDef
{
 gboolean show;
 gboolean negative;
 gdouble origin[3];
 gdouble radius;
 gdouble scal;
 gdouble xColor[3];
 gdouble yColor[3];
 gdouble zColor[3];
}AxesDef;

static AxesDef axes;
/************************************************************************/
void getAxesGeomProperties(gboolean* show, gboolean* negative, gdouble origin[], gdouble* radius, gdouble* scal, gdouble xColor[], gdouble yColor[], gdouble zColor[])
{
	gint i;
	*show = axes.show;
	*negative = axes.negative;
	for(i=0;i<3;i++) origin [i] = axes.origin[i];
	*radius = axes.radius;
	*scal = axes.scal;
	for(i=0;i<3;i++)
	{
		xColor[i] = axes.xColor[i]; 
		yColor[i] = axes.yColor[i]; 
		zColor[i] = axes.zColor[i]; 
	}
}
/************************************************************************/
void initAxesGeom()
{
	axes.show = FALSE;
	axes.negative = FALSE;
	axes.origin[0] = 0;
	axes.origin[1] = 0;
	axes.origin[2] = 0;
	axes.radius = 0.25;
	axes.scal = 5;
	axes.xColor[0] = 1.0; 
	axes.xColor[1] = 0.0; 
	axes.xColor[2] = 0.0; 
	axes.yColor[0] = 0.0; 
	axes.yColor[1] = 1.0; 
	axes.yColor[2] = 0.0; 
	axes.zColor[0] = 0.0; 
	axes.zColor[1] = 1.0; 
	axes.zColor[2] = 1.0; 
}
/******************************************************************/
void save_axes_geom_properties()
{
	gchar *axesfile;
	FILE *file;

	axesfile = g_strdup_printf("%s%saxesGeom",gabedit_directory(),G_DIR_SEPARATOR_S);

	file = fopen(axesfile, "w");

 	fprintf(file,"%d\n",axes.show);
 	fprintf(file,"%d\n",axes.negative);
 	fprintf(file,"%lf %lf %lf\n",axes.origin[0],axes.origin[1],axes.origin[2]);
 	fprintf(file,"%lf\n",axes.radius);
 	fprintf(file,"%lf\n",axes.scal);
 	fprintf(file,"%lf %lf %lf\n",axes.xColor[0],axes.xColor[1],axes.xColor[2]);
 	fprintf(file,"%lf %lf %lf\n",axes.yColor[0],axes.yColor[1],axes.yColor[2]);
 	fprintf(file,"%lf %lf %lf\n",axes.zColor[0],axes.zColor[1],axes.zColor[2]);

	fclose(file);

	g_free(axesfile);
}
/******************************************************************/
void read_axes_geom_properties()
{
	gchar *axesfile;
	FILE *file;
	gint n;

	initAxesGeom();
	axesfile = g_strdup_printf("%s%saxesGeom",gabedit_directory(),G_DIR_SEPARATOR_S);

	file = fopen(axesfile, "rb");
	if(!file) return;

 	n = fscanf(file,"%d\n",&axes.show);
	if(n != 1) { initAxesGeom(); return ; fclose(file); g_free(axesfile);}
 	n = fscanf(file,"%d\n",&axes.negative);
	if(n != 1) { initAxesGeom(); return ; fclose(file); g_free(axesfile);}
 	n = fscanf(file,"%lf %lf %lf\n",&axes.origin[0],&axes.origin[1],&axes.origin[2]);
	if(n != 3) { initAxesGeom(); return ; fclose(file); g_free(axesfile);}
 	n = fscanf(file,"%lf\n",&axes.radius);
	if(n != 1) { initAxesGeom(); return ; fclose(file); g_free(axesfile);}
 	n = fscanf(file,"%lf\n",&axes.scal);
	if(n != 1) { initAxesGeom(); return ; fclose(file); g_free(axesfile);}
 	n = fscanf(file,"%lf %lf %lf\n",&axes.xColor[0],&axes.xColor[1],&axes.xColor[2]);
	if(n != 3) { initAxesGeom(); return ; fclose(file); g_free(axesfile);}
 	n = fscanf(file,"%lf %lf %lf\n",&axes.yColor[0],&axes.yColor[1],&axes.yColor[2]);
	if(n != 3) { initAxesGeom(); return ; fclose(file); g_free(axesfile);}
 	n = fscanf(file,"%lf %lf %lf\n",&axes.zColor[0],&axes.zColor[1],&axes.zColor[2]);
	if(n != 3) { initAxesGeom(); return ; fclose(file); g_free(axesfile);}

	fclose(file);

	g_free(axesfile);
}
/******************************************************************/
static void set_axes(GtkWidget* fp,gpointer data)
{
	GtkWidget** entrys = (GtkWidget**)data;
	gdouble  *vX = g_object_get_data(G_OBJECT (fp), "XColor");
	gdouble  *vY = g_object_get_data(G_OBJECT (fp), "YColor");
	gdouble  *vZ = g_object_get_data(G_OBJECT (fp), "ZColor");
	gboolean  *negative = g_object_get_data(G_OBJECT (fp), "Negative");
	G_CONST_RETURN gchar* tentry;
	gint i;

	tentry = gtk_entry_get_text(GTK_ENTRY(entrys[0]));
	axes.scal = atof(tentry);
	if(axes.scal<1e-6) axes.scal = 1.0;
	for(i=1;i<4;i++)
	{
		tentry = gtk_entry_get_text(GTK_ENTRY(entrys[i]));
		axes.origin[i-1] = atof(tentry)*ANG_TO_BOHR;
	}

	tentry = gtk_entry_get_text(GTK_ENTRY(entrys[4]));
	axes.radius = atof(tentry)*ANG_TO_BOHR;

	if(axes.radius<1e-6) axes.radius = 0.1;

	for(i=0;i<3;i++)
	{
		 axes.xColor[i] = vX[i];
		 axes.yColor[i] = vY[i];
		 axes.zColor[i] = vZ[i];
	}
	axes.negative = *negative;
	rafresh_drawing();
}
/******************************************************************/
static void set_negative(GtkWidget* button,gpointer data)
{
	gboolean  *negative= g_object_get_data(G_OBJECT (button), "Negative");
	*negative = !(*negative);
}
/******************************************************************/
static void set_axes_color(GtkColorSelection *Sel,gpointer *d)
{
	GdkColor color;
	gdouble  *v = g_object_get_data(G_OBJECT (Sel), "Color");

	gtk_color_selection_get_current_color(Sel, &color);
	
	v[0] =color.red/65535.0;
	v[1] =color.green/65535.0;
	v[2] =color.blue/65535.0;
}
/******************************************************************/
static void set_axes_button_color(GtkObject *button,gpointer *data)
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
static void open_color_dlg_axes(GtkWidget *button,gpointer data)
{

	GtkColorSelectionDialog *colorDlg;
  	GtkStyle* style = g_object_get_data(G_OBJECT (button), "Style");
  	GtkWidget *win = g_object_get_data(G_OBJECT (button), "Win");
	gdouble* v = g_object_get_data(G_OBJECT (button), "Color");;
	GdkColor color;

	colorDlg = (GtkColorSelectionDialog *)gtk_color_selection_dialog_new("Set Axes Color");
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
		(GCallback)set_axes_color,GTK_OBJECT(colorDlg->colorsel));

  	g_object_set_data(G_OBJECT (colorDlg->ok_button), "Color", v);
  	g_object_set_data(G_OBJECT (colorDlg->ok_button), "Button", button);
  	g_object_set_data(G_OBJECT (colorDlg->ok_button), "Style", style);
	g_signal_connect(G_OBJECT(colorDlg->ok_button),"clicked", (GCallback)set_axes_button_color,NULL);

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
void set_axes_geom_dialog ()
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

  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(fp),TRUE);
  gtk_window_set_title(GTK_WINDOW(fp),_("Set the axes parameters"));
  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);

  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (fp), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(GeomDlg));

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
			gchar* t = g_strdup_printf("%lf",axes.radius*BOHR_TO_ANG);
			gtk_entry_set_text(GTK_ENTRY(entrys[i]),t);
			g_free(t);
		}
		else
		{
			gchar* t = g_strdup_printf("%lf",axes.origin[i-1]*BOHR_TO_ANG);
			gtk_entry_set_text(GTK_ENTRY(entrys[i]),t);
			g_free(t);
		}
	}
	else
	{
		gchar* t = g_strdup_printf("%lf",axes.scal);
		gtk_entry_set_text(GTK_ENTRY(entrys[i]),t);
		g_free(t);
	}
  }

  i = 5;
  add_label_table(table,_("Color for the X axes"),(gushort)i,0);
  add_label_table(table," : ",(gushort)i,1); 
  style = gtk_widget_get_style(fp);
  button = gtk_button_new_with_label(" ");
  style =  gtk_style_copy(style); 
  style->bg[0].red=axes.xColor[0]*65535.0;
  style->bg[0].green=axes.xColor[1]*65535.0;
  style->bg[0].blue=axes.xColor[2]*65535.0;

  vX[0] = axes.xColor[0];
  vX[1] = axes.xColor[1];
  vX[2] = axes.xColor[2];

  gtk_widget_set_style(button, style );
  gtk_table_attach(GTK_TABLE(table),button,2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
  gtk_widget_show (button);

  g_object_set_data(G_OBJECT (button), "Style", style);
  g_object_set_data(G_OBJECT (button), "Win", fp);
  g_object_set_data(G_OBJECT (button), "Color", vX);
  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)open_color_dlg_axes, NULL);

  i++;
  add_label_table(table,_("Color for the Y axes"),(gushort)i,0);
  add_label_table(table," : ",(gushort)i,1); 
  style = gtk_widget_get_style(fp);
  button = gtk_button_new_with_label(" ");
  style =  gtk_style_copy(style); 
  style->bg[0].red=axes.yColor[0]*65535.0;
  style->bg[0].green=axes.yColor[1]*65535.0;
  style->bg[0].blue=axes.yColor[2]*65535.0;

  vY[0] = axes.yColor[0];
  vY[1] = axes.yColor[1];
  vY[2] = axes.yColor[2];

  gtk_widget_set_style(button, style );
  gtk_table_attach(GTK_TABLE(table),button,2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
  gtk_widget_show (button);

  g_object_set_data(G_OBJECT (button), "Style", style);
  g_object_set_data(G_OBJECT (button), "Win", fp);
  g_object_set_data(G_OBJECT (button), "Color", vY);
  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)open_color_dlg_axes, NULL);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vboxframe), hbox, FALSE, FALSE, 1);

  i++;
  add_label_table(table,_("Color for the Z axes"),(gushort)i,0);
  add_label_table(table," : ",(gushort)i,1); 
  style = gtk_widget_get_style(fp);
  button = gtk_button_new_with_label(" ");
  style =  gtk_style_copy(style); 
  style->bg[0].red=axes.zColor[0]*65535.0;
  style->bg[0].green=axes.zColor[1]*65535.0;
  style->bg[0].blue=axes.zColor[2]*65535.0;

  vZ[0] = axes.zColor[0];
  vZ[1] = axes.zColor[1];
  vZ[2] = axes.zColor[2];

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
  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)open_color_dlg_axes, NULL);

  i++;
  button = gtk_check_button_new_with_label (_("Show the negative part of the axes"));
  negative = axes.negative;
  g_object_set_data(G_OBJECT (button), "Negative", &negative);
  gtk_table_attach(GTK_TABLE(table),button,0,3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), negative);
  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)set_negative, NULL);

  hbox = create_hbox(vboxall);

  button = create_button(GeomDlg,"Close");
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(fp));
  gtk_widget_show (button);

  button = create_button(GeomDlg,"Apply");
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_object_set_data(G_OBJECT (button), "XColor", vX);
  g_object_set_data(G_OBJECT (button), "YColor", vY);
  g_object_set_data(G_OBJECT (button), "ZColor", vZ);
  g_object_set_data(G_OBJECT (button), "Negative", &negative);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(set_axes),(gpointer)entrys);
  gtk_widget_show (button);

  button = create_button(GeomDlg,"OK");
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_object_set_data(G_OBJECT (button), "XColor", vX);
  g_object_set_data(G_OBJECT (button), "YColor", vY);
  g_object_set_data(G_OBJECT (button), "ZColor", vZ);
  g_object_set_data(G_OBJECT (button), "Negative", &negative);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(set_axes),(gpointer)entrys);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(fp));
  gtk_widget_show (button);


   
  gtk_widget_show_all(fp);
}
/************************************************************************/
gboolean testShowAxesGeom()
{
	return axes.show;
}
/************************************************************************/
void showAxesGeom()
{
	axes.show = TRUE;
}
/************************************************************************/
void hideAxesGeom()
{
	axes.show = FALSE;
}
/************************************************************************/
void gl_build_axes(gdouble* position)
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
	GLdouble radius = axes.radius;
	gint i;
	V3d origin;
	

	for(i=0;i<3;i++)
	{
		xDiffuse[i] = axes.xColor[i];
		yDiffuse[i] = axes.yColor[i];
		zDiffuse[i] = axes.zColor[i];

		xAmbiant[i] = xDiffuse[i]/10;
		yAmbiant[i] = yDiffuse[i]/10;
		zAmbiant[i] = zDiffuse[i]/10;
	}
	xDiffuse[3] = 1;
	yDiffuse[3] = 1;
	zDiffuse[3] = 1;

	for(i=0;i<3;i++)
	{
		vectorX[i] *= axes.scal;
		vectorY[i] *= axes.scal;
		vectorZ[i] *= axes.scal;
	}
	if(position)
		for(i=0;i<3;i++) origin[i] = position[i];
	else
		for(i=0;i<3;i++) origin[i] = axes.origin[i];
	

	Draw_Arrow(vectorX, radius, origin,  specular, xDiffuse, xAmbiant, axes.negative);
	Draw_Arrow(vectorY, radius, origin,  specular, yDiffuse, yAmbiant, axes.negative);
	Draw_Arrow(vectorZ, radius, origin,  specular, zDiffuse, zAmbiant, axes.negative);
}
/*********************************************************************************************/
void showLabelAxesGeom(gboolean ortho, gdouble* position, PangoContext *ft2_context)
{
	gint i;
	V4d color  = {0.8,0.8,0.8,1.0 };
	gchar buffer[BSIZE];
	gboolean show;
	gboolean negative;
	gdouble origin[3];
	gdouble radius;
	gdouble scal;
	gdouble xColor[3];
	gdouble yColor[3];
	gdouble zColor[3];
	gdouble vectorX[]  = {1,0,0};
	gdouble vectorY[]  = {0,1,0};
	gdouble vectorZ[]  = {0,0,1};

	if(!testShowAxesGeom()) return;

	getAxesGeomProperties(&show, &negative, origin, &radius, &scal, xColor, yColor, zColor);
	if(position)
	{
		for(i=0;i<3;i++)
			origin[i]=position[i];
	}

	for(i=0;i<3;i++)
	{
		vectorX[i] *= scal;
		vectorY[i] *= scal;
		vectorZ[i] *= scal;
	}
	for(i=0;i<3;i++)
	{
		vectorX[i] += origin[i];
		vectorY[i] += origin[i];
		vectorZ[i] += origin[i];
	}
	

	color[0] = FontsStyleLabel.TextColor.red/65535.0; 
	color[1] = FontsStyleLabel.TextColor.green/65535.0; 
	color[2] = FontsStyleLabel.TextColor.blue/65535.0; 

	 glInitFontsUsing(FontsStyleLabel.fontname, &ft2_context);

	if(radius<0.1) radius = 0.1;
	glDisable ( GL_LIGHTING ) ;
	glColor4dv(color);


	if(ortho)
	{
		sprintf(buffer,"X");
		glPrintOrtho(vectorX[0], vectorX[1], vectorX[2], buffer,TRUE,TRUE, ft2_context);
		sprintf(buffer,"Y");
		glPrintOrtho(vectorY[0], vectorY[1], vectorY[2], buffer,TRUE,TRUE, ft2_context);
		sprintf(buffer,"Z");
		glPrintOrtho(vectorZ[0], vectorZ[1], vectorZ[2], buffer,TRUE,TRUE, ft2_context);
	}
	else
	{
		/*
		sprintf(buffer,"X");
		glPrint(vectorX[0], vectorX[1], vectorX[2], buffer);
		sprintf(buffer,"Y");
		glPrint(vectorY[0], vectorY[1], vectorY[2], buffer);
		sprintf(buffer,"Z");
		glPrint(vectorZ[0], vectorZ[1], vectorZ[2], buffer);
		*/
		sprintf(buffer,"X");
		glPrintScale(vectorX[0], vectorX[1], vectorX[2], 1.2*radius, buffer, ft2_context);
		sprintf(buffer,"Y");
		glPrintScale(vectorY[0], vectorY[1], vectorY[2], 1.2*radius, buffer, ft2_context);
		sprintf(buffer,"Z");
		glPrintScale(vectorZ[0], vectorZ[1], vectorZ[2], 1.2*radius, buffer, ft2_context);
	}
	glEnable ( GL_LIGHTING ) ;
}

#endif
