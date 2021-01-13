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

#ifndef __GABEDIT_GEOMXYZ_H__
#define __GABEDIT_GEOMXYZ_H__

void compute_dipole_using_charges_of_xyz_geom();
void set_variable_one_atom_in_GeomXYZ(gint i);
void create_geomXYZ_list(GtkWidget *, GabEditTypeFileGeom readfile);
void create_variablesXYZ_list(GtkWidget *,guint);
void create_babel_read_dialogue();
void read_gabedit_file_add_list(gchar *NomFichier);
void read_gabedit_file_no_add_list(gchar *NomFichier);
void read_XYZ_file_no_add_list(G_CONST_RETURN  gchar *);
void read_mol2_tinker_file_no_add_list(gchar *NomFichier,gchar*type);
void read_pdb_file_no_add_list(gchar *NomFichier);
void read_hin_file_no_add_list(gchar *NomFichier);
void read_XYZ_from_demon_input_file(gchar *NomFichier, FilePosTypeGeom InfoFile );
void read_XYZ_from_gauss_input_file(gchar *NomFichier, FilePosTypeGeom InfoFile );
void read_XYZ_from_gamess_input_file(gchar *NomFichier );
void read_XYZ_from_mpqc_input_file(gchar *NomFichier );
void read_XYZ_from_nwchem_input_file(gchar *NomFichier );
void read_XYZ_from_psicode_input_file(gchar *NomFichier );
void read_XYZ_from_orca_input_file(gchar *NomFichier );
void read_XYZ_from_qchem_input_file(gchar *NomFichier );
void read_XYZ_from_mopac_input_file(gchar *NomFichier );
void read_geom_conv_from_dalton_output_file(gchar *NomFichier, gint numgeometry);
void read_geom_conv_from_gamess_output_file(gchar *NomFichier, gint numgeometry);
void read_geom_from_molden_geom_conv_file(gchar *fileName, gint numgeometry);
void read_geom_from_gabedit_geom_conv_file(gchar *fileName, gint numgeometry);
void read_geom_from_gamess_output_file(gchar *NomFichier, gint numgeometry);
void read_geom_from_gaussian_file(gchar *NomFichier, gint numgeometry);
void read_geom_from_mpqc_output_file(gchar *fileName, gint numGeometry);
void read_geom_from_molpro_file(gchar *NomFichier, gint numgeometry);
void read_geom_from_orca_file(gchar *NomFichier, gint numgeometry);
void read_geom_from_vasp_file(gchar *NomFichier, gint numgeometry);
void read_geom_from_vasp_xml_file(gchar *NomFichier, gint numgeometry);
void read_geom_from_nwchem_file(gchar *NomFichier, gint numgeometry);
void read_geom_from_psicode_file(gchar *NomFichier, gint numgeometry);
void read_geom_from_qchem_file(gchar *NomFichier, gint numgeometry);
void read_geom_from_mopac_output_file(gchar *NomFichier, gint numgeometry);
void read_geom_from_gamess_irc_file(gchar *NomFichier, gint numgeometry);
void read_XYZ_from_mopac_irc_output_file(gchar *FileName, gint numGeom);
void read_XYZ_from_mopac_scan_output_file(gchar *FileName, gint numGeom);
void read_geom_from_mopac_scan_output_file(gchar *FileName, gint numGeom);
void read_geom_from_mopac_irc_output_file(gchar *FileName, gint numGeom);
void read_geom_from_mopac_aux_file(gchar *NomFichier, gint numgeometry);
void read_geom_from_xyz_file(gchar *fileName, gint numGeom);
void read_geom_from_poscar_file(gchar *fileName);
void read_XYZ_from_molpro_input_file(gchar *NomFichier, FilePosTypeGeom InfoFile);
void create_GeomXYZ_from_draw_grometry();
void freeGeomXYZ();
void freeVariablesXYZ();
void FreeGeomXYZ(GeomXYZAtomDef* GeomXYZtemp, VariablesXYZDef* VariablesXYZtemp, gint Ncent, gint Nvar);
void selc_cif_file_nosym();
void selc_XYZ_file(GabEditTypeFileGeom itype);
void create_babel_save_dialogue();
void save_geometry_poscar_file(GabeditFileChooser *SelecFile, gint response_id);
void save_geometry_poscar_direct_file(GabeditFileChooser *SelecFile, gint response_id);
void save_geometry_cif_allatoms_file(GabeditFileChooser *SelecFile, gint response_id);
void save_geometry_cif_file(GabeditFileChooser *SelecFile, gint response_id);
void save_geometry_xyz_file(GabeditFileChooser *SelecFile, gint response_id);
void save_geometry_mol_file(GabeditFileChooser *SelecFile, gint response_id);
void save_geometry_mol2_file(GabeditFileChooser *SelecFile, gint response_id);
void save_geometry_tinker_file(GabeditFileChooser *SelecFile, gint response_id);
void save_geometry_pdb_file(GabeditFileChooser *SelecFile, gint response_id);
void save_geometry_hin_file(GabeditFileChooser *SelecFile, gint response_id);
void save_geometry_gabedit_file(GabeditFileChooser *SelecFile, gint response_id);
void save_geometry_cchemi_file(GabeditFileChooser *SelecFile, gint response_id);
void save_xyz_file_no_add_list(const gchar* FileName);
void save_poscar_file_no_add_list(const gchar* FileName);
gboolean connecteds(guint i,guint j);

