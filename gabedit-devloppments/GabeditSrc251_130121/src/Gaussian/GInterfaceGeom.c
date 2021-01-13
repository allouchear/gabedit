/* GInterfaceGeom.c */
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
#include "../Utils/Utils.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/GeomZmatrix.h"
#include "../Geometry/InterfaceGeom.h"
#include "GaussGlobal.h"

/************************************************************************************************************/
static gint get_number_of_model_connections()
{
	gint i;
	gint j;
	gint nc = 0;
	gint NC = NcentersXYZ;
	if(MethodeGeom == GEOM_IS_ZMAT) NC = NcentersZmat;
    	for(i=0;i<NC;i++)
	{
		if( MethodeGeom == GEOM_IS_XYZ) 
			if(strstr(GeomXYZ[i].Layer,"Me") || strstr(GeomXYZ[i].Layer,"Lo")) continue;
		if( MethodeGeom == GEOM_IS_ZMAT) 
			if(strstr(Geom[i].Layer,"Me") || strstr(Geom[i].Layer,"Lo")) continue;
    		for(j=0;j<NC;j++)
		{
			if(i==j) continue;
			if( MethodeGeom == GEOM_IS_XYZ) 
				if(!strstr(GeomXYZ[j].Layer,"Me") && !strstr(GeomXYZ[j].Layer,"Lo")) continue;
			if( MethodeGeom == GEOM_IS_ZMAT) 
				if(!strstr(Geom[j].Layer,"Me") && !strstr(Geom[j].Layer,"Lo")) continue;
			if(connecteds(i,j)) nc++;
		}
	}
	return nc;
}
/************************************************************************************************************/
static gint get_number_of_inter_connections()
{
	gint i;
	gint j;
	gint nc = 0;
	gint NC = NcentersXYZ;
	if(MethodeGeom == GEOM_IS_ZMAT) NC = NcentersZmat;
    	for(i=0;i<NC;i++)
	{

		if( MethodeGeom == GEOM_IS_XYZ) 
			if(strstr(GeomXYZ[i].Layer," ") || strstr(GeomXYZ[i].Layer,"Lo") || strstr(GeomXYZ[i].Layer,"Hi")) continue;
		if( MethodeGeom == GEOM_IS_ZMAT) 
			if(strstr(Geom[i].Layer," ") || strstr(Geom[i].Layer,"Lo") || strstr(Geom[i].Layer,"Hi")) continue;

    		for(j=0;j<NC;j++)
		{
			if(i==j) continue;
			if( MethodeGeom == GEOM_IS_XYZ) 
				if(!strstr(GeomXYZ[j].Layer," ") && !strstr(GeomXYZ[j].Layer,"Lo") && !strstr(GeomXYZ[j].Layer,"Hi")) continue;
			if( MethodeGeom == GEOM_IS_ZMAT) 
				if(!strstr(Geom[j].Layer," ") && !strstr(Geom[j].Layer,"Lo") && !strstr(Geom[j].Layer,"Hi")) continue;
			if(connecteds(i,j)) nc++;
		}
	}
	return nc;
}
/************************************************************************************************************/
void set_spin_of_electrons()
{
        gint i;
        guint NumberElectrons[3];
        guint SpinElectrons[3];
        gchar* chaine;

        if(EntryCS[0] == NULL ) return;
        NumberElectrons[2]= get_number_electrons(2);
        NumberElectrons[1]= get_number_electrons(1);
        NumberElectrons[0]= get_number_electrons(0);

        for(i=0;i<3;i++)
		SpinElectrons[i]=0;
        if(NMethodes==3)
	{
        	NumberElectrons[2] += get_number_of_model_connections();
        	NumberElectrons[1] += get_number_of_inter_connections();
	}
        if(NMethodes==2)
	{
        	NumberElectrons[1] += get_number_of_model_connections();
	}

        for(i=0;(guint)i<NMethodes;i++)
        	if((NumberElectrons[i]-TotalCharges[i])%2==0)
			SpinElectrons[i]=1;
                else
			SpinElectrons[i]=2;

        for(i=0;(guint)i<NMethodes;i++)
        {
		if(SpinMultiplicities[i]%2 != SpinElectrons[i]%2)
			SpinMultiplicities[i] = SpinElectrons[i];
         	chaine = g_strdup_printf("%d",SpinMultiplicities[i]);
	 	if(EntryCS[2*i+1] && GTK_IS_ENTRY(EntryCS[2*i+1]))
		{
         		gtk_entry_set_text(GTK_ENTRY(EntryCS[2*i+1]),chaine);
		}
        }
}
/************************************************************************************************************/
static void change_of_charge(GtkWidget *entry,gpointer d)
{
        G_CONST_RETURN gchar *entry_text;
        gint *Number;

        Number = (gint*)d;
        entry_text = gtk_entry_get_text(GTK_ENTRY(entry));
        TotalCharges[*Number] = atoi(entry_text);
	set_spin_of_electrons();

}
/************************************************************************************************************/
static void create_combo_charge(GtkWidget *hbox,gint Num,gchar *tlabel)
{
  gchar *tlist[]={"0","1","-1","2","-2","3","-3","4","-4"};
  gint *Number;
  Number = g_malloc(sizeof(gint));
  *Number = Num/2;
  EntryCS[Num] = create_label_combo(hbox,tlabel,tlist,9,TRUE,-1,(gint)(ScreenHeight*0.1));
  g_signal_connect(G_OBJECT(EntryCS[Num]), "changed", G_CALLBACK(change_of_charge), Number);
  TotalCharges[*Number] = 0;
}
/************************************************************************************************************/
static void create_combo_spin(GtkWidget *hbox,gint Num,gchar *tlabel)
{
  gchar *tlist[]={"1","2","3","4","5","6","7","8","9"};
 EntryCS[Num] = create_label_combo(hbox,tlabel,tlist,9,TRUE,-1,(gint)(ScreenHeight*0.1));
}
/************************************************************************************************************/
void GAjoutePageGeom(GtkWidget *NoteBook)
{
  GtkWidget *window1;
  GtkWidget *Frame;
  GtkWidget *LabelOnglet;
  GtkWidget *LabelMenu;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *hbox2;
  GtkWidget *FrameType;
  GtkWidget *combobox = NULL;
  gint i;
  
  geom_is_open();
  geominter=g_malloc(sizeof(GeomInter));
  geominter->window= gtk_frame_new(NULL);
  HboxGeom = NULL; 
  for(i=0;i<6;i++)
	EntryCS[i]=NULL;

  Frame = geominter->window;
  gtk_container_set_border_width(GTK_CONTAINER(Frame), 10);

  LabelOnglet = gtk_label_new(_("Molecular Specifications"));
  LabelMenu = gtk_label_new(_("Molecular Specifications"));
  gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),
                                Frame,
                                LabelOnglet, LabelMenu);

  window1 = Frame;
  g_object_set_data(G_OBJECT (window1), "window1", window1);

  vbox =create_vbox(window1);

  hbox =create_hbox_false(vbox);
  hbox2 =create_hbox_false(vbox);
  NMethodes = 0;

  if(Methodes[1]!=NULL || Methodes[2]!=NULL )
  {
  	create_label_hbox(hbox,_("Charge of    "),-1);
  	create_combo_charge(hbox,0,_("Real system : "));
  	create_label_hbox(hbox2,_("2*Spin+1 of "),-1);
  	create_combo_spin(hbox2,1,_("Real system : "));
        NMethodes++;
  }
  else
  {
  	create_label_hbox(hbox,_("Charge of   "),-1);
  	create_combo_charge(hbox,0,_("system : "));
  	create_label_hbox(hbox,_("2*Spin+1 of "),-1);
  	create_combo_spin(hbox,1,_("system : "));
        NMethodes++;
  }
  if(Methodes[1]!=NULL && Methodes[2]!=NULL )
  {
  	create_combo_charge(hbox,2,_("Intermediate system : "));
  	create_combo_spin(hbox2,3,_("Intermediate system : "));
  	NMethodes++;
  	create_combo_charge(hbox,4,_("Model system : "));
  	create_combo_spin(hbox2,5,_("Model system : "));
  	NMethodes++;
  }
  else
  if(Methodes[1]!=NULL || Methodes[2]!=NULL )
  {
  	create_combo_charge(hbox,2,_("Model system : "));
  	create_combo_spin(hbox2,3,_("Model system : "));
  	NMethodes++;
  }

  if(!GeomFromCheck )
  {
  create_hseparator(vbox);
  hbox =create_hbox_false(vbox);
  FrameType = create_frame(window1,hbox,_("TYPE"));
  combobox = create_geom(window1,FrameType);

  geominter->vbox=vbox;
  geominter->frametitle=g_strdup(_("GEOMETRY"));

 if(GeomXYZ != NULL && MethodeGeom == GEOM_IS_XYZ )
     create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
 if(Geom != NULL && MethodeGeom == GEOM_IS_ZMAT)
     create_geom_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

  }
  set_spin_of_electrons();
  if(combobox) gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 0);

}

