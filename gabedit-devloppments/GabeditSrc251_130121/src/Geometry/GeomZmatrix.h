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

#ifndef __GABEDIT_GEOMZMATRIX_H__
#define __GABEDIT_GEOMZMATRIX_H__

void set_variable_one_atom_in_GeomZMatrix(gint i);
void create_geom_list(GtkWidget *,guint);
void create_variables_list(GtkWidget *,guint);
void read_ZMatrix_file_no_add_list(gchar* );
void read_ZMatrix_mopac_file_no_add_list(gchar*FileName);
void read_Zmat_from_demon_input_file(gchar *, FilePosTypeGeom  );
void read_Zmat_from_gauss_input_file(gchar *, FilePosTypeGeom  );
void read_Zmat_from_orca_input_file(gchar *NomFichier);
void read_Zmat_from_nwchem_input_file(gchar *NomFichier);
void read_Zmat_from_psicode_input_file(gchar *NomFichier);
void read_Zmat_from_qchem_input_file(gchar *NomFichier);
void read_Zmat_from_mopac_input_file(gchar *NomFichier);
void read_Zmat_from_mopac_irc_output_file(gchar *FileName, gint numGeom);
void read_Zmat_from_mopac_scan_output_file(gchar *FileName, gint numGeom);
gint read_Zmat_from_molcas_input_file(gchar *fileName);
void read_Zmat_from_molpro_input_file(gchar *, FilePosTypeGeom  );
void freeGeom();
void freeVariables();
void selc_ZMatrix_mopac_file();
void selc_ZMatrix_file();
void save_geometry_gzmatrix_file(GabeditFileChooser *SelecFile, gint response_id);
void save_geometry_mzmatrix_file(GabeditFileChooser *SelecFile, gint response_id);
void read_ZMatrix_file(GabeditFileChooser *SelecteurFichier, gint response_id);
void read_ZMatrix_mopac_file(GabeditFileChooser *SelecteurFichier, gint response_id);

#endif /* __GABEDIT_GEOMZMATRIX_H__ */
