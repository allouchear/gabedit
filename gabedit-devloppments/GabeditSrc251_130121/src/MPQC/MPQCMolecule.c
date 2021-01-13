/* MPQCMolecule.c */
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

#include "../../Config.h"
#include "../Common/Global.h"
#include "../MPQC/MPQCTypes.h"
#include "../MPQC/MPQCGlobal.h"
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
#include "../Symmetry/MoleculeSymmetry.h"
#include "../Symmetry/MoleculeSymmetryInterface.h"

/************************************************************************************************************/
typedef enum
{
	AUTO = 0, GABEDIT, GABFIXED
}TypeOfSymmetryButton;


static gboolean symReduction = TRUE;
static GtkWidget* labelSymmetry = NULL;
static GtkWidget *buttonTolerance = NULL;
/************************************************************************************************************/
void initMoleculeButtons()
{
	labelSymmetry = NULL;
	buttonTolerance = NULL;
}
/************************************************************************************************************/
void initMPQCMolecule()
{
	mpqcMolecule.listOfAtoms = NULL;  
	mpqcMolecule.totalNumberOfElectrons = 0;
	mpqcMolecule.numberOfValenceElectrons = 0;
	mpqcMolecule.numberOfAtoms = 0;
	mpqcMolecule.groupSymmetry = NULL;
}
/************************************************************************************************************/
void freeMPQCMolecule()
{
	static gboolean first = TRUE;

	if(first)
	{
		initMPQCMolecule();
		first = FALSE;
		return;
	}

	if(mpqcMolecule.listOfAtoms) g_free(mpqcMolecule.listOfAtoms);
	if(mpqcMolecule.groupSymmetry) g_free(mpqcMolecule.groupSymmetry);
	initMPQCMolecule();
}
/************************************************************************************************************/
static gint setMPQCMoleculeFromSXYZ(gint nAtoms, gchar** symbols, gdouble* X, gdouble* Y, gdouble* Z)
{
	gint n;
	MPQCAtom* atomList = NULL;

	mpqcMolecule.listOfAtoms = NULL;  
	mpqcMolecule.numberOfAtoms = 0;
	if(nAtoms<1) return 1;

	mpqcMolecule.listOfAtoms = (MPQCAtom*)g_malloc(sizeof(MPQCAtom)*(nAtoms));
	if(mpqcMolecule.listOfAtoms==NULL) return -1;

	mpqcMolecule.numberOfAtoms = nAtoms;

	atomList = mpqcMolecule.listOfAtoms;
	for(n=0; n<mpqcMolecule.numberOfAtoms; n++)
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
static gchar* computeGroupSymmetry()
{
	gint i;
	gchar** symbols = NULL;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gdouble* Z = NULL;
	gint numberOfAtoms = mpqcMolecule.numberOfAtoms;
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
		symbols[i] = g_strdup(mpqcMolecule.listOfAtoms[i].symbol);
		X[i] = mpqcMolecule.listOfAtoms[i].position[0];
		Y[i] = mpqcMolecule.listOfAtoms[i].position[1];
		Z[i] = mpqcMolecule.listOfAtoms[i].position[2];
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
	return g_strdup(abelianPointGroupSymbol);
}
/************************************************************************************************************/
static gboolean setMPQCMoleculeFromGeomXYZ()
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

	mpqcMolecule.totalNumberOfElectrons = 0;
	for(i=0; i<numberOfAtoms; i++)
	{
		SAtomsProp prop = prop_atom_get(GeomXYZ[i].Symb);

		symbols[i] = g_strdup(GeomXYZ[i].Symb);
		setXYZFromGeomXYZ(i, &X[i] , &Y[i] , &Z[i]);
		mpqcMolecule.totalNumberOfElectrons += prop.atomicNumber;
	}
	mpqcMolecule.numberOfValenceElectrons = mpqcMolecule.totalNumberOfElectrons;
	setMPQCMoleculeFromSXYZ(numberOfAtoms, symbols, X, Y, Z);
	mpqcMolecule.groupSymmetry = computeGroupSymmetry();

	for (i=0;i<(gint)NcentersXYZ;i++) g_free( symbols[i]);
	g_free( symbols);
	g_free(X);
	g_free(Y);
	g_free(Z);
	return TRUE;
}
/************************************************************************************************************/
static gboolean setMPQCMoleculeFromGeomZMatrix()
{
	conversion_zmat_to_xyz();
	return setMPQCMoleculeFromGeomXYZ();
}
/************************************************************************************************************/
gboolean setMPQCMolecule()
{
	freeMPQCMolecule();
	if(setMPQCMoleculeFromGeomXYZ()) return TRUE;
	if(setMPQCMoleculeFromGeomZMatrix()) return TRUE;
	return FALSE;
}
/************************************************************************************************************/
void setMPQCGeometryFromInputFile(gchar* fileName)
{
	read_XYZ_from_mpqc_input_file(fileName);
	setMPQCMolecule();
}
/**************************************************************************************************************************************/
static void putMPQCMoleculeInTextEditor()
{
        gchar buffer[BSIZE];
	MPQCAtom* atomList = NULL;
	gint i;

	if(mpqcMolecule.numberOfAtoms<1) return;

	atomList = mpqcMolecule.listOfAtoms;
	sprintf(buffer,"\tsymmetry = %s\n",mpqcMolecule.groupSymmetry);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	sprintf(buffer,"\tunit = angstrom\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	sprintf(buffer,"\t{ atoms geometry } = {\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
      	for (i=0;i<mpqcMolecule.numberOfAtoms;i++)
	{
		sprintf(buffer,"\t\t%s [ %f %f %f ] \n",atomList[i].symbol, 
				atomList[i].position[0], atomList[i].position[1], atomList[i].position[2]);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	}
	sprintf(buffer,"\t}\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
}
/************************************************************************************************************/
static void putBeginGeometryInTextEditor()
{
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, "molecule<Molecule>:(\n",-1);
}
/************************************************************************************************************/
static void putTitleGeometryInTextEditor()
{
        gchar buffer[BSIZE];

	sprintf(buffer,"%c----------------------------------------------------------------\n",'%');
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	sprintf(buffer,"%c Molecule specification\n",'%');
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
}
/************************************************************************************************************/
static void putEndGeometryInTextEditor()
{
        gchar buffer[BSIZE];
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, ")\n",-1);
	sprintf(buffer,"%c----------------------------------------------------------------\n",'%');
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
}
/************************************************************************************************************/
void putMPQCGeometryInfoInTextEditor()
{
	putTitleGeometryInTextEditor();
	putBeginGeometryInTextEditor();
	putMPQCMoleculeInTextEditor();
	putEndGeometryInTextEditor();
}
/**************************************************************************************************************************************/
static void activateRadioButton(GtkWidget *button, gpointer data)
{
	gint* type = NULL;
	GtkWidget* label = NULL;
	GtkWidget* comboSymmetry = NULL;
	gchar buffer[BSIZE];
	 
	if(!GTK_IS_WIDGET(button)) return;

	type  = g_object_get_data(G_OBJECT (button), "Type");
	label = g_object_get_data(G_OBJECT (button), "Label");
	comboSymmetry = g_object_get_data(G_OBJECT (button), "ComboSymmetry");
	if(type)
	{
		setMPQCMolecule();

		if( GTK_TOGGLE_BUTTON (button)->active && *type == GABFIXED)
		{
			if(mpqcMolecule.groupSymmetry) g_free(mpqcMolecule.groupSymmetry);
			mpqcMolecule.groupSymmetry = g_strdup("C1");
			symReduction = FALSE;
			if(GTK_IS_WIDGET(comboSymmetry))
				gtk_widget_set_sensitive(comboSymmetry, TRUE);
		}
		if(GTK_TOGGLE_BUTTON (button)->active &&  *type == AUTO)
		{
			if(mpqcMolecule.groupSymmetry) g_free(mpqcMolecule.groupSymmetry);
			mpqcMolecule.groupSymmetry = g_strdup("auto");
			symReduction = FALSE;
			if(GTK_IS_WIDGET(comboSymmetry))
				gtk_widget_set_sensitive(comboSymmetry, FALSE);
		}
		if(GTK_TOGGLE_BUTTON (button)->active &&  *type == GABEDIT)
		{
			/* groupSymmetry define in setMPQCMolecule */
			symReduction = TRUE;
			if(GTK_IS_WIDGET(comboSymmetry))
				gtk_widget_set_sensitive(comboSymmetry, FALSE);
		}

		if(label) gtk_label_set_text(GTK_LABEL(label)," ");
		if(GTK_IS_WIDGET(buttonTolerance) && symReduction ) gtk_widget_set_sensitive(buttonTolerance, TRUE);
		if(GTK_IS_WIDGET(buttonTolerance) && !symReduction ) gtk_widget_set_sensitive(buttonTolerance, FALSE);

		if(GTK_TOGGLE_BUTTON (button)->active && label && symReduction)
		{
			sprintf(buffer,"%s group",mpqcMolecule.groupSymmetry);
			gtk_label_set_text(GTK_LABEL(label),buffer);
		}
	}
}
/************************************************************************************************************/
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
                  3,3);

	g_object_set_data(G_OBJECT (newButton), "Label",NULL);
	g_object_set_data(G_OBJECT (newButton), "Type",NULL);
	return newButton;
}
/**************************************************************************************************************************************/
static void resetTolerance(GtkWidget *win)
{
	gchar buffer[BSIZE];

	setMPQCMolecule();
	if(labelSymmetry)
	{
		sprintf(buffer,"%s group",mpqcMolecule.groupSymmetry);
		gtk_label_set_text(GTK_LABEL(labelSymmetry),buffer);
	}
}
/**************************************************************************************************************************************/
static void activateToleranceButton(GtkWidget *button, gpointer data)
{
	if(!GTK_IS_WIDGET(button)) return;
	createToleranceWindow(mpqcWin, resetTolerance);
}
/********************************************************************************/
static void setComboSymmetry(GtkWidget *comboSymmetry)
{
	GList *glist = NULL;

  	glist = g_list_append(glist,"C1");
  	glist = g_list_append(glist,"Cs");
  	glist = g_list_append(glist,"C2");
  	glist = g_list_append(glist,"D2");
  	glist = g_list_append(glist,"C2v");
  	glist = g_list_append(glist,"Ci");
  	glist = g_list_append(glist,"C2h");
  	glist = g_list_append(glist,"D2h");

  	gtk_combo_box_entry_set_popdown_strings( comboSymmetry, glist) ;

  	g_list_free(glist);
}
/**********************************************************************/
static void changedEntrySymmetry(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;
	if(mpqcMolecule.groupSymmetry) g_free(mpqcMolecule.groupSymmetry);
	mpqcMolecule.groupSymmetry = g_strdup(entryText);

	if(strstr(entryText,"C1")) symReduction = FALSE;
	else symReduction = FALSE;
}
/************************************************************************************************************/
void createMPQCSymmetryFrame(GtkWidget *win, GtkWidget *box)
{
	GtkWidget* button;
	GtkWidget* buttonAuto;
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* comboSymmetry = NULL;
	GtkWidget* entrySymmetry = NULL;
	GtkWidget* label = gtk_label_new(" ");
	GtkWidget *table = gtk_table_new(3,3,FALSE);
	static TypeOfSymmetryButton typeOfSymmetry[] = { AUTO,GABEDIT, GABFIXED};
	gchar* list[] = {"C1"};

	frame = gtk_frame_new (_("Symmetry"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	comboSymmetry = create_combo_box_entry(list, 1, TRUE, -1, -1);
	entrySymmetry = GTK_BIN(comboSymmetry)->child;
	gtk_widget_set_sensitive(entrySymmetry, FALSE);


	if(mpqcMolecule.groupSymmetry) g_free(mpqcMolecule.groupSymmetry);
	mpqcMolecule.groupSymmetry = g_strdup("auto");
	button = addRadioButtonToATable(table, NULL, _("Auto detection by MPQC"), 0, 0, 2);
	g_object_set_data(G_OBJECT (button), "Label",label);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfSymmetry[AUTO]);
	g_object_set_data(G_OBJECT (button), "ComboSymmetry",comboSymmetry);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
	buttonAuto = button;

	button = addRadioButtonToATable(table, button, _("Detected by Gabedit"), 1, 0, 1);
	g_object_set_data(G_OBJECT (button), "Label",label);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfSymmetry[GABEDIT]);
	g_object_set_data(G_OBJECT (button), "ComboSymmetry",comboSymmetry);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
	add_widget_table(table, label, 1, 1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);

	buttonTolerance = create_button(win,_("Tolerance"));
	add_widget_table(table, buttonTolerance, 1, 2);
	g_signal_connect(G_OBJECT(buttonTolerance),"clicked", G_CALLBACK(activateToleranceButton),NULL);

	labelSymmetry = label;
	gtk_widget_set_sensitive(buttonTolerance, FALSE);

	button = addRadioButtonToATable(table, button, _("Fixed Symmetry"), 2, 0,1);
	g_signal_connect(G_OBJECT(entrySymmetry),"changed", G_CALLBACK(changedEntrySymmetry),NULL);
	setComboSymmetry(comboSymmetry);
	gtk_table_attach(GTK_TABLE(table),comboSymmetry,1,1+2,2,2+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);
	g_object_set_data(G_OBJECT (button), "Label",label);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfSymmetry[GABFIXED]);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
	g_object_set_data(G_OBJECT (button), "ComboSymmetry",comboSymmetry);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
	gtk_widget_set_sensitive(comboSymmetry, FALSE);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonAuto), FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonAuto), TRUE);
}
