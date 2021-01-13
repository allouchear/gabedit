/* GInterfaceBasis.c */
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
#include "../Utils/UtilsInterface.h"
#include "../Geometry/GeomGlobal.h"
#include "GaussGlobal.h"
#include "GInterfaceBasis.h"

#define NC 3
#define NHboxBasis 3
/* extern GInterfaceBasis.h */
BasisType *BasisT;
BasisCenter *BasisC;

gint  NSymbTypes;
gint  NSymbCenters;
/**************************************************************************************************/

static gchar selectedRowForCenter[100]="-1";
static GtkWidget *listC;
static gchar selectedRowForType[100]="-1";
static GtkWidget *listT;
static GtkWidget *HboxBasis[NHboxBasis];
static GtkWidget *VboxBasis;
static GtkWidget *FrameBasis;
static GtkWidget *EntryB[NHboxBasis];
static gchar *BasisName;

static void DialogueEditT(GtkWidget *w,gpointer data);
static void DialogueEditC(GtkWidget *w,gpointer data);

/*************************************************************************************************/
static void selectRow(GtkWidget* list, gint row)
{
	GtkTreePath *path;
	gchar* tmp = g_strdup_printf("%d",row);

	path = gtk_tree_path_new_from_string  (tmp);
	g_free(tmp);
	gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (list)), path);
	gtk_tree_path_free(path);
}
/*************************************************************************************************/
static gchar *get_info_basis()
{
       	gchar *temp;
       	gchar **tmp;
  	G_CONST_RETURN gchar *entrytext;
        gint nentry=0;
        gboolean Modify=FALSE;
        
        if(EntryB[0]!=NULL)nentry++;
        if(EntryB[1]!=NULL)nentry++;
        tmp = g_strsplit(BasisName,"G",2);

        temp = g_strdup(tmp[0]);
  	if (strcmp(temp,BasisName) )
             Modify=TRUE;
        if(EntryB[0]!=NULL)
        {
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryB[0]));
  	if (strcmp(entrytext,_("No")) )
           temp=g_strdup_printf("%s%s",temp,entrytext);
        }
  	if (Modify)
        	temp = g_strdup_printf("%sG",temp);
        if(EntryB[1]!=NULL)
        {
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryB[1]));
  	if (strcmp(entrytext,_("No")) )
        {
  		if (strcmp(entrytext,_("Yes")) )
           		temp=g_strdup_printf("%s%s",temp,entrytext);
                else
           		temp=g_strdup_printf("AUG-%s",temp);
        }
        }

       return temp;
}
/*************************************************************************************************/
static void  c_basis_presents(gchar *ListAtoms)
{
  GtkWidget *label;
  gchar *tlabel;
  gchar *lwhite;
  gchar *letoile;
  tlabel = g_malloc(1000);
  lwhite = g_malloc(100);
  letoile = g_malloc(100);
  sprintf(lwhite,"\n");
  sprintf(letoile,"*******************************************************\n");
  sprintf(tlabel,_("%s%sThis basis is present for %s atoms\n%s")
                ,lwhite,letoile,ListAtoms,letoile);
  HboxBasis[2] = create_hbox(VboxBasis);
  label = gtk_label_new (tlabel);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (HboxBasis[2]), label, TRUE, TRUE, 2);
  g_free(tlabel);
  g_free(lwhite);
  g_free(letoile);
}
/*************************************************************************************************/
static void  c_polarization(guint nliste)
{
  gchar *liste[20];
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
  HboxBasis[1] = create_hbox(VboxBasis);
  EntryB[1]= create_combo_box_entry_liste(FrameBasis,HboxBasis[1],_(" Polarization function :"),liste,nliste);
}
/*************************************************************************************************/
static void  c_diffuse_pp(guint nliste)
{
  gchar *liste[3];
  liste[0]=g_strdup(_("No"));
  liste[1]=g_strdup("+");
  liste[2]=g_strdup("++");
  HboxBasis[0] = create_hbox(VboxBasis);
  EntryB[0]= create_combo_box_entry_liste(FrameBasis,HboxBasis[0],_(" Diffuse function :"),liste,nliste);
}
/*************************************************************************************************/
static void  c_diffuse_aug()
{
  gchar *liste[2];
  int nliste ;
  nliste=2;
  liste[0]=g_strdup(_("No"));
  liste[1]=g_strdup(_("Yes"));
  HboxBasis[1] = create_hbox(VboxBasis);
  EntryB[1]= create_combo_box_entry_liste(FrameBasis,HboxBasis[1],_(" Diffuse function :"),liste,nliste);
}
/********************************************************************************************************/
static void traite_basis (GtkComboBox *combobox, gpointer d)
{
	guint i;
	GtkTreeIter iter;
	gchar* data = NULL;


	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	if(BasisName) g_free(BasisName);
	BasisName = NULL;
  	if ( strcmp((char *)data,_("None(for DUMMY Center)")) ) 
		BasisName =g_strdup((char *)data);
	else
		BasisName =g_strdup("Noselect");

	for (i=0;i<NHboxBasis;i++)
	{
		if(HboxBasis[i]!=NULL) gtk_widget_destroy(HboxBasis[i]);
		EntryB[i]=NULL;
		HboxBasis[i] = NULL;
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
	gtk_widget_show_all(FrameBasis);
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
	VboxBasis=vbox;
	FrameBasis=frame;
	hbox =create_hbox(vbox);

	store = gtk_tree_store_new (1,G_TYPE_STRING);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("None(for DUMMY Center)"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "STO-3G", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "3-21G", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "6-21G", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "4-31G", -1);
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
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 1);
	return combobox;
}
/*************************************************************************************************/
void  create_tab_type()
{
 gint i;
 gint j;
 gboolean OK;

 if(NSymbCenters>0)
 {
 	NSymbTypes=1;
 	BasisT=g_malloc(sizeof(BasisType));
 	BasisT[NSymbTypes-1].CenterName = g_strdup(BasisC[0].CenterName);
 	BasisT[NSymbTypes-1].BasisName  =g_strdup(BasisC[0].BasisName); 
 	BasisT[NSymbTypes-1].Layer  =g_strdup(BasisC[0].Layer); 
 }
 else
 return;

 for(i=1;i<NSymbCenters;i++)
 {
 OK=TRUE;
  for (j=0;j<i;j++)
    if( !strcmp(BasisC[i].CenterName,BasisC[j].CenterName) && !strcmp(BasisC[i].Layer,BasisC[j].Layer) )
    {
      OK=FALSE;
      break;
    }
  if(!OK)continue;
  NSymbTypes++;
  BasisT=g_realloc(BasisT,NSymbTypes*sizeof(BasisType));
  BasisT[NSymbTypes-1].CenterName = g_strdup(BasisC[i].CenterName);
  BasisT[NSymbTypes-1].BasisName  = g_strdup(BasisC[i].BasisName); 
  BasisT[NSymbTypes-1].Layer  = g_strdup(BasisC[i].Layer); 
 }


}
/*************************************************************************************************/
void  create_tabs_list()
{
 guint Ncenters;
 guint i;
 gboolean OK[3];
 BasisCenter Tb;

 for(i=0;i<3;i++)
 	if(Methodes[i]!=NULL && Basis[i]!=NULL && !strcmp(Basis[i],"GEN") )
		OK[i]=TRUE;
        else
		OK[i]=FALSE;

 NSymbCenters=0;
 if( !OK[0] && !OK[1] && !OK[2] )
 {
 	NSymbTypes=0;
	return;
 }
  if(MethodeGeom == GEOM_IS_ZMAT)
    Ncenters = NcentersZmat;
  else
    Ncenters = NcentersXYZ;

 for(i=0;i<Ncenters;i++)
 {
 NSymbCenters++;
 if(NSymbCenters==1)
 	BasisC=g_malloc(sizeof(BasisCenter));
 else
 	BasisC=g_realloc(BasisC,NSymbCenters*sizeof(BasisCenter));
	
  if(MethodeGeom == GEOM_IS_ZMAT)
  {
   Tb.CenterName=g_strdup(Geom[i].Symb);	
   Tb.Layer=g_strdup(Geom[i].Layer);
  }
  else
  {
   Tb.CenterName=g_strdup(GeomXYZ[i].Symb);	
   Tb.Layer=g_strdup(GeomXYZ[i].Layer);
  }

 Tb.BasisName=g_strdup("STO-3G");
 if( (int)Tb.CenterName[0] == (int)'x' || (int)Tb.CenterName[0] == (int)'X' )
	Tb.BasisName=g_strdup(_("None"));
 Tb.Changed=FALSE;
 Tb.Number=i+1;
 if (OK[0] )
	BasisC[NSymbCenters-1]=Tb;
 else
	NSymbCenters--;


  if(OK[1] && ( !strcmp(Tb.Layer,"High") || !strcmp(Tb.Layer," ") ) )
  {
  	NSymbCenters++;
 	BasisC=g_realloc(BasisC,NSymbCenters*sizeof(BasisCenter));
 	BasisC[NSymbCenters-1]=Tb;
   	BasisC[NSymbCenters-1].Layer=g_strdup("Medium");
  }

  if(OK[2] && ( !strcmp(Tb.Layer,"High") || !strcmp(Tb.Layer," ") ) )
  {
  	NSymbCenters++;
 	BasisC=g_realloc(BasisC,NSymbCenters*sizeof(BasisCenter));
 	BasisC[NSymbCenters-1]=Tb;
   	BasisC[NSymbCenters-1].Layer=g_strdup("Low");
  }
  if(OK[1] && !strcmp(Tb.Layer,"Medium")  )
  {
  	NSymbCenters++;
 	BasisC=g_realloc(BasisC,NSymbCenters*sizeof(BasisCenter));
 	BasisC[NSymbCenters-1]=Tb;
   	BasisC[NSymbCenters-1].Layer=g_strdup("Medium");
  }
  if(OK[2] && !strcmp(Tb.Layer,"Medium")  )
  {
  	NSymbCenters++;
 	BasisC=g_realloc(BasisC,NSymbCenters*sizeof(BasisCenter));
 	BasisC[NSymbCenters-1]=Tb;
   	BasisC[NSymbCenters-1].Layer=g_strdup("Low");
  }
  if(OK[2] && !strcmp(Tb.Layer,"Low")  )
  {
  	NSymbCenters++;
 	BasisC=g_realloc(BasisC,NSymbCenters*sizeof(BasisCenter));
 	BasisC[NSymbCenters-1]=Tb;
   	BasisC[NSymbCenters-1].Layer=g_strdup("Low");
  }

 }

  create_tab_type();

}
/***********************************************************************************/
static void EditBasisC(GtkWidget *w,gpointer Entree)
{
       	gchar *texts[4];
       	gchar *temp;
        gint Nc;
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;
	gint k;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(listC));
        store = GTK_LIST_STORE (model);


        Nc=atoi(selectedRowForCenter);
        temp = get_info_basis();
  	if (!strcmp(temp,"Noselect") ) return;
  	if (strcmp(temp,BasisC[Nc].BasisName) )
        {
 		BasisC[Nc].Changed=TRUE;
        	BasisC[Nc].BasisName=g_strdup(temp);
  		texts[0] = g_strdup_printf("%d",BasisC[Nc].Number);
  		texts[1] = g_strdup(BasisC[Nc].CenterName);
  		texts[2] = g_strdup(BasisC[Nc].BasisName);
  		texts[3] = g_strdup(BasisC[Nc].Layer);
		if(gtk_tree_model_get_iter_from_string (model, &iter, selectedRowForCenter)) gtk_list_store_remove(store, &iter);
		gtk_list_store_insert(store, &iter, Nc);
		for(k=0;k<4;k++) gtk_list_store_set (store, &iter, k, texts[k], -1);
        }
	BasisName=g_strdup("Noselect");
	selectRow(listC, Nc);
	sprintf(selectedRowForCenter,"%d",Nc);
}
/***********************************************************************************/
static void DialogueEditC(GtkWidget *w,gpointer data)
{
  GtkWidget *Dialogue;
  GtkWidget *Bouton;
  GtkWidget *frame;
  gint Nc;
  guint i;
  
  Nc=atoi(selectedRowForCenter);
  if(Nc<0 ) {
   	Message(_("Please Select your center\n")," Warning ",TRUE);
   return;
  }
  Dialogue = gtk_dialog_new();
  gtk_window_set_modal(GTK_WINDOW(Dialogue),TRUE);
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);
  gtk_window_set_title(GTK_WINDOW(Dialogue),_("Basis"));
  frame = create_frame(GTK_WIDGET(Dialogue),GTK_DIALOG(Dialogue)->vbox,_("Title"));
  for (i=0;i<NHboxBasis;i++)
  {
	EntryB[i]=NULL;
	HboxBasis[i] = NULL;
  } 
  create_liste_basis(GTK_WIDGET(Dialogue),frame);

  gtk_widget_realize(Dialogue);

  Bouton = create_button(Dialogue,_("Cancel"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);

  Bouton = create_button(Dialogue,_("OK"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked", (GCallback)EditBasisC, NULL);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);

  gtk_widget_show_all(Dialogue);
}
/*************************************************************************************************/
static void eventDispatcherType(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{

	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (!event) return;
	if (event->window == gtk_tree_view_get_bin_window (GTK_TREE_VIEW (widget))
	    && !gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget), event->x, event->y, NULL, NULL, NULL, NULL)) {
		gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)));
	}
	if(gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget), event->x, event->y, &path, NULL, NULL, NULL))
	{
		if(path)
		{
			model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
			gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)), path);
			sprintf(selectedRowForType ,"%s",gtk_tree_path_to_string(path));
			gtk_tree_model_get_iter (model, &iter, path);
			gtk_tree_path_free(path);
  			if (event->type == GDK_2BUTTON_PRESS && ((GdkEventButton *) event)->button == 1)
				DialogueEditT(NULL,NULL);
		}
		else sprintf(selectedRowForType ,"-1");
	}
	GTK_WIDGET_GET_CLASS(widget)->button_press_event(widget, event);
}
/*************************************************************************************************/
static void eventDispatcherCenter(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{

	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (!event) return;
	if (event->window == gtk_tree_view_get_bin_window (GTK_TREE_VIEW (widget))
	    && !gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget), event->x, event->y, NULL, NULL, NULL, NULL)) {
		gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)));
	}
	if(gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget), event->x, event->y, &path, NULL, NULL, NULL))
	{
		if(path)
		{
			model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
			gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)), path);
			sprintf(selectedRowForCenter ,"%s",gtk_tree_path_to_string(path));
			gtk_tree_model_get_iter (model, &iter, path);
			gtk_tree_path_free(path);
  			if (event->type == GDK_2BUTTON_PRESS && ((GdkEventButton *) event)->button == 1)
				DialogueEditC(NULL,NULL);
		}
		else sprintf(selectedRowForCenter ,"-1");
	}
	GTK_WIDGET_GET_CLASS(widget)->button_press_event(widget, event);
}
/*************************************************************************************************/
static void EditBasisT(GtkWidget *w,gpointer Entree)
{
       	gchar *texts[3];
       	gchar *texts3[4];
       	gchar *temp;
        gint Nc;
        gint i;
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;
	gint k;
	GtkTreeModel *modelCenter;
        GtkListStore *storeCenter;
	GtkTreeIter  iterCenter;
	gchar pathString[10];
	gint kc = 0;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(listT));
        store = GTK_LIST_STORE (model);

	modelCenter = gtk_tree_view_get_model(GTK_TREE_VIEW(listC));
        storeCenter = GTK_LIST_STORE (modelCenter);

        Nc=atoi(selectedRowForType);
        temp = get_info_basis();
  	if (!strcmp(temp,"Noselect") ) return;
        BasisT[Nc].BasisName=g_strdup(temp);
  	texts[0] = g_strdup(BasisT[Nc].CenterName);
  	texts[1] = g_strdup(BasisT[Nc].BasisName);
  	texts[2] = g_strdup(BasisT[Nc].Layer);

	if(gtk_tree_model_get_iter_from_string (model, &iter, selectedRowForType)) gtk_list_store_remove(store, &iter);
	gtk_list_store_insert(store, &iter, Nc);
	for(k=0;k<3;k++) gtk_list_store_set (store, &iter, k, texts[k], -1);
	selectRow(listT, Nc);
	sprintf(selectedRowForType,"%d",Nc);

        for(i=0;i<NSymbCenters;i++)
	if(!strcmp(BasisC[i].CenterName,BasisT[Nc].CenterName) && !strcmp(BasisC[i].Layer,BasisT[Nc].Layer))
        {
 		BasisC[i].Changed=FALSE;
        	BasisC[i].BasisName=g_strdup(temp);
  		texts3[0] = g_strdup_printf("%d",BasisC[i].Number);
  		texts3[1] = g_strdup(BasisC[i].CenterName);
  		texts3[2] = g_strdup(BasisC[i].BasisName);
  		texts3[3] = g_strdup(BasisC[i].Layer);
		sprintf(pathString,"%d",i);
		if(gtk_tree_model_get_iter_from_string (modelCenter, &iterCenter, pathString)) gtk_list_store_remove(storeCenter, &iterCenter);
		gtk_list_store_insert(storeCenter, &iterCenter, i);
		for(k=0;k<4;k++) gtk_list_store_set (storeCenter, &iterCenter, k, texts3[k], -1);
		kc = i;
        }
	BasisName=g_strdup("Noselect");
	if(NSymbCenters>=0)
	{
		selectRow(listC, kc);
		sprintf(selectedRowForCenter,"%d",kc);
	}
}

