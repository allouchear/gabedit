/* NWChemMolecule.c */
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
#include "../NWChem/NWChemTypes.h"
#include "../NWChem/NWChemGlobal.h"
#include "../NWChem/NWChemKeywords.h"
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
#include "../Symmetry/MoleculeSymmetry.h"
#include "../Symmetry/MoleculeSymmetryInterface.h"

/************************************************************************************************************/
static gint totalCharge = 0;
static gint spinMultiplicity=1;
/************************************************************************************************************/
gint getNWChemMultiplicity()
{
	return spinMultiplicity;
}
/************************************************************************************************************/
void initNWChemMoleculeButtons()
{
}
/************************************************************************************************************/
void initNWChemMolecule()
{
	nwchemMolecule.listOfAtoms = NULL;  
	nwchemMolecule.totalNumberOfElectrons = 0;
	nwchemMolecule.numberOfValenceElectrons = 0;
	nwchemMolecule.numberOfAtoms = 0;
}
/************************************************************************************************************/
void freeNWChemMolecule()
{
	static gboolean first = TRUE;

	if(first)
	{
		initNWChemMolecule();
		first = FALSE;
		return;
	}

	if(nwchemMolecule.listOfAtoms) g_free(nwchemMolecule.listOfAtoms);
	initNWChemMolecule();
}
/************************************************************************************************************/
static gint setNWChemMoleculeFromSXYZ(gint nAtoms, gchar** symbols, gdouble* X, gdouble* Y, gdouble* Z)
{
	gint n;
	NWChemAtom* atomList = NULL;

	nwchemMolecule.listOfAtoms = NULL;  
	nwchemMolecule.numberOfAtoms = 0;
	if(nAtoms<1) return 1;

	nwchemMolecule.listOfAtoms = (NWChemAtom*)g_malloc(sizeof(NWChemAtom)*(nAtoms));
	if(nwchemMolecule.listOfAtoms==NULL) return -1;

	nwchemMolecule.numberOfAtoms = nAtoms;

	atomList = nwchemMolecule.listOfAtoms;
	for(n=0; n<nwchemMolecule.numberOfAtoms; n++)
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
static gboolean setNWChemMoleculeFromGeomXYZ()
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

	nwchemMolecule.totalNumberOfElectrons = 0;
	for(i=0; i<numberOfAtoms; i++)
	{
		SAtomsProp prop = prop_atom_get(GeomXYZ[i].Symb);

		symbols[i] = g_strdup(GeomXYZ[i].Symb);
		setXYZFromGeomXYZ(i, &X[i] , &Y[i] , &Z[i]);
		nwchemMolecule.totalNumberOfElectrons += prop.atomicNumber;
	}
	nwchemMolecule.numberOfValenceElectrons = nwchemMolecule.totalNumberOfElectrons;
	setNWChemMoleculeFromSXYZ(numberOfAtoms, symbols, X, Y, Z);

	for (i=0;i<(gint)NcentersXYZ;i++) g_free( symbols[i]);
	g_free( symbols);
	g_free(X);
	g_free(Y);
	g_free(Z);
	return TRUE;
}
/************************************************************************************************************/
static gboolean setNWChemMoleculeFromGeomZMatrix()
{
	iprogram=PROG_IS_NWCHEM;
	if(!zmat_to_xyz()) return FALSE;
	delete_dummy_atoms();
	/* conversion_zmat_to_xyz();*/
	return setNWChemMoleculeFromGeomXYZ();
}
/************************************************************************************************************/
gboolean setNWChemMolecule()
{
	freeNWChemMolecule();
	if(MethodeGeom==GEOM_IS_XYZ && setNWChemMoleculeFromGeomXYZ()) return TRUE;
	if(setNWChemMoleculeFromGeomZMatrix()) return TRUE;
	return FALSE;
}
/************************************************************************************************************/
void setNWChemGeometryFromInputFile(gchar* fileName)
{
	read_XYZ_from_nwchem_input_file(fileName);
	setNWChemMolecule();
}
/************************************************************************************************************/
static gboolean testAbelianGroup()
{
	gint i;
	gchar** symbols = NULL;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gdouble* Z = NULL;
	gint numberOfAtoms = nwchemMolecule.numberOfAtoms;
	gchar pointGroupSymbol[BSIZE];
	gchar abelianPointGroupSymbol[BSIZE];
	gchar message[BSIZE];
	gint maximalOrder = 20;
	gint nGenerators;
	gint nMolcas = 0;
	gint nElements;
	gchar* generators[3];
	gchar* molcasGenerators[3];
	gchar* elements[8];
	gdouble principalAxisTolerance = getTolerancePrincipalAxis();
	gdouble positionTolerance = getTolerancePosition();

	if(numberOfAtoms<1) return FALSE;

	for(i=0;i<3;i++)
	{
		generators[i] = g_malloc(100*sizeof(gchar));
		molcasGenerators[i] = g_malloc(100*sizeof(gchar));
	}
	for(i=0;i<8;i++) elements[i] = g_malloc(100*sizeof(gchar));

	symbols = (gchar**)g_malloc(sizeof(gchar*)*(numberOfAtoms));
	if(symbols == NULL) return FALSE;

	X = (gdouble*)g_malloc(sizeof(gdouble)*(numberOfAtoms));
	if(X == NULL) return FALSE;
	Y = (gdouble*)g_malloc(sizeof(gdouble)*(numberOfAtoms));
	if(Y == NULL) return FALSE;
	Z = (gdouble*)g_malloc(sizeof(gdouble)*(numberOfAtoms));
	if(Z == NULL) return FALSE;

	for(i=0; i<numberOfAtoms; i++)
	{
		symbols[i] = g_strdup(nwchemMolecule.listOfAtoms[i].symbol);
		X[i] = nwchemMolecule.listOfAtoms[i].position[0];
		Y[i] = nwchemMolecule.listOfAtoms[i].position[1];
		Z[i] = nwchemMolecule.listOfAtoms[i].position[2];
	}
	computeAbelianGroup(principalAxisTolerance, pointGroupSymbol, abelianPointGroupSymbol, maximalOrder, TRUE,
	       	&numberOfAtoms, symbols, X, Y, Z, 
		&nGenerators, generators, &nMolcas, molcasGenerators, &nElements, elements, &positionTolerance, message);

	for(i=0;i<3;i++)
	{
		g_free(generators[i]);
		g_free(molcasGenerators[i]);
	}
	for(i=0;i<8;i++) g_free(elements[i]);

	for (i=0;i<(gint)numberOfAtoms;i++) g_free( symbols[i]);
	g_free( symbols);
	g_free(X);
	g_free(Y);
	g_free(Z);
	if(!strcmp( pointGroupSymbol,abelianPointGroupSymbol))return TRUE;
	return FALSE;
}
/*************************************************************************************************************/
/*
static gdouble getMinDistance()
{
	gdouble d=0;
	gint i;
	gint k;
	NWChemAtom* atomList = nwchemMolecule.listOfAtoms;
	for(i=0; i<nwchemMolecule.numberOfAtoms-1; i++)
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
static void putNWChemMoleculeInTextEditor()
{
        gchar buffer[BSIZE];
	gint i;
	gint nV = 0;
	gdouble x,y,z;

	if(nwchemMolecule.numberOfAtoms<1) return;

	if(MethodeGeom==GEOM_IS_XYZ)
	{
		sprintf(buffer,"\ncharge %d\n",totalCharge);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, buffer, -1);
		sprintf(buffer,"\ngeometry\n");
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, buffer, -1);
      		for (i=0;i<nwchemMolecule.numberOfAtoms;i++)
		{
			gchar X[100];
			gchar Y[100];
			gchar Z[100];
			setXYZFromGeomXYZ(i, &x, &y, &z);
			sprintf(X,"%f",x);
			sprintf(Y,"%f",y);
			sprintf(Z,"%f",z);
			sprintf(buffer," %s  %s %s %s\n",nwchemMolecule.listOfAtoms[i].symbol, X,Y,Z);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
		}
		if(strstr(getNWChemExcitedMethod(),"CIS")||strstr(getNWChemExcitedMethod(),"TDDFT") || strstr(getNWChemTypeMethod(),"HL-"))
		{
			if(!testAbelianGroup())
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " symmetry C1\n", -1);
		}
		sprintf(buffer,"end\n");
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, buffer, -1);
		nV = 0;
        	if(NVariablesXYZ>0)
        	for(i=0;i<NVariablesXYZ;i++)
        	{
        		if(VariablesXYZ[i].Used) nV++;
        	}
		if(nV>0 && nV!= 3*nwchemMolecule.numberOfAtoms) 
		{
			sprintf(buffer,"\nconstraints\n");
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, buffer, -1);
      			for (i=0;i<nwchemMolecule.numberOfAtoms;i++)
			{
  				if(!(!test(GeomXYZ[i].X) || !test(GeomXYZ[i].Y) || !test(GeomXYZ[i].Z)))
				{
					sprintf(buffer," fix atom %d\n",i+1);
        				gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,buffer,-1);
				}
			}
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, "end\n", -1);
		}
	}
	else
	{
		sprintf(buffer,"\ncharge %d\n",totalCharge);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, buffer, -1);
		sprintf(buffer,"\ngeometry\n");
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, buffer, -1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\t",-1);
		sprintf(buffer,"zmatrix\n");
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, buffer, -1);
        	for(i=0;i<NcentersZmat;i++)
        	{
			SAtomsProp prop = prop_atom_get(Geom[i].Symb);
  			gchar *line;
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\t",-1);
			line=g_strdup_printf("%s",Geom[i].Symb);
        		if(Geom[i].Nentry>NUMBER_ENTRY_0) line=g_strdup_printf("%s\t%s\t%s",line,Geom[i].NR,Geom[i].R);
        		if(Geom[i].Nentry>NUMBER_ENTRY_R) line=g_strdup_printf("%s\t%s\t%s",line,Geom[i].NAngle,Geom[i].Angle);
        		if(Geom[i].Nentry>NUMBER_ENTRY_ANGLE) line=g_strdup_printf("%s\t%s\t%s",line,Geom[i].NDihedral,Geom[i].Dihedral);
  			line=g_strdup_printf("%s\n",line);
 			prop = prop_atom_get(Geom[i].Symb);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL,NULL, &prop.color,line,-1);
			g_free(line);
        	}
        	if(NVariables>0)
		{
			gint nV = 0;
        		for(i=0;i<NVariables;i++)
        		{
        			if(Variables[i].Used)
				{
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\tvariables\n",-1);
				nV++;
				break;
				}
        		}
        		for(i=0;i<NVariables;i++)
        		{
        			if(Variables[i].Used)
				{
  				gchar* line=g_strdup_printf("\t%s\t%s\n",Variables[i].Name,Variables[i].Value);
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,line,-1);
				g_free(line);
				}
        		}
			if(nV>0)
			{
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\t",-1);
				gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord,"end\n",-1);
			}
			
		}
		gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord,"end\n",-1);
	}

}
/************************************************************************************************************/
void putNWChemMoleculeInfoInTextEditor()
{
	putNWChemMoleculeInTextEditor();
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
	gint ne = nwchemMolecule.numberOfValenceElectrons - totalCharge;

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

	nlist = nwchemMolecule.numberOfValenceElectrons*2-2+1;

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
		setNWChemSCFMethod(TRUE);
	}
	else 
	{
		/* remove RHF from list*/
		setNWChemSCFMethod(FALSE);
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
		gint ne = nwchemMolecule.numberOfValenceElectrons - totalCharge;
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
static GtkWidget *addNWChemChargeToTable(GtkWidget *table, gint i)
{
	GtkWidget* entryCharge = NULL;
	GtkWidget* comboCharge = NULL;
	gint nlistCharge = 1;
	gchar* listCharge[] = {"0"};

	add_label_table(table,_("Charge"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entryCharge = addComboListToATable(table, listCharge, nlistCharge, i, 2, 1);
	comboCharge  = g_object_get_data(G_OBJECT (entryCharge), "Combo");
	gtk_widget_set_sensitive(entryCharge, FALSE);

	return comboCharge;
}
/***********************************************************************************************/
static GtkWidget *addNWChemSpinToTable(GtkWidget *table, gint i)
{
	GtkWidget* entrySpinMultiplicity = NULL;
	GtkWidget* comboSpinMultiplicity = NULL;
	gint nlistspinMultiplicity = 1;
	gchar* listspinMultiplicity[] = {"0"};

	add_label_table(table,_("Spin multiplicity"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entrySpinMultiplicity = addComboListToATable(table, listspinMultiplicity, nlistspinMultiplicity, i, 2, 1);
	comboSpinMultiplicity  = g_object_get_data(G_OBJECT (entrySpinMultiplicity), "Combo");
	gtk_widget_set_sensitive(entrySpinMultiplicity, FALSE);

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
void createNWChemChargeMultiplicityFrame(GtkWidget *box)
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
	comboCharge = addNWChemChargeToTable(table, i);
	i = 1;
	comboSpinMultiplicity = addNWChemSpinToTable(table, i);
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
		gint ne = nwchemMolecule.numberOfValenceElectrons - totalCharge;
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
