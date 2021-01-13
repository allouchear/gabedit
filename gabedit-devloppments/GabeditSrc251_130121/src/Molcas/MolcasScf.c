/* MolcasScf.c */
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

#include "../../Config.h"
#include "../Common/Global.h"
#include "../Molcas/MolcasTypes.h"
#include "../Molcas/MolcasGlobal.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/GeomConversion.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Symmetry/MoleculeSymmetry.h"
#include "../Symmetry/MoleculeSymmetryInterface.h"

static MolcasScf molcasScfTmp;
static GtkWidget* entryType = NULL;
/************************************************************************************************************/
static void initMolcasScf(MolcasScf* mScf)
{
	gint i;

	sprintf(mScf->title,"SCF data By Gabedit");
	sprintf(mScf->type,"Restricted");
	sprintf(mScf->method,"HF");

	mScf->typeOfOcupations = 0;
	mScf->charge[0] = TotalCharges[0];
	mScf->charge[1] = -1;

	mScf->aufBau[0] = 0;
	mScf->aufBau[1] = 0;

	for(i = 0; i<8; i++)
	{
		mScf->numberOfOccupedOrbitals[i] = -1;
		mScf->numberOfOccupedOrbitalsBeta[i] = -1;
	}

	mScf->numberOfOrbitals = 0;
	mScf->numberOfOrbitalsBeta = 0;
	mScf->occupationOrbitals = NULL;  
	mScf->occupationOrbitalsBeta = NULL;


	mScf->zSpin = SpinMultiplicities[0]-1;

	mScf->guessOrbitalsCore = FALSE;

	mScf->numberOfNDDOIterations = 0; 
	mScf->numberOfRHFIterations  = 0;

	for(i = 0; i<4; i++)
		mScf->convergenceThresholds[i] = -1;

	mScf->diskSize[0] = 1; /* 1MB */
	mScf->diskSize[1] = 0; /* 0 -> default -> The size of the I/O buffer by default (512 kByte) */

	mScf->ivo = FALSE;
}
/************************************************************************************************************/
static void initMolcasScfTmp()
{
	initMolcasScf(&molcasScfTmp);
}
/************************************************************************************************************/
static void copyScfParameters(MolcasScf* newCopy, MolcasScf* toCopy)
{
	gint i;

	sprintf( newCopy->title,"%s", toCopy->title);
	sprintf( newCopy->type, "%s", toCopy->type);
	sprintf( newCopy->method, "%s", toCopy->method);

	newCopy->typeOfOcupations = toCopy->typeOfOcupations;

	for(i = 0; i<2; i++) newCopy->charge[i] = toCopy->charge[i];

	for(i = 0; i<2; i++) newCopy->aufBau[i] = toCopy->aufBau[i];

	for(i = 0; i<8; i++) newCopy->numberOfOccupedOrbitals[i] = -1;
	for(i = 0; i<8; i++) newCopy->numberOfOccupedOrbitalsBeta[i] = -1;

	newCopy->numberOfOrbitals = toCopy->numberOfOrbitals;
	newCopy->numberOfOrbitalsBeta = toCopy->numberOfOrbitalsBeta;
	if(newCopy->occupationOrbitals) g_free(newCopy->occupationOrbitals);
	if(newCopy->occupationOrbitalsBeta) g_free(newCopy->occupationOrbitalsBeta);

	if(toCopy->numberOfOrbitals>0) 
		newCopy->occupationOrbitals  = g_malloc(toCopy->numberOfOrbitals*sizeof(gdouble));
	for(i = 0; i<toCopy->numberOfOrbitals; i++) 
		newCopy->occupationOrbitals[i] = toCopy->occupationOrbitals[i]; 

	if(toCopy->numberOfOrbitalsBeta>0) 
		newCopy->occupationOrbitalsBeta  = g_malloc(toCopy->numberOfOrbitalsBeta*sizeof(gdouble));
	for(i = 0; i<toCopy->numberOfOrbitalsBeta; i++) 
		newCopy->occupationOrbitalsBeta[i] = toCopy->occupationOrbitalsBeta[i]; 

	newCopy->zSpin = toCopy->zSpin;

	newCopy->guessOrbitalsCore = toCopy->guessOrbitalsCore;


	newCopy->numberOfNDDOIterations = toCopy->numberOfNDDOIterations;
	newCopy->numberOfRHFIterations = toCopy->numberOfRHFIterations;

	for(i=0;i<4;i++)
		newCopy-> convergenceThresholds [i] = toCopy->convergenceThresholds[i];

	for(i=0;i<2;i++)
		newCopy->diskSize[i] = toCopy->diskSize[i];

	newCopy->ivo = toCopy->ivo;
}
/************************************************************************************************************/
static void copyScfParametersFromTmp(GtkWidget *win, gpointer data)
{

	copyScfParameters(&molcasScf, &molcasScfTmp);
	TotalCharges[0] = molcasScf.charge[0];
	SpinMultiplicities[0] = molcasScf.zSpin+1;
}
/********************************************************************************/
static void setComboZSpin(GtkWidget *comboZSpin, gboolean restricted)
{
	GList *glist = NULL;
	gint i;
	gint nlist = 0;
	gchar** list = NULL;
	gint k;
	gint kinc;
	gint ne = molcasMolecule.numberOfValenceElectrons - molcasScfTmp.charge[0];

	if(restricted) nlist = 1;
	else
	{
		if(ne%2==0)
			nlist = ne/2+1;
		else
			nlist = (ne+1)/2;
	}

	if(nlist<1) return;
	list = g_malloc(nlist*sizeof(gchar*));
	if(!list) return;
	for(i=0;i<nlist;i++)
		list[i] = g_malloc(10*sizeof(gchar));


	if(restricted)
	{
		if(ne%2==0) sprintf(list[0],"0");
		else sprintf(list[0],"1");
		gtk_widget_set_sensitive(comboZSpin, FALSE);
	}
	else
	{
		gtk_widget_set_sensitive(comboZSpin, TRUE);
		if(ne%2==0)
		{
			k = 0;
			kinc = 1;
		}
		else
		{
			k = 1;
			kinc = 2;
		}
		for(i=0;i<nlist;i++)
		{
			sprintf(list[i],"+%d",k);
			k+=kinc;
		}
	}

  	for(i=0;i<nlist;i++)
			glist = g_list_append(glist,list[i]);

  	gtk_combo_box_entry_set_popdown_strings( comboZSpin, glist) ;
  	g_list_free(glist);
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
static void setComboCharge(GtkWidget *comboCharge, gboolean restricted)
{
	GList *glist = NULL;
	gint i;
	gint nlist = (molcasMolecule.numberOfValenceElectrons-1)/2*2 + 1;
	gchar** list = NULL;
	gint k;

	if(restricted) nlist = (molcasMolecule.numberOfValenceElectrons-1)/2*2 + 1;
	else nlist = molcasMolecule.numberOfValenceElectrons*2-2+1;

	if(nlist<1) return;
	list = g_malloc(nlist*sizeof(gchar*));
	if(!list) return;
	for(i=0;i<nlist;i++)
		list[i] = g_malloc(10*sizeof(gchar));


	if(restricted)
	{
		if(molcasMolecule.numberOfValenceElectrons%2==0)
		{
			sprintf(list[0],"0");
			k = 2;
		}
		else
		{
			sprintf(list[0],"1");
			k = 1;
		}
		for(i=1;i<nlist-1;i+=2)
		{
			sprintf(list[i],"+%d",k);
			sprintf(list[i+1],"%d",-k);
			k += 2;
		}
	}
	else
	{
		sprintf(list[0],"0");
		k = 1;
		for(i=1;i<nlist-1;i+=2)
		{
			sprintf(list[i],"+%d",k);
			sprintf(list[i+1],"%d",-k);
			k += 1;
		}
	}

  	for(i=0;i<nlist;i++)
			glist = g_list_append(glist,list[i]);

  	gtk_combo_box_entry_set_popdown_strings( comboCharge, glist) ;
  	g_list_free(glist);
	if(list)
	{
		for(i=0;i<nlist;i++) if(list[i]) g_free(list[i]);
		g_free(list);
	}
	setChargeComboCharge(comboCharge, molcasScfTmp.charge[0]);
}
/**********************************************************************/
static void changedEntryZSpin(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	molcasScfTmp.zSpin=atoi(entryText);
}
/**********************************************************************/
static void changedEntryCharge(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	GtkWidget* comboZSpin = NULL;
	GtkWidget* labelZSpin = NULL;
	GtkWidget* labelNumberOfElectrons = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	molcasScfTmp.typeOfOcupations = 1;
	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	molcasScfTmp.charge[0]=atoi(entryText);

	comboZSpin  = g_object_get_data(G_OBJECT (entry), "Combo");
	labelZSpin  = g_object_get_data(G_OBJECT (entry), "Label");
	if(!GTK_IS_WIDGET(comboZSpin)) return;
	if(strcmp(molcasScfTmp.type,"Restricted")==0)
	{
		setComboZSpin(comboZSpin, TRUE);
		if(GTK_IS_WIDGET(labelZSpin)) gtk_widget_set_sensitive(labelZSpin, FALSE);
	}
	else
	{
		setComboZSpin(comboZSpin, FALSE);
		if(GTK_IS_WIDGET(labelZSpin)) gtk_widget_set_sensitive(labelZSpin, TRUE);
	}
	labelNumberOfElectrons = g_object_get_data(G_OBJECT (entry), "LabelNumberOfElectrons");
	if(GTK_IS_WIDGET(labelNumberOfElectrons))
	{
		gint ne = molcasMolecule.numberOfValenceElectrons - molcasScfTmp.charge[0];
		gchar buffer[BSIZE];
		sprintf(buffer, _("Number Of Electrons = %d"),ne);
		gtk_label_set_text(GTK_LABEL(labelNumberOfElectrons),buffer);
	}
}
/**********************************************************************/
static void changedEntryType(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	GtkWidget* comboCharge = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	sprintf(molcasScfTmp.type, "%s", entryText);
	comboCharge  = g_object_get_data(G_OBJECT (entry), "Combo");
	if(!GTK_IS_WIDGET(comboCharge)) return;
	if(strcmp(molcasScfTmp.type,"Restricted")==0) setComboCharge(comboCharge, TRUE);
	else setComboCharge(comboCharge, FALSE);
	
}
/**********************************************************************/
static void changedEntryMethod(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	sprintf(molcasScfTmp.method,"%s", entryText);
}
/*********************************************************************************************/
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

	return entry;
}
/***********************************************************************************************/
static void initScfEntryType()
{
	entryType = NULL;
}
/***********************************************************************************************/
void activateScfEntryType()
{
	if(entryType && GTK_IS_WIDGET(entryType))
	{
		gtk_entry_set_text(GTK_ENTRY(entryType)," ");
		if(SpinMultiplicities[0]==1)
		gtk_entry_set_text(GTK_ENTRY(entryType),"Restricted"); /* pour activer entryType */
		else
		gtk_entry_set_text(GTK_ENTRY(entryType),"UnRestricted"); /* pour activer entryType */
	}
}
/***********************************************************************************************/
void createScfFrame(GtkWidget *win, GtkWidget *box, GtkWidget *OkButton)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget *table = gtk_table_new(3,4,FALSE);
	GtkWidget* entryMethod = NULL;
	GtkWidget* labelTypes = gtk_label_new(_("Type : "));
	GtkWidget* labelMethods = gtk_label_new(_("Method : "));
	gint nlistTypes = 2;
	gchar* listTypes[] = {"Restricted", "UnRestricted"};
	gint nlistMethods = 14;
	gchar* listMethods[] = { "HF", "BLYP", "B3LYP", "B3LYP5", "HFB", "HFS", 
	   	"LDA", "LDA5", "LSDA", "LSDA5", "SVWN", "SVWN5", "TLYP", "XPBE" 
	};
	GtkWidget* entryCharge = NULL;
	GtkWidget* comboCharge = NULL;
	GtkWidget* labelCharge = gtk_label_new(_("Charge : "));
	gint nlistCharge = 1;
	gchar* listCharge[] = {"0"};

	GtkWidget* labelNumberOfElectrons = gtk_label_new("         ");

	GtkWidget* entryZSpin = NULL;
	GtkWidget* comboZSpin = NULL;
	GtkWidget* labelZSpin = gtk_label_new(_("2* Z Spin : "));
	gint nlistZSpin = 1;
	gchar* listZSpin[] = {"0"};

	initMolcasScfTmp();
	initMolcasScf(&molcasScf);

	frame = gtk_frame_new (_("SCF Program"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	add_widget_table(table, labelTypes, 0, 0);
	entryType = addComboListToATable(table, listTypes, nlistTypes, 0, 1, 1);
	gtk_widget_set_sensitive(entryType, FALSE);

	add_widget_table(table, labelMethods, 0, 2);
	entryMethod = addComboListToATable(table, listMethods, nlistMethods, 0, 3, 1);
	gtk_widget_set_sensitive(entryMethod, FALSE);


	add_widget_table(table, labelCharge, 1, 0);
	entryCharge = addComboListToATable(table, listCharge, nlistCharge, 1, 1, 1);
	comboCharge  = g_object_get_data(G_OBJECT (entryCharge), "Combo");
	gtk_widget_set_sensitive(entryCharge, FALSE);

	gtk_table_attach(GTK_TABLE(table),labelNumberOfElectrons,2,2+2,1,1+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);

	add_widget_table(table, labelZSpin, 2, 0);
	entryZSpin = addComboListToATable(table, listZSpin, nlistZSpin, 2, 1, 1);
	comboZSpin  = g_object_get_data(G_OBJECT (entryZSpin), "Combo");
	gtk_widget_set_sensitive(entryZSpin, FALSE);

	g_signal_connect(G_OBJECT(entryType),"changed", G_CALLBACK(changedEntryType),NULL);
	g_object_set_data(G_OBJECT (entryType), "Combo", comboCharge);
	g_signal_connect(G_OBJECT(entryZSpin),"changed", G_CALLBACK(changedEntryZSpin),NULL);
	g_object_set_data(G_OBJECT (entryCharge), "Combo", comboZSpin);
	g_object_set_data(G_OBJECT (entryCharge), "Label", labelZSpin);
	g_object_set_data(G_OBJECT (entryCharge), "LabelNumberOfElectrons", labelNumberOfElectrons);
	g_signal_connect(G_OBJECT(entryCharge),"changed", G_CALLBACK(changedEntryCharge),NULL);
	g_signal_connect(G_OBJECT(entryMethod),"changed", G_CALLBACK(changedEntryMethod),NULL);


	gtk_entry_set_text(GTK_ENTRY(entryMethod),"HF");

	activateScfEntryType();

	molcasScfTmp.charge[0] = TotalCharges[0];
	if(strcmp(molcasScfTmp.type,"Restricted")==0) setComboCharge(comboCharge, TRUE);
	else setComboCharge(comboCharge, FALSE);

	if(OkButton)
	{
		g_signal_connect(G_OBJECT(OkButton), "clicked",G_CALLBACK(copyScfParametersFromTmp),NULL);
		g_signal_connect(G_OBJECT(OkButton), "clicked",G_CALLBACK(initScfEntryType),NULL);
	}
}
/************************************************************************************************************/
static void putBeginScfInTextEditor()
{

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.program, &molcasColorBack.program, " &SCF\n",-1);
}
/************************************************************************************************************/
static void putTitleScfInTextEditor()
{
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.subProgram, NULL, "Title=",-1);

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, molcasScf.title,-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);

}
/************************************************************************************************************/
static void putMethodScfInTextEditor()
{
        gchar buffer[BSIZE];

	if(strcmp(molcasScf.type,"Restricted")==0 && strcmp(molcasScf.method,"HF")==0) return;

	if(strcmp(molcasScf.type,"UnRestricted")==0)
	{
		sprintf(buffer,"UHF\n");
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL,  &molcasColorFore.subProgram, NULL, buffer,-1);
	}
	
	if(strcmp(molcasScf.method,"HF")!=0)
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL,  &molcasColorFore.subProgram, NULL, "KSDFT\n",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, molcasScf.method,-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
	}


}
/************************************************************************************************************/
static void putChargeScfInTextEditor()
{
        gchar buffer[BSIZE];

	if(molcasScf.typeOfOcupations ==0 ) return;

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.subProgram, NULL, "Charge=",-1);
	if(molcasScf.charge[1]>0)
		sprintf(buffer,"%d %d\n", molcasScf.charge[0], molcasScf.charge[1]);
	else
		sprintf(buffer,"%d\n", molcasScf.charge[0]);

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);

}
/************************************************************************************************************/
static void putZSpinScfInTextEditor()
{
        gchar buffer[BSIZE];

	if(strcmp(molcasScf.type,"Restricted")==0) return;

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.subProgram, NULL, "ZSpin=",-1);
	sprintf(buffer,"%d\n", molcasScf.zSpin);

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);

}
/************************************************************************************************************/
static void putIterationsScfInTextEditor()
{
        gchar buffer[BSIZE];

	if(molcasScf.numberOfNDDOIterations == 0 && molcasScf.numberOfRHFIterations == 0) return;

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.subProgram, NULL, "Iterations\n",-1);

	sprintf(buffer,"* NDDO and RHF   iterations\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);

	sprintf(buffer," %d %d\n", molcasScf.numberOfNDDOIterations, molcasScf.numberOfRHFIterations);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	

}
/************************************************************************************************************/
static void putThresholdScfInTextEditor()
{
        gchar buffer[BSIZE];
	gint i;

	for(i=0; i<4; i++) if(molcasScf.convergenceThresholds[i] <0) return;

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.subProgram, NULL, "Threshold\n",-1);

	sprintf(buffer,"*   convergence   thresholds \n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);

	sprintf(buffer,"*  EThr   DThr   FThr   DltNTh\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);

	sprintf(buffer,"%e %e %e %e\n",
			molcasScf.convergenceThresholds[0],
			molcasScf.convergenceThresholds[1],
			molcasScf.convergenceThresholds[2],
			molcasScf.convergenceThresholds[3]
			);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);

}
/************************************************************************************************************/
static void putDiskScfInTextEditor()
{
        gchar buffer[BSIZE];


	if( molcasScf.diskSize[0] == 1 &&  molcasScf.diskSize[1] == 0) return;

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.subProgram, NULL, "Disk\n",-1);

	sprintf(buffer,"* 1 -> Semi-direct algorithm writing max 128k words (1MByte) to disk \n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);

	sprintf(buffer,"* 0 -> The size of the I/O buffer by default (512 kByte) \n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);

	sprintf(buffer,"1   0\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);

}
/************************************************************************************************************/
static void putIVOScfInTextEditor()
{
        gchar buffer[BSIZE];

	if(!molcasScf.ivo) return;
	if(molcasOptimization.numberOfIterations>0) return;

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.subProgram, NULL, "Ivo\n",-1);

	sprintf(buffer,"* Improve the virtuals for MCSCF\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);

}
/************************************************************************************************************/
static void putEndScfInTextEditor()
{
	/*
	sprintf(buffer,"*----------------------------------------------------------------\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	*/
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.program, &molcasColorBack.program, "End of input\n\n",-1);

	/* this action is done automatically - all grid & molden files are copied to submit directory.*/
	/*
	if(molcasOptimization.numberOfIterations<1)
	{
        	gchar buffer[BSIZE];
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n\n", -1);
		sprintf(buffer,"! cp $Project.scf.molden   $MOLCAS_SUBMIT_PWD/$Project.scf.molden\n");
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.shellCommand, &molcasColorBack.shellCommand, buffer, -1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n", -1);
	}
	*/
}
/************************************************************************************************************/
void putScfInfoInTextEditor()
{


	copyScfParameters(&molcasScf, &molcasScfTmp);
	putBeginScfInTextEditor();
	putTitleScfInTextEditor();
	putMethodScfInTextEditor();
	putChargeScfInTextEditor();
	putZSpinScfInTextEditor();
	putIterationsScfInTextEditor();
	putThresholdScfInTextEditor();
	putDiskScfInTextEditor();
	putIVOScfInTextEditor();
	putEndScfInTextEditor();
}
/************************************************************************************************************/
