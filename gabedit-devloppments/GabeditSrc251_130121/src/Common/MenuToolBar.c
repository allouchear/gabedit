/* MenuToolBar.c */
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
#include "../Common/Global.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Common/Run.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Files/FileChooser.h"
#include "../Common/TextEdit.h"
#include "../Common/Preferences.h"
#include "../Molcas/Molcas.h"
#include "../Molcas/MolcasBasisLibrary.h"
#include "../MPQC/MPQCBasisLibrary.h"
#include "../Molpro/Molpro.h"
#include "../Molpro/MolproBasisLibrary.h"
#include "../Gaussian/Gaussian.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Utils/ConvUtils.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/OpenBabel.h"
#include "../Common/Printer.h"
#include "../NetWork/Process.h"
#include "../NetWork/Batch.h"
#include "../MolecularMechanics/SetMMParameters.h"
#include "../MolecularMechanics/SetPDBTemplate.h"
#include "../Common/StockIcons.h"
#include "../Display/Vibration.h"
#include "../Utils/GabeditXYPlot.h"
#include "../Utils/GabeditContoursPlot.h"
#include "../Spectrum/IRSpectrum.h"
#include "../Spectrum/RamanSpectrum.h"
#include "../Spectrum/UVSpectrum.h"
#include "../Spectrum/ECDSpectrum.h"
#include "../Spectrum/NMRSpectrum.h"
#include "../VibrationalCorrections/GabeditGaussianInput.h"
#include "../IsotopeDistribution/IsotopeDistributionCalculatorDlg.h"
#include "../QFF/Gabedit2MRQFF.h"
#include "../Spectrum/VASPSpectra.h"
#include "../Spectrum/IGVPT2Spectrum.h"
#include "../Utils/UtilsVASP.h"