/*******************************************************************/
static void DialogueEditT(GtkWidget *w,gpointer data)
{
  GtkWidget *Dialogue;
  GtkWidget *Bouton;
  GtkWidget *frame;
  gint Nc;
  guint i;
  
  Nc=atoi(selectedRowForType);
  if(Nc<0 ) {
   	Message(_("Please Select your center \n")," Warning ",TRUE);
   return;
  }
  Dialogue = gtk_dialog_new();
  gtk_window_set_modal(GTK_WINDOW(Dialogue),TRUE);
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);
  gtk_window_set_title(GTK_WINDOW(Dialogue),_("Basis"));

  frame = create_frame(GTK_WIDGET(Dialogue),GTK_DIALOG(Dialogue)->vbox,_("Title"));
  for (i=0;i<NHboxBasis;i++)
  {
	EntryB[i]=NULL;
	HboxBasis[i] = NULL;
  } 
  create_liste_basis(GTK_WIDGET(Dialogue),frame);

  gtk_widget_realize(Dialogue);

  Bouton = create_button(Dialogue,_("Cancel"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);

  Bouton = create_button(Dialogue,_("OK"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked", (GCallback)EditBasisT, NULL);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);

  gtk_widget_show_all(Dialogue);
}
/***************************************************************************************************/
static void append_list_type()
{
	gchar *texts[3];
	gint Nc;
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;
	gint k;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(listT));
        store = GTK_LIST_STORE (model);

	for(Nc=0;Nc<NSymbTypes;Nc++)
	{
		texts[0] = g_strdup(BasisT[Nc].CenterName);
		texts[1] = g_strdup(BasisT[Nc].BasisName);
		texts[2] = g_strdup(BasisT[Nc].Layer);
		gtk_list_store_append(store, &iter);
		for(k=0;k<3;k++) gtk_list_store_set (store, &iter, k, texts[k], -1);
	}
	if(NSymbTypes>0)
	{
		selectRow(listT, 0);
		sprintf(selectedRowForType,"0");
	}
}
/***************************************************************************************************/
void create_basis_type_list(GtkWidget *vbox)
{
  GtkWidget *scr;
  GtkWidget *button;
  GtkWidget *hbox;
  guint i;
  guint Factor=7;
  guint widall=0;
  gchar *titres[3]={	N_(" Symbol "),N_(" Basis "), N_(" Layer ")};
  gint width[3]={6,18,8 };

  GtkListStore *store;
  GtkTreeModel *model;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkTreeSelection *select;
  
  sprintf(selectedRowForType,"-1");
  for(i=0;i<2;i++)
      widall+=width[i];
  widall=widall*Factor;

  scr=gtk_scrolled_window_new(NULL,NULL);
  gtk_box_pack_start(GTK_BOX (vbox), scr,FALSE, FALSE, 2);
  gtk_widget_set_size_request(scr,widall,(gint)(ScreenHeight*0.3));

  store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
  model = GTK_TREE_MODEL (store);

  listT = gtk_tree_view_new_with_model (model);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (listT), TRUE);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (listT), TRUE);
  gtk_tree_view_set_reorderable(GTK_TREE_VIEW (listT), TRUE);

  for (i=0;i<3;i++)
  {
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, titres[i]);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_min_width(column, width[i]*Factor);
	gtk_tree_view_column_set_attributes (column, renderer, "text", i, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (listT), column);
  }
  
  gtk_container_add(GTK_CONTAINER(scr),listT);
  select = gtk_tree_view_get_selection (GTK_TREE_VIEW (listT));
  gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
  g_signal_connect(listT, "button_press_event", G_CALLBACK(eventDispatcherType), NULL);

  gtk_widget_show (listT);
  hbox = create_hbox_false(vbox);

  button = gtk_button_new_with_label (_("Edit Basis"));
  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)DialogueEditT,NULL);
  gtk_box_pack_start (GTK_BOX(hbox), button, TRUE, TRUE, 0);

  gtk_widget_show (button);
}
/******************************************************************************/
static void append_list_center()
{
	gchar *texts[4];
	gint Nc;
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;
	gint k;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(listC));
        store = GTK_LIST_STORE (model);
  
	for(Nc=0;Nc<NSymbCenters;Nc++)
	{
		texts[0] = g_strdup_printf("%d",BasisC[Nc].Number);
		texts[1] = g_strdup(BasisC[Nc].CenterName);
		texts[2] = g_strdup(BasisC[Nc].BasisName);
		texts[3] = g_strdup(BasisC[Nc].Layer);
		gtk_list_store_append(store, &iter);
		for(k=0;k<4;k++) gtk_list_store_set (store, &iter, k, texts[k], -1);
	}
	if(NSymbCenters>0)
	{
		selectRow(listC, 0);
		sprintf(selectedRowForCenter,"0");
	}
}
/******************************************************************************/
void create_basis_center_list(GtkWidget *vbox)
{
  GtkWidget *scr;
  GtkWidget *button;
  GtkWidget *hbox;
  guint i;
  guint Factor=7;
  guint widall=0;
  gchar *titres[4]={	N_(" Center Number "), N_(" Symbol "),N_(" Basis "), N_(" Layer ")};
  gint width[4]={12,6,18,8 };
  GtkListStore *store;
  GtkTreeModel *model;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkTreeSelection *select;
  
  for(i=0;i<2;i++) widall+=width[i];
  widall=widall*Factor;

  scr=gtk_scrolled_window_new(NULL,NULL);
  gtk_box_pack_start(GTK_BOX (vbox), scr,FALSE, FALSE, 2);
  gtk_widget_set_size_request(scr,widall,(gint)(ScreenHeight*0.3));

  store = gtk_list_store_new (4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
  model = GTK_TREE_MODEL (store);

  listC = gtk_tree_view_new_with_model (model);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (listC), TRUE);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (listC), TRUE);
  gtk_tree_view_set_reorderable(GTK_TREE_VIEW (listC), TRUE);

  for (i=0;i<4;i++)
  {
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, titres[i]);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_min_width(column, width[i]*Factor);
	gtk_tree_view_column_set_attributes (column, renderer, "text", i, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (listC), column);
  }
  
  gtk_container_add(GTK_CONTAINER(scr),listC);
  select = gtk_tree_view_get_selection (GTK_TREE_VIEW (listC));
  gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
  g_signal_connect(listC, "button_press_event", G_CALLBACK(eventDispatcherCenter), NULL);

  hbox = create_hbox_false(vbox);

  button = gtk_button_new_with_label (_("Edit Basis"));
  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)DialogueEditC,NULL);
  gtk_box_pack_start (GTK_BOX(hbox), button, TRUE, TRUE, 0);

  gtk_widget_show (button);


}
/*************************************************************************************************/
void create_basis_interface (GtkWidget *window,GtkWidget *hbox)
{
  GtkWidget *vbox;
  GtkWidget *label;
  GtkWidget *vseparator;
  GtkWidget *Frame;

  Frame = create_frame(window,hbox,_("Basis"));

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (Frame), hbox);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 5);

  label = gtk_label_new (_(" Define basis by type of center "));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 2);

  create_basis_type_list(vbox);
  append_list_type();

  vseparator = create_vseparator (hbox);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 5);

  label = gtk_label_new (_(" Define basis center by center (not recommended) "));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 2);



  create_basis_center_list(vbox);
  append_list_center();
  gtk_widget_show_all (Frame);
}
/*************************************************************************************************/
void GAjoutePageBasis(GtkWidget *NoteBook)
{
  GtkWidget *window1;
  GtkWidget *Frame;
  GtkWidget *LabelOnglet;
  GtkWidget *LabelMenu;
  GtkWidget *vbox;
  GtkWidget *hbox;
  gint i;
  

  for (i=0;i<NHboxBasis;i++)
  {
   EntryB[i]=NULL;
   HboxBasis[i] = NULL;
  } 
  BasisName=g_strdup("Noselect");
  create_tabs_list();
  Frame = gtk_frame_new(NULL);
  gtk_container_set_border_width(GTK_CONTAINER(Frame), 10);

  LabelOnglet = gtk_label_new(_("Basis Generated"));
  LabelMenu = gtk_label_new(_("Basis"));
  gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),
                                Frame,
                                LabelOnglet, LabelMenu);

  window1 = Frame;
  g_object_set_data(G_OBJECT (window1), "window1", window1);
  vbox =create_vbox(window1);
  hbox =create_hbox_false(vbox);
  create_basis_interface(window1,hbox);
}

