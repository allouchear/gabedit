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

#ifndef __GABEDIT_UTILSORB_H__
#define __GABEDIT_UTILSORB_H__

void InitializeAll();
gint GetTotalNelectrons();
gdouble GetSumAbsCharges();
void DefineNOccs();
gdouble Dpn(gdouble e,gint n);
char GetSymmetry(gint l);
gint GetNelectrons(char *symb);
void printLineChar(char c,gint n);
gdouble **CalculSm12(gdouble *S,gint n,gint nvec);
gdouble** CreateTable2(gint N);
gdouble** FreeTable2(gdouble **T,gint N);
void PrintAllOrb(gdouble** M);
gint get_type_file_orb(gchar *fileName);
gint get_type_basis_in_gamess_file(gchar *fileName);
gint get_type_basis_in_gaussian_file(gchar *fileName);
gint get_type_basis_in_qchem_file(gchar *fileName);
gint get_type_basis_in_nwchem_file(gchar *fileName);
gint get_type_basis_in_gabedit_file(gchar *fileName);
gint get_type_basis_in_molden_file(gchar *fileName);
gboolean testi(char c);
gboolean this_is_a_real(gchar *t);
gboolean this_is_a_integer(gchar *t);
gboolean testapointeE(char c);
gboolean testpointeE(const gchar *t);
void free_data_all();
GtkWidget *create_grid_frame( GtkWidget *vboxall,gchar *title);
void create_grid(gchar* title);
void read_any_file(gchar* FileName);
gint get_number_of_point(GtkWidget* Entry);
gboolean get_a_float(GtkWidget* Entry,gdouble* value, gchar* erroMessage);
void initialise_global_orbitals_variables();
void close_window_orb(GtkWidget*win, gpointer data);
void add_glarea_child(GtkWidget* winchild,gchar* title);
void create_color_surfaces_file();
void read_color_surfaces_file();
void create_opengl_file();
void read_opengl_file();
void create_grid_sas(gchar* title);
void set_scale_ball_stick_dlg();
void createColorMapOptionsWindow(GtkWidget* win);
void create_grid_ELF_Dens_analyze(gboolean ongrid);
GtkWidget* set_alphaFED_dialog ();
void resetAllColorMapOrb();

#endif /* __GABEDIT_UTILSORB_H__ */