/*********************************************************************************************************************/
static gboolean ViewWindows = FALSE;
static gboolean ViewList = TRUE;
static GtkToolbar* toolBar = NULL;
static GtkWidget* handleBoxToolBar = NULL;
static gboolean ViewToolBar = TRUE;
static	GtkUIManager *manager = NULL;
/*********************************************************************************************************************/
static void view_icons()
{
	static gboolean mini=TRUE;
	mini = !mini;
	if(mini)
	gtk_rc_parse_string("gtk-toolbar-icon-size = small-toolbar");
	else
	gtk_rc_parse_string("gtk-toolbar-icon-size = large-toolbar");
}
/*********************************************************************************************************************/
static void view_windows_frame ()
{
	 if(!FrameWins) return;
	if(ViewWindows) gtk_widget_hide (GTK_WIDGET(FrameWins));
	else gtk_widget_show (GTK_WIDGET(FrameWins));
	ViewWindows = !ViewWindows;
}
/*********************************************************************************************************************/
static void view_projects_list ()
{
	if(!FrameList) return;
	if(ViewList)
	{
 		gtk_widget_hide (GTK_WIDGET(FrameList));
		gtk_widget_set_size_request(GTK_WIDGET(FrameList),1,-1);
 		gtk_widget_hide (GTK_WIDGET(Hpaned));
  		GTK_PANED(Hpaned)->child1_size = 1;    
 		gtk_widget_show (GTK_WIDGET(Hpaned));
	}
	else
 	{
  		GTK_PANED(Hpaned)->child1_size = (gint)(ScreenHeight*0.15);    
		gtk_widget_set_size_request(GTK_WIDGET(FrameList),(gint)(ScreenHeight*0.15),-1);
 		gtk_widget_hide (GTK_WIDGET(Hpaned));
  		gtk_widget_show (GTK_WIDGET(FrameList));
 		gtk_widget_show (GTK_WIDGET(Hpaned));
	}
	ViewList = !ViewList;
}
/*********************************************************************************************************************/
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
static void cut_text()
{
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(NoteBookText))==0)
		gabedit_text_cut_clipboard(text); 
	else
		gabedit_text_cut_clipboard(textresult); 
}
/********************************************************************************/
static void copy_text()
{
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(NoteBookText))==0)
		gabedit_text_copy_clipboard(text); 
	else
		gabedit_text_copy_clipboard(textresult); 
}
/********************************************************************************/
static void paste_text()
{
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(NoteBookText))==0)
		gabedit_text_paste_clipboard(text); 
	else
		gabedit_text_paste_clipboard(textresult); 
}
/*********************************************************************************************************************/
static void activate_action (GtkAction *action)
{
	const gchar *name = gtk_action_get_name (action);
	/* const gchar *typename = G_OBJECT_TYPE_NAME (action);*/

	if(!strcmp(name,"GamessInput")) new_gamess(NULL, NULL);
	else if(!strcmp(name,"DeMonInput")) new_demon(NULL, NULL);
	else if(!strcmp(name,"GaussianInput")) new_gauss(NULL, NULL);
	else if(!strcmp(name,"MolcasInput")) new_molcas(NULL, NULL);
	else if(!strcmp(name,"MolproInput")) new_molpro(NULL, NULL);
	else if(!strcmp(name,"MopacInput")) new_mopac(NULL, NULL);
	else if(!strcmp(name,"MPQCInput")) new_mpqc(NULL, NULL);
	else if(!strcmp(name,"NWChemInput")) new_nwchem(NULL, NULL);
	else if(!strcmp(name,"PsicodeInput")) new_psicode(NULL, NULL);
	else if(!strcmp(name,"OrcaInput")) new_orca(NULL, NULL);
	else if(!strcmp(name,"FireFlyInput")) new_firefly(NULL, NULL);
	else if(!strcmp(name,"QChemInput")) new_qchem(NULL, NULL);
	else if(!strcmp(name,"OtherInput")) new_other(NULL, NULL);
	else if(!strcmp(name,"Open")) open_file(NULL, NULL);
	else if(!strcmp(name,"Include")) inserrer_doc();
	else if(!strcmp(name,"SaveAs")) save_as_doc();
	else if(!strcmp(name,"Save")) save_doc();
	else if(!strcmp(name,"Print")) create_print_page();
	else if(!strcmp(name,"Exit")) 
	{
		if(imodif == DATA_MOD_NO ) exit_all(NULL,NULL);
		else Save_YesNo();
	}
	else if(!strcmp(name,"Cut"))  cut_text();
	else if(!strcmp(name,"Copy"))  copy_text();
	else if(!strcmp(name,"Paste")) paste_text();
	else if(!strcmp(name,"Find")) find_text_win();
	else if(!strcmp(name,"SelectAll")) select_all();
	else if(!strcmp(name,"InsertGaussianGeometry")) { iprogram = PROG_IS_GAUSS; insert_gaussian(2); }
	else if(!strcmp(name,"InsertGaussianMultiStep")) { iprogram = PROG_IS_GAUSS; insert_doc_gauss();}
	else if(!strcmp(name,"InsertMolcasAddToFile")) { iprogram = PROG_IS_MOLCAS; insertMolcas();}
	else if(!strcmp(name,"InsertMolproGeneral")) {iprogram = PROG_IS_MOLPRO; insert_molpro(1);  }
	else if(!strcmp(name,"InsertMolproGeometry")) {iprogram = PROG_IS_MOLPRO; insert_molpro(2);  }
	else if(!strcmp(name,"InsertMolproBasis")) { iprogram = PROG_IS_MOLPRO; insert_molpro(3);}
	else if(!strcmp(name,"InsertMolproCommands")) { iprogram = PROG_IS_MOLPRO; insert_molpro(4);}
	else if(!strcmp(name,"GeometryMolpro")) {iprogram = PROG_IS_MOLPRO; insert_molpro(2); }
	else if(!strcmp(name,"GeometryGaussian")) {iprogram = PROG_IS_GAUSS; insert_gaussian(2); }
	else if(!strcmp(name,"GeometryEdit")) {edit_geometry();}
	else if(!strcmp(name,"GeometryDraw"))
	{
		if(GeomDrawingArea==NULL) create_window_drawing();
		else rafresh_drawing();
	}
	else if(!strcmp(name,"DisplayDensity")) { draw_density_orbitals_gamess_or_gauss_or_molcas_or_molpro(NULL,NULL); }
	else if(!strcmp(name,"ToolsUnitConversion")) {create_conversion_dlg();}
	else if(!strcmp(name,"ToolsProcessLocalAll")) {run_process_all(FALSE);}
	else if(!strcmp(name,"ToolsProcessLocalUser")) {run_process_user(FALSE,NULL,NULL,NULL);}
	else if(!strcmp(name,"ToolsProcessRemoteAll")) {create_process_remote(TRUE);}
	else if(!strcmp(name,"ToolsProcessRemoteUser")) {create_process_remote(FALSE);}
	else if(!strcmp(name,"ToolsBatchLocalAll")) {run_batch_all(FALSE);}
	else if(!strcmp(name,"ToolsBatchLocalUser")) {run_batch_user(FALSE,NULL,NULL,NULL);}
	else if(!strcmp(name,"ToolsBatchRemoteAll")) {create_batch_remote(TRUE);}
	else if(!strcmp(name,"ToolsBatchRemoteUser")) {create_batch_remote(FALSE);}
	else if(!strcmp(name,"ToolsOpenBabel")) {create_babel_dialogue();}
	else if(!strcmp(name,"ToolsXYPlot")) { gabedit_xyplot_new_window(NULL,Fenetre);}
	else if(!strcmp(name,"ToolsContoursPlot")) { gabedit_contoursplot_new_window(NULL,Fenetre);}
	else if(!strcmp(name,"ToolsIRSpectrumGabedit")) { createIRSpectrum(Fenetre,GABEDIT_TYPEFILE_GABEDIT);}
	else if(!strcmp(name,"ToolsIRSpectrumDalton")) { createIRSpectrum(Fenetre,GABEDIT_TYPEFILE_DALTON);}
	else if(!strcmp(name,"ToolsIRSpectrumGamess")) { createIRSpectrum(Fenetre,GABEDIT_TYPEFILE_GAMESS);}
	else if(!strcmp(name,"ToolsIRSpectrumGamessAnharmonic")) { createIRSpectrum(Fenetre,GABEDIT_TYPEFILE_GAMESS_ANHARMONIC);}
	else if(!strcmp(name,"ToolsIRSpectrumNWChem")) { createIRSpectrum(Fenetre,GABEDIT_TYPEFILE_NWCHEM);}
	else if(!strcmp(name,"ToolsIRSpectrumPsicode")) { createIRSpectrum(Fenetre,GABEDIT_TYPEFILE_PSICODE);}
	else if(!strcmp(name,"ToolsIRSpectrumOrca")) { createIRSpectrum(Fenetre,GABEDIT_TYPEFILE_ORCA);}
	else if(!strcmp(name,"ToolsIRSpectrumFireFly")) { createIRSpectrum(Fenetre,GABEDIT_TYPEFILE_FIREFLY);}
	else if(!strcmp(name,"ToolsIRSpectrumGaussian")) { createIRSpectrum(Fenetre,GABEDIT_TYPEFILE_GAUSSIAN);}
	else if(!strcmp(name,"ToolsIRSpectrumGaussianAnharmonic")) { createIRSpectrum(Fenetre,GABEDIT_TYPEFILE_GAUSSIAN_ANHARMONIC);}
	else if(!strcmp(name,"ToolsIRSpectrumMolpro")) { createIRSpectrum(Fenetre,GABEDIT_TYPEFILE_MOLPRO);}
	else if(!strcmp(name,"ToolsIRSpectrumQChem")) { createIRSpectrum(Fenetre,GABEDIT_TYPEFILE_QCHEM);}
	else if(!strcmp(name,"ToolsIRSpectrumAdf")) { createIRSpectrum(Fenetre,GABEDIT_TYPEFILE_ADF);}
	else if(!strcmp(name,"ToolsIRSpectrumMolden")) { createIRSpectrum(Fenetre,GABEDIT_TYPEFILE_MOLDEN);}
	else if(!strcmp(name,"ToolsIRSpectrumiGVPT2")) { createIRSpectrum(Fenetre,GABEDIT_TYPEFILE_IGVPT2);}
	else if(!strcmp(name,"ToolsIRSpectrumTxt")) { createIRSpectrum(Fenetre,GABEDIT_TYPEFILE_TXT);}
	else if(!strcmp(name,"ToolsRamanSpectrumGabedit")) { createRamanSpectrum(Fenetre,GABEDIT_TYPEFILE_GABEDIT);}
	else if(!strcmp(name,"ToolsRamanSpectrumGamess")) { createRamanSpectrum(Fenetre,GABEDIT_TYPEFILE_GAMESS);}
	else if(!strcmp(name,"ToolsRamanSpectrumGaussian")) { createRamanSpectrum(Fenetre,GABEDIT_TYPEFILE_GAUSSIAN);}
	else if(!strcmp(name,"ToolsRamanSpectrumNWChem")) { createRamanSpectrum(Fenetre,GABEDIT_TYPEFILE_NWCHEM);}
	else if(!strcmp(name,"ToolsRamanSpectrumPsicode")) { createRamanSpectrum(Fenetre,GABEDIT_TYPEFILE_PSICODE);}
	else if(!strcmp(name,"ToolsRamanSpectrumQChem")) { createRamanSpectrum(Fenetre,GABEDIT_TYPEFILE_QCHEM);}
	else if(!strcmp(name,"ToolsRamanSpectrumOrca")) { createRamanSpectrum(Fenetre,GABEDIT_TYPEFILE_ORCA);}
	else if(!strcmp(name,"ToolsRamanSpectrumFireFly")) { createRamanSpectrum(Fenetre,GABEDIT_TYPEFILE_GAMESS);}
	else if(!strcmp(name,"ToolsRamanSpectrumMolden")) { createRamanSpectrum(Fenetre,GABEDIT_TYPEFILE_MOLDEN);}
	else if(!strcmp(name,"ToolsRamanSpectrumTxt")) { createRamanSpectrum(Fenetre,GABEDIT_TYPEFILE_TXT);}
	else if(!strcmp(name,"ToolsUVSpectrumGabedit")) { createUVSpectrum(Fenetre,GABEDIT_TYPEFILE_GABEDIT);}
	else if(!strcmp(name,"ToolsUVSpectrumGamess")) { createUVSpectrum(Fenetre,GABEDIT_TYPEFILE_GAMESS);}
	else if(!strcmp(name,"ToolsUVSpectrumGaussian")) { createUVSpectrum(Fenetre,GABEDIT_TYPEFILE_GAUSSIAN);}
	else if(!strcmp(name,"ToolsUVSpectrumFireFly")) { createUVSpectrum(Fenetre,GABEDIT_TYPEFILE_FIREFLY);}
	else if(!strcmp(name,"ToolsUVSpectrumNWChem")) { createUVSpectrum(Fenetre,GABEDIT_TYPEFILE_NWCHEM);}
	else if(!strcmp(name,"ToolsUVSpectrumPsicode")) { createUVSpectrum(Fenetre,GABEDIT_TYPEFILE_PSICODE);}
	else if(!strcmp(name,"ToolsUVSpectrumOrca")) { createUVSpectrum(Fenetre,GABEDIT_TYPEFILE_ORCA);}
	else if(!strcmp(name,"ToolsUVSpectrumQChem")) { createUVSpectrum(Fenetre,GABEDIT_TYPEFILE_QCHEM);}
	else if(!strcmp(name,"ToolsUVSpectrumTxt")) { createUVSpectrum(Fenetre,GABEDIT_TYPEFILE_TXT);}

	else if(!strcmp(name,"ToolsECDSpectrumGabedit")) { createECDSpectrum(Fenetre,GABEDIT_TYPEFILE_GABEDIT);}
	else if(!strcmp(name,"ToolsECDSpectrumGaussian")) { createECDSpectrum(Fenetre,GABEDIT_TYPEFILE_GAUSSIAN);}
	else if(!strcmp(name,"ToolsECDSpectrumNWChem")) { createECDSpectrum(Fenetre,GABEDIT_TYPEFILE_NWCHEM);}
	else if(!strcmp(name,"ToolsECDSpectrumPsicode")) { createECDSpectrum(Fenetre,GABEDIT_TYPEFILE_PSICODE);}
	else if(!strcmp(name,"ToolsECDSpectrumOrca")) { createECDSpectrum(Fenetre,GABEDIT_TYPEFILE_ORCA);}
	else if(!strcmp(name,"ToolsECDSpectrumTxt")) { createECDSpectrum(Fenetre,GABEDIT_TYPEFILE_TXT);}

	else if(!strcmp(name,"ToolsNMRSpectrumTxt")) { createNMRSpectrum(Fenetre,GABEDIT_TYPEFILE_TXT);}
	else if(!strcmp(name,"ToolsNMR2SpectrumTxt")) { createNMR2Spectrum(Fenetre,GABEDIT_TYPEFILE_TXT);}

	else if(!strcmp(name,"ToolsVASPOptic")) { read_vasp_xml_file_dlg();}
	else if(!strcmp(name,"ToolsVASPBands")) { read_bands_vasp_xml_file_dlg();}
	else if(!strcmp(name,"ToolsVASPDOS")) { read_dos_vasp_xml_file_dlg();}

	else if(!strcmp(name,"ToolsiGVPT2Read")) { createIRSpectrum(Fenetre,GABEDIT_TYPEFILE_IGVPT2);}
	else if(!strcmp(name,"ToolsiGVPT2Save")) { create_igvpt2_file_dlg(FALSE);}
	else if(!strcmp(name,"ToolsiGVPT2Run")) { create_igvpt2_file_dlg(TRUE);}

	else if(!strcmp(name,"ToolsVibCorrectionsGaussian")) {read_vibcorrection_gaussian_file_dlg(); }
	else if(!strcmp(name,"ToolsVib2MRQFF")) {read_2mrqff_file_dlg(); }
	else if(!strcmp(name,"ToolsAutoCorrelationGaussian")) {read_admp_build_dipole_dipole_autocorrelation_dlg(); }
	else if(!strcmp(name,"ToolsAutoCorrelationDipole")) {read_dipole_build_dipole_dipole_autocorrelation_dlg(); }
	else if(!strcmp(name,"ToolsIsotopeDistribution")) { compute_distribution_dlg(Fenetre, NULL); }

	else if(!strcmp(name,"RunAbinitio")) {create_run();}
	else if(!strcmp(name,"RunViewResult")) {view_result();}
	else if(!strcmp(name,"SettingsMolecularMechanicsParameters")) {setMMParamatersDlg();}
	else if(!strcmp(name,"SettingsPDBTemplate")) {setPDBTemplateDlg();}
	else if(!strcmp(name,"SettingsBasisMolcas")) {setMolcasBasisDlg();}
	else if(!strcmp(name,"SettingsBasisMolpro")) {setMolproBasisDlg();}
	else if(!strcmp(name,"SettingsBasisMPQC")) {setMPQCBasisDlg();}
	else if(!strcmp(name,"SettingsPreferences")) {create_preferences();}
	else if(!strcmp(name,"HelpAbout")) {show_about();}
	else if(!strcmp(name,"HelpVersion")) {show_version();}
}
/*********************************************************************************************************************/
static GtkActionEntry gtkActionEntries[] =
{
	{"File",     NULL, N_("_File")},
	{"FileNew",  GTK_STOCK_NEW, N_("_New")},
	{"DeMonInput", GABEDIT_STOCK_DEMON, N_("_DeMon input"), NULL, "New DeMon input file", G_CALLBACK (activate_action) },
	{"GamessInput", GABEDIT_STOCK_GAMESS, N_("_Gamess input"), NULL, "New Gamess input file", G_CALLBACK (activate_action) },
	{"GaussianInput", GABEDIT_STOCK_GAUSSIAN, N_("_Gaussian input"), NULL, "New Gaussian input file", G_CALLBACK (activate_action) },
	{"MolcasInput", GABEDIT_STOCK_MOLCAS, N_("Mol_cas input"), NULL, "New Molcas input file", G_CALLBACK (activate_action) },
	{"MolproInput", GABEDIT_STOCK_MOLPRO, N_("Mol_pro input"), NULL, "New Molpro input file", G_CALLBACK (activate_action) },
	{"MopacInput", GABEDIT_STOCK_MOPAC, N_("_Mopac input"), NULL, "New Mopac input file", G_CALLBACK (activate_action) },
	{"MPQCInput", GABEDIT_STOCK_MPQC, N_("MP_QC input"), NULL, "New MPQC input file", G_CALLBACK (activate_action) },
	{"NWChemInput", GABEDIT_STOCK_NWCHEM, N_("_NWChem input"), NULL, "New NWChem input file", G_CALLBACK (activate_action) },
	{"PsicodeInput", GABEDIT_STOCK_PSICODE, N_("_Psicode input"), NULL, "New psicode input file", G_CALLBACK (activate_action) },
	{"OrcaInput", GABEDIT_STOCK_ORCA, N_("_Orca input"), NULL, "New Orca input file", G_CALLBACK (activate_action) },
	{"FireFlyInput", GABEDIT_STOCK_FIREFLY, N_("_FireFly input"), NULL, "New FireFly input file", G_CALLBACK (activate_action) },
	{"QChemInput", GABEDIT_STOCK_QCHEM, N_("Q-_Chem input"), NULL, "New Q-Chem input file", G_CALLBACK (activate_action) },
	{"OtherInput", NULL, N_("_Other"), NULL, "Other", G_CALLBACK (activate_action) },
	{"Open", GTK_STOCK_OPEN, N_("_Open"), "<control>O", "open a file", G_CALLBACK (activate_action) },
	{"Save", GTK_STOCK_SAVE, N_("_Save"), "<control>S", "Save", G_CALLBACK (activate_action) },
	{"SaveAs", GTK_STOCK_SAVE_AS, N_("Save _as"), "<control>s", "Save as", G_CALLBACK (activate_action) },
	{"Include", GABEDIT_STOCK_INSERT, N_("_Include"), "<control>I", "Include a file", G_CALLBACK (activate_action) },
	{"Print", GTK_STOCK_PRINT, N_("_Print"), "<control>P", "Print", G_CALLBACK (activate_action) },
	{"Exit", GTK_STOCK_QUIT, N_("E_xit"), "<control>Q", "Exit", G_CALLBACK (activate_action) },
	{"Edit",  NULL, "_Edit"},
	{"Cut", GTK_STOCK_CUT, N_("C_ut"), "<control>X", "Cut the selected text to the clipboard", G_CALLBACK (activate_action) },
	{"Copy", GTK_STOCK_COPY, N_("_Copy"), "<control>C", "Copy the selected text to the clipboard", G_CALLBACK (activate_action) },
	{"Paste", GTK_STOCK_PASTE, N_("_Paste"), "<control>V", "Paste the text from the clipboard", G_CALLBACK (activate_action) },
	{"Find", GTK_STOCK_FIND, N_("_Find"), "<control>F", "Find a string", G_CALLBACK (activate_action) },
	{"SelectAll",  GABEDIT_STOCK_SELECT_ALL, N_("Select _all"), "<control>A", "Select All", G_CALLBACK (activate_action) },

	{"Insert",  NULL, N_("_Insert")},
	{"InsertGaussian",  GABEDIT_STOCK_GAUSSIAN, N_("_Gaussian")},
	{"InsertMolcas",  GABEDIT_STOCK_MOLCAS, N_("Mol_cas")},
	{"InsertMolpro",  GABEDIT_STOCK_MOLPRO, N_("Mol_pro")},
	{"InsertGaussianMultiStep", NULL, N_("_Add Input File(Multi-Step Job)"), NULL, "Add Input File(Multi-Step Job)", G_CALLBACK (activate_action) },
	{"InsertGaussianGeometry", NULL, N_("G_eometry"), NULL, "Insert Gaussian geometry", G_CALLBACK (activate_action) },
	{"InsertMolproGeneral", NULL, N_("_General"), NULL, "Insert Molpro general", G_CALLBACK (activate_action) },
	{"InsertMolproGeometry", NULL, N_("G_eometry"), NULL, "Insert Molpro geometry", G_CALLBACK (activate_action) },
	{"InsertMolproBasis", NULL, N_("_Basis"), NULL, "Insert Molpro basis", G_CALLBACK (activate_action) },
	{"InsertMolproCommands", NULL, N_("_Commands"), NULL, "Insert Molpro commands", G_CALLBACK (activate_action) },
	{"InsertMolcasAddToFile", NULL, N_("_Add to file"), NULL, "Insert Molcas add to file", G_CALLBACK (activate_action) },

	{"View",  NULL, N_("_View")},

	{"Geometry",  NULL, N_("_Geometry")},
	{"GeometryMolpro", GABEDIT_STOCK_MOLPRO, N_("Mol_pro"), NULL, "Edit Molpro geometry", G_CALLBACK (activate_action) },
	{"GeometryGaussian", GABEDIT_STOCK_GAUSSIAN, N_("_Gaussian"), NULL, "Edit Gaussian geometry", G_CALLBACK (activate_action) },
	{"GeometryEdit", NULL, N_("_Edit"), NULL, "Edit geometry", G_CALLBACK (activate_action) },
	{"GeometryDraw", GABEDIT_STOCK_GEOMETRY, N_("_Draw"), NULL, "Draw geometry", G_CALLBACK (activate_action) },

	{"DisplayDensity", GABEDIT_STOCK_ORBITALS, N_("_Display"), NULL, "Display Geometry/Orbitals/Density/Vibration", G_CALLBACK (activate_action) },

	{"Tools",  NULL, N_("_Tools")},
	{"ToolsProcess",  NULL, N_("_Process")},
	{"ToolsProcessLocal",  NULL, N_("_Local")},
	{"ToolsUnitConversion", GABEDIT_STOCK_CONVERT_UNIT, N_("Unit _conversion utility"), NULL, "Unit conversion utility", G_CALLBACK (activate_action) },
	{"ToolsProcessLocalAll", NULL, N_("_All local process"), NULL, "All local process", G_CALLBACK (activate_action) },
	{"ToolsProcessLocalUser", NULL, N_("_User local process"), NULL, "User local process", G_CALLBACK (activate_action) },
	{"ToolsProcessRemote",  NULL, N_("_Remote")},
	{"ToolsProcessRemoteAll", NULL, N_("_All remote process"), NULL, "All remote process", G_CALLBACK (activate_action) },
	{"ToolsProcessRemoteUser", NULL, N_("_User remote process"), NULL, "User remote process", G_CALLBACK (activate_action) },
	{"ToolsBatch",  NULL, N_("_Batch")},
	{"ToolsBatchLocal",  NULL, N_("_Local")},
	{"ToolsBatchLocalAll", NULL, N_("_All local batch jobs"), NULL, "All local batch jobs", G_CALLBACK (activate_action) },
	{"ToolsBatchLocalUser", NULL, N_("_User local batch jobs"), NULL, "User local batch jobs", G_CALLBACK (activate_action) },
	{"ToolsBatchRemote",  NULL, N_("_Remote")},
	{"ToolsBatchRemoteAll", NULL, N_("_All remote batch jobs"), NULL, "All remote batch jobs", G_CALLBACK (activate_action) },
	{"ToolsBatchRemoteUser", NULL, N_("_User remote batch jobs"), NULL, "User remote batch jobs", G_CALLBACK (activate_action) },
	{"ToolsOpenBabel",  GABEDIT_STOCK_OPEN_BABEL, N_("Open B_abel"), NULL, "Open babel", G_CALLBACK (activate_action) },
	{"ToolsContoursPlot",  GABEDIT_STOCK_GABEDIT, N_("Contours plotter"), NULL, "ContoursPlotter", G_CALLBACK (activate_action) },
	{"ToolsXYPlot",  GABEDIT_STOCK_GABEDIT, N_("XY plotter"), NULL, "XYPlotter", G_CALLBACK (activate_action) },
	{"ToolsIRSpectrum",  NULL, "_IR spectrum"},

	{"ToolsIRSpectrumGabedit",  GABEDIT_STOCK_GABEDIT, N_("Read frequencies and intensities from a _Gabedit file"), NULL, "Gabedit", G_CALLBACK (activate_action) },
	{"ToolsIRSpectrumDalton",  GABEDIT_STOCK_DALTON, N_("Read frequencies and intensities from a _Dalton output file"), NULL, "Dalton", G_CALLBACK (activate_action) },
	{"ToolsIRSpectrumGamess",  GABEDIT_STOCK_GAMESS, N_("Read frequencies and intensities from a _Gamess output file"), NULL, "Gamess", G_CALLBACK (activate_action) },
	{"ToolsIRSpectrumGamessAnharmonic",  GABEDIT_STOCK_GAMESS, N_("Read Anharmonic IR spectrum from a _Gamess output file"), NULL, "Gamess", G_CALLBACK (activate_action) },
	{"ToolsIRSpectrumGaussian",  GABEDIT_STOCK_GAUSSIAN, N_("Read frequencies and intensities from a _Gaussian output file"), NULL, "Gaussian", G_CALLBACK (activate_action) },
	{"ToolsIRSpectrumGaussianAnharmonic",  GABEDIT_STOCK_GAUSSIAN, N_("Read Anharmonic IR spectrum from a _Gaussian output file"), NULL, "Gaussian", G_CALLBACK (activate_action) },
	{"ToolsIRSpectrumMolpro",  GABEDIT_STOCK_MOLPRO, N_("Read frequencies and intensities from a _Molpro output file"), NULL, "Molpro", G_CALLBACK (activate_action) },
	{"ToolsIRSpectrumNWChem",  GABEDIT_STOCK_NWCHEM, N_("Read frequencies and intensities from a _NWChem output file"), NULL, "NWChem", G_CALLBACK (activate_action) },
	{"ToolsIRSpectrumPsicode",  GABEDIT_STOCK_PSICODE, N_("Read frequencies and intensities from a _Psicode output file"), NULL, "Psicode", G_CALLBACK (activate_action) },
	{"ToolsIRSpectrumOrca",  GABEDIT_STOCK_ORCA, N_("Read frequencies and intensities from a _Orca output file"), NULL, "Orca", G_CALLBACK (activate_action) },
	{"ToolsIRSpectrumFireFly",  GABEDIT_STOCK_FIREFLY, N_("Read frequencies and intensities from a _FireFly output file"), NULL, "FireFly", G_CALLBACK (activate_action) },
	{"ToolsIRSpectrumQChem",  GABEDIT_STOCK_QCHEM, N_("Read frequencies and intensities from a _Q-Chem output file"), NULL, "QChem", G_CALLBACK (activate_action) },
	{"ToolsIRSpectrumAdf",  GABEDIT_STOCK_ADF, N_("Read frequencies and intensities from a _ADF output file"), NULL, "ADF", G_CALLBACK (activate_action) },
	{"ToolsIRSpectrumMolden",  GABEDIT_STOCK_MOLDEN, N_("Read frequencies and intensities from a Mo_lden file"), NULL, "Molden", G_CALLBACK (activate_action) },
	{"ToolsIRSpectrumiGVPT2",  GABEDIT_STOCK_GABEDIT, N_("Read frequencies and intensities from a _iGVPT2 output file"), NULL, "iGVPT2", G_CALLBACK (activate_action) },
	{"ToolsIRSpectrumTxt",  NULL, N_("Read frequencies and intensities from an ASCII XY file(2 columns)"), NULL, "Txt", G_CALLBACK (activate_action) },
	{"ToolsRamanSpectrum",  NULL, N_("_Raman spectrum")},

	{"ToolsRamanSpectrumGabedit",  GABEDIT_STOCK_GABEDIT, N_("Read frequencies and intensities from a _Gabedit file"), NULL, "Gabedit", G_CALLBACK (activate_action) },
	{"ToolsRamanSpectrumGamess",  GABEDIT_STOCK_GAMESS, N_("Read frequencies and intensities from a Game_ss output file"), NULL, "Gamess", G_CALLBACK (activate_action) },
	{"ToolsRamanSpectrumGaussian",  GABEDIT_STOCK_GAUSSIAN, N_("Read frequencies and intensities from a _Gaussian output file"), NULL, "Gaussian", G_CALLBACK (activate_action) },
	{"ToolsRamanSpectrumNWChem",  GABEDIT_STOCK_NWCHEM, N_("Read frequencies and intensities from a _NWChem output file"), NULL, "NWChem", G_CALLBACK (activate_action) },
	{"ToolsRamanSpectrumPsicode",  GABEDIT_STOCK_PSICODE, N_("Read frequencies and intensities from a _Psicode output file"), NULL, "Psicode", G_CALLBACK (activate_action) },
	{"ToolsRamanSpectrumQChem",  GABEDIT_STOCK_QCHEM, N_("Read frequencies and intensities from a _QChem output file"), NULL, "Psicode", G_CALLBACK (activate_action) },
	{"ToolsRamanSpectrumOrca",  GABEDIT_STOCK_ORCA, N_("Read frequencies and intensities from a _Orca output file"), NULL, "Orca", G_CALLBACK (activate_action) },
	{"ToolsRamanSpectrumFireFly",  GABEDIT_STOCK_FIREFLY, N_("Read frequencies and intensities from a _FireFly output file"), NULL, "FireFly", G_CALLBACK (activate_action) },
	{"ToolsRamanSpectrumMolden",  GABEDIT_STOCK_MOLDEN, N_("Read frequencies and intensities from a Mo_lden file"), NULL, "Molden", G_CALLBACK (activate_action) },
	{"ToolsRamanSpectrumTxt",  NULL, N_("Read frequencies and intensities from an ASCII XY file(2 columns)"), NULL, "Txt", G_CALLBACK (activate_action) },
	{"ToolsUVSpectrum",  NULL, "_UV spectrum"},
	{"ToolsUVSpectrumGabedit",  GABEDIT_STOCK_GABEDIT, N_("Read energies and intensities from a _Gabedit file"), NULL, "Gabedit", G_CALLBACK (activate_action) },
	{"ToolsUVSpectrumGamess",  GABEDIT_STOCK_GAMESS, N_("Read energies and intensities from a _Gamess output file"), NULL, "Gamess", G_CALLBACK (activate_action) },
	{"ToolsUVSpectrumGaussian",  GABEDIT_STOCK_GAUSSIAN, N_("Read energies and intensities from a _Gaussian output file"), NULL, "Gaussian", G_CALLBACK (activate_action) },
	{"ToolsUVSpectrumFireFly",  GABEDIT_STOCK_FIREFLY, N_("Read energies and intensities from a _FireFly output file"), NULL, "FireFly", G_CALLBACK (activate_action) },
	{"ToolsUVSpectrumNWChem",  GABEDIT_STOCK_NWCHEM, N_("Read energies and intensities from a NWChem output file"), NULL, "NWChem", G_CALLBACK (activate_action) },
	{"ToolsUVSpectrumPsicode",  GABEDIT_STOCK_PSICODE, N_("Read energies and intensities from a Psicode output file"), NULL, "Psicode", G_CALLBACK (activate_action) },
	{"ToolsUVSpectrumOrca",  GABEDIT_STOCK_ORCA, N_("Read energies and intensities from a Orca output file"), NULL, "Orca", G_CALLBACK (activate_action) },
	{"ToolsUVSpectrumQChem",  GABEDIT_STOCK_QCHEM, N_("Read energies and intensities from a Q_Chem output file"), NULL, "QChem", G_CALLBACK (activate_action) },
	{"ToolsUVSpectrumTxt",  NULL, N_("Read energies and intensities from an ASCII XY file(2 columns)"), NULL, "Txt", G_CALLBACK (activate_action) },

	{"ToolsECDSpectrum",  NULL, N_("_ECD spectrum")},
	{"ToolsECDSpectrumGabedit",  GABEDIT_STOCK_GABEDIT, N_("Read energies and intensities from a _Gabedit file"), NULL, "Gabedit", G_CALLBACK (activate_action) },
	{"ToolsECDSpectrumGaussian",  GABEDIT_STOCK_GAUSSIAN, N_("Read energies and intensities from a _Gaussian output file"), NULL, "Gaussian", G_CALLBACK (activate_action) },
	{"ToolsECDSpectrumNWChem",  GABEDIT_STOCK_NWCHEM, N_("Read energies and intensities from a _NWChem output file"), NULL, "NWChem", G_CALLBACK (activate_action) },
	{"ToolsECDSpectrumPsicode",  GABEDIT_STOCK_PSICODE, N_("Read energies and intensities from a _Psicode output file"), NULL, "Psicode", G_CALLBACK (activate_action) },
	{"ToolsECDSpectrumOrca",  GABEDIT_STOCK_ORCA, N_("Read energies and intensities from a _Orca output file"), NULL, "Orca", G_CALLBACK (activate_action) },
	{"ToolsECDSpectrumTxt",  NULL, N_("Read energies and intensities from an ASCII XY file(2 columns)"), NULL, "Txt", G_CALLBACK (activate_action) },

	{"ToolsNMRSpectrum",  NULL, N_("_NMR spectrum")},
	{"ToolsNMRSpectrumTxt",  NULL, N_("NMR Spin-Spin Splitting Simulation"), NULL, "Txt", G_CALLBACK (activate_action) },
	{"ToolsNMR2SpectrumTxt",  NULL, N_("Read NMR frequencies and intensities from an ASCII XY file(2 columns)"), NULL, "Txt", G_CALLBACK (activate_action) },

	{"ToolsVibCorrections",  NULL, N_("Ro_vibrational corrections")},
	{"ToolsVibCorrectionsGaussian",  NULL, N_("Read Gaussian output file to compute the ro-vibrational corrections"), NULL, "Gaussian", G_CALLBACK (activate_action) },

	{"ToolsVibQFF",  NULL, N_("_QFF potentials")},
	{"ToolsVib2MRQFF",  NULL, N_("Read energies and compute the 2MR-QFF constants"), NULL, "Gabedit", G_CALLBACK (activate_action) },

	{"ToolsAutoCorrelation",  NULL, N_("_Auto correlation")},
	{"ToolsAutoCorrelationGaussian",  NULL, N_("Read Gaussian output file to compute the dipole-dipole auto correlation function"), NULL, "Gaussian", G_CALLBACK (activate_action) },
	{"ToolsAutoCorrelationDipole",  NULL, N_("Read an assci text file (4 cols: time(fs),mux,muy,muz) to compute the dipole-dipole auto correlation function"), NULL, "TXT", G_CALLBACK (activate_action) },
	{"ToolsIsotopeDistribution",  NULL, N_("Compute the isotope distribution for a molecule"), NULL, "Isotope distribution", G_CALLBACK (activate_action) },

	{"ToolsVASP",  NULL, N_("_VASP")},
	{"ToolsVASPOptic",  NULL, N_("Read _dielectric from a VASP xml file and compute optic properties"), NULL,"Txt", G_CALLBACK (activate_action)},
	{"ToolsVASPBands",  NULL, N_("Read energies from xml file and draw bands structure"),               NULL,"Txt", G_CALLBACK (activate_action)},
	{"ToolsVASPDOS",  NULL, N_("Read DOS from xml file"),                                               NULL,"Txt", G_CALLBACK (activate_action)},

	{"ToolsiGVPT2",  NULL, N_("_iGVPT2")},
	{"ToolsiGVPT2Read",  NULL, N_("Read frequencies and intensities from an iGVPT2 output file"), NULL,"out", G_CALLBACK (activate_action)},
	{"ToolsiGVPT2Save",  NULL, N_("Create an QM/MMFF94 iGVPT2 input file"),               NULL,"ici", G_CALLBACK (activate_action)},
	{"ToolsiGVPT2Run",  NULL, N_("Create an QM/MMFF94 input file & Run iGVPT2"),          NULL,"ici", G_CALLBACK (activate_action)},

	{"Run",  NULL, N_("_Run")},
	{"RunAbinitio", GTK_STOCK_EXECUTE, N_("_Run a Computation Chemistry program"),  "<control>R", "Run a program", G_CALLBACK (activate_action) },
	{"RunViewResult", NULL, N_("_View result of calculation"), NULL, "View result of calculation", G_CALLBACK (activate_action) },

	{"Settings",  NULL, N_("_Settings")},
	{"SettingsMolecularMechanicsParameters", NULL, N_("_Molecular Mechanics Parameters"), NULL, "Set Molecular Mechanics Parameters", G_CALLBACK (activate_action) },
	{"SettingsPDBTemplate", NULL, N_("_PDB Template"), NULL, "Set PDB Template", G_CALLBACK (activate_action) },
	{"SettingsBasis",  NULL, N_("_Basis")},
	{"SettingsBasisMolcas", GABEDIT_STOCK_MOLCAS, N_("Mol_cas"), NULL, "Set molcas basis list", G_CALLBACK (activate_action) },
	{"SettingsBasisMolpro", GABEDIT_STOCK_MOLPRO, N_("Mol_pro"), NULL, "Set molpro basis list", G_CALLBACK (activate_action) },
	{"SettingsBasisMPQC", GABEDIT_STOCK_MPQC, N_("MP_QC"), NULL, "Set MPQC basis list", G_CALLBACK (activate_action) },
	{"SettingsPreferences", GTK_STOCK_PREFERENCES, N_("P_references"), NULL, "Set preference parameters", G_CALLBACK (activate_action) },

	{"Window",  NULL, N_("_Window"), NULL, NULL, G_CALLBACK (activate_action)},

	{"Help",  NULL, N_("_Help")},
	{"HelpAbout", GTK_STOCK_ABOUT, N_("_About..."), NULL, "About...", G_CALLBACK (activate_action) },
	{"HelpVersion", NULL, N_("_Version..."), NULL, "Version...", G_CALLBACK (activate_action) },
};
static guint numberOfGtkActionEntries = G_N_ELEMENTS (gtkActionEntries);
/*********************************************************************************************************************/
static void toggle_action (GtkAction *action)
{
	const gchar *name = gtk_action_get_name (action);
	/* const gchar *typename = G_OBJECT_TYPE_NAME (action);*/

	if(!strcmp(name,"ViewWindowsFrame")) { view_windows_frame(); }
	else if(!strcmp(name,"ViewProjectsList")) { view_projects_list( );}
	else if(!strcmp(name,"ViewToolbar")) { view_toolbar(); }
	else if(!strcmp(name,"ViewMiniIcons")) { view_icons(); }
}
static GtkToggleActionEntry gtkActionToggleEntries[] =
{

	{ "ViewWindowsFrame", NULL, N_("Show _Windows frame"), NULL, "Show windows frame", G_CALLBACK (toggle_action), FALSE },
	{ "ViewProjectsList", NULL, N_("Show _Projects List"), NULL, "Show projects List", G_CALLBACK (toggle_action), TRUE },
	{ "ViewToolbar", NULL, N_("Show toolbar"), NULL, "Show toolbar", G_CALLBACK (toggle_action), TRUE},
	{ "ViewMiniIcons", NULL, N_("Mini icons"), NULL, "Mini icons", G_CALLBACK (toggle_action), TRUE}
};

