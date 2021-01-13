/* Preferences.h */
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

#ifndef __GABEDIT_PREFERENCES_H__
#define __GABEDIT_PREFERENCES_H__

void create_preferences();
void  create_execucte_commands(GtkWidget *Wins,GtkWidget *Box, gboolean expand);
void  create_network_protocols(GtkWidget *Wins,GtkWidget *Box,gboolean expand);
void  modify_gaussian_command();
void  modify_molpro_command();
void  create_font_color_in_box(GtkWidget *Win,GtkWidget *Box);
void set_font_other (gchar *fontname);
#ifdef G_OS_WIN32
void  create_gamess_directory(GtkWidget *Wins,GtkWidget *vbox,gboolean expand);
void  create_pscpplink_directory(GtkWidget *Wins,GtkWidget *vbox,gboolean expand);
#endif

#endif /* __GABEDIT_PREFERENCES_H__ */

