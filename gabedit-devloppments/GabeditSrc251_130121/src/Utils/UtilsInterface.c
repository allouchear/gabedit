/* UtilsInterface.c */
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
#include <ctype.h>
#include <unistd.h>

#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Gaussian/GaussGlobal.h"
#include "../Files/FileChooser.h"
#include "../Gamess/Gamess.h"
#include "../FireFly/FireFly.h"
#include "../Molcas/Molcas.h"
#include "../Molpro/Molpro.h"
#include "../MPQC/MPQC.h"
#include "../DeMon/DeMon.h"
#include "../Orca/Orca.h"
#include "../NWChem/NWChem.h"
#include "../Psicode/Psicode.h"
#include "../QChem/QChem.h"
#include "../Mopac/Mopac.h"
#include "../Gaussian/Gaussian.h"
#include "../Geometry/GeomGlobal.h"
#include "../Utils/UtilsInterface.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Utils/Utils.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Geometry/ResultsAnalise.h"
#include "../Geometry/EnergiesCurves.h"
#include "../Common/Windows.h"
#include "../Files/ListeFiles.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/GeomZmatrix.h"
#include "../Common/SplashScreen.h"
#include "../Display/ViewOrb.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Common/Exit.h"
#include "../Molcas/MolcasVariables.h"
#include "../Molcas/MolcasGateWay.h"
#include "../Utils/GabeditXYPlot.h"

#include "../../pixmaps/Ok.xpm"
#include "../../pixmaps/Cancel.xpm"
#include "../../pixmaps/Yes.xpm"
#include "../../pixmaps/No.xpm"
#include "../../pixmaps/SaveMini.xpm"
#include "../../pixmaps/Close.xpm"
#include "../../pixmaps/Apply.xpm"
#include "../../pixmaps/Question.xpm"
#include "../../pixmaps/Warning.xpm"
#include "../../pixmaps/Error.xpm"
#include "../../pixmaps/Info.xpm"
#include "../../pixmaps/A0p.xpm"
#include "../../pixmaps/A0d.xpm"
#include "../../pixmaps/Open.xpm"
#include "../../pixmaps/Gabedit.xpm"
#include "../../pixmaps/Print.xpm"
#include "../../pixmaps/Next.xpm"
#include "../../pixmaps/Stop.xpm"
#include "../../pixmaps/Play.xpm"
#include "../Common/StockIcons.h"

#include "../Spectrum/IRSpectrum.h"
#include "../Spectrum/RamanSpectrum.h"


