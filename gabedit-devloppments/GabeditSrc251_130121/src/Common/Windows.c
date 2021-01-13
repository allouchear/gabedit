/* Windows.c */
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

#include "Global.h"
#include "../Utils/Utils.h"
#include "../Common/MenuToolBar.h"
#include <gdk/gdk.h>

/********************************************************************************/
void create_frame_windows(GtkWidget *box)
{
  GtkWidget *frame;
  GtkWidget *hbox;
  frame = gtk_frame_new (_("Windows"));
  g_object_ref (frame);
  g_object_set_data_full (G_OBJECT (Fenetre), "frame", frame, (GDestroyNotify) g_object_unref);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX (box), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_widget_set_size_request(GTK_WIDGET(frame),-1,(gint)(ScreenHeight*0.05));
/*  gtk_widget_set_size_request(GTK_WIDGET(frame),-1,height);*/
/*  Debug("height = %d\n",height);*/
  gtk_widget_show (frame);
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_widget_show (hbox);
  HboxWins = hbox;
  FrameWins = frame;
}
/********************************************************************************/
void destroy_button_windows(GtkWidget *Win)
{
  GtkWidget *button = g_object_get_data (G_OBJECT (Win), "ButtonWins");  
  if(button) gtk_widget_destroy (button);

  window_delete(Win);
}
/********************************************************************************/
void show_windows(GtkWidget *Win,gpointer data)
{
  gtk_widget_hide (Win);
  gtk_widget_show (Win);
}
/********************************************************************************/
GtkWidget *add_button_windows(gchar *title,GtkWidget *Win)
{
  GtkWidget *button;
  button = gtk_button_new_with_label(title);
  gtk_box_pack_start (GTK_BOX(HboxWins),button, FALSE, FALSE, 1);
  gtk_widget_show (button);
  g_object_set_data(G_OBJECT (Win), "ButtonWins", button);  
  g_object_set_data(G_OBJECT (Win), "ButtonStr", g_strdup(title));  
  g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)show_windows, GTK_OBJECT(Win));
  window_add(title,Win);
  return button;
}
/********************************************************************************/
