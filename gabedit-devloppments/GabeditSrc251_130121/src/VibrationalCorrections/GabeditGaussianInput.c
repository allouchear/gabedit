/* Vibration.c */
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
#include <ctype.h>
#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "../Utils/UtilsInterface.h"
#include "../Files/FileChooser.h"
#include "../Files/FolderChooser.h"
#include "../Files/GabeditFolderChooser.h"
#include "../Common/Help.h"
#include "../Common/StockIcons.h"
#include "../Utils/GabeditTextEdit.h"

#define DEBUGFLAG 0

/************************************************************************************************************/
typedef struct _VibCorrectionsSigma
{
	gdouble tensor[3][3];
	gdouble isotropic;
	gdouble anisotropy;
}VibCorrectionsSigma;

typedef struct _VibCorrectionsAtom
{
	gchar* symbol;
	gdouble coordinates[3];
}VibCorrectionsAtom;
typedef struct _VibCorrectionsGeom
{
	gint numberOfAtoms;
	VibCorrectionsAtom* atoms;
}VibCorrectionsGeom;

typedef struct _VibCorrections
{
	gint numberOfFrequencies;
	gdouble* frequences;
	gdouble* calculatedFrequencies;
	gdouble* mass;
	gint nTemperatures;
	gdouble* temperatures;
	gdouble* akI;
	gdouble** sumF;
	gdouble delta;
	gdouble **energies;
	gdouble **F;
	VibCorrectionsGeom geom;
	VibCorrectionsSigma* sigmas;
	VibCorrectionsSigma** sigmasVC;
	gdouble **spinspins;
	gdouble ***spinspinsVC;
	gboolean centrifugeFlag;
}VibCorrections;
/************************************************************************************************************/
static void initVibCorrections(VibCorrections* vibCorrections, gint nf, gint nT, gdouble* T)
{
	gint i;
	vibCorrections->numberOfFrequencies = nf;
	vibCorrections->frequences = NULL;
	vibCorrections->mass = NULL;
	vibCorrections->nTemperatures = 0;
	vibCorrections->temperatures = NULL;
	vibCorrections->akI = NULL;
	vibCorrections->sumF = NULL;
	vibCorrections->energies = NULL;
	vibCorrections->F = NULL;
	if(nf<=0) return;
	vibCorrections->frequences = g_malloc(nf*sizeof(gdouble));
	vibCorrections->calculatedFrequencies = g_malloc(nf*sizeof(gdouble));
	vibCorrections->mass = g_malloc(nf*sizeof(gdouble));

	vibCorrections->nTemperatures = nT;
	vibCorrections->temperatures = g_malloc(vibCorrections->nTemperatures*sizeof(gdouble));
	for(i=0;i<vibCorrections->nTemperatures;i++) vibCorrections->temperatures[i] = T[i];
	vibCorrections->sumF = g_malloc(vibCorrections->nTemperatures*sizeof(gdouble*));
	for(i=0;i<vibCorrections->nTemperatures;i++) vibCorrections->sumF[i]= g_malloc(nf*sizeof(gdouble));

	vibCorrections->akI = g_malloc(nf*sizeof(gdouble));
	for(i=0;i<nf;i++) vibCorrections->akI[i] = 0;

	vibCorrections->energies = g_malloc((2*nf+1)*sizeof(gdouble*));
	for(i=0;i<2*nf+1;i++) vibCorrections->energies[i] = g_malloc((2*nf+1)*sizeof(gdouble));
	vibCorrections->F = g_malloc(nf*sizeof(gdouble*));
	for(i=0;i<nf;i++) vibCorrections->F[i] = g_malloc(nf*sizeof(gdouble));
	vibCorrections->geom.numberOfAtoms = 0;
	vibCorrections->geom.atoms = NULL;
	vibCorrections->sigmas = NULL;
	vibCorrections->sigmasVC = g_malloc(vibCorrections->nTemperatures*sizeof(VibCorrectionsSigma*));
	for(i=0;i<vibCorrections->nTemperatures;i++) vibCorrections->sigmasVC[i] = NULL;
	vibCorrections->spinspins = NULL;
	vibCorrections->spinspinsVC = g_malloc(vibCorrections->nTemperatures*sizeof(gdouble**));
	for(i=0;i<vibCorrections->nTemperatures;i++) vibCorrections->spinspinsVC[i] = NULL;
	vibCorrections->centrifugeFlag = TRUE;

}
/************************************************************************************************************/
static void freeVibCorrections(VibCorrections* vibCorrections)
{
	gint i;
	gint nf = vibCorrections->numberOfFrequencies;
	if( vibCorrections->frequences) g_free(vibCorrections->frequences);
	if( vibCorrections->calculatedFrequencies) g_free(vibCorrections->calculatedFrequencies);
	if( vibCorrections->mass) g_free(vibCorrections->mass);
	if( vibCorrections->temperatures) g_free(vibCorrections->temperatures);
	if( vibCorrections->akI) g_free(vibCorrections->akI);
	if( vibCorrections->sigmas) g_free(vibCorrections->sigmas);
	if( vibCorrections->sumF) 
	{
		for(i=0;i<vibCorrections->nTemperatures;i++) g_free(vibCorrections->sumF[i]);
		g_free(vibCorrections->sumF);
	}
	if( vibCorrections->energies) 
	{
		for(i=0;i<2*nf+1;i++) 
			if(vibCorrections->energies[i]) g_free(vibCorrections->energies[i]);
		g_free(vibCorrections->energies);
	}
	if(vibCorrections->F) 
	{
		for(i=0;i<nf;i++) 
			if(vibCorrections->F[i]) g_free(vibCorrections->F[i]);
		g_free(vibCorrections->F);
	}
	if(vibCorrections->geom.atoms) 
	{
		for(i=0;i<vibCorrections->geom.numberOfAtoms;i++) 
			if(vibCorrections->geom.atoms[i].symbol) g_free(vibCorrections->geom.atoms[i].symbol);
		g_free(vibCorrections->geom.atoms);
	}
	if(vibCorrections->sigmasVC)
	{
		gint t;
		for(t=0;t<vibCorrections->nTemperatures;t++) 
			if(vibCorrections->sigmasVC[t]) g_free(vibCorrections->sigmasVC[t]);
		g_free(vibCorrections->sigmasVC);
	}
	if(vibCorrections->spinspins)
	{
		for(i=0;i<vibCorrections->geom.numberOfAtoms;i++) 
			if(vibCorrections->spinspins[i]) g_free(vibCorrections->spinspins[i]);
		g_free(vibCorrections->spinspins);
	}
	if(vibCorrections->spinspinsVC)
	{
		gint t;
		for(t=0;t<vibCorrections->nTemperatures;t++) 
		{
			if(vibCorrections->spinspinsVC[t])
			{
				for(i=0;i<vibCorrections->geom.numberOfAtoms;i++) 
					if(vibCorrections->spinspinsVC[t][i]) 
						g_free(vibCorrections->spinspinsVC[t][i]);
				g_free(vibCorrections->spinspinsVC[t]);
			}
		}
		g_free(vibCorrections->spinspinsVC);
	}
	vibCorrections->centrifugeFlag = TRUE;
}
/********************************************************************************/
static void save_result(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;
	GtkWidget *textResult = NULL;
	gchar *temp;
	FILE *file;
	gint i;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	textResult = g_object_get_data (G_OBJECT (SelecFile), "TextResult");

	if(!textResult) return;

	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();
 
	file = FOpen(fileName, "wb");
	if(file == NULL)
	{
		Message(_("Sorry, I can not save file"),_("Error"),TRUE);
		return;
	}
	temp=gabedit_text_get_chars(textResult,0,-1);
	for(i=0;i<strlen(temp);i++)
		if(temp[i]=='\r') temp[i] = ' ';
	fprintf(file,"%s",temp);
	fclose(file);
	g_free(temp);
} 
/********************************************************************************/
static void save_result_dlg(GtkWidget *textResult, gpointer data)
{       
	GtkWidget* chooser = file_chooser_save(save_result,_("Save result"),GABEDIT_TYPEFILE_TXT,GABEDIT_TYPEWIN_OTHER);
	g_object_set_data (G_OBJECT (chooser), "TextResult",textResult);
	gtk_window_set_modal (GTK_WINDOW (chooser), TRUE);
}
/********************************************************************************/
static GtkWidget* createResultWindow(gchar* title)
{
	GtkWidget *resultWindow;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *vboxall;
	GtkWidget *button;
	GtkWidget *textWidget;

	resultWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(resultWindow),title);
	gtk_window_set_position(GTK_WINDOW(resultWindow),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(resultWindow),GTK_WINDOW(Fenetre));

	gtk_widget_realize(resultWindow);
	init_child(resultWindow,gtk_widget_destroy," Vib. Corr. ");
	g_signal_connect(G_OBJECT(resultWindow),"delete_event",(GCallback)destroy_children,NULL);

	gtk_container_set_border_width (GTK_CONTAINER (resultWindow), 4);
	vboxall = create_vbox(resultWindow);

	textWidget = create_text_widget(vboxall,NULL,&frame);
  	gabedit_text_set_editable (GABEDIT_TEXT (textWidget), TRUE); 
	set_font (textWidget,FontsStyleResult.fontname);
	set_base_style(textWidget,FontsStyleResult.BaseColor.red ,FontsStyleResult.BaseColor.green ,FontsStyleResult.BaseColor.blue);
	set_text_style(textWidget,FontsStyleResult.TextColor.red ,FontsStyleResult.TextColor.green ,FontsStyleResult.TextColor.blue);

	hbox = gtk_hbox_new (FALSE, 4);
	gtk_box_pack_start (GTK_BOX(vboxall), hbox, FALSE, FALSE, 1);
	gtk_box_set_homogeneous(GTK_BOX(hbox), FALSE);
	gtk_widget_realize(resultWindow);

	button = create_button(resultWindow,_("OK"));
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, FALSE, 1);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)destroy_children,G_OBJECT(resultWindow));
	gtk_widget_show (button);

	button = create_button(resultWindow,_("Save"));
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, FALSE, 1);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)save_result_dlg,G_OBJECT(textWidget));
	gtk_widget_show (button);

	gtk_window_set_default_size (GTK_WINDOW(resultWindow), 4*ScreenWidth/5, 4*ScreenHeight/5);
  	g_object_set_data (G_OBJECT (resultWindow), "TextWiddget", textWidget);
	return resultWindow;
}
/************************************************************************************************************/
static void showShieldingTensors(GtkWidget* textResult, VibCorrectionsGeom* geom, VibCorrectionsSigma* sigmas)
{
	gchar tmp[BSIZE];
	gint i;
	gint k,l;
	for(i=0;i<geom->numberOfAtoms;i++)
	{
		sprintf(tmp,"%s %f %f %f ",
				geom->atoms[i].symbol,
				geom->atoms[i].coordinates[0]*BOHR_TO_ANG,
				geom->atoms[i].coordinates[1]*BOHR_TO_ANG,
				geom->atoms[i].coordinates[2]*BOHR_TO_ANG);
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
		sprintf(tmp," Isotropic = %f Anisotropy = %f\n",sigmas[i].isotropic,sigmas[i].anisotropy);
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
		for(k=0;k<3;k++) 
		{
			for(l=0;l<3;l++) 
			{
				sprintf(tmp,"\t%f",sigmas[i].tensor[k][l]);
				gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
			}
			gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"\n",-1);   
		}
	}
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"\n",-1);   
}
/************************************************************************************************************/
static void showSpinSpins(GtkWidget* textResult, VibCorrectionsGeom* geom, gdouble** spinspins)
{
	gchar tmp[BSIZE];
	gint i,j;
	for(i=0;i<geom->numberOfAtoms;i++)
	for(j=i;j<geom->numberOfAtoms;j++)
	{
		sprintf(tmp,"%s[%d]  %s[%d] %f\n",
				geom->atoms[i].symbol,i+1,
				geom->atoms[j].symbol,j+1,
				spinspins[i][j]
		      );
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	}
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"\n",-1);   
}
/************************************************************************************************************/
static void showVibCorrections(VibCorrections* vibCorrections)
{
	gchar tmp[BSIZE];
	gint i,j;
	gint nf = vibCorrections->numberOfFrequencies;
	GtkWidget* resultWindow = createResultWindow("Vibrational corrections");
	GtkWidget* textResult = g_object_get_data (G_OBJECT (resultWindow), "TextWiddget");
	gint t;

	sprintf(tmp,"%s",
		"Vibrational corrections to Shielding, indirect nuclear spin-spin coupling constantes\n"
		"See Ruden et al. J. Chem. Phys. 118, 9572 (2003) and\n"
		);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	sprintf(tmp,"%s",
		"Calculation of NMR and EPR parameters: theory and applications\n"
		"By Martin Kaupp, Michael Buhl, Vladimir G. Malkin\n"
		"Published by Wiley-VCH, 2004,    ISBN 3527307796, 9783527307791\n"
		"Page 153\n"
		);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	sprintf(tmp,
		"====================================================================================\n"
		);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	if(!vibCorrections->centrifugeFlag)
	{
		sprintf(tmp,"%s",
		"Warnning : I cannot read the centrifugal coefficients from your gaussian output file\n"
		"           So the centrifugal correction is neglected.\n"
		);
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
		sprintf(tmp,
			"------------------------------------------------------------------------------------\n"
			);
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	}
	sprintf(tmp,"\nDisplacement = %f\n\n",vibCorrections->delta);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	/*
	sprintf(tmp,"Energies\n");
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	for(i=0;i<2*nf+1;i++) 
	{
		for(j=0;j<2*nf+1;j++) 
		{
			sprintf(tmp,"%0.10f ",vibCorrections->energies[i][j]);
			gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
			if((j+1)%4==0) 
			{
				sprintf(tmp,"\n");
				gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
			}
		}
		sprintf(tmp,"\n\n");
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	}
	*/
	sprintf(tmp,"Geometry(Angstrom)\n");
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	for(i=0;i<vibCorrections->geom.numberOfAtoms;i++) 
	{
		sprintf(tmp,"%s %f %f %f\n",
				vibCorrections->geom.atoms[i].symbol,
				vibCorrections->geom.atoms[i].coordinates[0]*BOHR_TO_ANG,
				vibCorrections->geom.atoms[i].coordinates[1]*BOHR_TO_ANG,
				vibCorrections->geom.atoms[i].coordinates[2]*BOHR_TO_ANG);
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	}
	sprintf(tmp,"\n");
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   

	sprintf(tmp,"i Freq(cm-1)  Calc.Freq   Mass(amu) CentCoef[amu^(-1/2) Bohr^(-1)]\n");
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	for(i=0;i<nf;i++) 
	{
		sprintf(tmp,"%d %f %f %f %f\n",i+1,
				vibCorrections->frequences[i], 
				vibCorrections->calculatedFrequencies[i], 
				vibCorrections->mass[i], vibCorrections->akI[i]);
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	}
	sprintf(tmp,"\n");
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	sprintf(tmp,"Cubic Force Const.[Hartree*amu(-3/2)*Bohr(-3)]\n");
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	sprintf(tmp,"i j k\tKijk\n");
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	for(i=0;i<nf;i++) 
		for(j=0;j<nf;j++) 
		{
			sprintf(tmp,"%d %d %d\t%f\n",i+1,i+1,j+1,vibCorrections->F[i][j]*AMU_TO_AU*sqrt(AMU_TO_AU));
			gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
		}
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"\n",-1);   
	if(vibCorrections->sigmas)
	{
		sprintf(tmp,"Shielding tensor at equilibrium geometry (SigmaEq)\n");
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
		showShieldingTensors(textResult, &vibCorrections->geom, vibCorrections->sigmas);
	}
	for(t=0;t<vibCorrections->nTemperatures;t++)
	{
		if(vibCorrections->sigmasVC[t])
		{
			if(fabs(vibCorrections->temperatures[t])<1e-6) sprintf(tmp,"Zero-point vibrational corrections to Sigma(ppm)\n");
			else sprintf(tmp,"Vibrational correction at T  = %f K\n",vibCorrections->temperatures[t]);
			gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
			showShieldingTensors(textResult, &vibCorrections->geom, vibCorrections->sigmasVC[t]);
		}
		if(vibCorrections->sigmas && vibCorrections->sigmasVC[t] && fabs(vibCorrections->temperatures[t])>1e-6)
		{
			VibCorrectionsSigma* sigmasDiff;
			sigmasDiff=g_malloc(vibCorrections->geom.numberOfAtoms*sizeof(VibCorrectionsSigma));
			for(j=0;j<vibCorrections->geom.numberOfAtoms;j++) 
			{
				gint k,l;
    				sigmasDiff[j].isotropic = vibCorrections->sigmasVC[t][j].isotropic-vibCorrections->sigmasVC[0][j].isotropic;
    				sigmasDiff[j].anisotropy = vibCorrections->sigmasVC[t][j].anisotropy-vibCorrections->sigmasVC[0][j].anisotropy;
				for(k=0;k<3;k++) 
				for(l=0;l<3;l++) 
    					sigmasDiff[j].tensor[k][l] = vibCorrections->sigmasVC[t][j].tensor[k][l]-vibCorrections->sigmasVC[0][j].tensor[k][l];
			}
			sprintf(tmp,"Sigma(T=%f) - Sigma(T=0)\n",vibCorrections->temperatures[t]);
			gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
			showShieldingTensors(textResult,&vibCorrections->geom, sigmasDiff);
			if(sigmasDiff) g_free(sigmasDiff);
		}
		if(vibCorrections->sigmas && vibCorrections->sigmasVC[t])
		{
			VibCorrectionsSigma* sigmasAll;
			sigmasAll=g_malloc(vibCorrections->geom.numberOfAtoms*sizeof(VibCorrectionsSigma));
			for(j=0;j<vibCorrections->geom.numberOfAtoms;j++) 
			{
				gint k,l;
    				sigmasAll[j].isotropic = vibCorrections->sigmas[j].isotropic+vibCorrections->sigmasVC[t][j].isotropic;
    				sigmasAll[j].anisotropy = vibCorrections->sigmas[j].anisotropy+vibCorrections->sigmasVC[t][j].anisotropy;
				for(k=0;k<3;k++) 
				for(l=0;l<3;l++) 
    					sigmasAll[j].tensor[k][l] = vibCorrections->sigmas[j].tensor[k][l]+vibCorrections->sigmasVC[t][j].tensor[k][l];
			}
			if(fabs(vibCorrections->temperatures[t])<1e-6)
				sprintf(tmp,"Sigma  = SigmaEq + Zero-point vibrational corrections\n");
			else
				sprintf(tmp,"Sigma  = SigmaEq + vibrational corrections at T = %f\n",vibCorrections->temperatures[t]);
			gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
			showShieldingTensors(textResult,&vibCorrections->geom, sigmasAll);
			if(sigmasAll) g_free(sigmasAll);
		}
	}
	if(vibCorrections->spinspins)
	{
		sprintf(tmp,"Total nuclear spin-spin coupling J (Hz) at equilibrium geometry\n");
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
		showSpinSpins(textResult, &vibCorrections->geom, vibCorrections->spinspins);
	}
	for(t=0;t<vibCorrections->nTemperatures;t++)
	{
		if(vibCorrections->spinspinsVC[t])
		{
			if(fabs(vibCorrections->temperatures[t])<1e-6)
			sprintf(tmp,"Zero-point vibrational corrections to total nuclear spin-spin coupling J (Hz)\n");
			else
			sprintf(tmp,"Vibrational corrections at T= %f, to total nuclear spin-spin coupling J (Hz)\n",vibCorrections->temperatures[t]);
			gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);
			showSpinSpins(textResult, &vibCorrections->geom, vibCorrections->spinspinsVC[t]);
		}
		if(vibCorrections->spinspins &&vibCorrections->spinspinsVC[t] && fabs(vibCorrections->temperatures[t])>1e-6)
		{
			gdouble** spinspinsDiff;
			gint nAtoms = vibCorrections->geom.numberOfAtoms;
    			spinspinsDiff=g_malloc(nAtoms*sizeof(gdouble*));
			for(i=0;i<nAtoms;i++) 
			{
				spinspinsDiff[i]=g_malloc(nAtoms*sizeof(gdouble));
				for(j=0;j<nAtoms;j++) 
					spinspinsDiff[i][j]= vibCorrections->spinspinsVC[t][i][j]-vibCorrections->spinspinsVC[0][i][j];
			}

			sprintf(tmp,"J(Hz at T=%f) - J(Hz at T=0)\n",vibCorrections->temperatures[t]);
			gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
			showSpinSpins(textResult,&vibCorrections->geom, spinspinsDiff);
			for(i=0;i<nAtoms;i++) g_free(spinspinsDiff[i]);
			g_free(spinspinsDiff);
		}
		if(vibCorrections->spinspins &&vibCorrections->spinspinsVC[t])
		{
			gdouble** spinspinsAll;
			gint nAtoms = vibCorrections->geom.numberOfAtoms;
    			spinspinsAll=g_malloc(nAtoms*sizeof(gdouble*));
			for(i=0;i<nAtoms;i++) 
			{
				spinspinsAll[i]=g_malloc(nAtoms*sizeof(gdouble));
				for(j=0;j<nAtoms;j++) 
					spinspinsAll[i][j]= vibCorrections->spinspins[i][j]+vibCorrections->spinspinsVC[t][i][j];
			}

			if(fabs(vibCorrections->temperatures[t])<1e-6)
				sprintf(tmp,"J(Hz) = JEq + Zero-point vibrational corrections\n");
			else
				sprintf(tmp,"J(Hz) = JEq + Vibrational corrections at T= %f\n",vibCorrections->temperatures[t]);
			gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
			showSpinSpins(textResult,&vibCorrections->geom, spinspinsAll);
			for(i=0;i<nAtoms;i++) g_free(spinspinsAll[i]);
			g_free(spinspinsAll);
		}
	}
	gtk_widget_show_all(resultWindow);
}
/************************************************************************************************************/
static void printShieldingTensors(VibCorrectionsGeom* geom, VibCorrectionsSigma* sigmas)
{
	gint i;
	gint k,l;
	for(i=0;i<geom->numberOfAtoms;i++)
	{
		printf("%s %f %f %f ",
				geom->atoms[i].symbol,
				geom->atoms[i].coordinates[0]*BOHR_TO_ANG,
				geom->atoms[i].coordinates[1]*BOHR_TO_ANG,
				geom->atoms[i].coordinates[2]*BOHR_TO_ANG);
		printf(" Isotropic = %f Anisotropy = %f\n",sigmas[i].isotropic,sigmas[i].anisotropy);
		for(k=0;k<3;k++) 
		{
			for(l=0;l<3;l++) 
				printf("\t%f",sigmas[i].tensor[k][l]);
			printf("\n");
		}
	}
}
/************************************************************************************************************/
static void printSpinSpins(VibCorrectionsGeom* geom, gdouble** spinspins)
{
	gint i,j;
	for(i=0;i<geom->numberOfAtoms;i++)
	for(j=i;j<geom->numberOfAtoms;j++)
	{
		printf("%s[%d]  %s[%d] %f\n",
				geom->atoms[i].symbol,i+1,
				geom->atoms[j].symbol,j+1,
				spinspins[i][j]
		      );
	}
}
/************************************************************************************************************/
static void printVibCorrections(VibCorrections* vibCorrections)
{
	gint i,j;
	gint t;
	gint nf = vibCorrections->numberOfFrequencies;
	if(!vibCorrections->centrifugeFlag)
	{
		printf("------------------------------------------------------------------------------------\n");
		printf("%s",
		"Warnning : I cannot read the centrifugal coefficients from your gaussian output file\n"
		"           So the centrifugal correction is neglected.\n"
		);
		printf("------------------------------------------------------------------------------------\n");
	}
	printf("\nDisplacement = %f\n",vibCorrections->delta);
	printf("Energies\n");
	for(i=0;i<2*nf+1;i++) 
	{
		for(j=0;j<2*nf+1;j++) 
		{
			printf("%0.10f ",vibCorrections->energies[i][j]);
			if((j+1)%4==0) printf("\n");
		}
		printf("\n\n");
	}
	printf("Geometry(Angstrom)\n");
	for(i=0;i<vibCorrections->geom.numberOfAtoms;i++) 
		printf("%s %f %f %f\n",
				vibCorrections->geom.atoms[i].symbol,
				vibCorrections->geom.atoms[i].coordinates[0]*BOHR_TO_ANG,
				vibCorrections->geom.atoms[i].coordinates[1]*BOHR_TO_ANG,
				vibCorrections->geom.atoms[i].coordinates[2]*BOHR_TO_ANG);
	printf("\n");

	printf("i Freq(cm-1)  Calc.Freq   Mass(amu) CentCoef[amu^(-1/2) Bohr^(-1)]\n");
	for(i=0;i<nf;i++) 
		printf("%d %f %f %f %f\n",i+1,
				vibCorrections->frequences[i], 
				vibCorrections->calculatedFrequencies[i], 
				vibCorrections->mass[i], vibCorrections->akI[i]);
	printf("\n");
	printf("Cubic Force Const.[Hartree*amu(-3/2)*Bohr(-3)]\n");
	printf("i j k\tKijk\n");
	for(i=0;i<nf;i++) 
		for(j=0;j<nf;j++) 
			printf("%d %d %d\t%f\n",i+1,i+1,j+1,vibCorrections->F[i][j]*AMU_TO_AU*sqrt(AMU_TO_AU));
	if(vibCorrections->sigmas)
	{
		printf("Shielding tensor at equilibrium geometry (SigmaEq)\n");
		printShieldingTensors(&vibCorrections->geom, vibCorrections->sigmas);
	}
	for(t=0;t<vibCorrections->nTemperatures;t++)
	{
		if(vibCorrections->sigmasVC[t])
		{
			if(fabs(vibCorrections->temperatures[t])<1e-6) printf("Zero-point vibrational corrections to Sigma(ppm)\n");
			else printf("Vibrational correction at T  = %f K\n",vibCorrections->temperatures[t]);
			printShieldingTensors(&vibCorrections->geom, vibCorrections->sigmasVC[t]);
		}
		if(vibCorrections->sigmas && vibCorrections->sigmasVC[t] && fabs(vibCorrections->temperatures[t])>1e-6)
		{
			VibCorrectionsSigma* sigmasDiff;
			sigmasDiff=g_malloc(vibCorrections->geom.numberOfAtoms*sizeof(VibCorrectionsSigma));
			for(j=0;j<vibCorrections->geom.numberOfAtoms;j++) 
			{
				gint k,l;
    				sigmasDiff[j].isotropic = vibCorrections->sigmasVC[t][j].isotropic-vibCorrections->sigmasVC[0][j].isotropic;
    				sigmasDiff[j].anisotropy = vibCorrections->sigmasVC[t][j].anisotropy-vibCorrections->sigmasVC[0][j].anisotropy;
				for(k=0;k<3;k++) 
				for(l=0;l<3;l++) 
    					sigmasDiff[j].tensor[k][l] = vibCorrections->sigmasVC[t][j].tensor[k][l]-vibCorrections->sigmasVC[0][j].tensor[k][l];
			}
			printf("Sigma(T=%f) - Sigma(T=0)\n",vibCorrections->temperatures[t]);
			printShieldingTensors(&vibCorrections->geom, sigmasDiff);
			if(sigmasDiff) g_free(sigmasDiff);
		}
		if(vibCorrections->sigmas && vibCorrections->sigmasVC[t])
		{
			VibCorrectionsSigma* sigmasAll;
			sigmasAll=g_malloc(vibCorrections->geom.numberOfAtoms*sizeof(VibCorrectionsSigma));
			for(j=0;j<vibCorrections->geom.numberOfAtoms;j++) 
			{
				gint k,l;
    				sigmasAll[j].isotropic = vibCorrections->sigmas[j].isotropic+vibCorrections->sigmasVC[t][j].isotropic;
    				sigmasAll[j].anisotropy = vibCorrections->sigmas[j].anisotropy+vibCorrections->sigmasVC[t][j].anisotropy;
				for(k=0;k<3;k++) 
				for(l=0;l<3;l++) 
    					sigmasAll[j].tensor[k][l] = vibCorrections->sigmas[j].tensor[k][l]+vibCorrections->sigmasVC[t][j].tensor[k][l];
			}
			if(fabs(vibCorrections->temperatures[t])<1e-6)
				printf("Sigma  = SigmaEq + Zero-point vibrational corrections\n");
			else
				printf("Sigma  = SigmaEq + vibrational corrections at T = %f\n",vibCorrections->temperatures[t]);
			printShieldingTensors(&vibCorrections->geom, sigmasAll);
			if(sigmasAll) g_free(sigmasAll);
		}
	}
	if(vibCorrections->spinspins)
	{
		printf("Total nuclear spin-spin coupling J (Hz) at equilibrium geometry\n");
		printSpinSpins(&vibCorrections->geom, vibCorrections->spinspins);
	}
	for(t=0;t<vibCorrections->nTemperatures;t++)
	{
		if(vibCorrections->spinspinsVC[t])
		{
			if(fabs(vibCorrections->temperatures[t])<1e-6)
			printf("Zero-point vibrational corrections to total nuclear spin-spin coupling J (Hz)\n");
			else
			printf("Vibrational corrections at T= %f, to total nuclear spin-spin coupling J (Hz)\n",vibCorrections->temperatures[t]);
			printSpinSpins(&vibCorrections->geom, vibCorrections->spinspinsVC[t]);
		}
		if(vibCorrections->spinspins &&vibCorrections->spinspinsVC[t] && fabs(vibCorrections->temperatures[t])>1e-6)
		{
			gdouble** spinspinsDiff;
			gint nAtoms = vibCorrections->geom.numberOfAtoms;
    			spinspinsDiff=g_malloc(nAtoms*sizeof(gdouble*));
			for(i=0;i<nAtoms;i++) 
			{
				spinspinsDiff[i]=g_malloc(nAtoms*sizeof(gdouble));
				for(j=0;j<nAtoms;j++) 
					spinspinsDiff[i][j]= vibCorrections->spinspinsVC[t][i][j]-vibCorrections->spinspinsVC[0][i][j];
			}

			printf("J(Hz at T=%f) - J(Hz at T=0)\n",vibCorrections->temperatures[t]);
			printSpinSpins(&vibCorrections->geom, spinspinsDiff);
			for(i=0;i<nAtoms;i++) g_free(spinspinsDiff[i]);
			g_free(spinspinsDiff);
		}
		if(vibCorrections->spinspins &&vibCorrections->spinspinsVC[t])
		{
			gdouble** spinspinsAll;
			gint nAtoms = vibCorrections->geom.numberOfAtoms;
    			spinspinsAll=g_malloc(nAtoms*sizeof(gdouble*));
			for(i=0;i<nAtoms;i++) 
			{
				spinspinsAll[i]=g_malloc(nAtoms*sizeof(gdouble));
				for(j=0;j<nAtoms;j++) 
					spinspinsAll[i][j]= vibCorrections->spinspins[i][j]+vibCorrections->spinspinsVC[t][i][j];
			}

			if(fabs(vibCorrections->temperatures[t])<1e-6)
				printf("J(Hz) = JEq + Zero-point vibrational corrections\n");
			else
				printf("J(Hz) = JEq + Vibrational corrections at T= %f\n",vibCorrections->temperatures[t]);
			printSpinSpins(&vibCorrections->geom, spinspinsAll);
			for(i=0;i<nAtoms;i++) g_free(spinspinsAll[i]);
			g_free(spinspinsAll);
		}
	}
}
/************************************************************************************************************/
gboolean getValueFromLine(gchar* str, gchar* label, gchar* tag, gdouble* value)
{
	gchar* capStr = NULL;
	gchar* capLabel = NULL;
	gchar* capTag = NULL;
	gchar* posLabel = NULL;
	*value = 0;
	if(!str) return FALSE;
	if(!label) return FALSE;
	capStr = g_strdup(str);
	capLabel = g_strdup(label);
	capTag = g_strdup(tag);
	uppercase(capStr);
	uppercase(capLabel);
	uppercase(capTag);
	posLabel = strstr(capStr,capLabel);
	if(posLabel && strstr(posLabel,capTag))
	{
		gchar* posTag = strstr(posLabel,capTag);
		*value = atof(posTag+strlen(capTag));
		if(capTag) g_free(capTag);
		if(capStr) g_free(capStr);
		if(capLabel) g_free(capLabel);
		return TRUE;
	}
	if(capTag) g_free(capTag);
	if(capStr) g_free(capStr);
	if(capLabel) g_free(capLabel);
	return FALSE;
}
/*************************************************************************************************************/
gint getNumberOfFrequencies(gchar* fileName)
{
	gchar t[BSIZE];
	FILE* file;
 	gint nf = 0;

 	file = FOpen(fileName, "rb"); 
        if(!file) return -1;
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file))break;
		if(strstr(t,"------------------------------------------"))
		{
			if(!fgets(t,BSIZE,file))break;
			uppercase(t);
			if(strstr(t,"MODE") && strstr(t,"MASS") && strstr(t,"QEQ +")) nf++;
		}
	}
	fclose(file);
	return nf;
}
/************************************************************************************************************/
static gboolean readFrequenciesMassDelta(gchar* fileName, VibCorrections* vibCorrections)
{
	gchar t[BSIZE];
	FILE* file;
 	gint nf = 0;
	gboolean Ok = TRUE;
	gdouble d = 0;
	

 	file = FOpen(fileName, "rb"); 
        if(!file) return -1;
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file))break;
		if(strstr(t,"------------------------------------------"))
		{
			if(!fgets(t,BSIZE,file))break;
			uppercase(t);
			if(strstr(t,"MODE") && strstr(t,"MASS") && strstr(t,"QEQ +")) 
			{
				Ok =getValueFromLine(t,"AKI","=",&d);
				if(Ok) vibCorrections->akI[nf] = d;
				else vibCorrections->centrifugeFlag = FALSE;
				Ok =getValueFromLine(t,"FREQ","=",&vibCorrections->frequences[nf]);
				if(!Ok) break;
				Ok =getValueFromLine(t,"MASS","=",&vibCorrections->mass[nf]);
				if(!Ok) break;
				Ok =getValueFromLine(t,"QEQ","+",&d);
				if(!Ok) break;
				if(nf==0) vibCorrections->delta = d;
				else if(fabs(vibCorrections->delta - d)>1e-8) 
				{
					Ok = FALSE;
					break;
				}
				nf++;
				if(nf>=vibCorrections->numberOfFrequencies) break;
			}
		}
	}
	fclose(file);
	return Ok;
}
/************************************************************************************************************/
static gboolean readEnergies(gchar* fileName, VibCorrections* vibCorrections)
{
	gchar t[BSIZE];
	FILE* file;
 	gint nf = vibCorrections->numberOfFrequencies;
 	gint twonf = 2*nf;
 	gint twonfp1 = twonf+1;
	gboolean Ok = TRUE;
	gint i = 0;
	gint j = 0;
	gint nAll = 0;
	

 	file = FOpen(fileName, "rb"); 
        if(!file) return FALSE;
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file))break;
		if(strstr(t,"E(") && strstr(t,"SCF Done"))
		{
			Ok =getValueFromLine(t,"E(","=",&vibCorrections->energies[i][j]);
			if(!Ok) break;
			nAll++;
			j++;
			if(nAll == twonfp1+twonf*twonf) break;
			if(nAll == twonfp1)
			{
				for(i=1;i<twonfp1;i++)
					vibCorrections->energies[i][0] = vibCorrections->energies[0][i];
				i = 1;
				j = 1;
			}
			else if(nAll>twonfp1 && (nAll-twonfp1)%twonf==0)
			{
				i++;
				j = 1; 
			}
		}
	}
	if(nAll != twonfp1+twonf*twonf) Ok = FALSE;
	fclose(file);
	return Ok;
}
/********************************************************************************/
static gboolean read_gaussian_file_geomi_str(VibCorrectionsGeom* geom, gchar *fileName,gint num,gchar* str)
{
 	gchar t[BSIZE];
 	gboolean OK;
 	gchar tmpStr[5][100];
 	FILE *fd;
 	guint idummy;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numgeom;
 	guint itype=0;

 	fd = FOpen(fileName, "rb");
	if(!fd) return FALSE;

	numgeom =1;
 	do 
 	{
 		OK=FALSE;
 		while(!feof(fd))
		{
	  		if(!fgets(t,BSIZE,fd))break;
	 		if(strstr( t,str))
	  		{
	  			if(!fgets(t,BSIZE,fd))break;
	  			if(!fgets(t,BSIZE,fd))break;
	  			if(!fgets(t,BSIZE,fd))break;
				if(strstr( t, "Type" )) itype=1;
				else itype=0;
	  			if(!fgets(t,BSIZE,fd))break;
                		numgeom++;
				OK = TRUE;
				break;
	  		}
		}
 		if(!OK && (numgeom == 1) )
		{
 			fclose(fd);
			return FALSE;
		}
 		if(!OK)break;

  		j=-1;
  		while(!feof(fd) )
  		{
    			if(!fgets(t,BSIZE,fd))break;
    			if(strstr( t, "----------------------------------" ))break;
    			j++;
    			if(geom->atoms==NULL) geom->atoms=g_malloc(sizeof(VibCorrectionsAtom));
    			else geom->atoms=g_realloc(geom->atoms,(j+1)*sizeof(VibCorrectionsAtom));

    			if(itype==0) sscanf(t,"%d %s %s %s %s",&idummy,tmpStr[0],tmpStr[1],tmpStr[2],tmpStr[3]);
    			else sscanf(t,"%d %s %d %s %s %s",&idummy,tmpStr[0],&idummy,tmpStr[1],tmpStr[2],tmpStr[3]);

			tmpStr[0][0]=toupper(tmpStr[0][0]);
	 		l=strlen(tmpStr[0]);
          		if (l>1) tmpStr[0][1]=tolower(tmpStr[0][1]);

    			geom->atoms[j].symbol=g_strdup(symb_atom_get((guint)atoi(tmpStr[0])));
    			for(i=0;i<3;i++)
    				geom->atoms[j].coordinates[i]=atof(ang_to_bohr(tmpStr[i+1]));

  		}
		if(num >0 && (gint)numgeom-1 == num) break;
 	}while(!feof(fd));

 	geom->numberOfAtoms = j+1;
 	fclose(fd);
 	if(geom->numberOfAtoms == 0 ) 
	{
		if(geom->atoms) g_free(geom->atoms);
		geom->atoms = NULL;
		return FALSE;
	}
	return TRUE;
}
/********************************************************************************/
static gboolean read_gaussian_file_geomi(VibCorrectionsGeom* geom, gchar *fileName,gint num)
{
	if(read_gaussian_file_geomi_str(geom, fileName,num,"Standard orientation:")) return TRUE;
	if(read_gaussian_file_geomi_str(geom, fileName,num,"Input orientation:")) return TRUE;
	/* for calculation with nosym option */
	if(!read_gaussian_file_geomi_str(geom, fileName,num,"Z-Matrix orientation:"))
	{
  		Message(_("Sorry\nI can not read geometry in this file"),_("Error"),TRUE);
		return FALSE;
	}
	return TRUE;
}
/************************************************************************************************************/
static gdouble getOneTerm(gdouble p0, gdouble pp, gdouble pm, gdouble OneOverFourW, gdouble OneOverFourW2sumF, gdouble fm, gdouble f2m, gdouble fCent)
{
	gdouble d=0,d2=0;
	gdouble dCent=0;
	gdouble d1;
	d1 = (pp-pm)*fm;
	dCent = fCent*d1;
	d = -d1*OneOverFourW2sumF;
	d2 = (pp+pm-2*p0)*f2m;
	d2 = d2*OneOverFourW;
	/*
	printf("pp=%f pm=%f s=%f\n",pp,pm,(pp+pm-2*p0)*f2m);
	*/
    	return d+d2+dCent;
}
/************************************************************************************************************/
static gboolean readLastShieldingTensors(FILE* file, VibCorrectionsSigma* sigmas, gint nAtoms)
{
	gchar t[BSIZE];
	gboolean Ok = TRUE;
	gchar* str = "shielding tensor (ppm)";
	gint i;

	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file)) return FALSE;
		if(strstr(t,str)) break;
	}
	if(!strstr(t,str)) return FALSE;
	for(i=0;i<nAtoms;i++)
	{
		if(!fgets(t,BSIZE,file))break;
		changeDInE(t);
		Ok =getValueFromLine(t,"Isotropic","=",&sigmas[i].isotropic);
		if(!Ok) break;
		Ok =getValueFromLine(t,"Anisotropy","=",&sigmas[i].anisotropy);
		if(!Ok) break;
		if(!fgets(t,BSIZE,file))break;
		Ok =getValueFromLine(t,"XX","=",&sigmas[i].tensor[0][0]);
		if(!Ok) break;
		Ok =getValueFromLine(t,"YX","=",&sigmas[i].tensor[1][0]);
		if(!Ok) break;
		Ok =getValueFromLine(t,"ZX","=",&sigmas[i].tensor[2][0]);
		if(!Ok) break;
		if(!fgets(t,BSIZE,file))break;
		Ok =getValueFromLine(t,"XY","=",&sigmas[i].tensor[0][1]);
		if(!Ok) break;
		Ok =getValueFromLine(t,"YY","=",&sigmas[i].tensor[1][1]);
		if(!Ok) break;
		Ok =getValueFromLine(t,"ZY","=",&sigmas[i].tensor[2][1]);
		if(!Ok) break;
		if(!fgets(t,BSIZE,file))break;
		Ok =getValueFromLine(t,"XZ","=",&sigmas[i].tensor[0][2]);
		if(!Ok) break;
		Ok =getValueFromLine(t,"YZ","=",&sigmas[i].tensor[1][2]);
		if(!Ok) break;
		Ok =getValueFromLine(t,"ZZ","=",&sigmas[i].tensor[2][2]);
		if(!Ok) break;
		if(!fgets(t,BSIZE,file))break;
	}
	if(i!=nAtoms) return FALSE;
	return Ok;
}
/************************************************************************************************************/
static gboolean readShieldingTensors(gchar* fileName,VibCorrections* vibCorrections)
{
	FILE* file;
	VibCorrectionsSigma* sigmas0;
	VibCorrectionsSigma* sigmasP;
	VibCorrectionsSigma* sigmasM;
	gint nAtoms = vibCorrections->geom.numberOfAtoms;
	gboolean Ok;
	gint nf = vibCorrections->numberOfFrequencies;
	gint i,j;
	gdouble f  = 2*vibCorrections->delta*sqrt(AMU_TO_AU);
	gdouble f2 = vibCorrections->delta*vibCorrections->delta*AMU_TO_AU;
	gint t;

	if(nAtoms<1) return FALSE;
 	file = FOpen(fileName, "rb"); 
        if(!file) return FALSE;
    	sigmas0=g_malloc(nAtoms*sizeof(VibCorrectionsSigma));
    	sigmasP=g_malloc(nAtoms*sizeof(VibCorrectionsSigma));
    	sigmasM=g_malloc(nAtoms*sizeof(VibCorrectionsSigma));
	Ok = readLastShieldingTensors(file, sigmas0, nAtoms);
	if(Ok)
	{
    		vibCorrections->sigmas=g_malloc(nAtoms*sizeof(VibCorrectionsSigma));
		for(j=0;j<nAtoms;j++) vibCorrections->sigmas[j]= sigmas0[j];


		for(t=0;t<vibCorrections->nTemperatures;t++) 
		{
    			vibCorrections->sigmasVC[t]=g_malloc(nAtoms*sizeof(VibCorrectionsSigma));
			for(j=0;j<nAtoms;j++) 
			{
				gint k,l;
    				vibCorrections->sigmasVC[t][j].isotropic = 0.0;
    				vibCorrections->sigmasVC[t][j].anisotropy = 0.0; 
				for(k=0;k<3;k++) 
				for(l=0;l<3;l++) 
    					vibCorrections->sigmasVC[t][j].tensor[k][l] = 0.0; 
			}
		}
	}
	if(Ok)
	for(i=0;i<nf;i++)
	{
		gdouble w =vibCorrections->frequences[i]/AU_TO_CM1; 
		gdouble w2 =w*w;
		gdouble fm = 1/(f*sqrt(vibCorrections->mass[i]));
		gdouble f2m = 1/(f2*vibCorrections->mass[i]);
		Ok = readLastShieldingTensors(file, sigmasP, nAtoms);
		if(!Ok) break;
		Ok = readLastShieldingTensors(file, sigmasM, nAtoms);
		if(!Ok) break;
		if(DEBUGFLAG != 0)
		{
			printf("Sigma+ nf = %d\n",i+1);
			printShieldingTensors(&vibCorrections->geom, sigmasP);
			printf("Sigma- nf = %d\n",i+1);
			printShieldingTensors(&vibCorrections->geom, sigmasM);
		}
		for(t=0;t<vibCorrections->nTemperatures;t++) 
		{
			gdouble OneOverFourW2sumF = 1.0/4/w2*vibCorrections->sumF[t][i];
			gdouble kT = 3.16681552e-06*vibCorrections->temperatures[t];
			gdouble fCent = kT*vibCorrections->akI[i]/w2/2/sqrt(AMU_TO_AU);
			gdouble OneOverFourW = 1.0/4/w;
			if(fabs(vibCorrections->temperatures[t])>1e-6)
			{
				gdouble x = w/2/kT;
				OneOverFourW *= cosh(x)/sinh(x);
			}
			for(j=0;j<nAtoms;j++) 
			{
				gint k,l;
    				vibCorrections->sigmasVC[t][j].isotropic += 
				getOneTerm(sigmas0[j].isotropic, sigmasP[j].isotropic, sigmasM[j].isotropic, 
				OneOverFourW, OneOverFourW2sumF, fm, f2m,fCent);
    				vibCorrections->sigmasVC[t][j].anisotropy += 
				getOneTerm(sigmas0[j].anisotropy, sigmasP[j].anisotropy, sigmasM[j].anisotropy, 
				OneOverFourW, OneOverFourW2sumF, fm, f2m,fCent);
				for(k=0;k<3;k++) 
				for(l=0;l<3;l++) 
    					vibCorrections->sigmasVC[t][j].tensor[k][l] += 
					getOneTerm(sigmas0[j].tensor[k][l], sigmasP[j].tensor[k][l], 
						sigmasM[j].tensor[k][l], OneOverFourW, OneOverFourW2sumF, fm, f2m,fCent);
			}
		}
	}
	g_free(sigmas0);
	g_free(sigmasP);
	g_free(sigmasM);
	fclose(file);
	return TRUE;
}
/************************************************************************************************************/
static gboolean readLastSpinSpins(FILE* file, gdouble** spinspins, gint nAtoms)
{
	gchar t[BSIZE];
	gboolean Ok = TRUE;
	gchar* str = "Total nuclear spin-spin coupling J (Hz):";
	gint nBlock = nAtoms/5;
	gint rest = nAtoms%5;
	gint i,j,k,l,a,b,nl,ib;
	gdouble A[5];

	if(rest!=0) nBlock++;

	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file)) return FALSE;
		if(strstr(t,str)) break;
	}
	if(!strstr(t,str)) return FALSE;
	for(ib=0;Ok && ib<nBlock;ib++)
	{
		if(!fgets(t,BSIZE,file)) /* num atoms */
		{
			Ok = FALSE;
			break; 
		}
		b =atoi(t);
		nl = nAtoms-ib*5;
		for(i=0;i<nl;i++)
		{
			if(!fgets(t,BSIZE,file))
			{
				Ok = FALSE;
				break; 
			}
			l = strlen(t);
			for(j=0;j<l;j++) if(t[j]=='D') t[j] = 'e';
			k = sscanf(t,"%d %lf %lf %lf %lf %lf", &a, &A[0],&A[1],&A[2],&A[3],&A[4]);
			for(j=0;j<k-1;j++)
			{
				spinspins[a-1][b+j-1]=A[j];
				spinspins[b+j-1][a-1] = spinspins[a-1][b+j-1];
			}
		}
	}
	if(!Ok) return FALSE;
	return Ok;
}
/************************************************************************************************************/
static gboolean readSpinSpins(gchar* fileName,VibCorrections* vibCorrections)
{
	FILE* file;
	gdouble** spinspins0;
	gdouble** spinspinsP;
	gdouble** spinspinsM;
	gint nAtoms = vibCorrections->geom.numberOfAtoms;
	gboolean Ok;
	gint nf = vibCorrections->numberOfFrequencies;
	gint i,j;
	gdouble f  = 2*vibCorrections->delta*sqrt(AMU_TO_AU);
	gdouble f2 = vibCorrections->delta*vibCorrections->delta*AMU_TO_AU;
	gint t;

	if(nAtoms<1) return FALSE;
 	file = FOpen(fileName, "rb"); 
        if(!file) return FALSE;
    	spinspins0=g_malloc(nAtoms*sizeof(gdouble*));
	for(i=0;i<nAtoms;i++) spinspins0[i]=g_malloc(nAtoms*sizeof(gdouble));
    	spinspinsP=g_malloc(nAtoms*sizeof(gdouble*));
	for(i=0;i<nAtoms;i++) spinspinsP[i]=g_malloc(nAtoms*sizeof(gdouble));
    	spinspinsM=g_malloc(nAtoms*sizeof(gdouble*));
	for(i=0;i<nAtoms;i++) spinspinsM[i]=g_malloc(nAtoms*sizeof(gdouble));

	Ok = readLastSpinSpins(file, spinspins0, nAtoms);
	if(Ok)
	{
    		vibCorrections->spinspins=g_malloc(nAtoms*sizeof(gdouble*));
		for(i=0;i<nAtoms;i++) 
		{
			vibCorrections->spinspins[i]=g_malloc(nAtoms*sizeof(gdouble));
			for(j=0;j<nAtoms;j++) 
				vibCorrections->spinspins[i][j]= spinspins0[i][j];
		}
		for(t=0;t<vibCorrections->nTemperatures;t++) 
		{
    			vibCorrections->spinspinsVC[t]=g_malloc(nAtoms*sizeof(gdouble*));
			for(i=0;i<nAtoms;i++) 
			{
				vibCorrections->spinspinsVC[t][i]=g_malloc(nAtoms*sizeof(gdouble));
				for(j=0;j<nAtoms;j++) 
					vibCorrections->spinspinsVC[t][i][j]= 0;
			}
		}
	}
	if(Ok)
	for(i=0;i<nf;i++)
	{
		gdouble w =vibCorrections->frequences[i]/AU_TO_CM1; 
		gdouble w2 =w*w;
		gdouble fm = 1/(f*sqrt(vibCorrections->mass[i]));
		gdouble f2m = 1/(f2*vibCorrections->mass[i]);
		Ok = readLastSpinSpins(file, spinspinsP, nAtoms);
		if(!Ok) break;
		Ok = readLastSpinSpins(file, spinspinsM, nAtoms);
		if(!Ok) break;
		for(t=0;t<vibCorrections->nTemperatures;t++) 
		{
			gdouble OneOverFourW2sumF = 1.0/4/w2*vibCorrections->sumF[t][i];
			gdouble kT = 3.16681552e-06*vibCorrections->temperatures[t];
			gdouble fCent = kT*vibCorrections->akI[i]/w2/2/sqrt(AMU_TO_AU);
			gdouble OneOverFourW = 1.0/4/w;
			if(fabs(vibCorrections->temperatures[t])>1e-6)
			{
				gdouble x = w/2/kT;
				OneOverFourW *= cosh(x)/sinh(x);
			}
			for(j=0;j<nAtoms;j++) 
			{
				gint k;
				for(k=0;k<nAtoms;k++) 
    					vibCorrections->spinspinsVC[t][j][k] += 
					getOneTerm(spinspins0[j][k], spinspinsP[j][k], spinspinsM[j][k], 
					OneOverFourW, OneOverFourW2sumF, fm, f2m,fCent);
			}
		}
	}
	for(i=0;i<nAtoms;i++) g_free(spinspins0[i]);
	g_free(spinspins0);
	for(i=0;i<nAtoms;i++) g_free(spinspinsP[i]);
	g_free(spinspinsP);
	for(i=0;i<nAtoms;i++) g_free(spinspinsM[i]);
	g_free(spinspinsM);
	fclose(file);
	return TRUE;
}
/************************************************************************************************************/
static void computeFrequencies(VibCorrections* vibCorrections)
{
 	gint nf = vibCorrections->numberOfFrequencies;
	gdouble f = vibCorrections->delta*vibCorrections->delta*AMU_TO_AU;
	gint i;
	for(i=0;i<nf;i++)
	{
		vibCorrections->calculatedFrequencies[i]=sqrt(fabs((
		vibCorrections->energies[0][2*i+1]+
		vibCorrections->energies[0][2*i+2]-
		2*vibCorrections->energies[0][0])/
		(f*vibCorrections->mass[i])))*AU_TO_CM1;
	}
}
/************************************************************************************************************/
static void computeCubicForces(VibCorrections* vibCorrections)
{
	gdouble deriv0;  /* nul on well */
	gdouble derivP; 
	gdouble derivM; 
 	gint nf = vibCorrections->numberOfFrequencies;
	gdouble f  = 2*vibCorrections->delta*sqrt(AMU_TO_AU);
	gdouble f2 = vibCorrections->delta*vibCorrections->delta*AMU_TO_AU;
	gint i,j;
	if(nf<1) return;

	for(i=0;i<nf;i++)
	{
		gdouble f2m = 1/(f2*vibCorrections->mass[i]);
		for(j=0;j<nf;j++)
		{
			gdouble fm = 1/(f*sqrt(vibCorrections->mass[j]));
			deriv0 =(vibCorrections->energies[0][2*j+1]-vibCorrections->energies[0][2*j+2])*fm;
			derivP =(vibCorrections->energies[2*i+1][2*j+1]-vibCorrections->energies[2*i+1][2*j+2])*fm;
			derivM =(vibCorrections->energies[2*i+2][2*j+1]-vibCorrections->energies[2*i+2][2*j+2])*fm;
			vibCorrections->F[i][j] = (derivP+derivM-2*deriv0)*f2m;
		}
	}
}
/************************************************************************************************************/
static void computeSumCubicForces(VibCorrections* vibCorrections)
{
 	gint nf = vibCorrections->numberOfFrequencies;
	gint i,j,t;
	for(t=0;t<vibCorrections->nTemperatures;t++)
	{
		gdouble twokT = 2*3.16681552e-06*vibCorrections->temperatures[t];
		for(i=0;i<nf;i++)
		{
			vibCorrections->sumF[t][i] = 0;
			for(j=0;j<nf;j++)
			{
				gdouble w =vibCorrections->frequences[j]/AU_TO_CM1; 
				if(fabs(vibCorrections->temperatures[t])<1e-6)
					vibCorrections->sumF[t][i] += vibCorrections->F[j][i]/w;
				else
				{
					gdouble x = w/twokT;
					vibCorrections->sumF[t][i] += vibCorrections->F[j][i]/w*cosh(x)/sinh(x);
				}
			}

		}
	}
}
/********************************************************************************/
static gboolean read_gaussian_file(GabeditFileChooser *filesel, gint response_id)
{
	gint nf = 0;
	gchar* fileName = NULL;
	VibCorrections vibCorrections;
	gboolean Ok = TRUE;
	gint nT = 2;
	gdouble* T =NULL;
	GtkWidget* entryTemperatureBegin = NULL;
	GtkWidget* entryTemperatureN = NULL;
	GtkWidget* entryTemperatureStep = NULL;
	gdouble TBegin     = 300;
	gdouble TStep      = 10;
	gint nStep      = 1;
	gint i;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	fileName = gabedit_file_chooser_get_current_file(filesel);

	entryTemperatureBegin = g_object_get_data (G_OBJECT (filesel), "EntryTemperatureBegin");
	entryTemperatureN = g_object_get_data (G_OBJECT (filesel), "EntryTemperatureN");
	entryTemperatureStep = g_object_get_data (G_OBJECT (filesel), "EntryTemperatureStep");
	if(!entryTemperatureBegin) return FALSE;
	if(!entryTemperatureN) return FALSE;
	if(!entryTemperatureStep) return FALSE;
	TBegin     = atof(gtk_entry_get_text(GTK_ENTRY(entryTemperatureBegin)));
	if(TBegin<0) TBegin = -TBegin;
	nStep     = atoi(gtk_entry_get_text(GTK_ENTRY(entryTemperatureN)));
	if(nStep<0) nStep = 0;
	TStep     = atof(gtk_entry_get_text(GTK_ENTRY(entryTemperatureStep)));
	if(TStep<0) TStep = -TStep;
	nT = nStep +1;
	T = g_malloc(nT*sizeof(gdouble));
	T[0] = 0;
	for(i=1;i<=nStep;i++)
		T[i] = TBegin + TStep*(i-1);

	nf = getNumberOfFrequencies(fileName);
	if(nf<0) return FALSE;
	initVibCorrections(&vibCorrections,nf, nT,T);
	g_free(T);
	Ok = read_gaussian_file_geomi(&vibCorrections.geom, fileName,1);
	if(Ok) Ok = readFrequenciesMassDelta(fileName, &vibCorrections);
	if(Ok) Ok = readEnergies(fileName, &vibCorrections);
	if(Ok) computeFrequencies(&vibCorrections);
	if(Ok) computeCubicForces(&vibCorrections);
	if(Ok) computeSumCubicForces(&vibCorrections);
	if(Ok) readShieldingTensors(fileName,&vibCorrections);
	if(Ok) readSpinSpins(fileName,&vibCorrections);
	if(Ok && DEBUGFLAG != 0 ) printVibCorrections(&vibCorrections);
	if(Ok) showVibCorrections(&vibCorrections);
	freeVibCorrections(&vibCorrections);
	return TRUE;
}
/********************************************************************************/
void read_vibcorrection_gaussian_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gaussian_file,
			_("Read the Gaussian output file for a input file created by Gabedit"),
			GABEDIT_TYPEFILE_GAUSSIAN,GABEDIT_TYPEWIN_OTHER);
	GtkWidget* entryTemperatureBegin = gtk_entry_new();
	GtkWidget* entryTemperatureN = gtk_entry_new();
	GtkWidget* entryTemperatureStep = gtk_entry_new();
	GtkWidget* hbox = gtk_hbox_new(FALSE,1);
	GtkWidget* hsep1 = gtk_hseparator_new();
	GtkWidget* hsep2 = gtk_hseparator_new();
	GtkWidget* labelBegin = gtk_label_new(_("Temperatures (K) from : "));
	GtkWidget* labelN = gtk_label_new(_("       Number of values : "));
	GtkWidget* labelStep = gtk_label_new(_("     with a step(K) : "));

	gtk_entry_set_text(GTK_ENTRY(entryTemperatureBegin),"300.0");
	gtk_entry_set_text(GTK_ENTRY(entryTemperatureStep),"10.0");
	gtk_entry_set_text(GTK_ENTRY(entryTemperatureN),"1");

	gtk_box_pack_start (GTK_BOX (hbox), labelBegin, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), entryTemperatureBegin, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), labelN, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), entryTemperatureN, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), labelStep, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), entryTemperatureStep, FALSE, FALSE, 0); 

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG(filesel)->vbox), hsep1, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG(filesel)->vbox), hbox, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG(filesel)->vbox), hsep2, FALSE, FALSE, 0);
	gtk_widget_show_all(hsep1);
	gtk_widget_show_all(hsep2);
	gtk_widget_show_all(hbox);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	g_object_set_data (G_OBJECT (filesel), "EntryTemperatureBegin",entryTemperatureBegin);
	g_object_set_data (G_OBJECT (filesel), "EntryTemperatureN",entryTemperatureN);
	g_object_set_data (G_OBJECT (filesel), "EntryTemperatureStep",entryTemperatureStep);
}
