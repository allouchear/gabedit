/* MenuToolBarGeom.c */
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


#include "../../Config.h"
#include <stdlib.h>
#include <math.h>

#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Utils/Utils.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/Measure.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/Povray.h"
#include "../Common/Windows.h"
#include "../Utils/Transformation.h"
#include "../MolecularMechanics/MolecularMechanicsDlg.h"
#include "../MolecularMechanics/SetMMParameters.h"
#include "../MolecularMechanics/SetPDBTemplate.h"
#include "../SemiEmpirical/SemiEmpiricalDlg.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/GeomZmatrix.h"
#include "../Geometry/GeomSymmetry.h"
#include "../Files/FileChooser.h"
#include "../Geometry/ImagesGeom.h"
#include "../Geometry/RotFragments.h"
#include "../Geometry/BuildLinear.h"
#include "../Geometry/BuildRing.h"
#include "../Geometry/BuildRoZPhi.h"
#include "../Geometry/BuildPolyPeptide.h"
#include "../Geometry/BuildCrystal.h"
#include "../Geometry/BuildPolySaccharide.h"
#include "../Geometry/BuildPolyNucleicAcid.h"
#include "../Geometry/BuildNanoTube.h"
#include "../Geometry/PersonalFragments.h"
#include "../Geometry/ResultsAnalise.h"
#include "../Geometry/GeomConversion.h"
#include "../Geometry/MenuToolBarGeom.h"
#include "../Geometry/OpenBabel.h"
#include "../Geometry/FragmentsSelector.h"
#include "../Geometry/ExportGeom.h"
#include "../Utils/HydrogenBond.h"
#include "../Symmetry/MoleculeSymmetryInterface.h"
#include "../Common/StockIcons.h"
#include "../Geometry/AxesGeomGL.h"
#include "../Geometry/SelectionDlg.h"