/********************************************************************************/
static gint destroy_popup(gpointer data)
{
        gtk_widget_hide(GTK_WIDGET(data));
        gtk_object_destroy(GTK_OBJECT(data));
        return FALSE;
}
/********************************************************************************/
void create_popup_win(gchar* label)
{
	GtkWidget *MainFrame;
	GtkWidget *Label;
	GtkWidget *hbox = gtk_hbox_new(0,FALSE);
	

	MainFrame = gtk_window_new (GTK_WINDOW_POPUP);

	/* center it on the screen*/
        gtk_window_set_position(GTK_WINDOW (MainFrame), GTK_WIN_POS_MOUSE);

	/* set up key and mound button press to hide splash screen*/

        gtk_widget_add_events(MainFrame,
                              GDK_BUTTON_PRESS_MASK|
                              GDK_BUTTON_RELEASE_MASK|
                              GDK_KEY_PRESS_MASK);

        g_signal_connect(G_OBJECT(MainFrame),"button_press_event",
                G_CALLBACK(gtk_widget_destroy),NULL);
        g_signal_connect(G_OBJECT(MainFrame),"key_press_event",
                G_CALLBACK(gtk_widget_destroy),NULL);
        gtk_widget_realize(MainFrame);

	Label = gtk_label_new(label);
   	gtk_label_set_justify(GTK_LABEL(Label),GTK_JUSTIFY_LEFT);
 	set_font (Label,"helvetica bold 24");  
	gtk_box_pack_start (GTK_BOX (hbox), Label, FALSE, FALSE, 0);
        gtk_widget_show(Label);
        gtk_widget_show(hbox);
        gtk_container_add (GTK_CONTAINER (MainFrame), hbox);
        gtk_widget_show(MainFrame);

	/* force it to draw now.*/
	gdk_flush();

	/* go into main loop, processing events.*/
        while(gtk_events_pending())
                gtk_main_iteration();

	/* after 3 seconds, destroy the splash screen.*/
        g_timeout_add( 1000, destroy_popup, MainFrame );
}
/********************************************************************************/
void init_child(GtkWidget *widget, GabeditSignalFunc func,gchar *buttonlabel)
{
  WidgetChildren  *children = g_malloc(sizeof(WidgetChildren));
  g_object_set_data(G_OBJECT (widget), "Children", children);
  children->nchildren = 1;
  children->children = g_malloc(sizeof(GtkWidget*));
  children->destroychildren = g_malloc(sizeof(GCallback));
  children->children[0] = widget;
  g_object_set_data(G_OBJECT(children->children[0]),"Parent",widget);
  children->destroychildren[0] = func;
  if(buttonlabel)
	add_button_windows(buttonlabel,widget);
}
/********************************************************************************/
void add_child(GtkWidget *widget,GtkWidget *childwidget,GabeditSignalFunc func,gchar *buttonlabel)
{
  WidgetChildren  *children = (WidgetChildren  *)g_object_get_data(G_OBJECT(widget),"Children");
  children->nchildren++;
  children->children = g_realloc(children->children,children->nchildren*sizeof(GtkWidget*));
  children->destroychildren = g_realloc(children->destroychildren,children->nchildren*sizeof(GCallback));
  children->children[children->nchildren-1] = childwidget;
  g_object_set_data(G_OBJECT(children->children[children->nchildren-1]),"Parent",widget);
  children->destroychildren[children->nchildren-1] = func;
  if(buttonlabel)
	add_button_windows(buttonlabel,childwidget);
}
/********************************************************************************/
void delete_child(GtkWidget *childwidget)
{
  GtkWidget *widget = NULL;
  WidgetChildren  *children;
  gint i;
  gint k;

  widget = GTK_WIDGET(g_object_get_data(G_OBJECT(childwidget),"Parent"));
  if(!widget)
	return;
  children = (WidgetChildren  *)g_object_get_data(G_OBJECT(widget),"Children");

  k = -1;
  for(i=0;i<children->nchildren;i++)
  {
	if(children->children[i]==childwidget)
        {
		destroy_button_windows(children->children[i]);
		children->destroychildren[i](children->children[i]);
		children->children[i] = NULL;
		k = i;
		break;
	}
  }
  if(k!=-1)
  {
  	for(i=k;i<children->nchildren-1;i++)
  	{
		children->children[i] = children->children[i+1];
		children->destroychildren[i] = children->destroychildren[i+1];
  	}
  	children->nchildren--;
  	children->children = g_realloc(children->children,children->nchildren*sizeof(GtkWidget*));
  	children->destroychildren = g_realloc(children->destroychildren,children->nchildren*sizeof(GCallback));
  }
}
/********************************************************************************/
void delete_all_children(GtkWidget *widget)
{
  WidgetChildren  *children;
  gint i;

  if(!widget)
	return;
  children = (WidgetChildren  *)g_object_get_data(G_OBJECT(widget),"Children");
  for(i=1;i<children->nchildren;i++)
  {
	if(children->children[i])
	{
		destroy_button_windows(children->children[i]);
		children->destroychildren[i](children->children[i]);
		children->children[i] = NULL;
	}
  }
  children->nchildren = 1;
  children->children = g_realloc(children->children,children->nchildren*sizeof(GtkWidget*));
  children->destroychildren = g_realloc(children->destroychildren,children->nchildren*sizeof(GCallback));
   
}
/********************************************************************************/
void destroy_children(GtkWidget *widget)
{
  WidgetChildren  *children = (WidgetChildren  *)g_object_get_data(G_OBJECT(widget),"Children");
  gint i;
  for(i=children->nchildren-1;i>=0;i--)
  {
   if(children->children[i])
   {
	destroy_button_windows(children->children[i]);
	children->destroychildren[i](children->children[i]);
   }
  }
  g_free(children->children);
  g_free(children->destroychildren);
  g_free(children);
}
/********************************************************************************/
/* create_pixmap, convenience function to create a pixmap widget, from data */
GtkWidget *create_pixmap(GtkWidget *widget, gchar **data)
{
  GtkStyle *style;
  GdkBitmap *mask;
  GdkPixmap *gdk_pixmap;
  GtkWidget *gtk_pixmap;

  style = gtk_widget_get_style(widget);
  g_assert(style != NULL);

  gdk_pixmap = gdk_pixmap_create_from_xpm_d(widget->window,&mask, &style->bg[GTK_STATE_NORMAL],data);
  g_assert(gdk_pixmap != NULL);
  gtk_pixmap = gtk_image_new_from_pixmap(gdk_pixmap, mask);

  g_assert(gtk_pixmap != NULL);
  gtk_widget_show(gtk_pixmap);

  return(gtk_pixmap);
}
/********************************************************************************/
 GdkPixmap *get_pixmap(GtkWidget  *widget,gushort red,gushort green,gushort blue)
{
  GdkPixmap *pixmap = NULL;
  GdkGC *gc = NULL;
  gint width = ScreenHeight/20;
  gint height = ScreenHeight/50;
  GdkColormap *colormap;
  GdkColor Color;

  gc = gdk_gc_new(widget->window);
  Color.red =  red;
  Color.green =  green;
  Color.blue =  blue;

  pixmap = gdk_pixmap_new(widget->window,width,height, -1);
  gdk_draw_rectangle (pixmap,
                      widget->style->black_gc,
                      TRUE,
                      0, 0, width, height);    
  colormap  = gdk_drawable_get_colormap(widget->window);
  gdk_colormap_alloc_color(colormap,&Color,FALSE,TRUE);
  gdk_gc_set_foreground(gc,&Color);
  gdk_draw_rectangle (pixmap,
                      gc,
                      TRUE,
                      1, 1, width-1, height-1);    

  g_object_unref(gc);
  return pixmap;
}   
/********************************************************************************/
GtkWidget *create_hbox_pixmap_color(GtkWidget *widget,gushort red,gushort green,gushort blue)
{
  GdkBitmap *mask = NULL;
  GdkPixmap *gdk_pixmap;
  GtkWidget *gtk_pixmap;
  GtkWidget *hbox;

  hbox = gtk_hbox_new (TRUE, 0);
 
  gdk_pixmap = get_pixmap(widget,red,green,blue);
  gtk_pixmap = gtk_image_new_from_pixmap(gdk_pixmap, mask);
 
  g_object_unref(gdk_pixmap);
 
 
  gtk_box_pack_start (GTK_BOX (hbox), gtk_pixmap, TRUE, TRUE, 1);
 
  return hbox ;
}  
/********************************************************************************/
GtkWidget *create_button_pixmap_color(GtkWidget *widget,gushort red,gushort green,gushort blue)
{
  GtkWidget *hbox = create_hbox_pixmap_color(widget,red,green,blue);
  GtkWidget *Button = gtk_button_new();

  gtk_container_add(GTK_CONTAINER(Button),hbox);
  gtk_widget_show (hbox);
  g_object_set_data(G_OBJECT (Button), "Hbox", hbox);
 
  return Button ;
}  
/********************************************************************************/
GtkWidget *create_radio_button_pixmap(GtkWidget *widget, gchar **data,gchar *string,GtkWidget *oldbutton)
{
  GtkStyle *style;
  GdkBitmap *mask;
  GdkPixmap *gdk_pixmap;
  GtkWidget *gtk_pixmap;
  GtkWidget *Button;
  GtkWidget *hbox;
  GtkWidget *Label = NULL;

  hbox = gtk_hbox_new (TRUE, 0);
  g_object_ref (hbox);
  gtk_widget_show (hbox);
 
 
  style = gtk_widget_get_style(widget);
  g_assert(style != NULL);
 
  gdk_pixmap = gdk_pixmap_create_from_xpm_d(widget->window,
                                            &mask, &style->bg[GTK_STATE_NORMAL],data);
  g_assert(gdk_pixmap != NULL);
  gtk_pixmap = gtk_image_new_from_pixmap(gdk_pixmap, mask);
 
  g_object_unref(gdk_pixmap);
  g_object_unref(mask);
 
  g_assert(gtk_pixmap != NULL);
/*  gtk_widget_show(gtk_pixmap);*/
 
  /*
  if(oldbutton)
  Button = gtk_radio_button_new(gtk_radio_button_group (GTK_RADIO_BUTTON (oldbutton)));
  else
  Button = gtk_radio_button_new(NULL);
  */
  Button = gtk_toggle_button_new(); 

  gtk_container_add(GTK_CONTAINER(Button),hbox);
  if(string)
  	Label = gtk_label_new(string);
  gtk_box_pack_start (GTK_BOX (hbox), gtk_pixmap, FALSE, FALSE, 1);
  if(string)
  	gtk_box_pack_start (GTK_BOX (hbox), Label, FALSE, FALSE, 1);
 
  return Button ;
} 
/********************************************************************************/
void set_icon_default()
{
  GdkPixbuf *pixbuf;
   
  pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)gabedit_xpm);
  gtk_window_set_default_icon (pixbuf);
}     
/********************************************************************************/
void set_icone(GtkWidget *widget)
{
  GtkStyle *style;
  GdkBitmap *mask;
  GdkPixmap *gdk_pixmap;

   
  style = gtk_widget_get_style(widget);
  gdk_pixmap = gdk_pixmap_create_from_xpm_d (widget->window,&mask, &style->bg[GTK_STATE_NORMAL],gabedit_xpm);
  gdk_window_set_icon(widget->window,NULL,gdk_pixmap,mask); 
}     
/********************************************************************************/
void data_modify(gboolean mod)
{
        if(mod)
         imodif = DATA_MOD_YES;
        else
         imodif = DATA_MOD_NO;
 	change_all_labels();
}     
/********************************************************************************/
GtkWidget *create_label_pixmap(GtkWidget *widget, gchar **data,gchar *string)
{
  GtkStyle *style;
  GdkBitmap *mask;
  GdkPixmap *gdk_pixmap;
  GtkWidget *gtk_pixmap;
  GtkWidget *Label;
  GtkWidget *hbox;

  hbox = gtk_hbox_new (FALSE, 0);
  g_object_ref (hbox);
  gtk_widget_show (hbox);
 
  style = gtk_widget_get_style(widget);
  g_assert(style != NULL);
 
  gdk_pixmap = gdk_pixmap_create_from_xpm_d(widget->window,
                                            &mask, &style->bg[GTK_STATE_NORMAL],data);
  g_assert(gdk_pixmap != NULL);
  gtk_pixmap = gtk_image_new_from_pixmap(gdk_pixmap, mask);
 
  g_object_unref(gdk_pixmap);
  g_object_unref(mask);
 
  g_assert(gtk_pixmap != NULL);
 
  Label = gtk_label_new(string);
  gtk_label_set_justify(GTK_LABEL(Label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_end (GTK_BOX (hbox), gtk_pixmap, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (hbox), Label, FALSE, FALSE, 2);
 
  return hbox;
}  
/********************************************************************************/
GtkWidget *create_pixmap_label(GtkWidget *widget, gchar **data,gchar *string)
{
  GtkStyle *style;
  GdkBitmap *mask;
  GdkPixmap *gdk_pixmap;
  GtkWidget *gtk_pixmap;
  GtkWidget *Label;
  GtkWidget *hbox;

  hbox = gtk_hbox_new (FALSE, 0);
  g_object_ref (hbox);
  gtk_widget_show (hbox);
 
  style = gtk_widget_get_style(widget);
  g_assert(style != NULL);
 
  gdk_pixmap = gdk_pixmap_create_from_xpm_d(widget->window,
                                            &mask, &style->bg[GTK_STATE_NORMAL],data);
  g_assert(gdk_pixmap != NULL);
  gtk_pixmap = gtk_image_new_from_pixmap(gdk_pixmap, mask);
 
  g_object_unref(gdk_pixmap);
  g_object_unref(mask);
 
  g_assert(gtk_pixmap != NULL);
 
  Label = gtk_label_new(string);
  gtk_label_set_justify(GTK_LABEL(Label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (hbox), gtk_pixmap, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (hbox), Label, FALSE, FALSE, 2);
 
  return hbox;
}  
/********************************************************************************/
GtkWidget *create_label_with_pixmap(GtkWidget *Win,gchar *string,gchar *type)
{
    GtkWidget *hbox;
    GtkWidget *Label;
    if( strstr(type,_("Question")) )
    	hbox = create_pixmap_label(Win,question_xpm,string);
    else
    if( strstr(type,_("Warning")) )
    	hbox = create_pixmap_label(Win,warning_xpm,string);
    else
    if( strstr(type,_("Error")))
    	hbox = create_pixmap_label(Win,error_xpm,string);
    else
    if( strstr(type,_("Info")) )
    	hbox = create_pixmap_label(Win,info_xpm,string);
    else
    {
  	hbox = gtk_hbox_new (TRUE, 0);
  	gtk_widget_show (hbox);
        Label = gtk_label_new(string);
        gtk_box_pack_start (GTK_BOX (hbox), Label, FALSE, FALSE, 1);
    }

    return hbox;
}
/********************************************************************************/
GtkWidget *create_button_pixmap(GtkWidget *widget, gchar **data,gchar *string)
{
  GtkStyle *style;
  GdkBitmap *mask;
  GdkPixmap *gdk_pixmap;
  GtkWidget *gtk_pixmap;
  GtkWidget *Button;
  GtkWidget *Label = NULL;
  GtkWidget *hbox;

  hbox = gtk_hbox_new (FALSE, 1);
  g_object_ref (hbox);
  gtk_widget_show (hbox);
 
 
  style = gtk_widget_get_style(widget);
  g_assert(style != NULL);
 
  gdk_pixmap = gdk_pixmap_create_from_xpm_d (widget->window,&mask, &style->bg[GTK_STATE_NORMAL],data);
  g_assert(gdk_pixmap != NULL);
  gtk_pixmap = gtk_image_new_from_pixmap(gdk_pixmap, mask);
 
  g_object_unref(gdk_pixmap);
  g_object_unref(mask);
 
  g_assert(gtk_pixmap != NULL);
 
  Button = gtk_button_new();
  gtk_container_add(GTK_CONTAINER(Button),hbox);
  gtk_box_pack_start (GTK_BOX (hbox), gtk_pixmap, FALSE, FALSE, 2);

  if(string)
  {
  	Label = gtk_label_new(string);
  	gtk_box_pack_end (GTK_BOX (hbox), Label, FALSE, FALSE, 1);
  }
 
  return Button ;
}  

/********************************************************************************/
void exit_all(GtkWidget *widget, gchar *data)
{
 if(imodif == DATA_MOD_NO ) 
	ExitDlg(NULL, NULL);
 else
 	Save_YesNo();
}
/********************************************************************************/
GtkWidget *create_button(GtkWidget *Win,gchar *string)
{
    GtkWidget *button;
    if( strstr(string,_("OK")) )
    	button = create_button_pixmap(Win,ok_xpm,_("OK"));
    else
    if(strstr(string,_("Cancel")) )
    	button = create_button_pixmap(Win,cancel_xpm,_("Cancel"));
    else
    if(strstr(string,_("Yes")) )
    	button = create_button_pixmap(Win,yes_xpm,_("Yes"));
    else
    if(strstr(string,_("No")) )
    	button = create_button_pixmap(Win,no_xpm,_("No"));
    else
    if(strstr(string,_("Save&Apply&Close")) )
    	button = create_button_pixmap(Win,savemini_xpm,_("Save&Apply&Close"));
    else
    if(strstr(string,_("Save&Close")) )
    	button = create_button_pixmap(Win,savemini_xpm,_("Save&Close"));
    else
    if(strstr(string,_("Apply&Close")) )
    	button = create_button_pixmap(Win,apply_xpm,_("Apply&Close"));
    else
    if(strstr(string,_("Save")) )
    	button = create_button_pixmap(Win,savemini_xpm,_("Save"));
    else
    if(strstr(string,_("Close")) )
    	button = create_button_pixmap(Win,close_xpm,_("Close"));
    else
    if(strstr(string,_("Apply")) )
    	button = create_button_pixmap(Win,apply_xpm,_("Apply"));
    else
    if(strstr(string," *a0 ") )
		button = create_button_pixmap(Win,a0p_xpm,NULL);
    else
    if(strstr(string," /a0 ") )
		button = create_button_pixmap(Win,a0d_xpm,NULL);
    else
    if(strstr(string,_("Print")) )
    	button = create_button_pixmap(Win,print_xpm,_("Print"));
    else
    if(strstr(string,_("Stop")) )
    	button = create_button_pixmap(Win,stop_xpm,_("Stop"));
    else
    if(strstr(string,_("Play")) )
    	button = create_button_pixmap(Win,play_xpm,_("Play"));
    else
    if(strstr(string,_("Next")) )
    	button = create_button_pixmap(Win,next_xpm,_("Next"));
    else
    	button = gtk_button_new_with_label(string);

    return button;
}
/********************************************************************************/
GtkWidget* Message(char *message,char *titre,gboolean center)
{
    GtkWidget *DialogueMessage = NULL;
    GtkWidget *Label, *Bouton;
    GtkWidget *frame, *vboxframe;


    DialogueMessage = gtk_dialog_new();
    gtk_widget_realize(GTK_WIDGET(DialogueMessage));

    gtk_window_set_title(GTK_WINDOW(DialogueMessage),titre);
   gtk_window_set_transient_for(GTK_WINDOW(DialogueMessage),GTK_WINDOW(Fenetre));
    gtk_window_set_position(GTK_WINDOW(DialogueMessage),GTK_WIN_POS_CENTER);

   g_signal_connect(G_OBJECT(DialogueMessage), "delete_event", (GCallback)destroy_button_windows, NULL);
    g_signal_connect(G_OBJECT(DialogueMessage), "delete_event", (GCallback)gtk_widget_destroy, NULL);
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
   gtk_box_pack_start( GTK_BOX(GTK_DIALOG(DialogueMessage)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

    if(center)
    {
    	Label = create_label_with_pixmap(Fenetre,message,titre);
    	gtk_box_pack_start(GTK_BOX(vboxframe), Label,TRUE,TRUE,0);
    }
    else
    {
	GtkWidget* hbox;
    	Label = gtk_label_new(message);
  	gtk_widget_show (Label);
   	gtk_label_set_justify(GTK_LABEL(Label),GTK_JUSTIFY_LEFT);
  	hbox = gtk_hbox_new (FALSE, 5);
  	gtk_box_pack_start (GTK_BOX (vboxframe), hbox, FALSE, FALSE, 5);
  	gtk_box_pack_start (GTK_BOX(hbox), Label, FALSE, FALSE, 5);
  	gtk_widget_show (hbox);
    }
    
    gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(DialogueMessage)->action_area), FALSE);
  
  Bouton = create_button(DialogueMessage,_("OK"));
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(DialogueMessage)->action_area), Bouton, FALSE, TRUE, 5);  
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);
 g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",
                          (GCallback)destroy_button_windows,
                          GTK_OBJECT(DialogueMessage));
    g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",
                          (GCallback)gtk_widget_destroy,
                          GTK_OBJECT(DialogueMessage));

    add_button_windows(titre,DialogueMessage);
    gtk_widget_show_all(DialogueMessage);
    return DialogueMessage;
}
/********************************************************************************/
GtkWidget* MessageTxt(gchar *message,gchar *title)
{
	GtkWidget *dlgWin = NULL;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *txtWid;
	GtkWidget *button;


	dlgWin = gtk_dialog_new();
	gtk_widget_realize(GTK_WIDGET(dlgWin));

	gtk_window_set_title(GTK_WINDOW(dlgWin),title);
	gtk_window_set_transient_for(GTK_WINDOW(dlgWin),GTK_WINDOW(Fenetre));
	gtk_window_set_position(GTK_WINDOW(dlgWin),GTK_WIN_POS_CENTER);

	g_signal_connect(G_OBJECT(dlgWin), "delete_event", (GCallback)destroy_button_windows, NULL);
	g_signal_connect(G_OBJECT(dlgWin), "delete_event", (GCallback)gtk_widget_destroy, NULL);
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(dlgWin)->vbox), frame,TRUE,TRUE,0);

	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	txtWid = create_text_widget(vboxframe,NULL,&frame);
	if(message) gabedit_text_insert (GABEDIT_TEXT(txtWid), NULL, NULL, NULL,message,-1);   

	gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), FALSE);
  
	button = create_button(dlgWin,_("OK"));
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), button, FALSE, TRUE, 5);  
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)destroy_button_windows, GTK_OBJECT(dlgWin));
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(dlgWin));

	add_button_windows(title,dlgWin);
	gtk_window_set_default_size (GTK_WINDOW(dlgWin), (gint)(ScreenHeight*0.4), (gint)(ScreenHeight*0.4));
	gtk_widget_show_all(dlgWin);
	return dlgWin;
}
/********************************************************************************/
void select_all()
{
	
	if( gtk_notebook_get_current_page(GTK_NOTEBOOK(NoteBookText))==0)
		gabedit_text_select_all(text);
	else
		gabedit_text_select_all(textresult);
}
/********************************************************************************/
void goto_end(GtkWidget *current,gpointer data)
{
	gabedit_text_goto_end(current);
}      
/********************************************************************************/
void goto_end_result(GtkWidget *win,gpointer data)
{
 	goto_end(textresult,data);
}      
/********************************************************************************/
void ClearText(GtkWidget* text)
{
        gint nchar;
        nchar=gabedit_text_get_length(GABEDIT_TEXT(text));
        gabedit_text_set_point(GABEDIT_TEXT(text),0);
        gabedit_text_forward_delete(GABEDIT_TEXT(text),nchar);
        
        gabedit_text_set_point(GABEDIT_TEXT(text),0);

}
/********************************************************************************/
void destroy( GtkWidget *widget, gpointer   data )
{
  guint i;
           if(iframe==2)
 		geom_is_not_open();
           gtk_widget_destroy(widget);
           iedit=1;
           gabedit_text_set_editable(GABEDIT_TEXT(text), TRUE);
           if(iframe==2 && iprogram == PROG_IS_GAUSS)
       	     for(i=0;i<6;i++)
       		EntryCS[i]=NULL;
           iframe=1;
           widget = NULL;
}
/********************************************************************************/
void Cancel_YesNo(GtkWidget *widget, gpointer   data, GabeditSignalFunc func)
{
    GtkWidget *DialogueMessage = NULL;
    GtkWidget *Label, *Bouton;
	GtkWidget *frame, *vboxframe;
    DialogueMessage = gtk_dialog_new();
	gtk_window_set_position(GTK_WINDOW(DialogueMessage),GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(DialogueMessage),_("Warning"));
    gtk_window_set_modal (GTK_WINDOW (DialogueMessage), TRUE);

    g_signal_connect(G_OBJECT(DialogueMessage),
                       "delete_event",
                       (GCallback)gtk_widget_destroy,
                       NULL);
    
	frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
    g_object_ref (frame);
    g_object_set_data_full (G_OBJECT (DialogueMessage), "frame",
	  frame,(GDestroyNotify) g_object_unref);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
    gtk_box_pack_start( GTK_BOX(GTK_DIALOG(DialogueMessage)->vbox), frame,TRUE,TRUE,0);
    gtk_widget_show (frame);
    vboxframe = create_vbox(frame);

    Label = gtk_label_new(_("Are you sure to \ncancel this window ?"));
    gtk_box_pack_start(GTK_BOX(vboxframe), Label,TRUE,TRUE,0);
    gtk_label_set_justify(GTK_LABEL(Label),
                          GTK_JUSTIFY_CENTER);
    gtk_misc_set_padding(GTK_MISC(Label), 10, 10);
    
    gtk_widget_realize(DialogueMessage);

    Bouton = create_button(DialogueMessage,_("No"));
    gtk_box_pack_start( GTK_BOX(GTK_DIALOG(DialogueMessage)->action_area), Bouton,TRUE,TRUE,0);
    g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(DialogueMessage));
    GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
    gtk_widget_grab_default(Bouton);

    Bouton = create_button(DialogueMessage,_("Yes"));
    gtk_box_pack_start( GTK_BOX(GTK_DIALOG(DialogueMessage)->action_area), Bouton,TRUE,TRUE,0);
    GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
    g_signal_connect_swapped(G_OBJECT(Bouton),"clicked", (GCallback)func, GTK_OBJECT(widget));
    g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(DialogueMessage));


    gtk_widget_show_all(DialogueMessage);
}
/**********************************************************************************/
FilePosTypeGeom get_geometry_type_from_molpro_input_file(gchar *NomFichier)
{
 gchar *t;
 FILE *fd;
 guint taille=BSIZE;
 FilePosTypeGeom j;
 gint k;
 gchar *t1;
 gchar *t2;
 gchar *t3 = NULL;

 
 j.geomtyp=GEOM_IS_ZMAT;
 j.numline=0;
 j.units=0;
 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 if(fd!=NULL)
 {
 	k= 0;
 	while(!feof(fd) )    
 	{
 		if(!fgets(t, taille, fd))
			break;
 		k++; 
                t1 = g_strdup(t);
                t2= g_strdup("Geometry={");
                uppercase(t1);
                uppercase(t2);
                t3 = NULL;
                t3 = strstr(t1,t2);
                if(t3 != NULL)
 			j.numline=k;
                g_free(t2); 
                t3 = NULL;
                t2= g_strdup("ANGSTROM");
                t3 = strstr(t1,t2);
                if(t3 != NULL)
 			j.units=1;
                g_free(t2); 
                t2= g_strdup("geomtyp=xyz");
                uppercase(t2);
                t3 = NULL;
                t3 = strstr(t1,t2);
                if(t3 != NULL)
 			j.geomtyp=GEOM_IS_XYZ;
                g_free(t2); 
                g_free(t1);
  	}
  	fclose(fd);
  }
  if(j.numline==0)
	j.geomtyp=GEOM_IS_OTHER;
  g_free(t);
  return j;
}
/**********************************************************************************/
FilePosTypeGeom get_geometry_type_from_demon_input_file(gchar *NomFichier)
{
 gchar *t;
 FILE *fd;
 guint taille=BSIZE;
 gboolean OK=TRUE;
 gint i;
 FilePosTypeGeom j;
 gint k;
 gchar *t1;
 gchar *t2;
 gchar *t3 = NULL;
 gchar dum[100];

 
 sprintf(dum,"NONE");
 j.geomtyp=GEOM_IS_OTHER;
 j.numline=0;
 j.units=1;
 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 if(fd!=NULL)
 {
/* Charge and Spin */
 k = 0;
 while(!feof(fd) )    
 {
 	if(!fgets(t, taille, fd)) break;
	deleteFirstSpaces(t);
     	if((int)t[0]==(int)'#') continue;
        uppercase(t);
	sscanf(t,"%s", dum);
	if(strstr(dum,"MULTIPLICIT")) 
	{
		sscanf(t,"%s %d", dum, &SpinMultiplicities[0]);
		SpinMultiplicities[1] = SpinMultiplicities[0];
		SpinMultiplicities[2] = SpinMultiplicities[0];
		k++;
	}
	if(strstr(dum,"CHARGE")) 
	{
		sscanf(t,"%s %d", dum, &SpinMultiplicities[0]);
		SpinMultiplicities[1] = SpinMultiplicities[0];
		SpinMultiplicities[2] = SpinMultiplicities[0];
		k++;
	}
	if(k==2) break;
 }
 fseek(fd, 0L, SEEK_SET);
/* Geometry, Type and unit */
 while(!feof(fd) )    
 {
 	if(!fgets(t, taille, fd)) break;
 	j.numline++;
	deleteFirstSpaces(t);
     	if((int)t[0]==(int)'#' ) continue;
        uppercase(t);
	sscanf(t,"%s", dum);
	if(strstr(dum,"GEOMETRY")) 
	{
		if(strstr(t,"CARTES")) j.geomtyp = GEOM_IS_XYZ;
		else j.geomtyp = GEOM_IS_ZMAT;
		if(strstr(t,"BOHR")) j.units = 0;
 		j.numline++;
		break;
	}
 }
}
  fclose(fd);
  g_free(t);
  return j;
}
/**********************************************************************************/
FilePosTypeGeom get_geometry_type_from_gauss_input_file(gchar *NomFichier)
{
 gchar *t;
 FILE *fd;
 guint taille=BSIZE;
 gboolean OK=TRUE;
 gint i;
 FilePosTypeGeom j;
 gint k;
 gchar *t1;
 gchar *t2;
 gchar *t3 = NULL;

 
 j.geomtyp=GEOM_IS_OTHER;
 j.numline=0;
 j.units=1;
 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 if(fd!=NULL)
 {
/* Commands lines */
 while(!feof(fd) )    
 {
 	if(!fgets(t, taille, fd))
		break;
 	j.numline++;
     	if((int)t[0]==(int)'#' || (int)t[0]==(int)'%' )
        {
                t1 = g_strdup(t);
                t2= g_strdup("Units(Au");
                uppercase(t1);
                uppercase(t2);
                t3 = strstr(t1,t2);
                if(t3 != NULL)
 			j.units=0;
                g_free(t1);
                g_free(t2); 
        	continue;
        }
     	else
		break;
  }
/* Title */
 while(!feof(fd) )    
 {
 	if(!fgets(t, taille, fd))
		break;
 	j.numline++;
      	OK=TRUE;
      	for(i=0;i<(gint)strlen(t);i++)
      	if(t[i]!=' ' && t[i] !='\n' )
      	{
		OK=FALSE;
        	break;
      	}
        if(OK)
        	break;
  }

/* Charge and Spin */
 if(!feof(fd) )    
 {
	 if(fgets(t, taille, fd)) j.numline++;
	 sscanf(t,"%d %d %d %d %d %d",
		  &TotalCharges[0],
		  &SpinMultiplicities[0],
		  &TotalCharges[1],
		  &SpinMultiplicities[1],
		  &TotalCharges[2],
		  &SpinMultiplicities[2]);
 }
/* First line of geometry */
  if(!feof(fd) )    
  {
 	if(!fgets(t, taille, fd))
        {
  		j.geomtyp = GEOM_IS_OTHER;
        }
        else
        {
		gchar dump[5][BSIZE];
 		j.numline++;

		k = sscanf(t,"%s %s %s %s %s",dump[0],dump[1],dump[2],dump[3],dump[4]);
		if(k==5)
			j.geomtyp = GEOM_IS_XYZ;
		else
		if(k==4)
			j.geomtyp = GEOM_IS_XYZ;
		else
		if(k==1)
			j.geomtyp = GEOM_IS_ZMAT;
		else
			j.geomtyp = GEOM_IS_OTHER;
		/*
  		k=0;
  		for(i=strlen(t)-1;i>0;i--)
    		if(t[i]!=' ' || t[i] !='\n' )
    		{
			k=i;
       			break;
    		}
  		if(k>2)
   			j.geomtyp = GEOM_IS_XYZ;
  		else
  		if(k>0 && k<=2)
   			j.geomtyp = GEOM_IS_ZMAT;
  		else j.geomtyp = GEOM_IS_OTHER;
		*/
        }
   }
 }     
  fclose(fd);
  g_free(t);
  return j;
}
/**********************************************************************************/
FilePosTypeGeom get_geometry_type_from_nwchem_input_file(gchar *NomFichier)
{
 gchar *t;
 FILE *fd;
 guint taille=BSIZE;
 FilePosTypeGeom j;

 
 j.geomtyp=GEOM_IS_OTHER;
 j.numline=0;
 j.units=1;
 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 if(fd==NULL) return j;
 while(!feof(fd) )    
 {
 	if(!fgets(t, taille, fd)) break;
	uppercase(t);
	if(strstr(t,"GEOMETRY")) 
	{
		j.geomtyp = GEOM_IS_XYZ;
 		if(!fgets(t, taille, fd)) break;
		uppercase(t);
		if(strstr(t,"ZMATRIX")) j.geomtyp = GEOM_IS_ZMAT;
		break;
	}
  	j.numline++;
  }
  fclose(fd);
  g_free(t);
  return j;
}
/**********************************************************************************/
FilePosTypeGeom get_geometry_type_from_psicode_input_file(gchar *NomFichier)
{
 gchar *t;
 FILE *fd;
 guint taille=BSIZE;
 FilePosTypeGeom j;

 
 j.geomtyp=GEOM_IS_OTHER;
 j.numline=0;
 j.units=1;
 /* printf("NomFichier=%s\n",NomFichier);*/
 t = g_malloc(taille*sizeof(gchar));
 fd = FOpen(NomFichier, "rb");
 if(fd==NULL) return j;
 while(!feof(fd) )    
 {
 	if(!fgets(t, taille, fd)) break;
	uppercase(t);
		/* printf("t=%s\n",t);*/
	if(strstr(t,"MOLECULE")) 
	{
		gint charge,mult;
		gboolean OK = FALSE;
		gint i;
		if(fgets(t,taille,fd) && 2==sscanf(t,"%d %d",&charge,&mult)) OK = TRUE;
		else OK = FALSE;
		/* printf("t=%s\n",t);*/
		if(OK && fgets(t,taille,fd))
		{
			gchar*t1 = g_malloc(taille*sizeof(gchar));
			gchar*t2 = g_malloc(taille*sizeof(gchar));
			gchar*t3 = g_malloc(taille*sizeof(gchar));
			gchar*t4 = g_malloc(taille*sizeof(gchar));
		/* printf("t=%s\n",t);*/
        		i = sscanf(t,"%s %s %s %s",t1,t2,t3,t4);
			if(i==4)j.geomtyp = GEOM_IS_XYZ;
			else j.geomtyp = GEOM_IS_ZMAT;
  			g_free(t1);
  			g_free(t2);
  			g_free(t3);
  			g_free(t4);
		}
		break;
	}
  	j.numline++;
  }
  fclose(fd);
  g_free(t);
  return j;
}
/**********************************************************************************/
FilePosTypeGeom get_geometry_type_from_orca_input_file(gchar *NomFichier)
{
 gchar *t;
 FILE *fd;
 guint taille=BSIZE;
 FilePosTypeGeom j;

 
 j.geomtyp=GEOM_IS_OTHER;
 j.numline=0;
 j.units=1;
 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 if(fd==NULL) return j;
 while(!feof(fd) )    
 {
 	if(!fgets(t, taille, fd)) break;
	uppercase(t);
	if(strstr(t,"* XYZ")) 
	{
		j.geomtyp = GEOM_IS_XYZ;
		break;
	}
	if(strstr(t,"* INT")) 
	{
		j.geomtyp = GEOM_IS_ZMAT;
		break;
	}
  	j.numline++;
  }
  fclose(fd);
  g_free(t);
  return j;
}
/**********************************************************************************/
FilePosTypeGeom get_geometry_type_from_qchem_input_file(gchar *NomFichier)
{
 gchar *t;
 FILE *fd;
 guint taille=BSIZE;
 FilePosTypeGeom j;
 gint k;

 
 j.geomtyp=GEOM_IS_OTHER;
 j.numline=0;
 j.units=1;
 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 if(fd!=NULL)
 {
 while(!feof(fd) )    
 {
 	if(!fgets(t, taille, fd)) break;
	if(strstr(t,"$molecule")) break;
  	j.numline++;
  }
/* Charge and Spin */
 if(!feof(fd) )    
 {
 if(fgets(t, taille, fd))
  	j.numline++;
 }
/* First line of geometry */
  if(!feof(fd) )    
  {
 	if(!fgets(t, taille, fd))
        {
  		j.geomtyp = GEOM_IS_OTHER;
        }
        else
        {
		gchar dump[5][BSIZE];
 		j.numline++;

		k = sscanf(t,"%s %s %s %s %s",dump[0],dump[1],dump[2],dump[3],dump[4]);
		if(k==5)
			j.geomtyp = GEOM_IS_XYZ;
		else
		if(k==4)
			j.geomtyp = GEOM_IS_XYZ;
		else
		if(k==1)
			j.geomtyp = GEOM_IS_ZMAT;
		else
			j.geomtyp = GEOM_IS_OTHER;
        }
   }
 }     
  fclose(fd);
  g_free(t);
  return j;
}
/**********************************************************************************/
FilePosTypeGeom get_geometry_type_from_mopac_input_file(gchar *NomFichier)
{
 gchar *t;
 FILE *fd;
 guint taille=BSIZE;
 FilePosTypeGeom j;
 gint k;

 
 j.geomtyp=GEOM_IS_OTHER;
 j.numline=0;
 j.units=1;
 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 if(fd!=NULL)
 {
  	while(!feof(fd) )    
  	{
 		if(!fgets(t, taille, fd))break;
  		j.numline++;
		if(t[0] !='*') break;
	}
    	if(!feof(fd)) { char* e = fgets(t,taille,fd);}
  	j.numline++;
    	if(!feof(fd)) { char* e = fgets(t,taille,fd);}
  	j.numline++;
/* First line of geometry */
  if(!feof(fd) )    
  {
 	if(!fgets(t, taille, fd))
        {
  		j.geomtyp = GEOM_IS_OTHER;
        }
        else
        {
		gchar dump[8][BSIZE];
 		j.numline++;

		k = sscanf(t,"%s %s %s %s %s %s %s %s",dump[0],dump[1],dump[2],dump[3],dump[4], dump[5], dump[6], dump[7]);
		if(k==8)
			j.geomtyp = GEOM_IS_ZMAT;
		else
		if(k==7)
			j.geomtyp = GEOM_IS_XYZ;
		else
			j.geomtyp = GEOM_IS_OTHER;
        }
   }
 }     
  fclose(fd);
  g_free(t);
  return j;
}
/**********************************************************************************/
void read_geom_in_gamess_input(gchar *fileName)
{
	gchar* logfile;
	gchar* t;
	FILE* file;
	t = get_suffix_name_file(fileName);
	logfile = g_strdup_printf("%s.log",t);
	file = FOpen(logfile, "rb");
	if(!file)
	{
		if(logfile) g_free(logfile);
		logfile = g_strdup_printf("%s.out",t);
		file = FOpen(logfile, "rb");
		if(!file) return;
	}
	fclose(file);
	read_geom_from_gamess_output_file(logfile,1);
}
/**********************************************************************************/
void read_geom_in_firefly_input(gchar *fileName)
{
	gchar* logfile;
	gchar* t;
	FILE* file;
	t = get_suffix_name_file(fileName);
	logfile = g_strdup_printf("%s.log",t);
	file = FOpen(logfile, "rb");
	if(!file)
	{
		if(logfile) g_free(logfile);
		logfile = g_strdup_printf("%s.out",t);
		file = FOpen(logfile, "rb");
		if(!file) return;
	}
	fclose(file);
	read_geom_from_gamess_output_file(logfile,1);
}
/**********************************************************************************/
void read_geom_in_mpqc_input(gchar *fileName)
{
	read_XYZ_from_mpqc_input_file(fileName);
}
/**********************************************************************************/
void read_geom_in_demon_input(gchar *NameFile)
{
        FilePosTypeGeom j;
        j=  get_geometry_type_from_demon_input_file(NameFile);
        if( j.geomtyp == GEOM_IS_XYZ)
                read_XYZ_from_demon_input_file(NameFile,j);
        else
        if( j.geomtyp == GEOM_IS_ZMAT)
                read_Zmat_from_demon_input_file(NameFile,j);
        else
                Message(_("Sorry\nI can not read gemetry in DeMon input file\n"),_("Warning"),TRUE);
}