static guint numberOfGtkActionToggleEntries = G_N_ELEMENTS (gtkActionToggleEntries);
/*********************************************************************************************************************/

/* XML description of the menus for the test app.  The parser understands
 * a subset of the Bonobo UI XML format, and uses GMarkup for parsing */
static const gchar *uiInfo =
"  <menubar>\n"
"    <menu name=\"File\" action=\"File\">\n"
"      <menu name=\"New\" action=\"FileNew\">\n"
"         <menuitem name=\"DeMonInput\" action=\"DeMonInput\" />\n"
"         <menuitem name=\"GamessInput\" action=\"GamessInput\" />\n"
"         <menuitem name=\"GaussianInput\" action=\"GaussianInput\" />\n"
"         <menuitem name=\"MolcasInput\" action=\"MolcasInput\" />\n"
"         <menuitem name=\"MolproInput\" action=\"MolproInput\" />\n"
"         <menuitem name=\"MopacInput\" action=\"MopacInput\" />\n"
"         <menuitem name=\"MPQCInput\" action=\"MPQCInput\" />\n"
"         <menuitem name=\"NWChemInput\" action=\"NWChemInput\" />\n"
"         <menuitem name=\"PsicodeInput\" action=\"PsicodeInput\" />\n"
"         <menuitem name=\"OrcaInput\" action=\"OrcaInput\" />\n"
"         <menuitem name=\"FireFlyInput\" action=\"FireFlyInput\" />\n"
"         <menuitem name=\"QChemInput\" action=\"QChemInput\" />\n"
"         <menuitem name=\"Other\" action=\"OtherInput\" />\n"
"      </menu>\n"
"      <menuitem name=\"Open\" action=\"Open\" />\n"
"      <menuitem name=\"Save\" action=\"Save\" />\n"
"      <menuitem name=\"SaveAs\" action=\"SaveAs\" />\n"
"      <menuitem name=\"Include\" action=\"Include\" />\n"
"      <separator name=\"sepMenuPrint\" />\n"
"      <menuitem name=\"Print\" action=\"Print\" />\n"
"      <separator name=\"sepMenuExit\" />\n"
"      <menuitem name=\"Exit\" action=\"Exit\" />\n"
"    </menu>\n"
"    <menu name=\"_Edit\" action=\"Edit\">\n"
"      <menuitem name=\"C_ut\" action=\"Cut\" />\n"
"      <menuitem name=\"_Copy\" action=\"Copy\" />\n"
"      <menuitem name=\"_Paste\" action=\"Paste\" />\n"
"      <menuitem name=\"_Select All\" action=\"SelectAll\" />\n"
"      <separator name=\"sepMenuFind\" />\n"
"      <menuitem name=\"_Find\" action=\"Find\" />\n"
"    </menu>\n"
"    <menu name=\"_Insert\" action=\"Insert\">\n"
"      <menu name=\"_Gaussian\" action=\"InsertGaussian\">\n"
"        <menuitem name=\"_Add Input File(Multi-Step Job)\" action=\"InsertGaussianMultiStep\" />\n"
"        <menuitem name=\"G_eometry\" action=\"InsertGaussianGeometry\" />\n"
"      </menu>\n"
"      <menu name=\"Mol_cas\" action=\"InsertMolcas\">\n"
"        <menuitem name=\"_Add to file\" action=\"InsertMolcasAddToFile\" />\n"
"      </menu>\n"
"      <menu name=\"Mol_pro\" action=\"InsertMolpro\">\n"
"        <menuitem name=\"_General\" action=\"InsertMolproGeneral\" />\n"
"        <menuitem name=\"G_eometry\" action=\"InsertMolproGeometry\" />\n"
"        <menuitem name=\"_Basis\" action=\"InsertMolproBasis\" />\n"
"        <menuitem name=\"_Commands\" action=\"InsertMolproCommands\" />\n"
"      </menu>\n"
"    </menu>\n"
"    <menu name=\"View\" action=\"View\">\n"
"      <menuitem name=\"ViewWindowsFrame frame\" action=\"ViewWindowsFrame\" />\n"
"      <menuitem name=\"ViewProjectsList\" action=\"ViewProjectsList\" />\n"
"      <menuitem name=\"ViewToolbar\" action=\"ViewToolbar\" />\n"
"      <menuitem name=\"ViewMiniIcons\" action=\"ViewMiniIcons\" />\n"
"      <separator name=\"sepMenuIcons\" />\n"
"    </menu>\n"
"    <menu name=\"_Geometry\" action=\"Geometry\">\n"
"      <menuitem name=\"GeometryGaussian\" action=\"GeometryGaussian\" />\n"
"      <menuitem name=\"GeometryMolpro\" action=\"GeometryMolpro\" />\n"
"      <menuitem name=\"GeometryEdit\" action=\"GeometryEdit\" />\n"
"      <menuitem name=\"GeometryDraw\" action=\"GeometryDraw\" />\n"
"    </menu>\n"
"    <menu name=\"Tools\" action=\"Tools\">\n"
"      <menu name=\"ToolsProcess\" action=\"ToolsProcess\">\n"
"        <menu name=\"ToolsProcessLocal\" action=\"ToolsProcessLocal\">\n"
"          <menuitem name=\"ToolsProcessLocalAll\" action=\"ToolsProcessLocalAll\" />\n"
"          <menuitem name=\"ToolsProcessLocalUser\" action=\"ToolsProcessLocalUser\" />\n"
"        </menu>\n"
"        <menu name=\"ToolsProcessRemote\" action=\"ToolsProcessRemote\">\n"
"          <menuitem name=\"ToolsProcessRemoteAll\" action=\"ToolsProcessRemoteAll\" />\n"
"          <menuitem name=\"ToolsProcessRemoteUser\" action=\"ToolsProcessRemoteUser\" />\n"
"        </menu>\n"
"      </menu>\n"
"      <menu name=\"ToolsBatch\" action=\"ToolsBatch\">\n"
"        <menu name=\"ToolsBatchLocal\" action=\"ToolsBatchLocal\">\n"
"          <menuitem name=\"ToolsBatchLocalAll\" action=\"ToolsBatchLocalAll\" />\n"
"          <menuitem name=\"ToolsBatchLocalUser\" action=\"ToolsBatchLocalUser\" />\n"
"        </menu>\n"
"        <menu name=\"ToolsBatchRemote\" action=\"ToolsBatchRemote\">\n"
"          <menuitem name=\"ToolsBatchRemoteAll\" action=\"ToolsBatchRemoteAll\" />\n"
"          <menuitem name=\"ToolsBatchRemoteUser\" action=\"ToolsBatchRemoteUser\" />\n"
"        </menu>\n"
"      </menu>\n"
"      <menuitem name=\"ToolsOpenBabel\" action=\"ToolsOpenBabel\" />\n"
"      <separator name=\"sepUnitConv\" />\n"
"      <menuitem name=\"ToolsUnitConversion\" action=\"ToolsUnitConversion\" />\n"
"      <separator name=\"sepXYPlot\" />\n"
"      <menuitem name=\"ToolsXYPlot\" action=\"ToolsXYPlot\" />\n"
"      <separator name=\"sepContoursPlot\" />\n"
"      <menuitem name=\"ToolsContousPlot\" action=\"ToolsContoursPlot\" />\n"

