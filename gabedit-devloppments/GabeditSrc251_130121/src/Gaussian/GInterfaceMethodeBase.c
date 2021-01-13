/* GInterfaceMethodeBase.c */
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Common/Global.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/InterfaceGeom.h"
#include "GInterfaceGeom.h"
#include "GaussGlobal.h"

/************************************************************************************************************/
void  c_basis_presents(gchar *ListAtoms)
{
  GtkWidget *label;
  gchar *tlabel;
  gchar *lwhite;
  gchar *letoile;
  NM=gtk_notebook_get_current_page((GtkNotebook*)NoteBookMB);
  tlabel = g_malloc(1000);
  lwhite = g_malloc(100);
  letoile = g_malloc(100);
  sprintf(lwhite,"\n");
  sprintf(letoile,"*******************************************************\n");
  sprintf(tlabel,_("%s%sThis basis is present for %s atoms\n%s")
                ,lwhite,letoile,ListAtoms,letoile);
  HboxB[2][NM] = create_hbox(VboxB[NM]);
  label = gtk_label_new (tlabel);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (HboxB[2][NM]), label, TRUE, TRUE, 2);
  g_free(tlabel);
  g_free(lwhite);
  g_free(letoile);
}
void  c_polarization(guint nliste)
{
  gchar *liste[20];
  NM=gtk_notebook_get_current_page((GtkNotebook*)NoteBookMB);
  liste[0]=g_strdup(_("No"));
  liste[1]=g_strdup("*");
  liste[2]=g_strdup("(d)");
  liste[3]=g_strdup("**");
  liste[4]=g_strdup("(d,p)");
  liste[5]=g_strdup("(d,pd)");
  liste[6]=g_strdup("(d,2pd)");
  liste[7]=g_strdup("(d,3pd)");
  liste[8]=g_strdup("(df,p)");
  liste[9]=g_strdup("(df,pd)");
  liste[10]=g_strdup("(df,2pd)");
  liste[11]=g_strdup("(df,3pd)");
  liste[12]=g_strdup("(2df,p)");
  liste[13]=g_strdup("(2df,pd)");
  liste[14]=g_strdup("(2df,2pd)");
  liste[15]=g_strdup("(2df,3pd)");
  liste[16]=g_strdup("(3df,p)");
  liste[17]=g_strdup("(3df,pd)");
  liste[18]=g_strdup("(3df,2pd)");
  liste[19]=g_strdup("(3df,3pd)");
  HboxB[1][NM] = create_hbox(VboxB[NM]);
  EntryBasis[1][NM]= create_combo_box_entry_liste(FrameB[NM],HboxB[1][NM],_(" Polarization function :"),liste,nliste);
}
void  c_diffuse_pp(guint nliste)
{
  gchar *liste[3];
  NM=gtk_notebook_get_current_page((GtkNotebook*)NoteBookMB);
  liste[0]=g_strdup(_("No"));
  liste[1]=g_strdup("+");
  liste[2]=g_strdup("++");
  HboxB[0][NM] = create_hbox(VboxB[NM]);
  EntryBasis[0][NM]= create_combo_box_entry_liste(FrameB[NM],HboxB[0][NM],_(" Diffuse function :"),liste,nliste);
}
/********************************************************************************************************/
void  c_diffuse_aug()
{
  gchar *liste[2];
  int nliste ;
  NM=gtk_notebook_get_current_page((GtkNotebook*)NoteBookMB);
  nliste=2;
  liste[0]=g_strdup(_("No"));
  liste[1]=g_strdup(_("Yes"));
  HboxB[1][NM] = create_hbox(VboxB[NM]);
  EntryBasis[1][NM]= create_combo_box_entry_liste(FrameB[NM],HboxB[1][NM],_(" Diffuse function :"),liste,nliste);
}
/********************************************************************************************************/
static void traite_basis (GtkComboBox *combobox, gpointer d)
{
	guint i;
	GtkTreeIter iter;
	gchar* data = NULL;
	NM=gtk_notebook_get_current_page((GtkNotebook*)NoteBookMB);
	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	if(Basis[NM]) g_free(Basis[NM]);
	Basis[NM] = NULL;
  	if ( strcmp((char *)data,_("Please select your basis")) && strcmp((char *)data,_("None")) ) 
		Basis[NM] =g_strdup((char *)data);

	for (i=0;i<nHboxB;i++)
	{
		if(HboxB[i][NM]!=NULL)
		{
			gtk_widget_destroy(HboxB[i][NM]);
			if(EntryBasis[i][NM]!=NULL) EntryBasis[i][NM]=NULL;
			HboxB[i][NM] = NULL;
		}
	} 
	if (!strcmp((char *)data,"STO-3G") ) { c_polarization(2); c_basis_presents("H-Xe"); }
	else if (!strcmp((char *)data,"3-21G") ) { c_diffuse_pp(2); c_polarization(4); c_basis_presents("H-Xe"); }
	else if (!strcmp((char *)data,"6-21G") ) { c_polarization(3); c_basis_presents("H-Cl"); }
	else if (!strcmp((char *)data,"4-31G") ) { c_diffuse_pp(3); c_polarization(5); c_basis_presents("H-Ne"); }
	else if (!strcmp((char *)data,"6-31G") ) { c_diffuse_pp(3); c_polarization(20); c_basis_presents("H-Cl"); }
	else if (!strcmp((char *)data,"6-311G") ) { c_diffuse_pp(3); c_polarization(20); c_basis_presents("H-Kr"); }
	else if (!strcmp((char *)data,"D95") ) { c_diffuse_pp(3); c_polarization(20); c_basis_presents("H-Cl"); }
	else if (!strcmp((char *)data,"D95V") ) { c_diffuse_pp(3); c_polarization(5); c_basis_presents("H-Ne"); }
	else if (!strcmp((char *)data,"SHC") ) { c_diffuse_pp(3); c_polarization(20); c_basis_presents("H-Cl"); }
	else if (!strcmp((char *)data,"CEP-4G") ) { c_diffuse_pp(3); c_polarization(20); c_basis_presents("H-Cl"); }
	else if (!strcmp((char *)data,"CEP-31G") ) { c_diffuse_pp(3); c_polarization(20); c_basis_presents("H-Cl"); }
	else if (!strcmp((char *)data,"CEP-121G") ) { c_diffuse_pp(3); c_polarization(20); c_basis_presents("H-Cl"); }
	else if (!strcmp((char *)data,"LANL2MB") ) { c_basis_presents("H-Ba,La-Bi"); }
	else if (!strcmp((char *)data,"LANL2DZ") ) { c_basis_presents("H,Li-Ba,La-Bi"); }
	else if (!strcmp((char *)data,"cc-pVDZ") ) { c_diffuse_aug(); c_basis_presents("H-He,B-Ne"); }
	else if (!strcmp((char *)data,"cc-pVTZ") ) { c_diffuse_aug(); c_basis_presents("H-He,B-Ne"); }
	else if (!strcmp((char *)data,"cc-pVOZ") ) { c_diffuse_aug(); c_basis_presents("H-He,B-Ne"); }
	else if (!strcmp((char *)data,"cc-pV5Z") ) { c_diffuse_aug(); c_basis_presents("H,B-Ne"); }
/*
	else if (!strcmp((char *)data,"ccl-pVDZ") ) { c_diffuse_aug(); c_basis_presents("Al-Ar"); }
	else if (!strcmp((char *)data,"ccl-pVTZ") ) { c_diffuse_aug(); c_basis_presents("Al-Ar"); }
*/
	else if (!strcmp((char *)data,"cc-pV6Z") ) { c_diffuse_aug(); c_basis_presents("H,B-Ne"); }
	else if (!strcmp((char *)data,"SDD") ) { c_basis_presents("All"); }
	else if (!strcmp((char *)data,"SV") ) { c_basis_presents("H-Kr"); }
	else if (!strcmp((char *)data,"SVP") ) { c_basis_presents("H-Kr"); }
	else if (!strcmp((char *)data,"TZV") ) { c_polarization(20); c_basis_presents("H-Kr"); }
	else if (!strcmp((char *)data,"MIDIX") ) { c_basis_presents("H,C,N,O,F,P,S,Cl"); }
	 gtk_widget_show_all(FrameB[NM]);
}
/********************************************************************************************************/
static GtkWidget *create_liste_basis(GtkWidget*win,GtkWidget *frame)
{
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *hseparator;
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;

	vbox =create_vbox(frame);
	VboxB[NM]=vbox;
	hbox =create_hbox(vbox);

	store = gtk_tree_store_new (1,G_TYPE_STRING);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("None"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "STO-3G", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "3-21G", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "4-31G", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "6-21G", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "6-31G", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "6-311G", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "D95", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "D95V", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "SHC", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "CEP-4G", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "CEP-31G", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "CEP-121G", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "LANL2MB", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "LANL2DZ", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "SDD", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "cc-pVDZ", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "cc-pVTZ", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "cc-pVOZ", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "cc-pV5Z", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "cc-pV6Z", -1);
        gtk_tree_store_append (store, &iter, NULL);
/*
        gtk_tree_store_set (store, &iter, 0, "ccl-pVDZ", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "ccl-pVTZ", -1);
        gtk_tree_store_append (store, &iter, NULL);
*/
        gtk_tree_store_set (store, &iter, 0, "SV", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "SVP", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "TZV", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "MIDIX", -1);

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	g_object_unref (model);
	gtk_box_pack_start (GTK_BOX (hbox), combobox, TRUE, TRUE, 1);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traite_basis), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	hseparator = gtk_hseparator_new ();
	gtk_widget_show (hseparator);
	gtk_box_pack_start (GTK_BOX (vbox), hseparator, FALSE, FALSE, 1);
	return combobox;
}
/********************************************************************************************************/
void c_hf_combo()
{
  gchar *liste[9];
  int nliste = 0;
  NM=gtk_notebook_get_current_page((GtkNotebook*)NoteBookMB);
  nliste=3;
  liste[0]=g_strdup("HF");
  liste[1]=g_strdup("UHF");
  liste[2]=g_strdup("ROHF");
  HboxM[0][NM] = create_hbox(VboxM[NM]);
  EntryMethods[0][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[0][NM],_(" Select your method :"),liste,nliste);
}
/********************************************************************************************************/
void  c_ci_combo()
{
  gchar *liste[9];
  int nliste = 0;
  NM=gtk_notebook_get_current_page((GtkNotebook*)NoteBookMB);
  nliste=6;
  liste[0]=g_strdup("CISD");
  liste[1]=g_strdup("QCISD");
  liste[2]=g_strdup("CCD");
  liste[2]=g_strdup("CCSD(T)");
  liste[3]=g_strdup("QCISD(T)");
  liste[4]=g_strdup("QCISD(TQ)");
  liste[5]=g_strdup("CCDSD");
  HboxM[0][NM] = create_hbox(VboxM[NM]);
  EntryMethods[0][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[0][NM],_(" Select your method :"),liste,nliste);
  nliste=2;
  liste[0]=g_strdup("FC");
  liste[1]=g_strdup("FULL");
  HboxM[1][NM] = create_hbox(VboxM[NM]);
  EntryMethods[1][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[1][NM],_(" correlation  :"),liste,nliste);
  nliste=3;
  liste[0]=g_strdup("7");
  liste[1]=g_strdup("6");
  liste[2]=g_strdup("8");
 HboxM[2][NM] = create_hbox(VboxM[NM]);
  EntryMethods[2][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[2][NM],_(" Convergence  :"),liste,nliste);
  nliste=3;
  liste[0]=g_strdup("20");
  liste[1]=g_strdup("30");
  liste[2]=g_strdup("40");
  HboxM[3][NM] = create_hbox(VboxM[NM]);
  EntryMethods[3][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[3][NM],_(" Number of cycles  :"),liste,nliste);
}
/********************************************************************************************************/
void  c_mp_combo()
{
  gchar *liste[9];
  int nliste = 0;
  NM=gtk_notebook_get_current_page((GtkNotebook*)NoteBookMB);
  nliste=5;
  liste[0]=g_strdup("MP2");
  liste[1]=g_strdup("MP3");
  liste[2]=g_strdup("MP4(SDQ)");
  liste[3]=g_strdup("MP4(SDTQ)");
  liste[4]=g_strdup("MP5");
  HboxM[0][NM] = create_hbox(VboxM[NM]);
  EntryMethods[0][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[0][NM],_(" Select your method :"),liste,nliste);
  nliste=2;
  liste[0]=g_strdup("FC");
  liste[1]=g_strdup("FULL");
  HboxM[1][NM] = create_hbox(VboxM[NM]);
  EntryMethods[1][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[1][NM],_(" correlation  :"),liste,nliste);
  nliste=5;
  liste[0]=g_strdup(_("default"));
  liste[1]=g_strdup("FullDirect");
  liste[2]=g_strdup("SemiDirect");
  liste[3]=g_strdup("Direct");
  liste[4]=g_strdup("InCore");
 HboxM[2][NM] = create_hbox(VboxM[NM]);
  EntryMethods[2][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[2][NM],_(" Options  :"),liste,nliste);
}
/********************************************************************************************************/
void c_semi_combo()
{
  gchar *liste[9];
  int nliste = 0;
  NM=gtk_notebook_get_current_page((GtkNotebook*)NoteBookMB);
  nliste=7;
  liste[0]=g_strdup("CNDO");
  liste[1]=g_strdup("INDO");
  liste[2]=g_strdup("MNDO");
  liste[3]=g_strdup("MNDO3");
  liste[4]=g_strdup("PM3");
  liste[5]=g_strdup("PM3MM");
  liste[6]=g_strdup("AM1");
  HboxM[0][NM] = create_hbox(VboxM[NM]);
  EntryMethods[0][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[0][NM],_(" Select your method :"),liste,nliste);
}
/********************************************************************************************************/
void c_mecha_combo()
{
  gchar *liste[9];
  int nliste = 0;
  NM=gtk_notebook_get_current_page((GtkNotebook*)NoteBookMB);
  nliste=3;
  liste[0]=g_strdup("AMBER");
  liste[1]=g_strdup("DREIDING");
  liste[2]=g_strdup("UFF");
  HboxM[0][NM] = create_hbox(VboxM[NM]);
  EntryMethods[0][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[0][NM],_(" Select your method :"),liste,nliste);
}
/********************************************************************************************************/
void  c_hybrid_combo()
{
  gchar *liste[35];
  int nliste = 0;
  NM=gtk_notebook_get_current_page((GtkNotebook*)NoteBookMB);
  nliste=35;
  liste[0]=g_strdup("B3LYP");
  liste[1]=g_strdup("B3P86");
  liste[2]=g_strdup("B3PW91");
  liste[3]=g_strdup("B1B96");
  liste[4]=g_strdup("B1LYP");
  liste[5]=g_strdup("MPW1PW91");
  liste[6]=g_strdup("G961LYP");
  liste[7]=g_strdup("BHandH");
  liste[8]=g_strdup("BHandHLYP");
  liste[9]=g_strdup("CAM-B3LYP");
  liste[10]=g_strdup("LC-wPBE");
  liste[11]=g_strdup("wB97X");
  liste[12]=g_strdup("wB97XD");
  liste[13]=g_strdup("PBE1PBE");
  liste[14]=g_strdup("HSEh1PBE");
  liste[15]=g_strdup("HSE2PBE");
  liste[16]=g_strdup("HSE1PBE");
  liste[17]=g_strdup("PBEh1PBE");
  liste[18]=g_strdup("O3LYP");
  liste[19]=g_strdup("TPSSh");
  liste[20]=g_strdup("BMK");
  liste[21]=g_strdup("M06");
  liste[22]=g_strdup("M06HF");
  liste[23]=g_strdup("M062X");
  liste[24]=g_strdup("M05");
  liste[25]=g_strdup("M052X");
  liste[26]=g_strdup("X3LYP");
  liste[27]=g_strdup("VSXC");
  liste[28]=g_strdup("HCTH");
  liste[29]=g_strdup("HCTH93");
  liste[30]=g_strdup("HCTH147");
  liste[31]=g_strdup("HCTH407");
  liste[32]=g_strdup("tHCTH");
  liste[33]=g_strdup("M06L");
  liste[34]=g_strdup("B97D");
  HboxM[0][NM] = create_hbox(VboxM[NM]);
  EntryMethods[0][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[0][NM],_(" Select your method :"),liste,nliste);
}
/********************************************************************************************************/
void c_dft_combo()
{
  gchar *liste[14];
  int nliste = 0;
  NM=gtk_notebook_get_current_page((GtkNotebook*)NoteBookMB);
  nliste=13;
  liste[0]=g_strdup("B");
  liste[1]=g_strdup("S");
  liste[2]=g_strdup("XA");
  liste[3]=g_strdup("PW91");
  liste[4]=g_strdup("MPW");
  liste[5]=g_strdup("G96");
  liste[6]=g_strdup("PBE");
  liste[7]=g_strdup("O");
  liste[8]=g_strdup("TPSS");
  liste[9]=g_strdup("BRx");
  liste[10]=g_strdup("PKZB");
  liste[11]=g_strdup("wPBEh");
  liste[12]=g_strdup("PBEh");
  HboxM[0][NM] = create_hbox(VboxM[NM]);
  EntryMethods[0][NM] = create_combo_box_entry_liste(FrameM[NM],HboxM[0][NM],_(" Exchange Functional :"),liste,nliste);
  nliste=14;
  liste[0]=g_strdup("LYP");
  liste[1]=g_strdup("VWN");
  liste[2]=g_strdup("VWN5");
  liste[3]=g_strdup("PL");
  liste[4]=g_strdup("P86");
  liste[5]=g_strdup("PW91");
  liste[6]=g_strdup("B95");
  liste[7]=g_strdup("PBE");
  liste[8]=g_strdup("TPSS");
  liste[9]=g_strdup("KCIS");
  liste[10]=g_strdup("BRC");
  liste[11]=g_strdup("PKZB");
  liste[12]=g_strdup("VP86");
  liste[13]=g_strdup("V5LYP");
  HboxM[1][NM] = create_hbox(VboxM[NM]);
  EntryMethods[1][NM] = create_combo_box_entry_liste(FrameM[NM],HboxM[1][NM],_(" Correlation Functionals :"),liste,nliste);

  nliste=2;
  liste[0]=g_strdup("None");
  liste[1]=g_strdup("LC-");
  HboxM[2][NM] = create_hbox(VboxM[NM]);
  EntryMethods[2][NM] = create_combo_box_entry_liste(FrameM[NM],HboxM[2][NM],_(" Corrections :"),liste,nliste);
}
/********************************************************************************************************/
void  c_excited_combo()
{
  gchar *liste[9];
  int nliste = 0;
  NM=gtk_notebook_get_current_page((GtkNotebook*)NoteBookMB);
  nliste=3;
  liste[0]=g_strdup("CIS");
  liste[1]=g_strdup("RPA");
  liste[2]=g_strdup("TDDFT");
  HboxM[0][NM] = create_hbox(VboxM[NM]);
  EntryMethods[0][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[0][NM],_(" Select your method :"),liste,nliste);
  nliste=3;
  liste[0]=g_strdup("Singlets");
  liste[1]=g_strdup("Triplets");
  liste[2]=g_strdup("50-50");
  HboxM[1][NM] = create_hbox(VboxM[NM]);
  EntryMethods[1][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[1][NM],_(" Type  :"),liste,nliste);
  nliste=3;
  liste[0]=g_strdup(_("default"));
  liste[1]=g_strdup("3");
  liste[2]=g_strdup("6");
 HboxM[2][NM] = create_hbox(VboxM[NM]);
  EntryMethods[2][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[2][NM],_(" NStates  :"),liste,nliste);
  nliste=4;
  liste[0]=g_strdup(_("default"));
  liste[1]=g_strdup("1");
  liste[2]=g_strdup("2");
  liste[3]=g_strdup("3");
  HboxM[3][NM] = create_hbox(VboxM[NM]);
  EntryMethods[3][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[3][NM],_(" Root  :"),liste,nliste);
}
/********************************************************************************************************/
void  c_casscf_combo()
{
  gchar *liste[9];
  int nliste = 0;
  NM=gtk_notebook_get_current_page((GtkNotebook*)NoteBookMB);
  nliste=1;
  liste[0]=g_strdup(" ");
  HboxM[0][NM] = create_hbox(VboxM[NM]);
  EntryMethods[0][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[0][NM],_(" Number of electrons :"),liste,nliste);
  nliste=1;
  liste[0]=g_strdup(" ");
  HboxM[1][NM] = create_hbox(VboxM[NM]);
  EntryMethods[1][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[1][NM],_(" Number of active orbitals :"),liste,nliste);
  nliste=2;
  liste[1]=g_strdup(_("Yes"));
  liste[0]=g_strdup(_("No"));
  HboxM[2][NM] = create_hbox(VboxM[NM]);
  EntryMethods[2][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[2][NM],_(" With MP2 correction ? :"),liste,nliste);
  nliste=2;
  liste[1]=g_strdup(_("Yes"));
  liste[0]=g_strdup(_("No"));
  HboxM[3][NM] = create_hbox(VboxM[NM]);
  EntryMethods[3][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[3][NM],_(" With Spin Orbit correction ? :"),liste,nliste);
  nliste=2;
  liste[1]=g_strdup(_("Yes"));
  liste[0]=g_strdup(_("No"));
  HboxM[4][NM] = create_hbox(VboxM[NM]);
  EntryMethods[4][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[4][NM],_(" Conical intersections ? :"),liste,nliste);
  nliste=5;
  liste[0]=g_strdup("1");
  liste[1]=g_strdup("2");
  liste[2]=g_strdup("3");
  liste[3]=g_strdup("4");
  liste[4]=g_strdup("5");
  HboxM[5][NM] = create_hbox(VboxM[NM]);
  EntryMethods[5][NM]= create_combo_box_entry_liste(FrameM[NM],HboxM[5][NM],_(" Number of states  :"),liste,nliste);
}
/********************************************************************************************************/
static void traite_methods (GtkComboBox *combobox, gpointer d)
{
	guint i;
	GtkTreeIter iter;
	gchar* data = NULL;
	NM=gtk_notebook_get_current_page((GtkNotebook*)NoteBookMB);
	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	if(Methodes[NM]) g_free(Methodes[NM]);
	Methodes[NM] = NULL;
	if (strcmp((char *)data,_("Nothing"))) Methodes[NM]=g_strdup((char*)data);
	for (i=0;i<nHboxM;i++)
	{
		if(NM>=0 && HboxM[i][NM]!=NULL)
		{
			gtk_widget_destroy(HboxM[i][NM]);
			if(EntryMethods[i][NM]!=NULL)
			EntryMethods[i][NM]=NULL;
			HboxM[i][NM] = NULL;
		}
	} 
	if (!strcmp((char *)data,_("Hartree Fock")) ) c_hf_combo();
	else if (!strcmp((char *)data,_("CI Methods")) ) c_ci_combo();
	else if (!strcmp((char *)data,_("CASSCF Method")) ) c_casscf_combo();
	else if (!strcmp((char *)data,_("MP Methods")) ) c_mp_combo ();
	else if (!strcmp((char *)data,_("Semi-Empirical Methods")) ) c_semi_combo();
	else if (!strcmp((char *)data,_("Molecular Mechanics Methods")) ) c_mecha_combo();
	else if (!strcmp((char *)data,_("Hybrid/Standalone Functional Methods")) ) c_hybrid_combo();
	else if (!strcmp((char *)data,_("Density Functional Methods")) ) c_dft_combo();
	else if (!strcmp((char *)data,_("Excited States")) ) c_excited_combo();
	gtk_widget_show_all(FrameM[NM]);
}
/********************************************************************************************************/
static GtkWidget *create_liste_methods(GtkWidget*win,GtkWidget *frame, gboolean nothing)
{
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *hseparator;
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;

	vbox =create_vbox(frame);
	VboxM[NM]=vbox;
	hbox =create_hbox(vbox);

	store = gtk_tree_store_new (1,G_TYPE_STRING);
	if(nothing)
	{
        	gtk_tree_store_append (store, &iter, NULL);
        	gtk_tree_store_set (store, &iter, 0, _("Nothing"), -1);
	}
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Hartree Fock"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("MP Methods"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("CI Methods"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("CASSCF Method"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Density Functional Methods"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Hybrid/Standalone Functional Methods"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Excited States"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Semi-Empirical Methods"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Molecular Mechanics Methods"), -1);

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	g_object_unref (model);
	gtk_box_pack_start (GTK_BOX (hbox), combobox, TRUE, TRUE, 1);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traite_methods), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	hseparator = gtk_hseparator_new ();
	gtk_widget_show (hseparator);
	gtk_box_pack_start (GTK_BOX (vbox), hseparator, FALSE, FALSE, 1);
	return combobox;
}
/********************************************************************************************************/
static void GAjoutePageMB(GtkWidget *NoteBook,gchar * tNote,guint j, GtkWidget **comboMethod, GtkWidget **comboBasis)
{
  GtkWidget *Frame;
  GtkWidget *LabelOnglet;
  GtkWidget *LabelMenu;
  GtkWidget *window1;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *FrameMethode;
  GtkWidget *FrameBasis;
  guint i;
  GtkWidget *combobox;
  gboolean nothing = TRUE;

  nothing = FALSE;
  
  NM=j;
  Basis[NM] =NULL;
  Methodes[NM]=NULL;
  for(i=0;i<nHboxM;i++)
  {
  	HboxM[i][j]=NULL;
        EntryMethods[i][j]=NULL;
  }
  for(i=0;i<nHboxB;i++)
  {
  	HboxB[i][j]=NULL;
  	EntryBasis[i][j]=NULL;
  }
  Frame = gtk_frame_new(NULL);

  LabelOnglet = gtk_label_new(tNote);
  LabelMenu = gtk_label_new(tNote);
  gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook), Frame, LabelOnglet, LabelMenu);

  window1 = Frame;
  g_object_set_data(G_OBJECT (window1), "window1", window1);

  vbox =create_vbox(window1);

  hbox =create_hbox(vbox);

  FrameMethode =create_vbox(hbox);
  gtk_container_set_border_width(GTK_CONTAINER(FrameMethode), 2);
  FrameM[NM]=FrameMethode;
  combobox = create_liste_methods(window1,FrameMethode, nothing);
  *comboMethod = combobox;

  FrameBasis =create_vbox(hbox);
  gtk_container_set_border_width(GTK_CONTAINER(FrameBasis), 2);
  FrameB[NM]=FrameBasis;
  combobox = create_liste_basis(window1,FrameBasis);
  *comboBasis = combobox;

}

