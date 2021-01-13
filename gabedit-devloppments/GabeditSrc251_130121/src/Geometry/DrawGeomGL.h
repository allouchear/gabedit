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

#ifndef __GABEDIT_DRAWGEOMGL_H__
#define __GABEDIT_DRAWGEOMGL_H__

#include "Fragments.h"
#include "../../gl2ps/gl2ps.h"
#include "../Geometry/AxesGeomGL.h"

typedef enum
{
ROTATION,ROTATIONZ,TRANSMOVIE,CENTER,
SCALEGEOM,SCALESTICK,SCALEBALL,SCALEDIPOLE,
SELECTOBJECTS,
SELECTFRAG,SELECTRESIDUE,
DELETEOBJECTS, /* Frag or atom or bond */
DELETEFRAG, MOVEFRAG,ROTLOCFRAG,ROTZLOCFRAG,
ADDFRAGMENT,
EDITOBJECTS, /* insert a atom(s) and or a bond, change a bond */
ADDATOMSBOND,
CHANGEBOND,CUTBOND,
MEASURE,SHOWMEASURE,
RENDERSTICK,RENDERBALL,
LABELNO,LABELSYMB,LABELNUMB,LABELMMTYP,LABELPDBTYP,LABELLAYER,LABELSYMBNUMB,
LABELCHARGE,LABELSYMBCHARGE,LABELNUMBCHARGE,LABELRESIDUES,LABELCOORDINATES,
FXYZ,FMOL2,FTINKER,FPDB,FHIN,FGABEDIT,FGZMAT,FMZMAT,
FDALTONIN,FDALTONFIRST,FDALTONLAST,
FGAMESSIN,FGAMESSFIRST,FGAMESSLAST,
FGAUSSIN,FGAUSSOUTFIRST,FGAUSSOUTLAST,
FMOLCASIN,FMOLCASOUTFIRST, FMOLCASOUTLAST,
FMOLPROIN,FMOLPROOUTFIRST, FMOLPROOUTLAST,
FMPQCIN,FMPQCOUTFIRST, FMPQCOUTLAST,
FGEOMCONVDALTON,FGEOMCONVGAMESS,FGEOMCONVGAUSS,FGEOMCONVMOLPRO, FGEOMCONVMOLDEN,FGEOMCONVGABEDIT,FGEOMCONVMPQC, FGEOMCONVXYZ,
SAVEJPEG, SAVEPPM, SAVEBMP, SAVEPS
}GabEditGeomOperation;

typedef enum
{
 RECTANGLE,CIRCLE,ATOMS
}GabEditSelectType;

typedef enum
{
 LOW_LAYER=0, MEDIUM_LAYER, HIGH_LAYER
}GabEditLayerType;

typedef struct _Camera
{
 gdouble position;
 gdouble f;
}Camera;

typedef struct _GeomDef
{
 gdouble X;
 gdouble Y;
 gdouble Z;
 gdouble Charge;
 SAtomsProp Prop;
 gchar* mmType;
 gchar* pdbType;
 gchar* Residue;
 gint ResidueNumber;
 gboolean show;
 gint Xi;
 gint Yi;
 gushort Rayon;
 gdouble Coefpers;
 guint N;
 gboolean ColorAlloc;
 GabEditLayerType Layer;
 gboolean Variable;
 gint* typeConnections;
}GeomDef;

typedef struct _GeomDraw
{
	gint nAtoms;
	GeomDef* atoms;
}GeomDraw;

typedef struct _CoordMaxMin
{
 gdouble Xmax;
 gdouble Xmin;
 gdouble Ymax;
 gdouble Ymin;
 gdouble Zmax;
 gdouble Zmin;
 gdouble Cmax;
}CoordMaxMin;

typedef struct _FragmentsItems
{
  gchar* Name;
  gchar* Menu;
  gint Num;
}FragmentsItems;

extern FragmentsItems *FragItems;
extern gint NFrags;

extern CoordMaxMin coordmaxmin;

extern GeomDef *geometry;
extern GeomDef *geometry0;
extern guint Natoms;

extern gint *NumFatoms;
extern guint NFatoms;

extern gint TransX;
extern gint TransY;
extern GtkWidget *GeomDlg;
extern GtkWidget *StopButton;
extern gboolean StopCalcul;

