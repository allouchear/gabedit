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

#ifndef __GABEDIT_GRIDCUBE_H__
#define __GABEDIT_GRIDCUBE_H__

void square_cube();
void restriction_cube();
void load_cube_gauss_orbitals_file(GabeditFileChooser *SelecFile, gint response_id);
void load_cube_gauss_density_file(GabeditFileChooser *SelecFile, gint response_id);
void load_cube_gauss_gradient_file(GabeditFileChooser *SelecFile, gint response_id);
void load_cube_gauss_laplacian_file(GabeditFileChooser *SelecFile, gint response_id);
void load_cube_gauss_normgrad_file(GabeditFileChooser *SelecFile, gint response_id);
void load_cube_gauss_potential_file(GabeditFileChooser *SelecFile, gint response_id);
void load_cube_molpro_orbital_file(GabeditFileChooser *SelecFile, gint response_id);
void load_cube_molpro_orbitals_file(GabeditFileChooser *SelecFile, gint response_id);
void load_cube_molpro_density_file(GabeditFileChooser *SelecFile, gint response_id);
void load_cube_molpro_density_gradient_file(GabeditFileChooser *SelecFile, gint response_id);
void load_cube_molpro_laplacian_density_file(GabeditFileChooser *SelecFile, gint response_id);
void load_cube_molpro_laplacian_laplacian_file(GabeditFileChooser *SelecFile, gint response_id);
void load_cube_gabedit_file(GabeditFileChooser *SelecFile, gint response_id);
void load_dx_grid_file(GabeditFileChooser *selFile, gint response_id);
void create_scale_dlg();
void subtract_cube(GabeditFileChooser *SelecFile, gint response_id);
void mapping_cube(GabeditFileChooser *SelecFile, gint response_id);
void save_cube_gabedit_file(GabeditFileChooser *SelecFile, gint response_id);
void mapping_with_mep(gint N[],GridLimits limits, PoissonSolverMethod psMethod);
void mapping_with_mep_from_multipol(gint lmax);
void mapping_with_mep_from_charges();
void mapping_with_fed(gint n);
gboolean read_dx_grid_file(gchar* filename, gboolean showisowin);
gboolean read_gauss_molpro_cube_orbitals_file(gchar* filename,gint numorb,gint Norbs,gint typefile, gboolean showisowin);
void compute_integral_all_space();
void applyCutLeft0(GtkWidget *Win,gpointer data);
void applyCutRight0(GtkWidget *Win,gpointer data);
void applyCutLeft1(GtkWidget *Win,gpointer data);
void applyCutRight1(GtkWidget *Win,gpointer data);
void applyCutLeft2(GtkWidget *Win,gpointer data);
void applyCutRight2(GtkWidget *Win,gpointer data);

#endif /* __GABEDIT_GRIDCUBE_H__ */