"      <separator name=\"sepIRSpectrum\" />\n"
"      <menu name=\"ToolsIRSpectrum\" action=\"ToolsIRSpectrum\">\n"
"          <menuitem name=\"ToolsIRSpectrumGabedit\" action=\"ToolsIRSpectrumGabedit\" />\n"
"          <menuitem name=\"ToolsIRSpectrumDalton\" action=\"ToolsIRSpectrumDalton\" />\n"
"          <menuitem name=\"ToolsIRSpectrumGamess\" action=\"ToolsIRSpectrumGamess\" />\n"
"          <menuitem name=\"ToolsIRSpectrumGamessAnharmonic\" action=\"ToolsIRSpectrumGamessAnharmonic\" />\n"
"          <menuitem name=\"ToolsIRSpectrumGaussian\" action=\"ToolsIRSpectrumGaussian\" />\n"
"          <menuitem name=\"ToolsIRSpectrumGaussianAnharmonic\" action=\"ToolsIRSpectrumGaussianAnharmonic\" />\n"
"          <menuitem name=\"ToolsIRSpectrumMolpro\" action=\"ToolsIRSpectrumMolpro\" />\n"
"          <menuitem name=\"ToolsIRSpectrumNWChem\" action=\"ToolsIRSpectrumNWChem\" />\n"
/*"          <menuitem name=\"ToolsIRSpectrumPsicode\" action=\"ToolsIRSpectrumPsicode\" />\n"*/
"          <menuitem name=\"ToolsIRSpectrumOrca\" action=\"ToolsIRSpectrumOrca\" />\n"
"          <menuitem name=\"ToolsIRSpectrumFireFly\" action=\"ToolsIRSpectrumFireFly\" />\n"
"          <menuitem name=\"ToolsIRSpectrumQChem\" action=\"ToolsIRSpectrumQChem\" />\n"
"          <menuitem name=\"ToolsIRSpectrumAdf\" action=\"ToolsIRSpectrumAdf\" />\n"
"          <menuitem name=\"ToolsIRSpectrumMolden\" action=\"ToolsIRSpectrumMolden\" />\n"
"          <menuitem name=\"ToolsIRSpectrumiGVPT2\" action=\"ToolsIRSpectrumiGVPT2\" />\n"
"          <menuitem name=\"ToolsIRSpectrumTxt\" action=\"ToolsIRSpectrumTxt\" />\n"
"      </menu>\n"
"      <separator name=\"sepRamanSpectrum\" />\n"
"      <menu name=\"ToolsRamanSpectrum\" action=\"ToolsRamanSpectrum\">\n"
"          <menuitem name=\"ToolsRamanSpectrumGabedit\" action=\"ToolsRamanSpectrumGabedit\" />\n"
"          <menuitem name=\"ToolsRamanSpectrumGamess\" action=\"ToolsRamanSpectrumGamess\" />\n"
"          <menuitem name=\"ToolsRamanSpectrumGaussian\" action=\"ToolsRamanSpectrumGaussian\" />\n"
"          <menuitem name=\"ToolsRamanSpectrumNWChem\" action=\"ToolsRamanSpectrumNWChem\" />\n"
/*"          <menuitem name=\"ToolsRamanSpectrumPsicode\" action=\"ToolsRamanSpectrumPsicode\" />\n"*/
"          <menuitem name=\"ToolsRamanSpectrumOrca\" action=\"ToolsRamanSpectrumOrca\" />\n"
"          <menuitem name=\"ToolsRamanSpectrumQChem\" action=\"ToolsRamanSpectrumQChem\" />\n"
"          <menuitem name=\"ToolsRamanSpectrumFireFly\" action=\"ToolsRamanSpectrumFireFly\" />\n"
"          <menuitem name=\"ToolsRamanSpectrumMolden\" action=\"ToolsRamanSpectrumMolden\" />\n"
"          <menuitem name=\"ToolsRamanSpectrumTxt\" action=\"ToolsRamanSpectrumTxt\" />\n"
"      </menu>\n"
"      <separator name=\"sepUVSpectrum\" />\n"
"      <menu name=\"ToolsUVSpectrum\" action=\"ToolsUVSpectrum\">\n"
"          <menuitem name=\"ToolsUVSpectrumGabedit\" action=\"ToolsUVSpectrumGabedit\" />\n"
"          <menuitem name=\"ToolsUVSpectrumGamess\" action=\"ToolsUVSpectrumGamess\" />\n"
"          <menuitem name=\"ToolsUVSpectrumGaussian\" action=\"ToolsUVSpectrumGaussian\" />\n"
"          <menuitem name=\"ToolsUVSpectrumNWChem\" action=\"ToolsUVSpectrumNWChem\" />\n"
"          <menuitem name=\"ToolsUVSpectrumPsicode\" action=\"ToolsUVSpectrumPsicode\" />\n"
"          <menuitem name=\"ToolsUVSpectrumOrca\" action=\"ToolsUVSpectrumOrca\" />\n"
"          <menuitem name=\"ToolsUVSpectrumFireFly\" action=\"ToolsUVSpectrumFireFly\" />\n"
"          <menuitem name=\"ToolsUVSpectrumQChem\" action=\"ToolsUVSpectrumQChem\" />\n"
"          <menuitem name=\"ToolsUVSpectrumTxt\" action=\"ToolsUVSpectrumTxt\" />\n"
"      </menu>\n"
"      <separator name=\"sepECDSpectrum\" />\n"
"      <menu name=\"ToolsECDSpectrum\" action=\"ToolsECDSpectrum\">\n"
"          <menuitem name=\"ToolsECDSpectrumGabedit\" action=\"ToolsECDSpectrumGabedit\" />\n"
"          <menuitem name=\"ToolsECDSpectrumGaussian\" action=\"ToolsECDSpectrumGaussian\" />\n"
"          <menuitem name=\"ToolsECDSpectrumNWChem\" action=\"ToolsECDSpectrumNWChem\" />\n"
/*"          <menuitem name=\"ToolsECDSpectrumPsicode\" action=\"ToolsECDSpectrumPsicode\" />\n"*/
"          <menuitem name=\"ToolsECDSpectrumOrca\" action=\"ToolsECDSpectrumOrca\" />\n"
"          <menuitem name=\"ToolsECDSpectrumTxt\" action=\"ToolsECDSpectrumTxt\" />\n"
"      </menu>\n"
"      <separator name=\"sepNMRSpectrum\" />\n"
"      <menu name=\"ToolsNMRSpectrum\" action=\"ToolsNMRSpectrum\">\n"
"          <menuitem name=\"ToolsNMRSpectrumTxt\" action=\"ToolsNMRSpectrumTxt\" />\n"
"          <menuitem name=\"ToolsNMR2SpectrumTxt\" action=\"ToolsNMR2SpectrumTxt\" />\n"
"      </menu>\n"
"      <separator name=\"sepIsotopDistribution\" />\n"
"      <menuitem name=\"ToolsIsotopeDistribution\" action=\"ToolsIsotopeDistribution\" />\n"

