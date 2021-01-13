/* MPQCMole.c */
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
#include "../MPQC/MPQCBasis.h"
#include "../MPQC/MPQCFunctionals.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/GeomConversion.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Common/Windows.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Symmetry/MoleculeSymmetry.h"
#include "../Symmetry/MoleculeSymmetryInterface.h"
/************************************************************************************************************/
void initMPQCMole()
{
	mpqcMole.method = g_strdup("CLHF");
	mpqcMole.functional = g_strdup("B3LYP");
	mpqcMole.molecule = g_strdup("$:molecule");
	mpqcMole.basis = g_strdup("$:basis");
	mpqcMole.auxBasisName = g_strdup("STO-3G");
	mpqcMole.memory = g_strdup("16000000");
	mpqcMole.totalCharge = TotalCharges[0];
	mpqcMole.spinMultiplicity = SpinMultiplicities[0];
	mpqcMole.printNPA = TRUE;
	mpqcMole.printNAO = TRUE;
	mpqcMole.guessWaveFunction = g_strdup("$:guess_wavefunction");
}
/************************************************************************************************************/
void freeMPQCMole()
{
	if(mpqcMole.method ) g_free(mpqcMole.method);
	if(mpqcMole.functional ) g_free(mpqcMole.functional);
	if(mpqcMole.molecule  ) g_free(mpqcMole.molecule );
	if(mpqcMole.basis ) g_free(mpqcMole.basis);
	if(mpqcMole.auxBasisName ) g_free(mpqcMole.auxBasisName);
	if(mpqcMole.memory ) g_free(mpqcMole.memory);
	if(mpqcMole.guessWaveFunction) g_free(mpqcMole.guessWaveFunction);
	mpqcMole.method = NULL;
	mpqcMole.functional = NULL;
	mpqcMole.molecule  = NULL;
	mpqcMole.basis = NULL;
	mpqcMole.memory = NULL;
	mpqcMole.guessWaveFunction = NULL;
}
/************************************************************************************************************/
static void setComboMethod(GtkWidget *comboMethod)
{
	GList *glist = NULL;
	GtkWidget* entrySpinMultiplicity = NULL;
	G_CONST_RETURN gchar* entryText = NULL;
	int multiplicity;

	entrySpinMultiplicity  = g_object_get_data(G_OBJECT (comboMethod), "EntrySpinMultiplicity");
	if(!(GTK_IS_WIDGET(entrySpinMultiplicity))) return;

	 
	entryText = gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity));
	if(strlen(entryText)<1)return;
	multiplicity = atoi(entryText);

	if(multiplicity==1)
	{
  		glist = g_list_append(glist,"CLHF");
  		glist = g_list_append(glist,"HSOSHF");
  		glist = g_list_append(glist,"CLKS");
  		glist = g_list_append(glist,"HSOSKS");
  		glist = g_list_append(glist,"UHF");
  		glist = g_list_append(glist,"UKS");
  		glist = g_list_append(glist,"MBPT2");
  		glist = g_list_append(glist,"MBPT2_R12/A");
  		glist = g_list_append(glist,"MBPT2_R12/A'");
	}
	else
	{
  		glist = g_list_append(glist,"UHF");
  		glist = g_list_append(glist,"HSOSHF");
  		glist = g_list_append(glist,"UKS");
  		glist = g_list_append(glist,"HSOSKS");
  		glist = g_list_append(glist,"ZAPT");
  		glist = g_list_append(glist,"OPT1");
  		glist = g_list_append(glist,"OPT2");
	}
  	gtk_combo_box_entry_set_popdown_strings( comboMethod, glist) ;
}
/************************************************************************************************************/
static void setLabelMethod(GtkWidget* labelMethod, gchar* method)
{
	static GList *glistMethod = NULL;
	static GList *glistLabel = NULL;
	GList *glistM = NULL;
	GList *glistL = NULL;
	if(!glistMethod)
	{
  		glistMethod = g_list_append(glistMethod,"CLHF");
  		glistLabel = g_list_append(glistLabel,"Closed Shell Hartree-Fock");

  		glistMethod = g_list_append(glistMethod,"HSOSHF");
  		glistLabel = g_list_append(glistLabel,"Restricted Open Shell Hartree-Fock");

  		glistMethod = g_list_append(glistMethod,"CLKS");
  		glistLabel = g_list_append(glistLabel,"Closed Shell Kohn-Sham");

  		glistMethod = g_list_append(glistMethod,"HSOSKS");
  		glistLabel = g_list_append(glistLabel,"Restricted Open Shell Kohn-Sham ");

  		glistMethod = g_list_append(glistMethod,"UHF");
  		glistLabel = g_list_append(glistLabel,"Unrestricted Hartree-Fock");

  		glistMethod = g_list_append(glistMethod,"UKS");
  		glistLabel = g_list_append(glistLabel,"Unrestricted Kohn-Sham ");

  		glistMethod = g_list_append(glistMethod,"MBPT2");
  		glistLabel = g_list_append(glistLabel,"MP2 closed shell");

  		glistMethod = g_list_append(glistMethod,"MBPT2_R12/A");
  		glistLabel = g_list_append(glistLabel,"MP2 + R12 correlation closed shell");

  		glistMethod = g_list_append(glistMethod,"MBPT2_R12/A'");
  		glistLabel = g_list_append(glistLabel,"MP2 + R12 correlation closed shell");

  		glistMethod = g_list_append(glistMethod,"ZAPT");
  		glistLabel = g_list_append(glistLabel,"MP2 open shell");

  		glistMethod = g_list_append(glistMethod,"OPT1");
  		glistLabel = g_list_append(glistLabel,"MP2 open shell");

  		glistMethod = g_list_append(glistMethod,"OPT2");
  		glistLabel = g_list_append(glistLabel,"MP2 open shell");
	}
	glistM = glistMethod;
	glistL = glistLabel;
	while(glistM != NULL && glistL!=NULL)
	{
		if(strcmp((gchar*)glistM->data,method)==0)
		{
			gtk_label_set_text(GTK_LABEL(labelMethod),(gchar*)glistL->data);
			break;
		}
		glistM = glistM->next;
		glistL = glistL->next;
	}

}
/************************************************************************************************************/
static void changedEntryMethod(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	GtkWidget* labelMethod = NULL;
	GtkWidget* wid = NULL;
	gboolean KS = FALSE;
	gboolean R12 = FALSE;
	gboolean ZAPT = FALSE;
	gboolean OPT = FALSE;
	GtkWidget* entryFunctional = NULL;
	GtkWidget* buttonSinglePoint = g_object_get_data(G_OBJECT (entry), "ButtonSinglePoint");
	GtkWidget* buttonOptimization = g_object_get_data(G_OBJECT (entry), "ButtonOptimization");
	gboolean OkOptimize = TRUE;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;
	if(strstr(entryText,"KS"))KS = TRUE;
	if(strstr(entryText,"R12"))R12 = TRUE;
	if(strstr(entryText,"ZAPT"))ZAPT = TRUE;
	if(strstr(entryText,"OPT"))OPT = TRUE;
	if(R12 || ZAPT || OPT ) OkOptimize = FALSE;

	if(!OkOptimize)
	{
		if(GTK_IS_WIDGET(buttonSinglePoint))
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonSinglePoint), TRUE);
		if(GTK_IS_WIDGET(buttonOptimization)) gtk_widget_set_sensitive(buttonOptimization,FALSE);
	}
	else
	{
		if(GTK_IS_WIDGET(buttonSinglePoint))
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonSinglePoint), TRUE);
		if(GTK_IS_WIDGET(buttonOptimization)) gtk_widget_set_sensitive(buttonOptimization,TRUE);
	}

	if(mpqcMole.method) g_free(mpqcMole.method);
	mpqcMole.method=g_strdup(entryText);

	labelMethod  = g_object_get_data(G_OBJECT (entry), "LabelMethod");
	if(GTK_IS_WIDGET(labelMethod)) setLabelMethod(labelMethod, mpqcMole.method);

	wid = g_object_get_data(G_OBJECT (entry), "LabelAuxBasis1");
	if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid,R12);
	wid = g_object_get_data(G_OBJECT (entry), "LabelAuxBasis2");
	if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid,R12);
	wid = g_object_get_data(G_OBJECT (entry), "ComboAuxBasis");
	if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid,R12);

	wid = g_object_get_data(G_OBJECT (entry), "LabelFunctional1");
	if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid,KS);
	wid = g_object_get_data(G_OBJECT (entry), "LabelFunctional2");
	if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid,KS);
	wid = g_object_get_data(G_OBJECT (entry), "ComboFunctional");
	if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid,KS);
	if(KS && GTK_IS_COMBO_BOX(wid))
	{
		gboolean sumDen = FALSE;
		entryFunctional=GTK_BIN(wid)->child;
		entryText = gtk_entry_get_text(GTK_ENTRY(entryFunctional));
		if(strstr(entryText,"Your")) sumDen = TRUE;
		wid = g_object_get_data(G_OBJECT (entry), "ButtonWhat");
		if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid,!sumDen);
		wid = g_object_get_data(G_OBJECT (entry), "ButtonSumDenFunctional");
		if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid,sumDen);
	}
	else
	{
		wid = g_object_get_data(G_OBJECT (entry), "ButtonWhat");
		if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid,KS);
		wid = g_object_get_data(G_OBJECT (entry), "ButtonSumDenFunctional");
		if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid,KS);
	}
}
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
	gint ne = mpqcMolecule.numberOfValenceElectrons - mpqcMole.totalCharge;

	if(ne%2==0)
		nlist = ne/2+1;
	else
		nlist = (ne+1)/2;

	if(nlist<1) return;
	list = g_malloc(nlist*sizeof(gchar*));
	if(!list) return;
	for(i=0;i<nlist;i++)
		list[i] = g_malloc(10*sizeof(gchar));


	if(GTK_IS_WIDGET(comboSpinMultiplicity)) gtk_widget_set_sensitive(comboSpinMultiplicity, TRUE);
	if(ne%2==0)
		k = 1;
	else
		k = 2;

	kinc = 2;
	for(i=0;i<nlist;i++)
	{
		sprintf(list[i],"+%d",k);
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

	nlist = mpqcMolecule.numberOfValenceElectrons*2-2+1;

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
	setChargeComboCharge(comboCharge, mpqcMole.totalCharge);
}
/**********************************************************************/
static void changedEntrySpinMultiplicity(GtkWidget *entry, gpointer data)
{
	GtkWidget* comboMethod;
	G_CONST_RETURN gchar* entryText = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	mpqcMole.spinMultiplicity=atoi(entryText);
	if(mpqcMole.method) g_free(mpqcMole.method);
	if(mpqcMole.spinMultiplicity==1) mpqcMole.method = g_strdup("CLHF");
	else mpqcMole.method = g_strdup("UHF");

	comboMethod = g_object_get_data(G_OBJECT (entry), "ComboMethod");
	if(GTK_IS_WIDGET(comboMethod)) setComboMethod(comboMethod);
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

	mpqcMole.totalCharge = atoi(entryText);
	TotalCharges[0] = mpqcMole.totalCharge;

	comboSpinMultiplicity  = g_object_get_data(G_OBJECT (entry), "ComboSpinMultiplicity");
	if(GTK_IS_WIDGET(comboSpinMultiplicity)) setComboSpinMultiplicity(comboSpinMultiplicity);

	labelNumberOfElectrons = g_object_get_data(G_OBJECT (entry), "LabelNumberOfElectrons");

	if(GTK_IS_WIDGET(labelNumberOfElectrons))
	{
		gint ne = mpqcMolecule.numberOfValenceElectrons - mpqcMole.totalCharge;
		gchar buffer[BSIZE];
		sprintf(buffer, _("Number of electrons = %d"),ne);
		gtk_label_set_text(GTK_LABEL(labelNumberOfElectrons),buffer);
	}
}
/************************************************************************************************************/
static void setComboFunctional(GtkWidget *comboFunctional)
{
	GList *glist = NULL;
	gint n = mPW1PW91+1;
	gint i;
	for(i=0;i<n;i++)
  		glist = g_list_append(glist,stdFunctionalsMPQC[i].name);
  	glist = g_list_append(glist,_("Your functional"));
  	gtk_combo_box_entry_set_popdown_strings( comboFunctional, glist) ;
}
/************************************************************************************************************/
static void changedEntryFunctional(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	gboolean sumDen = FALSE;
	GtkWidget* wid;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	if(mpqcMole.functional) g_free(mpqcMole.functional);
	mpqcMole.functional=g_strdup(entryText);
	if(strcmp(entryText,_("Your functional"))==0) sumDen = TRUE;

	wid = g_object_get_data(G_OBJECT (entry), "ButtonWhat");
	if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid,!sumDen);
	wid = g_object_get_data(G_OBJECT (entry), "ButtonSumDenFunctional");
	if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid,sumDen);
}
/***********************************************************************************************/
void whatFunctional(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	gint n = mPW1PW91+1;
	gint i;
	gchar* dump = NULL;
	gchar* message = NULL;
	gboolean OK = FALSE;

	if(!GTK_IS_WIDGET(entry)) return;
	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	dump = g_malloc(BSIZE*sizeof(gchar));
	message = g_malloc(BSIZE*sizeof(gchar));
	for(i=0;i<n;i++)
		if(strcmp(entryText,stdFunctionalsMPQC[i].name)==0)
		{
			gint j;
			OK = TRUE;
			sprintf(message,"%s",_(" This is :\n"));
			for(j=0;j<stdFunctionalsMPQC[i].n;j++)
			{
				MPQCFunctionalType type = stdFunctionalsMPQC[i].listOfTypes[j];
				gdouble coef = stdFunctionalsMPQC[i].coefficients[j];
			 	sprintf(dump,"%s",message);
				if(j==0)
				{
					if(coef==1)
			 			sprintf(message,"%s       %s\n",dump, functionalsMPQC[type].comment);
					else
			 			sprintf(message,"%s     %0.3f %s\n",dump, coef, functionalsMPQC[type].comment);
				}
				else
				{
					if(coef==1)
			 			sprintf(message,"%s +    %s\n",dump, functionalsMPQC[type].comment);
					else
			 			sprintf(message,"%s + %0.3f %s\n",dump, coef, functionalsMPQC[type].comment);
				}
			}
		}

	if(OK) 
	{
		GtkWidget* mess = Message(message,_("Info"),TRUE);
    		gtk_window_set_modal (GTK_WINDOW (mess), TRUE);
	}
	g_free(message);
	g_free(dump);
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
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);
	entry = GTK_BIN (combo)->child;
	g_object_set_data(G_OBJECT (entry), "Combo",combo);
	gtk_widget_set_size_request(GTK_WIDGET(entry),(gint)(ScreenHeight*0.2),-1);

	return entry;
}
/***********************************************************************************************/
static GtkWidget *addMPQCChargeToTable(GtkWidget *table, gint i)
{
	GtkWidget* entryCharge = NULL;
	GtkWidget* comboCharge = NULL;
	gint nlistCharge = 1;
	gchar* listCharge[] = {"0"};
	GtkWidget* labelNumberOfElectrons = NULL;
	GtkWidget* hbox = NULL;

	add_label_table(table,"Charge",(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entryCharge = addComboListToATable(table, listCharge, nlistCharge, i, 2, 1);
	comboCharge  = g_object_get_data(G_OBJECT (entryCharge), "Combo");
	gtk_widget_set_sensitive(entryCharge, FALSE);

	labelNumberOfElectrons = gtk_label_new(" ");
	hbox = gtk_hbox_new(0,FALSE);
	gtk_box_pack_start (GTK_BOX (hbox), labelNumberOfElectrons, FALSE, FALSE, 0);
	gtk_table_attach(GTK_TABLE(table),hbox,3,3+2,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);

	g_object_set_data(G_OBJECT (entryCharge), "LabelNumberOfElectrons", labelNumberOfElectrons);
	g_signal_connect(G_OBJECT(entryCharge),"changed", G_CALLBACK(changedEntryCharge),NULL);
	setComboCharge(comboCharge);
	return comboCharge;
}
/***********************************************************************************************/
static GtkWidget *addMPQCSpinToTable(GtkWidget *table, gint i)
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
	setComboSpinMultiplicity(comboSpinMultiplicity);
	return comboSpinMultiplicity;
}
/***********************************************************************************************/
static GtkWidget* addMPQCMethodToTable(GtkWidget *table, gint i, GtkWidget *comboSpinMultiplicity)
{
	GtkWidget* entryMethod = NULL;
	GtkWidget* comboMethod = NULL;
	gint nlistMethod = 1;
	gchar* listMethod[] = {"CLHF"};
	GtkWidget* labelMethod = NULL;
	GtkWidget* entrySpinMultiplicity = NULL;
	GtkWidget* hbox = NULL;

	entrySpinMultiplicity  = GTK_BIN (comboSpinMultiplicity)->child;

	add_label_table(table, _("Method"), (gushort)i, 0);
	add_label_table(table, ":", (gushort)i, 1);
	entryMethod = addComboListToATable(table, listMethod, nlistMethod, i, 2, 1);
	comboMethod  = g_object_get_data(G_OBJECT (entryMethod), "Combo");
	gtk_widget_set_sensitive(entryMethod, FALSE);


	/*labelMethod = add_label_table(table, " ", i, 3);*/
	labelMethod = gtk_label_new(" ");
	hbox = gtk_hbox_new(0,FALSE);
	gtk_box_pack_start (GTK_BOX (hbox), labelMethod, FALSE, FALSE, 0);
	gtk_table_attach(GTK_TABLE(table),hbox,3,3+2,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);

	g_object_set_data(G_OBJECT (entryMethod), "ComboSpinMultiplicity", comboSpinMultiplicity);
	g_object_set_data(G_OBJECT (entryMethod), "LabelMethod", labelMethod);
	g_object_set_data(G_OBJECT (entryMethod), "EntrySpinMultiplicity", entrySpinMultiplicity);
	g_object_set_data(G_OBJECT (comboMethod), "EntrySpinMultiplicity", entrySpinMultiplicity);
	g_object_set_data(G_OBJECT (entrySpinMultiplicity), "ComboMethod", comboMethod);
	g_signal_connect(G_OBJECT(entryMethod),"changed", G_CALLBACK(changedEntryMethod),NULL);
	setComboMethod(comboMethod);
	return comboMethod;
}
/***********************************************************************************************/
static void addMPQCFunctionalToTable(GtkWidget *table, gint i, GtkWidget* comboMethod)
{
	GtkWidget* entryFunctional = NULL;
	GtkWidget* comboFunctional = NULL;
	gint nlistFunctional = 1;
	gchar* listFunctional[] = {"B3LYP"};
	GtkWidget* entryMethod = NULL;
	GtkWidget* label = NULL;
	GtkWidget* what = NULL;
	GtkWidget* sumDenFunctional = NULL;
	GtkWidget* hbox = NULL;

	if(GTK_IS_COMBO_BOX(comboMethod)) entryMethod = GTK_BIN(comboMethod)->child;

	label = add_label_table(table, _("Functional"), (gushort)i, 0);
	if(entryMethod) g_object_set_data(G_OBJECT (entryMethod), "LabelFunctional1", label);
	add_label_table(table, ":", (gushort)i, 1);
	if(entryMethod) g_object_set_data(G_OBJECT (entryMethod), "LabelFunctional2", label);
	entryFunctional = addComboListToATable(table, listFunctional, nlistFunctional, i, 2, 1);
	comboFunctional  = g_object_get_data(G_OBJECT (entryFunctional), "Combo");
	if(entryMethod) g_object_set_data(G_OBJECT (entryMethod), "ComboFunctional", comboFunctional);
	gtk_widget_set_sensitive(entryFunctional, FALSE);


	what = gtk_button_new_with_label(_("What this ?"));
	hbox = gtk_hbox_new(0,FALSE);
	gtk_box_pack_start (GTK_BOX (hbox), what, FALSE, FALSE, 0);
	add_widget_table(table,hbox,(gushort)i,3);
	if(entryMethod) g_object_set_data(G_OBJECT (entryMethod), "ButtonWhat", what);

	sumDenFunctional = gtk_button_new_with_label(_("Your functional"));
	hbox = gtk_hbox_new(0,FALSE);
	gtk_box_pack_start (GTK_BOX (hbox), sumDenFunctional, TRUE, TRUE, 0);
	add_widget_table(table,hbox,(gushort)i,4);
	if(entryMethod) g_object_set_data(G_OBJECT (entryMethod), "ButtonSumDenFunctional", sumDenFunctional);

	g_object_set_data(G_OBJECT (entryFunctional), "ButtonWhat", what);
	g_object_set_data(G_OBJECT (entryFunctional), "ButtonSumDenFunctional", sumDenFunctional);
	g_signal_connect(G_OBJECT(entryFunctional),"changed", G_CALLBACK(changedEntryFunctional),NULL);
	setComboFunctional(comboFunctional);
	/* activate sensitivity */
	if(GTK_IS_WIDGET(comboMethod)) setComboMethod(comboMethod);
	g_signal_connect_swapped(G_OBJECT(what), "clicked",G_CALLBACK(whatFunctional),GTK_OBJECT(entryFunctional));
	g_signal_connect_swapped(G_OBJECT(sumDenFunctional), "clicked",G_CALLBACK(mpqcSumDensityFunctionalWindow),GTK_OBJECT(entryFunctional));
}
/***********************************************************************************************/
void createMPQCMole(GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* comboSpinMultiplicity = NULL;
	GtkWidget* comboCharge = NULL;
	GtkWidget* comboMethod = NULL;
	GtkWidget *table = NULL;
	gint i;

	table = gtk_table_new(6,5,FALSE);

	frame = gtk_frame_new (_("Parameters for computing the molecule's energy"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	i = 0;
	comboCharge = addMPQCChargeToTable(table, i);
	i = 1;
	comboSpinMultiplicity = addMPQCSpinToTable(table, i);

	if(GTK_IS_COMBO_BOX(comboCharge))
		g_object_set_data(G_OBJECT (GTK_BIN(comboCharge)->child), "ComboSpinMultiplicity", comboSpinMultiplicity);

	i = 2;
	comboMethod = addMPQCMethodToTable(table, i, comboSpinMultiplicity);
	i = 3;
	addMPQCFunctionalToTable(table,i, comboMethod);
	i = 4;
	addMPQCBasisToTable(table, i);
	i = 5;
	addMPQCAuxBasisToTable(table, i, comboMethod);
	/* activate sensitivity */
	if(GTK_IS_WIDGET(comboMethod)) setComboMethod(comboMethod);
	g_object_set_data(G_OBJECT (box), "EntryMethod", GTK_BIN(comboMethod)->child);
}
/************************************************************************************************************/
static void putMPQCFunctionalInTextEditor()
{
        gchar buffer[BSIZE];
        gchar dump[BSIZE];

	sprintf(buffer,"\t%c Functional method\n",'%');
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	if(strstr(mpqcMole.functional,"Your"))
	{
		gint n = G96X + 1;
		gint i;
		sprintf(buffer,"functional<SumDenFunctional>: (\n");
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\t", -1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);
		sprintf(buffer,"\t\tcoefs = [ ");
		for(i=0;i<n;i++)
		{
			if(i!=HFX && sumFunctionalsMPQC[i] != 0)
			{
				sprintf(dump,"%s",buffer);
				sprintf(buffer,"%s %0.3f ", dump, sumFunctionalsMPQC[i]);
			}
		}
		sprintf(dump,"%s",buffer);
		sprintf(buffer,"%s ] \n", dump);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

		if(sumFunctionalsMPQC[HFX] != 0)
		{
			sprintf(buffer,"\t\ta0 = %0.3f\n", sumFunctionalsMPQC[HFX]);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
		}
		sprintf(buffer,"\t\tfuncs : [ \n");
		for(i=0;i<n;i++)
		{
			if(i!=HFX && sumFunctionalsMPQC[i] != 0)
			{
				sprintf(dump,"%s",buffer);
				sprintf(buffer,"%s\t\t<%s>:()\n", dump, functionalsMPQC[i].name);
			}
		}
		sprintf(dump,"%s",buffer);
		sprintf(buffer,"%s\t\t]\n", dump);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

		sprintf(buffer,")\n");
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\t", -1);
		gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);
	}
	else
	{
		sprintf(buffer,"functional<StdDenFunctional>: (\n");
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\t", -1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);

		sprintf(buffer,"\t\tname = \"%s\" \n", mpqcMole.functional);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
		sprintf(buffer,")\n");
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\t", -1);
		gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);
	}
}
/************************************************************************************************************/
static void putMPQCAuxBasisInTextEditor()
{
        gchar buffer[BSIZE];

	sprintf(buffer,"\t%c Auxiliary basis\n",'%');
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	sprintf(buffer,"aux_basis<GaussianBasisSet>: (\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\t", -1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);

	sprintf(buffer,"\t\tname = \"%s\" \n", mpqcMole.auxBasisName);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	sprintf(buffer,"\t\tmolecule = $:molecule\n");
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	sprintf(buffer,")\n");
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\t", -1);
	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);
}
/************************************************************************************************************/
static void putMPQCMoleInTextEditor()
{
        gchar buffer[BSIZE];

	sprintf(buffer,"%c Method for computing the molecule's energy\n",'%');
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	sprintf(buffer,"mole<%s>: (\n",mpqcMole.method);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);

	sprintf(buffer,"\ttotal_charge = %d\n", mpqcMole.totalCharge);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	if(!strstr(mpqcMole.method,"CL"))
	{
		sprintf(buffer,"\tmultiplicity = %d\n", mpqcMole.spinMultiplicity);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	}

	sprintf(buffer,"\tmolecule = %s\n", mpqcMole.molecule);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	sprintf(buffer,"\tmemory = %s\n", mpqcMole.memory);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	if(mpqcMole.printNPA) sprintf(buffer,"\tprint_npa =  yes\n");
	else sprintf(buffer,"\tprint_npa =  no\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	if(mpqcMole.printNAO) sprintf(buffer,"\tprint_nao =  yes\n");
	else sprintf(buffer,"\tprint_nao =  no\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);


	sprintf(buffer,"\tbasis = %s\n", mpqcMole.basis);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	if(strstr(mpqcMole.method,"KS") || strstr(mpqcMole.method,"ks")) putMPQCFunctionalInTextEditor();
	if(!mpqcGuessWaveFunction.fromHCore)
	{
		sprintf(buffer,"\tguess_wavefunction = %s\n",mpqcMole.guessWaveFunction);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	}

	sprintf(buffer,")\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);

	sprintf(buffer,"%c----------------------------------------------------------------\n",'%');
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
}
/************************************************************************************************************/
static void putMPQCReferenceInTextEditor()
{
        gchar buffer[BSIZE];

	sprintf(buffer,"%c Reference state for MP2\n",'%');
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	sprintf(buffer,"reference<HSOSHF>: (\n");
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);
	sprintf(buffer,"\ttotal_charge = %d\n", mpqcMole.totalCharge);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	sprintf(buffer,"\tmultiplicity = %d\n", mpqcMole.spinMultiplicity);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	
	sprintf(buffer,"\tbasis = %s\n", mpqcMole.basis);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	sprintf(buffer,"\tmolecule = %s\n", mpqcMole.molecule);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	sprintf(buffer,"\tmemory = %s\n", mpqcMole.memory);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);


	if(mpqcMole.printNPA) sprintf(buffer,"\tprint_npa =  yes\n");
	else sprintf(buffer,"\tprint_npa =  no\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	if(mpqcMole.printNAO) sprintf(buffer,"\tprint_nao =  yes\n");
	else sprintf(buffer,"\tprint_nao =  no\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	if(!mpqcGuessWaveFunction.fromHCore)
	{
		sprintf(buffer,"\tguess_wavefunction = %s\n",mpqcMole.guessWaveFunction);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	}

	sprintf(buffer,")\n");
	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);
}
/************************************************************************************************************/
static void putMPQCMoleMP2InTextEditor()
{
        gchar buffer[BSIZE];
	gchar* stdapprox = NULL;
	gchar* method = NULL;
	gchar* algorithm = NULL;

	sprintf(buffer,"%c Method for computing the molecule's energy\n",'%');
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	if(!strstr(mpqcMole.method,"R12"))
	{
		if(strstr(mpqcMole.method,"MBPT2"))
		{
			sprintf(buffer,"mole<%s>: (\n",mpqcMole.method);
			method = g_strdup("mp");
			algorithm = g_strdup("v1");
		}
		else
		if(strstr(mpqcMole.method,"ZAPT"))
		{
			sprintf(buffer,"mole<MBPT2>: (\n");
			method = g_strdup("zapt");
			algorithm = g_strdup("v1");
		}
		else
		if(strstr(mpqcMole.method,"OPT1"))
		{
			sprintf(buffer,"mole<MBPT2>: (\n");
			method = g_strdup("opt1");
			algorithm = g_strdup("v1");
		}
		else
		if(strstr(mpqcMole.method,"OPT2"))
		{
			sprintf(buffer,"mole<MBPT2>: (\n");
			method = g_strdup("opt2");
			algorithm = g_strdup("v1");
		}
		else
			sprintf(buffer,"mole<%s>: (\n",mpqcMole.method);
	}
	else
	{
		sprintf(buffer,"mole<MBPT2_R12>: (\n");
		if(strstr(mpqcMole.method,"A'"))
			stdapprox = g_strdup_printf("\"A'\"");
		else
			stdapprox = g_strdup_printf("\"A\"");
	}
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);


	sprintf(buffer,"\tmolecule = %s\n", mpqcMole.molecule);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	sprintf(buffer,"\tbasis = %s\n", mpqcMole.basis);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	sprintf(buffer,"\tmemory = %s\n", mpqcMole.memory);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	if(strstr(mpqcMole.method,"R12"))
	{
		if(stdapprox)
		{
			sprintf(buffer,"\tstdapprox = %s\n",stdapprox);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
			g_free(stdapprox);
		}
		putMPQCAuxBasisInTextEditor();
	}
	if(method)
	{
		sprintf(buffer,"\tmethod = %s\n",method);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	}
	if(algorithm)
	{
		if(method && strstr(method,"mp"))
		{
			sprintf(buffer,"\t%c algorithm = v1, v2, v2lb or memgrp \n",'%');
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
		}
		else
		{
			sprintf(buffer,"\t%c algorithm = v1, v2 or v2lb \n",'%');
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
		}
		sprintf(buffer,"\talgorithm = %s\n",algorithm);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	}
	if(algorithm) g_free(algorithm);
	if(method) g_free(method);

	sprintf(buffer,"\treference = $:reference\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	sprintf(buffer,")\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);

	sprintf(buffer,"%c----------------------------------------------------------------\n",'%');
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
}
/************************************************************************************************************/
void putMPQCMoleInfoInTextEditor()
{
	if(
		strstr(mpqcMole.method,"MBPT2") ||
		strstr(mpqcMole.method,"ZAPT") ||
		strstr(mpqcMole.method,"OPT")
	  )
	{
		putMPQCReferenceInTextEditor();
		putMPQCMoleMP2InTextEditor();
	}
	else
		putMPQCMoleInTextEditor();
}
