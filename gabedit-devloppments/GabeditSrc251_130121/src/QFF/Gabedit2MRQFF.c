/* Gabedit2MRTWOMRQFF.c */
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
typedef struct _TWOMRQFFDipole
{
	gdouble values[3];
}TWOMRQFFDipole;

typedef struct _TWOMRQFFAtom
{
	gchar* symbol;
	gdouble coordinates[3];
}TWOMRQFFAtom;

typedef struct _TWOMRQFFGeom
{
	gint numberOfAtoms;
	TWOMRQFFAtom* atoms;
}TWOMRQFFGeom;

typedef struct _TWOMRQFF
{
	gint numberOfFrequencies;
	gdouble* gradients;
	gdouble* frequencies;
	gint numberOfFirstDipolesInput;
	gint numberOfEnergies;
	gint numberOfDipoles;
	gdouble** firstDipolesInput;
	gdouble* calculatedFrequencies;
	gdouble* mass;
	gdouble* delta;//[i:0..Nfreq]
	gdouble V0;
	gdouble **VI; // [i:0..Nfreq][0..5]
	gdouble ***VIJ; // [i:0..Nfreq][j:0..Nfreq][0..3]
	gdouble ***VI3J; // [i:0..Nfreq][j:0..Nfreq][0..3]
	gdouble** cubicEnergies;//[i:0..Nfreq][j:0..Nfreq] for tiii, tiij
	gdouble** quarticEnergiesIIJJ;//[i:0..Nfreq][j:0..i-1] for uiijj
	gdouble** quarticEnergiesIIIJ;//[i:0..Nfreq][j:0..Nfreq] for uiiij and uiiii

	gdouble *dipole0; // [0..2] // 0..2 : x,y,z
	gdouble ***dipolesI; // mu[i:0..Nfreq][0..5][0..2] // 0..2 : x,y,z
	gdouble ****dipolesIJ; // mu[i:0..Nfreq][j:0..Nfreq][0..3][0..2] // 0..2 : x,y,z
	gdouble ****dipolesI3J; // mu[i:0..Nfreq][j:0..Nfreq][0..3][0..2] // 0..2 : x,y,z
	gdouble** firstDipoles;//[i:0..Nfreq][0..2] 
	gdouble*** secondDipoles;//[i:0..Nfreq][i:0..Nfreq][0..2] 
	gdouble*** cubicDipoles;//[i:0..Nfreq][i:0..Nfreq][0..2] // diii, diij
	gdouble*** quarticDipolesIIJJ;//[i:0..Nfreq][j:0..i-1][0..2] for uiijj
	gdouble*** quarticDipolesIIIJ;//[i:0..Nfreq][j:0..Nfreq][0..2] for uiiij and uiiii

	TWOMRQFFGeom geom;
}TWOMRQFF;
/************************************************************************************************************/
static void initTWOMRQFF(TWOMRQFF* qffConstants, gint nf)
{
	gint i;
	qffConstants->numberOfFrequencies = nf;
	qffConstants->numberOfFirstDipolesInput = 0;
	qffConstants->numberOfEnergies = 0;
	qffConstants->numberOfDipoles = 0;

	qffConstants->frequencies = newVectorDouble(qffConstants->numberOfFrequencies);
        initVectorDouble(qffConstants->frequencies, qffConstants->numberOfFrequencies, 0.0);
	qffConstants->gradients = newVectorDouble(qffConstants->numberOfFrequencies);
        initVectorDouble(qffConstants->gradients, qffConstants->numberOfFrequencies, 0.0);
	qffConstants->calculatedFrequencies = newVectorDouble(qffConstants->numberOfFrequencies);
        initVectorDouble(qffConstants->calculatedFrequencies, qffConstants->numberOfFrequencies, 0.0);
	qffConstants->mass = newVectorDouble(qffConstants->numberOfFrequencies);
        initVectorDouble(qffConstants->mass, qffConstants->numberOfFrequencies, 0.0);
	qffConstants->delta = newVectorDouble(qffConstants->numberOfFrequencies);
        initVectorDouble(qffConstants->delta, qffConstants->numberOfFrequencies, 0.0);

	qffConstants->V0 = 0;
	qffConstants->VI = newMatrixDouble(qffConstants->numberOfFrequencies,6);
        initMatrixDouble(qffConstants->VI, qffConstants->numberOfFrequencies,6, 0.0);
	qffConstants->cubicEnergies = newMatrixDouble(qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies);
        initMatrixDouble(qffConstants->cubicEnergies, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies, 0.0);

	qffConstants->quarticEnergiesIIJJ = newMatrixDouble(qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies);
        initMatrixDouble(qffConstants->quarticEnergiesIIJJ, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies, 0.0);
	qffConstants->quarticEnergiesIIIJ = newMatrixDouble(qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies);
        initMatrixDouble(qffConstants->quarticEnergiesIIIJ, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies, 0.0);

	qffConstants->quarticDipolesIIJJ = newCubeDouble(qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies,3);
        initCubeDouble(qffConstants->quarticDipolesIIJJ, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies,3, 0.0);
	qffConstants->quarticDipolesIIIJ = newCubeDouble(qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies,3);
        initCubeDouble(qffConstants->quarticDipolesIIIJ, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies,3, 0.0);

	qffConstants->firstDipoles = newMatrixDouble(qffConstants->numberOfFrequencies,3);
        initMatrixDouble(qffConstants->firstDipoles, qffConstants->numberOfFrequencies,3, 0.0);

	qffConstants->firstDipolesInput = newMatrixDouble(qffConstants->numberOfFrequencies,3);
        initMatrixDouble(qffConstants->firstDipolesInput, qffConstants->numberOfFrequencies,3, 0.0);

        qffConstants->VIJ = newCubeDouble(qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies,4);
        initCubeDouble(qffConstants->VIJ, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies,4,0.0);
        qffConstants->VI3J = newCubeDouble(qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies,4);
        initCubeDouble(qffConstants->VI3J, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies,4,0.0);

        qffConstants->dipole0 = newVectorDouble(3);
        initVectorDouble(qffConstants->dipole0,3,0.0);

        qffConstants->dipolesI = newCubeDouble(qffConstants->numberOfFrequencies,6,3);
        initCubeDouble(qffConstants->dipolesI, qffConstants->numberOfFrequencies,6,3,0.0);

        qffConstants->secondDipoles = newCubeDouble(qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies,3);
        initCubeDouble(qffConstants->secondDipoles, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies,3,0.0);

        qffConstants->cubicDipoles = newCubeDouble(qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies,3);
        initCubeDouble(qffConstants->cubicDipoles, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies,3,0.0);

        qffConstants->dipolesIJ = newQuarticDouble(qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies,4,3);
        initQuarticDouble(qffConstants->dipolesIJ, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies,4,3,0.0);

        qffConstants->dipolesI3J = newQuarticDouble(qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies,4,3);
        initQuarticDouble(qffConstants->dipolesI3J, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies,4,3,0.0);

	qffConstants->geom.numberOfAtoms = 0;
	qffConstants->geom.atoms = NULL;

}
/************************************************************************************************************/
static void freeTWOMRQFF(TWOMRQFF* qffConstants)
{
	freeVectorDouble(&qffConstants->frequencies);
	freeVectorDouble(&qffConstants->gradients);
	freeVectorDouble(&qffConstants->calculatedFrequencies);
	freeVectorDouble(&qffConstants->mass);
	freeVectorDouble(&qffConstants->delta);
	freeMatrixDouble(&qffConstants->VI, qffConstants->numberOfFrequencies);
	freeMatrixDouble(&qffConstants->cubicEnergies, qffConstants->numberOfFrequencies);
	freeMatrixDouble(&qffConstants->quarticEnergiesIIJJ, qffConstants->numberOfFrequencies);
	freeMatrixDouble(&qffConstants->quarticEnergiesIIIJ, qffConstants->numberOfFrequencies);
	freeCubeDouble(&qffConstants->quarticDipolesIIJJ, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies);
	freeCubeDouble(&qffConstants->quarticDipolesIIIJ, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies);
	freeMatrixDouble(&qffConstants->firstDipoles, qffConstants->numberOfFrequencies);
	freeMatrixDouble(&qffConstants->firstDipolesInput, qffConstants->numberOfFrequencies);
	freeCubeDouble(&qffConstants->VIJ, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies);
	freeCubeDouble(&qffConstants->VI3J, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies);

	freeVectorDouble(&qffConstants->dipole0);
	freeCubeDouble(&qffConstants->dipolesI, qffConstants->numberOfFrequencies,6);
	freeCubeDouble(&qffConstants->secondDipoles, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies);
	freeCubeDouble(&qffConstants->cubicDipoles, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies);

	freeQuarticDouble(&qffConstants->dipolesIJ, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies,4);
	freeQuarticDouble(&qffConstants->dipolesI3J, qffConstants->numberOfFrequencies,qffConstants->numberOfFrequencies,4);
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
	textResult = g_object_get_data (G_OBJECT (SelecFile), "TextWidget");

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
	/* for(i=0;i<strlen(temp);i++) if(temp[i]=='\r') temp[i] = ' ';*/
	fprintf(file,"%s",temp);
	fclose(file);
	g_free(temp);
} 
/********************************************************************************/
static void save_result_dlg(GtkWidget *resultWindow, gpointer data)
{       
	TWOMRQFF* qffConstants;
	GtkWidget* chooser = file_chooser_save(save_result,_("Save result"),GABEDIT_TYPEFILE_TXT,GABEDIT_TYPEWIN_OTHER);
	GtkWidget* textResult = g_object_get_data (G_OBJECT (resultWindow), "TextWidget");
	qffConstants = g_object_get_data (G_OBJECT (resultWindow), "TWOMRQFFDATA");
	g_object_set_data (G_OBJECT (chooser), "TWOMRQFFDATA",qffConstants);
	g_object_set_data (G_OBJECT (chooser), "TextWidget",textResult);
	gtk_window_set_modal (GTK_WINDOW (chooser), TRUE);
}
/********************************************************************************/
static GtkWidget* destroy_result_window(GtkWidget *resultWindow)
{
	TWOMRQFF* qffConstants;
	if(!GTK_IS_WIDGET(resultWindow)) return NULL;
	qffConstants = g_object_get_data (G_OBJECT (resultWindow), "TWOMRQFFDATA");
	freeTWOMRQFF(qffConstants); 
	destroy_children(resultWindow);
	return NULL;
}
/********************************************************************************/
static GtkWidget* createResultWindow(gchar* title, TWOMRQFF* qffConstants)
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
	g_object_set_data (G_OBJECT (resultWindow), "TWOMRQFFDATA",qffConstants);

	gtk_widget_realize(resultWindow);
	init_child(resultWindow,gtk_widget_destroy," Vib. Corr. ");
	g_signal_connect(G_OBJECT(resultWindow),"delete_event",(GCallback)destroy_result_window,NULL);

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
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)destroy_result_window,G_OBJECT(resultWindow));
	gtk_widget_show (button);

	button = create_button(resultWindow,_("Save"));
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, FALSE, 1);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)save_result_dlg,G_OBJECT(resultWindow));
	gtk_widget_show (button);

	gtk_window_set_default_size (GTK_WINDOW(resultWindow), 4*ScreenWidth/5, 4*ScreenHeight/5);
  	g_object_set_data (G_OBJECT (resultWindow), "TextWidget", textWidget);
	return resultWindow;
}
/************************************************************************************************************/
static void showTWOMRQFF(TWOMRQFF* qffConstants)
{
	gchar tmp[BSIZE];
	gint i,j;
	gint nf = qffConstants->numberOfFrequencies;
	GtkWidget* resultWindow = createResultWindow("TWOMRQFF constants",qffConstants);
	GtkWidget* textResult = g_object_get_data (G_OBJECT (resultWindow), "TextWidget");
	gint t;
	gchar txyz[]={'X','Y','Z'};
	gint xyz;

	sprintf(tmp, "#====================================================================================\n");
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	sprintf(tmp,"%s",
		"# 2MR-QFF constants\n"
		"# See Yagi et al. J. Chem. Phys. 121, 1383 (2004)\n"
		);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	sprintf(tmp, "#====================================================================================\n");
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   

	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"\n",-1);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"VPT2Model=GVPT2\n",-1);   
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"# VPT2Model=DCPT2\n",-1);   
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"# VPT2Model=HDCPT2\n",-1);   
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"# alphaHDCPT2=1.0\n",-1);   
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"# betaHDCPT2=5e5\n",-1);   
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"\n",-1);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"PropModel=GVPT2\n",-1);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"# PropModel=HDCPT2\n",-1);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"# PropModel=DCPT2\n",-1);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"# alphaPropHDCPT2=1.0\n",-1);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"# betaPropHDCPT2=5e5\n",-1);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"# alphaPropHDCPT2=1.0\n",-1);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"# betaPropHDCPT2=5e5\n",-1);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"maxFrequencyDifferenceFermi=200\n",-1);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"MartinCutOff1=1.0\n",-1);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"MartinCutOff2=1.0\n",-1);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"# ZCutOff=0.08\n",-1);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"\n",-1);
	sprintf(tmp, "#====================================================================================\n");
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   

	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"\n",-1);   
	sprintf(tmp,"nFrequencies=%d\n",nf);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	sprintf(tmp,"nDim=%d\n",3);
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"\n",-1);   

	sprintf(tmp,"#i Freq(cm-1)  Calc.Freq   dQ(Bohr)  Mass(amu)\tGradient[ H amu^(-1/2) Bohr^(-1)]\n");
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"Hessian\n",-1);   
	for(i=0;i<nf;i++) 
	{
		sprintf(tmp,"%d %d %f %f %f %f\t%f\n",i+1, i+1, qffConstants->frequencies[i], 
				qffConstants->calculatedFrequencies[i], 
				qffConstants->delta[i], qffConstants->mass[i], qffConstants->gradients[i]);
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	}
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"END\n\n",-1);


	sprintf(tmp,"# i\tj\tk\tReduced values [cm-1]\n");
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"Cubic\n",-1);   
	for(i=0;i<nf;i++)
        {
                for(j=0;j<nf;j++)
                {
			if(fabs(qffConstants->cubicEnergies[i][j])<1e-12) continue;
			sprintf(tmp,"%d\t%d\t%d\t%14.6f\n",i+1, i+1, j+1, qffConstants->cubicEnergies[i][j]);
			gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
                }
        }
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"END\n\n",-1);


	sprintf(tmp,"# i\tj\tk\tl\tReduced values [cm-1]\n");
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"Quartic\n",-1);   
	for(i=0;i<nf;i++)
        {
		for(j=0;j<=i;j++)
		{
			if(fabs(qffConstants->quarticEnergiesIIJJ[i][j])<1e-12) continue;
			sprintf(tmp,"%d\t%d\t%d\t%d\t%14.6f\n",i+1, i+1, j+1, j+1, qffConstants->quarticEnergiesIIJJ[i][j]);
			gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
		}
		for(j=0;j<nf;j++)
		{
			if(j==i) continue;
			if(fabs(qffConstants->quarticEnergiesIIIJ[i][j])<1e-12) continue;
			sprintf(tmp,"%d\t%d\t%d\t%d\t%14.6f\n",i+1, i+1, i+1, j+1, qffConstants->quarticEnergiesIIIJ[i][j]);
			gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
		}
        }
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"END\n\n",-1);

	if(qffConstants->numberOfFirstDipolesInput==0)
	{
		sprintf(tmp,"#xyz\ti\tValues[au cm^1/2]\n");
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"First derivatives\n",-1);
		for(i=0;i<nf;i++)
		for(xyz=0;xyz<3;xyz++)
		{
			if(fabs(qffConstants->firstDipoles[i][xyz])<1e-12) continue;
			sprintf(tmp,"%c\t%d\t%14.6f\n",txyz[xyz], i+1,qffConstants->firstDipoles[i][xyz]);
			gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
		}
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"END\n\n",-1);
	}
	else
	{
		sprintf(tmp,"#xyz\ti\tInput values[au cm^1/2]\tCalculated values[au cm^1/2]\n");
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"First derivatives\n",-1);
		for(i=0;i<nf;i++)
		for(xyz=0;xyz<3;xyz++)
		{
			if(fabs(qffConstants->firstDipolesInput[i][xyz])<1e-12) continue;
			sprintf(tmp,"%c\t%d\t%14.6f\t\t%14.6f\n",txyz[xyz], i+1,qffConstants->firstDipolesInput[i][xyz], qffConstants->firstDipoles[i][xyz]);
			gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
		}
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"END\n\n",-1);
	}

	sprintf(tmp,"#xyz\ti\tj\tValues[au cm]\n");
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"Second derivatives\n",-1);
	for(i=0;i<nf;i++)
	for(j=0;j<nf;j++)
	for(xyz=0;xyz<3;xyz++)
	{
		if(fabs(qffConstants->secondDipoles[i][j][xyz])<1e-12) continue;
		sprintf(tmp,"%c\t%d\t%d\t%14.6f\n",txyz[xyz], i+1,j+1,qffConstants->secondDipoles[i][j][xyz]);
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	}
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"END\n\n",-1);

	sprintf(tmp,"#xyz\ti\tj\tk\tValues[au cm^3/2]\n");
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"Cubic derivatives\n",-1);
	for(i=0;i<nf;i++)
	for(j=0;j<nf;j++)
	for(xyz=0;xyz<3;xyz++)
	{
		if(fabs(qffConstants->cubicDipoles[i][j][xyz])<1e-12) continue;
		sprintf(tmp,"%c\t%d\t%d\t%d\t%14.6f\n",txyz[xyz], i+1,i+1,j+1,qffConstants->cubicDipoles[i][j][xyz]);
		gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
		if(i!=j)
		{
			sprintf(tmp,"%c\t%d\t%d\t%d\t%14.6f\n",txyz[xyz], i+1,j+1,i+1,qffConstants->cubicDipoles[i][j][xyz]);
			gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
			sprintf(tmp,"%c\t%d\t%d\t%d\t%14.6f\n",txyz[xyz], j+1,i+1,i+1,qffConstants->cubicDipoles[i][j][xyz]);
			gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,tmp,-1);   
		}
	}
	gabedit_text_insert (GABEDIT_TEXT(textResult), NULL, NULL, NULL,"END\n\n",-1);

	gtk_widget_show_all(resultWindow);
}
/************************************************************************************************************/
static void printTWOMRQFF(TWOMRQFF* qffConstants)
{
	gint i,j;
	gint t;
	gint nf = qffConstants->numberOfFrequencies;
	printf("nf = %d\n",nf);
	for(i=0;i<nf;i++) 
	{
		printf("%d %f %f %f %f\n",i+1,
				qffConstants->frequencies[i], 
				qffConstants->calculatedFrequencies[i], 
				qffConstants->delta[i], 
				qffConstants->mass[i]);
	}
}
/************************************************************************************************************/
static gboolean readFrequenciesInitTWOMRQFF(FILE* file, TWOMRQFF* qffConstants)
{
	gchar t[BSIZE];
 	gint nf = 0;
	gboolean Ok = TRUE;
	gdouble d = 0;
	gint nn=1;
	gdouble dum;
	gint i;
	

	if(!goToStr(file, "Frequencies"))
	{
		fprintf(stderr,"I cannot read the harmonic frequencies\nChech your input file\n");
		return FALSE;
	}
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file))break;
		nn = sscanf(t,"%lf",&dum);
		if(nn<1) break;
		nf++;
	}
	if(nf==0)
	{
		fprintf(stderr,"I cannot read the harmonic frequencies\nChech your input file\n");
		return FALSE;
	}
	initTWOMRQFF(qffConstants,nf);
	rewind(file);
	goToStr(file, "Frequencies");
	for(i=0;i<nf;i++)
	{
		if(!fgets(t,BSIZE,file))break;
		nn = sscanf(t,"%lf",&qffConstants->frequencies[i]);
		if(nn<1) break;
	}
	qffConstants->numberOfFirstDipolesInput = 0;
	if(nn==1)
	{
		rewind(file);
		if(goToStr(file, "First derivatives"))
		for(i=0;i<nf && nn==1 ;i++)
		{
			gint xyz;
			for(xyz=0;xyz<3 && nn==1 ;xyz++)
				nn = fscanf(file,"%lf",&qffConstants->firstDipolesInput[i][xyz]);
			qffConstants->numberOfFirstDipolesInput+= nn;
		}
		if(qffConstants->numberOfFirstDipolesInput != nf) qffConstants->numberOfFirstDipolesInput=0;
	}
	if(nn!=1) Ok = FALSE;
	return Ok;
}
/************************************************************************************************************/
static gboolean readVectorReal(FILE* file, gchar* tag, gint n, gdouble* values)
{
        gchar* TAG = NULL;
        int i=0;
        int ii;
        double v;
	int nn = 0;
        if(!tag) return FALSE;
        if(!values) return FALSE;

        TAG = g_strdup(tag);
        uppercase(TAG);
        rewind(file);
	if(!goToStr(file, TAG)) 
	{
		fprintf(stderr,"I cannot find %s in our file\n",tag);
		if(TAG) g_free(TAG);
		return FALSE;
	}
	for(i=0;i<n;i++)
        {
		nn = fscanf(file,"%lf",&values[i]);
                if(nn!=1) break;
        }
	if(i!=n)
	{
		fprintf(stderr,"I cannot read %s\nCheck  the number of values\n",tag);
		return FALSE;
	}
	return TRUE;
}
/************************************************************************************************************/
static gboolean readEnergies(FILE* file, TWOMRQFF* qffConstants)
{
	gint i = 0;
	gint j = 0;
	gint k = 0;
	int nn=1;
        gchar* TAG = NULL;
        TAG = g_strdup("ENERGIES");
        uppercase(TAG);
        rewind(file);
	if(!goToStr(file, TAG)) 
	{
		fprintf(stderr,"I cannot find %s in our file\n",TAG);
		if(TAG) g_free(TAG);
		return FALSE;
	}
	qffConstants->numberOfEnergies = 0;
	nn = fscanf(file,"%lf",&qffConstants->V0);
	qffConstants->numberOfEnergies += nn;
	for(i=0;i<qffConstants->numberOfFrequencies && nn==1;i++)
	for(k=0;k<6 && nn==1;k++)
        {
		nn = fscanf(file,"%lf",&qffConstants->VI[i][k]);
		qffConstants->numberOfEnergies += nn;
                if(nn!=1) break;
        }
	if(i!=qffConstants->numberOfFrequencies || k!=6)
	{
		fprintf(stderr,"I cannot read the 1MR energies\n");
		return FALSE;
	}

        for(j=0;j<qffConstants->numberOfFrequencies && nn==1;j++)
        {
                for(i=0;i<j && nn==1;i++)
                {
			for(k=0;k<4 && nn==1;k++)
        		{
				nn = fscanf(file,"%lf",&qffConstants->VIJ[j][i][k]);
				qffConstants->numberOfEnergies += nn;
                		if(nn!=1) break;
       	 		}
			if(nn==1)
			{
			qffConstants->VIJ[i][j][0] = qffConstants->VIJ[j][i][0];
			qffConstants->VIJ[i][j][1] = qffConstants->VIJ[j][i][2];
			qffConstants->VIJ[i][j][2] = qffConstants->VIJ[j][i][1];
			qffConstants->VIJ[i][j][3] = qffConstants->VIJ[j][i][3];
			}
                }
                for(i=0;i<qffConstants->numberOfFrequencies && nn==1;i++)
                {
                        if(i==j) continue;
			for(k=0;k<4 && nn==1;k++)
        		{
				nn = fscanf(file,"%lf",&qffConstants->VI3J[j][i][k]);
				qffConstants->numberOfEnergies += nn;
                		if(nn!=1) break;
       	 		}
		}
	}
	return TRUE;
}
/************************************************************************************************************/
static gboolean readDipoles(FILE* file, TWOMRQFF* qffConstants)
{
	gint i = 0;
	gint j = 0;
	gint k = 0;
	gint xyz = 0;
	int nn=1;
        gchar* TAG = NULL;
        TAG = g_strdup("DIPOLES");
        uppercase(TAG);
        rewind(file);
	if(!goToStr(file, TAG)) 
	{
		fprintf(stderr,"I cannot find %s in our file\n",TAG);
		if(TAG) g_free(TAG);
		return FALSE;
	}
	qffConstants->numberOfDipoles = 0;
	for(xyz=0;xyz<3 && nn==1;xyz++)
	{
		nn = fscanf(file,"%lf",&qffConstants->dipole0[xyz]);
		qffConstants->numberOfDipoles += nn;
	}

	if(nn!=1) 
	{
		fprintf(stderr,"Warning : I cannot read the dipole\n");
		if(TAG) g_free(TAG);
		return FALSE;
	}

	for(i=0;i<qffConstants->numberOfFrequencies && nn==1;i++)
	for(k=0;k<6 && nn==1;k++)
        {
		for(xyz=0;xyz<3 && nn==1;xyz++)
		{
			nn = fscanf(file,"%lf",&qffConstants->dipolesI[i][k][xyz]);
			qffConstants->numberOfDipoles += nn;
		}
        }
	if(i!=qffConstants->numberOfFrequencies || k!=6 || xyz!=3)
	{
		fprintf(stderr,"Warning : I cannot read the 1MR dipoles\n");
		if(TAG) g_free(TAG);
		return FALSE;
	}

        for(j=0;j<qffConstants->numberOfFrequencies && nn==1;j++)
        {
                for(i=0;i<j && nn==1;i++)
                {
			for(k=0;k<4 && nn==1;k++)
        		{
				for(xyz=0;xyz<3 && nn==1;xyz++)
				{
					nn = fscanf(file,"%lf",&qffConstants->dipolesIJ[j][i][k][xyz]);
					qffConstants->numberOfDipoles += nn;
                			if(nn!=1) break;
				}
       	 		}
			if(nn==1)
			{
				for(xyz=0;xyz<3 && nn==1;xyz++)
				{
					qffConstants->dipolesIJ[i][j][0][xyz] = qffConstants->dipolesIJ[j][i][0][xyz];
					qffConstants->dipolesIJ[i][j][1][xyz] = qffConstants->dipolesIJ[j][i][2][xyz];
					qffConstants->dipolesIJ[i][j][2][xyz] = qffConstants->dipolesIJ[j][i][1][xyz];
					qffConstants->dipolesIJ[i][j][3][xyz] = qffConstants->dipolesIJ[j][i][3][xyz];
				}
			}
                }
                for(i=0;i<qffConstants->numberOfFrequencies && nn==1;i++)
                {
                        if(i==j) continue;
			for(k=0;k<4 && nn==1;k++)
        		{
				for(xyz=0;xyz<3 && nn==1;xyz++)
				{
					nn = fscanf(file,"%lf",&qffConstants->dipolesI3J[j][i][k][xyz]);
					qffConstants->numberOfDipoles += nn;
					qffConstants->dipolesI3J[i][j][k][xyz] = qffConstants->dipolesI3J[j][i][k][xyz];
                			if(nn!=1) break;
				}
       	 		}
		}
	}
	qffConstants->numberOfDipoles /= 3;
	return TRUE;
}
/************************************************************************************************************/
static void computeGradients(TWOMRQFF* qffConstants)
{
 	gint nf = qffConstants->numberOfFrequencies;
	gint i;
	for(i=0;i<nf;i++)
	{
		gdouble di = qffConstants->delta[i]*sqrt(qffConstants->mass[i]*AMU_TO_AU);
		qffConstants->gradients[i]=(
				+qffConstants->VI[i][0]+
				-9*qffConstants->VI[i][1]+
				+45*qffConstants->VI[i][2]+
				-45*qffConstants->VI[i][3]+
				+9*qffConstants->VI[i][4]
				-qffConstants->VI[i][5]
				)/(60.0*di);
	}
}
/************************************************************************************************************/
static void computeFrequencies(TWOMRQFF* qffConstants)
{
 	gint nf = qffConstants->numberOfFrequencies;
	gint i;
	for(i=0;i<nf;i++)
	{
		gdouble di = qffConstants->delta[i]*qffConstants->delta[i]*AMU_TO_AU*qffConstants->mass[i];
		gdouble f = 1/di/180;
		qffConstants->calculatedFrequencies[i]=AU_TO_CM1*sqrt(f*fabs((
		2*qffConstants->VI[i][0]+
		-27*qffConstants->VI[i][1]+
		+270*qffConstants->VI[i][2]+
		-490*qffConstants->V0+
		+270*qffConstants->VI[i][3]+
		-27*qffConstants->VI[i][4]+
		2*qffConstants->VI[i][5]
		)));
		/*
		qffConstants->calculatedFrequencies[i]=sqrt(fabs((
		  qffConstants->VI[i][2]
		- 2*qffConstants->V0
		+ qffConstants->VI[i][3]
		)
		/di))*AU_TO_CM1;
		*/
	}
}
/************************************************************************************************************/
static void computeCubicForces(TWOMRQFF* qffConstants)
{
 	gint nf = qffConstants->numberOfFrequencies;
	gint i,j;
	gdouble f3cm1 = AU_TO_CM1*sqrt(AU_TO_CM1)*AU_TO_CM1;
	if(nf<1) return;
	// tiii

	for(i=0;i<nf;i++)
	{
		gdouble mi = sqrt(qffConstants->mass[i]*AMU_TO_AU);
		gdouble f = 1.0/(8.0*qffConstants->delta[i]*qffConstants->delta[i]*qffConstants->delta[i]*mi*mi*mi);
		f = f/qffConstants->frequencies[i]/sqrt(qffConstants->frequencies[i])*f3cm1;
		qffConstants->cubicEnergies[i][i] = f*(-qffConstants->VI[i][0]+8*qffConstants->VI[i][1]-13*qffConstants->VI[i][2]+13*qffConstants->VI[i][3]-8*qffConstants->VI[i][4]+qffConstants->VI[i][5]);
		/*
		gdouble mi = sqrt(qffConstants->mass[i]);
		gdouble f = 1.0/(2.0*qffConstants->delta[i]*qffConstants->delta[i]*qffConstants->delta[i]*mi*mi*mi);
		qffConstants->cubicEnergies[i][i] = f*(-1*qffConstants->VI[i][1]+2*qffConstants->VI[i][2]-2*qffConstants->VI[i][3]+1*qffConstants->VI[i][4]);
		*/
	}
	// tiij
	if(qffConstants->numberOfEnergies > 1+6*nf)
	for(i=0;i<nf;i++)
	{
		gdouble fi = 1.0/(2.0*qffConstants->delta[i]*qffConstants->delta[i]*qffConstants->mass[i]*AMU_TO_AU);
		fi = fi/qffConstants->frequencies[i];
		for(j=0;j<nf;j++)
		{
			gdouble fj,f;
			if(j==i) continue;
			fj = 1.0/(qffConstants->delta[j]*sqrt(qffConstants->mass[j]*AMU_TO_AU));
			fj = fj/sqrt(qffConstants->frequencies[j]);
			f = fi*fj*f3cm1;
			qffConstants->cubicEnergies[i][j] = f*(
				 (qffConstants->VIJ[i][j][0]+qffConstants->VIJ[i][j][2]-2*qffConstants->VI[j][2])
				-(qffConstants->VIJ[i][j][1]+qffConstants->VIJ[i][j][3]-2*qffConstants->VI[j][3])
			);

		}
	}
}
/************************************************************************************************************/
static void computeQuarticForces(TWOMRQFF* qffConstants)
{
 	gint nf = qffConstants->numberOfFrequencies;
	gdouble f4cm1 = AU_TO_CM1*AU_TO_CM1*AU_TO_CM1;
	gint i,j;
	if(nf<1) return;

	// uiiii
	for(i=0;i<nf;i++)
	{
		gdouble mdi = sqrt(qffConstants->mass[i]*AMU_TO_AU)*qffConstants->delta[i];
		gdouble f = 1.0/(6.0*mdi*mdi*mdi*mdi)*f4cm1;
		f = f/(qffConstants->frequencies[i]*qffConstants->frequencies[i]);
		qffConstants->quarticEnergiesIIJJ[i][i] = f*(
		-qffConstants->VI[i][0]
		+12*qffConstants->VI[i][1]
		-39*qffConstants->VI[i][2]
		+56*qffConstants->V0
		-39*qffConstants->VI[i][3]
		+12*qffConstants->VI[i][4]
		-qffConstants->VI[i][5]
		);
		/*
		gdouble f = 1.0/(mdi*mdi*mdi*mdi)*f4cm1;
		f = f/(qffConstants->frequencies[i]*qffConstants->frequencies[i]);
		qffConstants->quarticEnergiesIIJJ[i][i] = f*(
		qffConstants->VI[i][1]
		-4*qffConstants->VI[i][2]
		+6*qffConstants->V0
		-4*qffConstants->VI[i][3]
		+qffConstants->VI[i][4]
		);
		*/
		qffConstants->quarticEnergiesIIIJ[i][i] = qffConstants->quarticEnergiesIIJJ[i][i];
	}
	// uiiij
	if(qffConstants->numberOfEnergies > 1+6*nf)
	for(i=0;i<nf;i++)
	{
		gdouble mdi = sqrt(qffConstants->mass[i]*AMU_TO_AU)*qffConstants->delta[i];
		gdouble fi = 1.0/(16.0*mdi*mdi*mdi);
		fi = fi/(qffConstants->frequencies[i]*sqrt(qffConstants->frequencies[i]));
		for(j=0;j<nf;j++)
		{
			gdouble fj,f;
			if(j==i) continue;
			fj = 1.0/(qffConstants->delta[j]*sqrt(qffConstants->mass[j]*AMU_TO_AU));
			fj = fj/(sqrt(qffConstants->frequencies[j]));
			f = fi*fj*f4cm1;
			qffConstants->quarticEnergiesIIIJ[i][j] = f*(
				 (qffConstants->VI3J[i][j][0]-3*qffConstants->VIJ[i][j][0]+3*qffConstants->VIJ[i][j][2]-qffConstants->VI3J[i][j][2])
				-(qffConstants->VI3J[i][j][1]-3*qffConstants->VIJ[i][j][1]+3*qffConstants->VIJ[i][j][3]-qffConstants->VI3J[i][j][3])
			);
		}
	}
	// uiijj
	if(qffConstants->numberOfEnergies > 1+6*nf)
	for(i=0;i<nf;i++)
	{
		gdouble mdi = qffConstants->mass[i]*AMU_TO_AU*qffConstants->delta[i]*qffConstants->delta[i];
		gdouble f = 1.0/(mdi*mdi)*f4cm1;
		f = f/(qffConstants->frequencies[i]*qffConstants->frequencies[i]);
		for(j=0;j<i;j++)
		{
			qffConstants->quarticEnergiesIIJJ[i][j] = f*(
				   (qffConstants->VIJ[i][j][0]+qffConstants->VIJ[i][j][2]+qffConstants->VIJ[i][j][1]+qffConstants->VIJ[i][j][3])
				-2*(qffConstants->VI[i][2]+qffConstants->VI[i][3]+qffConstants->VI[j][2]+qffConstants->VI[j][3])
				+4*qffConstants->V0
			);
			qffConstants->quarticEnergiesIIJJ[j][i] = qffConstants->quarticEnergiesIIJJ[i][j];
		}
	}
}
/************************************************************************************************************/
static void computeFirstDerivativesDipoles(TWOMRQFF* qffConstants)
{
 	gint nf = qffConstants->numberOfFrequencies;
	gint i;
	gint xyz = 0;
	for(i=0;i<nf;i++)
	{
		gdouble di = 60*qffConstants->delta[i]*sqrt(qffConstants->mass[i]*AMU_TO_AU);
		gdouble f = 1/di;
		f =f*sqrt(AU_TO_CM1);
		for(xyz=0;xyz<3;xyz++)
			qffConstants->firstDipoles[i][xyz]=f*(
				+qffConstants->dipolesI[i][0][xyz]+
				-9*qffConstants->dipolesI[i][1][xyz]+
				+45*qffConstants->dipolesI[i][2][xyz]+
				-45*qffConstants->dipolesI[i][3][xyz]+
				+9*qffConstants->dipolesI[i][4][xyz]
				-qffConstants->dipolesI[i][5][xyz]
				);
	}
}
/************************************************************************************************************/
static void changeUnitInputFirstDerivativesDipoles(TWOMRQFF* qffConstants)
{
        gdouble mu0 = 4*PI*1e-7;
        gdouble eps0 = 1.0/(mu0*slight*slight);
        gdouble   kmmolm1 = 4*PI*PI*PI*NAvogadro/3/hPlank/slight/4/PI/eps0*1e-3*100.0*8.47835267e-30*8.47835267e-30;/* 1e-3 m to km, 100 : cm-1 to m-1 */
	gdouble f = 1.0/sqrt(kmmolm1);

        gint nf = qffConstants->numberOfFrequencies;
        gint i;
        gint xyz = 0;
	if(qffConstants->numberOfFirstDipolesInput==0) return;
        for(i=0;i<nf;i++)
        {
                for(xyz=0;xyz<3;xyz++)
                        qffConstants->firstDipolesInput[i][xyz] *=f;
        }
}
/************************************************************************************************************/
static void computeSecondDerivativesDipoles(TWOMRQFF* qffConstants)
{
 	gint nf = qffConstants->numberOfFrequencies;
	gint i;
	gint j;
	gint xyz = 0;
	// dii
	for(i=0;i<nf;i++)
	{
		gdouble di = qffConstants->delta[i]*qffConstants->delta[i]*AMU_TO_AU*qffConstants->mass[i];
		gdouble f = 1/(180*di);
		f =f*(AU_TO_CM1);
		for(xyz=0;xyz<3;xyz++)
			qffConstants->secondDipoles[i][i][xyz]=f*(
				 2*qffConstants->dipolesI[i][0][xyz]
				-27*qffConstants->dipolesI[i][1][xyz]
				+270*qffConstants->dipolesI[i][2][xyz]
				-490*qffConstants->dipole0[xyz]
				+270*qffConstants->dipolesI[i][3][xyz]
				-27*qffConstants->dipolesI[i][4][xyz]
				+2*qffConstants->dipolesI[i][5][xyz]
				);
			/*
			qffConstants->secondDipoles[i][i][xyz]=180*f*(
				+qffConstants->dipolesI[i][2][xyz]
				-2*qffConstants->dipole0[xyz]
				+qffConstants->dipolesI[i][3][xyz]
				);
			*/
	}
	// dij
	if(qffConstants->numberOfEnergies > 1+6*nf)
	for(i=0;i<nf;i++)
	{
		gdouble di = qffConstants->delta[i]*sqrt(qffConstants->mass[i]*AMU_TO_AU);
		for(j=0;j<nf;j++)
		{
			gdouble dj = qffConstants->delta[j]*sqrt(qffConstants->mass[j]*AMU_TO_AU);
			gdouble f = 1.0/(4.0*di*dj);
			if(i==j) continue;
			f =f*AU_TO_CM1;
			for(xyz=0;xyz<3;xyz++)
				qffConstants->secondDipoles[i][j][xyz]=f*(
				  qffConstants->dipolesIJ[i][j][0][xyz]
				- qffConstants->dipolesIJ[i][j][1][xyz]
				- qffConstants->dipolesIJ[i][j][2][xyz]
				+ qffConstants->dipolesIJ[i][j][3][xyz]
				);
			//for(xyz=0;xyz<3;xyz++)
				//qffConstants->secondDipoles[j][i][xyz]=qffConstants->secondDipoles[i][j][xyz];
		}
	}
}
/************************************************************************************************************/
static void computeCubicDerivativesDipoles(TWOMRQFF* qffConstants)
{
 	gint nf = qffConstants->numberOfFrequencies;
	gint i,j;
	gint xyz;
	if(nf<1) return;
	// diii
	for(i=0;i<nf;i++)
	{
		gdouble mi = sqrt(qffConstants->mass[i]*AMU_TO_AU);
		gdouble f = 1.0/(8.0*qffConstants->delta[i]*qffConstants->delta[i]*qffConstants->delta[i]*mi*mi*mi);
		f =f*AU_TO_CM1*sqrt(AU_TO_CM1);
		for(xyz=0;xyz<3;xyz++)
			qffConstants->cubicDipoles[i][i][xyz] = f*
			(
			-   qffConstants->dipolesI[i][0][xyz]
			+8* qffConstants->dipolesI[i][1][xyz]
			-13*qffConstants->dipolesI[i][2][xyz]
			+13*qffConstants->dipolesI[i][3][xyz]
			-8* qffConstants->dipolesI[i][4][xyz]
			+   qffConstants->dipolesI[i][5][xyz]
			);
		//qffConstants->cubicDipoles[i][i][xyz] = f*(qffConstants->dipolesI[i][0][xyz]-3*qffConstants->dipolesI[i][2][xyz]+3*qffConstants->dipolesI[i][3][xyz]-qffConstants->dipolesI[i][5][xyz]);
	}
	// tiij
	if(qffConstants->numberOfEnergies > 1+6*nf)
	for(i=0;i<nf;i++)
	{
		gdouble fi = 1.0/(2.0*qffConstants->delta[i]*qffConstants->delta[i]*qffConstants->mass[i]*AMU_TO_AU);
		for(j=0;j<nf;j++)
		{
			gdouble fj,f;
			if(j==i) continue;
			fj = 1.0/(qffConstants->delta[j]*sqrt(qffConstants->mass[j]*AMU_TO_AU));
			f = fi*fj;
			f =f*AU_TO_CM1*sqrt(AU_TO_CM1);
			for(xyz=0;xyz<3;xyz++)
				qffConstants->cubicDipoles[i][j][xyz] = f*(
				 (qffConstants->dipolesIJ[i][j][0][xyz]+qffConstants->dipolesIJ[i][j][2][xyz]-2*qffConstants->dipolesI[j][2][xyz])
				-(qffConstants->dipolesIJ[i][j][1][xyz]+qffConstants->dipolesIJ[i][j][3][xyz]-2*qffConstants->dipolesI[j][3][xyz])
			);
		}
	}
}
/************************************************************************************************************/
static void computeQuarticDerivativesDipoles(TWOMRQFF* qffConstants)
{
 	gint nf = qffConstants->numberOfFrequencies;
	gint i,j;
	gint xyz;
	if(nf<1) return;

	// diiii
	for(i=0;i<nf;i++)
	{
		gdouble mdi = sqrt(qffConstants->mass[i]*AMU_TO_AU)*qffConstants->delta[i];
		gdouble f = 1.0/(6*mdi*mdi*mdi*mdi);
		f =f*AU_TO_CM1*AU_TO_CM1;
		for(xyz=0;xyz<3;xyz++)
		{
			qffConstants->quarticDipolesIIJJ[i][i][xyz] = f*(
		   	-qffConstants->dipolesI[i][0][xyz]
		   	+12*qffConstants->dipolesI[i][1][xyz]
			-39*qffConstants->dipolesI[i][2][xyz]
			+56*qffConstants->dipole0[xyz]
			-39*qffConstants->dipolesI[i][3][xyz]
			+12*qffConstants->dipolesI[i][4][xyz]
			-qffConstants->dipolesI[i][5][xyz]);
			qffConstants->quarticDipolesIIIJ[i][i][xyz] = qffConstants->quarticDipolesIIJJ[i][i][xyz];
		}
	}
	// diiij
	if(qffConstants->numberOfEnergies > 1+6*nf)
	for(i=0;i<nf;i++)
	{
		gdouble mdi = sqrt(qffConstants->mass[i]*AMU_TO_AU)*qffConstants->delta[i];
		gdouble fi = 1.0/(16.0*mdi*mdi*mdi);
		for(j=0;j<nf;j++)
		{
			gdouble fj,f;
			if(j==i) continue;
			fj = 1.0/(qffConstants->delta[j]*sqrt(qffConstants->mass[j]*AMU_TO_AU));
			f = fi*fj;
			f =f*AU_TO_CM1*AU_TO_CM1;
			for(xyz=0;xyz<3;xyz++)
				qffConstants->quarticDipolesIIIJ[i][j][xyz] = f*(
				 (	qffConstants->dipolesI3J[i][j][0][xyz]
					-3*qffConstants->dipolesIJ[i][j][0][xyz]
					+3*qffConstants->dipolesIJ[i][j][2][xyz]
					-qffConstants->dipolesI3J[i][j][2][xyz]
				)
				-(
					qffConstants->dipolesI3J[i][j][1][xyz]
					-3*qffConstants->dipolesIJ[i][j][1][xyz]
					+3*qffConstants->dipolesIJ[i][j][3][xyz]
					-qffConstants->dipolesI3J[i][j][3][xyz]
				)
				);
		}
	}
	// diijj
	if(qffConstants->numberOfEnergies > 1+6*nf)
	for(i=0;i<nf;i++)
	{
		gdouble mdi = qffConstants->mass[i]*AMU_TO_AU*qffConstants->delta[i]*qffConstants->delta[i];
		gdouble f = 1.0/(mdi*mdi);
		f =f*AU_TO_CM1*AU_TO_CM1;
		for(j=0;j<i;j++)
		{
			for(xyz=0;xyz<3;xyz++)
			{
				qffConstants->quarticDipolesIIJJ[i][j][xyz] = f*(
				   (
					qffConstants->dipolesIJ[i][j][0][xyz]
					+qffConstants->dipolesIJ[i][j][2][xyz]
					+qffConstants->dipolesIJ[i][j][1][xyz]
					+qffConstants->dipolesIJ[i][j][3][xyz]
				   )
				-2*(
					qffConstants->dipolesI[i][2][xyz]
					+qffConstants->dipolesI[i][3][xyz]
					+qffConstants->dipolesI[j][2][xyz]
					+qffConstants->dipolesI[j][3][xyz]
				)
				+4*qffConstants->dipole0[xyz]
				);
				qffConstants->quarticDipolesIIJJ[j][i][xyz] = qffConstants->quarticDipolesIIJJ[i][j][xyz];
			}
		}
	}
}
/************************************************************************************************************/
static gboolean read_2mrqff_file(GabeditFileChooser *filesel, gint response_id)
{
	gint nf = 0;
	gchar* fileName = NULL;
	TWOMRQFF* qffConstants;
	gboolean Ok = TRUE;
	gint i;
	FILE* file = NULL;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	fileName = gabedit_file_chooser_get_current_file(filesel);

 	file = FOpen(fileName, "rb"); 
        if(!file) 
	{
		fprintf(stderr,"I cannot open the %s file\n",fileName);
		return FALSE;
	}
	qffConstants = (TWOMRQFF*) g_malloc(sizeof(TWOMRQFF));
	qffConstants->numberOfFrequencies = 0;
	Ok =  readFrequenciesInitTWOMRQFF(file, qffConstants);
	if(Ok) Ok = readVectorReal(file, "Mass", qffConstants->numberOfFrequencies, qffConstants->mass);
	if(Ok) Ok = readVectorReal(file, "Delta", qffConstants->numberOfFrequencies, qffConstants->delta);
	if(Ok) Ok = readEnergies(file, qffConstants);
	if(Ok) readDipoles(file, qffConstants);
	if(Ok) computeGradients(qffConstants);
	if(Ok) computeFrequencies(qffConstants);
	if(Ok) computeCubicForces(qffConstants);
	if(Ok) computeQuarticForces(qffConstants);
	if(Ok) changeUnitInputFirstDerivativesDipoles(qffConstants);
	if(Ok) computeFirstDerivativesDipoles(qffConstants);
	if(Ok) computeSecondDerivativesDipoles(qffConstants);
	if(Ok) computeCubicDerivativesDipoles(qffConstants);
	if(Ok) computeQuarticDerivativesDipoles(qffConstants);
	/*
	if(Ok && DEBUGFLAG != 0 ) printTWOMRQFF(qffConstants);
	*/
	if(DEBUGFLAG != 0 ) printTWOMRQFF(qffConstants);
	//if(Ok) showTWOMRQFF(qffConstants);
	showTWOMRQFF(qffConstants);
	//printf("numberOfDipoles = %d numberOfEnergies = %d\n",qffConstants->numberOfDipoles,qffConstants->numberOfEnergies);
	if(qffConstants->numberOfDipoles!=0 && qffConstants->numberOfDipoles  != qffConstants->numberOfEnergies)
		Message(_("????????????Error????????????\nNumber of dipoles != number of energies\n?????????????????????????????"),_("Error"),TRUE);
	//freeTWOMRQFF(qffConstants); // do not delete it here. It will used to create the data file for VPT2 program
	return TRUE;
}
/********************************************************************************/
void read_2mrqff_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_2mrqff_file, _("Read the  data from 2MR-QFF file"), GABEDIT_TYPEFILE_GABEDIT,GABEDIT_TYPEWIN_OTHER);
	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
