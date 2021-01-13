/* MolSymInterface.c */
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
#include "../Utils/AtomsProp.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Common/Windows.h"
#include "../Symmetry/MoleculeSymmetry.h"
#include "../Symmetry/SymmetryGabedit.h"

/* 0-> Position ; 1-> Principal axis */
static gint typeOfTolerance[] = { 0,1};
/* -1 = Default, tolerance = (min distance between atoms) /50 */
static gdouble tolerancePosition[] = { -1.0, 0.3, 0.1, 0.03, 0.003, 0.0};
static gdouble tolerancePrincipalAxis[] = { 5e-3, 0.3, 0.1, 0.03, 0.003, 0.0};

static gdouble tolerancePositionValue = -1.0;
static gdouble tolerancePrincipalAxisValue = 5e-3;

static gdouble tmpTolerancePositionValue = -1.0;
static gdouble tmpTolerancePrincipalAxisValue = 5e-3;

/****************************************************************************************************************************/
static void setToleranceParametersFromTmp(GtkWidget* TextWid, gchar* groupSymbol)
{
	tolerancePositionValue = tmpTolerancePositionValue;
	tolerancePrincipalAxisValue = tmpTolerancePrincipalAxisValue;
}
/****************************************************************************************************************************/
static void putInfoInTextWidget(GtkWidget* TextWid, 
		gchar* groupSymbol,
		gdouble principalAxisTolerance, gdouble eps, gchar* message)
{
	gchar  t[BSIZE];

 	gabedit_text_insert (GABEDIT_TEXT(TextWid), NULL, NULL, NULL,message,-1);   

	sprintf(t,_("Tolerance for principal axis classification : %0.5f\n"),principalAxisTolerance);
 	gabedit_text_insert (GABEDIT_TEXT(TextWid), NULL, NULL, NULL,t,-1);   

	sprintf(t,_("Precision for atom position : %0.5f\n\n"),eps);
 	gabedit_text_insert (GABEDIT_TEXT(TextWid), NULL, NULL, NULL,t,-1);   

	sprintf(t,_("Group Symmetry : %s\n"),groupSymbol);
 	gabedit_text_insert (GABEDIT_TEXT(TextWid), NULL, NULL, NULL,t,-1);   
	sprintf(t,"-----------------------------------------------------------------------------------------\n\n");
 	gabedit_text_insert (GABEDIT_TEXT(TextWid), NULL, NULL, NULL,t,-1);   
}
/****************************************************************************************************/
static void putInfoAbelianGroup(GtkWidget* TextWid, 
		gchar* groupName,
		gint nGenerators, 
		gchar** generators, 
		gint nMolcas, 
		gchar** molcasGenerators, 
		gint nElements, 
		gchar** elements) 
{
	gchar  t[BSIZE];
	gint i;

	sprintf(t,_("Abelian Group Name : %s\n\n"),groupName);
 	gabedit_text_insert (GABEDIT_TEXT(TextWid), NULL, NULL, NULL,t,-1);   

	sprintf(t,_("Generators : "));
	for(i=0;i<nGenerators;i++) sprintf(t + strlen(t)," %s, ", generators[i]);
	strcat(t,"\n");
 	gabedit_text_insert (GABEDIT_TEXT(TextWid), NULL, NULL, NULL,t,-1);   

	sprintf(t,"Molcas     : ");
	for(i=0;i<nMolcas;i++) sprintf(t + strlen(t)," %s, ", molcasGenerators[i]);
	strcat(t,"\n");
 	gabedit_text_insert (GABEDIT_TEXT(TextWid), NULL, NULL, NULL,t,-1);   

	sprintf(t,_("Elements   : "));
	for(i=0;i<nElements;i++) sprintf(t + strlen(t)," %s, ", elements[i]);
	strcat(t,"\n");
 	gabedit_text_insert (GABEDIT_TEXT(TextWid), NULL, NULL, NULL,t,-1);   
	sprintf(t,"-----------------------------------------------------------------------------------------\n\n");
 	gabedit_text_insert (GABEDIT_TEXT(TextWid), NULL, NULL, NULL,t,-1);   
}
/********************************************************************************/
static void putGeometryInTextWidget(GtkWidget* TextWid, gint numberOfAtoms, gchar** symbols, gdouble* X, gdouble* Y, gdouble* Z)
{
	gchar  t[BSIZE];
	gint n;
	GdkColor blue;
	GdkColor green;

	blue.red = 0;
	blue.green = 65535;
	blue.blue = 65535;

	green.red = 0;
	green.green = 50000;
	green.blue = 0;


	sprintf(t,_("%d is the number of atoms\n"),numberOfAtoms);
 	gabedit_text_insert (GABEDIT_TEXT(TextWid), NULL, NULL, NULL,t,-1);   
	sprintf(t,_(" Positions are in Ang\n"));
 	gabedit_text_insert (GABEDIT_TEXT(TextWid), NULL, NULL, NULL,t,-1);   

	for(n=0;n<numberOfAtoms;n++)
	{
		sprintf(t,"%5s %20.8f %20.8f %20.8f\n",
				symbols[n],
				X[n],
				Y[n],
				Z[n]
				);
		if(n%2==0)
 			gabedit_text_insert (GABEDIT_TEXT(TextWid), NULL, NULL, &blue,t,-1);   
		else
 			gabedit_text_insert (GABEDIT_TEXT(TextWid), NULL, NULL, &green,t,-1);   
	}
	sprintf(t,"================================================================\n\n");
 	gabedit_text_insert (GABEDIT_TEXT(TextWid), NULL, NULL, NULL,t,-1);   
}
/****************************************************************************************************/
void createGeometrySymmetryWindow(gint numberOfAtoms, 
		gchar** symbols, gdouble* X, gdouble* Y, gdouble* Z,
	       	gchar* groupSymbol)
{
	 GtkWidget *Dialogue = NULL;
	 GtkWidget *Bouton;
	 GtkWidget *frame;
	 GtkWidget *TextWid;
	 gchar * title = NULL;
	 gchar message[BSIZE];
	 gint err;
	 gchar** symbolstmp;
	 gdouble* Xtmp;
	 gdouble* Ytmp;
	 gdouble* Ztmp;
	 gint ntmp = numberOfAtoms;
	 gint i;
	 gint maximalOrder = 20;
	 
	 gdouble principalAxisTolerance = tolerancePrincipalAxisValue;
	 gdouble eps = tolerancePositionValue;
	 
	 if(numberOfAtoms<1)
	 {
		 Message(_("Sorry Number of atoms is not positive"),_("Error"),TRUE);
		 return;
	 }
	symbolstmp = (gchar**)g_malloc(sizeof(gchar*)*(ntmp));
	if(symbols == NULL) return;
	Xtmp = (gdouble*)g_malloc(sizeof(gdouble)*(ntmp));
	if(Xtmp == NULL) return;
	Ytmp = (gdouble*)g_malloc(sizeof(gdouble)*(ntmp));
	if(Ytmp == NULL) return;
	Ztmp = (gdouble*)g_malloc(sizeof(gdouble)*(ntmp));
	if(Ztmp == NULL) return;

	for (i=0;i<ntmp;i++)
	{
		symbolstmp[i] = g_strdup(symbols[i]);
		Xtmp[i] = X[i];
		Ytmp[i] = Y[i];
		Ztmp[i] = Z[i];
	}

	sprintf(groupSymbol,"NO");
	err = computeSymmetryOld(principalAxisTolerance, FALSE, groupSymbol,maximalOrder, TRUE, &numberOfAtoms,symbols, X, Y, Z, &eps, message);
	if(err != 0)
	{
		Message(message,"Error",TRUE);
	}
	 Dialogue = gtk_dialog_new();
	 gtk_widget_realize(GTK_WIDGET(Dialogue));
	 title = g_strdup(_("Group symmetry & Geometry with standard orientation"));
			
	 gtk_window_set_title(GTK_WINDOW(Dialogue),title);

	 gtk_window_set_modal (GTK_WINDOW (Dialogue), FALSE);
	gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);

	g_signal_connect(G_OBJECT(Dialogue), "delete_event", (GCallback)destroy_button_windows, NULL);
	g_signal_connect(G_OBJECT(Dialogue), "delete_event", (GCallback)gtk_widget_destroy, NULL);

	TextWid = create_text_widget(GTK_WIDGET(GTK_DIALOG(Dialogue)->vbox),NULL,&frame);
	gabedit_text_set_editable(GABEDIT_TEXT(TextWid), TRUE);

	gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Dialogue)->action_area), FALSE);
	
	Bouton = create_button(Dialogue,"OK");
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dialogue)->action_area), Bouton, FALSE, TRUE, 5);	
	GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Bouton);
	g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)destroy_button_windows, GTK_OBJECT(Dialogue));
	g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(Dialogue));

	add_button_windows(title,Dialogue);

	if(strcmp(groupSymbol,"C1")!=0)
	{
		sprintf(message , _("Group & Geometry with reduction of molecule to its basis set of atoms\n"));
		strcat(message,"*************************************************************************\n\n");
	}
	else
	{
		sprintf(message , _("Group & Geometry\n"));
		strcat(message,"***********************\n\n");
	}

	putInfoInTextWidget(TextWid, groupSymbol,  principalAxisTolerance, eps, message);
	putGeometryInTextWidget(TextWid,numberOfAtoms, symbols, X, Y, Z);

	if(strcmp(groupSymbol,"C1")!=0)
	{
		numberOfAtoms = ntmp;
		sprintf(groupSymbol,_("NO"));
		err = computeSymmetryOld(principalAxisTolerance, FALSE, groupSymbol,maximalOrder, FALSE, &numberOfAtoms,symbolstmp, Xtmp, Ytmp, Ztmp, &eps, message);
		sprintf(message,  _("Group & Geometry\n"));
		strcat(message,"***********************\n\n");
		putInfoInTextWidget(TextWid, groupSymbol,  principalAxisTolerance, eps, message);
		putGeometryInTextWidget(TextWid,numberOfAtoms, symbolstmp, Xtmp, Ytmp, Ztmp);
	}

	gtk_window_set_default_size (GTK_WINDOW(Dialogue), ScreenWidth/3, ScreenHeight/3);
	 gtk_widget_show_all(Dialogue);
	 g_free(title);


	{
		numberOfAtoms = ntmp;
		gdouble* mass = g_malloc(numberOfAtoms*sizeof(gdouble));
		gint i = 0;
		for(i=0;i<numberOfAtoms;i++) { SAtomsProp prop = prop_atom_get(symbolstmp[i]); mass[i] =  prop.masse;}
		SMolecule mol = newSMolecule();
		mol.setMolecule(&mol,  numberOfAtoms, symbolstmp, mass, Xtmp, Ytmp, Ztmp);
		Symmetry sym = newSymmetry(&mol, eps);
		sym.findAllPointGroups(&sym);
       		sym.printElementResults(&sym);
		g_free(mass);
		sym.getUniqueMolecule(&sym);
		sym.getSymmetrizeMolecule(&sym);
	}
	for (i=0;i<ntmp;i++)
		g_free(symbolstmp[i]);
	g_free( symbolstmp);
	g_free(Xtmp);
	g_free(Ytmp);
	g_free(Ztmp);
}
/****************************************************************************************************/
GtkWidget* createGeometryAbelianGroupWindow(gint numberOfAtoms, 
		gchar** symbols, gdouble* X, gdouble* Y, gdouble* Z, 
		gchar* pointGroupSymbol, gchar* abelianPointGroupSymbol)
{
	GtkWidget *Dialogue = NULL;
	GtkWidget *Bouton;
	GtkWidget *frame;
	GtkWidget *TextWid;
	gchar * title = NULL;
	gchar message[BSIZE];
	gint err;
	gint i;
	gint maximalOrder = 20;
	gint nGenerators;
	gint nMolcas;
	gint nElements;
	gchar* generators[3];
	gchar* molcasGenerators[3];
	gchar* elements[8];

	 gdouble principalAxisTolerance = tolerancePrincipalAxisValue;
	 gdouble eps = tolerancePositionValue;
	 
	 if(numberOfAtoms<1)
	 {
		 Message(_("Sorry Number of atoms is not positive"),_("Error"),TRUE);
		 return NULL;
	 }
	for(i=0;i<3;i++)
	{
		generators[i] = g_malloc(100*sizeof(gchar));
		molcasGenerators[i] = g_malloc(100*sizeof(gchar));
	}
	for(i=0;i<8;i++)
		elements[i] = g_malloc(100*sizeof(gchar));

	err = computeAbelianGroup(principalAxisTolerance, pointGroupSymbol, abelianPointGroupSymbol,
		maximalOrder, TRUE,
		&numberOfAtoms, 
		symbols, X, Y, Z, 
		&nGenerators, generators,
		&nMolcas, molcasGenerators,
		&nElements, elements,
		&eps, message);
	if(err != 0)
	{
		Message(message,_("Error"),TRUE);
		return NULL;
	}
	 Dialogue = gtk_dialog_new();
	 gtk_widget_realize(GTK_WIDGET(Dialogue));
	 title = g_strdup(_("Point group, abelian point group & Geometry with reduction (using abelian group)"));
			
	 gtk_window_set_title(GTK_WINDOW(Dialogue),title);

	 gtk_window_set_modal (GTK_WINDOW (Dialogue), FALSE);
	gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);

	g_signal_connect(G_OBJECT(Dialogue), "delete_event", (GCallback)destroy_button_windows, NULL);
	g_signal_connect(G_OBJECT(Dialogue), "delete_event", (GCallback)gtk_widget_destroy, NULL);

	TextWid = create_text_widget(GTK_WIDGET(GTK_DIALOG(Dialogue)->vbox),NULL,&frame);
	gabedit_text_set_editable(GABEDIT_TEXT(TextWid), TRUE);

	gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Dialogue)->action_area), FALSE);
	
	Bouton = create_button(Dialogue,"OK");
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dialogue)->action_area), Bouton, FALSE, TRUE, 5);	
	GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Bouton);
	g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)destroy_button_windows, GTK_OBJECT(Dialogue));
	g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(Dialogue));

	add_button_windows(title,Dialogue);

	if(strcmp(pointGroupSymbol,"C1")!=0)
	{
		sprintf(message,  _("Group & Geometry with reduction of molecule to its basis set of atoms\n"));
		strcat(message,"**************************************************************************\n\n");
	}
	else
	{
		sprintf(message,  _("Group & Geometry\n"));
		strcat(message,"************************\n\n");
	}

	putInfoInTextWidget(TextWid, pointGroupSymbol,  principalAxisTolerance, eps, message);
	putInfoAbelianGroup(TextWid, abelianPointGroupSymbol, nGenerators, generators, nMolcas, molcasGenerators, nElements, elements);
	putGeometryInTextWidget(TextWid,numberOfAtoms, symbols, X, Y, Z);

	gtk_window_set_default_size (GTK_WINDOW(Dialogue), ScreenWidth/3, ScreenHeight/3);
	 gtk_widget_show_all(Dialogue);
	 g_free(title);

	for(i=0;i<3;i++)
	{
		g_free(generators[i]);
		g_free(molcasGenerators[i]);
	}
	for(i=0;i<8;i++)
		g_free(elements[i]);
	return Dialogue;
}
/**************************************************************************************************************************************/
static void activateEntry(GtkWidget *entry, gpointer data)
{
	GtkWidget* button = ( GtkWidget*)data;
	gdouble value = -1;
	gint* type = NULL;
	G_CONST_RETURN gchar* entryText = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;
	if(!GTK_IS_WIDGET(button)) return;

	entry = g_object_get_data(G_OBJECT (button), "Entry");
	type = g_object_get_data(G_OBJECT (button), "Type");
	
	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	value = atof(entryText);
	if(value<0)
	{
		gchar newValue[BSIZE];
		value = -value;
		sprintf(newValue,"%f",value);
		gtk_entry_set_text(GTK_ENTRY(entry),newValue);
	}
	if(fabs(value)<1e-10)
	{
		value = 0.01;
		/* gtk_entry_set_text(GTK_ENTRY(entry),"0.01");*/
	}

	if(type && *type == typeOfTolerance[0])
		tmpTolerancePositionValue = value;
	else
		tmpTolerancePrincipalAxisValue = value;
}
/**************************************************************************************************************************************/
static void activateRadioButton(GtkWidget *button, gpointer data)
{
	GtkWidget* entry = NULL;
	gdouble* value = NULL;
	gint* type = NULL;
	 
	if(!GTK_IS_WIDGET(button)) return;

	entry = g_object_get_data(G_OBJECT (button), "Entry");
	type = g_object_get_data(G_OBJECT (button), "Type");
	value = g_object_get_data(G_OBJECT (button), "Value");
	if(entry)
	{
		if(value && fabs(*value)<1e-10)
			gtk_widget_set_sensitive(entry, TRUE);
		else
			gtk_widget_set_sensitive(entry, FALSE);
	}

	if(value && fabs(*value)>1e-10)
	{
		if(type && *type == typeOfTolerance[0])
			tmpTolerancePositionValue = *value;
		else
			tmpTolerancePrincipalAxisValue = *value;
	}
	else
	{
		if(type && *type == typeOfTolerance[0])
		{
			tmpTolerancePositionValue = 0.01;
		}
		else
		{
			tmpTolerancePrincipalAxisValue = 0.01;
		}

		gtk_entry_set_text(GTK_ENTRY(entry),"0.01");
	}
}
/**************************************************************************************************************************************/
static GtkWidget* addRadioButtonToATable(GtkWidget* table, GtkWidget* friendButton, gchar* label, gint i, gint j, gint k)
{
	GtkWidget *newButton;

	if(friendButton)
		newButton = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (friendButton)), label);
	else
		newButton = gtk_radio_button_new_with_label( NULL, label);

	gtk_table_attach(GTK_TABLE(table),newButton,j,j+k,i,i+1,
			/*
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
		  */
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);

	g_object_set_data(G_OBJECT (newButton), "Entry",NULL);
	g_object_set_data(G_OBJECT (newButton), "Type",NULL);
	g_object_set_data(G_OBJECT (newButton), "Value",NULL);
	return newButton;
}
/**************************************************************************************************************************************/
static void createTolerancePrincipalAxisFrame(GtkWidget *box)
{
	GtkWidget* button;
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* entry = gtk_entry_new();
	GtkWidget *table = gtk_table_new(6,2,TRUE);

	frame = gtk_frame_new (_("Tolerance for principal axis"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	button = addRadioButtonToATable(table, NULL, "Default [5e-3]            ", 0, 0,2);
	g_object_set_data(G_OBJECT (button), "Entry",entry);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfTolerance[1]);
	g_object_set_data(G_OBJECT (button), "Value",&tolerancePrincipalAxis[0]);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
	button = addRadioButtonToATable(table, button, "Very Coarser [0.3]", 1, 0, 2);
	g_object_set_data(G_OBJECT (button), "Entry",entry);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfTolerance[1]);
	g_object_set_data(G_OBJECT (button), "Value",&tolerancePrincipalAxis[1]);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
	button = addRadioButtonToATable(table, button, "Coarser [0.1]", 2, 0, 2);
	g_object_set_data(G_OBJECT (button), "Entry",entry);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfTolerance[1]);
	g_object_set_data(G_OBJECT (button), "Value",&tolerancePrincipalAxis[2]);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
	button = addRadioButtonToATable(table, button, "Medium [0.03]", 3, 0, 2);
	g_object_set_data(G_OBJECT (button), "Entry",entry);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfTolerance[1]);
	g_object_set_data(G_OBJECT (button), "Value",&tolerancePrincipalAxis[3]);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
	button = addRadioButtonToATable(table, button, "Fine [0.003]", 4, 0, 2);
	g_object_set_data(G_OBJECT (button), "Entry",entry);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfTolerance[1]);
	g_object_set_data(G_OBJECT (button), "Value",&tolerancePrincipalAxis[4]);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
	button = addRadioButtonToATable(table, button, "Other", 5, 0, 1);
	g_object_set_data(G_OBJECT (button), "Entry",entry);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfTolerance[1]);
	g_object_set_data(G_OBJECT (button), "Value",&tolerancePrincipalAxis[5]);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
	gtk_widget_set_size_request(GTK_WIDGET(entry),50,-1);
	add_widget_table(table, entry,5,1);
	gtk_widget_set_sensitive(entry, FALSE);
	gtk_entry_set_text(GTK_ENTRY(entry),"0.01");
	g_signal_connect(G_OBJECT(entry),"changed", G_CALLBACK(activateEntry),button);
	tmpTolerancePrincipalAxisValue = 5e-3;
}
/**************************************************************************************************************************************/
static void createTolerancePositionFrame(GtkWidget *box)
{
	GtkWidget* button;
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* entry =  gtk_entry_new();
	GtkWidget *table = gtk_table_new(6,2,TRUE);

	frame = gtk_frame_new (_("Tolerance for atom positions"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	button = addRadioButtonToATable(table, NULL, "Default [(min distance)/50]", 0, 0,2);
	g_object_set_data(G_OBJECT (button), "Entry",entry);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfTolerance[0]);
	g_object_set_data(G_OBJECT (button), "Value",&tolerancePosition[0]);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
	button = addRadioButtonToATable(table, button, "Very Coarser [0.3]", 1, 0, 2);
	g_object_set_data(G_OBJECT (button), "Entry",entry);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfTolerance[0]);
	g_object_set_data(G_OBJECT (button), "Value",&tolerancePosition[1]);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
	button = addRadioButtonToATable(table, button, "Coarser [0.1]", 2, 0, 2);
	g_object_set_data(G_OBJECT (button), "Entry",entry);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfTolerance[0]);
	g_object_set_data(G_OBJECT (button), "Value",&tolerancePosition[2]);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
	button = addRadioButtonToATable(table, button, "Medium [0.03]", 3, 0, 2);
	g_object_set_data(G_OBJECT (button), "Entry",entry);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfTolerance[0]);
	g_object_set_data(G_OBJECT (button), "Value",&tolerancePosition[3]);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
	button = addRadioButtonToATable(table, button, "Fine [0.003]", 4, 0, 2);
	g_object_set_data(G_OBJECT (button), "Entry",entry);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfTolerance[0]);
	g_object_set_data(G_OBJECT (button), "Value",&tolerancePosition[4]);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
	button = addRadioButtonToATable(table, button, "Other", 5, 0, 1);
	g_object_set_data(G_OBJECT (button), "Entry",entry);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfTolerance[0]);
	g_object_set_data(G_OBJECT (button), "Value",&tolerancePosition[5]);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
	gtk_widget_set_size_request(GTK_WIDGET(entry),50,-1);
	add_widget_table(table, entry,5,1);
	gtk_widget_set_sensitive(entry, FALSE);
	gtk_entry_set_text(GTK_ENTRY(entry),"0.01");
	g_signal_connect(G_OBJECT(entry),"changed", G_CALLBACK(activateEntry),button);
	tmpTolerancePositionValue = -1.0;
}
/****************************************************************************************************/
void createToleranceWindow(GtkWidget* win, GabeditSignalFunc myFunc)
{
	GtkWidget *dialogWindow = NULL;
	GtkWidget *button;
	GtkWidget *frame;
	GtkWidget *hbox;
	gchar title[BSIZE];
	 
	dialogWindow = gtk_dialog_new();
	gtk_widget_realize(GTK_WIDGET(dialogWindow));
	sprintf(title, _("Tolerance for computing the point group"));
			
	gtk_window_set_title(GTK_WINDOW(dialogWindow),title);

	gtk_window_set_modal (GTK_WINDOW (dialogWindow), TRUE);
	gtk_window_set_position(GTK_WINDOW(dialogWindow),GTK_WIN_POS_CENTER);

	g_signal_connect(G_OBJECT(dialogWindow), "delete_event", (GCallback)destroy_button_windows, NULL);
	g_signal_connect(G_OBJECT(dialogWindow), "delete_event", (GCallback)gtk_widget_destroy, NULL);

	frame = gtk_frame_new (NULL);
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (GTK_WIDGET(GTK_DIALOG(dialogWindow)->vbox)), frame, TRUE, TRUE, 3);

	hbox = gtk_hbox_new (FALSE, 3);
	gtk_widget_show (hbox);
	gtk_container_add (GTK_CONTAINER (frame), hbox);

	createTolerancePositionFrame(hbox);
	createTolerancePrincipalAxisFrame(hbox);

	gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(dialogWindow)->action_area), TRUE);

	button = create_button(dialogWindow,"Cancel");
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(dialogWindow)->action_area), button, FALSE, TRUE, 5);	
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)destroy_button_windows, GTK_OBJECT(dialogWindow));
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(dialogWindow));

	button = create_button(dialogWindow,"OK");
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(dialogWindow)->action_area), button, FALSE, TRUE, 5);	
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)setToleranceParametersFromTmp, GTK_OBJECT(dialogWindow));
	if(myFunc) g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)myFunc, GTK_OBJECT(dialogWindow));
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)destroy_button_windows, GTK_OBJECT(dialogWindow));
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(dialogWindow));
	
	add_button_windows(title,dialogWindow);

	gtk_widget_show_all(dialogWindow);
	if(GTK_IS_WIDGET(win))
		gtk_window_set_transient_for(GTK_WINDOW(dialogWindow),GTK_WINDOW(win));
}
/****************************************************************************************************************************/
void getToleranceValues(gdouble* tolerancePosition , gdouble* tolerancePrincipalAxis)
{
	*tolerancePosition = tolerancePositionValue;
	*tolerancePrincipalAxis = tolerancePrincipalAxisValue;
}
/****************************************************************************************************************************/
gdouble getTolerancePosition()
{
	return tolerancePositionValue;
}
/****************************************************************************************************************************/
gdouble getTolerancePrincipalAxis()
{
	return tolerancePrincipalAxisValue;
}
/****************************************************************************************************************************/
void buildStandardOrientationDlg(gint numberOfAtoms, gchar** symbols, gdouble* X, gdouble* Y, gdouble* Z)
{
	buildStandardOrientation(numberOfAtoms, symbols, X, Y, Z);
}
/****************************************************************************************************/
void createGeometrySymmetrizationWindow(gint numberOfAtoms, gchar** symbols, gdouble* X, gdouble* Y, gdouble* Z, gchar* groupSymbol)
{
	 GtkWidget *Dialogue = NULL;
	 GtkWidget *Bouton;
	 GtkWidget *frame;
	 GtkWidget *TextWid;
	 gchar * title = NULL;
	 gchar message[BSIZE];
	 gint err;
	 gint maximalOrder = 20;
	 
	 gdouble principalAxisTolerance = tolerancePrincipalAxisValue;
	 gdouble eps = tolerancePositionValue;
	 if(eps<0) eps = 0.1; 
	 
	 if(numberOfAtoms<1)
	 {
		 Message(_("Sorry Number of atoms is not positive"),_("Error"),TRUE);
		 return;
	 }
	sprintf(groupSymbol,"NO");
	err = computeSymmetrization(principalAxisTolerance, groupSymbol, maximalOrder, &numberOfAtoms, symbols, X, Y, Z,  &eps, message);
	if(err != 0)
	{
		Message(message,"Error",TRUE);
		return;
	}
	Dialogue = gtk_dialog_new();
	gtk_widget_realize(GTK_WIDGET(Dialogue));
	title = g_strdup(_("Group symmetry & Geometry"));
	gtk_window_set_title(GTK_WINDOW(Dialogue),title);

	gtk_window_set_modal (GTK_WINDOW (Dialogue), FALSE);
	gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);

	g_signal_connect(G_OBJECT(Dialogue), "delete_event", (GCallback)destroy_button_windows, NULL);
	g_signal_connect(G_OBJECT(Dialogue), "delete_event", (GCallback)gtk_widget_destroy, NULL);

	TextWid = create_text_widget(GTK_WIDGET(GTK_DIALOG(Dialogue)->vbox),NULL,&frame);
	gabedit_text_set_editable(GABEDIT_TEXT(TextWid), TRUE);

	gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Dialogue)->action_area), FALSE);
	
	Bouton = create_button(Dialogue,"OK");
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dialogue)->action_area), Bouton, FALSE, TRUE, 5);	
	GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Bouton);
	g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)destroy_button_windows, GTK_OBJECT(Dialogue));
	g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(Dialogue));

	add_button_windows(title,Dialogue);

	sprintf(message , _("Group & Geometry after symmetrization\n"));
	strcat(message,"*******************************************\n\n");

	putInfoInTextWidget(TextWid, groupSymbol,  principalAxisTolerance, eps, message);
	putGeometryInTextWidget(TextWid,numberOfAtoms, symbols, X, Y, Z);
	gtk_window_set_default_size (GTK_WINDOW(Dialogue), ScreenWidth/3, ScreenHeight/3);
	gtk_widget_show_all(Dialogue);
	g_free(title);

}