/**********************************************************************************/
void read_geom_in_gauss_input(gchar *NameFile)
{
	FilePosTypeGeom j;
 	j=  get_geometry_type_from_gauss_input_file(NameFile);
 	if( j.geomtyp == GEOM_IS_XYZ)
		read_XYZ_from_gauss_input_file(NameFile,j);
      	else
 	if( j.geomtyp == GEOM_IS_ZMAT)
		read_Zmat_from_gauss_input_file(NameFile,j);
        else
    		Message(_("Sorry\nI can not read gemetry in gaussian input file\n"),_("Warning"),TRUE);
}
/**********************************************************************************/
void read_geom_in_molcas_input(gchar *NameFile)
{
	setMolcasGeometryFromInputFile(NameFile);
}
/**********************************************************************************/
void read_geom_in_molpro_input(gchar *NameFile)
{
	FilePosTypeGeom j;
 	j=  get_geometry_type_from_molpro_input_file(NameFile);

 	if( j.geomtyp == GEOM_IS_XYZ)
		read_XYZ_from_molpro_input_file(NameFile,j);
        else
 	if( j.geomtyp == GEOM_IS_ZMAT)
		read_Zmat_from_molpro_input_file(NameFile,j);
        else
    		Message(_("Sorry\nI can not read gemetry in molpro input file\n"),_("Warning"),TRUE);
}
/**********************************************************************************/
void read_geom_in_nwchem_input(gchar *NameFile)
{
	FilePosTypeGeom j;
 	j=  get_geometry_type_from_nwchem_input_file(NameFile);
 	if( j.geomtyp == GEOM_IS_XYZ)
		read_XYZ_from_nwchem_input_file(NameFile);
      	else
 	if( j.geomtyp == GEOM_IS_ZMAT)
		read_Zmat_from_nwchem_input_file(NameFile);
        else
    		Message(_("Sorry\nI can not read gemetry in NWChem input file\n"),_("Warning"),TRUE);
}
/**********************************************************************************/
void read_geom_in_psicode_input(gchar *NameFile)
{
	FilePosTypeGeom j;
 	j=  get_geometry_type_from_psicode_input_file(NameFile);
 	if( j.geomtyp == GEOM_IS_XYZ)
		read_XYZ_from_psicode_input_file(NameFile);
      	else
 	if( j.geomtyp == GEOM_IS_ZMAT)
		read_Zmat_from_psicode_input_file(NameFile);
        else
    		Message(_("Sorry\nI can not read gemetry in Psicode input file\n"),_("Warning"),TRUE);
}
/**********************************************************************************/
void read_geom_in_orca_input(gchar *NameFile)
{
	FilePosTypeGeom j;
 	j=  get_geometry_type_from_orca_input_file(NameFile);
 	if( j.geomtyp == GEOM_IS_XYZ)
		read_XYZ_from_orca_input_file(NameFile);
      	else
 	if( j.geomtyp == GEOM_IS_ZMAT)
		read_Zmat_from_orca_input_file(NameFile);
        else
    		Message(_("Sorry\nI can not read gemetry in Orca input file\n"),_("Warning"),TRUE);
}
/**********************************************************************************/
void read_geom_in_qchem_input(gchar *NameFile)
{
	FilePosTypeGeom j;
 	j=  get_geometry_type_from_qchem_input_file(NameFile);
 	if( j.geomtyp == GEOM_IS_XYZ)
		read_XYZ_from_qchem_input_file(NameFile);
      	else
 	if( j.geomtyp == GEOM_IS_ZMAT)
		read_Zmat_from_qchem_input_file(NameFile);
        else
    		Message(_("Sorry\nI can not read gemetry in Q-Chem input file\n"),_("Warning"),TRUE);
}
/**********************************************************************************/
void read_geom_in_mopac_input(gchar *NameFile)
{
	FilePosTypeGeom j;
 	j=  get_geometry_type_from_mopac_input_file(NameFile);
 	if( j.geomtyp == GEOM_IS_XYZ)
		read_XYZ_from_mopac_input_file(NameFile);
      	else
 	if( j.geomtyp == GEOM_IS_ZMAT)
		read_Zmat_from_mopac_input_file(NameFile);
        else
    		Message(_("Sorry\nI can not read gemetry in Mopac input file\n"),_("Warning"),TRUE);
}
/**********************************************************************************/
void get_doc(gchar *NomFichier)
{
	gchar *t;
	FILE *fd;
	guint nchar;
	guint taille=BSIZE;

	if ((!NomFichier) || (strcmp(NomFichier,"") == 0)) return ;
 
	t=g_malloc(taille*sizeof(gchar));
	fd = FOpen(NomFichier, "rb");
	if(fd==NULL)
	{
		//fprintf(stderr,"Sorry, I can not open\n %s\n file",NomFichier);
		g_free(t);
		t = g_strdup_printf(_("Sorry, I can not open\n %s\n file"),NomFichier);
		Message(t,_("Error"),TRUE);
		g_free(t);
		return;
	}

	iprogram = get_type_of_program(fd);
	//fprintf(stderr,"iprog=%d",iprogram);

	nchar=gabedit_text_get_length(GABEDIT_TEXT(text));
	gabedit_text_set_point(GABEDIT_TEXT(text),0);
	gabedit_text_forward_delete(GABEDIT_TEXT(text),nchar);

	while(1)
	{
		nchar = fread(t, 1, taille, fd);
		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, t,nchar);
     		if(nchar<taille) break;
	}
	fclose(fd);

	g_free(t);
	gabedit_text_set_point(GABEDIT_TEXT(text),0);

	fileopen.remotehost = NULL;
	fileopen.remoteuser = NULL;
	fileopen.remotepass = NULL;
	fileopen.remotedir = NULL;
	fileopen.command = NULL;
	fileopen.netWorkProtocol = defaultNetWorkProtocol;

	if(iprogram == PROG_IS_MPQC) fileopen.command=g_strdup(NameCommandMPQC);
	else if(iprogram == PROG_IS_DEMON) fileopen.command=g_strdup(NameCommandDeMon);
	else if(iprogram == PROG_IS_GAMESS) fileopen.command=g_strdup(NameCommandGamess);
	else if(iprogram == PROG_IS_FIREFLY) fileopen.command=g_strdup(NameCommandFireFly);
	else if(iprogram == PROG_IS_GAUSS) fileopen.command=g_strdup(NameCommandGaussian);
	else if(iprogram == PROG_IS_MOLCAS) fileopen.command=g_strdup(NameCommandMolcas);
	else if(iprogram == PROG_IS_MOLPRO) fileopen.command=g_strdup(NameCommandMolpro);
	else if(iprogram == PROG_IS_QCHEM) fileopen.command=g_strdup(NameCommandQChem);
	else if(iprogram == PROG_IS_ORCA) fileopen.command=g_strdup(NameCommandOrca);
	else if(iprogram == PROG_IS_MOPAC) fileopen.command=g_strdup(NameCommandMopac);
     	else fileopen.command=NULL;

	if(iprogram> PROG_IS_OTHER )
	{
		fileopen.datafile = get_name_file(NomFichier); 
		t = get_suffix_name_file(NomFichier);
		fileopen.projectname = get_name_file(t);
		fileopen.localdir = get_name_dir(t);
		g_free(t);
		CreeFeuille(treeViewProjects, noeud[iprogram],fileopen.projectname,fileopen.datafile ,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,iprogram,  fileopen.command, fileopen.netWorkProtocol);
	}
	else
	{
  		fileopen.projectname = get_name_file(NomFichier);
  		fileopen.localdir = get_name_dir(NomFichier);
 		CreeFeuille(treeViewProjects, noeud[NBNOD-1],fileopen.projectname,fileopen.projectname,fileopen.localdir,
				fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,NBNOD-1,  fileopen.command, fileopen.netWorkProtocol);
	}

	if(iprogram == PROG_IS_GAMESS)
	{
 		fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
 		fileopen.outputfile=g_strdup_printf("%s.log",fileopen.projectname);
 		fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  		/* fileopen.moldenfile=g_strdup_printf("%s.molden",fileopen.projectname);*/
  		fileopen.moldenfile=g_strdup_printf("%s.log",fileopen.projectname);
	}
	else
	if(iprogram == PROG_IS_FIREFLY)
	{
 		fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
 		fileopen.outputfile=g_strdup_printf("%s.log",fileopen.projectname);
 		fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  		/* fileopen.moldenfile=g_strdup_printf("%s.molden",fileopen.projectname);*/
  		fileopen.moldenfile=g_strdup_printf("%s.log",fileopen.projectname);
	}
	else
	if(iprogram == PROG_IS_DEMON)
	{
 		fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
 		fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile=g_strdup_printf("%s.out",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("%s.molden",fileopen.projectname);
	}
	else
	if(iprogram == PROG_IS_MPQC)
	{
 		fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
 		fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile=g_strdup_printf("%s.out",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("'%s.molden'", fileopen.projectname);
	}
	else
	if(iprogram == PROG_IS_MOLCAS)
	{
 		fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
 		fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  		/* fileopen.moldenfile=g_strdup_printf("%s.molden",fileopen.projectname);*/
  		fileopen.moldenfile=g_strdup_printf("'%s.*.molden'", fileopen.projectname);
	}
	else
	if(iprogram == PROG_IS_MOLPRO)
	{
 		fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
 		fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("%s.molden",fileopen.projectname);
	}
	else if(iprogram == PROG_IS_GAUSS)
 	{
 		fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
 		fileopen.outputfile=g_strdup_printf("%s.log",fileopen.projectname);
 		fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("%s.log",fileopen.projectname);
 	}
	else if(iprogram == PROG_IS_ORCA)
 	{
 		fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
 		fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile=g_strdup_printf("%s.out",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);
 	}
	else if(iprogram == PROG_IS_NWCHEM)
 	{
 		fileopen.datafile = g_strdup_printf("%s.nw",fileopen.projectname);
 		fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile=g_strdup_printf("%s.out",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);
 	}
	else if(iprogram == PROG_IS_QCHEM)
 	{
 		fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
 		fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile=g_strdup_printf("%s.out",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);
 	}
	else if(iprogram == PROG_IS_MOPAC)
 	{
 		fileopen.datafile = g_strdup_printf("%s.mop",fileopen.projectname);
 		fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile=g_strdup_printf("%s.aux",fileopen.projectname);
  		fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);
 	}
	else
	{
 		fileopen.datafile = g_strdup_printf("%s",fileopen.projectname);
 		fileopen.outputfile=g_strdup_printf("%s","Unknown");
 		fileopen.logfile = g_strdup_printf("%s","Unknown");
	}

	if( iprogram == PROG_IS_GAUSS) read_geom_in_gauss_input(NomFichier);
	else if( iprogram == PROG_IS_DEMON) read_geom_in_demon_input(NomFichier);
	else if( iprogram == PROG_IS_GAMESS) read_geom_in_gamess_input(NomFichier);
	else if( iprogram == PROG_IS_FIREFLY) read_geom_in_firefly_input(NomFichier);
	else if( iprogram == PROG_IS_MOLPRO) read_geom_in_molpro_input(NomFichier);
	else if( iprogram == PROG_IS_MPQC) read_geom_in_mpqc_input(NomFichier);
	else if( iprogram == PROG_IS_ORCA) read_geom_in_orca_input(NomFichier);
	else if( iprogram == PROG_IS_NWCHEM) read_geom_in_nwchem_input(NomFichier);
	else if( iprogram == PROG_IS_PSICODE) read_geom_in_psicode_input(NomFichier);
	else if( iprogram == PROG_IS_QCHEM) read_geom_in_qchem_input(NomFichier);
	else if( iprogram == PROG_IS_MOPAC) read_geom_in_mopac_input(NomFichier);
	else if(iprogram == PROG_IS_MOLCAS)
	{
		setMolcasVariablesFromInputFile(NomFichier);
		read_geom_in_molcas_input(NomFichier);
	}

	data_modify(FALSE);

	if(GeomConvIsOpen) find_energy_all(NULL,NULL);

}
/********************************************************************************/
 void show_doc(GabeditFileChooser *SelecteurFichier, gint response_id)
{
 
 char *NomFichier;
 if(response_id != GTK_RESPONSE_OK) return;
 NomFichier = gabedit_file_chooser_get_current_file(SelecteurFichier);
 get_doc(NomFichier);
 }
