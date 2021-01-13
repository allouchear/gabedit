/* GLArea.c */
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
#include "../Common/Global.h"
#include "GlobalOrb.h"
#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
#include "../Utils/UtilsGL.h"
#include "../Utils/Constants.h"
#include "../Geometry/GeomGlobal.h"
#include "../Display/AxisGL.h"
#include "../Display/PrincipalAxisGL.h"
#include "../Display/UtilsOrb.h"
#include "../Display/GLArea.h"
#include "../Utils/UtilsInterface.h"

static gboolean showSymbols = FALSE;
static gboolean showNumbers = FALSE;
static gboolean showCharges = FALSE;
static gboolean showDistances = FALSE;
static gboolean showDipole = FALSE;
static gboolean showAxes = FALSE;
static gchar fontName[BSIZE] = "courier 14";
static gchar fontNameTitle[BSIZE] = "courier 48";
static gboolean ortho = FALSE;
static gchar* strTitle = NULL;
static gint xTitle = 0;
static gint yTitle = 0;
static GdkColor colorTitle = {65535,0,0};
static gboolean initColor = TRUE;

/*********************************************************************************************/
void init_labels_font()
{
	sprintf(fontName,"%s",FontsStyleLabel.fontname);
	sprintf(fontNameTitle,"%s","courier bold 20");
}
/*********************************************************************************************/
gboolean get_labels_ortho()
{
	return ortho;
}
/*********************************************************************************************/
void set_labels_ortho(gboolean o)
{
	ortho = o;
}
/*********************************************************************************************/
gboolean get_show_symbols()
{
	return showSymbols;
}
/*********************************************************************************************/
void set_show_symbols(gboolean ac)
{
	showSymbols=ac;
}
/*********************************************************************************************/
gboolean get_show_charges()
{
	return showCharges;
}
/*********************************************************************************************/
gboolean get_show_numbers()
{
	return showNumbers;
}
/*********************************************************************************************/
void set_show_numbers(gboolean ac)
{
	showNumbers=ac;
}
/*********************************************************************************************/
void set_show_charges(gboolean ac)
{
	showCharges=ac;
}
/*********************************************************************************************/
void showLabelSymbolsNumbersCharges(PangoContext *ft2_context)
{
	gint i;
	gchar buffer[BSIZE];
	gchar bSymbol[10];
	gchar bNumber[BSIZE];
	gchar bCharge[10];
	V4d color  = {0.8,0.8,0.8,1.0 };


	if(nCenters<1) return;

	glInitFontsUsing(FontsStyleLabel.fontname, &ft2_context);

	color[0] = FontsStyleLabel.TextColor.red/65535.0; 
	color[1] = FontsStyleLabel.TextColor.green/65535.0; 
	color[2] = FontsStyleLabel.TextColor.blue/65535.0; 
	glDisable ( GL_LIGHTING ) ;
	glColor4dv(color);

	for(i=0;i<(gint)nCenters;i++)
	{

		if(showSymbols) sprintf(bSymbol,"%s",GeomOrb[i].Symb);
		else bSymbol[0]='\0';
		if(showNumbers) sprintf(bNumber,"[%d]",i+1);
		else bNumber[0]='\0';
		if(showCharges) sprintf(bCharge,"%0.3f",GeomOrb[i].partialCharge);
		else bCharge[0]='\0';
		sprintf(buffer,"%s%s%s",bSymbol,bNumber,bCharge);
		if(ortho)
			glPrintOrtho(GeomOrb[i].C[0], GeomOrb[i].C[1], GeomOrb[i].C[2], buffer , TRUE, TRUE, ft2_context);
		else
		{
			/* glPrint(GeomOrb[i].C[0], GeomOrb[i].C[1], GeomOrb[i].C[2], buffer);*/
			glPrintScale(GeomOrb[i].C[0], GeomOrb[i].C[1], GeomOrb[i].C[2], 1.1*GeomOrb[i].Prop.radii,buffer, ft2_context);
		}
	}
	glEnable ( GL_LIGHTING ) ;
	glDeleteFontsList();
}
/*********************************************************************************************/
gboolean get_show_distances()
{
	return showDistances;
}
/*********************************************************************************************/
void set_show_distances(gboolean ac)
{
	showDistances=ac;
}
/*********************************************************************************************/
void showLabelDistances(PangoContext *ft2_context)
{
	gint i;
	gint k;
	gint j;
	gdouble distance;
	gdouble tmp[3];
	gchar buffer[BSIZE];
	V4d color  = {0.8,0.8,0.8,1.0 };

	if(nCenters<1) return;
	color[0] = FontsStyleLabel.TextColor.red/65535.0; 
	color[1] = FontsStyleLabel.TextColor.green/65535.0; 
	color[2] = FontsStyleLabel.TextColor.blue/65535.0; 

	glInitFontsUsing(FontsStyleLabel.fontname, &ft2_context);
	glDisable ( GL_LIGHTING ) ;
	glColor4dv(color);

	for(i=0;i<(gint)nCenters;i++)
	for(j=i+1;j<(gint)nCenters;j++)
	{
		for(k=0;k<3;k++)
			tmp[k] = (GeomOrb[i].C[k] - GeomOrb[j].C[k]);
		distance = v3d_length(tmp);
		if(distance>=(GeomOrb[i].Prop.covalentRadii + GeomOrb[j].Prop.covalentRadii))
			continue;
		for(k=0;k<3;k++)
			tmp[k] = (GeomOrb[i].C[k] + GeomOrb[j].C[k])/2;
		sprintf(buffer, "%0.3f",distance*BOHR_TO_ANG);

		if(ortho)
			glPrintOrtho(tmp[0], tmp[1], tmp[2], buffer, TRUE, TRUE, ft2_context);
		else
		{
			/* glPrint(tmp[0], tmp[1], tmp[2], buffer);*/
			gdouble r = GeomOrb[i].Prop.radii;
			if(GeomOrb[j].Prop.radii>r)r = GeomOrb[j].Prop.radii; 
			glPrintScale(tmp[0], tmp[1], tmp[2], 1.1*r,buffer, ft2_context);
		}
	}
	glEnable ( GL_LIGHTING ) ;
	glDeleteFontsList();
}
/*********************************************************************************************/
gboolean get_show_dipole()
{
	return showDipole;
}
/*********************************************************************************************/
void set_show_dipole(gboolean ac)
{
	showDipole=ac;
}
/*********************************************************************************************/
void showLabelDipole(PangoContext *ft2_context)
{
	gint i;
	V4d color  = {0.8,0.8,0.8,1.0 };
	V3d Base1Pos  = {Dipole.origin[0],Dipole.origin[1],Dipole.origin[2]};
	V3d Base2Pos  = {Dipole.origin[0]+Dipole.value[0],Dipole.origin[1]+Dipole.value[1],Dipole.origin[2]+Dipole.value[2]};

	GLdouble radius = Dipole.radius;
	V3d Center;
	GLdouble p1=90;
	GLdouble p2=10;
	GLdouble p = p1 + p2;
	GLdouble scal = 2;
	gdouble module;
	gchar buffer[BSIZE];

	if(nCenters<1) return;
	if(!showDipole) return;
	if(!ShowDipoleOrb) return;

	color[0] = FontsStyleLabel.TextColor.red/65535.0; 
	color[1] = FontsStyleLabel.TextColor.green/65535.0; 
	color[2] = FontsStyleLabel.TextColor.blue/65535.0; 

	glInitFontsUsing(FontsStyleLabel.fontname, &ft2_context);
	glDisable ( GL_LIGHTING ) ;
	glColor4dv(color);

	if(radius<0.1) radius = 0.1;

	Base2Pos[0] = Base1Pos[0]+Dipole.value[0]*scal;
	Base2Pos[1] = Base1Pos[1]+Dipole.value[1]*scal;
	Base2Pos[2] = Base1Pos[2]+Dipole.value[2]*scal;

	Center[0] = (Base1Pos[0]*p2 + Base2Pos[0]*p1)/p;
	Center[1] = (Base1Pos[1]*p2 + Base2Pos[1]*p1)/p;
	Center[2] = (Base1Pos[2]*p2 + Base2Pos[2]*p1)/p;

	module = 0;
	for(i=0;i<3;i++)
		module += Dipole.value[i]*Dipole.value[i];
	module = sqrt(module);
	module *= AUTODEB;
	sprintf(buffer,"%0.3f D",module);

	if(ortho)
		glPrintOrtho(Center[0], Center[1], Center[2], buffer, TRUE, TRUE, ft2_context);
	else
	{
	/*	glPrint(Center[0], Center[1], Center[2], buffer); */
		glPrintScale(Center[0], Center[1], Center[2], 1.1*radius,buffer, ft2_context);
	}
	glEnable ( GL_LIGHTING ) ;
	glDeleteFontsList();
}
/*********************************************************************************************/
gboolean get_show_axes()
{
	return showAxes;
}
/*********************************************************************************************/
void set_show_axes(gboolean ac)
{
	showAxes=ac;
}
/*********************************************************************************************/
void showLabelAxes(PangoContext *ft2_context)
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

	if(!showAxes) return;
	if(!testShowAxis()) return;

	getAxisProperties(&show, &negative, origin, &radius, &scal, xColor, yColor, zColor);

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
		glPrintOrtho(vectorX[0], vectorX[1], vectorX[2], buffer, TRUE, TRUE, ft2_context);
		sprintf(buffer,"Y");
		glPrintOrtho(vectorY[0], vectorY[1], vectorY[2], buffer, TRUE, TRUE, ft2_context);
		sprintf(buffer,"Z");
		glPrintOrtho(vectorZ[0], vectorZ[1], vectorZ[2], buffer, TRUE, TRUE, ft2_context);
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
		glPrintScale(vectorX[0], vectorX[1], vectorX[2], 1.1*radius, buffer, ft2_context);
		sprintf(buffer,"Y");
		glPrintScale(vectorY[0], vectorY[1], vectorY[2], 1.1*radius, buffer, ft2_context);
		sprintf(buffer,"Z");
		glPrintScale(vectorZ[0], vectorZ[1], vectorZ[2], 1.1*radius, buffer, ft2_context);
	}
	glEnable ( GL_LIGHTING ) ;
	glDeleteFontsList();
}
/*********************************************************************************************/
void showLabelPrincipalAxes(PangoContext *ft2_context)
{
	gint i;
	V4d color  = {0.8,0.8,0.8,1.0 };
	gchar buffer[BSIZE];
	gboolean show;
	gboolean negative;
	gboolean def;
	gdouble origin[3];
	gdouble radius;
	gdouble scal;
	gdouble c1[3];
	gdouble c2[3];
	gdouble c3[3];
	gdouble v1[]  = {1,0,0};
	gdouble v2[]  = {0,1,0};
	gdouble v3[]  = {0,0,1};
	gdouble I[]  = {1,1,1};

	if(!showAxes) return;
	if(!testShowPrincipalAxisGL()) return;

	getPrincipalAxisProperties(&show, &negative, &def, origin, &radius, &scal, v1,v2, v3, c1,c2, c3);
	if(!def) return;
	getPrincipalAxisInertias(I);

	for(i=0;i<3;i++)
	{
		v1[i] *= scal;
		v2[i] *= scal;
		v3[i] *= scal;
	}
	for(i=0;i<3;i++)
	{
		v1[i] += origin[i];
		v2[i] += origin[i];
		v3[i] += origin[i];
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
		sprintf(buffer,"I=%0.3f",I[0]);
		glPrintOrtho(v1[0], v1[1], v1[2], buffer, TRUE, TRUE, ft2_context);
		sprintf(buffer,"I=%0.3f",I[1]);
		glPrintOrtho(v2[0], v2[1], v2[2], buffer, TRUE, TRUE, ft2_context);
		sprintf(buffer,"I=%0.3f",I[2]);
		glPrintOrtho(v3[0], v3[1], v3[2], buffer, TRUE, TRUE, ft2_context);
	}
	else
	{
		/*
		sprintf(buffer,"I=%0.3f",I[0]);
		glPrint(v1[0], v1[1], v1[2], buffer);
		sprintf(buffer,"I=%0.3f",I[1]);
		glPrint(v2[0], v2[1], v2[2], buffer);
		sprintf(buffer,"I=%0.3f",I[2]);
		glPrint(v3[0], v3[1], v3[2], buffer);
		*/
		sprintf(buffer,"I=%0.3f",I[0]);
		glPrintScale(v1[0], v1[1], v1[2], 1.1*radius, buffer, ft2_context);
		sprintf(buffer,"I=%0.3f",I[1]);
		glPrintScale(v2[0], v2[1], v2[2], 1.1*radius, buffer, ft2_context);
		sprintf(buffer,"I=%0.3f",I[2]);
		glPrintScale(v3[0], v3[1], v3[2], 1.1*radius, buffer, ft2_context);
	}
	glEnable ( GL_LIGHTING ) ;
	glDeleteFontsList();
}
/*********************************************************************************************/
void showLabelTitle(gint width, gint height, PangoContext *ft2_context)
{
	V4d color  = {0.8,0.8,0.8,1.0 };

	if(nCenters<1) return;
	if(!strTitle) return;
	if(xTitle<0) return;
	if(yTitle<0) return;

	if(initColor) 
	{
		colorTitle.red = FontsStyleLabel.TextColor.red; 
		colorTitle.green = FontsStyleLabel.TextColor.green; 
		colorTitle.blue = FontsStyleLabel.TextColor.blue; 
		initColor = FALSE;
	}
	color[0] = colorTitle.red/65535.0; 
	color[1] = colorTitle.green/65535.0; 
	color[2] = colorTitle.blue/65535.0; 

	glInitFontsUsing(fontNameTitle, &ft2_context);
	glDisable ( GL_LIGHTING ) ;
	glColor4dv(color);

	glLoadIdentity();
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, width, 0, height);

	
	/* glPrintWin(xTitle/100.0*width,(yTitle)/100.0*height+glTextHeight(),height, strTitle, ft2_context);*/
	glPrintWin(xTitle/100.0*width,(yTitle)/100.0*height+2*glTextHeight(),height, strTitle, ft2_context);
	
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	
	glEnable ( GL_LIGHTING ) ;
	glDeleteFontsList();
}
/*********************************************************************************************/
gchar* get_label_title()
{
	return strTitle;
}
/*********************************************************************************************/
void set_label_title(gchar* str, gint x, gint y)
{
	if(strTitle) g_free(strTitle);
	strTitle = NULL;
	if(str) strTitle = g_strdup(str);
	if(x>=0) xTitle = x;
	if(y>=0) yTitle = y;
}
/********************************************************************************/
static void apply_set_title(GtkWidget *Win,gpointer data)
{
	GtkWidget* entry = NULL;
	GtkWidget* xSpinButton = NULL;
	GtkWidget* ySpinButton = NULL;
	GtkWidget* fontButton = NULL;
	GtkWidget* colorButton = NULL;
	gdouble xValue = 0;
	gdouble yValue = 0;
	G_CONST_RETURN gchar* fontStr = NULL;
	G_CONST_RETURN gchar* str = NULL;

	if(!GTK_IS_WIDGET(Win)) return;

	entry = g_object_get_data (G_OBJECT (Win), "Entry");
	xSpinButton = g_object_get_data (G_OBJECT (Win), "XSpinButton");
	ySpinButton = g_object_get_data (G_OBJECT (Win), "YSpinButton");
	fontButton = g_object_get_data (G_OBJECT (Win), "FontButton");
	colorButton = g_object_get_data (G_OBJECT (Win), "ColorButton");

	str = gtk_entry_get_text(GTK_ENTRY(entry));
	fontStr = gtk_font_button_get_font_name(GTK_FONT_BUTTON(fontButton));
	xValue = gtk_spin_button_get_value (GTK_SPIN_BUTTON(xSpinButton));
	yValue = gtk_spin_button_get_value (GTK_SPIN_BUTTON(ySpinButton));
	if(xValue>=0 && xValue<=100) xTitle = xValue;
	if(yValue>=0 && yValue<=100) yTitle = yValue;
	if(fontStr) sprintf(fontNameTitle,"%s",fontStr);
	if(str && strlen(str)>1)
	{
		if(strTitle) g_free(strTitle);
		strTitle = g_strdup(str);
	} 
	if(colorButton) gtk_color_button_get_color (GTK_COLOR_BUTTON(colorButton), &colorTitle);
	glarea_rafresh(GLArea);

}
/********************************************************************************/
static void apply_set_title_close(GtkWidget *Win,gpointer data)
{
	apply_set_title(Win,data);
	delete_child(Win);
}
/********************************************************************************/
static GtkWidget *add_entry_title( GtkWidget *table, gchar* strLabel, gint il)
{
	gushort i;
	gushort j;
	GtkWidget *entry;
	GtkWidget *label;

/*----------------------------------------------------------------------------------*/
	i = il;
	j = 0;
	label = gtk_label_new(strLabel);
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = il;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = il;
	j = 2;
	entry =  gtk_entry_new();
	if(strTitle) gtk_entry_set_text(GTK_ENTRY(entry),strTitle);
	else gtk_entry_set_text(GTK_ENTRY(entry),"");

	gtk_table_attach(GTK_TABLE(table),entry,
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);

  	return entry;
}
/********************************************************************************/
static GtkWidget *add_font_button( GtkWidget *table, gchar* strLabel, gint il)
{
	gushort i;
	gushort j;
	GtkWidget *fontButton;
	GtkWidget *label;

/*----------------------------------------------------------------------------------*/
	i = il;
	j = 0;
	label = gtk_label_new(strLabel);
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = il;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = il;
	j = 2;
	fontButton =  gtk_font_button_new_with_font   (fontNameTitle);

	gtk_table_attach(GTK_TABLE(table),fontButton,
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);

  	return fontButton;
}
/********************************************************************************/
static GtkWidget *add_spin_button( GtkWidget *table, gchar* strLabel, gint il)
{
	gushort i;
	gushort j;
	GtkWidget *spinButton;
	GtkWidget *label;

/*----------------------------------------------------------------------------------*/
	i = il;
	j = 0;
	label = gtk_label_new(strLabel);
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = il;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = il;
	j = 2;
	spinButton =  gtk_spin_button_new_with_range (0, 100, 1);

	gtk_table_attach(GTK_TABLE(table),spinButton,
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);

  	return spinButton;
}
/********************************************************************************/
static GtkWidget *add_color_button( GtkWidget *table, gchar* strLabel, gint il)
{
	gushort i;
	gushort j;
	GtkWidget *colorButton;
	GtkWidget *label;

	if(initColor) 
	{
		colorTitle.red = FontsStyleLabel.TextColor.red; 
		colorTitle.green = FontsStyleLabel.TextColor.green; 
		colorTitle.blue = FontsStyleLabel.TextColor.blue; 
		initColor = FALSE;
	}

/*----------------------------------------------------------------------------------*/
	i = il;
	j = 0;
	label = gtk_label_new(strLabel);
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = il;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = il;
	j = 2;

	colorButton =  gtk_color_button_new_with_color(&colorTitle);

	gtk_table_attach(GTK_TABLE(table),colorButton,
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);

  	return colorButton;
}
/********************************************************************************/
void set_title_dlg()
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *hbox;
	GtkWidget *table;
	GtkWidget *vboxall;
	GtkWidget *xSpinButton;
	GtkWidget *ySpinButton;
	GtkWidget *fontButton;
	GtkWidget *button;
	GtkWidget *entry;
	GtkWidget *colorButton;

	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),_("Set title"));
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,_("Title "));

	vboxall = create_vbox(Win);
	frame = gtk_frame_new (_("Set title"));
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	table = gtk_table_new(5,3,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);

	entry = add_entry_title(table, _("Title"), 0);
	xSpinButton = add_spin_button( table, _("X position(%)"), 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(xSpinButton),xTitle);
	ySpinButton = add_spin_button( table, _("Y position(%)"), 2);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ySpinButton),yTitle);
	fontButton =  add_font_button(table, _("Font"),  3);
	colorButton =  add_color_button(table, _("Color"),  4);

	g_object_set_data (G_OBJECT (Win), "Entry",entry);
	g_object_set_data (G_OBJECT (Win), "XSpinButton",xSpinButton);
	g_object_set_data (G_OBJECT (Win), "YSpinButton",ySpinButton);
	g_object_set_data (G_OBJECT (Win), "FontButton",fontButton);
	g_object_set_data (G_OBJECT (Win), "ColorButton",colorButton);

	hbox = create_hbox_false(vboxall);
	gtk_widget_realize(Win);

	button = create_button(Win,_("OK"));
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_set_title_close,G_OBJECT(Win));

	button = create_button(Win,_("Apply"));
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_set_title,G_OBJECT(Win));

	button = create_button(Win,_("Cancel"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, G_OBJECT(Win));
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,G_OBJECT(Win));
	gtk_widget_show (button);

	gtk_widget_show_all (Win);
}
