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

#ifndef __GABEDIT_RESULTSANALISE_H__
#define __GABEDIT_RESULTSANALISE_H__

typedef struct _DataGeomConv
{
	gint Npoint;
	gint Ntype;
	gchar *TypeCalcul; 
	gchar **TypeData; 
	gchar ***Data; 
	gint *NumGeom; 
	gchar *GeomFile; 
	GabEditTypeFile fileType; 
}DataGeomConv;
void set_sensitive_remote_frame(gboolean sensitive);
void create_bar_result(GtkWidget* Vbox);
DataGeomConv free_geom_conv(DataGeomConv);
void find_energy_all(GtkWidget *wid,gpointer data);
void read_geometries_conv_dalton(GabeditFileChooser *SelecFile, gint response_id);
void read_geometries_conv_gamess(GabeditFileChooser *SelecFile, gint response_id);
void read_geometries_conv_gaussian(GabeditFileChooser *SelecFile, gint response_id);
void read_geometries_conv_molpro(GabeditFileChooser *SelecFile, gint response_id);
void read_geometries_conv_mopac(GabeditFileChooser *SelecFile, gint response_id);
void read_geometries_conv_mopac_scan(GabeditFileChooser *SelecFile, gint response_id);
void read_geometries_conv_mopac_irc(GabeditFileChooser *SelecFile, gint response_id);
void read_geometries_conv_orca(GabeditFileChooser *SelecFile, gint response_id);
void read_geometries_conv_vasp_outcar(GabeditFileChooser *SelecFile, gint response_id);
void read_geometries_conv_vasp_xml(GabeditFileChooser *SelecFile, gint response_id);
void read_geometries_conv_qchem(GabeditFileChooser *SelecFile, gint response_id);
void read_geometries_conv_nwchem(GabeditFileChooser *SelecFile, gint response_id);
void read_geometries_conv_psicode(GabeditFileChooser *SelecFile, gint response_id);
void read_geometries_conv_gabedit(GabeditFileChooser *SelecFile, gint response_id);
void read_geometries_conv_molden(GabeditFileChooser *SelecFile, gint response_id);
void read_geometries_conv_mpqc(GabeditFileChooser *SelecFile, gint response_id);
void read_geometries_conv_xyz(GabeditFileChooser *SelecFile, gint response_id);
void read_geometries_irc_gamess(GabeditFileChooser *SelecFile, gint response_id);
void find_energy_molden(gchar* FileName);
void find_energy_gabedit(gchar* FileName);
void find_energy_gamess_output(gchar* fileName);
void find_energy_gamess_output_heat(gchar* fileName);
void find_energy_irc_output(gchar* fileName);
void find_energy_gauss_output(gchar* fileName);
void find_energy_mpqc_output(gchar* fileName);
void find_energy_molpro_output(gchar* fileName);
void find_energy_orca_output(gchar* fileName);
void find_energy_qchem_output(gchar* fileName);
gint find_energy_mopac_aux(gchar* fileName);
void find_energy_mopac_scan_output(gchar* fileName);
void find_energy_mopac_irc_output(gchar* fileName);

#endif /* __GABEDIT_RESULTSANALISE_H__ */