"      <separator name=\"sepVibCorrections\" />\n"
"      <menu name=\"ToolsVibCorrections\" action=\"ToolsVibCorrections\">\n"
"          <menuitem name=\"ToolsVibCorrectionsGaussian\" action=\"ToolsVibCorrectionsGaussian\" />\n"
"      </menu>\n"
"      <separator name=\"sepVibQFF\" />\n"
"      <menu name=\"ToolsVibQFF\" action=\"ToolsVibQFF\">\n"
"          <menuitem name=\"ToolsVib2MRQFF\" action=\"ToolsVib2MRQFF\" />\n"
"      </menu>\n"

"      <separator name=\"sepAutoCorrelation\" />\n"
"      <menu name=\"ToolsAutoCorrelation\" action=\"ToolsAutoCorrelation\">\n"
"          <menuitem name=\"ToolsAutoCorrelationGaussian\" action=\"ToolsAutoCorrelationGaussian\" />\n"
"          <menuitem name=\"ToolsAutoCorrelationDipole\" action=\"ToolsAutoCorrelationDipole\" />\n"
"      </menu>\n"

"      <separator name=\"sepVASP\" />\n"
"      <menu name=\"ToolsVASP\" action=\"ToolsVASP\">\n"
"          <menuitem name=\"ToolsVASPOptic\" action=\"ToolsVASPOptic\" />\n"
"          <menuitem name=\"ToolsVASPBands\" action=\"ToolsVASPBands\" />\n"
"          <menuitem name=\"ToolsVASPDOS\" action=\"ToolsVASPDOS\" />\n"
"      </menu>\n"