void read_hin_file(GabeditFileChooser *SelecFile, gint response_id);
void read_gabedit_file(GabeditFileChooser *SelecFile, gint response_id);
void read_first_mpqc_file(GabeditFileChooser *SelecFile, gint response_id);
void read_last_mpqc_file(GabeditFileChooser *SelecFile, gint response_id);
void read_first_dalton_file(GabeditFileChooser *SelecFile, gint response_id);
void read_last_dalton_file(GabeditFileChooser *SelecFile, gint response_id);
void read_first_gamess_file(GabeditFileChooser *SelecFile, gint response_id);
void read_last_gamess_file(GabeditFileChooser *SelecFile, gint response_id);
void read_aimall_file(GabeditFileChooser *SelecFile, gint response_id);
void read_first_turbomole_file(GabeditFileChooser *SelecFile, gint response_id);
void read_last_turbomole_file(GabeditFileChooser *SelecFile, gint response_id);
void read_last_gaussian_file(GabeditFileChooser *SelecFile , gint response_id);
void read_first_gaussian_file(GabeditFileChooser *SelecFile, gint response_id);
void read_fchk_gaussian_file(GabeditFileChooser *SelecFile , gint response_id);
void read_last_molcas_file(GabeditFileChooser *SelecFile , gint response_id);
void read_last_molpro_file(GabeditFileChooser *SelecFile , gint response_id);
void read_first_orca_file(GabeditFileChooser *SelecFile , gint response_id);
void read_last_orca_file(GabeditFileChooser *SelecFile , gint response_id);
void read_first_vasp_file(GabeditFileChooser *SelecFile , gint response_id);
void read_last_vasp_file(GabeditFileChooser *SelecFile , gint response_id);
void read_first_nwchem_file(GabeditFileChooser *SelecFile , gint response_id);
void read_first_psicode_file(GabeditFileChooser *SelecFile , gint response_id);
void read_last_nwchem_file(GabeditFileChooser *SelecFile , gint response_id);
void read_last_psicode_file(GabeditFileChooser *SelecFile , gint response_id);
void read_first_qchem_file(GabeditFileChooser *SelecFile , gint response_id);
void read_last_qchem_file(GabeditFileChooser *SelecFile , gint response_id);
void read_first_mopac_output_file(GabeditFileChooser *SelecFile , gint response_id);
void read_last_mopac_output_file(GabeditFileChooser *SelecFile , gint response_id);
void read_last_mopac_aux_file(GabeditFileChooser *SelecFile , gint response_id);
void read_last_irc_gamess_file(GabeditFileChooser *SelecFile, gint response_id);
void read_first_molcas_file(GabeditFileChooser *SelecFile, gint response_id);
void read_first_molpro_file(GabeditFileChooser *SelecFile, gint response_id);
void read_mol2_file(GabeditFileChooser *SelecFile , gint response_id);
void read_tinker_file(GabeditFileChooser *SelecFile, gint response_id);
void read_pdb_file(GabeditFileChooser *SelecFile, gint response_id);
void read_XYZ_file(GabeditFileChooser *SelecFile, gint  response_id);
void reset_connections_XYZ();
void compute_volume_cell();
void compute_volume_molecule();
GtkWidget* scale_cell_dialog ();


#endif /* __GABEDIT_GEOMXYZ_H__ */