/********************************************************************************/
 void insert_doc(GabeditFileChooser *SelecteurFichier, gint response_id)
{
 char *t;
 char *NomFichier;
 guint nchar;
 FILE *fd;
 guint taille=BSIZE;
 if(response_id != GTK_RESPONSE_OK) return;
 NomFichier = gabedit_file_chooser_get_current_file(SelecteurFichier);
 if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
		    return ;

 
 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 if(fd!=NULL)
 {
  while(1)
  {
     nchar = fread(t, 1, taille, fd);
     gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, t,nchar);
     if(nchar<taille) break;
  }
  fclose(fd);
 }
 g_free(t);
 gabedit_text_set_point(GABEDIT_TEXT(text),0);
 set_last_directory(NomFichier);
}
/********************************************************************************/
 void charge_doc(GtkWidget* wid, gpointer data)
{
 gtk_notebook_set_current_page((GtkNotebook*)NoteBookText,0);
 choose_file_to_open();
}
/********************************************************************************/
 void inserrer_doc(void)
{
 gtk_notebook_set_current_page((GtkNotebook*)NoteBookText,0);
 choose_file_to_insert();
 /*data_modify(TRUE);*/
}
/********************************************************************************/
static gboolean enreg_doc(gchar *NomFichier)
{
	gchar *temp;
	FILE *fd;
	gint i;
 
	fd = FOpen(NomFichier, "wb");
	if(fd == NULL)
	{
		Message(_("Sorry, I can not save file"),_("Error"),TRUE);
		return FALSE;
	}
	temp=gabedit_text_get_chars(text,0,-1);
	for(i=0;i<strlen(temp);i++)
		if(temp[i]=='\r') temp[i] = ' ';
	fprintf(fd,"%s",temp);
	fclose(fd);
	g_free(temp);
	set_last_directory(NomFichier);
	return TRUE;
}
/********************************************************************************/
void enreg_selec_doc(GabeditFileChooser *SelecteurFichier , gint response_id)
{
	gchar *temp;
	gchar *NomFichier;
  
 	if(response_id != GTK_RESPONSE_OK) return;
 	NomFichier = gabedit_file_chooser_get_current_file(SelecteurFichier);

	if ((!NomFichier) || (strcmp(NomFichier,"") == 0)) return ;
  
	if(iprogram == PROG_IS_MPQC) fileopen.command=g_strdup(NameCommandMPQC);
	else if(iprogram == PROG_IS_DEMON) fileopen.command=g_strdup(NameCommandDeMon);
	else if(iprogram == PROG_IS_GAMESS) fileopen.command=g_strdup(NameCommandGamess);
	else if(iprogram == PROG_IS_FIREFLY) fileopen.command=g_strdup(NameCommandFireFly);
	else if(iprogram == PROG_IS_GAUSS) fileopen.command=g_strdup(NameCommandGaussian);
	else if(iprogram == PROG_IS_MOLCAS) fileopen.command=g_strdup(NameCommandMolcas);
	else if(iprogram == PROG_IS_MOLPRO) fileopen.command=g_strdup(NameCommandMolpro);
     	else fileopen.command=NULL;

	if(iprogram>PROG_IS_OTHER)
	{
 		temp = get_suffix_name_file(NomFichier);
 		fileopen.projectname = get_name_file(temp);
 		fileopen.localdir = get_name_dir(temp);
 		g_free(temp);
		if(iprogram==PROG_IS_GAMESS)
		fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
		else
		if(iprogram==PROG_IS_FIREFLY)
		fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
		else
		if(iprogram==PROG_IS_DEMON)
		fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
		else
		if(iprogram==PROG_IS_QCHEM)
		fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
		else
		if(iprogram==PROG_IS_MOPAC)
		fileopen.datafile = g_strdup_printf("%s.mop",fileopen.projectname);
		else
		fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
		if(NomFichier) g_free(NomFichier);
		NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);

 		fileopen.remotehost = NULL;
 		fileopen.remoteuser = NULL;
 		fileopen.remotepass = NULL;
 		fileopen.remotedir = NULL;
		fileopen.netWorkProtocol = defaultNetWorkProtocol;

 		CreeFeuille(treeViewProjects, noeud[iprogram],fileopen.projectname,fileopen.datafile,fileopen.localdir,
					fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,iprogram,  fileopen.command, fileopen.netWorkProtocol);
	}
	else
	{
		temp = NomFichier;
 		fileopen.projectname = get_name_file(temp);
 		fileopen.localdir = get_name_dir(temp);
 		fileopen.remotehost = NULL;
 		fileopen.remoteuser = NULL;
 		fileopen.remotepass = NULL;
		fileopen.netWorkProtocol = defaultNetWorkProtocol;
 		CreeFeuille(treeViewProjects, noeud[NBNOD-1],fileopen.projectname,fileopen.projectname,
			fileopen.localdir,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,NBNOD-1,  fileopen.command, fileopen.netWorkProtocol);
	}

	if(iprogram == PROG_IS_MPQC)
	{
		fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
	 	fileopen.outputfile = g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile = g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.moldenfile = g_strdup_printf("%s.molden",fileopen.projectname);
	}
	else
	if(iprogram == PROG_IS_GAMESS)
	{
		fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
	 	fileopen.outputfile = g_strdup_printf("%s.log",fileopen.projectname);
 		fileopen.logfile = g_strdup_printf("%s.log",fileopen.projectname);
 		fileopen.moldenfile = g_strdup_printf("%s.log",fileopen.projectname);
	}
	else
	if(iprogram == PROG_IS_FIREFLY)
	{
		fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
	 	fileopen.outputfile = g_strdup_printf("%s.log",fileopen.projectname);
 		fileopen.logfile = g_strdup_printf("%s.log",fileopen.projectname);
 		fileopen.moldenfile = g_strdup_printf("%s.log",fileopen.projectname);
	}
	else
	if(iprogram == PROG_IS_DEMON)
	{
		fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
	 	fileopen.outputfile = g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.logfile = g_strdup_printf("%s.out",fileopen.projectname);
 		fileopen.moldenfile = g_strdup_printf("%s.out",fileopen.projectname);
	}
	else
	if(iprogram == PROG_IS_GAUSS)
	{
		fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
	 	fileopen.outputfile = g_strdup_printf("%s.log",fileopen.projectname);
 		fileopen.logfile = g_strdup_printf("%s.log",fileopen.projectname);
 		fileopen.moldenfile = g_strdup_printf("%s.log",fileopen.projectname);
	}
	else if(iprogram == PROG_IS_MOLCAS)
	{
 		fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
 		fileopen.outputfile = g_strdup_printf("%s.out",fileopen.projectname);
		fileopen.logfile = g_strdup_printf("%s.log",fileopen.projectname);
		fileopen.moldenfile = g_strdup_printf("%s.molden",fileopen.projectname);
	}
	else if(iprogram == PROG_IS_MOLPRO)
	{
 		fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
 		fileopen.outputfile = g_strdup_printf("%s.out",fileopen.projectname);
		fileopen.logfile = g_strdup_printf("%s.log",fileopen.projectname);
		fileopen.moldenfile = g_strdup_printf("%s.molden",fileopen.projectname);
	}
	else if(iprogram == PROG_IS_QCHEM)
	{
 		fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
 		fileopen.outputfile = g_strdup_printf("%s.out",fileopen.projectname);
		fileopen.logfile = g_strdup_printf("%s.out",fileopen.projectname);
		fileopen.moldenfile = g_strdup_printf("%s.molden",fileopen.projectname);
	}
	else if(iprogram == PROG_IS_ORCA)
	{
 		fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
 		fileopen.outputfile = g_strdup_printf("%s.out",fileopen.projectname);
		fileopen.logfile = g_strdup_printf("%s.out",fileopen.projectname);
		fileopen.moldenfile = g_strdup_printf("%s.molden",fileopen.projectname);
	}
     	else
	{
		fileopen.datafile = g_strdup(NomFichier);
		fileopen.outputfile = g_strdup("Unknown");
		fileopen.logfile = g_strdup("Unknown");
	}

	if(enreg_doc(NomFichier))
	{
 		change_label_onglet();
 		data_modify(FALSE);
	}
}
/********************************************************************************/
 void save_as_doc(void)
{
	choose_file_to_save();
}
/********************************************************************************/
void save_doc(void)
{
	 if ( strcmp(fileopen.datafile,"NoName") != 0)
	 {
		gchar* FileName =  g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
 		if(enreg_doc(FileName))
			data_modify(FALSE);

		g_free(FileName);
	 }
	else
		choose_file_to_save();
}
/********************************************************************************/
void new_doc_molcas(GtkWidget* wid, gpointer data)
{
 	newMolcas();
	iprogram = PROG_IS_MOLCAS;
}
/********************************************************************************/
 void new_doc_molpro(GtkWidget* wid, gpointer data)
{
 	molpro();
	iprogram = PROG_IS_MOLPRO;
}
/********************************************************************************/
 void new_doc_mpqc(GtkWidget* wid, gpointer data)
{
 	newMPQC();
	iprogram = PROG_IS_MPQC;
}
/********************************************************************************/
 void new_doc_gamess(GtkWidget* wid, gpointer data)
{
 	newGamess();
	iprogram = PROG_IS_GAMESS;
	fileopen.command=g_strdup(NameCommandGamess);
}
/********************************************************************************/
 void new_doc_demon(GtkWidget* wid, gpointer data)
{
 	newDeMon();
	iprogram = PROG_IS_DEMON;
	fileopen.command=g_strdup(NameCommandDeMon);
}
/********************************************************************************/
 void new_doc_firefly(GtkWidget* wid, gpointer data)
{
 	newFireFly();
	iprogram = PROG_IS_FIREFLY;
	fileopen.command=g_strdup(NameCommandFireFly);
}
/********************************************************************************/
 void new_doc_gauss(GtkWidget* wid, gpointer data)
{
 gauss(1);
}
/********************************************************************************/
 void new_doc_orca(GtkWidget* wid, gpointer data)
{
 	newOrca();
	iprogram = PROG_IS_ORCA;
	fileopen.command=g_strdup(NameCommandOrca);
}
/********************************************************************************/
 void new_doc_nwchem(GtkWidget* wid, gpointer data)
{
 	newNWChem();
	iprogram = PROG_IS_NWCHEM;
	fileopen.command=g_strdup(NameCommandNWChem);
}
/********************************************************************************/
void new_doc_psicode(GtkWidget* wid, gpointer data)
{
 	newPsicode();
	iprogram = PROG_IS_PSICODE;
	fileopen.command=g_strdup(NameCommandPsicode);
}
/********************************************************************************/
 void new_doc_qchem(GtkWidget* wid, gpointer data)
{
 	newQChem();
	iprogram = PROG_IS_QCHEM;
	fileopen.command=g_strdup(NameCommandQChem);
}
/********************************************************************************/
 void new_doc_mopac(GtkWidget* wid, gpointer data)
{
 	newMopac();
	iprogram = PROG_IS_MOPAC;
	fileopen.command=g_strdup(NameCommandMopac);
}
/********************************************************************************/
void new_doc_other(GtkWidget* wid, gpointer data)
{
	gint nchar;
 	reset_name_files();
	data_modify(TRUE);
    gabedit_text_set_point(GABEDIT_TEXT(text),0);
	nchar =  gabedit_text_get_length(GABEDIT_TEXT(text));
    gabedit_text_forward_delete(GABEDIT_TEXT(text),nchar);
	gtk_notebook_set_current_page((GtkNotebook*)NoteBookText,0);
}
/********************************************************************************/
 void insert_doc_gauss(void)
{
 gauss(0);
}
/********************************************************************************/
static void show_about_new()
 {
	static const gchar *authors[] = {
		"Abdul-Rahman Allouche <allouchear@users.sourceforge.net>",
		NULL
	};

	static const gchar *documenters[] = {
		"Abdul-Rahman Allouche <allouchear@users.sourceforge.net>",
		NULL
	};

	static const gchar *copyright =
		"Copyright \xc2\xa9 2002-2013 Abdul-Rahman Allouche.\n"
		"All rights reserved.\n";
	
	gchar *license =
		g_strdup_printf("%s%s",
			copyright,
			"\n"
			"Permission is hereby granted, free of charge, to any person obtaining a copy\n"
			"of this software(the Gabedit) and associated documentation files, to deal in\n"
			"the Software without restriction, including without limitation the rights to\n"
			"use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies\n"
			"of the Software, and to permit persons to whom the Software is furnished to \n"
			"do so, subject to the following conditions:\n"
			"\n"
			"The above copyright notice and this permission notice shall be included in all\n"
			"copies or substantial portions of the Software.\n"
			"\n"
			"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
			"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
			"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL\n"
			"THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
			"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
			"OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER\n"
			"DEALINGS IN THE SOFTWARE."
			);

	static const gchar *comments =
		"Graphical User Interface to DeMon, FireFly, GAMESS-US, Gaussian, Molcas, Molpro, "
		"OpenMopac, Orca, MPQC, NWChem and Q-Chem computational chemistry packages.\n\n"
		"Please use the following citations in any report or publication :\n"
		"A.R. ALLOUCHE, Gabedit - A graphical user interface for computational chemistry software,\n"
	        "Journal of Computational Chemistry, 32, 174-182(2011)\n";
	
	gchar *GABEDIT_VERSION =
		g_strdup_printf("%d.%d.%d",MAJOR_VERSION,MINOR_VERSION,MICRO_VERSION);
	
	gtk_about_dialog_set_url_hook(show_homepage, NULL, NULL);

	gtk_show_about_dialog (
		NULL,
		"name", "Gabedit",
		"version", GABEDIT_VERSION,
		"comments", comments,
		"copyright", copyright,
		"license", license,
		"website", "http://gabedit.sourceforge.net",
		"documenters", documenters,
		"authors", authors,
		"logo-icon-name", GTK_STOCK_ABOUT,
		NULL);
		
	g_free(GABEDIT_VERSION);
	g_free(license);
}
/********************************************************************************/
void show_about()
{
	if(GTK_MAJOR_VERSION>=2 && GTK_MINOR_VERSION>=6) show_about_new();
	else create_about_frame();
}
/********************************************************************************/
void show_version()
{
 gchar* Version_S = g_strdup_printf("%d.%d.%d",MAJOR_VERSION,MINOR_VERSION,MICRO_VERSION);
 gchar *temp=g_strdup_printf("\n Version %s \n\n Abdul-Rahman ALLOUCHE\n",Version_S);
 Message(temp,_("Version"),TRUE);
 g_free(Version_S);
 g_free(temp);
}
/********************************************************************************/
void show_homepage(GtkWidget *w,gpointer data)
{

#ifdef G_OS_WIN32
	gchar* Command = "Iexplore.exe http://gabedit.sourceforge.net/";
	system(Command);
#else
	gchar* Command = "mozilla  http://gabedit.sourceforge.net/ &";
	if (system(Command)<0)
	{
		gchar* Command = "galeon http://gabedit.sourceforge.net/ &";
		if (system(Command)<0)
		{
			gchar* Command = "konqueror http://gabedit.sourceforge.net/ &";
			{int ierr = system(Command);}
		}
	}
#endif
}
/********************************************************************************/
void desole(void)
{
 char *temp=_("Sorry , this option is not active");
 Message(temp,_("Warning"),TRUE);
}
/********************************************************************************/
 void change_insert(void)
{
 if (iedit==0)
      {
      iedit=1;
      gabedit_text_set_editable(GABEDIT_TEXT(text), TRUE);
      }
  else
   {
      iedit=0;
      gabedit_text_set_editable(GABEDIT_TEXT(text), FALSE);
   }
}
/********************************************************************************/
void  create_label_hbox(GtkWidget *hbox,gchar *tlabel,gint llen)
{
    GtkWidget* label;
    label = gtk_label_new(tlabel); 
    gtk_widget_set_size_request(GTK_WIDGET(label),llen,-1);
    gtk_box_pack_start(GTK_BOX(hbox), label,FALSE,FALSE,2);
}
/********************************************************************************/
GtkWidget *create_text(GtkWidget *win,GtkWidget *frame,gboolean editable)
{
  GtkWidget *Text;
  GtkWidget *scrolledwindow;
  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  g_object_ref (scrolledwindow);
  g_object_set_data_full (G_OBJECT (win), "scrolledwindow", scrolledwindow,
                            (GDestroyNotify) g_object_unref);
  gtk_widget_show (scrolledwindow);
  gtk_container_add (GTK_CONTAINER (frame), scrolledwindow);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  Text = gabedit_text_new ();
  g_object_ref (Text);
  g_object_set_data_full (G_OBJECT (win), "Text", Text,
                            (GDestroyNotify) g_object_unref);
  gtk_widget_show (Text);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), Text);
  gabedit_text_set_editable (GABEDIT_TEXT (Text), editable);
  return Text;
}
/********************************************************************************/
void  gtk_combo_box_entry_set_popdown_strings(GtkWidget* comboBoxEntry, GList *list)
{
	GList* l;
	GtkTreeModel * model = NULL;
	if(!list) return;
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(comboBoxEntry));
	gtk_list_store_clear(GTK_LIST_STORE(model));
	l = list;
	while(l)
	{
		gtk_combo_box_append_text (GTK_COMBO_BOX (comboBoxEntry), (gchar*)(l->data));
		l = l->next;
	}
  	gtk_combo_box_set_active(GTK_COMBO_BOX (comboBoxEntry), 0);
}
/********************************************************************************/
GtkWidget*  create_combo_box_entry(gchar **tlist,gint nlist, gboolean edit,gint llen,gint elen)
{
	GtkWidget* combo;
	gint i;
	combo = gtk_combo_box_entry_new_text();
	for (i=0;i<nlist;i++) gtk_combo_box_append_text (GTK_COMBO_BOX (combo), tlist[i]);
	gtk_widget_set_size_request(GTK_WIDGET(combo),elen,-1);
 	gtk_editable_set_editable((GtkEditable*) (GTK_BIN (combo)->child),edit);
	if(nlist>0) gtk_combo_box_set_active(GTK_COMBO_BOX (combo), 0);
	return combo;
}
/********************************************************************************/
GtkWidget *create_combo_box_entry_liste(GtkWidget* Window,GtkWidget* hbox,gchar *lname,gchar **liste,int n)
{
  int i;
  GtkWidget *label;
  GtkWidget *combo;
  GtkWidget *combo_entry;

  if(lname)
  {
  	label = gtk_label_new (lname);
  	gtk_widget_show (label);
  	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 2);
  }

  combo = gtk_combo_box_entry_new_text ();
  gtk_widget_set_size_request(combo, (gint)(ScreenHeight*0.1), -1);
  gtk_widget_show (combo);
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 2);

  for (i=0;i<n;i++) gtk_combo_box_append_text (GTK_COMBO_BOX (combo), liste[i]);
  if(n>0) gtk_combo_box_set_active(GTK_COMBO_BOX (combo), 0);

  combo_entry = (GTK_BIN (combo)->child);
  gtk_widget_show (combo_entry);
  gtk_entry_set_text (GTK_ENTRY (combo_entry), liste[0]);
  return combo_entry;
 }
