/* MopacMolecule.c */
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
#include "../Mopac/MopacTypes.h"
#include "../Mopac/MopacGlobal.h"
#include "../Mopac/MopacRem.h"
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
#include "../Utils/AtomsProp.h"

/************************************************************************************************************/
static gint totalCharge = 0;
static gint spinMultiplicity=1;

static gint numberOfPointsRP[2] = {10,10};
static gdouble stepValueRP[2] = {0.1,0.1};
static gchar typeRP[2][100] = {"Bond","Nothing"};
static gint atomRP[2] = {1,0};

/************************************************************************************************************/
void putMopacReactionPathInTextEditor()
{
	gchar buffer[BSIZE];
        gabedit_text_insert (GABEDIT_TEXT(text), NULL,  NULL, NULL, "1SCF ",-1);
	sprintf(buffer,"STEP=%g POINT=%d ",stepValueRP[0],numberOfPointsRP[0]);
	if(strcmp(typeRP[1],"Nothing"))
		sprintf(buffer,"STEP1=%g POINT1=%d STEP2=%g POINT2=%d",
				stepValueRP[0],numberOfPointsRP[0],
				stepValueRP[1],numberOfPointsRP[1]
				);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL,  NULL, NULL, buffer,-1);
}
/************************************************************************************************************/
gint getMopacMultiplicity()
{
	return spinMultiplicity;
}
/************************************************************************************************************/
void initMopacMoleculeButtons()
{
}
/************************************************************************************************************/
void initMopacMolecule()
{
	mopacMolecule.listOfAtoms = NULL;  
	mopacMolecule.totalNumberOfElectrons = 0;
	mopacMolecule.numberOfValenceElectrons = 0;
	mopacMolecule.numberOfAtoms = 0;
}
/************************************************************************************************************/
void freeMopacMolecule()
{
	static gboolean first = TRUE;

	if(first)
	{
		initMopacMolecule();
		first = FALSE;
		return;
	}

	if(mopacMolecule.listOfAtoms) g_free(mopacMolecule.listOfAtoms);
	initMopacMolecule();
}
/************************************************************************************************************/
static gint setMopacMoleculeFromSXYZ(gint nAtoms, gchar** symbols, gdouble* X, gdouble* Y, gdouble* Z)
{
	gint n;
	MopacAtom* atomList = NULL;

	mopacMolecule.listOfAtoms = NULL;  
	mopacMolecule.numberOfAtoms = 0;
	if(nAtoms<1) return 1;

	mopacMolecule.listOfAtoms = (MopacAtom*)g_malloc(sizeof(MopacAtom)*(nAtoms));
	if(mopacMolecule.listOfAtoms==NULL) return -1;

	mopacMolecule.numberOfAtoms = nAtoms;

	atomList = mopacMolecule.listOfAtoms;
	for(n=0; n<mopacMolecule.numberOfAtoms; n++)
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
static gboolean setMopacMoleculeFromGeomXYZ()
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

	mopacMolecule.totalNumberOfElectrons = 0;
	for(i=0; i<numberOfAtoms; i++)
	{
		SAtomsProp prop = prop_atom_get(GeomXYZ[i].Symb);

		symbols[i] = g_strdup(GeomXYZ[i].Symb);
		setXYZFromGeomXYZ(i, &X[i] , &Y[i] , &Z[i]);
		mopacMolecule.totalNumberOfElectrons += prop.atomicNumber;
	}
	mopacMolecule.numberOfValenceElectrons = mopacMolecule.totalNumberOfElectrons;
	setMopacMoleculeFromSXYZ(numberOfAtoms, symbols, X, Y, Z);

	for (i=0;i<(gint)NcentersXYZ;i++) g_free( symbols[i]);
	g_free( symbols);
	g_free(X);
	g_free(Y);
	g_free(Z);
	return TRUE;
}
/************************************************************************************************************/
static gboolean setMopacMoleculeFromGeomZMatrix()
{
	iprogram=PROG_IS_GAMESS;
	if(!zmat_to_xyz()) return FALSE;
	delete_dummy_atoms();
	/* conversion_zmat_to_xyz();*/
	return setMopacMoleculeFromGeomXYZ();
}
/************************************************************************************************************/
gboolean setMopacMolecule()
{
	freeMopacMolecule();
	if(MethodeGeom==GEOM_IS_XYZ && setMopacMoleculeFromGeomXYZ()) return TRUE;
	if(setMopacMoleculeFromGeomZMatrix()) return TRUE;
	return FALSE;
}
/************************************************************************************************************/
void setMopacGeometryFromInputFile(gchar* fileName)
{
	read_XYZ_from_mopac_input_file(fileName);
	setMopacMolecule();
}
/*************************************************************************************************************/
static gint getRealNumberXYZVariables()
{
	gint k=0;
	gint i;
        for(i=0;i<NcentersXYZ;i++)
	{
		if(!test(GeomXYZ[i].X))k++;
		if(!test(GeomXYZ[i].Y))k++;
		if(!test(GeomXYZ[i].Z))k++;
	}
	return k;
}
/*************************************************************************************************************/
static void setValueVariableXYZ(gchar* source, gchar* value, gint* k)
{
	if(!test(source)) 
	{
		sprintf(value,"%f ", get_value_variableXYZ(source));
		*k = 1;
	}
	else
	{
		sprintf(value,"%s ",source);
		*k = 0;
	}
}
/*************************************************************************************************************/
static void setValueVariableZmat(gchar* source, gchar* value, gint* k)
{
	if(!test(source)) 
	{
		sprintf(value,"%f ", get_value_variableZmat(source));
		*k = 1;
	}
	else
	{
		sprintf(value,"%s ",source);
		*k = 0;
	}
}
/*************************************************************************************************************/
static gint getRealNumberZmatVariables()
{
	gint k=0;
	gint i;
        for(i=0;i<NcentersZmat;i++)
	{
        	if(Geom[i].Nentry>NUMBER_ENTRY_0 && !test(Geom[i].R)) k++;
        	if(Geom[i].Nentry>NUMBER_ENTRY_R && !test(Geom[i].Angle)) k++;
        	if(Geom[i].Nentry>NUMBER_ENTRY_ANGLE && !test(Geom[i].Dihedral)) k++;
	}
	return k;
}
/*************************************************************************************************************/
static void putMopacMoleculeInTextEditor()
{
        gchar buffer[BSIZE];
        gchar b1[20];
        gchar b2[20];
        gchar b3[20];
        gchar con[20];
	gint i;
	gint k1 = 0;
	gint k2 = 0;
	gint k3 = 0;
	gint nvar = 0;
	gint k;
	gboolean RP = FALSE;

	if(mopacMolecule.numberOfAtoms<1) return;
	RP =!strcmp(getSeletedJobType(),"RP");
	if(MethodeGeom==GEOM_IS_XYZ)
		nvar  = getRealNumberXYZVariables();
	else
		nvar  = getRealNumberZmatVariables();

	if(MethodeGeom==GEOM_IS_XYZ)
      		for (i=0;i<NcentersXYZ;i++)
		{
			setValueVariableXYZ(GeomXYZ[i].X, b1, &k1);
			setValueVariableXYZ(GeomXYZ[i].Y, b2, &k2);
			setValueVariableXYZ(GeomXYZ[i].Z, b3, &k3);
			if(nvar==0) { k1 = 1; k2 = 1; k3 = 1;}
			if(RP)
			{
				for(k=0;k<2;k++)
				{
					if(!strcmp(typeRP[k],"X") && i==atomRP[k]-1) k1 = -1;
					if(!strcmp(typeRP[k],"Y") && i==atomRP[k]-1) k2 = -1;
					if(!strcmp(typeRP[k],"Z") && i==atomRP[k]-1) k3 = -1;
				}
			}

			sprintf(buffer,"%s  %s %d %s %d %s %d\n",GeomXYZ[i].Symb,
					b1, k1,
					b2, k2,
					b3, k3
					);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
		}
	else
	{
        	for(i=0;i<NcentersZmat;i++)
        	{
			SAtomsProp prop = prop_atom_get(Geom[i].Symb);
        		if(Geom[i].Nentry>NUMBER_ENTRY_ANGLE)
			{
				setValueVariableZmat(Geom[i].R, b1, &k1);
				setValueVariableZmat(Geom[i].Angle, b2, &k2);
				setValueVariableZmat(Geom[i].Dihedral, b3, &k3);
				sprintf(con,"%s %s %s", Geom[i].NR, Geom[i].NAngle, Geom[i].NDihedral);
				if(nvar==0) { k1 = 1; k2 = 1; k3 = 1;}

				if(RP)
				{
					for(k=0;k<2;k++)
					{
						if(!strcmp(typeRP[k],"Bond") && i==atomRP[k]-1) k1 = -1;
						if(!strcmp(typeRP[k],"Angle") && i==atomRP[k]-1) k2 = -1;
						if(!strcmp(typeRP[k],"Dihedral") && i==atomRP[k]-1) k3 = -1;
					}
				}

				sprintf(buffer,"%s  %s %d %s %d %s %d %s\n",
						Geom[i].Symb,
						b1, k1,
						b2, k2,
						b3, k3,
						con
						);
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, &prop.color, buffer, -1);
			}
			else
        		if(Geom[i].Nentry>NUMBER_ENTRY_R)
			{
				setValueVariableZmat(Geom[i].R, b1, &k1);
				setValueVariableZmat(Geom[i].Angle, b2, &k2);
				sprintf(b3,"0.0"); k3 = 0;
				sprintf(con,"%s %s %s", Geom[i].NR, Geom[i].NAngle, "0");

				if(nvar==0) { k1 = 1; k2 = 1;}
				if(RP)
				{
					for(k=0;k<2;k++)
					{
						if(!strcmp(typeRP[k],"Bond") && i==atomRP[k]-1) k1 = -1;
						if(!strcmp(typeRP[k],"Angle") && i==atomRP[k]-1) k2 = -1;
					}
				}
				sprintf(buffer,"%s  %s %d %s %d %s %d %s\n",
						Geom[i].Symb,
						b1, k1,
						b2, k2,
						b3, k3,
						con
						);

        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, &prop.color, buffer, -1);
			}
			else
        		if(Geom[i].Nentry>NUMBER_ENTRY_0)
			{
				setValueVariableZmat(Geom[i].R, b1, &k1);
				sprintf(b2,"0.0"); k2 = 0;
				sprintf(b3,"0.0"); k3 = 0;
				sprintf(con,"%s %s %s", Geom[i].NR, "0", "0");

				if(nvar==0) { k1 = 1;}
				if(RP)
				for(k=0;k<2;k++)
					if(!strcmp(typeRP[k],"Bond") && i==atomRP[k]-1) k1 = -1;

				sprintf(buffer,"%s  %s %d %s %d %s %d %s\n",
						Geom[i].Symb,
						b1, k1,
						b2, k2,
						b3, k3,
						con
						);
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, &prop.color, buffer, -1);
			}
			else
			{
				sprintf(b1,"0.0"); k1 = 0;
				sprintf(b2,"0.0"); k2 = 0;
				sprintf(b3,"0.0"); k3 = 0;
				sprintf(con,"%s %s %s", "0.0", "0", "0");
				sprintf(buffer,"%s  %s %d %s %d %s %d %s\n",
						Geom[i].Symb,
						b1, k1,
						b2, k2,
						b3, k3,
						con
						);
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, &prop.color, buffer, -1);
			}
        	}
	}

}
/************************************************************************************************************/
static void getMultiplicityName(gint multiplicity, gchar* buffer)
{
	if(multiplicity==1) sprintf(buffer,"Singlet");
	else if(multiplicity==2) sprintf(buffer,"Doublet");
	else if(multiplicity==3) sprintf(buffer,"Triplet");
	else if(multiplicity==4) sprintf(buffer,"Quartet");
	else if(multiplicity==5) sprintf(buffer,"Quintet");
	else if(multiplicity==6) sprintf(buffer,"Sextet");
	else sprintf(buffer,"UNKNOWN");
}
/************************************************************************************************************/
void putMopacChargeMultiplicityInTextEditor()
{
	gchar buffer[BSIZE];
	gchar mul[20];
	getMultiplicityName(spinMultiplicity, mul);
	if(spinMultiplicity>1)
	sprintf(buffer,"UHF CHARGE=%d %s ",totalCharge,mul);
	else
	sprintf(buffer,"CHARGE=%d %s ",totalCharge,mul);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL,  NULL, NULL, buffer,-1);
}
/************************************************************************************************************/
void putMopacMoleculeInfoInTextEditor()
{
	putMopacMoleculeInTextEditor();
}
/************************************************************************************************************/
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
	gint ne = mopacMolecule.numberOfValenceElectrons - totalCharge;

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

	nlist = mopacMolecule.numberOfValenceElectrons*2-2+1;

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
	}
	else 
	{
		/* remove RHF from list*/
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
		gint ne = mopacMolecule.numberOfValenceElectrons - totalCharge;
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
	gtk_widget_set_size_request(GTK_WIDGET(entry),(gint)(ScreenHeight*0.1),-1);

	return entry;
}
/***********************************************************************************************/
static GtkWidget *addMopacChargeToTable(GtkWidget *table, gint i)
{
	GtkWidget* entryCharge = NULL;
	GtkWidget* comboCharge = NULL;
	gint nlistCharge = 1;
	gchar* listCharge[] = {"0"};

	add_label_table(table,"Charge",(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entryCharge = addComboListToATable(table, listCharge, nlistCharge, i, 2, 1);
	comboCharge  = g_object_get_data(G_OBJECT (entryCharge), "Combo");
	gtk_widget_set_sensitive(entryCharge, FALSE);

	return comboCharge;
}
/***********************************************************************************************/
static GtkWidget *addMopacSpinToTable(GtkWidget *table, gint i)
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
void createMopacChargeMultiplicityFrame(GtkWidget *box)
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
	comboCharge = addMopacChargeToTable(table, i);
	i = 1;
	comboSpinMultiplicity = addMopacSpinToTable(table, i);
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
		gint ne = mopacMolecule.numberOfValenceElectrons - totalCharge;
		gchar buffer[BSIZE];
		sprintf(buffer, _("Number of electrons = %d"),ne);
		gtk_label_set_text(GTK_LABEL(labelNumberOfElectrons),buffer);
	}

	/* activate sensitivity */
	/*
	if(GTK_IS_WIDGET(comboMethod)) setComboMethod(comboMethod);
	g_object_set_data(G_OBJECT (box), "EntryMethod", GTK_BIN(comboMethod)->child);
	*/
}
/************************************************************************************************************/
static void setComboReactionPathVariableType(GtkWidget *comboReactionPathVariableType, gboolean nothing)
{
	GList *glist = NULL;
	gint i;
	gchar* listXYZ[] = {"Nothing","X","Y","Z"};
	gchar* listZMatrix[] = {"Nothing","Bond","Angle","Dihedral"};
	gchar** list = NULL;
	gint iBegin = 0;
	gint iEnd = 3;

	if(!nothing) iBegin = 1;
	if(MethodeGeom==GEOM_IS_XYZ) list = listXYZ;
	else list = listZMatrix;

	if(MethodeGeom!=GEOM_IS_XYZ)
	{
		if(mopacMolecule.numberOfAtoms<=2) iEnd-=2;
		else if(mopacMolecule.numberOfAtoms<=3) iEnd--;
	}

  	for(i=iBegin;i<=iEnd;i++) glist = g_list_append(glist,list[i]);

  	gtk_combo_box_entry_set_popdown_strings( comboReactionPathVariableType, glist) ;
  	g_list_free(glist);
}
/********************************************************************************/
static void setComboReactionPathAtoms(GtkWidget *comboReactionPathAtoms)
{
	GList *glist = NULL;
	gint i;
	gint nlist;
	gchar** list = NULL;
	GtkWidget* entry = NULL;
	gchar* typeRP   = NULL;
	gint iEnd;

	if(!comboReactionPathAtoms) return;
	entry = GTK_BIN (comboReactionPathAtoms)->child;
	if(!entry) return;
	typeRP   = g_object_get_data(G_OBJECT (entry), "TypeRP");
	if(!typeRP) return;

	nlist = mopacMolecule.numberOfAtoms;
	iEnd = nlist-1;
	if(nlist<1) return;

	list = g_malloc(nlist*sizeof(gchar*));
	if(!list) return;
	for(i=0;i<nlist;i++) list[i] = NULL;
	if(!strcmp(typeRP,"X") || !strcmp(typeRP,"Y") || !strcmp(typeRP,"Z"))
	{
		iEnd = nlist-1;
		for(i=0;i<=iEnd;i++)
			list[i] = g_strdup_printf("%d",i+1);
	}
	else if(!strcmp(typeRP,"Bond"))
	{
		iEnd = nlist-2;
		for(i=0;i<=iEnd;i++)
		{
			gint a = i+1;
			gint b = atoi(Geom[a].NR)-1;
			list[i] = g_strdup_printf("%d [%s%d-%s%d]",a+1,Geom[a].Symb,a+1,Geom[b].Symb,b+1);
		}
	}
	else if(!strcmp(typeRP,"Angle"))
	{
		iEnd = nlist-3;
		for(i=0;i<=iEnd;i++)
		{
			gint a = i+2;
			gint b = atoi(Geom[a].NR)-1;
			gint c = atoi(Geom[a].NAngle)-1;
			list[i] = g_strdup_printf("%d [%s%d-%s%d-%s%d]",a+1, 
					Geom[a].Symb,a+1,
					Geom[b].Symb,b+1,
					Geom[c].Symb,c+1);
		}
	}
	else if(!strcmp(typeRP,"Dihedral"))
	{
		iEnd = nlist-4;
		for(i=0;i<=iEnd;i++)
		{
			gint a = i+3;
			gint b = atoi(Geom[a].NR)-1;
			gint c = atoi(Geom[a].NAngle)-1;
			gint d = atoi(Geom[a].NDihedral)-1;
			list[i] = g_strdup_printf("%d [%s%d-%s%d-%s%d-%s%d]",a+1, 
					Geom[a].Symb,a+1,
					Geom[b].Symb,b+1,
					Geom[c].Symb,c+1,
					Geom[d].Symb,d+1
					);
		}
	}
	else 
	{
		iEnd = 0;
		list[0] = g_strdup_printf(" ");
	}

  	for(i=0;i<=iEnd;i++) glist = g_list_append(glist,list[i]);

  	gtk_combo_box_entry_set_popdown_strings( comboReactionPathAtoms, glist) ;
  	g_list_free(glist);
	if(list)
	{
		for(i=0;i<nlist;i++) if(list[i]) g_free(list[i]);
		g_free(list);
	}
}
/**********************************************************************/
static void changedEntryReactionPathType(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	gchar* typeRP;
	GtkWidget* entryStep = NULL;
	GtkWidget* entryNbPoints = NULL;
	GtkWidget* comboAtoms = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;
	typeRP   = g_object_get_data(G_OBJECT (entry), "TypeRP");
	if(!typeRP) return;
	sprintf(typeRP,"%s",entryText);
	entryStep = g_object_get_data(G_OBJECT (entry), "EntryStep");
	entryNbPoints =	g_object_get_data(G_OBJECT (entry), "EntryNbPoints");
	if (!strcmp(typeRP,"X") || !strcmp(typeRP,"Y") || !strcmp(typeRP,"Z") || !strcmp(typeRP,"Bond"))
	{
		if(entryStep) gtk_entry_set_text(GTK_ENTRY(entryStep),"0.1");
		if(entryNbPoints) gtk_entry_set_text(GTK_ENTRY(entryNbPoints),"20");
	}
	else
	{
		if(entryStep) gtk_entry_set_text(GTK_ENTRY(entryStep),"1.0");
		if(entryNbPoints) gtk_entry_set_text(GTK_ENTRY(entryNbPoints),"360");
	}
	comboAtoms = g_object_get_data(G_OBJECT (entry), "ComboAtoms");
	if(comboAtoms)setComboReactionPathAtoms(comboAtoms);
}
/**********************************************************************/
static void changedEntryReactionPathAtoms(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	gint* atomRP = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;
	atomRP   = g_object_get_data(G_OBJECT (entry), "AtomRP");
	if(!atomRP) return;
	sscanf(entryText,"%d",atomRP);
}
/**********************************************************************/
static void changedEntryReactionPathStep(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	gdouble* stepValue = NULL;

	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;
	stepValue = g_object_get_data(G_OBJECT (entry), "StepValueRP");
	if(!stepValue) return;
	*stepValue = atof(entryText);
}
/**********************************************************************/
static void changedEntryReactionPathNbPoints(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	gint* numberOfPoints = NULL;

	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;
	if(atoi(entryText)<1) return;
	numberOfPoints = g_object_get_data(G_OBJECT (entry), "NumberOfPointsRP");
	if(!numberOfPoints) return;
	*numberOfPoints = atoi(entryText);
}
/***********************************************************************************************/
static GtkWidget *addMopacReactionPathVariableToTable(GtkWidget *table, gint i,gint j)
{
	GtkWidget* entry = NULL;
	GtkWidget* combo = NULL;
	gint nlist = 1;
	gchar* list[] = {"Bond"};

	entry = addComboListToATable(table, list, nlist, i, j, 1);
	combo  = g_object_get_data(G_OBJECT (entry), "Combo");
	gtk_widget_set_sensitive(entry, FALSE);
	g_signal_connect(G_OBJECT(entry),"changed", G_CALLBACK(changedEntryReactionPathType),NULL);

	return combo;
}
/***********************************************************************************************/
static GtkWidget *addMopacReactionPathAtomsToTable(GtkWidget *table, gint i, gint j)
{
	GtkWidget* entry = NULL;
	GtkWidget* combo = NULL;
	gint nlist = 1;
	gchar* list[] = {"1-2"};

	entry = addComboListToATable(table, list, nlist, i, j, 1);
	combo  = g_object_get_data(G_OBJECT (entry), "Combo");
	gtk_widget_set_sensitive(entry, FALSE);

	g_signal_connect(G_OBJECT(entry),"changed", G_CALLBACK(changedEntryReactionPathAtoms),NULL);
	return combo;
}
/***********************************************************************************************/
void createReactionPathFrame(GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* label;
	GtkWidget* comboVariableType[2] = {NULL,NULL}; /* X, Y, Z, R, Angle or Dihedral */
	GtkWidget* comboAtoms[2] = {NULL,NULL}; /* n1-n2-n3-n4 (Symb1-Symb2-Symb3-Symb4)*/
	GtkWidget* entryNbPoints[2] = {NULL,NULL};
	GtkWidget* entryStep[2] = {NULL,NULL};
	GtkWidget *table = NULL;
	gint i;
	gint j;
	gint k;

	table = gtk_table_new(3,5,FALSE);

	sprintf(typeRP[1],"Nothing");
	if(MethodeGeom==GEOM_IS_XYZ) sprintf(typeRP[0],"X");
	else sprintf(typeRP[0],"Bond");

	numberOfPointsRP[0] = 10;
	numberOfPointsRP[1] = 10;
	stepValueRP[0] = 0.1;
	stepValueRP[1] = 0.1;

	frame = gtk_frame_new (_("Reaction path"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	i = 0; j = 1;
	label = gtk_label_new(_("Type"));
	gtk_table_attach(GTK_TABLE(table),label,j,j+1,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	i = 0; j = 2;
	label = gtk_label_new(_("Atoms"));
	gtk_table_attach(GTK_TABLE(table),label,j,j+1,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	i = 0; j = 3;
	label = gtk_label_new(_("Nb points"));
	gtk_table_attach(GTK_TABLE(table),label,j,j+1,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	i = 0; j = 4;
	label = gtk_label_new(_("Step value"));
	gtk_table_attach(GTK_TABLE(table),label,j,j+1,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);

	i = 1; 
	j = 0;
	label = gtk_label_new(_("First"));
	gtk_table_attach(GTK_TABLE(table),label,j,j+1,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	j = 1;
	comboVariableType[0] = addMopacReactionPathVariableToTable(table, i,j);
	j = 2;
	comboAtoms[0] = addMopacReactionPathAtomsToTable(table, i,j);
	j = 3;
	entryNbPoints[0] = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entryNbPoints[0]),(gint)(ScreenHeight*0.1),-1);
	gtk_table_attach(GTK_TABLE(table),entryNbPoints[0],j,j+1,i,i+1,
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  2,2);
	j = 4;
	entryStep[0] = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entryStep[0]),(gint)(ScreenHeight*0.1),-1);
	gtk_table_attach(GTK_TABLE(table),entryStep[0],j,j+1,i,i+1,
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  2,2);
	i = 2; 
	j = 0;
	label = gtk_label_new(_("Second"));
	gtk_table_attach(GTK_TABLE(table),label,j,j+1,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	j = 1;
	comboVariableType[1] = addMopacReactionPathVariableToTable(table, i,j);
	j = 2;
	comboAtoms[1] = addMopacReactionPathAtomsToTable(table, i,j);
	j = 3;
	entryNbPoints[1] = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entryNbPoints[1]),(gint)(ScreenHeight*0.1),-1);
	gtk_table_attach(GTK_TABLE(table),entryNbPoints[1],j,j+1,i,i+1,
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  2,2);
	j = 4;
	entryStep[1] = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entryStep[1]),(gint)(ScreenHeight*0.1),-1);
	gtk_table_attach(GTK_TABLE(table),entryStep[1],j,j+1,i,i+1,
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  2,2);

	for(k=0;k<2;k++) gtk_entry_set_text(GTK_ENTRY(entryStep[k]),"0.1");
	for(k=0;k<2;k++) gtk_entry_set_text(GTK_ENTRY(entryNbPoints[k]),"20");
	for(k=0;k<2;k++)
	if(GTK_IS_COMBO_BOX(comboVariableType[k]))
	{
		g_object_set_data(G_OBJECT (GTK_BIN(comboVariableType[k])->child), "ComboAtoms", comboAtoms[k]);
		g_object_set_data(G_OBJECT (GTK_BIN(comboVariableType[k])->child), "TypeRP", typeRP[k]);
		g_object_set_data(G_OBJECT (GTK_BIN(comboVariableType[k])->child), "EntryStep", entryStep[k]);
		g_object_set_data(G_OBJECT (GTK_BIN(comboVariableType[k])->child), "EntryNbPoints", entryNbPoints[k]);
	}
	for(k=0;k<2;k++)
	if(GTK_IS_COMBO_BOX(comboAtoms[k]))
	{
		g_object_set_data(G_OBJECT (GTK_BIN(comboAtoms[k])->child), "EntryStep", entryStep[k]);
		g_object_set_data(G_OBJECT (GTK_BIN(comboAtoms[k])->child), "EntryNbPoints", entryNbPoints[k]);
		g_object_set_data(G_OBJECT (GTK_BIN(comboAtoms[k])->child), "AtomRP", &atomRP[k]);
		g_object_set_data(G_OBJECT (GTK_BIN(comboAtoms[k])->child), "TypeRP", typeRP[k]);
	}
	for(k=0;k<2;k++) g_object_set_data(G_OBJECT (entryNbPoints[k]), "NumberOfPointsRP", &numberOfPointsRP[k]);
	for(k=0;k<2;k++) g_object_set_data(G_OBJECT (entryStep[k]), "StepValueRP", &stepValueRP[k]);

	for(k=0;k<2;k++) g_signal_connect(G_OBJECT(entryStep[k]),"changed", G_CALLBACK(changedEntryReactionPathStep),NULL);
	for(k=0;k<2;k++) g_signal_connect(G_OBJECT(entryNbPoints[k]),"changed", G_CALLBACK(changedEntryReactionPathNbPoints),NULL);

	for(k=0;k<2;k++) setComboReactionPathVariableType(comboVariableType[k],k>0);
	for(k=0;k<2;k++) setComboReactionPathAtoms(comboAtoms[k]);
}
/************************************************************************************************************/
