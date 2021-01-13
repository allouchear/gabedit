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

#ifndef __GABEDIT_ORBITALS_H__
#define __GABEDIT_ORBITALS_H__

void destroy_win_list();
void free_orbitals();
void applygrid(GtkWidget *Win,gpointer data);
void create_iso_orbitals();
void create_grid_orbitals();
void create_list_orbitals();
gboolean read_last_orbitals_in_gaussian_file(gchar *fileName,gint itype);
void read_gauss_orbitals(gchar* FileName);
void read_fchk_gauss_orbitals(gchar* fileName);
void read_gabedit_orbitals(gchar* FileName);
void read_molden_orbitals(gchar* FileName);
void read_dalton_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id);
void read_gamess_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id);
void read_nbo_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id);
void read_gauss_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id);
void read_fchk_gauss_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id);
void read_molpro_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id);
void read_orca_orbitals_sel_2mkl(GabeditFileChooser *SelecFile, gint response_id);
void read_orca_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id);
void read_qchem_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id);
void read_nwchem_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id);
void read_mopac_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id);
void read_gabedit_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id);
void save_gabedit_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id);
void save_wfx_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id);
void read_molden_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id);
void read_wfx_orbitals_sel(GabeditFileChooser *SelecFile, gint response_id);
void read_orbitals_sel(GabeditFileChooser *selecFile, gint response_id);

#endif /* __GABEDIT_ORBITALS_H__ */