/********************************************************************************/
GtkWidget*  create_label_combo_in_table(GtkWidget *table,gchar *tlabel,gchar **tlist,gint nlist, gboolean edit,gint llen,gint elen, gint iligne)
{
        GtkWidget* combo;
        GtkWidget* label;
	GtkWidget *hbox = gtk_hbox_new(0,FALSE);
        gint i;


        label = gtk_label_new(tlabel);
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);
        gtk_widget_set_size_request(GTK_WIDGET(label),llen,-1);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, iligne, iligne+1, GTK_FILL, GTK_FILL, 1, 1);
        gtk_widget_show(label);
        gtk_widget_show(hbox);

        label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, iligne, iligne+1, (GtkAttachOptions)(GTK_FILL|GTK_EXPAND), (GtkAttachOptions)(GTK_FILL|GTK_SHRINK), 1, 1);

        combo = gtk_combo_box_entry_new_text();
        for (i=0;i<nlist;i++) gtk_combo_box_append_text (GTK_COMBO_BOX (combo), tlist[i]);
        if(nlist>0) gtk_combo_box_set_active(GTK_COMBO_BOX (combo), 0);
        gtk_widget_set_size_request(GTK_WIDGET(combo),elen,-1);
	gtk_table_attach(GTK_TABLE(table), combo, 2, 3, iligne, iligne+1, (GtkAttachOptions)(GTK_FILL|GTK_EXPAND), (GtkAttachOptions)(GTK_FILL|GTK_SHRINK), 1, 1);
        gtk_widget_set_sensitive(GTK_BIN(combo)->child,edit);
        return GTK_BIN(combo)->child;
}
/********************************************************************************/
GtkWidget *create_frame(GtkWidget *win,GtkWidget *box,gchar *title)
{
  GtkWidget *frame;
  frame = gtk_frame_new (title);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_widget_show (frame);
  return frame;
}
/********************************************************************************/
GtkWidget *create_hbox(GtkWidget *vbox)
{
  GtkWidget *hbox;
  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  return hbox;
}
/********************************************************************************/
GtkWidget *create_vbox(GtkWidget *win)
{
  GtkWidget *vbox;
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (win), vbox);
  return vbox;
}
/********************************************************************************/
GtkWidget *create_label_button(GtkWidget *win,GtkWidget *frame,GtkWidget* Vbox,
                         gchar *tlabel,gchar *tbutton)
{
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *hbox;
 
  hbox = gtk_hbox_new (FALSE, 2);
  g_object_ref (hbox);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (Vbox), hbox, FALSE, FALSE, 0);

  label = gtk_label_new (tlabel);
  g_object_ref (label);
  g_object_set_data_full (G_OBJECT (win), "label", label,
                            (GDestroyNotify) g_object_unref);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 2);

  button = gtk_button_new_with_label (tbutton);
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 2);
  gtk_widget_show (button);
  return button;
}
/********************************************************************************/
void change_label_onglet()
{
	GtkWidget *LabelOnglet = GTK_WIDGET(g_object_get_data(G_OBJECT(text),"LabelOnglet"));
	gchar *temp;
	GtkStyle *style;

	temp=get_name_file(fileopen.datafile);
	gtk_label_set_text(GTK_LABEL(LabelOnglet),temp);

	if(imodif == DATA_MOD_YES)
	{
		/*
		PangoFontDescription *font_desc;

		font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);
		*/
		style = (GtkStyle*)g_object_get_data(G_OBJECT(text),"StyleRed");
	

		/*
		if (style && font_desc)
		{
			style->font_desc = font_desc;
		}
		*/
		gtk_widget_set_style(LabelOnglet, style );
	}
	else
	{
		/*
  		PangoFontDescription *font_desc;

		font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);
		*/
		style = (GtkStyle*)g_object_get_data(G_OBJECT(text),"StyleDef");

		/*
		if (style && font_desc)
		{
			style->font_desc = font_desc;
		}
		*/

		if(style) gtk_widget_set_style(LabelOnglet, style );
	}

	temp=get_name_file(fileopen.outputfile);
	LabelOnglet = GTK_WIDGET(g_object_get_data(G_OBJECT(textresult),"LabelOnglet"));
	gtk_label_set_text(GTK_LABEL(LabelOnglet),temp);

	g_free(temp);

}
/********************************************************************************/
void change_all_labels()
{
  change_label_onglet();
   set_label_infos_file();
}
/********************************************************************************/
void add_widget_table(GtkWidget *Table,GtkWidget *wid,gushort line,gushort colonne)
{

	gtk_table_attach(GTK_TABLE(Table),wid,colonne,colonne+1,line,line+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  3,3);

}

