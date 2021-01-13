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

#ifndef __GABEDIT_GEOMORBXYZ_H__
#define __GABEDIT_GEOMORBXYZ_H__

void DefineType();
void reset_old_geometry();
gboolean this_is_a_new_geometry();
void free_geometry();
void gl_create_babel_read_dialogue();
void gl_read_xyz_file_sel(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_pdb_file_sel(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_hin_file_sel(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_aimall_file_sel(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_first_dalton_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_last_dalton_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_first_gamess_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_last_gamess_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_first_gauss_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_last_gauss_file(GabeditFileChooser *SelecFile, gint response_id);
gboolean gl_read_fchk_gauss_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_first_molcas_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_last_molcas_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_first_molpro_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_last_molpro_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_first_mpqc_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_last_mpqc_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_first_mopac_output_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_last_mopac_output_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_first_mopac_aux_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_last_mopac_aux_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_wfx_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_first_nwchem_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_last_nwchem_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_first_psicode_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_last_psicode_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_nbo_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_first_qchem_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_last_qchem_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_first_orca_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_last_orca_file(GabeditFileChooser *SelecFile, gint response_id);
void gl_read_hessian_orca_file(GabeditFileChooser *SelecFile, gint response_id);
gboolean gl_read_wfx_file_geom(gchar *fileName);
gboolean gl_read_xyz_file(gchar *FileName);
gboolean gl_read_molpro_file_geom_pos(gchar *FileName,long int pos);
gboolean gl_read_dalton_file_geomi(gchar *FileName,gint num);
gboolean gl_read_gamess_file_geomi(gchar *FileName,gint num);
gboolean gl_read_molpro_file_geomi(gchar *FileName,gint num);
gboolean gl_read_mopac_output_file_geomi(gchar *FileName,gint num);
gboolean gl_read_mopac_aux_file_geomi(gchar *FileName,gint num);
gboolean gl_read_molcas_file_geomi(gchar *FileName,gint num);
gboolean gl_read_gaussn_file_geomi(gchar *FileName,gint num);
gboolean gl_read_fchk_gaussn_file_geom(gchar *FileName);
gboolean gl_read_mpqc_file_geomi(gchar *FileName,gint num);
gboolean gl_read_orca_file_geomi(gchar *FileName,gint num);
gboolean gl_read_qchem_file_geomi(gchar *FileName,gint num);
gulong gl_read_nwchem_file_geomi(gchar *FileName,gint num);
gboolean gl_read_xyz_file_geomi(gchar *FileName,gint num);
void gl_read_gabedit_file(GabeditFileChooser *SelecFile, gint response_id);
gboolean gl_read_gabedit_file_geom(gchar *FileName);
void gl_read_molden_file(GabeditFileChooser *SelecFile, gint response_id);
gboolean gl_read_molden_file_geom(gchar* FileName);
gboolean gl_read_molden_gabedit_geom_conv_file(gchar *fileName, gint geometryNumber, GabEditTypeFile type);
gdouble get_cube_length();
void reset_grid_limits();
void gl_get_charges_from_mopac_output_file(FILE* fd);
void gl_read_geom_any_file_sel(GabeditFileChooser *selecFile, gint response_id);

#endif /* __GABEDIT_GEOMORBXYZ_H__ */

