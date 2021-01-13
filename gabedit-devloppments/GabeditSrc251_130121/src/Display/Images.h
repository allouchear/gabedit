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

#ifndef __GABEDIT_IMAGES_H__
#define __GABEDIT_IMAGES_H__

void save_bmp_file(GabeditFileChooser *SelecFile, gint response_id);
void save_png_without_background_file(GabeditFileChooser *SelecFile, gint response_id);
void save_png_file(GabeditFileChooser *SelecFile, gint response_id);
void save_jpeg_file(GabeditFileChooser *SelecFile, gint response_id);
void copy_to_clipboard();
void copy_to_clipboard_without_background();
void save_ppm_file(GabeditFileChooser *SelecFile, gint response_id);
void save_ps_file(GabeditFileChooser *SelecFile, gint response_id);
gchar* new_bmp(gchar* dirname, int i);
gchar* new_ppm(gchar* dirname, int i);
gchar* new_jpeg(gchar* dirname, int i);
gchar* new_png(gchar* dirname, int i);
gchar* new_png_without_background(gchar* dirname, int i);
void gabedit_save_image_gl(GtkWidget* widget, gchar *fileName, gchar* type, guchar* colorTrans);

#endif /* __GABEDIT_IMAGES_H__ */