/********************************************************************************/
GtkWidget *add_label_table(GtkWidget *Table, G_CONST_RETURN gchar *label,gushort line,gushort colonne)
{
	GtkWidget *Label;
	GtkWidget *hbox = gtk_hbox_new(0,FALSE);
	
	Label = gtk_label_new (label);
   	gtk_label_set_justify(GTK_LABEL(Label),GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX (hbox), Label, FALSE, FALSE, 0);
	add_widget_table(Table,hbox,line,colonne);

	return Label;
}
/********************************************************************************/
GtkWidget *add_label_at_table(GtkWidget *Table,gchar *label,gushort line,gushort colonne,GtkJustification just)
{
	GtkWidget *Label;
	GtkWidget *hbox = gtk_hbox_new(0,FALSE);
	
	Label = gtk_label_new (label);
   	gtk_label_set_justify(GTK_LABEL(Label),just);
	if(just ==GTK_JUSTIFY_CENTER) 
		gtk_box_pack_start (GTK_BOX (hbox), Label, TRUE, TRUE, 0);
	else
		gtk_box_pack_start (GTK_BOX (hbox), Label, FALSE, FALSE, 0);
	
	add_widget_table(Table,hbox,line,colonne);

	return Label;
}
/********************************************************************************/
void get_result()
{
	gchar *t;
	gchar *fileName;
	guint nchar;
	guint taille=BSIZE;
	nchar=gabedit_text_get_length(GABEDIT_TEXT(textresult));
	if ((!fileopen.outputfile) || (strcmp(fileopen.outputfile,"") == 0)) return ;
	if ((!fileopen.localdir) || (strcmp(fileopen.localdir,"") == 0)) return ;

	gabedit_text_set_point(GABEDIT_TEXT(textresult),0);
	gabedit_text_forward_delete(GABEDIT_TEXT(textresult),nchar);
 
	fileName = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.outputfile);
	t = readFile(fileName);
	g_free(fileName);
	if(t!=NULL)
	{
		gtk_widget_set_sensitive(ResultLocalFrame, FALSE);
		nchar=strlen(t);
		gabedit_text_insert (GABEDIT_TEXT(textresult), NULL, NULL, NULL, t,nchar);
		gtk_widget_set_sensitive(ResultLocalFrame, TRUE);
		//g_free(t);
	}
	gabedit_text_set_point(GABEDIT_TEXT(textresult),0);
}
/********************************************************************************/
 void view_result_end()
{
	gtk_notebook_set_current_page((GtkNotebook*)NoteBookText,1);
	get_result();
	goto_end_result(NULL,NULL);
}
/********************************************************************************/
void view_result()
{
	gtk_notebook_set_current_page((GtkNotebook*)NoteBookText,1);
	get_result();
}
/********************************************************************************/
GtkWidget * create_hseparator(GtkWidget *vbox)
{
	GtkWidget *hseparator;
	hseparator = gtk_hseparator_new ();
	g_object_ref (hseparator);
	g_object_set_data_full (G_OBJECT (vbox), "hseparator", hseparator, (GDestroyNotify) g_object_unref);
	gtk_widget_show (hseparator);
	gtk_box_pack_start (GTK_BOX (vbox), hseparator, FALSE, FALSE, 1);
	return hseparator;
}
/********************************************************************************/
GtkWidget * create_vseparator(GtkWidget *hbox)
{
	GtkWidget *vseparator;
	vseparator = gtk_vseparator_new ();
	g_object_ref (vseparator);
	g_object_set_data_full (G_OBJECT (hbox), "vseparator", vseparator, (GDestroyNotify) g_object_unref);
	gtk_widget_show (vseparator);
	gtk_box_pack_start (GTK_BOX (hbox), vseparator, FALSE, FALSE, 1);
	return vseparator;
}
/********************************************************************************/
GtkWidget*  create_vbox_false(GtkWidget *win)
{
	GtkWidget *vbox;
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (win), vbox);
	gtk_widget_show (vbox);
	return vbox;
}
/********************************************************************************/
GtkWidget*  create_hbox_false(GtkWidget *vbox)
{
	GtkWidget *hbox;
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
	gtk_widget_show (hbox);
	return hbox;
}
/********************************************************************************/
GtkWidget*  create_hbox_true(GtkWidget *vbox)
{
	GtkWidget *hbox;
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), hbox, TRUE, TRUE, 2);
	gtk_widget_show (hbox);
	return hbox;
}
/********************************************************************************/
GtkWidget*  create_label_entry(GtkWidget *hbox,gchar *tlabel,gint llen,gint elen)
{
	 GtkWidget* entry;
	GtkWidget* label;
    
	label = gtk_label_new(tlabel); 
	gtk_widget_set_size_request(GTK_WIDGET(label),llen,-1);
	gtk_box_pack_start(GTK_BOX(hbox), label,FALSE,FALSE,2);
   
	entry = gtk_entry_new();
	gtk_widget_set_size_request(GTK_WIDGET(entry),elen,-1);
	gtk_box_pack_start(GTK_BOX(hbox), entry,FALSE,FALSE,2);
	return entry;
}
/********************************************************************************/
GtkWidget*  create_label_combo(GtkWidget *hbox,gchar *tlabel,gchar **tlist,gint nlist, gboolean edit,gint llen,gint elen)
{
	GtkWidget* combo;
	GtkWidget* label;
	gint i;

	label = gtk_label_new(tlabel); 
	gtk_widget_set_size_request(GTK_WIDGET(label),llen,-1);
	gtk_box_pack_start(GTK_BOX(hbox), label,FALSE,FALSE,2);

	combo = gtk_combo_box_entry_new_text();
	for (i=0;i<nlist;i++) gtk_combo_box_append_text (GTK_COMBO_BOX (combo), tlist[i]);
	if(nlist>0) gtk_combo_box_set_active(GTK_COMBO_BOX (combo), 0);
	gtk_widget_set_size_request(GTK_WIDGET(combo),elen,-1);
	gtk_box_pack_start(GTK_BOX(hbox), combo,FALSE,FALSE,2);
	gtk_widget_set_sensitive(GTK_BIN(combo)->child,edit);
	return GTK_BIN(combo)->child;
}
/********************************************************************************/
void show_forbidden_characters()
{
	gchar *temp;
	gchar Forbidden[]={
	'0','1','2','3','4','5','6','7','8','9',
    	'+','-','/','%','$','*','!','@','#','^',
    	'&','(',')','|','\\','<','>','?',',','~',
    	'`','\'','.','"',':',';'};
	guint All=36;
	guint i;
	temp = g_strdup(_("Sorry the name of variable is not valid !\n\n"));
	temp = g_strdup_printf(_("%s The first character can not be one of : \n"),temp);
	for(i=0;i<10;i++)
		temp = g_strdup_printf("%s %c",temp,Forbidden[i]);
	temp = g_strdup_printf("%s \n\n",temp);
	temp = g_strdup_printf(_("%s Do not use the following characters :\n"),temp);
	for(i=11;i<All;i++)
	{
		temp = g_strdup_printf("%s %c ",temp,Forbidden[i]);
		if(i%10 == 0) temp = g_strdup_printf("%s \n",temp);
	}
	Message(temp,_("Error"),TRUE);
	g_free(temp);
}
/********************************************************************************/
void  cree_p_vbox()
{
	vboxmain = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (Fenetre), vboxmain);
	gtk_widget_show (vboxmain);
}
/********************************************************************************/
void  cree_vboxs_list_text()
{
	GtkWidget *hpaned;

	hpaned = gtk_hpaned_new();
	gtk_container_add (GTK_CONTAINER (vboxmain), hpaned);

	vboxlistfiles = gtk_vbox_new (FALSE, 0);
	gtk_paned_add1(GTK_PANED(hpaned), vboxlistfiles);
	gtk_widget_show (vboxlistfiles);

	vboxtexts = gtk_vbox_new (FALSE, 0);
	gtk_paned_add2(GTK_PANED(hpaned), vboxtexts);
	gtk_widget_realize (vboxtexts);
	gtk_widget_show (vboxtexts);
	Hpaned = hpaned;
}
/********************************************************************************/
GtkWidget *create_checkbutton(GtkWidget *win,GtkWidget *box,gchar *tlabel)
{
	GtkWidget* checkbutton;
	checkbutton = gtk_check_button_new_with_label (tlabel);
	g_object_ref (checkbutton);
	g_object_set_data_full (G_OBJECT (win), "checkbutton", checkbutton, (GDestroyNotify) g_object_unref);
	gtk_widget_show (checkbutton);
	gtk_box_pack_start (GTK_BOX (box), checkbutton, FALSE, FALSE, 0);
	return checkbutton;
}
/********************************************************************************/
GtkWidget *Continue_YesNo(void (*func)(GtkWidget*,gpointer data),gpointer data,gchar *message)
{
	GtkWidget *DialogueMessage = NULL;
	GtkWidget *Label, *Bouton;
	GtkWidget *frame, *vboxframe;
	DialogueMessage = gtk_dialog_new();
	gtk_window_set_position(GTK_WINDOW(DialogueMessage),GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(DialogueMessage),_("Warning"));
	gtk_window_set_transient_for(GTK_WINDOW(DialogueMessage),GTK_WINDOW(Fenetre));
	gtk_window_set_modal (GTK_WINDOW (DialogueMessage), TRUE);


	g_signal_connect(G_OBJECT(DialogueMessage), "delete_event", (GCallback)gtk_widget_destroy, NULL);
    
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	g_object_ref (frame);
	g_object_set_data_full (G_OBJECT (DialogueMessage), "frame", frame,(GDestroyNotify) g_object_unref);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(DialogueMessage)->vbox), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);
	vboxframe = create_vbox(frame);
	gtk_widget_realize(DialogueMessage);
	Label = create_label_with_pixmap(DialogueMessage,message,_("Question"));  
	gtk_box_pack_start(GTK_BOX(vboxframe), Label,TRUE,TRUE,0);
    
	gtk_widget_realize(DialogueMessage);

	Bouton = create_button(DialogueMessage,_("No"));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(DialogueMessage)->action_area),Bouton,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(DialogueMessage));
	GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Bouton);

	Bouton = create_button(DialogueMessage,_("Yes"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(DialogueMessage)->action_area), Bouton,TRUE,TRUE,0);
	GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
	g_signal_connect(G_OBJECT(Bouton), "clicked", (GCallback)func,data);
	g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(DialogueMessage));


	gtk_widget_show_all(DialogueMessage);
	return DialogueMessage;
}
/**********************************************************************************/
void open_file(GtkWidget *widget, gchar *data)
{
	gchar *t;
 	if(imodif == DATA_MOD_YES)
        {
		t = g_strdup_printf(_("\nThe \"%s\" file has been modified.\n\n"),get_name_file(fileopen.datafile));
		t = g_strdup_printf(_(" %sIf you continue, you lose what you have changed.\n\n"),t);
		t = g_strdup_printf(_(" %sYou want to continue?\n"),t);
		Continue_YesNo(charge_doc, NULL,t);
		g_free(t);
        }
        else
        {
		charge_doc(NULL, NULL);
 		data_modify(FALSE);
 	}
}
/**********************************************************************************/
void new_molcas(GtkWidget *widget, gchar *data)
{
	gchar t[BSIZE];
	gchar t1[BSIZE];
 	if(imodif == DATA_MOD_YES)
        {
		sprintf(t,_("\nThe \"%s\" file has been modified.\n\n"),get_name_file(fileopen.datafile));
		sprintf(t1,"%s",t);
		sprintf(t,_(" %sIf you continue, you lose what you have changed.\n\n"),t1);
		sprintf(t1,"%s",t);
		sprintf(t,_(" %sYou want to continue?\n"),t1);
		Continue_YesNo(new_doc_molcas, NULL,t);
        }
        else
        {
		new_doc_molcas(NULL, NULL);
		iprogram = PROG_IS_MOLCAS;
		fileopen.command=g_strdup(NameCommandMolcas);
 	}
}
/**********************************************************************************/
void new_molpro(GtkWidget *widget, gchar *data)
{
	gchar *t;
 	if(imodif == DATA_MOD_YES)
        {
		t = g_strdup_printf(_("\nThe \"%s\" file has been modified.\n\n"),get_name_file(fileopen.datafile));
		t = g_strdup_printf(_(" %sIf you continue, you lose what you have changed.\n\n"),t);
		t = g_strdup_printf(_(" %sYou want to continue?\n"),t);
		Continue_YesNo(new_doc_molpro, NULL,t);
		g_free(t);
        }
        else
        {
		new_doc_molpro(NULL, NULL);
		iprogram = PROG_IS_MOLPRO;
		fileopen.command=g_strdup(NameCommandMolpro);
 	}
}
/**********************************************************************************/
void new_mpqc(GtkWidget *widget, gchar *data)
{
	gchar *t;
 	if(imodif == DATA_MOD_YES)
        {
		t = g_strdup_printf(_("\nThe \"%s\" file has been modified.\n\n"),get_name_file(fileopen.datafile));
		t = g_strdup_printf(_(" %sIf you continue, you lose what you have changed.\n\n"),t);
		t = g_strdup_printf(_(" %sYou want to continue?\n"),t);
		Continue_YesNo(new_doc_mpqc, NULL,t);
		g_free(t);
        }
        else
        {
		new_doc_mpqc(NULL, NULL);
		iprogram = PROG_IS_MPQC;
		fileopen.command=g_strdup(NameCommandMPQC);
 	}
}
/**********************************************************************************/
void new_firefly(GtkWidget *widget, gchar *data)
{
	gchar *t;
 	if(imodif == DATA_MOD_YES)
        {
		t = g_strdup_printf(_("\nThe \"%s\" file has been modified.\n\n"),get_name_file(fileopen.datafile));
		t = g_strdup_printf(_(" %sIf you continue, you lose what you have changed.\n\n"),t);
		t = g_strdup_printf(_(" %sYou want to continue?\n"),t);
		Continue_YesNo(new_doc_firefly, NULL,t);
		g_free(t);
        }
        else
        {
		new_doc_firefly(NULL, NULL);
		iprogram = PROG_IS_FIREFLY;
		fileopen.command=g_strdup(NameCommandFireFly);
 	}
}
/**********************************************************************************/
void new_demon(GtkWidget *widget, gchar *data)
{
	gchar *t;
 	if(imodif == DATA_MOD_YES)
        {
		t = g_strdup_printf(_("\nThe \"%s\" file has been modified.\n\n"),get_name_file(fileopen.datafile));
		t = g_strdup_printf(_(" %sIf you continue, you lose what you have changed.\n\n"),t);
		t = g_strdup_printf(_(" %sYou want to continue?\n"),t);
		Continue_YesNo(new_doc_demon, NULL,t);
		g_free(t);
        }
        else
        {
		new_doc_demon(NULL, NULL);
		iprogram = PROG_IS_DEMON;
		fileopen.command=g_strdup(NameCommandDeMon);
 	}
}
/**********************************************************************************/
void new_gamess(GtkWidget *widget, gchar *data)
{
	gchar *t;
 	if(imodif == DATA_MOD_YES)
        {
		t = g_strdup_printf(_("\nThe \"%s\" file has been modified.\n\n"),get_name_file(fileopen.datafile));
		t = g_strdup_printf(_(" %sIf you continue, you lose what you have changed.\n\n"),t);
		t = g_strdup_printf(_(" %sYou want to continue?\n"),t);
		Continue_YesNo(new_doc_gamess, NULL,t);
		g_free(t);
        }
        else
        {
		new_doc_gamess(NULL, NULL);
		iprogram = PROG_IS_GAMESS;
		fileopen.command=g_strdup(NameCommandGamess);
 	}
}
/**********************************************************************************/
void new_gauss(GtkWidget *widget, gchar *data)
{
	gchar *t;
 	if(imodif == DATA_MOD_YES)
        {
		t = g_strdup_printf(_("\nThe \"%s\" file has been modified.\n\n"),get_name_file(fileopen.datafile));
		t = g_strdup_printf(_(" %sIf you continue, you lose what you have changed.\n\n"),t);
		t = g_strdup_printf(_(" %sYou want to continue?\n"),t);
		Continue_YesNo(new_doc_gauss, NULL,t);
		g_free(t);
        }
        else
        {
		new_doc_gauss(NULL, NULL);
		iprogram = PROG_IS_GAUSS;
		fileopen.command=g_strdup(NameCommandGaussian);
 	}
}
/**********************************************************************************/
void new_nwchem(GtkWidget *widget, gchar *data)
{
	gchar *t;
 	if(imodif == DATA_MOD_YES)
        {
		t = g_strdup_printf(_("\nThe \"%s\" file has been modified.\n\n"),get_name_file(fileopen.datafile));
		t = g_strdup_printf(_(" %sIf you continue, you lose what you have changed.\n\n"),t);
		t = g_strdup_printf(_(" %sYou want to continue?\n"),t);
		Continue_YesNo(new_doc_nwchem, NULL,t);
		g_free(t);
        }
        else
        {
		new_doc_nwchem(NULL, NULL);
		iprogram = PROG_IS_NWCHEM;
		fileopen.command=g_strdup(NameCommandNWChem);
 	}
}
/**********************************************************************************/
void new_psicode(GtkWidget *widget, gchar *data)
{
	gchar *t;
 	if(imodif == DATA_MOD_YES)
        {
		t = g_strdup_printf(_("\nThe \"%s\" file has been modified.\n\n"),get_name_file(fileopen.datafile));
		t = g_strdup_printf(_(" %sIf you continue, you lose what you have changed.\n\n"),t);
		t = g_strdup_printf(_(" %sYou want to continue?\n"),t);
		Continue_YesNo(new_doc_psicode, NULL,t);
		g_free(t);
        }
        else
        {
		new_doc_psicode(NULL, NULL);
		iprogram = PROG_IS_PSICODE;
		fileopen.command=g_strdup(NameCommandPsicode);
 	}
}
/**********************************************************************************/
void new_orca(GtkWidget *widget, gchar *data)
{
	gchar *t;
 	if(imodif == DATA_MOD_YES)
        {
		t = g_strdup_printf(_("\nThe \"%s\" file has been modified.\n\n"),get_name_file(fileopen.datafile));
		t = g_strdup_printf(_(" %sIf you continue, you lose what you have changed.\n\n"),t);
		t = g_strdup_printf(_(" %sYou want to continue?\n"),t);
		Continue_YesNo(new_doc_orca, NULL,t);
		g_free(t);
        }
        else
        {
		new_doc_orca(NULL, NULL);
		iprogram = PROG_IS_ORCA;
		fileopen.command=g_strdup(NameCommandOrca);
 	}
}
/**********************************************************************************/
void new_qchem(GtkWidget *widget, gchar *data)
{
	gchar *t;
 	if(imodif == DATA_MOD_YES)
        {
		t = g_strdup_printf(_("\nThe \"%s\" file has been modified.\n\n"),get_name_file(fileopen.datafile));
		t = g_strdup_printf(_(" %sIf you continue, you lose what you have changed.\n\n"),t);
		t = g_strdup_printf(_(" %sYou want to continue?\n"),t);
		Continue_YesNo(new_doc_qchem, NULL,t);
		g_free(t);
        }
        else
        {
		new_doc_qchem(NULL, NULL);
		iprogram = PROG_IS_QCHEM;
		fileopen.command=g_strdup(NameCommandQChem);
 	}
}
/**********************************************************************************/
void new_mopac(GtkWidget *widget, gchar *data)
{
	gchar *t;
 	if(imodif == DATA_MOD_YES)
        {
		t = g_strdup_printf(_("\nThe \"%s\" file has been modified.\n\n"),get_name_file(fileopen.datafile));
		t = g_strdup_printf(_(" %sIf you continue, you lose what you have changed.\n\n"),t);
		t = g_strdup_printf(_(" %sYou want to continue?\n"),t);
		Continue_YesNo(new_doc_mopac, NULL,t);
		g_free(t);
        }
        else
        {
		new_doc_mopac(NULL, NULL);
		iprogram = PROG_IS_MOPAC;
		fileopen.command=g_strdup(NameCommandMopac);
 	}
}
/**********************************************************************************/
 void new_other(GtkWidget *widget, gchar *data)
{
	gchar *t;
 	if(imodif == DATA_MOD_YES)
        {
		t = g_strdup_printf(_("\nThe \"%s\" file has been modified.\n\n"),get_name_file(fileopen.datafile));
		t = g_strdup_printf(_(" %sIf you continue, you lose what you have changed.\n\n"),t);
		t = g_strdup_printf(_(" %sYou want to continue?\n"),t);
		Continue_YesNo(new_doc_other, NULL,t);
		g_free(t);
        }
        else
        {
		new_doc_other(NULL, NULL);
		iprogram = PROG_IS_OTHER;
 	}
}
/**********************************************************************************/
GtkWidget *create_hbox_browser(GtkWidget* Wins,GtkWidget* vbox,gchar *tlabel,gchar *deffile,gchar** patterns)
{
  GtkWidget *Entry = NULL;
  GtkWidget *hbox ;
  GtkWidget *button;
  GtkWidget* Label;

  hbox = create_hbox_false(vbox);
  /* The label */
  Label = gtk_label_new(tlabel); 
  gtk_widget_set_size_request(GTK_WIDGET(Label),(gint)(ScreenHeight*0.06),-1);
  gtk_box_pack_start(GTK_BOX(hbox), Label,FALSE,FALSE,2);

  /* The Entry */
  Entry = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(hbox), Entry,TRUE,TRUE,2);
  if(deffile)
  {
  	gint len = strlen(deffile)*8;
  	 len = strlen(deffile)*(gint)(8);

  	gtk_widget_set_size_request(GTK_WIDGET(Entry),len,32);
	gtk_entry_set_text(GTK_ENTRY(Entry),deffile);
  }
  else
  	gtk_widget_set_size_request(GTK_WIDGET(Entry),350,32);

  /* The Button */
  button = create_button_pixmap(Wins,open_xpm,NULL);
  g_signal_connect_swapped(G_OBJECT (button), "clicked",G_CALLBACK(set_entry_selected_file),GTK_OBJECT(hbox));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 1);
  g_object_set_data(G_OBJECT (hbox), "Entry", Entry);
  g_object_set_data(G_OBJECT (hbox), "Button", button);
  g_object_set_data(G_OBJECT (hbox), "Label", Label);
  g_object_set_data(G_OBJECT (hbox), "Window", Wins);
  g_object_set_data(G_OBJECT (hbox), "Patterns",patterns);
  gtk_widget_show_all(hbox);
  return hbox;
}
/********************************************************************************/
GtkWidget*  create_table_browser(GtkWidget *Wins,GtkWidget *vbox)
{
  	GtkWidget *table = gtk_table_new(2,4,FALSE);
	GtkWidget* buttonDirSelector = NULL;
	GtkWidget* entryFileName = NULL;
	GtkWidget* label = NULL;
	gint i;
	gint j;

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	add_label_table(table,_("Folder"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1, (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) , (GtkAttachOptions)(GTK_FILL|GTK_SHRINK), 1,1);

	j = 2;
	buttonDirSelector =  gabedit_dir_button();
	gtk_widget_set_size_request(GTK_WIDGET(buttonDirSelector),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),buttonDirSelector, j,j+4,i,i+1, (GtkAttachOptions)(GTK_FILL|GTK_EXPAND), (GtkAttachOptions)(GTK_FILL|GTK_SHRINK), 1,1);
	g_object_set_data(G_OBJECT(Wins), "ButtonDirSelector", buttonDirSelector);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("File name"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1, (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) , (GtkAttachOptions)(GTK_FILL|GTK_SHRINK), 1,1);
	j = 2;
	entryFileName = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryFileName),"data.xyz");
	gtk_widget_set_size_request(GTK_WIDGET(entryFileName),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryFileName, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "EntryFileName", entryFileName);