/********************************************************************************************/
void create_notebook_MB(GtkWidget *frame)
{
	GtkWidget *vbox; 
	GtkWidget* comboMethodHight = NULL;
	GtkWidget* comboMethodMedium = NULL;
	GtkWidget* comboMethodLower = NULL;

	GtkWidget* comboBasisHight = NULL;
	GtkWidget* comboBasisMedium = NULL;
	GtkWidget* comboBasisLower = NULL;

	gboolean medium = geometry_with_medium_layer();
	gboolean lower = geometry_with_lower_layer();
	gint ilower = 2;
	gint i,j;

	nHboxM=NHBOXM_MAX;
	nHboxB=NHBOXB_MAX;

	for(i=0;i<nHboxB;i++)
	{
		for(j=0;j<3;j++)
		{
  			HboxB[i][j]=NULL;
  			EntryBasis[i][j]=NULL;
		}
	}
	for(j=0;j<3;j++) Methodes[j] = NULL;

	vbox =create_vbox(frame);
	NoteBookMB = gtk_notebook_new();
	gtk_notebook_set_tab_pos( GTK_NOTEBOOK(NoteBookMB),GTK_POS_LEFT);
	gtk_box_pack_start(GTK_BOX (vbox), NoteBookMB,FALSE, FALSE, 0);
	if(medium || lower) GAjoutePageMB(NoteBookMB,"High",0, &comboMethodHight, &comboBasisHight); 
	else GAjoutePageMB(NoteBookMB,NULL,0, &comboMethodHight, &comboBasisHight); 
	if(medium) GAjoutePageMB(NoteBookMB,"Medium",1, &comboMethodMedium, &comboBasisMedium); 
	if(lower) 
	{
		if(!medium) ilower = 1;
		GAjoutePageMB(NoteBookMB,"Lower",ilower, &comboMethodLower, &comboBasisLower); 
	}
  	gtk_widget_show_all (NoteBookMB);
	gtk_notebook_set_current_page((GtkNotebook*)NoteBookMB, 0);
	gtk_combo_box_set_active (GTK_COMBO_BOX (comboMethodHight), 5);
	gtk_combo_box_set_active (GTK_COMBO_BOX (comboBasisHight), 6);
	if(comboMethodMedium)
	{
		gtk_notebook_set_current_page((GtkNotebook*)NoteBookMB, 1);
		gtk_combo_box_set_active (GTK_COMBO_BOX (comboMethodMedium), 0);
		gtk_combo_box_set_active (GTK_COMBO_BOX (comboBasisMedium), 2);
	}
	if(comboBasisLower)
	{
		gtk_notebook_set_current_page((GtkNotebook*)NoteBookMB, ilower);
		gtk_combo_box_set_active (GTK_COMBO_BOX (comboMethodLower), 8);
		gtk_combo_box_set_active (GTK_COMBO_BOX (comboBasisLower), 0);
	}
	gtk_notebook_set_current_page((GtkNotebook*)NoteBookMB, 0);
}
