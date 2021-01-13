/* SplashScreen.c */
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
#include <stdio.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/AtomsProp.h"
#include "../Common/Status.h"
#include "../Common/Windows.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/PersonalFragments.h"
#include "../MolecularMechanics/Atom.h"
#include "../MolecularMechanics/Molecule.h"
#include "../MolecularMechanics/ForceField.h"
#include "../MolecularMechanics/MolecularMechanics.h"
#include "../MolecularMechanics/PDBTemplate.h"
#include "../Molpro/MolproBasisLibrary.h"
#include "../Molcas/MolcasBasisLibrary.h"
#include "../MPQC/MPQCBasisLibrary.h"
#include "../Display/UtilsOrb.h"

static GdkGC *gc = NULL;
/*static guint IdTimer = 0;*/
static	GtkWidget *status = NULL;
static guint idStatus = 0;
static gint splash_screen_cb(gpointer data);
static gint progress( gpointer data,gdouble step);
/********************************************************************************/
static void set_statubar_str(gchar* str)
{
	if(str)
	{
		gtk_statusbar_pop(GTK_STATUSBAR(status),idStatus);
		gtk_statusbar_push(GTK_STATUSBAR(status),idStatus,str);
	}
	while(gtk_events_pending())
		gtk_main_iteration();
}
/*************************************************************************************/
static void read_ressource_files(GtkWidget* MainFrame,GtkWidget* ProgressBar)
{
 
	gdouble t[] = {0.20,0.10,0.10,0.10,0.10,0.20,0.20};
	gint i = 0;
	static gdouble step = 2;

	set_statubar_str(" ");
	define_default_atoms_prop();
	set_statubar_str(_("Load atom properties....."));

	if(!read_atoms_prop())
	{
		Waiting(step*t[i]);
		progress( ProgressBar,t[i]);
 		set_statubar_str(_("I can not load atom properties from file, define default values....."));
 		define_default_atoms_prop();
	}
	else
	{
		Waiting(step*t[i]);
		progress( ProgressBar,t[i]);
	}

	i++;
	set_statubar_str(_("Load default commands from \"commands\" file....."));
	read_commands_file();
	Waiting(step*t[i]);
	progress( ProgressBar,t[i]);

	i++;
	set_statubar_str(_("Load recent fonts styles....."));
	read_fonts_file();
	Waiting(step*t[i]);
	progress( ProgressBar,t[i]);

	i++;
	set_statubar_str(_("Load recent hosts list....."));
	read_hosts_file();
	Waiting(step*t[i]);
	progress( ProgressBar,t[i]);

	i++;
	set_statubar_str(_("Load recent surfaces colors....."));
	read_color_surfaces_file();
	Waiting(step*t[i]);
	progress( ProgressBar,t[i]);

	i++;
	set_statubar_str(_("Load list of recent projects....."));
	Waiting(step*t[i]);
	progress( ProgressBar,t[i]);

	i++;
	set_statubar_str(_("Load Molecular mechanics parameters....."));
	loadAmberParameters();
	LoadPDBTpl();	
	Waiting(step*t[i]);
	progress( ProgressBar,t[i]);

	set_statubar_str(_("Load Personal Fragments....."));
	loadPersonalFragments(NULL);

	set_statubar_str(_("Load the list of the bases supported by Molpro......"));
	loadMolproBasis();

	set_statubar_str(_("Load the list of the bases supported by Molcas......"));
	loadMolcasBasis();

	set_statubar_str(_("Load the list of the bases supported by MPQC......"));
	loadMPQCBasis();

	set_statubar_str("   ");

	Waiting(step*0.05);

	splash_screen_cb(MainFrame);
}
/********************************************************************************/
static gint show_gabedit(gpointer data)
{
	gtk_widget_show_all(Fenetre);
	//gtk_window_move(GTK_WINDOW(Fenetre),0,0);
	hide_progress_connection();
  	gtk_widget_hide (GTK_WIDGET(FrameWins));
	{
		gchar* fileName = g_strdup_printf("%s%sdump.txt",  g_get_home_dir(), G_DIR_SEPARATOR_S);
		set_last_directory(fileName);
		g_free(fileName);
	}
	return FALSE;
}
/********************************************************************************/
/* show the splash screen*/
static gint splash_screen_cb(gpointer data)
{
	gtk_widget_hide(GTK_WIDGET(data));
	/* gtk_timeout_remove(IdTimer);*/
	gtk_object_destroy(GTK_OBJECT(data));
	show_gabedit(NULL);
	return FALSE;
}
/********************************************************************************/
/* static gint progress( gpointer data)*/
static gint progress( gpointer data, gdouble step)
{
    gdouble new_val;

    if(!this_is_an_object((GtkObject*)data))
    {
	return FALSE;	
    }
    new_val = gtk_progress_bar_get_fraction( GTK_PROGRESS_BAR(data) ) + step;
    if (new_val > 1) new_val = 1;

     gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (data), (gdouble)(new_val));

    return TRUE;
} 
/***********************************************************/
static GtkWidget *create_progress_bar_splash(GtkWidget *box)
{
	GtkWidget *table;
  	GtkWidget *ProgressBar;

	table = gtk_table_new(1,2,FALSE);
	
	gtk_box_pack_start (GTK_BOX(box), table, FALSE, TRUE, 2);

	status = gtk_statusbar_new();
	gtk_widget_show(status);
	gtk_table_attach(GTK_TABLE(table),status,0,1,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
	idStatus = gtk_statusbar_get_context_id(GTK_STATUSBAR(status)," ");

	ProgressBar = gtk_progress_bar_new();

	gtk_table_attach(GTK_TABLE(table),ProgressBar,1,2,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			1,1);
	gtk_widget_show (table);

	return ProgressBar;
}
/********************************************************************************/
static gint configure_event( GtkWidget *widget, GdkEventConfigure *event )
{
 
  	gint height = 0;
	GdkColor color;
	GdkColor tmpcolor;
	GdkColormap *colormap   = NULL;
  	GdkPixmap *pixmap = (GdkPixmap *)g_object_get_data(G_OBJECT(widget), "Pixmap");
  	PangoFontDescription* font_desc = (PangoFontDescription*)g_object_get_data(G_OBJECT(widget), "FontDesc");
	gchar* txt = (gchar*)g_object_get_data(G_OBJECT(widget), "Text");
   	GdkColor *pcolor = (GdkColor*)g_object_get_data(G_OBJECT(widget), "Color");
   	gint* lentxt = (gint*)g_object_get_data(G_OBJECT(widget), "LenTxt");
	gboolean Ok = TRUE;
        GdkVisual* vis;

	gint i;
	gint x;
	gint y;
	
	if (pixmap) g_object_unref(pixmap);

	pixmap = gdk_pixmap_new(widget->window,
                          widget->allocation.width,
                          widget->allocation.height,
                          -1);
	
        color = *pcolor;
	colormap  = gdk_drawable_get_colormap(widget->window);

  	height = widget->allocation.height;
        vis = gdk_colormap_get_visual(colormap);
        if(vis->depth >15)
		Ok = TRUE;
	else
		Ok = FALSE;
	gdk_colormap_alloc_color(colormap, &color, FALSE, TRUE);
	if(Ok)
  		for(i=0;i<widget->allocation.width;i++)
  		{
			gdouble t= 0.1+i/(gdouble)(widget->allocation.width)/4; 
    			tmpcolor.red = (gushort)(color.red*t);
			tmpcolor.green = (gushort)(color.green*t);
			tmpcolor.blue = (gushort)(color.blue*t);
			gdk_colormap_alloc_color(colormap, &tmpcolor, FALSE, TRUE);
			gdk_gc_set_foreground(gc,&tmpcolor);
    			gdk_draw_line(pixmap,gc,i,0,i,height);
  		}
	else
	{
		gdk_gc_set_foreground(gc,&color);
  		for(i=0;i<widget->allocation.width;i++)
    			gdk_draw_line(pixmap,gc,i,0,i,height);
  	}
	if(strstr(txt,"Copyright"))
	{
    			gdk_draw_line(pixmap,widget->style->black_gc,0,0,widget->allocation.width,0);
    			gdk_draw_line(pixmap,widget->style->white_gc,0,1,widget->allocation.width,1);
    			gdk_draw_line(pixmap,widget->style->black_gc,0,2,widget->allocation.width,2);
	}
	

	if(strstr(txt,"Gabedit") && !strstr(txt,"Copyright"))
	{
		x = 20;
		y = 10;
		gabedit_draw_string(widget, pixmap, font_desc, widget->style->black_gc , x+height/20,y+height/18, txt, FALSE, FALSE);
		gabedit_draw_string(widget, pixmap, font_desc, widget->style->white_gc , x,y, txt, FALSE, FALSE);
	}
	else
	if(strstr(txt,"Allouche")  && !strstr(txt,"Copyright"))
	{
		x = widget->allocation.width/4;
		y = 10;
		gabedit_draw_string(widget, pixmap, font_desc, widget->style->black_gc , x+height/20,y+height/18, txt, FALSE, FALSE);
		gabedit_draw_string(widget, pixmap, font_desc, widget->style->white_gc , x,y, txt, FALSE, FALSE);
	}
	else
	if(strstr(txt,_("Graphical")))
	{
		x = 6;
		y = 10;
    		tmpcolor.red = 0;
		tmpcolor.green =60000;
		tmpcolor.blue = 65535;
		gdk_colormap_alloc_color(colormap, &tmpcolor, FALSE, TRUE);
		gdk_gc_set_foreground(gc,&tmpcolor);
		if(vis->depth >15)
		{
			gabedit_draw_string(widget, pixmap, font_desc, widget->style->black_gc , x+height/15,y+height/15, txt, FALSE, FALSE);
			gabedit_draw_string(widget, pixmap, font_desc, gc , x,y, txt, FALSE, FALSE);
		}
		else
			gabedit_draw_string(widget, pixmap, font_desc, gc , x,y, txt, FALSE, FALSE);
	}
	else
	{
		x = 6;
		y = 10;
		gabedit_draw_string(widget, pixmap, font_desc, widget->style->black_gc , x+height/20,y+height/18, txt, FALSE, FALSE);
		gabedit_draw_string(widget, pixmap, font_desc, widget->style->white_gc , x,y, txt, FALSE, FALSE);
	}

	if(strstr(txt,"Gabedit"))
	{
		gdouble t;
        	gchar* Version_S = g_strdup_printf("%d.%d.%d",MAJOR_VERSION,MINOR_VERSION,MICRO_VERSION);
		gint width = 0;
		gint dum = 0;
		x = *lentxt;
  		gabedit_string_get_pixel_size(Fenetre, font_desc, Version_S, &i, &dum);
		width = 9.0/8*i;
#ifdef G_OS_WIN32
		x = *lentxt-(gint)(i/15.0);
#endif
    		tmpcolor.red = 20000;
		tmpcolor.green = 10000;
		tmpcolor.blue = 20000;
   		if(!gdk_color_parse("sky blue",&tmpcolor))
		{
    		tmpcolor.red = 65000;
		tmpcolor.green = 10000;
		tmpcolor.blue = 65000;
		}
		t = 0.1 + 0.25;
		t = 0.5;
    		tmpcolor.red = (gushort)(tmpcolor.red*t);
		tmpcolor.green = (gushort)(tmpcolor.green*t);
		tmpcolor.blue = (gushort)(tmpcolor.blue*t);

		gdk_colormap_alloc_color(colormap, &tmpcolor, FALSE, TRUE);
		gdk_gc_set_foreground(gc,&tmpcolor);
		gdk_draw_arc(pixmap, gc, TRUE,x,0, width, widget->allocation.height, 0, 360*64);
    		tmpcolor.red = 10000;
		tmpcolor.green = 10000;
		tmpcolor.blue = 50000;
		gdk_colormap_alloc_color(colormap, &tmpcolor, FALSE, TRUE);
		gdk_gc_set_foreground(gc,&tmpcolor);
        	x = *lentxt + 1.0/20*i;
		gabedit_draw_string(widget, pixmap, font_desc, gc , x,5, Version_S, FALSE, FALSE);

		g_free(Version_S);
	}

  	g_object_set_data(G_OBJECT(widget), "Pixmap", pixmap);
  	return TRUE;
}
/********************************************************************************/   
static gint expose_event(GtkWidget  *widget,GdkEventExpose *event )
{
  	GdkPixmap *pixmap = NULL;

	if(event->count >0)
		return FALSE;

  	pixmap = (GdkPixmap *)g_object_get_data(G_OBJECT(widget), "Pixmap");
	if(pixmap)
		gdk_draw_drawable(widget->window,
                  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                  pixmap,
                  event->area.x, event->area.y,
                  event->area.x, event->area.y,
                  event->area.width, event->area.height);
  	return FALSE;
}
/********************************************************************************/
static void create_welcome_frame_popup(GtkWidget *vbox,GtkWidget *MainFrame)
{
  GtkWidget *vboxframe;
  GtkWidget *darea;
  gint height = 0;
  gint width = 0;
  gint widthVersion = 0;
  gint heightVersion = 0;
  gchar* txt = g_strdup(_("       Welcome to :"));
  GdkPixmap *pixmap = NULL;
  GdkColor* color = g_malloc(sizeof(GdkColor));
  gchar* Version_S = g_strdup_printf("%d.%d.%d",MAJOR_VERSION,MINOR_VERSION,MICRO_VERSION);
  gint* lentxt = g_malloc(sizeof(gint));
  PangoFontDescription *font_desc = pango_font_description_from_string ("sans bold 20");

/*   if(!gdk_color_parse("royal blue",color))*/
   if(!gdk_color_parse("sky blue",color))
/*  if(!gdk_color_parse("dark orange",color))*/
  { 
	 	color->red = 40000; 
	  	color->green = 40000; 
	  	color->blue = 40000; 
  }
  gabedit_string_get_pixel_size(Fenetre, font_desc, txt, lentxt, &height);
  gabedit_string_get_pixel_size(Fenetre, font_desc, Version_S, &widthVersion, &heightVersion);
  height += 10;
  width = *lentxt + widthVersion;

  g_free(Version_S);

  vboxframe = vbox;
  darea = gtk_drawing_area_new();
  gtk_widget_set_size_request(GTK_WIDGET(darea), width, height);
  gtk_box_pack_start (GTK_BOX(vboxframe),darea, FALSE, FALSE, 0);
  gtk_widget_realize(darea);
  pixmap = gdk_pixmap_new(darea->window,darea->allocation.width,darea->allocation.height,-1);
  g_object_set_data(G_OBJECT(darea), "Text", txt);
  g_object_set_data(G_OBJECT(darea), "Pixmap", pixmap);
  g_object_set_data(G_OBJECT(darea), "FontDesc", font_desc);
  g_object_set_data(G_OBJECT(darea), "Color", color);
  g_object_set_data(G_OBJECT(darea), "LenTxt", lentxt);

  g_signal_connect(G_OBJECT(darea),"configure_event",(GCallback)configure_event,NULL);
  g_signal_connect(G_OBJECT(darea),"expose_event",(GCallback)expose_event,NULL);
 

 /* gtk_widget_show(darea);*/
}
/********************************************************************************/
static void create_name_frame_popup(GtkWidget *vbox,GtkWidget *MainFrame)
{
  GtkWidget *vboxframe;
  GtkWidget *darea;
  gint height = 0;
  gint width = 0;
  gint widthVersion = 0;
  gint heightVersion = 0;
  gchar* txt = g_strdup(_("Abdul-Rahman Allouche presents : "));
  GdkPixmap *pixmap = NULL;
  GdkColor* color = g_malloc(sizeof(GdkColor));
  gchar* Version_S = g_strdup_printf("%d.%d.%d",MAJOR_VERSION,MINOR_VERSION,MICRO_VERSION);
  gint* lentxt = g_malloc(sizeof(gint));
  PangoFontDescription *font_desc = pango_font_description_from_string ("sans bold 16");

/*   if(!gdk_color_parse("royal blue",color))*/
   if(!gdk_color_parse("sky blue",color))
/*  if(!gdk_color_parse("dark orange",color))*/
  { 
	 	color->red = 40000; 
	  	color->green = 40000; 
	  	color->blue = 40000; 
  }
  gabedit_string_get_pixel_size(Fenetre, font_desc, txt, lentxt, &height);
  gabedit_string_get_pixel_size(Fenetre, font_desc, Version_S, &widthVersion, &heightVersion);
  height += 10;
  width = *lentxt + widthVersion;

  g_free(Version_S);

  vboxframe = vbox;
  darea = gtk_drawing_area_new();
  gtk_widget_set_size_request(GTK_WIDGET(darea), width, height);
  gtk_box_pack_start (GTK_BOX(vboxframe),darea, FALSE, FALSE, 0);
  gtk_widget_realize(darea);
  pixmap = gdk_pixmap_new(darea->window,darea->allocation.width,darea->allocation.height,-1);
  g_object_set_data(G_OBJECT(darea), "Text", txt);
  g_object_set_data(G_OBJECT(darea), "Pixmap", pixmap);
  g_object_set_data(G_OBJECT(darea), "FontDesc", font_desc);
  g_object_set_data(G_OBJECT(darea), "Color", color);
  g_object_set_data(G_OBJECT(darea), "LenTxt", lentxt);

  g_signal_connect(G_OBJECT(darea),"configure_event",(GCallback)configure_event,NULL);
  g_signal_connect(G_OBJECT(darea),"expose_event",(GCallback)expose_event,NULL);
 

 /* gtk_widget_show(darea);*/
}
/********************************************************************************/
static void create_program_frame_popup(GtkWidget *vbox,GtkWidget *MainFrame)
{
  GtkWidget *vboxframe;
  GtkWidget *darea;
  gint height = 0;
  gint width = 0;
  gint widthVersion = 0;
  gint heightVersion = 0;
  gchar* txt = g_strdup("  The Gabedit ");
  GdkPixmap *pixmap = NULL;
  GdkColor* color = g_malloc(sizeof(GdkColor));
  gchar* Version_S = g_strdup_printf("%d.%d.%d",MAJOR_VERSION,MINOR_VERSION,MICRO_VERSION);
  gint* lentxt = g_malloc(sizeof(gint));
  PangoFontDescription *font_desc = pango_font_description_from_string ("sans bold 50");

/*   if(!gdk_color_parse("royal blue",color))*/
   if(!gdk_color_parse("sky blue",color))
/*  if(!gdk_color_parse("dark orange",color))*/
  { 
	 	color->red = 40000; 
	  	color->green = 40000; 
	  	color->blue = 40000; 
  }
  gabedit_string_get_pixel_size(Fenetre, font_desc, txt, lentxt, &height);
  gabedit_string_get_pixel_size(Fenetre, font_desc, Version_S, &widthVersion, &heightVersion);
  height += heightVersion/4;
  *lentxt = *lentxt + 4.0/8*widthVersion;
  width = *lentxt + 9.0/8*widthVersion+2;

  g_free(Version_S);

  vboxframe = vbox;
  darea = gtk_drawing_area_new();
  gtk_widget_set_size_request(GTK_WIDGET(darea), width, height);
  gtk_box_pack_start (GTK_BOX(vboxframe),darea, FALSE, FALSE, 0);
  gtk_widget_realize(darea);
  pixmap = gdk_pixmap_new(darea->window,darea->allocation.width,darea->allocation.height,-1);
  g_object_set_data(G_OBJECT(darea), "Text", txt);
  g_object_set_data(G_OBJECT(darea), "Pixmap", pixmap);
  g_object_set_data(G_OBJECT(darea), "FontDesc", font_desc);
  g_object_set_data(G_OBJECT(darea), "Color", color);
  g_object_set_data(G_OBJECT(darea), "LenTxt", lentxt);

  g_signal_connect(G_OBJECT(darea),"configure_event",(GCallback)configure_event,NULL);
  g_signal_connect(G_OBJECT(darea),"expose_event",(GCallback)expose_event,NULL);
 

 /* gtk_widget_show(darea);*/
}
/********************************************************************************/
static void create_gui_frame_popup(GtkWidget *vbox,GtkWidget *MainFrame)
{
  GtkWidget *vboxframe;
  GtkWidget *darea;
  gint height = 0;
  gint width = 0;
  gint widthVersion = 0;
  gint heightVersion = 0;
  /* gchar* txt = g_strdup("The Graphical User Interface for Gamess, Gaussian, Molcas, Molpro, MPQC, PG Gamess and Q-Chem ab initio programs");
   */
  gchar* txt = g_strdup(_("The Graphical User Interface for computational chemistry packages"));
  GdkPixmap *pixmap = NULL;
  GdkColor* color = g_malloc(sizeof(GdkColor));
  gchar* Version_S = g_strdup_printf("%d.%d.%d",MAJOR_VERSION,MINOR_VERSION,MICRO_VERSION);
  gint* lentxt = g_malloc(sizeof(gint));
  PangoFontDescription *font_desc = pango_font_description_from_string ("sans bold 16");

/*   if(!gdk_color_parse("royal blue",color))*/
   if(!gdk_color_parse("sky blue",color))
/*  if(!gdk_color_parse("dark orange",color))*/
  { 
	 	color->red = 40000; 
	  	color->green = 40000; 
	  	color->blue = 40000; 
  }
  gabedit_string_get_pixel_size(Fenetre, font_desc, txt, lentxt, &height);
  gabedit_string_get_pixel_size(Fenetre, font_desc, Version_S, &widthVersion, &heightVersion);
  height += 20;
  width = *lentxt + widthVersion;

  g_free(Version_S);

  vboxframe = vbox;
  darea = gtk_drawing_area_new();
  gtk_widget_set_size_request(GTK_WIDGET(darea), width, height);
  gtk_box_pack_start (GTK_BOX(vboxframe),darea, FALSE, FALSE, 0);
  gtk_widget_realize(darea);
  pixmap = gdk_pixmap_new(darea->window,darea->allocation.width,darea->allocation.height,-1);
  g_object_set_data(G_OBJECT(darea), "Text", txt);
  g_object_set_data(G_OBJECT(darea), "Pixmap", pixmap);
  g_object_set_data(G_OBJECT(darea), "FontDesc", font_desc);
  g_object_set_data(G_OBJECT(darea), "Color", color);
  g_object_set_data(G_OBJECT(darea), "LenTxt", lentxt);

  g_signal_connect(G_OBJECT(darea),"configure_event",(GCallback)configure_event,NULL);
  g_signal_connect(G_OBJECT(darea),"expose_event",(GCallback)expose_event,NULL);
 

 /* gtk_widget_show(darea);*/
}
/********************************************************************************/
static void create_splash_popupwin()
{
	GtkWidget *MainFrame;
	GtkWidget *ProgressBar;
	GtkWidget *vbox = gtk_vbox_new(0,FALSE);
	
	MainFrame = gtk_window_new (GTK_WINDOW_POPUP);

	/* center it on the screen*/
	gtk_window_set_position(GTK_WINDOW (MainFrame), GTK_WIN_POS_CENTER);
  	gtk_container_set_border_width (GTK_CONTAINER (MainFrame), 4);

	/* set up key and mound button press to hide splash screen*/

	gtk_widget_add_events(MainFrame,
                              GDK_BUTTON_PRESS_MASK|
                              GDK_BUTTON_RELEASE_MASK|
                              GDK_KEY_PRESS_MASK);

	gtk_widget_realize(MainFrame);

	gc = gdk_gc_new(MainFrame->window);
	gtk_container_add (GTK_CONTAINER (MainFrame), vbox);
	
	/* create_name_frame_popup(vbox,MainFrame);*/
	create_welcome_frame_popup(vbox, MainFrame);
	create_program_frame_popup(vbox,MainFrame);
 	create_gui_frame_popup(vbox,MainFrame);
	ProgressBar = create_progress_bar_splash(vbox);
  	g_object_set_data(G_OBJECT(ProgressBar), "MainFrame", MainFrame);
	gtk_widget_show_all(MainFrame);

	/* for force expose */
  	gtk_widget_set_size_request (GTK_WIDGET(MainFrame), -1, 20);

	/* go into main loop, processing events.*/
	while(gtk_events_pending())
		gtk_main_iteration();

  	/* IdTimer = gtk_timeout_add(10, (GtkFunction)progress, ProgressBar);*/
	read_ressource_files(MainFrame,ProgressBar);
}
/********************************************************************************/
void splash_screen()
{
      create_splash_popupwin();
}
/********************************************************************************/
static void create_copyright_frame_popup(GtkWidget *vbox,GtkWidget *MainFrame)
{
  GtkWidget *vboxframe;
  GtkWidget *darea;
  gint height = 0;
  gint width = 0;
  gint widthVersion = 0;
  gint heightVersion = 0;
  gchar* txt = g_strdup("Copyright (c) 2002-2013 Abdul-Rahman Allouche.");
  GdkPixmap *pixmap = NULL;
  GdkColor* color = g_malloc(sizeof(GdkColor));
  gchar* Version_S = g_strdup_printf("%d.%d.%d",MAJOR_VERSION,MINOR_VERSION,MICRO_VERSION);
  gint* lentxt = g_malloc(sizeof(gint));
  PangoFontDescription *font_desc = pango_font_description_from_string ("sans bold 16");

/*  if(!gdk_color_parse("dark orange",color))*/
/*   if(!gdk_color_parse("royal blue",color))*/
   if(!gdk_color_parse("sky blue",color))
/*  if(!gdk_color_parse("black",color))*/
  { 
	 	color->red = 40000; 
	  	color->green = 40000; 
	  	color->blue = 40000; 
  }
  gabedit_string_get_pixel_size(Fenetre, font_desc, txt, lentxt, &height);
  gabedit_string_get_pixel_size(Fenetre, font_desc, Version_S, &widthVersion, &heightVersion);
  height += 20;
  width = *lentxt + widthVersion;

  g_free(Version_S);

  vboxframe = vbox;
  darea = gtk_drawing_area_new();
  gtk_widget_set_size_request(GTK_WIDGET(darea), width, height);
  gtk_box_pack_start (GTK_BOX(vboxframe),darea, FALSE, FALSE, 0);
  gtk_widget_realize(darea);
  pixmap = gdk_pixmap_new(darea->window,darea->allocation.width,darea->allocation.height,-1);
  g_object_set_data(G_OBJECT(darea), "Text", txt);
  g_object_set_data(G_OBJECT(darea), "Pixmap", pixmap);
  g_object_set_data(G_OBJECT(darea), "FontDesc", font_desc);
  g_object_set_data(G_OBJECT(darea), "Color", color);
  g_object_set_data(G_OBJECT(darea), "LenTxt", lentxt);

  g_signal_connect(G_OBJECT(darea),"configure_event",(GCallback)configure_event,NULL);
  g_signal_connect(G_OBJECT(darea),"expose_event",(GCallback)expose_event,NULL);
 
}
/********************************************************************************/
void create_about_frame()
{
	GtkWidget *MainFrame;
	GtkWidget *vbox = gtk_vbox_new(0,FALSE);
	
	MainFrame = gtk_window_new (GTK_WINDOW_POPUP);

	/* center it on the screen*/
	gtk_window_set_position(GTK_WINDOW (MainFrame), GTK_WIN_POS_CENTER);
  	gtk_container_set_border_width (GTK_CONTAINER (MainFrame), 4);

	gtk_widget_add_events(MainFrame, GDK_BUTTON_PRESS_MASK| GDK_BUTTON_RELEASE_MASK| GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK);

	g_signal_connect(G_OBJECT(MainFrame),"button_release_event", G_CALLBACK(gtk_widget_destroy),NULL);
	g_signal_connect(G_OBJECT(MainFrame),"key_press_event", G_CALLBACK(gtk_widget_destroy),NULL);

	gtk_widget_realize(MainFrame);

	gc = gdk_gc_new(MainFrame->window);
/*	gtk_widget_show(vbox);*/
	gtk_container_add (GTK_CONTAINER (MainFrame), vbox);
	
	create_name_frame_popup(vbox,MainFrame);
	create_program_frame_popup(vbox,MainFrame);
 	create_gui_frame_popup(vbox,MainFrame);
	create_copyright_frame_popup(vbox,MainFrame);


	gtk_widget_show_all(MainFrame);
}
/********************************************************************************/
  