/*----------------------------------------------------------------------------------*/
	gtk_widget_show_all(table);
	return table;
}
/********************************************************************************/
void set_default_styles()
{
	/*
	gchar *font = NULL;
	gchar *fontLabel = NULL;
	gchar *rc_string = NULL;

	if(FontsStyleOther.fontname) font = g_strdup_printf("font =\"%s\"",FontsStyleOther.fontname);
	else font = g_strdup(" ");

	if(FontsStyleLabel.fontname) fontLabel = g_strdup_printf("font =\"%s\"",FontsStyleLabel.fontname);
	else fontLabel = g_strdup(" ");
	
	rc_string=g_strdup_printf(
	 "style \"allwid\""
     "{"
		"%s"
     "}"
	 "style \"labels\""
     "{"
		"%s"
     "}"
	 "style \"frames\""
     "{"
		"fg[NORMAL] = { 1., 0., 0. }"
		"bg[NORMAL] = { 0.5, 0.5, 0.5 }"
		"%s"
     "}"
	 "style \"buttons\""
     "{"
       "fg[PRELIGHT] = { 0, 0.1, 1.0 }\n"
       "bg[PRELIGHT] = { 0.4, 0.4, 0.4 }\n"
       "bg[ACTIVE] = { 1.0, 0, 0 }\n"
       "fg[ACTIVE] = { 0, 1.0, 0 }\n"
       "bg[NORMAL] = { 0.65, 0.65, 0.65 }\n"
       "fg[NORMAL] = { 0.0, 0.0, 1.0 }\n"
       "bg[INSENSITIVE] = { 1.0, 1.0, 1.0 }\n"
       "fg[INSENSITIVE] = { 1.0, 0, 1.0 }\n"
	   "%s"
     "}\n"
	 "style \"toggle_button\" = \"button\"\n"
     "{\n"
       "fg[NORMAL] = { 0.0, 0.0, 0.0 }\n"
       "fg[ACTIVE] = { 0.0, 0.0, 1.0 }\n"
       "%s"
     "}\n"

       "widget_class \"*Gtk*\" style \"allwid\"\n"
     "widget_class \"*GtkButton*\" style \"buttons\"\n"
     "widget_class \"*GtkCheckButton*\" style \"toggle_button\"\n"
     "widget_class \"*GtkRadioButton*\" style \"toggle_button\"\n"
     "widget_class \"*GtkToggleButton*\" style \"toggle_button\"\n"
	 "widget_class \"*GtkFrame\" style \"frames\"\n"
	 "widget_class \"*GtkLabel\" style \"labels\"\n"
	 ,font,fontLabel,font,font,font);

	gtk_rc_parse_string(rc_string);

	g_free(rc_string);
	g_free(font);
	g_free(fontLabel);
	*/
	gtk_rc_parse_string("gtk-icon-sizes = \"gtk-menu=13,13:gtk-small-toolbar=16,16:gtk-large-toolbar=24,24\"gtk-toolbar-icon-size = small-toolbar");
}
/********************************************************************************/
GtkWidget* create_text_widget(GtkWidget* box,gchar *title,GtkWidget **frame)
{
  GtkWidget *scrolledwindow;
  GtkWidget *Frame;
  GtkWidget *Text;

  Frame = gtk_frame_new(title);
  gtk_container_set_border_width(GTK_CONTAINER(Frame), 2);
  gtk_container_add (GTK_CONTAINER (box), Frame);

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow ),
                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 

  gtk_container_add (GTK_CONTAINER (Frame), scrolledwindow);
  Text = gabedit_text_new ();
  set_tab_size (Text, 8);
  /*
  gabedit_text_set_word_wrap(GABEDIT_TEXT(Text), TRUE);  
  gabedit_text_set_line_wrap(GABEDIT_TEXT(Text), TRUE);
  */
  
  gtk_widget_show (Text);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), Text);
  gabedit_text_set_editable (GABEDIT_TEXT (Text), FALSE); 
  *frame = Frame;

  return Text;
}
/*********************************************************************/
void draw_density_orbitals_gamess_or_gauss_or_molcas_or_molpro(GtkWidget *wid,gpointer data)
{
  if( iprogram == PROG_IS_GAMESS)
  {
 	gchar** FileName = g_malloc(2*sizeof(gchar*));
 	FileName[0] = NULL;
	FileName[1] = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.logfile);
 	view_orb(Fenetre,2,FileName);
	g_free(FileName[1] );
	g_free(FileName);
  }
  else
  if( iprogram == PROG_IS_FIREFLY)
  {
 	gchar** FileName = g_malloc(2*sizeof(gchar*));
 	FileName[0] = NULL;
	FileName[1] = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.logfile);
 	view_orb(Fenetre,2,FileName);
	g_free(FileName[1] );
	g_free(FileName);
  }
  else
  if( iprogram == PROG_IS_GAUSS)
  {
 	gchar** FileName = g_malloc(2*sizeof(gchar*));
 	FileName[0] = NULL;
	FileName[1] = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.logfile);
 	view_orb(Fenetre,2,FileName);
	g_free(FileName[1] );
	g_free(FileName);
  }
  else
  if( iprogram == PROG_IS_MOLCAS)
  {
 	gchar** FileName = g_malloc(2*sizeof(gchar*));
 	FileName[0] = NULL;
	FileName[1] = g_strdup_printf("%s%s%s.scf.molden",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.projectname);
 	view_orb(Fenetre,2,FileName);
	g_free(FileName[1] );
	g_free(FileName);
  }
  else
  if( iprogram == PROG_IS_MOLPRO)
  {
 	gchar** FileName = g_malloc(2*sizeof(gchar*));
 	FileName[0] = NULL;
	FileName[1] = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.outputfile);
 	view_orb(Fenetre,2,FileName);
	g_free(FileName[1] );
	g_free(FileName);
  }
  else
  if( iprogram == PROG_IS_QCHEM)
  {
 	gchar** FileName = g_malloc(2*sizeof(gchar*));
 	FileName[0] = NULL;
	FileName[1] = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.outputfile);
 	view_orb(Fenetre,2,FileName);
	g_free(FileName[1] );
	g_free(FileName);
  }
  if( iprogram == PROG_IS_MOPAC)
  {
 	gchar** FileName = g_malloc(2*sizeof(gchar*));
 	FileName[0] = NULL;
	FileName[1] = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.outputfile);
 	view_orb(Fenetre,2,FileName);
	g_free(FileName[1] );
	g_free(FileName);
  }
  else
 	view_orb(Fenetre,0,NULL);
}
/*********************************************************************/
void set_last_directory(G_CONST_RETURN gchar* FileName)
{
	gchar* temp = get_suffix_name_file(FileName);
	gchar* localdir = get_name_dir(temp);
	if(!localdir) return;
	if(lastdirectory) g_free(lastdirectory);
	lastdirectory = g_strdup(localdir);
	if(temp) g_free(temp);
	if(localdir) g_free(localdir);
	if(lastdirectory && gabedit_directory() && strcmp(lastdirectory,gabedit_directory())) 
		{int it = chdir(lastdirectory);}
}
/*********************************************************************/
gchar* get_last_directory()
{
	return lastdirectory;
}
/******************************************************************/
static void set_dipole_color(GtkColorSelection *Sel,gpointer *d)
{
	GdkColor* color;
	color = g_object_get_data(G_OBJECT (Sel), "Color");
	gtk_color_selection_get_current_color(Sel, color);
}
/******************************************************************/
static void set_dipole_button_color(GtkObject *button,gpointer *data)
{
	GtkStyle *style = g_object_get_data(G_OBJECT (button), "Style");
	GdkColor* color = g_object_get_data(G_OBJECT (button), "Color");
	GtkWidget *OldButton  = g_object_get_data(G_OBJECT (button), "Button");
  	style =  gtk_style_copy(style); 
  	style->bg[0].red=color->red;
  	style->bg[0].green=color->green;
  	style->bg[0].blue= color->blue;
	gtk_widget_set_style(OldButton, style);
}
/******************************************************************/
static void open_color_dlg_dipole(GtkWidget *button,gpointer data)
{

	GtkColorSelectionDialog *colorDlg;
  	GtkStyle* style = g_object_get_data(G_OBJECT (button), "Style");
  	GtkWidget *win = g_object_get_data(G_OBJECT (button), "Win");
	GdkColor* color = g_object_get_data(G_OBJECT (button), "Color");;

	colorDlg = (GtkColorSelectionDialog *)gtk_color_selection_dialog_new(_("Set Dipole Color"));
	gtk_color_selection_set_current_color (GTK_COLOR_SELECTION (colorDlg->colorsel), color);
	gtk_color_selection_set_current_color (GTK_COLOR_SELECTION (colorDlg->colorsel), color);
	gtk_window_set_transient_for(GTK_WINDOW(colorDlg),GTK_WINDOW(win));
        gtk_window_set_position(GTK_WINDOW(colorDlg),GTK_WIN_POS_CENTER);
  	gtk_window_set_modal (GTK_WINDOW (colorDlg), TRUE);
 	g_signal_connect(G_OBJECT(colorDlg), "delete_event",(GCallback)destroy_button_windows,NULL);
  	g_signal_connect(G_OBJECT(colorDlg), "delete_event",G_CALLBACK(gtk_widget_destroy),NULL);

  	g_object_set_data(G_OBJECT (colorDlg->colorsel), "Color", color);
  	gtk_widget_hide(colorDlg->help_button);
	g_signal_connect_swapped(G_OBJECT(colorDlg->ok_button),"clicked", (GCallback)set_dipole_color,GTK_OBJECT(colorDlg->colorsel));

  	g_object_set_data(G_OBJECT (colorDlg->ok_button), "Color", color);
  	g_object_set_data(G_OBJECT (colorDlg->ok_button), "Button", button);
  	g_object_set_data(G_OBJECT (colorDlg->ok_button), "Style", style);
	g_signal_connect(G_OBJECT(colorDlg->ok_button),"clicked", (GCallback)set_dipole_button_color,NULL);

  	g_signal_connect_swapped(G_OBJECT(colorDlg->ok_button), "clicked", (GCallback)destroy_button_windows,GTK_OBJECT(colorDlg));
	g_signal_connect_swapped(G_OBJECT(colorDlg->ok_button),"clicked", (GCallback)gtk_widget_destroy,GTK_OBJECT(colorDlg));

  	g_signal_connect_swapped(G_OBJECT(colorDlg->cancel_button), "clicked", (GCallback)destroy_button_windows,GTK_OBJECT(colorDlg));
	g_signal_connect_swapped(G_OBJECT(colorDlg->cancel_button),"clicked", (GCallback)gtk_widget_destroy,GTK_OBJECT(colorDlg));

  	add_button_windows(_(" Set Color "),GTK_WIDGET(colorDlg));
	gtk_widget_show(GTK_WIDGET(colorDlg));

}
/*********************************************************************/
GtkWidget* set_dipole_dialog ()
{
  GtkWidget *fp;
  GtkWidget *frame;
  GtkWidget *vboxall;
  GtkWidget *vboxframe;
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget *label;
  GtkStyle *style;
  static GtkWidget* entrys[8];
  static GdkColor color;
  gchar* tlabel[8]={"Factor : ","X(D) : ","Y(D) : ","Z(D) : ","Radius : ","X0(Ang) : ","Y0(Ang) : ","Z0(Ang) : "};
  gint i;

  /* principal Window */
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(fp),TRUE);
  gtk_window_set_title(GTK_WINDOW(fp),_("Set Dipole"));
  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);

  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (fp), TRUE);

  g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)gtk_widget_destroy,NULL);

  vboxall = create_vbox(fp);
  frame = gtk_frame_new (_("Set Dipole(Debye)"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  for(i=0;i<5;i++)
  {
	hbox = create_hbox(vboxframe);
	label = gtk_label_new (tlabel[i]);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, FALSE, 0);

	entrys[i] = gtk_entry_new ();
	gtk_widget_show (entrys[i]);
	gtk_box_pack_start (GTK_BOX (hbox), entrys[i], FALSE, TRUE, 0);

	if(i!=0)
	{
		if(i==4)
		{
			gchar* t = g_strdup_printf("%f",Dipole.radius*AUTODEB);
			gtk_entry_set_text(GTK_ENTRY(entrys[i]),t);
			g_free(t);
		}
		else
		{
			gchar* t = g_strdup_printf("%f",Dipole.value[i-1]*AUTODEB);
			gtk_entry_set_text(GTK_ENTRY(entrys[i]),t);
			g_free(t);
		}
	}
	else
	{
		if(i==0)
			gtk_entry_set_text(GTK_ENTRY(entrys[i]),"1.0");
		else
			gtk_entry_set_text(GTK_ENTRY(entrys[i]),"0.0");
	}
  }
  for(i=5;i<8;i++)
  {
	hbox = create_hbox(vboxframe);
	label = gtk_label_new (tlabel[i]);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, FALSE, 0);

	entrys[i] = gtk_entry_new ();
	gtk_widget_show (entrys[i]);
	gtk_box_pack_start (GTK_BOX (hbox), entrys[i], FALSE, TRUE, 0);
	{
		gchar* t = g_strdup_printf("%f",Dipole.origin[i-5]*BOHR_TO_ANG);
		gtk_entry_set_text(GTK_ENTRY(entrys[i]),t);
		g_free(t);
	}
  }
  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vboxframe), hbox, FALSE, FALSE, 1);

  label = gtk_label_new (_("Color : "));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, FALSE, 0);
  style = gtk_widget_get_style(fp);
  button = gtk_button_new_with_label(" ");
  style =  gtk_style_copy(style); 
  style->bg[0].red=Dipole.color[0];
  style->bg[0].green=Dipole.color[1];
  style->bg[0].blue=Dipole.color[2];

  color.red =style->bg[0].red;
  color.green =style->bg[0].green;
  color.blue =style->bg[0].blue;
  gtk_widget_set_style(button, style );
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 1);
  gtk_widget_show (button);
  g_object_set_data(G_OBJECT (button), "Style", style);
  g_object_set_data(G_OBJECT (button), "Win", fp);
  g_object_set_data(G_OBJECT (button), "Color", &color);
  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)open_color_dlg_dipole, NULL);

  hbox = create_hbox(vboxall);

  button = create_button(Fenetre,_("Close"));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(fp));

  button = create_button(Fenetre,_("Apply"));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_object_set_data(G_OBJECT (button), "Color", &color);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(set_dipole),(gpointer)entrys);
  gtk_widget_show (button);


  button = create_button(Fenetre,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_object_set_data(G_OBJECT (button), "Color", &color);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(set_dipole),(gpointer)entrys);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(fp));
  gtk_widget_show (button);


  gtk_widget_show (button);
   
  gtk_widget_show_all(fp);
  return fp;
}
/*************************************************************************************/
void fit_windows_position(GtkWidget* parent, GtkWidget* child)
{
	gint wChild = 0, hChild = 0;
	gint wParent=0, hParent=0;
	gdk_drawable_get_size(parent->window,&wParent,&hParent);
	gtk_window_move(GTK_WINDOW(parent),0,0);
	gdk_drawable_get_size(child->window,&wChild,&hChild);
	if(wParent+wChild+10<ScreenWidth) gtk_window_move(GTK_WINDOW(child),wParent+10,0);
	else if(wChild<ScreenWidth) gtk_window_move(GTK_WINDOW(child),ScreenWidth-wChild,0);
}
/*************************************************************************************/
static gdouble** readOneFile(gchar* fileName, gint n0, gint* nP, gdouble* dt)
{
	gint nPoints = 0;
	gdouble** Dipole= NULL;
	FILE* file;
	gchar t[BSIZE];
	gint i;
	gint k;
	file = fopen(fileName,"rb");
	if(!file) printf("I cannot open '%s'\n",fileName);
	if(!file) return 0;
	k = 0;
	*nP = 0;
	*dt = 0;
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file))break;
		if(strstr(t,"Maximum Steps") && strstr(t,"="))
		{
			nPoints = atoi(strstr(t,"=")+1);
		}
		if(strstr(t," Time Step") && strstr(t,"="))
		{
			*dt = atof(strstr(t,"=")+1);
		}
		if(nPoints>0 && *dt>0) break;
	}
	rewind(file);
	if(nPoints == 0 && nPoints<n0) { return NULL;}
	Dipole =   malloc(nPoints*sizeof(gdouble*));
        for(k=0;k<nPoints;k++) Dipole[k] = malloc(3*sizeof(gdouble));
	k = 0;
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file))break;
		if(strstr(t,"Dipole        ="))
		{
			gchar* tt = strstr(t,"=")+1;
			gchar dum[16];
			gint j;
			if(k<n0) { k++; continue;}
			for(i=0;i<strlen(t);i++) if(t[i]=='D' || t[i] == 'd') t[i] = 'e';
			for(j=0;j<3;j++)
			{
				gint ii = 15;
				dum[ii] = '\0';
				for(i=0;i<ii;i++) dum[i] = tt[i];
				Dipole[k-n0][j] = atof(dum);
				dum[ii] = '\0';
				tt += ii;
			}
			k++;
		}
		else continue;
		if(k==nPoints) break;
	}
	if(k!=nPoints) printf("Warning : k != nPoints in %s file\n",fileName);
	fclose(file);
	*nP = nPoints-n0;
	return Dipole;
}
/********************************************************************************/
static gint doAutoCorr(gdouble** Dipole, gdouble* X, gint M)
{
	int m,n,j;
	for (m = 0; m < M; m++) X[m] = 0.0;
	/* This algorithm was adapted from the formulas given in
	 J. Kohanoff Comp. Mat. Sci. 2 221-232 (1994). The estimator 
	 formulation used here is unbiased and statistically consistent, 
   	 Looping through all time origins to collect an average -
   	*/ 
	int NCorr = 3*M/4;
	int Nav = M - NCorr;
   	for (m = 0; m < NCorr; m++)
      	for (n = 0; n < Nav; n++)
            for (j = 0; j < 3; j++)
               X[m] += Dipole[n + m][j] * Dipole[n][j];
   	for (m = 0; m < NCorr; m++) X[m] /= Nav;
	return NCorr;
}
/********************************************************************************/
static gboolean read_admp_dipole_dipole_file(GabeditFileChooser *filesel, gint response_id)
{
	gchar* fileName = NULL;
	GtkWidget* entryN = NULL;
	gint n0      = 1;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gdouble* Ytmp = NULL;
	gdouble** Dipole = NULL;
	gdouble dt;
	gint M = 0;
	gint MC = 0;
	gint k;
	GtkWidget* xyplot;
	GtkWidget* window;
	GSList* lists = NULL;
	GSList* cur = NULL;
	gint nf = 0;
	if(response_id != GTK_RESPONSE_OK) return FALSE;

	lists = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(filesel));
	
	entryN = g_object_get_data (G_OBJECT (filesel), "EntryN");
	if(!entryN) return FALSE;
	n0     = atoi(gtk_entry_get_text(GTK_ENTRY(entryN)));
	if(n0<0) n0 = 0;

	create_popup_win(_("Please wait"));
	cur = lists;
	nf = 0;
	while(cur != NULL)
	{
		fileName = (gchar*)(cur->data);
		nf++;
		if(cur==lists)
		{
			Dipole = readOneFile(fileName, n0, &M, &dt);
			if(M<2) 
			{
    				Message(_("Error\n The number of steps <2 !\n"),_("Error"),TRUE);
				return FALSE;
			}
			X = g_malloc(M*sizeof(gdouble));
			Y = g_malloc(M*sizeof(gdouble));
			for(k=0;k<M;k++) X[k] = dt*k;
			MC = doAutoCorr(Dipole, Y, M);
			for(k=0;k<M;k++) g_free(Dipole[k]);
			g_free(Dipole);
		}
		else  
		{
			gint m = 0;
			gint mc = 0;
			gdouble dt0;
			Dipole = readOneFile(fileName, n0, &m, &dt0);
	
			if(m!=M || M<2) 
			{
    				Message(_("Error\n The number of steps is not same in all files\n"),_("Error"),TRUE);
				return FALSE;
			}

			Ytmp = g_malloc(m*sizeof(gdouble));
			mc = doAutoCorr(Dipole, Ytmp, m);
			for(k=0;k<M;k++) g_free(Dipole[k]);
			g_free(Dipole);
			for(k=0;k<MC;k++) Y[k] += Ytmp[k];
			g_free(Ytmp);
		}

		cur = cur->next;
	}
	if(nf>0) for(k=0;k<MC;k++) Y[k] /= nf;


	window = gabedit_xyplot_new_window(_("Dipole-Dipole autocorrelation"),NULL);
	xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");
	gabedit_xyplot_add_data_conv(GABEDIT_XYPLOT(xyplot),MC, X,  Y, 1.0, GABEDIT_XYPLOT_CONV_NONE,NULL);
	gabedit_xyplot_set_range_xmin (GABEDIT_XYPLOT(xyplot), 0.0);
	g_free(X); 
	g_free(Y);

	return TRUE;
}
/********************************************************************************/
void read_admp_build_dipole_dipole_autocorrelation_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_admp_dipole_dipole_file,
			_("Read the ADMP Gaussian output file to do a dipole_dipole autocorrelation function"),
			GABEDIT_TYPEFILE_GAUSSIAN,GABEDIT_TYPEWIN_OTHER);
	GtkWidget* entryN = gtk_entry_new();
	GtkWidget* hbox = gtk_hbox_new(FALSE,1);
	GtkWidget* hsep1 = gtk_hseparator_new();
	GtkWidget* hsep2 = gtk_hseparator_new();
	GtkWidget* labelN = gtk_label_new(_("     Number of steps to remove : "));

	gtk_entry_set_text(GTK_ENTRY(entryN),"0");

	gtk_box_pack_start (GTK_BOX (hbox), labelN, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), entryN, FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG(filesel)->vbox), hsep1, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG(filesel)->vbox), hbox, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG(filesel)->vbox), hsep2, FALSE, FALSE, 0);
	gtk_widget_show_all(hsep1);
	gtk_widget_show_all(hsep2);
	gtk_widget_show_all(hbox);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(filesel),TRUE);
	g_object_set_data (G_OBJECT (filesel), "EntryN",entryN);
}
/*************************************************************************************/
static gdouble** readOneTxtFile(gchar* fileName, gint n0, gint* nP, gdouble* dt)
{
	gint nPoints = 0;
	gdouble** Dipole= NULL;
	FILE* file;
	gchar t[BSIZE];
	gdouble d[4];
	gint i;
	gint k;
	double t0 = 0;
	file = fopen(fileName,"rb");
	if(!file) printf("I cannot open '%s'\n",fileName);
	if(!file) return 0;
	k = 0;
	*nP = 0;
	*dt = 0;
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file))break;
		if(sscanf(t,"%lf%lf%lf%lf",&d[0],&d[1],&d[2],&d[3])!=4)break;
		nPoints++;
	}
	rewind(file);
	if(nPoints == 0 || nPoints<n0) { return NULL;}
	Dipole =   malloc(nPoints*sizeof(gdouble*));
        for(k=0;k<nPoints;k++) Dipole[k] = malloc(3*sizeof(gdouble));
	k = 0;
	*dt = 0;
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file))break;
		if(sscanf(t,"%lf%lf%lf%lf",&d[0],&d[1],&d[2],&d[3])!=4)break;
		if(k>=n0) for(i=0;i<3;i++) Dipole[k-n0][i] = d[i+1];
		*dt = d[0];
		if(k==0) t0 = d[0];
		k++;
		if(k==nPoints) break;
	}
	*dt = (*dt-t0)/nPoints;
	fclose(file);
	*nP = nPoints-n0;
	return Dipole;
}
/********************************************************************************/
static gboolean read_dipole_text_file(GabeditFileChooser *filesel, gint response_id)
{
	gchar* fileName = NULL;
	GtkWidget* entryN = NULL;
	gint n0      = 1;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gdouble* Ytmp = NULL;
	gdouble** Dipole = NULL;
	gdouble dt;
	gint M = 0;
	gint MC = 0;
	gint k;
	GtkWidget* xyplot;
	GtkWidget* window;
	GSList* lists = NULL;
	GSList* cur = NULL;
	gint nf = 0;
	if(response_id != GTK_RESPONSE_OK) return FALSE;

	lists = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(filesel));
	
	entryN = g_object_get_data (G_OBJECT (filesel), "EntryN");
	if(!entryN) return FALSE;
	n0     = atoi(gtk_entry_get_text(GTK_ENTRY(entryN)));
	if(n0<0) n0 = 0;

	create_popup_win(_("Please wait"));
	cur = lists;
	nf = 0;
	while(cur != NULL)
	{
		fileName = (gchar*)(cur->data);
		nf++;
		if(cur==lists)
		{
			Dipole = readOneTxtFile(fileName, n0, &M, &dt);
			if(M<2) 
			{
    				Message(_("Error\n The number of steps <2 !\n"),_("Error"),TRUE);
				return FALSE;
			}
			X = g_malloc(M*sizeof(gdouble));
			Y = g_malloc(M*sizeof(gdouble));
			for(k=0;k<M;k++) X[k] = dt*k;
			MC = doAutoCorr(Dipole, Y, M);
			for(k=0;k<M;k++) g_free(Dipole[k]);
			g_free(Dipole);
		}
		else  
		{
			gint m = 0;
			gint mc = 0;
			gdouble dt0;
			Dipole = readOneTxtFile(fileName, n0, &m, &dt0);
	
			if(m!=M || M<2) 
			{
    				Message(_("Error\n The number of steps is not same in all files\n"),_("Error"),TRUE);
				return FALSE;
			}

			Ytmp = g_malloc(m*sizeof(gdouble));
			mc = doAutoCorr(Dipole, Ytmp, m);
			for(k=0;k<M;k++) g_free(Dipole[k]);
			g_free(Dipole);
			for(k=0;k<MC;k++) Y[k] += Ytmp[k];
			g_free(Ytmp);
		}

		cur = cur->next;
	}
	if(nf>0) for(k=0;k<MC;k++) Y[k] /= nf;


	window = gabedit_xyplot_new_window(_("Dipole-Dipole autocorrelation"),NULL);
	xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");
	gabedit_xyplot_add_data_conv(GABEDIT_XYPLOT(xyplot),MC, X,  Y, 1.0, GABEDIT_XYPLOT_CONV_NONE,NULL);
	gabedit_xyplot_set_range_xmin (GABEDIT_XYPLOT(xyplot), 0.0);
	g_free(X); 
	g_free(Y);

	return TRUE;
}
/********************************************************************************/
void read_dipole_build_dipole_dipole_autocorrelation_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_dipole_text_file,
			_("Read dipole from an ascii file(fs,au,au,au) to do a dipole_dipole autocorrelation function"),
			GABEDIT_TYPEFILE_TXT,GABEDIT_TYPEWIN_OTHER);
	GtkWidget* entryN = gtk_entry_new();
	GtkWidget* hbox = gtk_hbox_new(FALSE,1);
	GtkWidget* hsep1 = gtk_hseparator_new();
	GtkWidget* hsep2 = gtk_hseparator_new();
	GtkWidget* labelN = gtk_label_new(_("     Number of steps to remove : "));

	gtk_entry_set_text(GTK_ENTRY(entryN),"0");

	gtk_box_pack_start (GTK_BOX (hbox), labelN, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), entryN, FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG(filesel)->vbox), hsep1, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG(filesel)->vbox), hbox, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG(filesel)->vbox), hsep2, FALSE, FALSE, 0);
	gtk_widget_show_all(hsep1);
	gtk_widget_show_all(hsep2);
	gtk_widget_show_all(hbox);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(filesel),TRUE);
	g_object_set_data (G_OBJECT (filesel), "EntryN",entryN);
}
/********************************************************************************/
GtkWidget* gabedit_dir_button()
{
	gboolean ret = FALSE;
	GtkWidget* buttonDirSelector;
	gchar* initial_dir = g_get_current_dir ();

	buttonDirSelector =  gtk_file_chooser_button_new(_("Select your folder"), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	ret = gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(buttonDirSelector), initial_dir);
	if( ! ret ) ret = gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(buttonDirSelector), "/");
	return buttonDirSelector;
}

/********************************************************************************************************/
void  add_cancel_ok_buttons(GtkWidget *Win, GtkWidget *vbox, GCallback myFunc)
{
	GtkWidget *hbox;
	GtkWidget *button;
	/* buttons box */
	hbox = create_hbox_false(vbox);
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
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)myFunc,GTK_OBJECT(Win));

	gtk_widget_show_all(vbox);
}
