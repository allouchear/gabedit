/* OrcaMolecule.c */
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

#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "../../Config.h"
#include "../Common/Global.h"
#include "../Orca/OrcaTypes.h"
#include "../Orca/OrcaGlobal.h"
#include "../Orca/OrcaKeywords.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/GeomConversion.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Common/Windows.h"
#include "../Utils/Constants.h"
#include "../Utils/AtomsProp.h"

/************************************************************************************************************/
static gint totalCharge = 0;
static gint spinMultiplicity=1;
/************************************************************************************************************/
gint getOrcaMultiplicity()
{
	return spinMultiplicity;
}
/************************************************************************************************************/
void initOrcaMoleculeButtons()
{
}
/************************************************************************************************************/
void initOrcaMolecule()
{
	orcaMolecule.listOfAtoms = NULL;  
	orcaMolecule.totalNumberOfElectrons = 0;
	orcaMolecule.numberOfValenceElectrons = 0;
	orcaMolecule.numberOfAtoms = 0;
}
/************************************************************************************************************/
void freeOrcaMolecule()
{
	static gboolean first = TRUE;

	if(first)
	{
		initOrcaMolecule();
		first = FALSE;
		return;
	}

	if(orcaMolecule.listOfAtoms) g_free(orcaMolecule.listOfAtoms);
	initOrcaMolecule();
}
/************************************************************************************************************/
static gint setOrcaMoleculeFromSXYZ(gint nAtoms, gchar** symbols, gdouble* X, gdouble* Y, gdouble* Z)
{
	gint n;
	OrcaAtom* atomList = NULL;

	orcaMolecule.listOfAtoms = NULL;  
	orcaMolecule.numberOfAtoms = 0;
	if(nAtoms<1) return 1;

	orcaMolecule.listOfAtoms = (OrcaAtom*)g_malloc(sizeof(OrcaAtom)*(nAtoms));
	if(orcaMolecule.listOfAtoms==NULL) return -1;

	orcaMolecule.numberOfAtoms = nAtoms;

	atomList = orcaMolecule.listOfAtoms;
	for(n=0; n<orcaMolecule.numberOfAtoms; n++)
	{
		atomList->position[0]  = X[n];
		atomList->position[1]  = Y[n];
		atomList->position[2]  = Z[n];
		atomList->symbol  = g_strdup(symbols[n]);
		atomList++;
	}

	return 0;
}
/************************************************************************************************************/
static void setXYZFromGeomXYZ(gint i, gdouble* x, gdouble* y, gdouble *z)
{
  	if(!test(GeomXYZ[i].X))
    		*x = get_value_variableXYZ(GeomXYZ[i].X);
  	else
    		*x = atof(GeomXYZ[i].X);
  	if(!test(GeomXYZ[i].Y))
    		*y = get_value_variableXYZ(GeomXYZ[i].Y);
  	else
    		*y = atof(GeomXYZ[i].Y);
  	if(!test(GeomXYZ[i].Z))
    		*z = get_value_variableXYZ(GeomXYZ[i].Z);
  	else
    		*z = atof(GeomXYZ[i].Z);

         if(Units==0)
         {
              *x *= BOHR_TO_ANG;
              *y *= BOHR_TO_ANG;
              *z *= BOHR_TO_ANG;
         }
}
/************************************************************************************************************/
static gboolean setOrcaMoleculeFromGeomXYZ()
{
	gint i;
	gchar** symbols = NULL;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gdouble* Z = NULL;
	gint numberOfAtoms = NcentersXYZ;

	if(numberOfAtoms<1) return FALSE;

	symbols = (gchar**)g_malloc(sizeof(gchar*)*(numberOfAtoms));

	if(symbols == NULL) return FALSE;

	X = (gdouble*)g_malloc(sizeof(gdouble)*(numberOfAtoms));
	if(X == NULL) return FALSE;
	Y = (gdouble*)g_malloc(sizeof(gdouble)*(numberOfAtoms));
	if(Y == NULL) return FALSE;
	Z = (gdouble*)g_malloc(sizeof(gdouble)*(numberOfAtoms));
	if(Z == NULL) return FALSE;

	orcaMolecule.totalNumberOfElectrons = 0;
	for(i=0; i<numberOfAtoms; i++)
	{
		SAtomsProp prop = prop_atom_get(GeomXYZ[i].Symb);

		symbols[i] = g_strdup(GeomXYZ[i].Symb);
		setXYZFromGeomXYZ(i, &X[i] , &Y[i] , &Z[i]);
		orcaMolecule.totalNumberOfElectrons += prop.atomicNumber;
	}
	orcaMolecule.numberOfValenceElectrons = orcaMolecule.totalNumberOfElectrons;
	setOrcaMoleculeFromSXYZ(numberOfAtoms, symbols, X, Y, Z);

	for (i=0;i<(gint)NcentersXYZ;i++) g_free( symbols[i]);
	g_free( symbols);
	g_free(X);
	g_free(Y);
	g_free(Z);
	return TRUE;
}
/************************************************************************************************************/
static gboolean setOrcaMoleculeFromGeomZMatrix()
{
	iprogram=PROG_IS_ORCA;
	if(!zmat_to_xyz()) return FALSE;
	delete_dummy_atoms();
	/* conversion_zmat_to_xyz();*/
	return setOrcaMoleculeFromGeomXYZ();
}
/************************************************************************************************************/
gboolean setOrcaMolecule()
{
	freeOrcaMolecule();
	if(MethodeGeom==GEOM_IS_XYZ && setOrcaMoleculeFromGeomXYZ()) return TRUE;
	if(setOrcaMoleculeFromGeomZMatrix()) return TRUE;
	return FALSE;
}
/************************************************************************************************************/
void setOrcaGeometryFromInputFile(gchar* fileName)
{
	read_XYZ_from_orca_input_file(fileName);
	setOrcaMolecule();
}
/*************************************************************************************************************/
/*
static gdouble getMinDistance()
{
	gdouble d=0;
	gint i;
	gint k;
	OrcaAtom* atomList = orcaMolecule.listOfAtoms;
	for(i=0; i<orcaMolecule.numberOfAtoms-1; i++)
	{
		gdouble dd = 0;
		for(k=0;k<3;k++) 
		{
			gdouble xx = atomList->position[k]-atomList->position[k+1];
			dd += xx*xx;
		}
		if(i==0) d = dd;
		else if(d>dd) d= dd;
		atomList++;
	}
	d = sqrt(d);

	return d;
}
*/
/*************************************************************************************************************/
/*
static void setFirstAtomToXAxis(gint numberOfAtoms, gdouble* X, gdouble* Y, gdouble*Z)
{
	gdouble d;
	gdouble s;
	gdouble c;
	gint i;
	gdouble positionTolerance = -1;

	if(numberOfAtoms<1) return;
	d = X[0]*X[0]+Y[0]*Y[0];
	if(d<1e-10) return;
	d = sqrt(d);
	if(positionTolerance<0) positionTolerance= getMinDistance()/50;

	s = -Y[0]/d;
	c = +X[0]/d;

	for (i=0;i<numberOfAtoms;i++)
	 {
		 gdouble x = X[i];
		 gdouble y = Y[i];
		X[i] = c*x - s*y;
		Y[i] = s*x + c*y;
		if(fabs(Y[i])<positionTolerance) Y[i]=0.0;
	 }

}
*/
/*************************************************************************************************************/
/*
static gint getRealNumberXYZVariables()
{
	gint k=0;
	gint i;
        for(i=0;i<NcentersXYZ;i++)
	{
		if(test(GeomXYZ[i].X))k++;
		if(test(GeomXYZ[i].Y))k++;
		if(test(GeomXYZ[i].Z))k++;
	}
	return k;
}
*/
/*************************************************************************************************************/
static void putOrcaMoleculeInTextEditor()
{
        gchar buffer[BSIZE];
	gint i;
	gint nV = 0;
	gdouble x,y,z;

	if(orcaMolecule.numberOfAtoms<1) return;

	if(MethodeGeom==GEOM_IS_XYZ)
	{
		sprintf(buffer,"* xyz %d   %d\n",totalCharge,spinMultiplicity);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &orcaColorFore.keyWord, &orcaColorBack.keyWord, buffer, -1);
      		for (i=0;i<orcaMolecule.numberOfAtoms;i++)
		{
			gchar X[100];
			gchar Y[100];
			gchar Z[100];
			setXYZFromGeomXYZ(i, &x, &y, &z);
			sprintf(X,"%f",x);
			sprintf(Y,"%f",y);
			sprintf(Z,"%f",z);
			if(!strcmp(orcaMolecule.listOfAtoms[i].symbol,"X")) sprintf(buffer," %s  %s %s %s\n","DA", X,Y,Z);
			else sprintf(buffer," %s  %s %s %s\n",orcaMolecule.listOfAtoms[i].symbol, X,Y,Z);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
		}
		sprintf(buffer,"*\n");
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &orcaColorFore.keyWord, &orcaColorBack.keyWord, buffer, -1);
		nV = 0;
        	if(NVariablesXYZ>0)
        	for(i=0;i<NVariablesXYZ;i++)
        	{
        		if(VariablesXYZ[i].Used) nV++;
        	}
		/* printf("nV=%d\n",nV);*/
		if(nV>0 && nV!= 3*orcaMolecule.numberOfAtoms) 
		{
			sprintf(buffer,"%cgeom Constraints\n",'%');
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, &orcaColorFore.keyWord, &orcaColorBack.keyWord, buffer, -1);
      			for (i=0;i<orcaMolecule.numberOfAtoms;i++)
			{
  				if( !(!test(GeomXYZ[i].X) || !test(GeomXYZ[i].Y) || !test(GeomXYZ[i].Z)) )
				{
					sprintf(buffer,"  {C %d C}\n",i);
        				gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,buffer,-1);
				}
			}
			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," end #Constraints\n",-1);
			/* gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," invertConstraints true\n",-1);*/
			gabedit_text_insert (GABEDIT_TEXT(text), NULL, &orcaColorFore.keyWord, &orcaColorBack.keyWord," end #geom\n",-1);
		}
	}
	else
	{
		sprintf(buffer,"* int %d   %d\n",totalCharge,spinMultiplicity);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &orcaColorFore.keyWord, &orcaColorBack.keyWord, buffer, -1);
        	for(i=0;i<NcentersZmat;i++)
        	{
			SAtomsProp prop = prop_atom_get(Geom[i].Symb);
			gchar symb[100];
			if(!strcmp(Geom[i].Symb,"X")) sprintf(symb,"%s","DA");
			else sprintf(symb,"%s",Geom[i].Symb);
        		if(Geom[i].Nentry>NUMBER_ENTRY_ANGLE)
			{
				gchar R[100];
				gchar A[100];
				gchar D[100];
				sprintf(R,"%s",Geom[i].R);
  				if(!test(Geom[i].R)) sprintf(R,"%f",get_value_variableZmat(Geom[i].R));
				sprintf(A,"%s",Geom[i].Angle);
  				if(!test(Geom[i].Angle)) sprintf(A,"%f",get_value_variableZmat(Geom[i].Angle));
				sprintf(D,"%s",Geom[i].Dihedral);
  				if(!test(Geom[i].Dihedral)) sprintf(D,"%f",get_value_variableZmat(Geom[i].Dihedral));

				sprintf(buffer," %s  %s %s %s %s %s %s\n",
						symb,
						Geom[i].NR, Geom[i].NAngle, Geom[i].NDihedral, 
						R, A, D);
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, &prop.color, buffer, -1);
			}
			else
        		if(Geom[i].Nentry>NUMBER_ENTRY_R)
			{
				gchar R[100];
				gchar A[100];
				sprintf(R,"%s",Geom[i].R);
  				if(!test(Geom[i].R)) sprintf(R,"%f",get_value_variableZmat(Geom[i].R));
				sprintf(A,"%s",Geom[i].Angle);
  				if(!test(Geom[i].Angle)) sprintf(A,"%f",get_value_variableZmat(Geom[i].Angle));
				sprintf(buffer," %s  %s %s %s %s %s %s\n",
						symb,
						Geom[i].NR, Geom[i].NAngle, "0", 
						R, A, "0.0");
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, &prop.color, buffer, -1);
			}
			else
        		if(Geom[i].Nentry>NUMBER_ENTRY_0)
			{
				gchar R[100];
				sprintf(R,"%s",Geom[i].R);
  				if(!test(Geom[i].R)) sprintf(R,"%f",get_value_variableZmat(Geom[i].R));
				sprintf(buffer," %s  %s %s\n",
						symb,
						Geom[i].NR,R
						);
				sprintf(buffer," %s  %s %s %s %s %s %s\n",
						symb,
						Geom[i].NR, "0", "0", 
						R, "0.0", "0.0");
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, &prop.color, buffer, -1);
			}
			else
			{
				sprintf(buffer," %s  %s %s %s %s %s %s\n",
						symb,
						"0", "0", "0",
						"0.0", "0.0", "0.0");
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, &prop.color, buffer, -1);
			}
        	}
		sprintf(buffer,"*\n");
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &orcaColorFore.keyWord, &orcaColorBack.keyWord, buffer, -1);
		nV = 0;
        	if(NVariables>0)
        	for(i=0;i<NVariables;i++)
        	{
        		if(Variables[i].Used)
			{
				nV++;
				break;
			}
        	}
		if(nV>0) 
		{
			sprintf(buffer,"%cgeom Constraints\n",'%');
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,buffer,-1);
      			for (i=0;i<NcentersZmat;i++)
			{
  				if(Geom[i].Nentry>=NUMBER_ENTRY_R && !test(Geom[i].R)) 
				{
					sprintf(buffer,"  {B %d %d C}\n",atoi(Geom[i].NR)-1,i);
        				gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,buffer,-1);
				}
  				if(Geom[i].Nentry>=NUMBER_ENTRY_ANGLE && !test(Geom[i].Angle)) 
				{
					sprintf(buffer,"  {A %d %d %d C}\n",
							atoi(Geom[i].NAngle)-1,
							atoi(Geom[i].NR)-1,i);
        				gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,buffer,-1);
				}
  				if(Geom[i].Nentry>NUMBER_ENTRY_ANGLE && !test(Geom[i].Dihedral)) 
				{
					sprintf(buffer,"  {D %d %d %d %d C}\n",
							atoi(Geom[i].NDihedral)-1, 
							atoi(Geom[i].NAngle)-1,
							atoi(Geom[i].NR)-1,i);
        				gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,buffer,-1);
				}
			}
			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," end #Constraints\n",-1);
			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," invertConstraints true\n",-1);
			gabedit_text_insert (GABEDIT_TEXT(text), NULL, &orcaColorFore.keyWord, &orcaColorBack.keyWord," end #geom\n",-1);
		}
	}

}
/************************************************************************************************************/
void putOrcaMoleculeInfoInTextEditor()
{
	putOrcaMoleculeInTextEditor();
}
/************************************************************************************************************/
/*
static GtkWidget* addRadioButtonToATable(GtkWidget* table, GtkWidget* friendButton, gchar* label, gint i, gint j, gint k)
{
	GtkWidget *newButton;

	if(friendButton)
		newButton = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (friendButton)), label);
	else
		newButton = gtk_radio_button_new_with_label( NULL, label);

	gtk_table_attach(GTK_TABLE(table),newButton,j,j+k,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  2,2);

	g_object_set_data(G_OBJECT (newButton), "Label",NULL);
	g_object_set_data(G_OBJECT (newButton), "Type",NULL);
	return newButton;
}
*/
/********************************************************************************/
static void setSpinMultiplicityComboSpinMultiplicity(GtkWidget *comboSpinMultiplicity, gint spin)
{
	GtkWidget *entry = NULL;
	gchar* t = NULL;
	if(!comboSpinMultiplicity) return;
	entry = GTK_BIN (comboSpinMultiplicity)->child;
	t = g_strdup_printf("%d",spin);
	gtk_entry_set_text(GTK_ENTRY(entry),t);
	g_free(t);
}
/************************************************************************************************************/
static void setComboSpinMultiplicity(GtkWidget *comboSpinMultiplicity)
{
	GList *glist = NULL;
	gint i;
	gint nlist = 0;
	gchar** list = NULL;
	gint k;
	gint kinc;
	gint ne = orcaMolecule.numberOfValenceElectrons - totalCharge;

	if(ne%2==0) nlist = ne/2+1;
	else nlist = (ne+1)/2;

	if(nlist<1) return;
	list = g_malloc(nlist*sizeof(gchar*));
	if(!list) return;
	for(i=0;i<nlist;i++)
		list[i] = g_malloc(10*sizeof(gchar));


	if(GTK_IS_WIDGET(comboSpinMultiplicity)) gtk_widget_set_sensitive(comboSpinMultiplicity, TRUE);
	if(ne%2==0) k = 1;
	else k = 2;

	kinc = 2;
	for(i=0;i<nlist;i++)
	{
		sprintf(list[i],"%d",k);
		k+=kinc;
	}

  	for(i=0;i<nlist;i++) glist = g_list_append(glist,list[i]);

  	gtk_combo_box_entry_set_popdown_strings( comboSpinMultiplicity, glist) ;
  	g_list_free(glist);
	if( SpinMultiplicities[0]%2 == atoi(list[0])%2) setSpinMultiplicityComboSpinMultiplicity(comboSpinMultiplicity, SpinMultiplicities[0]);
	else SpinMultiplicities[0] = atoi(list[0]);
	if(list)
	{
		for(i=0;i<nlist;i++) if(list[i]) g_free(list[i]);
		g_free(list);
	}
}
/********************************************************************************/
static void setChargeComboCharge(GtkWidget *comboCharge, gint charge)
{
	GtkWidget *entry = NULL;
	gchar* t = NULL;
	if(!comboCharge) return;
	entry = GTK_BIN (comboCharge)->child;
	t = g_strdup_printf("%d",charge);
	gtk_entry_set_text(GTK_ENTRY(entry),t);
	g_free(t);
}
/********************************************************************************/
static void setComboCharge(GtkWidget *comboCharge)
{
	GList *glist = NULL;
	gint i;
	gint nlist;
	gchar** list = NULL;
	gint k;

	nlist = orcaMolecule.numberOfValenceElectrons*2-2+1;

	if(nlist<1) return;
	list = g_malloc(nlist*sizeof(gchar*));
	if(!list) return;
	for(i=0;i<nlist;i++)
		list[i] = g_malloc(10*sizeof(gchar));


	sprintf(list[0],"0");
	k = 1;
	for(i=1;i<nlist-1;i+=2)
	{
		sprintf(list[i],"+%d",k);
		sprintf(list[i+1],"%d",-k);
		k += 1;
	}

  	for(i=0;i<nlist;i++) glist = g_list_append(glist,list[i]);

  	gtk_combo_box_entry_set_popdown_strings( comboCharge, glist) ;
  	g_list_free(glist);
	if(list)
	{
		for(i=0;i<nlist;i++) if(list[i]) g_free(list[i]);
		g_free(list);
	}
	setChargeComboCharge(comboCharge, totalCharge);
}
/**********************************************************************/
static void changedEntrySpinMultiplicity(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	spinMultiplicity=atoi(entryText);
	if(spinMultiplicity==1)
	{
		/* OK RHF*/
		setOrcaSCFMethod(TRUE);
	}
	else 
	{
		/* remove RHF from list*/
		setOrcaSCFMethod(FALSE);
	}
}
/**********************************************************************/
static void changedEntryCharge(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	GtkWidget* comboSpinMultiplicity = NULL;
	GtkWidget* labelNumberOfElectrons = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	totalCharge = atoi(entryText);
	TotalCharges[0] = totalCharge;

	comboSpinMultiplicity  = g_object_get_data(G_OBJECT (entry), "ComboSpinMultiplicity");
	if(GTK_IS_WIDGET(comboSpinMultiplicity)) setComboSpinMultiplicity(comboSpinMultiplicity);

	labelNumberOfElectrons = g_object_get_data(G_OBJECT (entry), "LabelNumberOfElectrons");

	if(GTK_IS_WIDGET(labelNumberOfElectrons))
	{
		gint ne = orcaMolecule.numberOfValenceElectrons - totalCharge;
		gchar buffer[BSIZE];
		sprintf(buffer, "Number of electrons = %d",ne);
		gtk_label_set_text(GTK_LABEL(labelNumberOfElectrons),buffer);
	}
}
/**********************************************************************/
static GtkWidget* addComboListToATable(GtkWidget* table,
		gchar** list, gint nlist, gint i, gint j, gint k)
{
	GtkWidget *entry = NULL;
	GtkWidget *combo = NULL;

	combo = create_combo_box_entry(list, nlist, TRUE, -1, -1);

	gtk_table_attach(GTK_TABLE(table),combo,j,j+k,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	entry = GTK_BIN (combo)->child;
	g_object_set_data(G_OBJECT (entry), "Combo",combo);
	gtk_widget_set_size_request(GTK_WIDGET(entry),(gint)(ScreenHeight*0.2),-1);

	return entry;
}
/***********************************************************************************************/
static GtkWidget *addOrcaChargeToTable(GtkWidget *table, gint i)
{
	GtkWidget* entryCharge = NULL;
	GtkWidget* comboCharge = NULL;
	gint nlistCharge = 1;
	gchar* listCharge[] = {"0"};

	add_label_table(table,_("Charge"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entryCharge = addComboListToATable(table, listCharge, nlistCharge, i, 2, 1);
	comboCharge  = g_object_get_data(G_OBJECT (entryCharge), "Combo");
	gtk_widget_set_sensitive(entryCharge, TRUE);
	gtk_editable_set_editable(GTK_EDITABLE(entryCharge), FALSE);

	return comboCharge;
}
/***********************************************************************************************/
static GtkWidget *addOrcaSpinToTable(GtkWidget *table, gint i)
{
	GtkWidget* entrySpinMultiplicity = NULL;
	GtkWidget* comboSpinMultiplicity = NULL;
	gint nlistspinMultiplicity = 1;
	gchar* listspinMultiplicity[] = {"0"};

	add_label_table(table,_("Spin multiplicity"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entrySpinMultiplicity = addComboListToATable(table, listspinMultiplicity, nlistspinMultiplicity, i, 2, 1);
	comboSpinMultiplicity  = g_object_get_data(G_OBJECT (entrySpinMultiplicity), "Combo");
	gtk_widget_set_sensitive(entrySpinMultiplicity, TRUE);
	gtk_editable_set_editable(GTK_EDITABLE(entrySpinMultiplicity), FALSE);

	g_signal_connect(G_OBJECT(entrySpinMultiplicity),"changed", G_CALLBACK(changedEntrySpinMultiplicity),NULL);
	return comboSpinMultiplicity;
}
/***********************************************************************************************/
static GtkWidget *addLabelNumberOfElectronsToTable(GtkWidget *table, gint i, GtkWidget *comboCharge)
{
	GtkWidget* labelNumberOfElectrons = NULL;
	GtkWidget* hbox = NULL;
	GtkWidget* entryCharge = GTK_BIN(comboCharge)->child;

	labelNumberOfElectrons = gtk_label_new(" ");
	hbox = gtk_hbox_new(0,FALSE);
	gtk_box_pack_start (GTK_BOX (hbox), labelNumberOfElectrons, FALSE, FALSE, 0);
	gtk_table_attach(GTK_TABLE(table),hbox,0,0+3,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);

	g_object_set_data(G_OBJECT (entryCharge), "LabelNumberOfElectrons", labelNumberOfElectrons);
	g_signal_connect(G_OBJECT(entryCharge),"changed", G_CALLBACK(changedEntryCharge),NULL);
	return labelNumberOfElectrons;
}
/***********************************************************************************************/
void createOrcaChargeMultiplicityFrame(GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* sep;
	GtkWidget* labelNumberOfElectrons;
	GtkWidget* vboxFrame;
	GtkWidget* comboSpinMultiplicity = NULL;
	GtkWidget* comboCharge = NULL;
	GtkWidget *table = NULL;
	gint i;

	totalCharge = TotalCharges[0];
	spinMultiplicity=SpinMultiplicities[0];

	table = gtk_table_new(3,5,FALSE);

	frame = gtk_frame_new (_("Charge & Multiplicty"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	i = 0;
	comboCharge = addOrcaChargeToTable(table, i);
	i = 1;
	comboSpinMultiplicity = addOrcaSpinToTable(table, i);
	i = 2;
	sep = gtk_hseparator_new ();;
	gtk_table_attach(GTK_TABLE(table),sep,0,0+3,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  2,2);

	i = 3;
	labelNumberOfElectrons=addLabelNumberOfElectronsToTable(table, i, comboCharge);

	if(GTK_IS_COMBO_BOX(comboCharge))
		g_object_set_data(G_OBJECT (GTK_BIN(comboCharge)->child), "ComboSpinMultiplicity", comboSpinMultiplicity);
	setComboCharge(comboCharge);
	setComboSpinMultiplicity(comboSpinMultiplicity);
	if(GTK_IS_WIDGET(labelNumberOfElectrons))
	{
		gint ne = orcaMolecule.numberOfValenceElectrons - totalCharge;
		gchar buffer[BSIZE];
		sprintf(buffer, "Number of electrons = %d",ne);
		gtk_label_set_text(GTK_LABEL(labelNumberOfElectrons),buffer);
	}

	/* activate sensitivity */
	/*
	if(GTK_IS_WIDGET(comboMethod)) setComboMethod(comboMethod);
	g_object_set_data(G_OBJECT (box), "EntryMethod", GTK_BIN(comboMethod)->child);
	*/
}
/************************************************************************************************************/