extern gboolean ShadMode;
extern gboolean PersMode;
extern gboolean LightMode;
extern gboolean OrtepMode;
extern gboolean DrawDistance;
extern gboolean DrawDipole;
extern gboolean ShowDipole;
extern gboolean ShowHBonds;

#define NDIVDIPOLE 20
extern gdouble dipole[NDIVDIPOLE][3];
extern gdouble dipole0[NDIVDIPOLE][3];
extern gdouble dipole00[NDIVDIPOLE][3];
extern gint DXi[NDIVDIPOLE];
extern gint DYi[NDIVDIPOLE];
extern gint Ndipole[NDIVDIPOLE];
extern gchar* AtomToInsert;
extern gint NumSelAtoms[4];
extern gboolean Ddef;


void createIstopeDistributionCalculationFromDrawGeom();
void add_geometry_to_fifo();
void get_geometry_from_fifo(gboolean toNext);
gdouble get_factorstick();
gdouble get_factorball();
gdouble get_factordipole();
gdouble get_factor();

void set_camera_drawgeom();
void set_light_geom_on_off(gint i);
void set_light_positions_drawgeom(gchar* title);
void  copy_screen_geom_clipboard();
void  reset_origine_molecule_drawgeom();
void  get_origine_molecule_drawgeom(gdouble orig[]);
gdouble  get_symprec_from_geomdlg();
void  get_orgin_molecule_drawgeom(gdouble orig[]);
void  get_camera_values_drawgeom(gdouble* zn, gdouble* zf, gdouble* angle, gdouble* aspect, gboolean* persp);
void create_drawmolecule_file();
void read_drawmolecule_file();
GeomDef* copyGeometry(GeomDef* geom0);
void freeGeometry(GeomDef* geom);
gint get_connection_type(gint i, gint j);
void setMMTypeOfselectedAtomsDlg();
void setPDBTypeOfselectedAtomsDlg();
void setResidueNameOfselectedAtomsDlg();
void setSymbolOfselectedAtomsDlg();
void setChargeOfselectedAtomsDlg();
void scaleChargesOfSelectedAtomsDlg();
void addMaxHydrogens();
void addHydrogens();
void addOneHydrogen();
void addHydrogensTpl();
void set_fix_selected_atoms();
void set_variable_selected_atoms();
void messageAmberTypesDefine();
gboolean getShowMultipleBonds();
void RenderMultipleBonds(GtkWidget *win,gboolean show);
GabEditGeomOperation getOperationType();
void set_origin_to_center_of_fragment();
void set_xyz_to_principal_axes_of_selected_atoms(gpointer data, guint Operation,GtkWidget* wid);
void create_tolerance_window(GtkWidget*w, gpointer data);
void get_abelian_orientation_with_reduction(GtkWidget*w, gpointer data);
void get_standard_orientation_with_reduction(GtkWidget*w, gpointer data);
void get_standard_orientation_with_symmetrization(GtkWidget*w, gpointer data);
void setMMTypesCharges(gpointer data, guint Operation,GtkWidget* wid);
GeomDef* Free_One_Geom(GeomDef* geom,gint N);
void set_text_to_draw(gchar* str);
void set_statubar_operation_str(gchar* str);
GdkPixmap* get_drawing_pixmap();
GdkColormap* get_drawing_colormap();
guint label_option();
gboolean distances_draw_mode();
gboolean ortho_mode();
gboolean stick_mode();
gboolean space_fill_mode();
gboolean pers_mode();
gboolean shad_mode();
gboolean light_mode();
gboolean ortep_mode();
gboolean cartoon_mode();
gdouble get_frag_angle();
void set_frag_angle(gdouble a);
gboolean hbond_connections(gint i, gint j);
gchar *get_distance(gint i,gint j);
gchar *get_angle(gint i,gint j,gint l);
gchar *get_dihedral(gint i,gint j,gint l,gint m);
void create_window_drawing();
void rafresh_drawing();
void draw_geometry(GtkWidget *,gpointer);
gboolean draw_lines_yes_no(guint i,guint j);
guint get_num_min_rayonIJ(guint i,guint j);
void SetRadioPopupMenu(gchar* button);
void HideShowMeasure(gboolean hiding);
void AdjustHydrogensYesNo(gboolean adjust);
gboolean getAdjustHydrogensYesNo();
void RebuildConnectionsDuringEditionYesNo(gboolean rebuild);
gboolean getRebuildConnectionsDuringEditionYesNo();
void ActivateButtonOperation (GtkWidget *widget, guint data);
void RenderStick();
void RenderBallStick();
void RenderSpaceFill();
gboolean dipole_draw_mode();
gboolean dipole_mode();
void CreateDrawMenu();
void TraitementGeom(gpointer data, guint Operation,GtkWidget* wid);
void copySelectedAtoms();
void DeleteMolecule();
void SetOriginAtCenter(gpointer data, guint Operation,GtkWidget* wid);
void read_geometries_convergence(gpointer data, guint Operation,GtkWidget* wid);
void save_geometry(gpointer data, guint Operation,GtkWidget* wid);
void read_geometry(gpointer data, guint Operation,GtkWidget* wid);
void factor_default(GtkWidget *win,gpointer d);
void factor_stick_default(GtkWidget *win,gpointer d);
void factor_ball_default(GtkWidget *win,gpointer d);
void factor_dipole_default(GtkWidget *win,gpointer d);
void factor_all_default(GtkWidget *win,gpointer d);
void RenderShad(GtkWidget *,gboolean);
void RenderPers(GtkWidget *,gboolean);
void RenderLight(GtkWidget *,gboolean);
void RenderOrtep(GtkWidget *,gboolean);
void RenderCartoon(GtkWidget *,gboolean);
void RenderDipole(GtkWidget *,gboolean);
void RenderAxes(GtkWidget *,gboolean);
void RenderBox(GtkWidget *,gboolean);
void RenderHBonds(GtkWidget *,gboolean);
void RenderHAtoms(GtkWidget *,gboolean);
void set_layer_of_selected_atoms(GabEditLayerType l);
void SetLabelDistances(GtkWidget *,gboolean);
void SetLabelDipole(GtkWidget *win,gboolean YesNo);
void SetLabelsOrtho(GtkWidget *,gboolean);
void initLabelOptions(guint data);
void SetLabelOptions(GtkWidget *widget, guint data);
void AddFragment(GtkWidget *widget, guint data);
void add_a_fragment(GtkWidget*win, gchar* fragName);
void addAFragment(gchar* fragName);
void set_back_color_black();
void open_color_dlg(GtkWidget *win,gpointer *DrawingArea);
void set_HBonds_dialog_geom(GtkWidget *win, guint data);
void set_povray_options_geom(GtkWidget *win, guint data);
void destroy_drawing_and_children(GtkWidget *win,gpointer data);
void SetOperation(GtkWidget *,guint);
void set_dipole_from_charges();
gdouble get_sum_charges();
void compute_total_charge();
void compute_charge_by_residue();
void compute_charge_of_selected_atoms();
void compute_dipole_from_charges();
void setPersonalFragment(Fragment F);
void unselect_all_atoms();
void hide_selected_atoms();
void hide_not_selected_atoms();
void show_all_atoms();
void show_hydrogen_atoms();
void deleteSelectedAtoms();
void deleteHydrogenAtoms();
void define_geometry();
void reset_multiple_bonds();
void reset_all_connections();
void resetConnections();
void resetConnectionsBetweenSelectedAndNotSelectedAtoms();
void resetConnectionsBetweenSelectedAtoms();
void resetMultipleConnections();
void moveCenterOfSelectedAtomsToOrigin();
void alignPrincipalAxesOfSelectedAtomsToXYZ();
void alignSelectedAndNotSelectedAtoms();
void reset_hydrogen_bonds();
void export_geometry(gchar* fileName, gchar* fileType);
void set_sensitive_stop_button(gboolean sens);
void reset_charges_multiplicities();
void copy_connections(GeomDef* geom0, GeomDef* geom, gint n);
void set_optimal_geom_view();
void getQuatGeom(gdouble q[]);
void redrawGeomGL2PS();
void drawGeom();
void getOptimalCiCj(gint i, gint j, gdouble* Ci, gdouble* Cj, gdouble* C0);
void set_xyz_to_standard_orientation_all();
void set_xyz_to_standard_orientation_selected_atoms();
void set_xyz_to_standard_orientation_not_selected_atoms();
void set_xyz_to_standard_orientation_selected_and_not_selected_atoms();
gboolean testShowBoxGeom();


#endif /* __GABEDIT_DRAWGEOMGL_H__ */

