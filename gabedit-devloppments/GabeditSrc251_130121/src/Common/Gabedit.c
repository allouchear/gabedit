/* Gabedit.c */
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
#include <gdk/gdk.h>
#include <gtk/gtkgl.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <stdlib.h>
#include <unistd.h>

#include "Global.h"
#include "MenuToolBar.h"
#include "TextEdit.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/GeomGlobal.h"
#include "SplashScreen.h"
#include "Install.h"
#include "../Files/ListeFiles.h"
#include "Windows.h"
#include "StockIcons.h"

GtkWidget *hseparator;

int main(int argc, char *argv[])
{

  GtkWidget* vboxp;
  gchar* poDir = NULL;
  gint ierr;

  srand((unsigned int)time(NULL));
  /*
  printf("%s",g_filename_from_utf8("My test\n",-1,0,0,0));
  exit(1);
  */
  /*
  g_setenv("LANG","en_US",TRUE);
  g_setenv("GDM_LANG","en_US",TRUE);
  */
   /* setlocale(LC_ALL,"");*/
   setlocale(LC_ALL,"C");
   poDir = g_build_filename (g_get_current_dir(),"locale",NULL);
   bindtextdomain (GETTEXT_PACKAGE, poDir);
   /* printf("poDir = %s\n",poDir);*/
   g_free (poDir);
   bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
   textdomain (GETTEXT_PACKAGE);

   /*
   if (!g_thread_supported ()){ g_thread_init (NULL); }
   gdk_threads_init ();
   gdk_threads_enter ();
   */
   

  gtk_init(&argc, &argv);
  gtk_gl_init(&argc, &argv);
  setlocale(LC_NUMERIC,"C");
  gabedit_gtk_stock_init();
  
/* Initialisation */
  initialise_global_variables();
  /* set_default_styles();*/

  Fenetre = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(Fenetre),"Gabedit");
  gtk_widget_set_size_request (GTK_WIDGET(Fenetre), 2*ScreenWidth/5, 2*ScreenHeight/5);
#ifdef G_OS_WIN32
  gtk_window_set_default_size (GTK_WINDOW(Fenetre), ScreenWidth, ScreenHeight-65);
  gtk_window_set_position(GTK_WINDOW(Fenetre),GTK_WIN_POS_CENTER);
#else
  gtk_window_set_default_size (GTK_WINDOW(Fenetre), ScreenWidth, ScreenHeight-80);
#endif
  gtk_widget_set_size_request (GTK_WIDGET(Fenetre), 2*ScreenWidth/5, 2*ScreenHeight/5);
  gtk_window_set_default_size (GTK_WINDOW(Fenetre), 2*ScreenWidth/5, 2*ScreenHeight/5);

  gtk_widget_realize(Fenetre);
  g_signal_connect(G_OBJECT(Fenetre), "delete_event",(GCallback)exit_all,NULL);

  cree_p_vbox();
  vboxp = vboxmain;

  add_menu_toolbar();

  cree_vboxs_list_text();
  cree_text_notebook();
  create_frame_windows(vboxp);
  cree_files_out_err_notebook(vboxtexts);

  read_ressource_file();
  /* set_default_styles();*/
  ListeFiles(vboxlistfiles);
  resetFontLabelErrOut();

  if(argc>1) get_doc(argv[1]);

  imodif = DATA_MOD_NO;
  set_icon_default();
  set_icone(Fenetre);

  user_install_verify(splash_screen);
  set_default_styles();
  set_path();
  ierr = chdir(g_get_home_dir());

#ifndef DRAWGEOMGL
  Message(_("\n\n"
	    "Many tools implemented in Gabedit/OpenGL version\nare not available in Gabedit/Cairo one.\n"
	    "\n"),
	    _("Warning"),TRUE); 
#endif

  gtk_main();
  /* gdk_threads_leave ();*/
 
  return 0;
}