/* #define EXPERIMENTAL 1*/
/*********************************************************************************************************************/
void activate_insert_crystal();
static	GtkUIManager *manager = NULL;
static GtkWidget* handleBoxToolBar = NULL;
static gboolean ViewToolBar = TRUE;
static	GtkToolbar* toolBar = NULL;
/******************************************************************/
static void view_toolbar ()
{
	if(!handleBoxToolBar) return;
	if(ViewToolBar)
 		gtk_widget_hide (GTK_WIDGET(handleBoxToolBar));
	else
 		gtk_widget_show (GTK_WIDGET(handleBoxToolBar));
	ViewToolBar = !ViewToolBar;
}
/********************************************************************************/
static void set_atom(GtkWidget *button,gpointer data)
{
	GtkWidget* WinTable = g_object_get_data(G_OBJECT(button), "WinTable");
	/*
	GtkStyle *button_style = gtk_widget_get_style(WinTable); 
	gchar* told;
	gtk_button_set_label(GTK_BUTTON(AtomButton), (gchar*)data);
	told = GTK_BUTTON(AtomButton)->label_text; 
	GTK_BUTTON(AtomButton)->label_text = g_strdup(data);
	if(told) g_free(told);
	set_button_style(button_style,AtomButton,(gchar*)data);
	*/
	if(AtomToInsert) g_free(AtomToInsert);
	AtomToInsert = g_strdup((gchar*)data);
	if(GTK_IS_WIDGET(WinTable)) gtk_widget_destroy(WinTable);
}
/********************************************************************************/
static void select_atom()
{
	GtkWidget* Table;
	GtkWidget* button;
	GtkWidget* frame;
	GtkWidget* WinTable;
	guint i;
	guint j;
        GtkStyle *button_style;
        /* GtkStyle *style;*/

	gchar*** Symb = get_periodic_table();

	WinTable = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_modal(GTK_WINDOW(WinTable),TRUE);
	gtk_window_set_title(GTK_WINDOW(WinTable),_("Select your atom"));
	gtk_window_set_default_size (GTK_WINDOW(WinTable),(gint)(ScreenWidth*0.5),(gint)(ScreenHeight*0.4));

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

	gtk_container_add(GTK_CONTAINER(WinTable),frame);  
	gtk_widget_show (frame);

	Table = gtk_table_new(PERIODIC_TABLE_N_ROWS-1,PERIODIC_TABLE_N_COLUMNS,TRUE);
	gtk_container_add(GTK_CONTAINER(frame),Table);
	button_style = gtk_widget_get_style(WinTable); 
  
	for ( i = 0;i<PERIODIC_TABLE_N_ROWS-1;i++)
		for ( j = 0;j<PERIODIC_TABLE_N_COLUMNS;j++)
	{
		if(strcmp(Symb[j][i],"00"))
		{
			button = gtk_button_new_with_label(Symb[j][i]);
			/* style=set_button_style(button_style,button,Symb[j][i]);*/
			set_button_style(button_style,button,Symb[j][i]);
			g_object_set_data(G_OBJECT(button), "WinTable", WinTable);
			g_signal_connect(G_OBJECT(button), "clicked", (GCallback)set_atom,(gpointer )Symb[j][i]);
			gtk_table_attach(GTK_TABLE(Table),button,j,j+1,i,i+1,
				(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
				(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
		}
	}
 	
  	gtk_window_set_transient_for(GTK_WINDOW(WinTable),GTK_WINDOW(GeomDlg));
	gtk_widget_show_all(WinTable);
}
/*********************************************************************************************************************/
enum 
{
	OPERATION_TRANSLATE,
	OPERATION_ROTATION,
	OPERATION_ROTATION_Z,
	OPERATION_ZOOM,
	OPERATION_SCALE_STICK,
	OPERATION_SCALE_BALL,
	OPERATION_SCALE_DIPOLE,
	OPERATION_SELECTION_ATOMS,
	OPERATION_DELETE_OBJECTS,
	OPERATION_MOVE_ATOMS,
	OPERATION_ROTATION_ATOMS,
	OPERATION_ROTATION_Z_ATOMS,
	OPERATION_EDIT_OBJECTS,
	OPERATION_INSERT_FRAG,
	OPERATION_MEASURE,
};
static void render_operation_radio_action (GtkAction *action)
{
	gint value = gtk_radio_action_get_current_value (GTK_RADIO_ACTION (action));
	GtkWidget *atomToInsert = gtk_ui_manager_get_widget (manager, "/ToolbarGL/SetAtomToInsert");

	if(GTK_IS_WIDGET(atomToInsert)) gtk_widget_set_sensitive(atomToInsert, FALSE);

	switch(value)
	{
		case OPERATION_TRANSLATE : SetOperation(NULL, TRANSMOVIE); break;
		case  OPERATION_ROTATION : SetOperation(NULL, ROTATION); break;
		case OPERATION_ROTATION_Z : SetOperation(NULL,ROTATIONZ ); break;
		case  OPERATION_ZOOM : SetOperation(NULL, SCALEGEOM ); break;
		case  OPERATION_SCALE_STICK : SetOperation(NULL, SCALESTICK); break;
		case  OPERATION_SCALE_BALL : SetOperation(NULL, SCALEBALL); break;
		case  OPERATION_SCALE_DIPOLE : SetOperation(NULL, SCALEDIPOLE); break;
		case  OPERATION_SELECTION_ATOMS : SetOperation(NULL, SELECTOBJECTS); break;
		case OPERATION_DELETE_OBJECTS : SetOperation(NULL,DELETEOBJECTS ); break;
		case  OPERATION_MOVE_ATOMS : SetOperation(NULL, MOVEFRAG); break;
		case  OPERATION_ROTATION_ATOMS : SetOperation(NULL, ROTLOCFRAG ); break;
		case   OPERATION_ROTATION_Z_ATOMS : SetOperation(NULL, ROTZLOCFRAG); break;
		case   OPERATION_EDIT_OBJECTS : 
			    SetOperation(NULL, EDITOBJECTS );
			    if(GTK_IS_WIDGET(atomToInsert)) gtk_widget_set_sensitive(atomToInsert, TRUE);
			    break;
		case   OPERATION_INSERT_FRAG : 
				    		SetOperation(NULL, ADDFRAGMENT ); 
						create_window_fragments_selector("Functionals","Amine");
						break;
		case   OPERATION_MEASURE :
				    SetOperation(NULL, MEASURE );
				    {
					GtkAction * action = gtk_ui_manager_get_action(manager, "/ToolbarGL/ShowMeasureNoteBook");
					GtkWidget *notebook = gtk_ui_manager_get_widget (manager, "/ToolbarGL/ShowMeasureNoteBook");
					gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action), TRUE);
					gtk_widget_set_sensitive(notebook, FALSE);
				    }
				    break;
	}
	if(value != OPERATION_MEASURE) 
	{
		GtkWidget *notebook = gtk_ui_manager_get_widget (manager, "/ToolbarGL/ShowMeasureNoteBook");
		gtk_widget_set_sensitive(notebook, TRUE);
	}
}
static GtkRadioActionEntry operationsEntries[] = {
  { "OperationsTranslate", GABEDIT_STOCK_TRANSLATE, N_("_Translate"), NULL, "Translation", OPERATION_TRANSLATE },
  { "OperationsRotation", GABEDIT_STOCK_ROTATION, N_("_Rotation"), NULL, "Rotation", OPERATION_ROTATION },
  { "OperationsRotationZ", GABEDIT_STOCK_ROTATION_Z, N_("Rotation about _z axis"), NULL, "Rotation about z axis", OPERATION_ROTATION_Z },
  { "OperationsZoom", GABEDIT_STOCK_ZOOM, N_("Zo_om"), NULL, "Zoom", OPERATION_ZOOM },
  { "OperationsScaleStick", GABEDIT_STOCK_SCALE_STICK, N_("Scale _stick"), NULL, "Scale stick", OPERATION_SCALE_STICK },
  { "OperationsScaleBall", GABEDIT_STOCK_SCALE_BALL, N_("Scale _ball"), NULL, "Scale ball", OPERATION_SCALE_BALL },
  { "OperationsScaleDipole", GABEDIT_STOCK_SCALE_DIPOLE, N_("Scale _dipole"), NULL, "Scale dipole", OPERATION_SCALE_DIPOLE },
  { "OperationsSelectionOfAtoms", GABEDIT_STOCK_SELECT_RESIDUE, N_("_Selection of atoms"), NULL, "Selection of atoms", OPERATION_SELECTION_ATOMS },
  { "OperationsDeleteObjects", GABEDIT_STOCK_DELETE_CUT, N_("_Delete selected atoms/bond"), NULL, "Delete selected atoms/bond", OPERATION_DELETE_OBJECTS },
  { "OperationsMoveAtoms", GABEDIT_STOCK_MOVE_ATOM, N_("_Move selected atoms"), NULL, "Move selected atoms", OPERATION_MOVE_ATOMS },
  { "OperationsRotationAtoms", GABEDIT_STOCK_ROTATION_LOCAL, N_("R_otation of selected atoms"), NULL, "Rotation of selected atoms", OPERATION_ROTATION_ATOMS },
  { "OperationsRotationZAtoms", GABEDIT_STOCK_ROTATION_Z_LOCAL, N_("Rotation, about _z axis, of selected atoms"), NULL, "Rotation, about z axis, of selected atoms", OPERATION_ROTATION_Z_ATOMS },

  { "OperationsEditObjects", GABEDIT_STOCK_DRAW, N_("_Insert/Change atoms or bond"), NULL, "Insert/Change atoms or bond", OPERATION_EDIT_OBJECTS },
  { "OperationsInsertFrag", GABEDIT_STOCK_IFRAG, N_("_Insert a fragment"), NULL, "Insert a fragment", OPERATION_INSERT_FRAG},
  { "OperationsMeasure", GABEDIT_STOCK_MEASURE, N_("_Measure"), NULL, "Measure", OPERATION_MEASURE },
};
static guint numberOfOperationsEntries = G_N_ELEMENTS (operationsEntries);
/*********************************************************************************************************************/
enum 
{
	LABEL_NO,
	LABEL_SYMBOLS,
	LABEL_NUMBERS,
	LABEL_SYMBOLS_NUMBERS,
	LABEL_CHARGES,
	LABEL_SYMBOLS_CHARGES,
	LABEL_NUMBERS_CHARGES,
	LABEL_REDIDUES,
	LABEL_COORDINATES,
	LABEL_MMTYPES,
	LABEL_PDBTYPES,
	LABEL_LAYERS,
};
static void render_label_radio_action (GtkAction *action)
{
	gint value = gtk_radio_action_get_current_value (GTK_RADIO_ACTION (action));

	switch(value)
	{
		case LABEL_NO : SetLabelOptions (NULL,  LABELNO); break;
		case  LABEL_SYMBOLS : SetLabelOptions (NULL, LABELSYMB ); break;
		case  LABEL_NUMBERS : SetLabelOptions (NULL,  LABELNUMB); break;
		case  LABEL_MMTYPES : SetLabelOptions (NULL,  LABELMMTYP); break;
		case  LABEL_PDBTYPES : SetLabelOptions (NULL,  LABELPDBTYP); break;
		case  LABEL_LAYERS : SetLabelOptions (NULL,  LABELLAYER); break;
		case  LABEL_SYMBOLS_NUMBERS : SetLabelOptions (NULL,  LABELSYMBNUMB); break;
		case  LABEL_CHARGES : SetLabelOptions (NULL,  LABELCHARGE); break;
		case  LABEL_SYMBOLS_CHARGES: SetLabelOptions (NULL,  LABELSYMBCHARGE); break;
		case  LABEL_NUMBERS_CHARGES: SetLabelOptions (NULL,  LABELNUMBCHARGE); break;
		case  LABEL_REDIDUES: SetLabelOptions (NULL,  LABELRESIDUES); break;
		case  LABEL_COORDINATES: SetLabelOptions (NULL,  LABELCOORDINATES); break;
	}
}
static GtkRadioActionEntry labelEntries[] = {
  { "LabelsNothing", NULL, N_("_Nothing"), NULL, "no labels", LABEL_NO },
  { "LabelsSymbols", NULL, N_("_Symbols"), NULL, "show symbols", LABEL_SYMBOLS},
  { "LabelsNumbers", NULL, N_("_Numbers"), NULL, "show numbers of atoms", LABEL_NUMBERS},
  { "LabelsMMTypes", NULL, N_("_MM Types"), NULL, "show MM types of atoms", LABEL_MMTYPES},
  { "LabelsPDBTypes", NULL, N_("_PDB Types"), NULL, "show PDB types of atoms", LABEL_PDBTYPES},
  { "LabelsLayers", NULL, N_("_Layer"), NULL, "show layer of atoms", LABEL_LAYERS},
  { "LabelsSymbolsAndNumbers", NULL, N_("Symbols_&Numbers"), NULL, "show symbols and numbers of atoms", LABEL_SYMBOLS_NUMBERS},
  { "LabelsCharges", NULL, N_("_Charges"), NULL, "show charges of atoms", LABEL_CHARGES},
  { "LabelsSymbolsAndCharges", NULL, N_("Symbols_&Charges"), NULL, "show symbols and charges of atoms", LABEL_SYMBOLS_CHARGES},
  { "LabelsNumbersAndCharges", NULL, N_("Numbers_&Charges"), NULL, "show numbers and charges of atoms", LABEL_NUMBERS_CHARGES},
  { "LabelsRedidues", NULL, N_("Residues"), NULL, "show redidues", LABEL_REDIDUES},
  { "LabelsCoordinates", NULL, N_("C_oordinates"), NULL, "show coordinates of atoms", LABEL_COORDINATES},
};
static guint numberOfLabelEntries = G_N_ELEMENTS (labelEntries);
/*********************************************************************************************************************/
typedef enum 
{
	GEOMETRY_STICK,
	GEOMETRY_BALLSTICK,
	GEOMETRY_SPACEFILL,
}TypeRenderGeom;
static void render_geometry_radio_action (GtkAction *action)
{
	gint value = gtk_radio_action_get_current_value (GTK_RADIO_ACTION (action));

	switch(value)
	{
		case GEOMETRY_STICK :  RenderStick(); break;
		case GEOMETRY_BALLSTICK : RenderBallStick(); break;
		case GEOMETRY_SPACEFILL : RenderSpaceFill(); break;
	}
}
static GtkRadioActionEntry rendereGeometryEntries[] = {
  { "RenderGeometryStick", GABEDIT_STOCK_RENDER_STICK, N_("_Stick"), NULL, "render stick", GEOMETRY_STICK },
  { "RenderGeometryBallAndStick", GABEDIT_STOCK_RENDER_BALL_STICK, N_("_Ball&Stick"), NULL, "render Ball&Stick", GEOMETRY_BALLSTICK },
  { "RenderGeometrySpaceFill", GABEDIT_STOCK_RENDER_BALL_STICK, N_("_Space fill"), NULL, "render space fill", GEOMETRY_SPACEFILL },
};
static guint numberOfRenderGeometryEntries = G_N_ELEMENTS (rendereGeometryEntries);
/*********************************************************************************************************************/
static void toggle_action (GtkAction *action)
{
	const gchar *name = gtk_action_get_name (action);
	if(!strcmp(name,"LabelsDistances")) SetLabelDistances(NULL, TRUE);
	else if(!strcmp(name,"LabelsDipole")) SetLabelDipole(NULL, TRUE);
#ifdef DRAWGEOMGL
	else if(!strcmp(name,"RenderLabelsOrtho")) SetLabelsOrtho(NULL, TRUE);
#endif
	else if(!strcmp(name,"RenderPerspective")) RenderPers(NULL, TRUE);
	else if(!strcmp(name,"RenderLighting")) RenderLight(NULL, TRUE);
	else if(!strcmp(name,"RenderOrtep")) RenderOrtep(NULL, TRUE);
	else if(!strcmp(name,"RenderCartoon")) RenderCartoon(NULL, TRUE);
	else if(!strcmp(name,"RenderShad")) RenderShad(NULL, TRUE);
#ifdef DRAWGEOMGL
	else if(!strcmp(name,"RenderShowAxes")) RenderAxes(NULL, TRUE);
	else if(!strcmp(name,"RenderShowBox")) RenderBox(NULL, TRUE);
#endif
	else if(!strcmp(name,"RenderShowDipole")) RenderDipole(NULL, TRUE);
	else if(!strcmp(name,"RenderShowHydrogenBonds")) RenderHBonds(NULL, TRUE);
	else if(!strcmp(name,"RenderShowDoubleTripleBonds"))
	{
		gboolean show = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
		RenderMultipleBonds(NULL,show);
	}
	else if(!strcmp(name,"ShowToolBar")) view_toolbar();
	else if(!strcmp(name,"ShowStatusBox"))
	{
		GtkWidget* box = g_object_get_data(G_OBJECT(GeomDlg), "StatusBox");
		gboolean show = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
		gtk_widget_hide(box);
		if(show) gtk_widget_show(box);
	}
	else if(!strcmp(name,"ShowMeasureNoteBook"))
	{
		gboolean show = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
		HideShowMeasure(!show);
	}
	else if(!strcmp(name,"AdjustHydrogens"))
	{
		gboolean show = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
		AdjustHydrogensYesNo(show);
	}
	else if(!strcmp(name,"RebuildConnectionsDuringEdition"))
	{
		gboolean rebuild = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
		RebuildConnectionsDuringEditionYesNo(rebuild);
	}
#ifdef DRAWGEOMGL
	else if(!strcmp(name,"RenderLightOnOff1"))
	{
		set_light_geom_on_off(0);
		rafresh_drawing();
	}
	else if(!strcmp(name,"RenderLightOnOff2"))
	{
		set_light_geom_on_off(1);
		rafresh_drawing();
	}
	else if(!strcmp(name,"RenderLightOnOff3"))
	{
		set_light_geom_on_off(2);
		rafresh_drawing();
	}
#endif
}
/*--------------------------------------------------------------------------------------------------------------------*/
static GtkToggleActionEntry gtkActionToggleEntries[] =
{
	{ "LabelsDistances", NULL, N_("_Distances"), NULL, "show distances", G_CALLBACK (toggle_action), FALSE },
	{ "LabelsDipole", NULL, N_("_Dipole"), NULL, "show dipole", G_CALLBACK (toggle_action), FALSE },
#ifdef DRAWGEOMGL
	{ "RenderLabelsOrtho", NULL, N_("_Orthographic labels"), NULL, "Orthographic labels", G_CALLBACK (toggle_action), FALSE },
#endif

	{ "RenderPerspective", NULL, N_("_Perspective"), NULL, "render perspective", G_CALLBACK (toggle_action), FALSE },
	{ "RenderLighting", NULL, N_("_Lighting"), NULL, "render lighting", G_CALLBACK (toggle_action), FALSE },
	{ "RenderOrtep", NULL, N_("_Ortep"), NULL, "render ortep", G_CALLBACK (toggle_action), FALSE },
	{ "RenderCartoon", NULL, N_("_Cartoon"), NULL, "render cartoon", G_CALLBACK (toggle_action), FALSE },
	{ "RenderShad", NULL, N_("_Shad"), NULL, "render shad", G_CALLBACK (toggle_action), FALSE },
#ifdef DRAWGEOMGL
	{ "RenderShowAxes", NULL, N_("Show _Axes"), NULL, "Show axes", G_CALLBACK (toggle_action), FALSE },
	{ "RenderShowBox", NULL, N_("Show _Box"), NULL, "Show box", G_CALLBACK (toggle_action), FALSE },
#endif
	{ "RenderShowDipole", NULL, N_("Show _Dipole"), NULL, "Show dipole", G_CALLBACK (toggle_action), FALSE },
	{ "RenderShowHydrogenBonds", NULL, N_("Show _Hydrogen bonds"), NULL, "Show hydrogen bonds", G_CALLBACK (toggle_action), FALSE },
	{ "RenderShowDoubleTripleBonds", NULL, N_("Show _double & triple bonds"), NULL, "Show double&triple bonds", G_CALLBACK (toggle_action), TRUE },

	{ "ShowToolBar", NULL, N_("_Show toolbar"), NULL, "show toolbar", G_CALLBACK (toggle_action), TRUE },
	{ "ShowStatusBox", NULL, N_("_show status handlebox"), NULL, "show status handlebox", G_CALLBACK (toggle_action), TRUE},

	{ "ShowMeasureNoteBook", GABEDIT_STOCK_HIDE, N_("Show the measure notebook"), NULL, "show the measure notebook", G_CALLBACK (toggle_action), FALSE},
	{ "AdjustHydrogens", GABEDIT_STOCK_ADJUST_H, N_("Adjust _hydrogens"), NULL, "Adjus hydrogens", G_CALLBACK (toggle_action), FALSE},
	{ "RebuildConnectionsDuringEdition", GTK_STOCK_DISCONNECT, N_("Rebuild _connections during a move"), NULL, "Rebuild connections during a mov", G_CALLBACK (toggle_action), FALSE},
	{ "RenderLightOnOff1", NULL, N_("OnOff _1"), NULL, "On/Of the light number 1", G_CALLBACK (toggle_action), TRUE },
	{ "RenderLightOnOff2", NULL, N_("OnOff _2"), NULL, "On/Of the light number 2", G_CALLBACK (toggle_action), FALSE },
	{ "RenderLightOnOff3", NULL, N_("OnOff _3"), NULL, "On/Of the light number 3", G_CALLBACK (toggle_action), FALSE },

};

static guint numberOfGtkActionToggleEntries = G_N_ELEMENTS (gtkActionToggleEntries);
/*********************************************************************************************************************/
static void activate_action (GtkAction *action)
{
	const gchar *name = gtk_action_get_name (action);
	if(!strcmp(name,"ReadAuto")) read_geom_any_file_dlg();
	if(!strcmp(name,"ReadXYZ")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_XYZ); }
	else if(!strcmp(name,"ReadPOSCAR")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_VASPPOSCAR); }
	else if(!strcmp(name,"ReadCIF")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_CIF); }
	else if(!strcmp(name,"ReadCIFNoSym")) { MethodeGeom = GEOM_IS_XYZ;selc_cif_file_nosym(); }
	else if(!strcmp(name,"ReadMol2")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_MOL2); }
	else if(!strcmp(name,"ReadTinker")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_TINKER); }
	else if(!strcmp(name,"ReadPDB")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_PDB); }
	else if(!strcmp(name,"ReadHyperchem")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_HIN);}
	else if(!strcmp(name,"ReadAIMAll")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_AIMALL);}
	else if(!strcmp(name,"ReadMol")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_MOL);}
	else if(!strcmp(name,"ReadGabedit")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_GABEDIT);}
	else if(!strcmp(name,"ReadGaussianZMat")) { MethodeGeom = GEOM_IS_ZMAT;selc_ZMatrix_file(); }
	else if(!strcmp(name,"ReadMopacZMat")) { MethodeGeom = GEOM_IS_ZMAT;selc_ZMatrix_mopac_file(); }
	else if(!strcmp(name,"ReadDaltonFirst")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_DALTONFIRST); }
	else if(!strcmp(name,"ReadDaltonLast")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_DALTONLAST); }
	else if(!strcmp(name,"ReadGamessFirst")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_GAMESSFIRST); }
	else if(!strcmp(name,"ReadGamessLast")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_GAMESSLAST); }
	else if(!strcmp(name,"ReadTurbomoleFirst")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_TURBOMOLEFIRST); }
	else if(!strcmp(name,"ReadTurbomoleLast")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_TURBOMOLELAST); }
	else if(!strcmp(name,"ReadGaussianInput")) { selc_all_input_file(_("Read Geometry from a Gaussian input file")); }
	else if(!strcmp(name,"ReadGaussianFirst")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_GAUSSOUTFIRST); }
	else if(!strcmp(name,"ReadGaussianLast")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_GAUSSOUTLAST);}
	else if(!strcmp(name,"ReadGaussianFChk")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_GAUSSIAN_FCHK);}
	else if(!strcmp(name,"ReadMolcasInput")) { selc_all_input_file(_("Read Geometry from a Molcas input file"));}
	else if(!strcmp(name,"ReadMolcasFirst")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_MOLCASOUTFIRST);}
	else if(!strcmp(name,"ReadMolcasLast")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_MOLCASOUTLAST);}
	else if(!strcmp(name,"ReadMolproInput")) { selc_all_input_file(_("Read Geometry from a Molpro input file"));}
	else if(!strcmp(name,"ReadMolproFirst")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_MOLPROOUTFIRST);}
	else if(!strcmp(name,"ReadMolproLast")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_MOLPROOUTLAST);}
	else if(!strcmp(name,"ReadMopacInput")) { selc_all_input_file(_("Read Geometry from a Mopac input file"));}
	else if(!strcmp(name,"ReadMopacFirst")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_MOPACOUTFIRST);}
	else if(!strcmp(name,"ReadMopacLast")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_MOPACOUTLAST);}
	else if(!strcmp(name,"ReadMopacAux")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_MOPACAUX);}
	else if(!strcmp(name,"ReadWFX")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_WFX);}
	else if(!strcmp(name,"ReadMopacScan")) 
 	  	file_chooser_open(read_geometries_conv_mopac_scan,_("Read Geomtries From Mopac Scan Output file"), GABEDIT_TYPEFILE_MOPAC,GABEDIT_TYPEWIN_GEOM);
	else if(!strcmp(name,"ReadMopacIRC")) 
 	  	file_chooser_open(read_geometries_conv_mopac_irc,_("Read Geomtries From Mopac IRC Output file"), GABEDIT_TYPEFILE_MOPAC,GABEDIT_TYPEWIN_GEOM);
	else if(!strcmp(name,"ReadMPQCInput")) { selc_all_input_file(_("Read Geometry from a MPQC input file"));}
	else if(!strcmp(name,"ReadMPQCFirst")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_MPQCOUTFIRST);}
	else if(!strcmp(name,"ReadMPQCLast")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_MPQCOUTLAST);}
	else if(!strcmp(name,"ReadFireFlyFirst")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_GAMESSFIRST); }
	else if(!strcmp(name,"ReadFireFlyLast")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_GAMESSLAST); }
	else if(!strcmp(name,"ReadOrcaFirst")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_ORCAOUTFIRST);}
	else if(!strcmp(name,"ReadOrcaLast")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_ORCAOUTLAST);}
	else if(!strcmp(name,"ReadVaspFirst")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_VASPOUTFIRST);}
	else if(!strcmp(name,"ReadVaspLast")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_VASPOUTLAST);}
	else if(!strcmp(name,"ReadVaspXMLFirst")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_VASPXMLFIRST);}
	else if(!strcmp(name,"ReadVaspXMLLast")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_VASPXMLLAST);}
	else if(!strcmp(name,"ReadNWChemFirst")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_NWCHEMOUTFIRST);}
	else if(!strcmp(name,"ReadNWChemLast")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_NWCHEMOUTLAST);}

	else if(!strcmp(name,"ReadPsicodeFirst")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_PSICODEOUTFIRST);}
	else if(!strcmp(name,"ReadPsicodeLast")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_PSICODEOUTLAST);}
	else if(!strcmp(name,"ReadQChemFirst")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_QCHEMOUTFIRST);}
	else if(!strcmp(name,"ReadQChemLast")) { MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_QCHEMOUTLAST);}
	else if(!strcmp(name,"ReadUsingOpenBabel")) { create_babel_read_dialogue(); }
	else if(!strcmp(name,"ReadGeomConvDalton"))
 	  file_chooser_open(read_geometries_conv_dalton,_("Load Geom. Conv. From Dalton Output file"), GABEDIT_TYPEFILE_DALTON,GABEDIT_TYPEWIN_GEOM);
	else if(!strcmp(name,"ReadGeomConvGamess"))
 	  file_chooser_open(read_geometries_conv_gamess,_("Load Geom. Conv. From Gamess Output file"), GABEDIT_TYPEFILE_GAMESS,GABEDIT_TYPEWIN_GEOM);
	else if(!strcmp(name,"ReadGeomConvFireFly"))
 	  file_chooser_open(read_geometries_conv_gamess,_("Load Geom. Conv. From FireFly Output file"), GABEDIT_TYPEFILE_FIREFLY,GABEDIT_TYPEWIN_GEOM);
	else if(!strcmp(name,"ReadGeomIRCGamess"))
 	  file_chooser_open(read_geometries_irc_gamess,_("Load Geom. From Gamess IRC file"), GABEDIT_TYPEFILE_GAMESSIRC,GABEDIT_TYPEWIN_GEOM);
	else if(!strcmp(name,"ReadGeomIRCFireFly"))
 	  file_chooser_open(read_geometries_irc_gamess,_("Load Geom. From FireFly IRC file"), GABEDIT_TYPEFILE_GAMESSIRC,GABEDIT_TYPEWIN_GEOM);
	else if(!strcmp(name,"ReadGeomConvGaussian"))
 	  file_chooser_open(read_geometries_conv_gaussian,_("Load Geom. Conv. From Gaussian Output file"), GABEDIT_TYPEFILE_GAUSSIAN,GABEDIT_TYPEWIN_GEOM);
	else if(!strcmp(name,"ReadGeomConvMolpro"))
   	  file_chooser_open(read_geometries_conv_molpro,_("Load Geom. Conv. From Molpro log file"), GABEDIT_TYPEFILE_MOLPRO_LOG,GABEDIT_TYPEWIN_GEOM);
	else if(!strcmp(name,"ReadGeomConvMopac"))
   	  file_chooser_open(read_geometries_conv_mopac,_("Load Geom. Conv. From Molpac aux file"), GABEDIT_TYPEFILE_MOPAC_AUX,GABEDIT_TYPEWIN_GEOM);
	else if(!strcmp(name,"ReadGeomConvMPQC"))
   	  file_chooser_open(read_geometries_conv_mpqc,_("Load Geom. Conv. From MPQC output file"), GABEDIT_TYPEFILE_MPQC,GABEDIT_TYPEWIN_GEOM);
	else if(!strcmp(name,"ReadGeomConvOrca"))
   	  file_chooser_open(read_geometries_conv_orca,_("Load Geom. Conv. From ORCA output file"), GABEDIT_TYPEFILE_ORCA,GABEDIT_TYPEWIN_GEOM);
	else if(!strcmp(name,"ReadGeomConvVasp"))
   	  file_chooser_open(read_geometries_conv_vasp_outcar,_("Load Geom. Conv. From VASP output file"), GABEDIT_TYPEFILE_VASPOUTCAR,GABEDIT_TYPEWIN_GEOM);
	else if(!strcmp(name,"ReadGeomConvVaspXML"))
   	  file_chooser_open(read_geometries_conv_vasp_xml,_("Load Geom. Conv. From VASP xml file"), GABEDIT_TYPEFILE_VASPXML,GABEDIT_TYPEWIN_GEOM);

	else if(!strcmp(name,"ReadGeomConvNWChem"))
   	  file_chooser_open(read_geometries_conv_nwchem,_("Load Geom. Conv. From NWChem output file"), GABEDIT_TYPEFILE_NWCHEM,GABEDIT_TYPEWIN_GEOM);

	else if(!strcmp(name,"ReadGeomConvPsicode"))
   	  file_chooser_open(read_geometries_conv_psicode,_("Load Geom. Conv. From Psicode output file"), GABEDIT_TYPEFILE_PSICODE,GABEDIT_TYPEWIN_GEOM);

	else if(!strcmp(name,"ReadGeomConvQChem"))
   	  file_chooser_open(read_geometries_conv_qchem,_("Load Geom. Conv. From Q-Chem output file"), GABEDIT_TYPEFILE_QCHEM,GABEDIT_TYPEWIN_GEOM);

	else if(!strcmp(name,"ReadGeomConvGabedit"))
   	  file_chooser_open(read_geometries_conv_gabedit,_("Load Geom. Conv. From Gabedit file"), GABEDIT_TYPEFILE_GABEDIT,GABEDIT_TYPEWIN_GEOM);

	else if(!strcmp(name,"ReadGeomConvMolden"))
   	  file_chooser_open(read_geometries_conv_molden,_("Load Geom. Conv. From Molden file"), GABEDIT_TYPEFILE_MOLDEN,GABEDIT_TYPEWIN_GEOM);
	else if(!strcmp(name,"ReadGeomConvXYZ"))
  	  file_chooser_open(read_geometries_conv_xyz,_("Load Geom. Conv. From XYZ"), GABEDIT_TYPEFILE_XYZ,GABEDIT_TYPEWIN_GEOM);
	else if(!strcmp(name,"EditDeleteMolecule")) DeleteMolecule();
	else if(!strcmp(name,"EditDeleteHydrogenAtoms")) deleteHydrogenAtoms();
	else if(!strcmp(name,"EditDeleteSelectedAtoms")) deleteSelectedAtoms();
	else if(!strcmp(name,"EditMoveCenterOfSelectedAtomsToOrigin")) moveCenterOfSelectedAtomsToOrigin();
	else if(!strcmp(name,"EditAlignPrincipalAxesOfSelectedAtomsToXYZ")) alignPrincipalAxesOfSelectedAtomsToXYZ();
	else if(!strcmp(name,"EditAlignSelectedAndNotSelectedAtoms")) alignSelectedAndNotSelectedAtoms();
	else if(!strcmp(name,"EditCopySelectedAtoms")) copySelectedAtoms();
	else if(!strcmp(name,"EditResetAllConnections")) resetConnections();
	else if(!strcmp(name,"EditResetConnectionsBetweenSelectedAndNotSelectedAtoms")) resetConnectionsBetweenSelectedAndNotSelectedAtoms();
	else if(!strcmp(name,"EditResetConnectionsBetweenSelectedAtoms")) resetConnectionsBetweenSelectedAtoms();

	else if(!strcmp(name,"EditResetMultipleConnections")) resetMultipleConnections();
	else if(!strcmp(name,"EditSelectAll"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		SelectAllAtoms();
	}
	else if(!strcmp(name,"EditInvertSelection"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		InvertSelectionOfAtoms();
	}
	else if(!strcmp(name,"EditUnSelectAll"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		unSelectAllAtoms();
	}
	else if(!strcmp(name,"EditSelectHighAtoms"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		SelectLayerAtoms(HIGH_LAYER);
	}
	else if(!strcmp(name,"EditSelectMediumAtoms"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		SelectLayerAtoms(MEDIUM_LAYER);
	}
	else if(!strcmp(name,"EditSelectLowAtoms"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		SelectLayerAtoms(LOW_LAYER);
	}
	else if(!strcmp(name,"EditSelectFixedAtoms"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		SelectFixedVariableAtoms(FALSE);
	}
	else if(!strcmp(name,"EditSelectVariableAtoms"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		SelectFixedVariableAtoms(TRUE);
	}
	else if(!strcmp(name,"EditSelectFirstResidue"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		SelectFirstResidue();
	}
	else if(!strcmp(name,"EditSelectLastResidue"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		SelectLastResidue();
	}
	else if(!strcmp(name,"EditSelectResidueByNumber"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		selectResidueByNumberDlg();
	}
	else if(!strcmp(name,"EditSelectResidueByName"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		selectResidueByNameDlg();
	}
	else if(!strcmp(name,"EditSelectAtomsByMMType"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		selectAtomsByMMTypeDlg();
	}
	else if(!strcmp(name,"EditSelectAtomsByPDBType"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		selectAtomsByPDBTypeDlg();
	}
	else if(!strcmp(name,"EditSelectAtomsBySymbol"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		selectAtomsBySymbolDlg();
	}
	else if(!strcmp(name,"EditSelectAtomsBySphere"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		selectAtomsBySphereDlg();
	}
	else if(!strcmp(name,"EditSelectAtomsByPositiveCharges"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		selectAtomsByChargeValues(TRUE);
	}
	else if(!strcmp(name,"EditSelectAtomsByNegativeCharges"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		selectAtomsByChargeValues(FALSE);
	}
	else if(!strcmp(name,"EditSelectAtomsMultiple"))
	{
		GtkAction *selectionAtoms = gtk_ui_manager_get_action (manager, "/MenuGeom/Operations/OperationsSelectionOfAtoms");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(selectionAtoms), TRUE);
		selectionDlg();
	}
	else if(!strcmp(name,"EditOpenGeometryEditor")) 
	{
		if(MethodeGeom == GEOM_IS_XYZ) create_GeomXYZ_from_draw_grometry();
		edit_geometry();
		unSelectAllAtoms();
	}
	else if(!strcmp(name,"SaveAsXYZ"))
	{
		create_GeomXYZ_from_draw_grometry();
 	  	file_chooser_save(save_geometry_xyz_file,_("Save geometry in xyz file"), GABEDIT_TYPEFILE_XYZ,GABEDIT_TYPEWIN_GEOM);
	}
	else if(!strcmp(name,"SaveAsPOSCARCartn"))
	{
		create_GeomXYZ_from_draw_grometry();
 	  	file_chooser_save(save_geometry_poscar_file,_("Save geometry in POSCAR file (for VASP)"), GABEDIT_TYPEFILE_VASPPOSCAR,GABEDIT_TYPEWIN_GEOM);
	}
	else if(!strcmp(name,"SaveAsPOSCARDirect"))
	{
		create_GeomXYZ_from_draw_grometry();
 	  	file_chooser_save(save_geometry_poscar_direct_file,_("Save geometry in POSCAR file (for VASP)"), GABEDIT_TYPEFILE_VASPPOSCAR,GABEDIT_TYPEWIN_GEOM);
	}
	else if(!strcmp(name,"SaveAsCIFAllAtoms"))
	{
		create_GeomXYZ_from_draw_grometry();
 	  	file_chooser_save(save_geometry_cif_allatoms_file,_("Save geometry in CIF file (All atoms)"), GABEDIT_TYPEFILE_CIF,GABEDIT_TYPEWIN_GEOM);
	}
	else if(!strcmp(name,"SaveAsCIF"))
	{
		create_GeomXYZ_from_draw_grometry();
 	  	file_chooser_save(save_geometry_cif_file,_("Save geometry in CIF file (With sym operators)"), GABEDIT_TYPEFILE_CIF,GABEDIT_TYPEWIN_GEOM);
	}
	else if(!strcmp(name,"SaveAsGabedit"))
	{
		create_GeomXYZ_from_draw_grometry();
 	  	file_chooser_save(save_geometry_gabedit_file,_("Save geometry in Gabedit file"), GABEDIT_TYPEFILE_GABEDIT,GABEDIT_TYPEWIN_GEOM);
	}
	else if(!strcmp(name,"SaveAsCChemI"))
	{
		create_GeomXYZ_from_draw_grometry();
 	  	file_chooser_save(save_geometry_cchemi_file,_("Save geometry in CChemI file"), GABEDIT_TYPEFILE_CCHEMI,GABEDIT_TYPEWIN_GEOM);
	}
	else if(!strcmp(name,"SaveAsMol2"))
	{
		create_GeomXYZ_from_draw_grometry();
 		file_chooser_save(save_geometry_mol2_file,_("Save geometry in mol2 file"), GABEDIT_TYPEFILE_MOL2,GABEDIT_TYPEWIN_GEOM);
	}
	else if(!strcmp(name,"SaveAsMol"))
	{
		create_GeomXYZ_from_draw_grometry();
 		file_chooser_save(save_geometry_mol_file,_("Save geometry in mol file"), GABEDIT_TYPEFILE_MOL,GABEDIT_TYPEWIN_GEOM);
	}
	else if(!strcmp(name,"SaveAsTinker"))
	{
		create_GeomXYZ_from_draw_grometry();
 		file_chooser_save(save_geometry_tinker_file,_("Save geometry in tinker file"), GABEDIT_TYPEFILE_TINKER,GABEDIT_TYPEWIN_GEOM);
	}
	else if(!strcmp(name,"SaveAsPDB"))
	{
		create_GeomXYZ_from_draw_grometry();
 		file_chooser_save(save_geometry_pdb_file,_("Save geometry in pdb file"), GABEDIT_TYPEFILE_PDB,GABEDIT_TYPEWIN_GEOM);
	}
	else if(!strcmp(name,"SaveAsHyperchem"))
	{
		create_GeomXYZ_from_draw_grometry();
 		file_chooser_save(save_geometry_hin_file,_("Save geometry in hyperchem file"), GABEDIT_TYPEFILE_HIN,GABEDIT_TYPEWIN_GEOM);
	}
	else if(!strcmp(name,"SaveAsMopacZMat"))
	{
		create_GeomXYZ_from_draw_grometry(); 
		MethodeGeom = GEOM_IS_XYZ;
		if(!xyz_to_zmat())
		{
			Message(_("Sorry\nConversion is not possible from XYZ to Zmat"),"Error",TRUE);
			return;
		}
 		file_chooser_save(save_geometry_mzmatrix_file,_("Save geometry in mopac z-matrix file"), GABEDIT_TYPEFILE_MZMAT,GABEDIT_TYPEWIN_GEOM);
		create_GeomXYZ_from_draw_grometry(); 
		MethodeGeom = GEOM_IS_XYZ;
	}
	else if(!strcmp(name,"SaveAsGaussianZMat"))
	{
		create_GeomXYZ_from_draw_grometry(); 
		MethodeGeom = GEOM_IS_XYZ;
		if(!xyz_to_zmat())
		{
			Message(_("Sorry\nConversion is not possible from XYZ to Zmat"),_("Error"),TRUE);
			return;
		}
 		file_chooser_save(save_geometry_gzmatrix_file,_("Save geometry in gaussian z-matrix file"), GABEDIT_TYPEFILE_GZMAT,GABEDIT_TYPEWIN_GEOM);
		create_GeomXYZ_from_draw_grometry(); 
		MethodeGeom = GEOM_IS_XYZ;
	}
	else if(!strcmp(name,"SaveUsingOpenBabel")) { create_babel_save_dialogue(); }
	else if(!strcmp(name,"BuildLinearMolecule")) build_linear_molecule_dlg();
	else if(!strcmp(name,"BuildNanoTube")) build_nanotube_dlg();
	else if(!strcmp(name,"BuildRingMolecule")) build_ring_molecule_dlg();
	else if(!strcmp(name,"BuildMoleculeWithSymmetry")) build_rozphi_molecule_dlg();
	else if(!strcmp(name,"BuildPolyPeptide")) build_polypeptide_dlg();
#ifdef DRAWGEOMGL
	else if(!strcmp(name,"BuildCrystalGen")) build_crystal_dlg();
	else if(!strcmp(name,"BuildSuperCellSimple")) build_supercell_simple_dlg();
	else if(!strcmp(name,"BuildSuperCell")) build_supercell_dlg();
	else if(!strcmp(name,"BuildWulff")) build_wulff_dlg();
	else if(!strcmp(name,"BuildSlab")) build_slab_dlg();
	else if(!strcmp(name,"WrapAtomsToCell")) wrap_atoms_to_cell();
	else if(!strcmp(name,"ComputeSpaceGroupSym")) compute_space_symmetry_group();
	else if(!strcmp(name,"ComputeSymmetryInfo")) compute_symmetry_info();
	else if(!strcmp(name,"ComputeKPointsPath")) compute_kpoints_path();
	else if(!strcmp(name,"ReductionNiggli")) reduce_cell_niggli();
	else if(!strcmp(name,"ReductionDelaunay")) reduce_cell_delaunay();
	else if(!strcmp(name,"ReductionPrimitive")) reduce_cell_primitive();
	else if(!strcmp(name,"StandardizeCellPrimitive")) standardize_cell_primitive();
	else if(!strcmp(name,"StandardizeCellConv")) standardize_cell_conventional();
	else if(!strcmp(name,"HelpCrystal")) help_references_crystallography();
	else if(!strcmp(name,"PrototypeCrystal")) activate_insert_crystal();
	else if(!strcmp(name,"SetSymPrec")) setSymPrecDlg();
	else if(!strcmp(name,"ComputeVolume")) compute_volume_cell();
#endif
	else if(!strcmp(name,"BuildPolyNucleicAcid")) build_polynucleicacid_dlg();
	else if(!strcmp(name,"BuildPolySaccharide")) build_polysaccharide_dlg();
	else if(!strcmp(name,"PersonalFragmentsNewGroup")) newGroupeDlg(NULL, 0, NULL);
	else if(!strcmp(name,"PersonalFragmentsDeleteGroup")) deleteGroupeDlg(NULL, 0, NULL);
	else if(!strcmp(name,"PersonalFragmentsAddMolecule"))  addFragmentDlg(NULL, 0, NULL);
	else if(!strcmp(name,"PersonalFragmentsRemoveFragment")) deleteFragmentDlg(NULL, 0, NULL);
	else if(!strcmp(name, "RenderDefaultStick")) factor_stick_default(NULL, NULL);
	else if(!strcmp(name, "RenderDefaultBall"))factor_ball_default(NULL, NULL);
	else if(!strcmp(name, "RenderDefaultZoom")) factor_default(NULL, NULL);
	else if(!strcmp(name, "RenderDefaultDipole")) factor_dipole_default(NULL, NULL);
	else if(!strcmp(name, "RenderDefaultCenter")) SetOperation(NULL, CENTER);
	else if(!strcmp(name, "RenderDefaultAll")) factor_all_default(NULL, NULL);
	else if(!strcmp(name, "RenderBackgroundColorBlack")) set_back_color_black();
	else if(!strcmp(name, "RenderBackgroundColorOther")) open_color_dlg( NULL, NULL);
	else if(!strcmp(name,"RenderHideHydrogenAtoms")) RenderHAtoms(NULL, FALSE);
	else if(!strcmp(name, "RenderHideSelectedAtoms")) hide_selected_atoms();
	else if(!strcmp(name, "RenderHideNotSelectedAtoms")) hide_not_selected_atoms();
	else if(!strcmp(name, "RenderShowAllAtoms")) show_all_atoms();
	else if(!strcmp(name, "RenderShowHydrogenAtoms")) show_hydrogen_atoms();
	else if(!strcmp(name, "RenderOptimal")) set_optimal_geom_view();

	else if(!strcmp(name, "SymmetryRotationalConstantes")) create_symmetry_window( NULL, 0);
	else if(!strcmp(name, "SymmetryGroupSymmetry")) get_standard_orientation_with_reduction(NULL, 0);
	else if(!strcmp(name, "SymmetryGroupSymmetrize")) get_standard_orientation_with_symmetrization(NULL, 0);
	else if(!strcmp(name, "SymmetryAbelianGroup")) get_abelian_orientation_with_reduction(NULL, 0);
	else if(!strcmp(name, "SymmetrySetTolerance")) create_tolerance_window (NULL, 0);
	else if(!strcmp(name, "SetOriginToCenterOfMolecule")) SetOriginAtCenter(NULL, 0, NULL);
	else if(!strcmp(name, "SetOriginToCenterOfSelectedAtoms")) set_origin_to_center_of_fragment();
	else if(!strcmp(name, "SetXYZToPAX")) set_xyz_to_principal_axes_of_selected_atoms(NULL,0, NULL);
	else if(!strcmp(name, "SetXYZToPAZ")) set_xyz_to_principal_axes_of_selected_atoms(NULL,1, NULL);
	else if(!strcmp(name, "SetXYZToStandardOrientaion")) set_xyz_to_standard_orientation_all();
	else if(!strcmp(name, "SetXYZToStandardOrientaionSelectedAndNotSelected")) set_xyz_to_standard_orientation_selected_and_not_selected_atoms();
	else if(!strcmp(name, "SetSelectedAtomsToHighLayer")) set_layer_of_selected_atoms(HIGH_LAYER);
	else if(!strcmp(name, "SetSelectedAtomsToMediumLayer")) set_layer_of_selected_atoms(MEDIUM_LAYER);
	else if(!strcmp(name, "SetSelectedAtomsToLowLayer")) set_layer_of_selected_atoms(LOW_LAYER);
	else if(!strcmp(name, "SetSelectedAtomsToFixed")) set_fix_selected_atoms();
	else if(!strcmp(name, "SetSelectedAtomsToVariable")) set_variable_selected_atoms();
	else if(!strcmp(name, "SetMMTypeOfselectedAtoms")) setMMTypeOfselectedAtomsDlg();
	else if(!strcmp(name, "SetPDBTypeOfselectedAtoms")) setPDBTypeOfselectedAtomsDlg();
	else if(!strcmp(name, "SetResidueNameOfselectedAtoms")) setResidueNameOfselectedAtomsDlg();
	else if(!strcmp(name, "SetSymbolOfselectedAtoms")) setSymbolOfselectedAtomsDlg();
	else if(!strcmp(name, "SetChargeOfselectedAtoms")) setChargeOfselectedAtomsDlg();
	else if(!strcmp(name, "scaleChargesOfSelectedAtoms")) scaleChargesOfSelectedAtomsDlg();
	else if(!strcmp(name, "InsertAFragment")) activate_insert_fragment();
	else if(!strcmp(name, "AddMaxHydrogens")) addMaxHydrogens();
	else if(!strcmp(name, "AddHydrogens")) addHydrogens();
	else if(!strcmp(name, "AddHydrogensTpl")) addHydrogensTpl();
	else if(!strcmp(name, "AddOneHydrogen")) addOneHydrogen();
	else if(!strcmp(name, "SetDipole")) set_dipole_dialog();
	else if(!strcmp(name, "ScaleCell")) scale_cell_dialog();
	else if(!strcmp(name, "SetDipoleFormCharges")) set_dipole_from_charges();
	else if(!strcmp(name, "ComputeDipoleFormCharges")) compute_dipole_from_charges();
	else if(!strcmp(name, "ComputeIsotopeDistribution")) createIstopeDistributionCalculationFromDrawGeom();
	else if(!strcmp(name, "ComputeTotalCharge")) compute_total_charge();
	else if(!strcmp(name, "ComputeVolumeMolecule")) compute_volume_molecule();
	else if(!strcmp(name, "ComputeChargeForResidues")) compute_charge_by_residue();
	else if(!strcmp(name, "ComputeChargeOfSelectedsAtoms")) compute_charge_of_selected_atoms();
	else if(!strcmp(name, "SetHydrogenBonds"))set_HBonds_dialog_geom(NULL, 0);
	else if(!strcmp(name, "SetPropertiesOfAtoms")) create_table_prop();
	else if(!strcmp(name, "SetMolecularMechanicsParameters")) setMMParamatersDlg();
	else if(!strcmp(name, "SetPDBTemplate")) setPDBTemplateDlg();
	else if(!strcmp(name, "SetChargesUsingPDBTemplate")) setMMTypesCharges(NULL, 0, NULL);
	else if(!strcmp(name, "SetAtomTypesUsingPDBTemplate")) setMMTypesCharges(NULL, 1, NULL);
	else if(!strcmp(name, "SetAtomTypeAndChargeUsingPDBTemplate")) setMMTypesCharges(NULL, 2, NULL);
	else if(!strcmp(name, "SetAtomTypeCalcul")) setMMTypesCharges(NULL, 4, NULL);
	else if(!strcmp(name, "SetChargesToZero")) setMMTypesCharges(NULL, 3, NULL);

#ifdef DRAWGEOMGL
	else if(!strcmp(name, "SetCamera")) set_camera_drawgeom();
	else if(!strcmp(name, "SetLightPositions")) set_light_positions_drawgeom("Light positions");
	else if(!strcmp(name, "SetXYZAxesProperties")) set_axes_geom_dialog();
#endif

	else if(!strcmp(name, "SetPovrayBackground")) set_povray_options_geom(NULL,0);
	else if(!strcmp(name, "SetAtomToInsert")) select_atom();
	else if(!strcmp(name, "ExportPostscript")) export_geometry_dlg("ps");
	else if(!strcmp(name, "ExportEPS")) export_geometry_dlg("eps");
	else if(!strcmp(name, "ExportPovray")) exportPOVGeomDlg(GeomDlg);
	else if(!strcmp(name, "ExportPDF")) export_geometry_dlg("pdf");
	else if(!strcmp(name, "ExportSVG")) export_geometry_dlg("svg");
	else if(!strcmp(name, "ScreenCaptureJPG")) 
	{
 		GtkWidget* chooser = file_chooser_save(save_geometry_jpeg_file,_("Save image in jpeg file format"),GABEDIT_TYPEFILE_JPEG,GABEDIT_TYPEWIN_GEOM);
		/* fit_windows_position(GeomDlg, chooser);*/
	}
	else if(!strcmp(name, "ScreenCapturePPM"))
	{
 		GtkWidget* chooser = file_chooser_save(save_geometry_ppm_file,_("Save image in ppm file format"),GABEDIT_TYPEFILE_PPM,GABEDIT_TYPEWIN_GEOM);
		/* fit_windows_position(GeomDlg, chooser);*/
	}
	else if(!strcmp(name, "ScreenCaptureBMP"))
	{
 		GtkWidget* chooser = file_chooser_save(save_geometry_bmp_file,_("Save image in bmp file format"),GABEDIT_TYPEFILE_BMP,GABEDIT_TYPEWIN_GEOM);
		/* fit_windows_position(GeomDlg, chooser);*/
	}
	else if(!strcmp(name, "ScreenCapturePNG"))
	{
 		GtkWidget* chooser = file_chooser_save(save_geometry_png_file,_("Save image in png file format"),GABEDIT_TYPEFILE_PNG,GABEDIT_TYPEWIN_GEOM);
		/* fit_windows_position(GeomDlg, chooser);*/
	}
	else if(!strcmp(name, "ScreenCaptureTIF"))
	{
 		GtkWidget* chooser = file_chooser_save(save_geometry_tiff_file,_("Save image in tif file format"),GABEDIT_TYPEFILE_TIF,GABEDIT_TYPEWIN_GEOM);
		/* fit_windows_position(GeomDlg, chooser);*/
	}
	else if(!strcmp(name, "ScreenCapturePS"))
	{
 		GtkWidget* chooser = file_chooser_save(save_geometry_ps_file,_("Save image in ps file format"),GABEDIT_TYPEFILE_PS,GABEDIT_TYPEWIN_GEOM);
		/* fit_windows_position(GeomDlg, chooser);*/
	}
	else if(!strcmp(name, "ScreenCaptureCilpBoard")) 
	{
		copy_screen_geom_clipboard();
	}

	else if(!strcmp(name, "MolecularMechanicsEnergy")) 
	{
		MolecularMechanicsEnergyDlg();
		messageAmberTypesDefine();
	}
	else if(!strcmp(name, "MolecularMechanicsOptimization"))
	{
		MolecularMechanicsMinimizeDlg();
		messageAmberTypesDefine();
	}
	else if(!strcmp(name, "MolecularMechanicsDynamics"))
	{
		MolecularMechanicsDynamicsDlg();
		messageAmberTypesDefine();
	}
	else if(!strcmp(name, "MolecularMechanicsDynamicsConfo"))
	{
		MolecularMechanicsDynamicsConfoDlg();
		messageAmberTypesDefine();
	}
	else if(!strcmp(name, "SemiEmpiricalMD"))
	{
		semiEmpiricalMolecularDynamicsDlg();
	}
	else if(!strcmp(name, "SemiEmpiricalMDConfo"))
	{
		semiEmpiricalMolecularDynamicsConfoDlg();
	}
	else if(!strcmp(name, "SemiEmpiricalEnergyFireFlyAM1"))
	{
		semiEmpiricalDlg("AM1FireFlyEnergy");
	}
	else if(!strcmp(name, "SemiEmpiricalEnergyMopacPM6DH2"))
	{
		semiEmpiricalDlg("PM6DH2MopacEnergy");
	}
	else if(!strcmp(name, "SemiEmpiricalEnergyMopacPM6DH+"))
	{
		semiEmpiricalDlg("PM6DH+MopacEnergy");
	}
	else if(!strcmp(name, "SemiEmpiricalEnergyMopacPM6"))
	{
		semiEmpiricalDlg("PM6MopacEnergy");
	}
	else if(!strcmp(name, "SemiEmpiricalEnergyMopacAM1"))
	{
		semiEmpiricalDlg("AM1MopacEnergy");
	}
	else if(!strcmp(name, "SemiEmpiricalOptimizationFireFlyAM1"))
	{
		semiEmpiricalDlg("AM1FireFlyOptimize");
	}
	else if(!strcmp(name, "SemiEmpiricalOptimizationMopacPM6DH2"))
	{
		semiEmpiricalDlg("PM6DH2MopacOptimize");
	}
	else if(!strcmp(name, "SemiEmpiricalOptimizationMopacPM6DH+"))
	{
		semiEmpiricalDlg("PM6DH+MopacOptimize");
	}
	else if(!strcmp(name, "SemiEmpiricalOptimizationMopacPM6"))
	{
		semiEmpiricalDlg("PM6MopacOptimize");
	}
	else if(!strcmp(name, "SemiEmpiricalESPMopacPM6DH2"))
	{
		semiEmpiricalDlg("PM6DH2MopacESP");
	}
	else if(!strcmp(name, "SemiEmpiricalESPMopacPM6DH+"))
	{
		semiEmpiricalDlg("PM6DH+MopacESP");
	}
	else if(!strcmp(name, "SemiEmpiricalESPMopacPM6"))
	{
		semiEmpiricalDlg("PM6MopacESP");
	}
	else if(!strcmp(name, "SemiEmpiricalOptimizationMopac"))
	{
		semiEmpiricalDlg("MopacOptimize");
	}
	else if(!strcmp(name, "SemiEmpiricalOptimizationMopacSparkle"))
	{
		semiEmpiricalDlg("MopacOptimizeSparkle");
	}
	else if(!strcmp(name, "SemiEmpiricalESPMopac"))
	{
		semiEmpiricalDlg("MopacESP");
	}
	else if(!strcmp(name, "SemiEmpiricalScanMopacPM6DH2"))
	{
		semiEmpiricalDlg("MopacScanPM6DH2");
	}
	else if(!strcmp(name, "SemiEmpiricalScanMopacPM6DH+"))
	{
		semiEmpiricalDlg("MopacScanPM6DH+");
	}
	else if(!strcmp(name, "SemiEmpiricalScanMopacPM6"))
	{
		semiEmpiricalDlg("MopacScanPM6");
	}
	else if(!strcmp(name, "SemiEmpiricalScanMopacAM1"))
	{
		semiEmpiricalDlg("MopacScanAM1");
	}
	else if(!strcmp(name, "SemiEmpiricalScanMopac"))
	{
		semiEmpiricalDlg("MopacScan");
	}
	else if(!strcmp(name, "SemiEmpiricalEnergyMopac"))
	{
		semiEmpiricalDlg("MopacEnergy");
	}
	else if(!strcmp(name, "SemiEmpiricalOptimizationMopacAM1"))
	{
		semiEmpiricalDlg("AM1MopacOptimize");
	}
	else if(!strcmp(name, "SemiEmpiricalESPMopacAM1"))
	{
		semiEmpiricalDlg("AM1MopacESP");
	}
	else if(!strcmp(name, "SemiEmpiricalEnergyOrca"))
	{
		semiEmpiricalDlg("OrcaEnergy");
	}
	else if(!strcmp(name, "SemiEmpiricalOptimizationOrca"))
	{
		semiEmpiricalDlg("OrcaOptimize");
	}
	else if(!strcmp(name, "SemiEmpiricalEnergyOpenBabel"))
	{
		semiEmpiricalDlg("OpenBabelEnergy");
	}
	else if(!strcmp(name, "SemiEmpiricalOptimizationOpenBabel"))
	{
		semiEmpiricalDlg("OpenBabelOptimize");
	}
	else if(!strcmp(name, "SemiEmpiricalEnergyGeneric"))
	{
		semiEmpiricalDlg("GenericEnergy");
	}
	else if(!strcmp(name, "SemiEmpiricalOptimizationGeneric"))
	{
		semiEmpiricalDlg("GenericOptimize");
	}


	else if(!strcmp(name, "Close")) destroy_drawing_and_children(NULL, 0);
}
/*--------------------------------------------------------------------*/
static GtkActionEntry gtkActionEntries[] =
{
	{"Read", NULL, N_("_Read")},
	{"ReadAuto", NULL, N_("_Any file"), NULL, "Read a file", G_CALLBACK (activate_action) },
	{"ReadXYZ", NULL, N_("_XYZ file"), NULL, "Read a XYZ file", G_CALLBACK (activate_action) },
	{"ReadMol2", NULL, N_("_Mol2 file"), NULL, "Read a Mol2 file", G_CALLBACK (activate_action) },
	{"ReadTinker", NULL, N_("_Tinker file"), NULL, "Read a Tinker file", G_CALLBACK (activate_action) },
	{"ReadPDB", GABEDIT_STOCK_PDB, N_("_PDB file"), NULL, "Read a PDB file", G_CALLBACK (activate_action) },
	{"ReadHyperchem", NULL, N_("_Hyperchem file"), NULL, "Read a Hyperchem file", G_CALLBACK (activate_action) },
	{"ReadAIMAll", NULL, N_("_AIMAll file"), NULL, "Read a AIMAll file", G_CALLBACK (activate_action) },
	{"ReadMol", NULL, N_("_Mol file"), NULL, "Read a Mol file", G_CALLBACK (activate_action) },
	{"ReadGabedit", GABEDIT_STOCK_GABEDIT, N_("_Gabedit file"), NULL, "Read a Gabedit file", G_CALLBACK (activate_action) },
	{"ReadPOSCAR", NULL, N_("_POSCAR file"), NULL, "Read a POSCAR file", G_CALLBACK (activate_action) },
	{"ReadWFX", NULL, N_("_WFX file"), NULL, "Read a WFX file", G_CALLBACK (activate_action) },
	{"ReadCIF", NULL, N_("_CIF file"), NULL, "Read a cif file", G_CALLBACK (activate_action) },
	{"ReadCIFNoSym", NULL, N_("_CIF file without symmetry"), NULL, "Read a cif file without apply symmetry operators", G_CALLBACK (activate_action) },
	{"ReadGaussianZMat", GABEDIT_STOCK_GAUSSIAN, N_("_Gaussian Z-Matrix file"), NULL, "Read a Gaussian Z-Matrix file", G_CALLBACK (activate_action) },
	{"ReadMopacZMat", GABEDIT_STOCK_MOPAC, N_("_Mopac Z-Matrix file"), NULL, "Read a Mopac Z-Matrix file", G_CALLBACK (activate_action) },

	{"FireFly", GABEDIT_STOCK_FIREFLY, "_FireFly"},
	{"ReadFireFlyFirst", GABEDIT_STOCK_FIREFLY, N_("F_irst geometry from a FireFly output file"), NULL, "Read the first geometry from a FireFly output file", G_CALLBACK (activate_action) },
	{"ReadFireFlyLast", GABEDIT_STOCK_FIREFLY, N_("L_ast geometry from a FireFly output file"), NULL, "Read the last geometry from a FireFly output file", G_CALLBACK (activate_action) },

	{"Dalton", GABEDIT_STOCK_DALTON, "_Dalton"},
	{"ReadDaltonFirst", GABEDIT_STOCK_DALTON, N_("F_irst geometry from a Dalton output file"), NULL, "Read the first geometry from a Dalton output file", G_CALLBACK (activate_action) },
	{"ReadDaltonLast", GABEDIT_STOCK_DALTON, N_("L_ast geometry from a Dalton output file"), NULL, "Read the last geometry from a Dalton output file", G_CALLBACK (activate_action) },

	{"Gamess", GABEDIT_STOCK_GAMESS, "Ga_mess"},
	{"ReadGamessFirst", GABEDIT_STOCK_GAMESS, N_("F_irst geometry from a Gamess output file"), NULL, "Read the first geometry from a Gamess output file", G_CALLBACK (activate_action) },
	{"ReadGamessLast", GABEDIT_STOCK_GAMESS, N_("L_ast geometry from a Gamess output file"), NULL, "Read the last geometry from a Gamess output file", G_CALLBACK (activate_action) },

	{"Turbomole", NULL, "_Turbomole"},
	{"ReadTurbomoleFirst", NULL, N_("F_irst geometry from a Turbomole output file"), NULL, "Read the first geometry from a Turbomole output file", G_CALLBACK (activate_action) },
	{"ReadTurbomoleLast", NULL, N_("L_ast geometry from a Turbomole output file"), NULL, "Read the last geometry from a Turbomole output file", G_CALLBACK (activate_action) },

	{"Gaussian", GABEDIT_STOCK_GAUSSIAN, "_Gaussian"},
	{"ReadGaussianInput", GABEDIT_STOCK_GAUSSIAN, N_("_Gaussian Input file"), NULL, "Read a Gaussian Input file", G_CALLBACK (activate_action) },
	{"ReadGaussianFirst", GABEDIT_STOCK_GAUSSIAN, N_("F_irst geometry from a Gaussian output file"), NULL, "Read the first geometry from a Gaussian output file", G_CALLBACK (activate_action) },
	{"ReadGaussianLast", GABEDIT_STOCK_GAUSSIAN, N_("L_ast geometry from a Gaussian output file"), NULL, "Read the last geometry from a Gaussian output file", G_CALLBACK (activate_action) },
	{"ReadGaussianFChk", GABEDIT_STOCK_GAUSSIAN, N_("Geometry from a Gaussian fchk file"), NULL, "Read  geometry from a Gaussian fchk file", G_CALLBACK (activate_action) },

	{"Molcas", GABEDIT_STOCK_MOLCAS, "Mol_cas"},
	{"ReadMolcasInput", GABEDIT_STOCK_MOLCAS, N_("Mol_cas Input file"), NULL, "Read a Molcas Input file", G_CALLBACK (activate_action) },
	{"ReadMolcasFirst", GABEDIT_STOCK_MOLCAS, N_("F_irst geometry from a Molcas output file"), NULL, "Read the first geometry from a Molcas output file", G_CALLBACK (activate_action) },
	{"ReadMolcasLast", GABEDIT_STOCK_MOLCAS, N_("L_ast geometry from a Molcas output file"), NULL, "Read the last geometry from a Molcas output file", G_CALLBACK (activate_action) },

	{"Molpro", GABEDIT_STOCK_MOLPRO, "Mol_pro"},
	{"ReadMolproInput", GABEDIT_STOCK_MOLPRO, N_("Mol_pro Input file"), NULL, "Read a Molpro Input file", G_CALLBACK (activate_action) },
	{"ReadMolproFirst", GABEDIT_STOCK_MOLPRO, N_("F_irst geometry from a Molpro output file"), NULL, "Read the first geometry from a Molpro output file", G_CALLBACK (activate_action) },
	{"ReadMolproLast", GABEDIT_STOCK_MOLPRO, N_("L_ast geometry from a Molpro output file"), NULL, "Read the last geometry from a Molpro output file", G_CALLBACK (activate_action) },

	{"Mopac", GABEDIT_STOCK_MOPAC, "_Mopac"},
	{"ReadMopacInput", GABEDIT_STOCK_MOPAC, N_("_Mopac Input file"), NULL, "Read a Mopac Input file", G_CALLBACK (activate_action) },
	{"ReadMopacFirst", GABEDIT_STOCK_MOPAC, N_("F_irst geometry from a Mopac output file"), NULL, "Read the first geometry from a Mopac output file", G_CALLBACK (activate_action) },
	{"ReadMopacLast", GABEDIT_STOCK_MOPAC, N_("L_ast geometry from a Mopac output file"), NULL, "Read the last geometry from a Mopac output file", G_CALLBACK (activate_action) },
	{"ReadMopacAux", GABEDIT_STOCK_MOPAC, N_("L_ast geometry from a Mopac aux file"), NULL, "Read the last geometry from a Mopac aux file", G_CALLBACK (activate_action) },
	{"ReadMopacScan", GABEDIT_STOCK_MOPAC, N_("Geometries from a Mopac _scan output file"), NULL, "Geometries from a Mopac scan output file", G_CALLBACK (activate_action) },
	{"ReadMopacIRC", GABEDIT_STOCK_MOPAC, N_("Geometries from a Mopac _IRC output file"), NULL, "Geometries from a Mopac IRC output file", G_CALLBACK (activate_action) },

	{"MPQC", GABEDIT_STOCK_MPQC, "MP_QC"},
	{"ReadMPQCInput", GABEDIT_STOCK_MPQC, N_("MP_QC Input file"), NULL, "Read a MPQC Input file", G_CALLBACK (activate_action) },
	{"ReadMPQCFirst", GABEDIT_STOCK_MPQC, N_("F_irst geometry from a MPQC output file"), NULL, "Read the first geometry from a MPQC output file", G_CALLBACK (activate_action) },
	{"ReadMPQCLast", GABEDIT_STOCK_MPQC, N_("L_ast geometry from a MPQC output file"), NULL, "Read the last geometry from a MPQC output file", G_CALLBACK (activate_action) },

	{"Orca", GABEDIT_STOCK_ORCA, "_Orca"},
	{"ReadOrcaFirst", GABEDIT_STOCK_ORCA, N_("F_irst geometry from a Orca output file"), NULL, "Read the first geometry from a Orca output file", G_CALLBACK (activate_action) },
	{"ReadOrcaLast", GABEDIT_STOCK_ORCA, N_("L_ast geometry from a Orca output file"), NULL, "Read the last geometry from a Orca output file", G_CALLBACK (activate_action) },

	{"VASP", GABEDIT_STOCK_VASP, "_VASP"},
	{"ReadVaspFirst", GABEDIT_STOCK_VASP, N_("F_irst geometry from a VASP output file"), NULL, "Read the first geometry from a VASP output file", G_CALLBACK (activate_action) },
	{"ReadVaspLast", GABEDIT_STOCK_VASP, N_("L_ast geometry from a VASP output file"), NULL, "Read the last geometry from a VASP output file", G_CALLBACK (activate_action) },
	{"ReadVaspXMLFirst", GABEDIT_STOCK_VASP, N_("F_irst geometry from a VASP xml file"), NULL, "Read the first geometry from a VASP xml file", G_CALLBACK (activate_action) },
	{"ReadVaspXMLLast", GABEDIT_STOCK_VASP, N_("L_ast geometry from a VASP xml file"), NULL, "Read the last geometry from a VASP xml file", G_CALLBACK (activate_action) },


	{"QChem", GABEDIT_STOCK_QCHEM, "Q-_Chem"},
	{"ReadQChemFirst", GABEDIT_STOCK_QCHEM, N_("F_irst geometry from a Q-Chem output file"), NULL, "Read the first geometry from a Q-Chem output file", G_CALLBACK (activate_action) },
	{"ReadQChemLast", GABEDIT_STOCK_QCHEM, N_("L_ast geometry from a Q-Chem output file"), NULL, "Read the last geometry from a Q-Chem output file", G_CALLBACK (activate_action) },

	{"NWChem", GABEDIT_STOCK_NWCHEM, "_NWChem"},
	{"ReadNWChemFirst", GABEDIT_STOCK_NWCHEM, N_("F_irst geometry from a NWChem output file"), NULL, "Read the first geometry from a NWChem output file", G_CALLBACK (activate_action) },
	{"ReadNWChemLast", GABEDIT_STOCK_NWCHEM, N_("L_ast geometry from a NWChem output file"), NULL, "Read the last geometry from a NWChem output file", G_CALLBACK (activate_action) },

	{"Psicode", GABEDIT_STOCK_PSICODE, "_Psicode"},
	{"ReadPsicodeFirst", GABEDIT_STOCK_PSICODE, N_("F_irst geometry from a Psicode output file"), NULL, "Read the first geometry from a Psicode output file", G_CALLBACK (activate_action) },
	{"ReadPsicodeLast", GABEDIT_STOCK_PSICODE, N_("L_ast geometry from a Psicode output file"), NULL, "Read the last geometry from a Psicode output file", G_CALLBACK (activate_action) },

	{"ReadUsingOpenBabel", GABEDIT_STOCK_OPEN_BABEL, N_("_Other format (using open babel)"), NULL, "Other format (using open babel)", G_CALLBACK (activate_action) },

	{"ReadGeomConv", NULL, N_("Geometries _Convergence")},
	{"ReadGeomConvDalton", GABEDIT_STOCK_DALTON, N_("from a _Dalton output file"), NULL, "Read Geometries Convergence from a Dalton output file", G_CALLBACK (activate_action) },
	{"ReadGeomConvGamess", GABEDIT_STOCK_GAMESS, N_("from a _Gamess output file"), NULL, "Read Geometries Convergence from a Gamess output file", G_CALLBACK (activate_action) },
	{"ReadGeomConvFireFly", GABEDIT_STOCK_FIREFLY, N_("from a _FireFly output file"), NULL, "Read Geometries Convergence from a FireFly output file", G_CALLBACK (activate_action) },
	{"ReadGeomIRCGamess", GABEDIT_STOCK_GAMESS, N_("from a _Gamess IRC file"), NULL, "Read Geometries from a Gamess IRC file", G_CALLBACK (activate_action) },
	{"ReadGeomIRCFireFly", GABEDIT_STOCK_FIREFLY, N_("from a _FireFly IRC file"), NULL, "Read Geometries from a FireFly IRC file", G_CALLBACK (activate_action) },
	{"ReadGeomConvGaussian", GABEDIT_STOCK_GAUSSIAN, N_("from a _Gaussian output file"), NULL, "Read Geometries Convergence from a Gaussian output file", G_CALLBACK (activate_action) },
	{"ReadGeomConvMolpro", GABEDIT_STOCK_MOLPRO, N_("from a Mol_pro log file"), NULL, "Read Geometries Convergence from a Molpro log file", G_CALLBACK (activate_action) },
	{"ReadGeomConvMopac", GABEDIT_STOCK_MOPAC, N_("from a _Mopac aux file"), NULL, "Read Geometries Convergence from a Mopac aux file", G_CALLBACK (activate_action) },
	{"ReadGeomConvMPQC", GABEDIT_STOCK_MPQC, N_("from a MP_QC output file"), NULL, "Read Geometries Convergence from a MPQC output file", G_CALLBACK (activate_action) },
	{"ReadGeomConvOrca", GABEDIT_STOCK_ORCA, N_("from a _Orca output file"), NULL, "Read Geometries Convergence from a Orca output file", G_CALLBACK (activate_action) },
	{"ReadGeomConvVasp", GABEDIT_STOCK_VASP, N_("from a _VASP output file"), NULL, "Read Geometries Convergence from a VASP output file", G_CALLBACK (activate_action) },
	{"ReadGeomConvVaspXML", GABEDIT_STOCK_VASP, N_("from a _VASP xml file"), NULL, "Read Geometries Convergence from a VASP xml file", G_CALLBACK (activate_action) },
	{"ReadGeomConvNWChem", GABEDIT_STOCK_NWCHEM, N_("from a _NWChemoutput file"), NULL, "Read Geometries Convergence from a NWChem output file", G_CALLBACK (activate_action) },
	{"ReadGeomConvPsicode", GABEDIT_STOCK_PSICODE, N_("from a _Psicodeoutput file"), NULL, "Read Geometries Convergence from a Psicode output file", G_CALLBACK (activate_action) },
	{"ReadGeomConvQChem", GABEDIT_STOCK_QCHEM, N_("from a Q-_Chem output file"), NULL, "Read Geometries Convergence from a Q-Chem output file", G_CALLBACK (activate_action) },
	{"ReadGeomConvGabedit", GABEDIT_STOCK_GABEDIT, N_("from a G_abedit file"), NULL, "Read Geometries Convergence from a Gabedit file", G_CALLBACK (activate_action) },
	{"ReadGeomConvMolden", GABEDIT_STOCK_MOLDEN, N_("from a Mol_den file"), NULL, "Read Geometries Convergence from a Molden file", G_CALLBACK (activate_action) },
	{"ReadGeomConvXYZ", NULL, N_("from a _XYZ file"), NULL, "Read several Geometries from a XYZ file", G_CALLBACK (activate_action) },

	{"Edit", NULL, N_("_Edit")},
	{"EditResetAllConnections", GABEDIT_STOCK_ADD_BOND, N_("_Reset all connections"), NULL, "Reset all connections", G_CALLBACK (activate_action) },
	{"EditResetMultipleConnections", GABEDIT_STOCK_COPY, N_("_Reset multiple connections"), NULL, "Reset multiple connections", G_CALLBACK (activate_action) },

	{"EditResetConnectionsBetweenSelectedAndNotSelectedAtoms", GABEDIT_STOCK_ADD_BOND, N_("_Reset connections between selected and not selected atoms"), NULL, "Reset connections between selected and not selected atoms", G_CALLBACK (activate_action) },
	{"EditResetConnectionsBetweenSelectedAtoms", GABEDIT_STOCK_ADD_BOND, N_("_Reset connections between selected atoms"), NULL, "Reset connections between selected atoms", G_CALLBACK (activate_action) },

	{"EditCopySelectedAtoms", GABEDIT_STOCK_COPY, N_("_Copy&Paste selected atoms"), NULL, "Copy&Paste selected atoms", G_CALLBACK (activate_action) },
	{"EditDeleteHydrogenAtoms", GABEDIT_STOCK_CUT, N_("_Remove hydrogen atoms"), NULL, "Remove hydrogen atoms", G_CALLBACK (activate_action) },
	{"EditDeleteSelectedAtoms", GABEDIT_STOCK_CUT, N_("Remove selected atoms"), NULL, "Remove selected atoms", G_CALLBACK (activate_action) },
	{"EditMoveCenterOfSelectedAtomsToOrigin", GABEDIT_STOCK_MOVE_ATOM, N_("Move the center of selected atoms to origin"), NULL, "Move selected atoms to origin", G_CALLBACK (activate_action) },
	{"EditAlignPrincipalAxesOfSelectedAtomsToXYZ", NULL, N_("_Align the principal axes selected atoms to XYZ"), NULL, "Align the principal axes of selected atoms to XYZ", G_CALLBACK (activate_action) },
	{"EditAlignSelectedAndNotSelectedAtoms", NULL, N_("_Align selected and not selected atoms"), NULL, "Align selected and not selected atoms to XYZ", G_CALLBACK (activate_action) },
	{"EditDeleteMolecule", GABEDIT_STOCK_CUT, N_("_Delete molecule"), NULL, "Delete molecule", G_CALLBACK (activate_action) },
	{"EditOpenGeometryEditor", NULL, N_("_Open XYZ or GZMAT editor"), NULL, "Open XYZ or GZMAT editor", G_CALLBACK (activate_action) },
	{"Selection", NULL, N_("_Selection")},
	{"EditSelectAll", NULL, N_("Select _all atoms"), NULL, "Select all atoms", G_CALLBACK (activate_action) },
	{"EditInvertSelection", NULL, N_("_Invert selection"), NULL, "Invert selection", G_CALLBACK (activate_action) },
	{"EditUnSelectAll", NULL, N_("_Unselect all"), NULL, "Unselect all", G_CALLBACK (activate_action) },
	{"EditSelectHighAtoms", NULL, N_("Select atoms with _high layer"), NULL, "Select atoms with high layer", G_CALLBACK (activate_action) },
	{"EditSelectMediumAtoms", NULL, N_("Select atoms with _medium layer"), NULL, "Select atoms with medium layer", G_CALLBACK (activate_action) },
	{"EditSelectLowAtoms", NULL, N_("Select atoms with _low layer"), NULL, "Select atoms with low layer", G_CALLBACK (activate_action) },
	{"EditSelectFixedAtoms", NULL, N_("Select _freezing atoms during optimizations/MD"), NULL, "Select freezing atoms during optimizations/MD", G_CALLBACK (activate_action) },
	{"EditSelectVariableAtoms", NULL, N_("Select _not freezing atoms during optimizations/MD"), NULL, "Select not freezing atoms during optimizations/MD", G_CALLBACK (activate_action) },
	{"EditSelectFirstResidue", NULL, N_("Select the _first residue"), NULL, "Select the first residue", G_CALLBACK (activate_action) },
	{"EditSelectLastResidue", NULL, N_("Select the _last residue"), NULL, "Select the last residue", G_CALLBACK (activate_action) },
	{"EditSelectResidueByNumber", NULL, N_("Select redidue by number"), NULL, "Select residue by number", G_CALLBACK (activate_action) },
	{"EditSelectResidueByName", NULL, N_("Select residues by name"), NULL, "Select residues by name", G_CALLBACK (activate_action) },
	{"EditSelectAtomsByMMType", NULL, N_("Select atoms by MM type"), NULL, "Select atoms by MM type", G_CALLBACK (activate_action) },
	{"EditSelectAtomsByPDBType", NULL, N_("Select atoms by PDB type"), NULL, "Select atoms by PDB type", G_CALLBACK (activate_action) },
	{"EditSelectAtomsBySymbol", NULL, N_("Select atoms by symbol"), NULL, "Select atoms by symbol", G_CALLBACK (activate_action) },
	{"EditSelectAtomsBySphere", NULL, N_("Select atoms by sphere"), NULL, "Select atoms by sphere", G_CALLBACK (activate_action) },
	{"EditSelectAtomsByPositiveCharges", NULL, N_("Select atoms with positive charges"), NULL, "Select atoms with positive charges", G_CALLBACK (activate_action) },
	{"EditSelectAtomsByNegativeCharges", NULL, N_("Select atoms with negative charges"), NULL, "Select atoms with negative charges", G_CALLBACK (activate_action) },
	{"EditSelectAtomsMultiple", NULL, N_("Multiple select atoms"), NULL, "Multiple select atoms", G_CALLBACK (activate_action) },

	{"SaveAs", NULL, N_("_Save as")},
	{"SaveAsGabedit", GABEDIT_STOCK_GABEDIT, N_("_Gabedit file"), NULL, "Save geometry in a Gabedit file", G_CALLBACK (activate_action) },
	{"SaveAsXYZ", NULL, N_("_XYZ file"), NULL, "Save geometry in a XYZ file", G_CALLBACK (activate_action) },
	{"SaveAsMol2", NULL, N_("_Mol2 file"), NULL, "Save geometry in a Mol2 file", G_CALLBACK (activate_action) },
	{"SaveAsMol", NULL, N_("_Mol file"), NULL, "Save geometry in a Mol file", G_CALLBACK (activate_action) },
	{"SaveAsTinker", NULL, N_("_Tinker file"), NULL, "Save geometry in a Tinker file", G_CALLBACK (activate_action) },
	{"SaveAsPDB", GABEDIT_STOCK_PDB, N_("_pdb file"), NULL, "Save geometry in a pdb file", G_CALLBACK (activate_action) },
	{"SaveAsPOSCARCartn", NULL, N_("_POSCAR (Cartesian) file"), NULL, "Save geometry in a POSCAR file", G_CALLBACK (activate_action) },
	{"SaveAsPOSCARDirect", NULL, N_("_POSCAR (Direct) file"), NULL, "Save geometry in a POSCAR file", G_CALLBACK (activate_action) },
	{"SaveAsCIFAllAtoms", NULL, N_("_CIF file with all atoms"), NULL, "Save geometry in a CIF file", G_CALLBACK (activate_action) },
	{"SaveAsCIF", NULL, N_("_CIF file with symmetry operators"), NULL, "Save geometry in a CIF file", G_CALLBACK (activate_action) },
	{"SaveAsHyperchem", NULL, N_("_Hyperchem file"), NULL, "Save geometry in a Hyperchem file", G_CALLBACK (activate_action) },
	{"SaveAsCChemI", NULL, N_("_CChemI file"), NULL, "Save geometry in a CChemI file", G_CALLBACK (activate_action) },
	{"SaveAsMopacZMat", NULL, N_("_Mopac Zmatrix file"), NULL, "Save geometry in a Mopac Zmatrix file", G_CALLBACK (activate_action) },
	{"SaveAsGaussianZMat", GABEDIT_STOCK_GAUSSIAN, N_("_Gaussian Zmatrix file"), NULL, "Save geometry in a Gaussian Zmatrix file", G_CALLBACK (activate_action) },
	{"SaveUsingOpenBabel", GABEDIT_STOCK_OPEN_BABEL, N_("_Other format (using open babel)"), NULL, "Other format (using open babel)", G_CALLBACK (activate_action) },

	{"Add", NULL, N_("_Add")},
	{"PersonalFragments", NULL, N_("Personal _fragment")},
	{"PersonalFragmentsNewGroup", NULL, N_("_New Group"), NULL, "New Group", G_CALLBACK (activate_action) },
	{"PersonalFragmentsDeleteGroup", NULL, N_("_Delete a Group"), NULL, "Delete a Group", G_CALLBACK (activate_action) },
	{"PersonalFragmentsAddMolecule", NULL, N_("_Add this molecule to personal Fragments"), NULL, "Add this molecule to personal Fragments", G_CALLBACK (activate_action) },
	{"PersonalFragmentsRemoveFragment", NULL, N_("_Remove a Fragment"), NULL, "Remove a Fragment", G_CALLBACK (activate_action) },
	{"InsertAFragment", NULL, N_("Add a _fragment"), NULL, "Add a fragment", G_CALLBACK (activate_action) },
	{"AddMaxHydrogens", NULL, N_("Add _Max Hydrogens"), NULL, "Add Max Hydrogens", G_CALLBACK (activate_action) },
	{"AddHydrogens", NULL, N_("Add _Hydrogens"), NULL, "Add Hydrogens", G_CALLBACK (activate_action) },
	{"AddOneHydrogen", NULL, N_("Add _one Hydrogen"), NULL, "Add one Hydrogen", G_CALLBACK (activate_action) },
	{"AddHydrogensTpl", NULL, N_("Add _Hydrogens using PDB template"), NULL, "Add Hydrogens using PDB template", G_CALLBACK (activate_action) },

	{"Build", NULL, N_("_Build")},
	{"BuildLinearMolecule", NULL, N_("_Linear Molecule"), NULL, "build a linear molecule", G_CALLBACK (activate_action) },
	{"BuildRingMolecule", NULL, N_("_Ring Molecule"), NULL, "build a ring molecule", G_CALLBACK (activate_action) },
	{"BuildMoleculeWithSymmetry", NULL, N_("_Molecule with a symmetry axis of rotation"), NULL, "build a molecule with a symmetry axis of rotation", G_CALLBACK (activate_action) },
	{"BuildPolyPeptide", NULL, N_("Poly_Peptide"), NULL, "build a polypeptide", G_CALLBACK (activate_action) },
	{"BuildPolySaccharide", NULL, N_("Poly_Saccharide"), NULL, "build a Polysaccharide", G_CALLBACK (activate_action) },
	{"BuildPolyNucleicAcid", NULL, N_("Poly_Nucleic Acid"), NULL, "build a polynucleic acid", G_CALLBACK (activate_action) },
	{"BuildNanoTube", NULL, N_("Nano_tube"), NULL, "nanotube", G_CALLBACK (activate_action) },

#ifdef DRAWGEOMGL
	{"Crystallography", NULL, N_("_Crystallography")},
	{"StandardizeCellPrimitive", NULL, N_("_Standardize cell with reduction to primitive"), NULL, "Standardize Cell Primitive", G_CALLBACK (activate_action) },
	{"StandardizeCellConv", NULL, N_("_Standardize conventional cell"), NULL, "Standardize Cell", G_CALLBACK (activate_action) },
	{"WrapAtomsToCell", NULL, N_("_Wrap atoms to Cell"), NULL, "Wrap", G_CALLBACK (activate_action) },
	{"PrototypeCrystal", NULL, N_("Get a _Prototype crystal"), NULL, "Prototype crystal", G_CALLBACK (activate_action) },
	{"ComputeVolume", NULL, N_("Compute _volume of cell"), NULL, "Volume", G_CALLBACK (activate_action) },
	{"ComputeSpaceGroupSym", NULL, N_("Get Space _Group symmetry"), NULL, "SpaceGroupe", G_CALLBACK (activate_action) },
	{"ComputeSymmetryInfo", NULL, N_("Get _symmetry info"), NULL, "Symmetry Info", G_CALLBACK (activate_action) },
	{"ComputeKPointsPath", NULL, N_("Get k-points for band structure calculation"), NULL, "kpoints path primitive", G_CALLBACK (activate_action) },
	{"SetSymPrec", NULL, N_("Set symmetry precision"), NULL, "Set symprec", G_CALLBACK (activate_action) },
	{"HelpCrystal", NULL, N_("Help & _references"), NULL, "Help", G_CALLBACK (activate_action) },

	{"BuildCrystalsDeriv", NULL, "_Build"},
	{"BuildSuperCellSimple", NULL, N_("_SuperCell (simple)"), NULL, "Super cell (simple) ", G_CALLBACK (activate_action) },
	{"BuildSuperCell", NULL, N_("_SuperCell"), NULL, "Super cell ", G_CALLBACK (activate_action) },
	{"BuildWulff", NULL, N_("Cluster using _Wulff construction"), NULL, "Wulff", G_CALLBACK (activate_action) },
	{"BuildSlab", NULL, N_("_Slab"), NULL, "Slab", G_CALLBACK (activate_action) },
	{"BuildCrystalGen", NULL, N_("_Crystal"), NULL, "build  crystal", G_CALLBACK (activate_action) },


	{"Reduction", NULL, "_Reduction"},
	{"ReductionNiggli", NULL, N_("Reduce to _Niggli cell"), NULL, "Niggli", G_CALLBACK (activate_action) },
	{"ReductionDelaunay", NULL, N_("Reduction using _Delaunay method"), NULL, "Delaunay", G_CALLBACK (activate_action) },
	{"ReductionPrimitive", NULL, N_("Reduction to _primitive"), NULL, "Primitive", G_CALLBACK (activate_action) },
#endif

	{"Operations", NULL, N_("_Operations")},
	{"Labels", NULL, N_("_Labels")},
	{"Render", NULL, N_("_Render")},
	{"RenderDefault", NULL, N_("_Default")},
	{"RenderDefaultStick", GABEDIT_STOCK_RENDER_STICK, N_("_Stick"), NULL, "default stick scale", G_CALLBACK (activate_action) },
	{"RenderDefaultBall", GABEDIT_STOCK_RENDER_BALL_STICK, N_("_Ball"), NULL, "default ball scale", G_CALLBACK (activate_action) },
	{"RenderDefaultZoom", GABEDIT_STOCK_ZOOM, N_("_Zoom"), NULL, "default zoom scale", G_CALLBACK (activate_action) },
	{"RenderDefaultDipole", NULL, N_("_Dipole"), NULL, "default dipole scale", G_CALLBACK (activate_action) },
	{"RenderDefaultCenter", NULL, N_("_Center"), NULL, "center of molecule on centre of screen", G_CALLBACK (activate_action) },
	{"RenderDefaultAll", NULL, N_("_All"), NULL, "reset default parameters", G_CALLBACK (activate_action) },
	{"RenderBackgroundColor", NULL, N_("_Background Color")},
	{"RenderBackgroundColorBlack", NULL, N_("_Black"), NULL, "black background", G_CALLBACK (activate_action) },
	{"RenderBackgroundColorOther", NULL, N_("_Other"), NULL, "reset the background color", G_CALLBACK (activate_action) },
	{"RenderHideHydrogenAtoms", NULL, N_("Hide _hydrogen atoms"), NULL, "Hide hydrogen atoms", G_CALLBACK (activate_action) },
	{"RenderHideNotSelectedAtoms", NULL, N_("Hide _not selected atoms"), NULL, "Hide not selected atoms", G_CALLBACK (activate_action) },
	{"RenderHideSelectedAtoms", NULL, N_("Hide _selected atoms"), NULL, "Hide selected atoms", G_CALLBACK (activate_action) },
	{"RenderShowHydrogenAtoms", NULL, N_("_Show hydrogen atoms"), NULL, "Show hydrogen atoms", G_CALLBACK (activate_action) },
	{"RenderShowAllAtoms", NULL, N_("_Show all atoms"), NULL, "Show all atoms", G_CALLBACK (activate_action) },
#ifdef DRAWGEOMGL
	{"RenderLight",     NULL, N_("_Light")},
#endif
	{"RenderOptimal",  GABEDIT_STOCK_O, N_("_Optimal camera"), NULL, "optimal camera", G_CALLBACK (activate_action) },

	{"Symmetry", NULL, N_("_Symmetry")},
	{"SymmetryRotationalConstantes", NULL, N_("Rotational Constantes & Dipole at there principal axis"), NULL, "compute the rotational constantes &  the dipole at there principal axis", G_CALLBACK (activate_action) },
	{"SymmetryGroupSymmetry", NULL, N_("_Group of symmetry & Geometry with reduce molecule to its basis set of atoms"), NULL, "compute the _Groupe symmetry and geometry with  reduce molecule to its basis set of atoms", G_CALLBACK (activate_action) },
	{"SymmetryAbelianGroup", NULL, N_("_Abelian group & Geometry with  reduce molecule to its basis set of atoms"), NULL, "compute the _Abelian group and Geometry with reduce molecule to its basis set of atoms", G_CALLBACK (activate_action) },
	{"SymmetryGroupSymmetrize", NULL, N_("_Symmetrize"), NULL, "compute the _Groupe symmetry and geometry with reduce symmetrization", G_CALLBACK (activate_action) },
	{"SymmetrySetTolerance", NULL, N_("_Set tolerance parameters"), NULL, "Set tolerance parameters", G_CALLBACK (activate_action) },

	{"Set", NULL, N_("_Set")},
	{"SetOriginToCenterOfMolecule", NULL, N_("Set origin at _Center of molecule"), NULL, "Set origin at center of molecule", G_CALLBACK (activate_action) },
	{"SetOriginToCenterOfSelectedAtoms", NULL, N_("Set origin at Center of _selected atoms"), NULL, "Set origin at center of selected atoms", G_CALLBACK (activate_action) },
	{"SetXYZToPAX", NULL, N_("Set XYZ axes to the principal axes of selected atoms (_X = min inertia)"), NULL, "Set XYZ axes to the principal axes of selected atoms (X = min inertia)", G_CALLBACK (activate_action) },
	{"SetXYZToPAZ", NULL, N_("Set XYZ axes to the principal axes of selected atoms (_Z = min inertia)"), NULL, "Set XYZ axes to the principal axes of selected atoms (Z axis = min inertia)", G_CALLBACK (activate_action) },
	{"SetXYZToStandardOrientaion", NULL, N_("Set XYZ axes to the standard orientation"), NULL, "Set XYZ axes to standard orientation", G_CALLBACK (activate_action) },
	{"SetXYZToStandardOrientaionSelectedAndNotSelected", NULL, N_("Align seletecd and not selected fragments"), NULL, "Align 2 fragments", G_CALLBACK (activate_action) },
	{"SetSelectedAtomsToHighLayer", NULL, N_("Set selected atoms to _Hight layer"), NULL, "Set selected atoms to Hight layer", G_CALLBACK (activate_action) },
	{"SetSelectedAtomsToMediumLayer", NULL, N_("Set selected atoms to _Medium layer"), NULL, "Set selected atoms to Medium layer", G_CALLBACK (activate_action) },
	{"SetSelectedAtomsToLowLayer", NULL, N_("Set selected atoms to _Low layer"), NULL, "Set selected atoms to Low layer", G_CALLBACK (activate_action) },
	{"SetSelectedAtomsToFixed", NULL, N_("Set selected atoms to _freeze during optimizations/MD"), NULL, "Set selected atoms to freeze during optimizations/MD", G_CALLBACK (activate_action) },
	{"SetSelectedAtomsToVariable", NULL, N_("Set selected atoms to _not freeze during optimizations/MD"), NULL, "Set selected atoms to not freeze during optimizations/MD", G_CALLBACK (activate_action) },
	{"SetMMTypeOfselectedAtoms", NULL, N_("Set the _MM type of selected atoms"), NULL, "Set the MM type of selected atoms", G_CALLBACK (activate_action) },
	{"SetPDBTypeOfselectedAtoms", NULL, N_("Set the _PDB type of selected atoms"), NULL, "Set the PDB type of selected atoms", G_CALLBACK (activate_action) },
	{"SetResidueNameOfselectedAtoms", NULL, N_("Set the Residue _Name of selected atoms"), NULL, "Set the Residue name of selected atoms", G_CALLBACK (activate_action) },
	{"SetSymbolOfselectedAtoms", NULL, N_("Set the symbol of selected atoms"), NULL, "Set the symbol of selected atoms", G_CALLBACK (activate_action) },
	{"SetChargeOfselectedAtoms", NULL, N_("Set the _Charge of selected atoms"), NULL, "Set the charge of selected atoms", G_CALLBACK (activate_action) },
	{"scaleChargesOfSelectedAtoms", NULL, N_("scale the _Charge of selected atoms"), NULL, "scale the charge of selected atoms", G_CALLBACK (activate_action) },
	{"SetDipole", NULL, N_("_Dipole"), NULL, "Set dipole", G_CALLBACK (activate_action) },
	{"ScaleCell", NULL, N_("_Scale cell"), NULL, "Scale cell", G_CALLBACK (activate_action) },
	{"SetDipoleFormCharges", NULL, N_("_Compute Dipole from charges"), NULL, "Compute dipole using charges", G_CALLBACK (activate_action) },
	{"SetHydrogenBonds", NULL, N_("_Hydrogen bonds parameters"), NULL, "Set the hydrogen bonds parameters", G_CALLBACK (activate_action) },
	{"SetPropertiesOfAtoms", NULL, N_("P_roperties of atoms"), NULL, "Set properties of atoms", G_CALLBACK (activate_action) },
	{"SetMolecularMechanicsParameters", NULL, N_("_Molecular Mechanics Parameters"), NULL, "Set molecular mechanics parameters", G_CALLBACK (activate_action) },
	{"SetPDBTemplate", NULL, N_("_PDB Template"), NULL, "Set PDB Template", G_CALLBACK (activate_action) },
	{"SetChargesUsingPDBTemplate", NULL, N_("_Charges using PDB Template"), NULL, "Set charges using PDB Template", G_CALLBACK (activate_action) },
	{"SetAtomTypesUsingPDBTemplate", NULL, N_("Atom _Types using PDB Template"), NULL, "Set atom types using PDB Template", G_CALLBACK (activate_action) },
	{"SetAtomTypeAndChargeUsingPDBTemplate", NULL, N_("Atom Type&Charge using PDB Template"), NULL, "Set atom type and charge using PDB Template", G_CALLBACK (activate_action) },
	{"SetAtomTypeCalcul", NULL, N_("Atom Types using connections types"), NULL, "Compute atom types using the types of connections", G_CALLBACK (activate_action) },
	{"SetChargesToZero", NULL, N_("Charges to _zero"), NULL, "Set charges to zero", G_CALLBACK (activate_action) },
	{"SetPovrayBackground", NULL, N_("_Povray background"), NULL, "Set povray background", G_CALLBACK (activate_action) },
#ifdef DRAWGEOMGL
	{"SetCamera", NULL, N_("_Camera"), NULL, "Set camera", G_CALLBACK (activate_action) },
	{"SetLightPositions", NULL, N_("_Light position"), NULL, "Set light position", G_CALLBACK (activate_action) },
	{"SetXYZAxesProperties", NULL, N_("_XYZ axes properties"), NULL, "Set axes properties", G_CALLBACK (activate_action) },
#endif

	{"SetAtomToInsert", GABEDIT_STOCK_ATOMTOINSERT, N_("Set _atom to insert"), NULL, "Set atom to insert", G_CALLBACK (activate_action) },

	{"Export", NULL, N_("Ex_port")},
	{"ExportPostscript", NULL, "P_ostscript", NULL, "create a postscript file", G_CALLBACK (activate_action) },
	{"ExportEPS", NULL, "_Encapsuled Postscript", NULL, "create a Encapsuled Postscript file", G_CALLBACK (activate_action) },
	{"ExportPovray", NULL, "Po_vray", NULL, "create a povray file", G_CALLBACK (activate_action) },
	{"ExportPDF", NULL, "_PDF", NULL, "create a pdf file", G_CALLBACK (activate_action) },
	{"ExportSVG", NULL, "_SVG", NULL, "create a svg file", G_CALLBACK (activate_action) },
	{"Tools", NULL, N_("_Tools")},
	{"ComputeTotalCharge", NULL, N_("_Compute total charge"), NULL, "Compute total charge", G_CALLBACK (activate_action) },
	{"ComputeChargeForResidues", NULL, N_("Compute charge for _residues"), NULL, "Compute charge for residues", G_CALLBACK (activate_action) },
	{"ComputeChargeOfSelectedsAtoms", NULL, N_("Compute charge of _selected atoms"), NULL, "Compute charge of selected atoms", G_CALLBACK (activate_action) },
	{"ComputeDipoleFormCharges", NULL, N_("_Compute dipole from charges"), NULL, "Compute dipole from charges", G_CALLBACK (activate_action) },
	{"ComputeIsotopeDistribution", NULL, N_("_Isotope distribution calculator"), NULL, "Isotope distribution calculator", G_CALLBACK (activate_action) },
	{"ComputeVolumeMolecule", NULL, N_("_Compute volume of molecule"), NULL, "Compute volume of molecule", G_CALLBACK (activate_action) },

	{"ScreenCapture", NULL, N_("Screen Ca_pture")},
	{"ScreenCaptureJPG", NULL, N_("_JPG format"), NULL, "create a JPEG file", G_CALLBACK (activate_action) },
	{"ScreenCapturePPM", NULL, N_("_PPM format"), NULL, "create a PPM file", G_CALLBACK (activate_action) },
	{"ScreenCaptureBMP", NULL, N_("_BMP format"), NULL, "create a BMP file", G_CALLBACK (activate_action) },
	{"ScreenCapturePNG", NULL, N_("_PNG format"), NULL, "create a PNG file", G_CALLBACK (activate_action) },
	{"ScreenCaptureTIF", NULL, N_("_TIF format"), NULL, "create a TIF file", G_CALLBACK (activate_action) },
	{"ScreenCapturePS", NULL, N_("_PS format"), NULL, "create a PS file", G_CALLBACK (activate_action) },
	{"ScreenCaptureCilpBoard", NULL, N_("_Copy to clipboard"), NULL, "copy to clipboard", G_CALLBACK (activate_action) },

	{"MolecularMechanics", NULL, N_("_Amber potential")},
	{"MolecularMechanicsEnergy", NULL, N_("_Energy"), NULL, "compute the energy using the MM method", G_CALLBACK (activate_action) },
	{"MolecularMechanicsOptimization", NULL, N_("_Optimization"), NULL, "optimize the geometry using the MM method", G_CALLBACK (activate_action) },
	{"MolecularMechanicsDynamics", NULL, N_("Molecular _Dynamics"), NULL, "Molecular dynamics using the MM method", G_CALLBACK (activate_action) },
	{"MolecularMechanicsDynamicsConfo", NULL, N_("Molecular _Dynamics Conformational search"), NULL, "Molecular dynamics conformational search using the MM method", G_CALLBACK (activate_action) },

	{"SemiEmpirical", NULL, N_("(Semi-)_empirical")},
	{"SemiEmpiricalEnergyFireFlyAM1", NULL, N_("FireFly AM1 _Energy"), NULL, "compute the energy using the AM1 method from FireFly", G_CALLBACK (activate_action) },
	{"SemiEmpiricalOptimizationFireFlyAM1", NULL, N_("FireFly AM1 _Optimization"), NULL, "optimize the geometry using the AM1 method from FireFly", G_CALLBACK (activate_action) },

	{"SemiEmpiricalEnergyMopac", NULL, N_("Mopac _Energy"), NULL, "compute the energy using Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalOptimizationMopac", NULL, N_("Mopac _Optimization"), NULL, "optimize the geometry using Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalOptimizationMopacSparkle", NULL, N_("Mopac _Sparkle Optimization"), NULL, "optimize the geometry of a lanthanide complex using Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalESPMopac", NULL, N_("Mopac _ESP charges"), NULL, "ESP Charge using Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalScanMopac", NULL, N_("Mopac _Reaction path"), NULL, "Mopac Scan calculation", G_CALLBACK (activate_action) },

	{"SemiEmpiricalEnergyMopacPM6DH2", NULL, N_("Mopac PM6-DH2 _Energy"), NULL, "compute the energy using the PM6-DH2 method from Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalEnergyMopacPM6DH+", NULL, N_("Mopac PM6-DH+ _Energy"), NULL, "compute the energy using the PM6-DH+ method from Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalOptimizationMopacPM6DH2", NULL, N_("Mopac PM6-DH2 _Optimization"), NULL, "optimize the geometry using the PM6-DH2 method from Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalOptimizationMopacPM6DH+", NULL, N_("Mopac PM6-DH+ _Optimization"), NULL, "optimize the geometry using the PM6-DH+ method from Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalESPMopacPM6DH2", NULL, N_("Mopac PM6-DH2 _ESP charges"), NULL, "ESP Charge using the PM6-DH2 method from Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalESPMopacPM6DH+", NULL, N_("Mopac PM6-DH+ _ESP charges"), NULL, "ESP Charge using the PM6-DH+ method from Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalScanMopacPM6DH2", NULL, N_("Mopac PM6-DH2 _Reaction path"), NULL, "Reaction path using the PM6-DH2 method from Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalScanMopacPM6DH+", NULL, N_("Mopac PM6-DH+ _Reaction path"), NULL, "Reaction path using the PM6-DH+ method from Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalEnergyMopacPM6", NULL, N_("Mopac PM6 _Energy"), NULL, "compute the energy using the PM6 method from Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalOptimizationMopacPM6", NULL, N_("Mopac PM6 _Optimization"), NULL, "optimize the geometry using the PM6 method from Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalESPMopacPM6", NULL, N_("Mopac PM6 _ESP charges"), NULL, "ESP Charge using the PM6 method from Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalScanMopacPM6", NULL, N_("Mopac PM6 _Reaction path"), NULL, "Reaction path using the PM6 method from Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalEnergyMopacAM1", NULL, N_("Mopac AM1 _Energy"), NULL, "compute the energy using the AM1 method from Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalOptimizationMopacAM1", NULL, N_("Mopac AM1 _Optimization"), NULL, "optimize the geometry using the AM1 method from Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalESPMopacAM1", NULL, N_("Mopac AM1 _ESP charges"), NULL, "ESP Charge using the AM1 method from Mopac", G_CALLBACK (activate_action) },
	{"SemiEmpiricalScanMopacAM1", NULL, N_("Mopac AM1 _Reaction path"), NULL, "Reaction path using the AM1 method from Mopac", G_CALLBACK (activate_action) },

	{"SemiEmpiricalEnergyOrca", NULL, N_("Orca _Energy"), NULL, "compute the energy using Orca", G_CALLBACK (activate_action) },
	{"SemiEmpiricalOptimizationOrca", NULL, N_("Orca _Optimization"), NULL, "optimize the geometry using Orca", G_CALLBACK (activate_action) },

	{"SemiEmpiricalEnergyOpenBabel", NULL, N_("OpenBabel _Energy"), NULL, "compute the energy using OpenBabel", G_CALLBACK (activate_action) },
	{"SemiEmpiricalOptimizationOpenBabel", NULL, N_("OpenBabel _Optimization"), NULL, "optimize the geometry using OpenBabel", G_CALLBACK (activate_action) },

	{"SemiEmpiricalEnergyGeneric", NULL, N_("Generic _Energy"), NULL, "compute the energy using your own program", G_CALLBACK (activate_action) },
	{"SemiEmpiricalOptimizationGeneric", NULL, N_("Generic _Optimization"), NULL, "optimize the geometry using your own program", G_CALLBACK (activate_action) },

	{"SemiEmpiricalMD", NULL, N_("Molecular _Dynamics"), NULL, "Molecular dynamics using a semi-empirical method", G_CALLBACK (activate_action) },
	{"SemiEmpiricalMDConfo", NULL, N_("Molecular _Dynamics Conformational search"), NULL, "Molecular dynamics conformational search using a semi-empirical  method", G_CALLBACK (activate_action) },


	{"View", NULL, N_("_View")},

	{"Close", GABEDIT_STOCK_CLOSE, N_("_Close"), NULL, "Close", G_CALLBACK (activate_action) },
};
static guint numberOfGtkActionEntries = G_N_ELEMENTS (gtkActionEntries);
/********************************************************************************/
/* XML description of the menus for the test app.  The parser understands
 * a subset of the Bonobo UI XML format, and uses GMarkup for parsing */
static const gchar *uiMenuInfo =
"  <popup name=\"MenuGeom\">\n"
"    <separator name=\"sepMenuPopRead\" />\n"
"    <menu name=\"Read\" action=\"Read\">\n"
"      <menuitem name=\"ReadAuto\" action=\"ReadAuto\" />\n"
"      <separator name=\"sepMenuReadAuto\" />\n"
"      <menuitem name=\"ReadGabedit\" action=\"ReadGabedit\" />\n"
"      <menuitem name=\"ReadXYZ\" action=\"ReadXYZ\" />\n"
"      <menuitem name=\"ReadMol2\" action=\"ReadMol2\" />\n"
"      <menuitem name=\"ReadMol\" action=\"ReadMol\" />\n"
"      <menuitem name=\"ReadTinker\" action=\"ReadTinker\" />\n"
"      <menuitem name=\"ReadPDB\" action=\"ReadPDB\" />\n"
"      <menuitem name=\"ReadHyperchem\" action=\"ReadHyperchem\" />\n"
"      <menuitem name=\"ReadPOSCAR\" action=\"ReadPOSCAR\" />\n"
"      <menuitem name=\"ReadWFX\" action=\"ReadWFX\" />\n"
"      <menuitem name=\"ReadCIF\" action=\"ReadCIF\" />\n"
"      <menuitem name=\"ReadCIFNoSym\" action=\"ReadCIFNoSym\" />\n"
"      <menuitem name=\"ReadAIMAll\" action=\"ReadAIMAll\" />\n"
"      <separator name=\"sepMenuReadFireFly\" />\n"
"      <menu name=\"FireFly\" action=\"FireFly\">\n"
"        <menuitem name=\"ReadFireFlyFirst\" action=\"ReadFireFlyFirst\" />\n"
"        <menuitem name=\"ReadFireFlyLast\" action=\"ReadFireFlyLast\" />\n"
"      </menu>\n"
"      <separator name=\"sepMenuReadDalton\" />\n"
"      <menu name=\"Dalton\" action=\"Dalton\">\n"
"        <menuitem name=\"ReadDaltonFirst\" action=\"ReadDaltonFirst\" />\n"
"        <menuitem name=\"ReadDaltonLast\" action=\"ReadDaltonLast\" />\n"
"      </menu>\n"
"      <separator name=\"sepMenuReadGamess\" />\n"
"      <menu name=\"Gamess\" action=\"Gamess\">\n"
"        <menuitem name=\"ReadGamessFirst\" action=\"ReadGamessFirst\" />\n"
"        <menuitem name=\"ReadGamessLast\" action=\"ReadGamessLast\" />\n"
"      </menu>\n"
"      <separator name=\"sepMenuReadGaussian\" />\n"
"      <menu name=\"Gaussian\" action=\"Gaussian\">\n"
"        <menuitem name=\"ReadGaussianInput\" action=\"ReadGaussianInput\" />\n"
"        <menuitem name=\"ReadGaussianFirst\" action=\"ReadGaussianFirst\" />\n"
"        <menuitem name=\"ReadGaussianLast\" action=\"ReadGaussianLast\" />\n"
"        <menuitem name=\"ReadGaussianFChk\" action=\"ReadGaussianFChk\" />\n"
"        <menuitem name=\"ReadGaussianZMat\" action=\"ReadGaussianZMat\" />\n"
"      </menu>\n"
"      <separator name=\"sepMenuReadMolcas\" />\n"
"      <menu name=\"Molcas\" action=\"Molcas\">\n"
"        <menuitem name=\"ReadMolcasInput\" action=\"ReadMolcasInput\" />\n"
"        <menuitem name=\"ReadMolcasFirst\" action=\"ReadMolcasFirst\" />\n"
"        <menuitem name=\"ReadMolcasLast\" action=\"ReadMolcasLast\" />\n"
"      </menu>\n"
"      <separator name=\"sepMenuReadMolpro\" />\n"
"      <menu name=\"Molpro\" action=\"Molpro\">\n"
"        <menuitem name=\"ReadMolproInput\" action=\"ReadMolproInput\" />\n"
"        <menuitem name=\"ReadMolproFirst\" action=\"ReadMolproFirst\" />\n"
"        <menuitem name=\"ReadMolproLast\" action=\"ReadMolproLast\" />\n"
"      </menu>\n"
"      <separator name=\"sepMenuReadMopac\" />\n"
"      <menu name=\"Mopac\" action=\"Mopac\">\n"
"        <menuitem name=\"ReadMopacInput\" action=\"ReadMopacInput\" />\n"
"        <menuitem name=\"ReadMopacFirst\" action=\"ReadMopacFirst\" />\n"
"        <menuitem name=\"ReadMopacLast\" action=\"ReadMopacLast\" />\n"
"        <menuitem name=\"ReadMopacAux\" action=\"ReadMopacAux\" />\n"
"        <menuitem name=\"ReadMopacScan\" action=\"ReadMopacScan\" />\n"
"        <menuitem name=\"ReadMopacIRC\" action=\"ReadMopacIRC\" />\n"
"        <menuitem name=\"ReadMopacZMat\" action=\"ReadMopacZMat\" />\n"
"      </menu>\n"
"      <separator name=\"sepMenuReadMPQC\" />\n"
"      <menu name=\"MPQC\" action=\"MPQC\">\n"
"        <menuitem name=\"ReadMPQCInput\" action=\"ReadMPQCInput\" />\n"
"        <menuitem name=\"ReadMPQCFirst\" action=\"ReadMPQCFirst\" />\n"
"        <menuitem name=\"ReadMPQCLast\" action=\"ReadMPQCLast\" />\n"
"      </menu>\n"
"      <separator name=\"sepMenuReadOrca\" />\n"
"      <menu name=\"Orca\" action=\"Orca\">\n"
"        <menuitem name=\"ReadOrcaFirst\" action=\"ReadOrcaFirst\" />\n"
"        <menuitem name=\"ReadOrcaLast\" action=\"ReadOrcaLast\" />\n"
"      </menu>\n"

"      <separator name=\"sepMenuReadVasp\" />\n"
"      <menu name=\"VASP\" action=\"VASP\">\n"
"        <menuitem name=\"ReadVaspFirst\" action=\"ReadVaspFirst\" />\n"
"        <menuitem name=\"ReadVaspLast\" action=\"ReadVaspLast\" />\n"
"        <menuitem name=\"ReadVaspXMLFirst\" action=\"ReadVaspXMLFirst\" />\n"
"        <menuitem name=\"ReadVaspXMLLast\" action=\"ReadVaspXMLLast\" />\n"
"      </menu>\n"

"      <separator name=\"sepMenuReadQChem\" />\n"
"      <menu name=\"QChem\" action=\"QChem\">\n"
"        <menuitem name=\"ReadQChemFirst\" action=\"ReadQChemFirst\" />\n"
"        <menuitem name=\"ReadQChemLast\" action=\"ReadQChemLast\" />\n"
"      </menu>\n"
"      <separator name=\"sepMenuReadNWChem\" />\n"
"      <menu name=\"NWChem\" action=\"NWChem\">\n"
"        <menuitem name=\"ReadNWChemFirst\" action=\"ReadNWChemFirst\" />\n"
"        <menuitem name=\"ReadNWChemLast\" action=\"ReadNWChemLast\" />\n"
"      </menu>\n"
"      <separator name=\"sepMenuReadPsicode\" />\n"
"      <menu name=\"Psicode\" action=\"Psicode\">\n"
"        <menuitem name=\"ReadPsicodeFirst\" action=\"ReadPsicodeFirst\" />\n"
"        <menuitem name=\"ReadPsicodeLast\" action=\"ReadPsicodeLast\" />\n"
"      </menu>\n"
"      <separator name=\"sepMenuReadTurbomole\" />\n"
"      <menu name=\"Turbomole\" action=\"Turbomole\">\n"
"        <menuitem name=\"ReadTurbomoleFirst\" action=\"ReadTurbomoleFirst\" />\n"
"        <menuitem name=\"ReadTurbomoleLast\" action=\"ReadTurbomoleLast\" />\n"
"      </menu>\n"
"      <separator name=\"sepMenuReadOpenBabel\" />\n"
"      <menuitem name=\"ReadUsingOpenBabel\" action=\"ReadUsingOpenBabel\" />\n"
"      <separator name=\"sepMenuReadGeomConv\" />\n"
"      <menu name=\"ReadGeomConv\" action=\"ReadGeomConv\">\n"
"        <menuitem name=\"ReadGeomConvFireFly\" action=\"ReadGeomConvFireFly\" />\n"
"        <menuitem name=\"ReadGeomConvDalton\" action=\"ReadGeomConvDalton\" />\n"
"        <menuitem name=\"ReadGeomConvGamess\" action=\"ReadGeomConvGamess\" />\n"
"        <menuitem name=\"ReadGeomConvGaussian\" action=\"ReadGeomConvGaussian\" />\n"
"        <menuitem name=\"ReadGeomConvMolpro\" action=\"ReadGeomConvMolpro\" />\n"
"        <menuitem name=\"ReadGeomConvMopac\" action=\"ReadGeomConvMopac\" />\n"
"        <menuitem name=\"ReadGeomConvMPQC\" action=\"ReadGeomConvMPQC\" />\n"
"        <menuitem name=\"ReadGeomConvOrca\" action=\"ReadGeomConvOrca\" />\n"
"        <menuitem name=\"ReadGeomConvVasp\" action=\"ReadGeomConvVasp\" />\n"
"        <menuitem name=\"ReadGeomConvVaspXML\" action=\"ReadGeomConvVaspXML\" />\n"
"        <menuitem name=\"ReadGeomConvNWChem\" action=\"ReadGeomConvNWChem\" />\n"
"        <menuitem name=\"ReadGeomConvPsicode\" action=\"ReadGeomConvPsicode\" />\n"
"        <menuitem name=\"ReadGeomConvQChem\" action=\"ReadGeomConvQChem\" />\n"
"        <menuitem name=\"ReadGeomConvGabedit\" action=\"ReadGeomConvGabedit\" />\n"
"        <menuitem name=\"ReadGeomConvMolden\" action=\"ReadGeomConvMolden\" />\n"
"        <menuitem name=\"ReadGeomConvXYZ\" action=\"ReadGeomConvXYZ\" />\n"
"        <menuitem name=\"ReadGeomIRCGamess\" action=\"ReadGeomIRCGamess\" />\n"
"        <menuitem name=\"ReadGeomIRCFireFly\" action=\"ReadGeomIRCFireFly\" />\n"
"      </menu>\n"
"    </menu>\n"
"      <menu name=\"Edit\" action=\"Edit\">\n"
"        <menuitem name=\"EditResetAllConnections\" action=\"EditResetAllConnections\" />\n"
"        <menuitem name=\"EditResetMultipleConnections\" action=\"EditResetMultipleConnections\" />\n"
"        <menuitem name=\"EditResetConnectionsBetweenSelectedAndNotSelectedAtoms\" action=\"EditResetConnectionsBetweenSelectedAndNotSelectedAtoms\" />\n"
"        <menuitem name=\"EditResetConnectionsBetweenSelectedAtoms\" action=\"EditResetConnectionsBetweenSelectedAtoms\" />\n"
"        <separator name=\"sepMenuEditCopySelectedAtoms\" />\n"
"        <menuitem name=\"EditCopySelectedAtoms\" action=\"EditCopySelectedAtoms\" />\n"
"        <separator name=\"sepMenuEditDeleteMolecule\" />\n"
"        <menuitem name=\"EditDeleteHydrogenAtoms\" action=\"EditDeleteHydrogenAtoms\" />\n"
"        <menuitem name=\"EditDeleteSelectedAtoms\" action=\"EditDeleteSelectedAtoms\" />\n"
"        <menuitem name=\"EditDeleteMolecule\" action=\"EditDeleteMolecule\" />\n"
"        <separator name=\"sepMenuEditMove\" />\n"
"        <menuitem name=\"EditMoveCenterOfSelectedAtomsToOrigin\" action=\"EditMoveCenterOfSelectedAtomsToOrigin\" />\n"
#ifdef EXPERIMENTAL
"        <menuitem name=\"EditAlignPrincipalAxesOfSelectedAtomsToXYZ\" action=\"EditAlignPrincipalAxesOfSelectedAtomsToXYZ\" />\n"
"        <menuitem name=\"EditAlignSelectedAndNotSelectedAtoms\" action=\"EditAlignSelectedAndNotSelectedAtoms\" />\n"
#endif
"        <separator name=\"sepMenuEditOpenGeometryEditor\" />\n"
"        <menuitem name=\"EditOpenGeometryEditor\" action=\"EditOpenGeometryEditor\" />\n"
"        <separator name=\"sepMenuPersonalFragments\" />\n"
"        <menu name=\"PersonalFragments\" action=\"PersonalFragments\">\n"
"          <menuitem name=\"PersonalFragmentsNewGroup\" action=\"PersonalFragmentsNewGroup\" />\n"
"          <menuitem name=\"PersonalFragmentsDeleteGroup\" action=\"PersonalFragmentsDeleteGroup\" />\n"
"          <separator name=\"sepMenuPersonalFragmentsAddMolecule\" />\n"
"          <menuitem name=\"PersonalFragmentsAddMolecule\" action=\"PersonalFragmentsAddMolecule\" />\n"
"          <menuitem name=\"PersonalFragmentsRemoveFragment\" action=\"PersonalFragmentsRemoveFragment\" />\n"
"        </menu>\n"
"      </menu>\n"
"      <menu name=\"Selection\" action=\"Selection\">\n"
"        <menuitem name=\"EditSelectAll\" action=\"EditSelectAll\" />\n"
"        <menuitem name=\"EditInvertSelection\" action=\"EditInvertSelection\" />\n"
"        <menuitem name=\"EditUnSelectAll\" action=\"EditUnSelectAll\" />\n"
"        <separator name=\"sepMenuSelectHighAtoms\" />\n"
"        <menuitem name=\"EditSelectHighAtoms\" action=\"EditSelectHighAtoms\" />\n"
"        <menuitem name=\"EditSelectMediumAtoms\" action=\"EditSelectMediumAtoms\" />\n"
"        <menuitem name=\"EditSelectLowAtoms\" action=\"EditSelectLowAtoms\" />\n"
"        <separator name=\"sepMenuSelectFixedAtoms\" />\n"
"        <menuitem name=\"EditSelectFixedAtoms\" action=\"EditSelectFixedAtoms\" />\n"
"        <menuitem name=\"EditSelectVariableAtoms\" action=\"EditSelectVariableAtoms\" />\n"
"        <menuitem name=\"EditSelectFirstResidue\" action=\"EditSelectFirstResidue\" />\n"
"        <menuitem name=\"EditSelectLastResidue\" action=\"EditSelectLastResidue\" />\n"
"        <menuitem name=\"EditSelectResidueByNumber\" action=\"EditSelectResidueByNumber\" />\n"
"        <menuitem name=\"EditSelectResidueByName\" action=\"EditSelectResidueByName\" />\n"
"        <menuitem name=\"EditSelectAtomsByMMType\" action=\"EditSelectAtomsByMMType\" />\n"
"        <menuitem name=\"EditSelectAtomsByPDBType\" action=\"EditSelectAtomsByPDBType\" />\n"
"        <menuitem name=\"EditSelectAtomsBySymbol\" action=\"EditSelectAtomsBySymbol\" />\n"
"        <menuitem name=\"EditSelectAtomsBySphere\" action=\"EditSelectAtomsBySphere\" />\n"
"        <menuitem name=\"EditSelectAtomsByPositiveCharges\" action=\"EditSelectAtomsByPositiveCharges\" />\n"
"        <menuitem name=\"EditSelectAtomsByNegativeCharges\" action=\"EditSelectAtomsByNegativeCharges\" />\n"
"        <menuitem name=\"EditSelectAtomsMultiple\" action=\"EditSelectAtomsMultiple\" />\n"
"      </menu>\n"
"      <menu name=\"SaveAs\" action=\"SaveAs\">\n"
"        <menuitem name=\"SaveAsGabedit\" action=\"SaveAsGabedit\" />\n"
"        <menuitem name=\"SaveAsXYZ\" action=\"SaveAsXYZ\" />\n"
"        <menuitem name=\"SaveAsMol2\" action=\"SaveAsMol2\" />\n"
"        <menuitem name=\"SaveAsMol\" action=\"SaveAsMol\" />\n"
"        <menuitem name=\"SaveAsTinker\" action=\"SaveAsTinker\" />\n"
"        <menuitem name=\"SaveAsPDB\" action=\"SaveAsPDB\" />\n"
"        <menuitem name=\"SaveAsPOSCARCartn\" action=\"SaveAsPOSCARCartn\" />\n"
"        <menuitem name=\"SaveAsPOSCARDirect\" action=\"SaveAsPOSCARDirect\" />\n"
"        <menuitem name=\"SaveAsCIFAllAtoms\" action=\"SaveAsCIFAllAtoms\" />\n"
"        <menuitem name=\"SaveAsCIF\" action=\"SaveAsCIF\" />\n"
"        <menuitem name=\"SaveAsHyperchem\" action=\"SaveAsHyperchem\" />\n"
"        <menuitem name=\"SaveAsCChemI\" action=\"SaveAsCChemI\" />\n"
"        <separator name=\"sepMenuSaveAsZmat\" />\n"
"        <menuitem name=\"SaveAsMopacZMat\" action=\"SaveAsMopacZMat\" />\n"
"        <menuitem name=\"SaveAsGaussianZMat\" action=\"SaveAsGaussianZMat\" />\n"
"        <separator name=\"sepMenuSaveOpenBabel\" />\n"
"        <menuitem name=\"SaveUsingOpenBabel\" action=\"SaveUsingOpenBabel\" />\n"
"      </menu>\n"
"      <separator name=\"sepMenuAdd\" />\n"
"      <menu name=\"Add\" action=\"Add\">\n"
"        <menuitem name=\"InsertAFragment\" action=\"InsertAFragment\" />\n"
"          <separator name=\"sepMenuAddEnd\" />\n"
"          <menuitem name=\"AddHydrogens\" action=\"AddHydrogens\" />\n"
"          <menuitem name=\"AddMaxHydrogens\" action=\"AddMaxHydrogens\" />\n"
"          <menuitem name=\"AddOneHydrogen\" action=\"AddOneHydrogen\" />\n"
"          <separator name=\"sepMenuAddTpl\" />\n"
"          <menuitem name=\"AddHydrogensTpl\" action=\"AddHydrogensTpl\" />\n"
"      </menu>\n"
"      <menu name=\"Build\" action=\"Build\">\n"
"        <menuitem name=\"BuildLinearMolecule\" action=\"BuildLinearMolecule\" />\n"
"        <separator name=\"sepBuildRingMolecule\" />\n"
"        <menuitem name=\"BuildRingMolecule\" action=\"BuildRingMolecule\" />\n"
"        <separator name=\"sepBuildBuildMoleculeWithSymmetry\" />\n"
"        <menuitem name=\"BuildMoleculeWithSymmetry\" action=\"BuildMoleculeWithSymmetry\" />\n"
"        <separator name=\"sepBuildBuildPolyPeptide\" />\n"
"        <menuitem name=\"BuildPolyPeptide\" action=\"BuildPolyPeptide\" />\n"
"        <separator name=\"sepBuildBuildPolyNucleicAcid\" />\n"
"        <menuitem name=\"BuildPolyNucleicAcid\" action=\"BuildPolyNucleicAcid\" />\n"
"        <separator name=\"sepBuildBuildPolySaccharide\" />\n"
"        <menuitem name=\"BuildPolySaccharide\" action=\"BuildPolySaccharide\" />\n"
"        <separator name=\"sepBuildBuildNanoTube\" />\n"
"        <menuitem name=\"BuildNanoTube\" action=\"BuildNanoTube\" />\n"
"      </menu>\n"
#ifdef DRAWGEOMGL
"    <separator name=\"sepMenuCrystallography\" />\n"
"      <menu name=\"Crystallography\" action=\"Crystallography\">\n"
"        <menuitem name=\"PrototypeCrystal\" action=\"PrototypeCrystal\" />\n"
"        <separator name=\"sepPrototypeCrystal\" />\n"
"        <menuitem name=\"StandardizeCellPrimitive\" action=\"StandardizeCellPrimitive\" />\n"
"        <menuitem name=\"StandardizeCellConv\" action=\"StandardizeCellConv\" />\n"
"        <menuitem name=\"WrapAtomsToCell\" action=\"WrapAtomsToCell\" />\n"
"        <separator name=\"sepStandardizeCell\" />\n"
"      <menu name=\"Reduction\" action=\"Reduction\">\n"
"        <menuitem name=\"ReductionPrimitive\" action=\"ReductionPrimitive\" />\n"
"        <menuitem name=\"ReductionNiggli\" action=\"ReductionNiggli\" />\n"
"        <menuitem name=\"ReductionDelaunay\" action=\"ReductionDelaunay\" />\n"
"      </menu>\n"
"        <separator name=\"sepreduction\" />\n"
"        <menuitem name=\"ComputeSpaceGroupSym\" action=\"ComputeSpaceGroupSym\" />\n"
"        <menuitem name=\"ComputeSymmetryInfo\" action=\"ComputeSymmetryInfo\" />\n"
"        <menuitem name=\"ComputeKPointsPath\" action=\"ComputeKPointsPath\" />\n"
"        <separator name=\"sepComputeCrystal\" />\n"
"      <menu name=\"BuildCrystalsDeriv\" action=\"BuildCrystalsDeriv\">\n"
"        <menuitem name=\"BuildSuperCellSimple\" action=\"BuildSuperCellSimple\" />\n"
"        <menuitem name=\"BuildSuperCell\" action=\"BuildSuperCell\" />\n"
"        <menuitem name=\"BuildSlab\" action=\"BuildSlab\" />\n"
"        <menuitem name=\"BuildWulff\" action=\"BuildWulff\" />\n"
"        <separator name=\"sepBuildCrystal\" />\n"
"        <menuitem name=\"BuildCrystalGen\" action=\"BuildCrystalGen\" />\n"
"      </menu>\n"
"        <separator name=\"sepBuildCrystalAll\" />\n"
"        <menuitem name=\"ComputeVolume\" action=\"ComputeVolume\" />\n"
"        <separator name=\"sepComputeVolume\" />\n"
"        <menuitem name=\"SetSymPrec\" action=\"SetSymPrec\" />\n"
"        <separator name=\"sepSetSymPrec\" />\n"
"        <menuitem name=\"HelpCrystal\" action=\"HelpCrystal\" />\n"
"        <separator name=\"sepHelpCrystal\" />\n"
"      </menu>\n"
#endif
"    <separator name=\"sepMenuOperations\" />\n"
"    <menu name=\"Operations\" action=\"Operations\">\n"
"      <menuitem name=\"OperationsTranslate\" action=\"OperationsTranslate\" />\n"
"      <menuitem name=\"OperationsRotation\" action=\"OperationsRotation\" />\n"
"      <menuitem name=\"OperationsRotationZ\" action=\"OperationsRotationZ\" />\n"
"      <menuitem name=\"OperationsZoom\" action=\"OperationsZoom\" />\n"
"      <menuitem name=\"OperationsScaleStick\" action=\"OperationsScaleStick\" />\n"
"      <menuitem name=\"OperationsScaleBall\" action=\"OperationsScaleBall\" />\n"
"      <menuitem name=\"OperationsScaleDipole\" action=\"OperationsScaleDipole\" />\n"
"      <separator name=\"sepMenuSelectionOfAtoms\" />\n"
"      <menuitem name=\"OperationsSelectionOfAtoms\" action=\"OperationsSelectionOfAtoms\" />\n"
"      <menuitem name=\"OperationsDeleteObjects\" action=\"OperationsDeleteObjects\" />\n"
"      <menuitem name=\"OperationsMoveAtoms\" action=\"OperationsMoveAtoms\" />\n"
"      <menuitem name=\"OperationsRotationAtoms\" action=\"OperationsRotationAtoms\" />\n"
"      <menuitem name=\"OperationsRotationZAtoms\" action=\"OperationsRotationZAtoms\" />\n"
"      <menuitem name=\"OperationsEditObjects\" action=\"OperationsEditObjects\" />\n"
"      <menuitem name=\"OperationsInsertFrag\" action=\"OperationsInsertFrag\" />\n"
"      <separator name=\"sepMenuMeasure\" />\n"
"      <menuitem name=\"OperationsMeasure\" action=\"OperationsMeasure\" />\n"
"    </menu>\n"
"    <menu name=\"Labels\" action=\"Labels\">\n"
"      <menuitem name=\"LabelsNothing\" action=\"LabelsNothing\" />\n"
"      <menuitem name=\"LabelsSymbols\" action=\"LabelsSymbols\" />\n"
"      <menuitem name=\"LabelsNumbers\" action=\"LabelsNumbers\" />\n"
"      <menuitem name=\"LabelsMMTypes\" action=\"LabelsMMTypes\" />\n"
"      <menuitem name=\"LabelsPDBTypes\" action=\"LabelsPDBTypes\" />\n"
"      <menuitem name=\"LabelsLayers\" action=\"LabelsLayers\" />\n"
"      <menuitem name=\"LabelsSymbolsAndNumbers\" action=\"LabelsSymbolsAndNumbers\" />\n"
"      <menuitem name=\"LabelsCharges\" action=\"LabelsCharges\" />\n"
"      <menuitem name=\"LabelsSymbolsAndCharges\" action=\"LabelsSymbolsAndCharges\" />\n"
"      <menuitem name=\"LabelsNumbersAndCharges\" action=\"LabelsNumbersAndCharges\" />\n"
"      <menuitem name=\"LabelsRedidues\" action=\"LabelsRedidues\" />\n"
"      <menuitem name=\"LabelsCoordinates\" action=\"LabelsCoordinates\" />\n"
"      <separator name=\"sepMenuLabelsDistances\" />\n"
"      <menuitem name=\"LabelsDistances\" action=\"LabelsDistances\" />\n"
"      <menuitem name=\"LabelsDipole\" action=\"LabelsDipole\" />\n"
#ifdef DRAWGEOMGL
"      <separator name=\"sepMenuLabelsOrtho\" />\n"
"      <menuitem name=\"RenderLabelsOrtho\" action=\"RenderLabelsOrtho\" />\n"
#endif
"    </menu>\n"
"    <menu name=\"Render\" action=\"Render\">\n"
"      <menuitem name=\"RenderGeometryStick\" action=\"RenderGeometryStick\" />\n"
"      <menuitem name=\"RenderGeometryBallAndStick\" action=\"RenderGeometryBallAndStick\" />\n"
#ifdef DRAWGEOMGL
"      <menuitem name=\"RenderGeometrySpaceFill\" action=\"RenderGeometrySpaceFill\" />\n"
#endif
"      <separator name=\"sepMenuPerspective\" />\n"
"      <menuitem name=\"RenderPerspective\" action=\"RenderPerspective\" />\n"
#ifndef DRAWGEOMGL
"      <menuitem name=\"RenderLighting\" action=\"RenderLighting\" />\n"
"      <menuitem name=\"RenderOrtep\" action=\"RenderOrtep\" />\n"
"      <menuitem name=\"RenderCartoon\" action=\"RenderCartoon\" />\n"
"      <menuitem name=\"RenderShad\" action=\"RenderShad\" />\n"
#endif
"      <separator name=\"sepMenuShowDipole\" />\n"
#ifdef DRAWGEOMGL
"      <menuitem name=\"RenderShowAxes\" action=\"RenderShowAxes\" />\n"
"      <menuitem name=\"RenderShowBox\" action=\"RenderShowBox\" />\n"
#endif
"      <menuitem name=\"RenderShowDipole\" action=\"RenderShowDipole\" />\n"
"      <menuitem name=\"RenderShowHydrogenBonds\" action=\"RenderShowHydrogenBonds\" />\n"
"      <menuitem name=\"RenderShowDoubleTripleBonds\" action=\"RenderShowDoubleTripleBonds\" />\n"
"      <separator name=\"sepMenuDefault\" />\n"
"      <menu name=\"RenderDefault\" action=\"RenderDefault\">\n"
"         <menuitem name=\"RenderDefaultStick\" action=\"RenderDefaultStick\" />\n"
"         <menuitem name=\"RenderDefaultBall\" action=\"RenderDefaultBall\" />\n"
"         <menuitem name=\"RenderDefaultZoom\" action=\"RenderDefaultZoom\" />\n"
"         <menuitem name=\"RenderDefaultDipole\" action=\"RenderDefaultDipole\" />\n"
"         <menuitem name=\"RenderDefaultCenter\" action=\"RenderDefaultCenter\" />\n"
"         <menuitem name=\"RenderDefaultAll\" action=\"RenderDefaultAll\" />\n"
"      </menu>\n"
"      <menu name=\"RenderBackgroundColor\" action=\"RenderBackgroundColor\">\n"
"         <menuitem name=\"RenderBackgroundColorBlack\" action=\"RenderBackgroundColorBlack\" />\n"
"         <menuitem name=\"RenderBackgroundColorOther\" action=\"RenderBackgroundColorOther\" />\n"
"      </menu>\n"
#ifdef DRAWGEOMGL
"       <separator name=\"sepMenuRenderLight\" />\n"
"       <menu name=\"RenderLight\" action = \"RenderLight\">\n"
"           <menuitem name=\"RenderLightOnOff1\" action=\"RenderLightOnOff1\" />\n"
"           <menuitem name=\"RenderLightOnOff2\" action=\"RenderLightOnOff2\" />\n"
"           <menuitem name=\"RenderLightOnOff3\" action=\"RenderLightOnOff3\" />\n"
"       </menu>\n"
#endif
"      <separator name=\"sepMenuShowHide\" />\n"
"      <menuitem name=\"RenderHideHydrogenAtoms\" action=\"RenderHideHydrogenAtoms\" />\n"
"      <menuitem name=\"RenderHideNotSelectedAtoms\" action=\"RenderHideNotSelectedAtoms\" />\n"
"      <menuitem name=\"RenderHideSelectedAtoms\" action=\"RenderHideSelectedAtoms\" />\n"
"      <menuitem name=\"RenderShowHydrogenAtoms\" action=\"RenderShowHydrogenAtoms\" />\n"
"      <menuitem name=\"RenderShowAllAtoms\" action=\"RenderShowAllAtoms\" />\n"
"    </menu>\n"
"    <separator name=\"sepMenuSymmetry\" />\n"
"    <menu name=\"Symmetry\" action=\"Symmetry\">\n"
"      <menuitem name=\"SymmetryRotationalConstantes\" action=\"SymmetryRotationalConstantes\" />\n"
"      <menuitem name=\"SymmetryGroupSymmetry\" action=\"SymmetryGroupSymmetry\" />\n"
"      <menuitem name=\"SymmetryAbelianGroup\" action=\"SymmetryAbelianGroup\" />\n"
"      <separator name=\"sepMenuSymmetryGroupSymmetrize\" />\n"
"      <menuitem name=\"SymmetryGroupSymmetrize\" action=\"SymmetryGroupSymmetrize\" />\n"
"      <separator name=\"sepMenuSymmetrySetTolerance\" />\n"
"      <menuitem name=\"SymmetrySetTolerance\" action=\"SymmetrySetTolerance\" />\n"
"    </menu>\n"
"    <separator name=\"sepMenuSet\" />\n"
"    <menu name=\"Set\" action=\"Set\">\n"
"      <menuitem name=\"SetOriginToCenterOfMolecule\" action=\"SetOriginToCenterOfMolecule\" />\n"
"      <menuitem name=\"SetOriginToCenterOfSelectedAtoms\" action=\"SetOriginToCenterOfSelectedAtoms\" />\n"
"      <menuitem name=\"SetXYZToPAX\" action=\"SetXYZToPAX\" />\n"
"      <menuitem name=\"SetXYZToPAZ\" action=\"SetXYZToPAZ\" />\n"
"      <menuitem name=\"SetXYZToStandardOrientaion\" action=\"SetXYZToStandardOrientaion\" />\n"
"      <menuitem name=\"SetXYZToStandardOrientaionSelectedAndNotSelected\" action=\"SetXYZToStandardOrientaionSelectedAndNotSelected\" />\n"
"      <separator name=\"sepMenuSetLayer\" />\n"
"      <menuitem name=\"SetSelectedAtomsToHighLayer\" action=\"SetSelectedAtomsToHighLayer\" />\n"
"      <menuitem name=\"SetSelectedAtomsToMediumLayer\" action=\"SetSelectedAtomsToMediumLayer\" />\n"
"      <menuitem name=\"SetSelectedAtomsToLowLayer\" action=\"SetSelectedAtomsToLowLayer\" />\n"
"      <separator name=\"sepMenuSetFixed\" />\n"
"      <menuitem name=\"SetSelectedAtomsToFixed\" action=\"SetSelectedAtomsToFixed\" />\n"
"      <menuitem name=\"SetSelectedAtomsToVariable\" action=\"SetSelectedAtomsToVariable\" />\n"
"      <separator name=\"sepMenuSetType\" />\n"
"      <menuitem name=\"SetSymbolOfselectedAtoms\" action=\"SetSymbolOfselectedAtoms\" />\n"
"      <menuitem name=\"SetMMTypeOfselectedAtoms\" action=\"SetMMTypeOfselectedAtoms\" />\n"
"      <menuitem name=\"SetPDBTypeOfselectedAtoms\" action=\"SetPDBTypeOfselectedAtoms\" />\n"
"      <menuitem name=\"SetResidueNameOfselectedAtoms\" action=\"SetResidueNameOfselectedAtoms\" />\n"
"      <menuitem name=\"SetChargeOfselectedAtoms\" action=\"SetChargeOfselectedAtoms\" />\n"
"      <menuitem name=\"scaleChargesOfSelectedAtoms\" action=\"scaleChargesOfSelectedAtoms\" />\n"
"      <separator name=\"sepMenuSetDipole\" />\n"
"      <menuitem name=\"SetDipole\" action=\"SetDipole\" />\n"
"      <menuitem name=\"SetDipoleFormCharges\" action=\"SetDipoleFormCharges\" />\n"

"      <separator name=\"sepMenuScaleCell\" />\n"
"      <menuitem name=\"ScaleCell\" action=\"ScaleCell\" />\n"
"      <separator name=\"sepMenuSetHydrogenBonds\" />\n"
"      <menuitem name=\"SetHydrogenBonds\" action=\"SetHydrogenBonds\" />\n"
"      <separator name=\"sepMenuSetPropertiesOfAtoms\" />\n"
"      <menuitem name=\"SetPropertiesOfAtoms\" action=\"SetPropertiesOfAtoms\" />\n"
"      <separator name=\"sepMenuSetMolecularMechanicsParameters\" />\n"
"      <menuitem name=\"SetMolecularMechanicsParameters\" action=\"SetMolecularMechanicsParameters\" />\n"
"      <menuitem name=\"SetPDBTemplate\" action=\"SetPDBTemplate\" />\n"
"      <separator name=\"sepMenuSetChargesUsingPDBTemplate\" />\n"
"      <menuitem name=\"SetChargesUsingPDBTemplate\" action=\"SetChargesUsingPDBTemplate\" />\n"
"      <menuitem name=\"SetAtomTypesUsingPDBTemplate\" action=\"SetAtomTypesUsingPDBTemplate\" />\n"
"      <menuitem name=\"SetAtomTypeAndChargeUsingPDBTemplate\" action=\"SetAtomTypeAndChargeUsingPDBTemplate\" />\n"
"      <menuitem name=\"SetAtomTypeCalcul\" action=\"SetAtomTypeCalcul\" />\n"
"      <menuitem name=\"SetChargesToZero\" action=\"SetChargesToZero\" />\n"
"      <separator name=\"sepMenuSetPovrayBackground\" />\n"
"      <menuitem name=\"SetPovrayBackground\" action=\"SetPovrayBackground\" />\n"
#ifdef DRAWGEOMGL
"      <separator name=\"sepMenuGL\" />\n"
"      <menuitem name=\"SetCamera\" action=\"SetCamera\" />\n"
"      <menuitem name=\"SetLightPositions\" action=\"SetLightPositions\" />\n"
"      <separator name=\"sepMenuAxes\" />\n"
"      <menuitem name=\"SetXYZAxesProperties\" action=\"SetXYZAxesProperties\" />\n"
#endif
"    </menu>\n"
"    <separator name=\"sepExport\" />\n"
"    <menu name=\"Export\" action=\"Export\">\n"
"      <menuitem name=\"ExportPostscript\" action=\"ExportPostscript\" />\n"
"      <menuitem name=\"ExportEPS\" action=\"ExportEPS\" />\n"
"      <menuitem name=\"ExportPDF\" action=\"ExportPDF\" />\n"
"      <menuitem name=\"ExportSVG\" action=\"ExportSVG\" />\n"
"      <menuitem name=\"ExportPovray\" action=\"ExportPovray\" />\n"
"    </menu>\n"
"    <separator name=\"sepScreenCapture\" />\n"
"    <menu name=\"ScreenCapture\" action=\"ScreenCapture\">\n"
"      <menuitem name=\"ScreenCaptureJPG\" action=\"ScreenCaptureJPG\" />\n"
"      <menuitem name=\"ScreenCapturePPM\" action=\"ScreenCapturePPM\" />\n"
"      <menuitem name=\"ScreenCaptureBMP\" action=\"ScreenCaptureBMP\" />\n"
"      <menuitem name=\"ScreenCapturePNG\" action=\"ScreenCapturePNG\" />\n"
"      <menuitem name=\"ScreenCaptureTIF\" action=\"ScreenCaptureTIF\" />\n"
"      <menuitem name=\"ScreenCapturePS\" action=\"ScreenCapturePS\" />\n"
"      <menuitem name=\"ScreenCaptureCilpBoard\" action=\"ScreenCaptureCilpBoard\" />\n"
"    </menu>\n"
"    <separator name=\"sepTools\" />\n"
"    <menu name=\"Tools\" action=\"Tools\">\n"
"      <menuitem name=\"ComputeTotalCharge\" action=\"ComputeTotalCharge\" />\n"
"      <menuitem name=\"ComputeDipoleFormCharges\" action=\"ComputeDipoleFormCharges\" />\n"
"      <separator name=\"sepSel\" />\n"
"      <menuitem name=\"ComputeChargeForResidues\" action=\"ComputeChargeForResidues\" />\n"
"      <menuitem name=\"ComputeChargeOfSelectedsAtoms\" action=\"ComputeChargeOfSelectedsAtoms\" />\n"
"      <separator name=\"sepIsotope\" />\n"
"      <menuitem name=\"ComputeIsotopeDistribution\" action=\"ComputeIsotopeDistribution\" />\n"
"      <separator name=\"sepVolMolecule\" />\n"
"      <menuitem name=\"ComputeVolumeMolecule\" action=\"ComputeVolumeMolecule\" />\n"
"    </menu>\n"
"    <separator name=\"sepMolecularMechanics\" />\n"
"    <menu name=\"MolecularMechanics\" action=\"MolecularMechanics\">\n"
"      <menuitem name=\"MolecularMechanicsEnergy\" action=\"MolecularMechanicsEnergy\" />\n"
"      <menuitem name=\"MolecularMechanicsOptimization\" action=\"MolecularMechanicsOptimization\" />\n"
"      <menuitem name=\"MolecularMechanicsDynamics\" action=\"MolecularMechanicsDynamics\" />\n"
"      <menuitem name=\"MolecularMechanicsDynamicsConfo\" action=\"MolecularMechanicsDynamicsConfo\" />\n"
"    </menu>\n"
"    <separator name=\"sepSemiEmpirical\" />\n"
"    <menu name=\"SemiEmpirical\" action=\"SemiEmpirical\">\n"
"      <separator name=\"sepSemiEmpiricalMopac\" />\n"
"      <menuitem name=\"SemiEmpiricalEnergyMopac\" action=\"SemiEmpiricalEnergyMopac\" />\n"
"      <menuitem name=\"SemiEmpiricalOptimizationMopac\" action=\"SemiEmpiricalOptimizationMopac\" />\n"
"      <menuitem name=\"SemiEmpiricalOptimizationMopacSparkle\" action=\"SemiEmpiricalOptimizationMopacSparkle\" />\n"
"      <menuitem name=\"SemiEmpiricalESPMopac\" action=\"SemiEmpiricalESPMopac\" />\n"
"      <menuitem name=\"SemiEmpiricalScanMopac\" action=\"SemiEmpiricalScanMopac\" />\n"

"      <separator name=\"sepSemiEmpiricalOrca\" />\n"
"      <menuitem name=\"SemiEmpiricalEnergyOrca\" action=\"SemiEmpiricalEnergyOrca\" />\n"
"      <menuitem name=\"SemiEmpiricalOptimizationOrca\" action=\"SemiEmpiricalOptimizationOrca\" />\n"

"      <separator name=\"sepSemiEmpiricalFireFly\" />\n"
"      <menuitem name=\"SemiEmpiricalEnergyFireFlyAM1\" action=\"SemiEmpiricalEnergyFireFlyAM1\" />\n"
"      <menuitem name=\"SemiEmpiricalOptimizationFireFlyAM1\" action=\"SemiEmpiricalOptimizationFireFlyAM1\" />\n"

"      <separator name=\"sepSemiEmpiricalOpenBabel\" />\n"
"      <menuitem name=\"SemiEmpiricalEnergyOpenBabel\" action=\"SemiEmpiricalEnergyOpenBabel\" />\n"
"      <menuitem name=\"SemiEmpiricalOptimizationOpenBabel\" action=\"SemiEmpiricalOptimizationOpenBabel\" />\n"

"      <separator name=\"sepSemiEmpiricalGeneric\" />\n"
"      <menuitem name=\"SemiEmpiricalEnergyGeneric\" action=\"SemiEmpiricalEnergyGeneric\" />\n"
"      <menuitem name=\"SemiEmpiricalOptimizationGeneric\" action=\"SemiEmpiricalOptimizationGeneric\" />\n"

"      <separator name=\"sepSemiEmpiricalMD\" />\n"
"      <menuitem name=\"SemiEmpiricalMD\" action=\"SemiEmpiricalMD\" />\n"
"      <menuitem name=\"SemiEmpiricalMDConfo\" action=\"SemiEmpiricalMDConfo\" />\n"
"    </menu>\n"
"    <separator name=\"sepView\" />\n"
"    <menu name=\"View\" action=\"View\">\n"
"      <menuitem name=\"ShowToolBar\" action=\"ShowToolBar\" />\n"
"      <menuitem name=\"ShowStatusBox\" action=\"ShowStatusBox\" />\n"
"    </menu>\n"
"    <separator name=\"sepClose\" />\n"
"    <menuitem name=\"Close\" action=\"Close\" />\n"
"  </popup>\n"
"  <toolbar action=\"ToolbarGL\">\n"
"      <toolitem name=\"OperationsTranslate\" action=\"OperationsTranslate\" />\n"
"      <toolitem name=\"OperationsRotation\" action=\"OperationsRotation\" />\n"
"      <toolitem name=\"OperationsRotationZ\" action=\"OperationsRotationZ\" />\n"
"      <toolitem name=\"OperationsZoom\" action=\"OperationsZoom\" />\n"
"      <toolitem name=\"RenderOptimal\" action=\"RenderOptimal\" />\n"
"      <separator name=\"sepToolBarSelectionOfAtoms\" />\n"
"      <toolitem name=\"OperationsEditObjects\" action=\"OperationsEditObjects\" />\n"
"      <toolitem name=\"SetAtomToInsert\" action=\"SetAtomToInsert\" />\n"
"      <toolitem name=\"AdjustHydrogens\" action=\"AdjustHydrogens\" />\n"
"      <toolitem name=\"OperationsInsertFrag\" action=\"OperationsInsertFrag\" />\n"
"      <toolitem name=\"OperationsSelectionOfAtoms\" action=\"OperationsSelectionOfAtoms\" />\n"
"      <toolitem name=\"OperationsDeleteObjects\" action=\"OperationsDeleteObjects\" />\n"
"      <toolitem name=\"OperationsMoveAtoms\" action=\"OperationsMoveAtoms\" />\n"
"      <toolitem name=\"OperationsRotationAtoms\" action=\"OperationsRotationAtoms\" />\n"
"      <toolitem name=\"OperationsRotationZAtoms\" action=\"OperationsRotationZAtoms\" />\n"
"      <toolitem name=\"RebuildConnectionsDuringEdition\" action=\"RebuildConnectionsDuringEdition\" />\n"
"      <separator name=\"sepToolBarMeasure\" />\n"
"      <separator name=\"sepToolBarMeasure1\" />\n"
"      <toolitem name=\"OperationsMeasure\" action=\"OperationsMeasure\" />\n"
"      <separator name=\"sepToolBarGeometryStick\" />\n"
"      <separator name=\"sepToolBarGeometryStick1\" />\n"
"      <toolitem name=\"RenderGeometryStick\" action=\"RenderGeometryStick\" />\n"
"      <toolitem name=\"RenderGeometryBallAndStick\" action=\"RenderGeometryBallAndStick\" />\n"
"      <separator name=\"sepToolBarShowMeasureNoteBook\" />\n"
"      <separator name=\"sepToolBarShowMeasureNoteBook1\" />\n"
"      <toolitem name=\"ShowMeasureNoteBook\" action=\"ShowMeasureNoteBook\" />\n"
"  </toolbar>\n"
;
/*******************************************************************************************************************************/
static void set_init_gtkActionToggleEntries()
{
	gint i=0;
	gtkActionToggleEntries[i++].is_active = distances_draw_mode(); /* LabelsDistances */
	gtkActionToggleEntries[i++].is_active = dipole_draw_mode(); /* LabelsDipole */
#ifdef DRAWGEOMGL
	gtkActionToggleEntries[i++].is_active = ortho_mode(); /* RenderLabelsOrtho */
#endif
	gtkActionToggleEntries[i++].is_active = pers_mode(); /* RenderPerspective */
	gtkActionToggleEntries[i++].is_active = light_mode(); /* RenderLighting */
	gtkActionToggleEntries[i++].is_active = ortep_mode(); /* RenderOrtep */
	gtkActionToggleEntries[i++].is_active = cartoon_mode(); /* RenderCartoon */
	gtkActionToggleEntries[i++].is_active = shad_mode(); /* RenderShad */
#ifdef DRAWGEOMGL
	gtkActionToggleEntries[i++].is_active = testShowAxesGeom(); /* RenderShowAxes */
	gtkActionToggleEntries[i++].is_active = testShowBoxGeom(); /* RenderShowBox */
#endif
	gtkActionToggleEntries[i++].is_active = dipole_mode(); /* RenderShowDipole */
	gtkActionToggleEntries[i++].is_active = ShowHBonds; /* RenderShowHydrogenBonds */
	gtkActionToggleEntries[i++].is_active = getShowMultipleBonds(); /* RenderShowDoubleTripleBonds */
	gtkActionToggleEntries[i++].is_active = TRUE; /* ShowToolBar */
	gtkActionToggleEntries[i++].is_active = TRUE; /* ShowStatusBox */
	gtkActionToggleEntries[i++].is_active = !MeasureIsHide; /* ShowMeasureNoteBook */
	gtkActionToggleEntries[i++].is_active = getAdjustHydrogensYesNo(); /* Ajust hydrogens */
	gtkActionToggleEntries[i++].is_active = getRebuildConnectionsDuringEditionYesNo(); /* rebuild connection */
}
/*******************************************************************************************************************************/
static void add_widget (GtkUIManager *merge, GtkWidget   *widget, GtkContainer *container)
{
	GtkWidget *handlebox;

	if (!GTK_IS_TOOLBAR (widget))  return;

	handlebox =gtk_handle_box_new ();
	g_object_ref (handlebox);
  	gtk_handle_box_set_handle_position  (GTK_HANDLE_BOX(handlebox),GTK_POS_TOP);
	/*   GTK_SHADOW_NONE,  GTK_SHADOW_IN,  GTK_SHADOW_OUT, GTK_SHADOW_ETCHED_IN, GTK_SHADOW_ETCHED_OUT */
	gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(handlebox),GTK_SHADOW_OUT);
	gtk_box_pack_start (GTK_BOX (container), handlebox, FALSE, FALSE, 0);

	if (GTK_IS_TOOLBAR (widget)) 
	{
		GtkToolbar *toolbar;
		toolbar = GTK_TOOLBAR (widget);
		gtk_toolbar_set_show_arrow (toolbar, TRUE);
		gtk_toolbar_set_style(toolbar, GTK_TOOLBAR_ICONS);
		gtk_toolbar_set_orientation(toolbar,  GTK_ORIENTATION_VERTICAL);
		toolBar = toolbar;
		handleBoxToolBar = handlebox;
	}
	gtk_widget_show (widget);
	gtk_container_add (GTK_CONTAINER (handlebox), widget);
	gtk_widget_show (handlebox);
}
/*********************************************************************************************************************/
void activate_rotation()
{
	GtkAction * actionRotation = gtk_ui_manager_get_action(manager, "/MenuGeom/Operations/OperationsRotation");
	if(GTK_IS_TOGGLE_ACTION(actionRotation)) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(actionRotation), TRUE);
}
/*********************************************************************************************************************/
void activate_edit_objects()
{
	GtkAction * actionEdition = gtk_ui_manager_get_action(manager, "/MenuGeom/Operations/OperationsEditObjects");
	if(GTK_IS_TOGGLE_ACTION(actionEdition)) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(actionEdition), TRUE);
}
/*********************************************************************************************************************/
void activate_insert_fragment()
{
	GtkAction * action = gtk_ui_manager_get_action(manager, "/MenuGeom/Operations/OperationsInsertFrag");
	if(GTK_IS_TOGGLE_ACTION(action)) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action), TRUE);
}
/*********************************************************************************************************************/
void activate_insert_crystal()
{
	GtkAction * action = gtk_ui_manager_get_action(manager, "/MenuGeom/Operations/OperationsInsertFrag");
	if(GTK_IS_TOGGLE_ACTION(action)) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action), TRUE);
	create_window_fragments_selector("Prototype crystals","Prototype crystals/CaF2_cF12_Fm-3m_225");
}
/*******************************************************************************************************************************/
void create_toolbar_and_popup_menu_geom(GtkWidget* box)
{
	GtkActionGroup *actionGroup = NULL;
	GtkUIManager *merge = NULL;
	GError *error = NULL;
	TypeRenderGeom mode = GEOMETRY_STICK;

  	merge = gtk_ui_manager_new ();
  	g_signal_connect_swapped (GeomDlg, "destroy", G_CALLBACK (g_object_unref), merge);

	actionGroup = gtk_action_group_new ("GabeditPopupMenuGeomActions");
	gtk_action_group_set_translation_domain(actionGroup,GETTEXT_PACKAGE);
	gtk_action_group_add_actions (actionGroup, gtkActionEntries, numberOfGtkActionEntries, NULL);

	set_init_gtkActionToggleEntries();
	gtk_action_group_add_toggle_actions (actionGroup, gtkActionToggleEntries, numberOfGtkActionToggleEntries, NULL);

	if(getOperationType() == ROTATION) 
	gtk_action_group_add_radio_actions (actionGroup, operationsEntries, numberOfOperationsEntries, OPERATION_ROTATION, G_CALLBACK (render_operation_radio_action), NULL);
	else
	gtk_action_group_add_radio_actions (actionGroup, operationsEntries, numberOfOperationsEntries, OPERATION_EDIT_OBJECTS, G_CALLBACK (render_operation_radio_action), NULL);

	initLabelOptions (LABELNO);
	gtk_action_group_add_radio_actions (actionGroup, labelEntries , numberOfLabelEntries, LABEL_NO, G_CALLBACK (render_label_radio_action), NULL);

	if(!stick_mode()) mode = GEOMETRY_BALLSTICK;
	gtk_action_group_add_radio_actions (actionGroup, rendereGeometryEntries, numberOfRenderGeometryEntries, mode, G_CALLBACK (render_geometry_radio_action), NULL);

  	gtk_ui_manager_insert_action_group (merge, actionGroup, 0);

	g_signal_connect (merge, "add_widget", G_CALLBACK (add_widget), box);
  	gtk_window_add_accel_group (GTK_WINDOW (GeomDlg), gtk_ui_manager_get_accel_group (merge));
	if (!gtk_ui_manager_add_ui_from_string (merge, uiMenuInfo, -1, &error))
	{
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}
	else
	{
		/*
		add_attach_functional_to_menu(merge);
		add_hydrocarbon_to_menu(merge);
		add_rings_to_menu(merge);
		add_drugs_to_menu(merge);
		add_miscellaneous_to_menu(merge);
		add_fullerene_to_menu(merge);
		*/
	}
	manager = merge;
	/*
	addGroupesToMenu();
	addFragmentsToMenu();
	*/
}
/*********************************************************************************************************************/
static void set_sensitive()
{
	GtkWidget *saveAs = gtk_ui_manager_get_widget (manager, "/MenuGeom/SaveAs");
	GtkWidget *deleteMolecule = gtk_ui_manager_get_widget (manager, "/MenuGeom/Edit/EditDeleteMolecule");
	GtkWidget *deleteHydrogenAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Edit/EditDeleteHydrogenAtoms");
	GtkWidget *deleteSelectedAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Edit/EditDeleteSelectedAtoms");
	GtkWidget *moveSelectedAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Edit/EditMoveCenterOfSelectedAtomsToOrigin");
	GtkWidget *alignSelectedAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Edit/EditAlignPrincipalAxesOfSelectedAtomsToXYZ");
	GtkWidget *alignSelectedAndNotSelectedAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Edit/EditAlignSelectedAndNotSelectedAtoms");
	GtkWidget *copySelectedAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Edit/EditCopySelectedAtoms");
	GtkWidget *resetAllConnections = gtk_ui_manager_get_widget (manager, "/MenuGeom/Edit/EditResetAllConnections");
	GtkWidget *resetMultipleConnections = gtk_ui_manager_get_widget (manager, "/MenuGeom/Edit/EditResetMultipleConnections");
	GtkWidget *resetSelectedConnections = gtk_ui_manager_get_widget (manager, "/MenuGeom/Edit/EditResetConnectionsBetweenSelectedAtoms");
	GtkWidget *resetSelectedAndNotSelectedConnections = gtk_ui_manager_get_widget (manager, "/MenuGeom/Edit/EditResetConnectionsBetweenSelectedAndNotSelectedAtoms");

	GtkWidget *selectAllAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Selection/EditSelectAll");
	GtkWidget *invertSelection = gtk_ui_manager_get_widget (manager, "/MenuGeom/Selection/EditInvertSelection");
	GtkWidget *unSelectAll = gtk_ui_manager_get_widget (manager, "/MenuGeom/Selection/EditUnSelectAll");
	GtkWidget *selectHighAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Selection/EditSelectHighAtoms");
	GtkWidget *selectMediumAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Selection/EditSelectMediumAtoms");
	GtkWidget *selectLowAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Selection/EditSelectLowAtoms");
	GtkWidget *selectFixedAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Selection/EditSelectFixedAtoms");
	GtkWidget *selectVariableAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Selection/EditSelectVariableAtoms");
	GtkWidget *selectFirstResidue = gtk_ui_manager_get_widget (manager, "/MenuGeom/Selection/EditSelectFirstResidue");
	GtkWidget *selectLastResidue = gtk_ui_manager_get_widget (manager, "/MenuGeom/Selection/EditSelectLastResidue");
	GtkWidget *selectResidueByName = gtk_ui_manager_get_widget (manager, "/MenuGeom/Selection/EditSelectResidueByName");
	GtkWidget *selectAtomsByType = gtk_ui_manager_get_widget (manager, "/MenuGeom/Selection/EditSelectAtomsByType");
	GtkWidget *selectAtomsBySymbol = gtk_ui_manager_get_widget (manager, "/MenuGeom/Selection/EditSelectAtomsBySymbol");
	GtkWidget *selectAtomsBySphere = gtk_ui_manager_get_widget (manager, "/MenuGeom/Selection/EditSelectAtomsBySphere");
	GtkWidget *selectAtomsByPositiveCharges = gtk_ui_manager_get_widget (manager, "/MenuGeom/Selection/EditSelectAtomsByPositiveCharges");
	GtkWidget *selectAtomsByNegativeCharges = gtk_ui_manager_get_widget (manager, "/MenuGeom/Selection/EditSelectAtomsByNegativeCharges");
	GtkWidget *selectAtomsMultiple = gtk_ui_manager_get_widget (manager, "/MenuGeom/Selection/EditSelectAtomsMultiple");
	GtkWidget *symmetry = gtk_ui_manager_get_widget (manager, "/MenuGeom/Symmetry");
	GtkWidget *export = gtk_ui_manager_get_widget (manager, "/MenuGeom/Export");
	GtkWidget *mm = gtk_ui_manager_get_widget (manager, "/MenuGeom/MolecularMechanics");
	GtkWidget *sm = gtk_ui_manager_get_widget (manager, "/MenuGeom/SemiEmpirical");
	GtkWidget *origMolecule = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetOriginToCenterOfMolecule");
	GtkWidget *dipoleCharges = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetDipoleFormCharges");
	GtkWidget *charges = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetChargesUsingPDBTemplate");
	GtkWidget *type = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetAtomTypesUsingPDBTemplate");
	GtkWidget *typeCharges = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetAtomTypeAndChargeUsingPDBTemplate");
	GtkWidget *typeCalcul = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetAtomTypeCalcul");
	GtkWidget *chargesZero = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetChargesToZero");

	GtkWidget *origAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetOriginToCenterOfSelectedAtoms");
	GtkWidget *pax = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetXYZToPAX");
	GtkWidget *paz = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetXYZToPAZ");
	GtkWidget *so = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetXYZToStandardOrientaion");
	GtkWidget *sosns = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetXYZToStandardOrientaionSelectedAndNotSelected");

	GtkWidget *layerHigh = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetSelectedAtomsToHighLayer");
	GtkWidget *layerMedium = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetSelectedAtomsToMediumLayer");
	GtkWidget *layerLow = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetSelectedAtomsToLowLayer");
	GtkWidget *fixedAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetSelectedAtomsToFixed");
	GtkWidget *variableAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetSelectedAtomsToVariable");
	GtkWidget *setMMTypeOfselectedAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetMMTypeOfselectedAtoms");
	GtkWidget *setPDBTypeOfselectedAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetPDBTypeOfselectedAtoms");
	GtkWidget *setResidueNameOfselectedAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetResidueNameOfselectedAtoms");
	GtkWidget *setSymbolOfselectedAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetSymbolOfselectedAtoms");
	GtkWidget *setChargeOfselectedAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/SetChargeOfselectedAtoms");
	GtkWidget *scaleChargeOfselectedAtoms = gtk_ui_manager_get_widget (manager, "/MenuGeom/Set/ScaleChargeOfselectedAtoms");
	GtkWidget *addPersonnalFragment = gtk_ui_manager_get_widget (manager, "/MenuGeom/Edit/PersonalFragments/PersonalFragmentsAddMolecule");
	GtkWidget *removePersonnalFragment = gtk_ui_manager_get_widget (manager, "/MenuGeom/Edit/PersonalFragments/PersonalFragmentsRemoveFragment");
	GtkWidget *removeAGroup = gtk_ui_manager_get_widget (manager, "/MenuGeom/Edit/PersonalFragments/PersonalFragmentsDeleteGroup");
	gboolean sensitive = TRUE;

  	if(Natoms<1) sensitive = FALSE;
	if(GTK_IS_WIDGET(saveAs)) gtk_widget_set_sensitive(saveAs, sensitive);
	if(GTK_IS_WIDGET(deleteHydrogenAtoms)) gtk_widget_set_sensitive(deleteHydrogenAtoms, sensitive);
	if(GTK_IS_WIDGET(deleteMolecule)) gtk_widget_set_sensitive(deleteMolecule, sensitive);
	if(GTK_IS_WIDGET(selectAllAtoms)) gtk_widget_set_sensitive(selectAllAtoms, sensitive);
	if(GTK_IS_WIDGET(selectHighAtoms)) gtk_widget_set_sensitive(selectHighAtoms, sensitive);
	if(GTK_IS_WIDGET(selectMediumAtoms)) gtk_widget_set_sensitive(selectMediumAtoms, sensitive);
	if(GTK_IS_WIDGET(selectLowAtoms)) gtk_widget_set_sensitive(selectLowAtoms, sensitive);
	if(GTK_IS_WIDGET(selectFixedAtoms)) gtk_widget_set_sensitive(selectFixedAtoms, sensitive);
	if(GTK_IS_WIDGET(selectVariableAtoms)) gtk_widget_set_sensitive(selectVariableAtoms, sensitive);
	if(GTK_IS_WIDGET(selectFirstResidue)) gtk_widget_set_sensitive(selectFirstResidue, sensitive);
	if(GTK_IS_WIDGET(selectLastResidue)) gtk_widget_set_sensitive(selectLastResidue, sensitive);
	if(GTK_IS_WIDGET(selectResidueByName)) gtk_widget_set_sensitive(selectResidueByName, sensitive);
	if(GTK_IS_WIDGET(selectAtomsByType)) gtk_widget_set_sensitive(selectAtomsByType, sensitive);
	if(GTK_IS_WIDGET(selectAtomsBySymbol)) gtk_widget_set_sensitive(selectAtomsBySymbol, sensitive);
	if(GTK_IS_WIDGET(selectAtomsBySphere)) gtk_widget_set_sensitive(selectAtomsBySphere, sensitive);
	if(GTK_IS_WIDGET(selectAtomsBySphere) && NFatoms<1) gtk_widget_set_sensitive(selectAtomsBySphere, FALSE);
	if(GTK_IS_WIDGET(selectAtomsByPositiveCharges)) gtk_widget_set_sensitive(selectAtomsByPositiveCharges, sensitive);
	if(GTK_IS_WIDGET(selectAtomsByNegativeCharges)) gtk_widget_set_sensitive(selectAtomsByNegativeCharges, sensitive);
	if(GTK_IS_WIDGET(selectAtomsMultiple)) gtk_widget_set_sensitive(selectAtomsMultiple, sensitive);
	if(GTK_IS_WIDGET(symmetry)) gtk_widget_set_sensitive(symmetry, sensitive);
	if(GTK_IS_WIDGET(export)) gtk_widget_set_sensitive(export, sensitive);
	if(GTK_IS_WIDGET(mm)) gtk_widget_set_sensitive(mm, sensitive);
	if(GTK_IS_WIDGET(sm)) gtk_widget_set_sensitive(sm, sensitive);
	if(GTK_IS_WIDGET(origMolecule)) gtk_widget_set_sensitive(origMolecule, sensitive);
	if(GTK_IS_WIDGET(dipoleCharges)) gtk_widget_set_sensitive(dipoleCharges, sensitive);
	if(GTK_IS_WIDGET(charges)) gtk_widget_set_sensitive(charges, sensitive);
	if(GTK_IS_WIDGET(type)) gtk_widget_set_sensitive(type, sensitive);
	if(GTK_IS_WIDGET(typeCharges)) gtk_widget_set_sensitive(typeCharges, sensitive);
	if(GTK_IS_WIDGET(typeCalcul)) gtk_widget_set_sensitive(typeCalcul, sensitive);
	if(GTK_IS_WIDGET(chargesZero)) gtk_widget_set_sensitive(chargesZero, sensitive);

	if(NFatoms<1) sensitive = FALSE;
	if(GTK_IS_WIDGET(origAtoms)) gtk_widget_set_sensitive(origAtoms, sensitive);
	if(
			getOperationType()!=SELECTFRAG && 
			getOperationType()!=SELECTRESIDUE && 
			getOperationType()!=SELECTOBJECTS
	)sensitive = FALSE;
	if(GTK_IS_WIDGET(deleteSelectedAtoms)) gtk_widget_set_sensitive(deleteSelectedAtoms, sensitive);
	if(GTK_IS_WIDGET(moveSelectedAtoms)) gtk_widget_set_sensitive(moveSelectedAtoms, sensitive);
	if(GTK_IS_WIDGET(alignSelectedAtoms)) gtk_widget_set_sensitive(alignSelectedAtoms, sensitive);
	if(GTK_IS_WIDGET(alignSelectedAndNotSelectedAtoms)) gtk_widget_set_sensitive(alignSelectedAndNotSelectedAtoms, sensitive);
	if(GTK_IS_WIDGET(copySelectedAtoms)) gtk_widget_set_sensitive(copySelectedAtoms, sensitive);
	if(GTK_IS_WIDGET(setMMTypeOfselectedAtoms)) gtk_widget_set_sensitive(setMMTypeOfselectedAtoms, sensitive);
	if(GTK_IS_WIDGET(setPDBTypeOfselectedAtoms)) gtk_widget_set_sensitive(setPDBTypeOfselectedAtoms, sensitive);
	if(GTK_IS_WIDGET(setResidueNameOfselectedAtoms)) gtk_widget_set_sensitive(setResidueNameOfselectedAtoms, sensitive);
	if(GTK_IS_WIDGET(setSymbolOfselectedAtoms)) gtk_widget_set_sensitive(setSymbolOfselectedAtoms, sensitive);
	if(GTK_IS_WIDGET(setChargeOfselectedAtoms)) gtk_widget_set_sensitive(setChargeOfselectedAtoms, sensitive);
	if(GTK_IS_WIDGET(scaleChargeOfselectedAtoms)) gtk_widget_set_sensitive(scaleChargeOfselectedAtoms, sensitive);
	if(GTK_IS_WIDGET(resetSelectedConnections)) gtk_widget_set_sensitive(resetSelectedConnections, sensitive);
	if(GTK_IS_WIDGET(resetSelectedAndNotSelectedConnections)) gtk_widget_set_sensitive(resetSelectedAndNotSelectedConnections, sensitive);


	sensitive = TRUE;
  	if(Natoms<2) sensitive = FALSE;
	if(GTK_IS_WIDGET(resetAllConnections)) gtk_widget_set_sensitive(resetAllConnections, sensitive);
	if(GTK_IS_WIDGET(resetMultipleConnections)) gtk_widget_set_sensitive(resetMultipleConnections, sensitive);
	if(GTK_IS_WIDGET(so)) gtk_widget_set_sensitive(so, sensitive);

	sensitive = TRUE;
	if(NFatoms<2) sensitive = FALSE;
	if(GTK_IS_WIDGET(pax)) gtk_widget_set_sensitive(pax, sensitive);
	if(GTK_IS_WIDGET(paz)) gtk_widget_set_sensitive(paz, sensitive);

	sensitive = TRUE;
	if(NFatoms<1) sensitive = FALSE;
	if(GTK_IS_WIDGET(fixedAtoms)) gtk_widget_set_sensitive(fixedAtoms, sensitive);
	if(GTK_IS_WIDGET(variableAtoms)) gtk_widget_set_sensitive(variableAtoms, sensitive);

	sensitive = TRUE;
  	if(Natoms<1) sensitive = FALSE;
	if(NFatoms<1) sensitive = FALSE;
	if(GTK_IS_WIDGET(layerHigh)) gtk_widget_set_sensitive(layerHigh, sensitive);
	if(GTK_IS_WIDGET(invertSelection)) gtk_widget_set_sensitive(invertSelection, sensitive);
	if(GTK_IS_WIDGET(unSelectAll)) gtk_widget_set_sensitive(unSelectAll, sensitive);
	if(GTK_IS_WIDGET(sosns)) gtk_widget_set_sensitive(sosns, sensitive);
  	if(Natoms<2) sensitive = FALSE;
	if(GTK_IS_WIDGET(layerMedium)) gtk_widget_set_sensitive(layerMedium, sensitive);
	if(GTK_IS_WIDGET(layerLow)) gtk_widget_set_sensitive(layerLow, sensitive);

	 sensitive = TRUE;
	if(getNumberOfGroups()<1) sensitive = FALSE;
	if(GTK_IS_WIDGET(addPersonnalFragment)) gtk_widget_set_sensitive(addPersonnalFragment, sensitive);
	if(GTK_IS_WIDGET(removePersonnalFragment)) gtk_widget_set_sensitive(removePersonnalFragment, sensitive);
	if(GTK_IS_WIDGET(removeAGroup)) gtk_widget_set_sensitive(removeAGroup, sensitive);

}
/*********************************************************************************************************************/
gboolean popup_menu_geom(guint button, guint32 time)
{
	GtkWidget *menu = gtk_ui_manager_get_widget (manager, "/MenuGeom");
	if (GTK_IS_MENU (menu)) 
	{
		set_sensitive();
		gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, button, time);
		return TRUE;
	}
	else printf(_("Error menu is not a menu\n"));
	return FALSE;
}
/*******************************************************************************************************************************/
/*
static void free_id_list(GObject* win, gchar* groupName)
{
	GSList* listOfId = NULL;
	if(!G_IS_OBJECT(manager)) return;
	listOfId = g_object_get_data(G_OBJECT(manager),groupName);
	if(!listOfId) return;
	g_slist_free(listOfId);
}
void add_a_personal_group_to_menu(gchar* groupName)
{
	guint  merge_id;
	GtkActionGroup *dag = NULL;
	GtkActionEntry *actionEntry;
	gchar* name;
	gchar* label;
	gchar* tmp;
	gchar* dagName = g_strdup_printf("PersonalFragmentsActions%s", groupName);
	GSList* listOfId = g_slist_alloc();

	if(!manager) return;
	merge_id = gtk_ui_manager_new_merge_id (GTK_UI_MANAGER (manager));

	dag = gtk_action_group_new (dagName);
	gtk_action_group_set_translation_domain(dag,GETTEXT_PACKAGE);
	gtk_ui_manager_insert_action_group (manager, dag, 0);
  	g_signal_connect_swapped (GeomDlg, "destroy", G_CALLBACK (g_object_unref), dag);
  	g_signal_connect(GeomDlg, "destroy", G_CALLBACK (free_id_list), groupName);

	name = g_strdup_printf("%s", groupName);
	label = g_strdup_printf ("%s", groupName);

	actionEntry = g_malloc(sizeof(GtkActionEntry));
	actionEntry->name = name;
	actionEntry->stock_id = NULL;
	actionEntry->label = label;
	actionEntry->accelerator = NULL;
	actionEntry->tooltip = NULL;
	actionEntry->callback = NULL;
        gtk_action_group_add_actions (dag, actionEntry, 1, NULL);

	tmp = g_strdup_printf("/MenuGeom/Edit/PersonalFragments");
	gtk_ui_manager_add_ui(GTK_UI_MANAGER (manager), merge_id, tmp, groupName, groupName, GTK_UI_MANAGER_MENU, FALSE);
	g_free(tmp);

	listOfId = g_slist_prepend(listOfId, GUINT_TO_POINTER(merge_id));
	g_object_set_data(G_OBJECT(manager),groupName,listOfId);
	g_object_set_data(G_OBJECT(manager),dagName,dag);

	g_free(dagName);

	gtk_ui_manager_ensure_update (manager);
}
void delete_a_personal_group_from_menu(gchar* groupName)
{
	GSList* listOfId = NULL;
	gchar* dagName = NULL;
	GtkActionGroup *dag = NULL;
	guint merge_id;
	GSList*  list = NULL;

	if(!manager) return;

	listOfId = g_object_get_data(G_OBJECT(manager),groupName);
	if(!listOfId) return;

	dagName = g_strdup_printf("PersonalFragmentsActions%s", groupName);
	dag = g_object_get_data(G_OBJECT(manager), dagName);
	g_free(dagName);

	list =   listOfId;
	while(list != NULL)
	{
		merge_id = GPOINTER_TO_INT(list->data);
		if(merge_id != 0) gtk_ui_manager_remove_ui(GTK_UI_MANAGER (manager), merge_id);
		list = list->next;
	}
	if(dag)
	{
		gtk_ui_manager_remove_action_group (GTK_UI_MANAGER (manager), dag);
	}
	gtk_ui_manager_ensure_update (manager);
}
static void activate_add_personal_fragment (GtkAction *action, gpointer data)
{
	gchar* nameFrag = NULL;

	if(data) nameFrag = (gchar*) data;
	if(nameFrag)
	{
		addPersonalFragment(nameFrag, 0, NULL);
	}

}
void add_a_personal_fragement_to_menu(gchar* groupName,gchar* fragName)
{
	const gchar* menuBase = "Edit/PersonalFragments";
	guint  merge_id;
	static GtkActionGroup *dag = NULL;
	GtkActionEntry *actionEntry;
	GtkAction *action;
	gchar* name;
	gchar* label;
	gchar* data;
	GList*  listOfActions = NULL;
	GList*  list = NULL;
	gint i;
	gchar* tmp;
	GSList* listOfId = NULL;

	if(!manager) return;
	merge_id = gtk_ui_manager_new_merge_id (GTK_UI_MANAGER (manager));

	if(!dag)
	{
		dag = gtk_action_group_new ("FragmentsActions");
		gtk_action_group_set_translation_domain(dag,GETTEXT_PACKAGE);
		gtk_ui_manager_insert_action_group (manager, dag, 0);
  		g_signal_connect (GeomDlg, "destroy", G_CALLBACK (delete_dag), &dag);
	}
	listOfActions =   gtk_action_group_list_actions(dag);
	i = 0;
	list = listOfActions;
	tmp = g_strdup_printf("%s%s", groupName, fragName);
	while(list != NULL)
	{
		G_CONST_RETURN gchar* nameAction;
		action = (GtkAction *)(list->data);
		if(!GTK_IS_ACTION(action)) break;
		nameAction = gtk_action_get_name(GTK_ACTION(action));
		if(strstr(nameAction,tmp)) i++;
		list = list->next;
	}
	g_free(tmp);
	if(i==0)
	{
		name = g_strdup_printf("%s%s", groupName, fragName);
		label = g_strdup_printf ("%s", fragName);
		data = g_strdup_printf("%s/%s", groupName, fragName);
	}
	else
	{
		name = g_strdup_printf("%s%s%d", groupName, fragName,i);
		label = g_strdup_printf ("%s%d", fragName,i);
		data = g_strdup_printf("%s/%s%d", groupName, fragName,i);
	}
	actionEntry = g_malloc(sizeof(GtkActionEntry));
	actionEntry->name = name;
	actionEntry->stock_id = NULL;
	actionEntry->label = label;
	actionEntry->accelerator = NULL;
	actionEntry->tooltip = NULL;
	actionEntry->callback = G_CALLBACK(activate_add_personal_fragment);
        gtk_action_group_add_actions (dag, actionEntry, 1, data);


	tmp = g_strdup_printf("/MenuGeom/%s/%s", menuBase, groupName);
	gtk_ui_manager_add_ui(GTK_UI_MANAGER (manager), merge_id, tmp, name, name, GTK_UI_MANAGER_MENUITEM, TRUE);
	g_free(tmp);

	listOfId = g_object_get_data(G_OBJECT(manager),groupName);
	if(listOfId)
	{
		listOfId = g_slist_prepend(listOfId, GUINT_TO_POINTER(merge_id));
		g_object_set_data(G_OBJECT(manager),groupName,listOfId);
	}
	g_object_set_data(G_OBJECT(manager),name,GUINT_TO_POINTER(merge_id));

	gtk_ui_manager_ensure_update (manager);

}
void delete_a_personal_fragment_from_menu(gchar* groupName,gchar* fragName)
{
	GSList* listOfId = NULL;
	guint merge_id;
	GSList*  list = NULL;
	gpointer id;
	guint myId;
	gchar* name = NULL;

	if(!manager) return;

	name = g_strdup_printf("%s%s", groupName, fragName);
	id = g_object_get_data(G_OBJECT(manager),name);
	g_free(name);
	if(!id) return;

	myId = GPOINTER_TO_INT(id);
	if(myId == 0) return;

	listOfId = g_object_get_data(G_OBJECT(manager),groupName);
	if(!listOfId) return;

	list =   listOfId;
	while(list != NULL)
	{
		merge_id = GPOINTER_TO_INT(list->data);
		if(merge_id != 0 && merge_id == myId ) list->data = 0;
		list = list->next;
	}
	gtk_ui_manager_remove_ui(GTK_UI_MANAGER (manager), myId);
	gtk_ui_manager_ensure_update (manager);
}
*/