"      <separator name=\"sepiGVPT2\" />\n"
"      <menu name=\"ToolsiGVPT2\" action=\"ToolsiGVPT2\">\n"
"          <menuitem name=\"ToolsiGVPT2Read\" action=\"ToolsiGVPT2Read\" />\n"
"          <menuitem name=\"ToolsiGVPT2Save\" action=\"ToolsiGVPT2Save\" />\n"
"          <menuitem name=\"ToolsiGVPT2Run\" action=\"ToolsiGVPT2Run\" />\n"
"      </menu>\n"
"    </menu>\n"

"    <menu name=\"Run\" action=\"Run\">\n"
"      <menuitem name=\"RunAbinitio\" action=\"RunAbinitio\" />\n"
"      <menuitem name=\"RunViewResult\" action=\"RunViewResult\" />\n"
"    </menu>\n"
"    <menu name=\"Settings\" action=\"Settings\">\n"
"      <menuitem name=\"SettingsMolecularMechanicsParameters\" action=\"SettingsMolecularMechanicsParameters\" />\n"
"      <menuitem name=\"SettingsPDBTemplate\" action=\"SettingsPDBTemplate\" />\n"
"      <separator name=\"sepMenuBasis\" />\n"
"      <menu name=\"SettingsBasis\" action=\"SettingsBasis\">\n"
"         <menuitem name=\"SettingsBasisMolpro\" action=\"SettingsBasisMolpro\" />\n"
"         <menuitem name=\"SettingsBasisMolcas\" action=\"SettingsBasisMolcas\" />\n"
"         <menuitem name=\"SettingsBasisMPQC\" action=\"SettingsBasisMPQC\" />\n"
"      </menu>\n"
"      <separator name=\"sepMenuPreferences\" />\n"
"      <menuitem name=\"SettingsPreferences\" action=\"SettingsPreferences\" />\n"
"    </menu>\n"
"    <menu name=\"Window\" action=\"Window\">\n"
"      <separator name=\"sepMenuWin\" />\n"
"    </menu>\n"
"    <menu name=\"Help\" action=\"Help\" position=\"bot\">\n"
"      <menuitem name=\"HelpAbout\" action=\"HelpAbout\" />\n"
"      <menuitem name=\"HelpVersion\" action=\"HelpVersion\" />\n"
"    </menu>\n"
"  </menubar>\n"
"  <toolbar action=\"Toolbar\">\n"
"      <toolitem name=\"DeMonInput\" action=\"DeMonInput\" />\n"
"      <toolitem name=\"GamessInput\" action=\"GamessInput\" />\n"
"      <toolitem name=\"GaussianInput\" action=\"GaussianInput\" />\n"
"      <toolitem name=\"MolcasInput\" action=\"MolcasInput\" />\n"
"      <toolitem name=\"MolproInput\" action=\"MolproInput\" />\n"
"      <toolitem name=\"MopacInput\" action=\"MopacInput\" />\n"
"      <toolitem name=\"MPQCInput\" action=\"MPQCInput\" />\n"
"      <toolitem name=\"NWChemInput\" action=\"NWChemInput\" />\n"
"      <toolitem name=\"PsicodeInput\" action=\"PsicodeInput\" />\n"
"      <toolitem name=\"OrcaInput\" action=\"OrcaInput\" />\n"
"      <toolitem name=\"FireFlyInput\" action=\"FireFlyInput\" />\n"
"      <toolitem name=\"QChemInput\" action=\"QChemInput\" />\n"
"      <toolitem name=\"Include\" action=\"Include\" />\n"
"      <toolitem name=\"Open\" action=\"Open\" />\n"
"      <toolitem name=\"Save\" action=\"Save\" />\n"
"      <toolitem name=\"SaveAs\" action=\"SaveAs\" />\n"
"      <toolitem name=\"Print\" action=\"Print\" />\n"
"      <separator name=\"sepToolBarCut\" />\n"
"      <toolitem name=\"Cut\" action=\"Cut\" />\n"
"      <toolitem name=\"Copy\" action=\"Copy\" />\n"
"      <toolitem name=\"Paste\" action=\"Paste\" />\n"
"      <toolitem name=\"Find\" action=\"Find\" />\n"
"      <toolitem name=\"SelectAll\" action=\"SelectAll\" />\n"
"      <separator name=\"sepToolBarRun\" />\n"
"      <toolitem name=\"RunAbinitio\" action=\"RunAbinitio\" />\n"
"      <separator name=\"sepToolBarGeom\" />\n"
"      <toolitem name=\"GeometryDraw\" action=\"GeometryDraw\" />\n"
"      <toolitem name=\"DisplayDensity\" action=\"DisplayDensity\" />\n"
"      <separator name=\"sepToolBarBabel\" />\n"
/*
"      <toolitem name=\"Open B_abel\" action=\"ToolsOpenBabel\" />\n"
*/
"      <toolitem name=\"ToolsUnitConversion\" action=\"ToolsUnitConversion\" />\n"
"      <separator name=\"sepToolBarExit\" />\n"
"      <toolitem name=\"Exit\" action=\"Exit\" />\n"
"      <separator name=\"sepToolBarAbout\" />\n"
"      <toolitem name=\"HelpAbout\" action=\"HelpAbout\" />\n"
"  </toolbar>\n"
;

