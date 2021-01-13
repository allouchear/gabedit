
#ifndef __GABEDIT_BUILDCRYSTAL_H__
#define __GABEDIT_BUILDCRYSTAL_H__

void build_crystal_dlg();
void build_supercell_simple_dlg();
void build_supercell_dlg();
void build_wulff_dlg();
void build_slab_dlg();
gboolean wrap_atoms_to_cell();
void help_references_crystallography();
gboolean reduce_cell_niggli();
gboolean reduce_cell_delaunay();
gboolean reduce_cell_primitive();
void compute_space_symmetry_group();
void compute_symmetry_info();
void standardize_cell_primitive();
void standardize_cell_conventional();
void setSymPrecDlg();
void compute_kpoints_path();
void setSymmetryPrecision(GtkWidget* GeomDlg, G_CONST_RETURN gchar* value);

#endif /* __GABEDIT_BUILDCRYSTAL_H__ */