/*******************************************************************************************************************************/
static void add_widget (GtkUIManager *merge, GtkWidget   *widget, GtkContainer *container)
{
	GtkWidget *handlebox;

	handlebox =gtk_handle_box_new ();
	g_object_ref (handlebox);
	gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(handlebox),GTK_SHADOW_NONE);
	gtk_box_pack_start (GTK_BOX (container), handlebox, FALSE, FALSE, 0);

	if (GTK_IS_TOOLBAR (widget)) 
	{
		GtkToolbar *toolbar;
		toolbar = GTK_TOOLBAR (widget);
		gtk_toolbar_set_show_arrow (toolbar, TRUE);
		gtk_toolbar_set_style(toolbar, GTK_TOOLBAR_ICONS);
		gtk_toolbar_set_orientation(toolbar,  GTK_ORIENTATION_HORIZONTAL);
		toolBar = toolbar;
		handleBoxToolBar = handlebox;
	}
	gtk_widget_show (widget);
	gtk_container_add (GTK_CONTAINER (handlebox), widget);
}
/*******************************************************************************************************************************/
void add_menu_toolbar()
{
	GtkWidget *box;
  	GtkUIManager *merge;
	GError *error = NULL;
	GtkActionGroup *actionGroup = NULL;

	box = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vboxmain), box, FALSE, FALSE, 0);
	gtk_widget_show (box);

  	merge = gtk_ui_manager_new ();
  	g_signal_connect_swapped (Fenetre, "destroy", G_CALLBACK (g_object_unref), merge);

	actionGroup = gtk_action_group_new ("GabeditActions");
	gtk_action_group_set_translation_domain(actionGroup,GETTEXT_PACKAGE);
	gtk_action_group_add_actions (actionGroup, gtkActionEntries, numberOfGtkActionEntries, NULL);
	gtk_action_group_add_toggle_actions (actionGroup, gtkActionToggleEntries, numberOfGtkActionToggleEntries, NULL);
  	gtk_ui_manager_insert_action_group (merge, actionGroup, 0);



	g_signal_connect (merge, "add_widget", G_CALLBACK (add_widget), box);
	
  	gtk_window_add_accel_group (GTK_WINDOW (Fenetre), gtk_ui_manager_get_accel_group (merge));
	if (!gtk_ui_manager_add_ui_from_string (merge, uiInfo, -1, &error))
	{
		g_message (_("building menus failed: %s"), error->message);
		g_error_free (error);
	}
	manager = merge;
}
/*********************************************************************************************************************/
static void activate_action_window (GtkAction *action, gpointer data)
{
	const gchar *name = gtk_action_get_name (action);

	if(strstr(name,"WinDynamic"))
	{
		GtkWidget* win = (GtkWidget*)data;
		if(GTK_IS_WIDGET(win))
		{
			gtk_widget_hide(win);
			gtk_widget_show(win);
		}
	}
}
/*******************************************************************************************************************************/
void window_add(gchar *str,GtkWidget* Win)
{
	guint  merge_id;
	static GtkActionGroup *dag = NULL;
	GtkActionEntry *actionEntry;
	GtkAction *action;
	gchar* name;
	gchar* label;
	GList*  listOfActions = NULL;
	GList*  list = NULL;
	gint i;
	gchar* tmp;

	if(!manager) return;
	merge_id = gtk_ui_manager_new_merge_id (GTK_UI_MANAGER (manager));

	if(!dag)
	{
		dag = gtk_action_group_new ("WindowActions");
		gtk_action_group_set_translation_domain(dag,GETTEXT_PACKAGE);
		gtk_ui_manager_insert_action_group (manager, dag, 0);
	}
	listOfActions =   gtk_action_group_list_actions(dag);
	i = 0;
	list = listOfActions;
	tmp = g_strdup_printf("WinDynamic%s", str);
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
		name = g_strdup_printf ("WinDynamic%s", str);
		label = g_strdup_printf ("%s", str);
	}
	else
	{
		name = g_strdup_printf ("WinDynamic%s%d", str,i);
		label = g_strdup_printf ("%s%d", str,i);
	}
	actionEntry = g_malloc(sizeof(GtkActionEntry));
	actionEntry->name = name;
	actionEntry->stock_id = NULL;
	actionEntry->label = label;
	actionEntry->accelerator = NULL;
	actionEntry->tooltip = NULL;
	actionEntry->callback = G_CALLBACK(activate_action_window);
        gtk_action_group_add_actions (dag, actionEntry, 1, Win);


	gtk_ui_manager_add_ui(GTK_UI_MANAGER (manager), merge_id, "/menubar/_Window", name, name, GTK_UI_MANAGER_MENUITEM, TRUE);
	g_object_set_data(G_OBJECT(Win),"WinId",GUINT_TO_POINTER(merge_id));
	gtk_ui_manager_ensure_update (manager);

}
/*******************************************************************************************************************************/
void window_delete(GtkWidget* Win)
{
	guint  merge_id;

	if(!manager) return;
	merge_id =  GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (Win), "WinId"));
	gtk_ui_manager_remove_ui(GTK_UI_MANAGER (manager), merge_id);
	gtk_ui_manager_ensure_update (manager);
}
