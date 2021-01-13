/* IRSpectrum.c */
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
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Files/FileChooser.h"
#include "../Common/Windows.h"
#include "../Utils/GabeditXYPlot.h"
#include "../Display/Vibration.h"
#include "../Spectrum/IRSpectrum.h"
#include "../Spectrum/SpectrumWin.h"
#include "../Utils/GabeditTextEdit.h"

/********************************************************************************/
static gchar* searchPosTxt(gchar* str, gchar* tag, gint num, gint numCols[])
{
	gchar* t1=NULL;
	gchar* t2=NULL;
	gint i;
	gdouble dum1;
	gdouble dum2;
	gint nR = 0;
	gint nM=0;
	gchar tmp[7][50];
	gint nc;

	for(i=0;i<4;i++) numCols[i] = -1;
	t1 = strstr(str,tag);
	for(i=1;i<num;i++)
	{
		if(!t1) break;
		if(!strstr(t1+1,tag))break;
		t2 = strstr(t1+1,tag);
		t1 = t2;
	}
	while(t1 && nR!=2)
	{
		if(!t1) break;
		t2 = strstr(t1+1,"\n");
		t1 = t2;
		nR = sscanf(t1+1,"%lf %lf",&dum1,&dum2);
		if(nR==2) break;
		nc = sscanf(t1+1,"%s %s %s %s %s %s %s",tmp[0],tmp[1],tmp[2], tmp[3], tmp[4],tmp[5],tmp[6]);
		for(i=0;i<nc;i++)
		{ 
			if(strstr(tmp[i],"E(ha")) numCols[0] = i+1;
			if(strstr(tmp[i],"E(Ha")) numCols[0] = i+1;
			if(strstr(tmp[i],"I(ha")) numCols[1] = i+1;
			if(strstr(tmp[i],"I(Ha")) numCols[1] = i+1;
			if(strstr(tmp[i],"E(anha")) numCols[2] = i+1;
			if(strstr(tmp[i],"E(Anha")) numCols[2] = i+1;
			if(strstr(tmp[i],"I(anha")) numCols[3] = i+1;
			if(strstr(tmp[i],"I(Anha")) numCols[3] = i+1;
		}
	}
	t1 = t2;
	if(t1) t2 = t1 +1;
	return t2;
}
/********************************************************************************/
static gint getNumberOfLinesInBlock(gchar* str)
{
	gint n;
	gint k;
	gdouble dum;
	gchar* t1 = str;
	gchar* t2;

	if(!str) return 0;
	n = 0;
	do{
		k = sscanf(t1,"%lf", &dum);
		if(k!=1) break;
		n++;
		t2 = strstr(t1+1,"\n");
		t1 = t2;
	}while(k==1);
	return n;
}
/********************************************************************************/
static gint getValuesOneColInBlock(gchar* str, gint col, gdouble tab[])
{
	gint n;
	gint k;
	gdouble dum;
	gdouble d[7];
	gchar* t1 = str;
	gchar* t2;

	n = 0;
	do{
		k = 0;
		if(col==1) k = sscanf(t1,"%lf", &dum);
		else if(col==2) k = sscanf(t1,"%lf %lf", &d[0], &dum);
		else if(col==3) k = sscanf(t1,"%lf %lf %lf", &d[0], &d[1], &dum);
		else if(col==4) k = sscanf(t1,"%lf %lf %lf %lf", &d[0], &d[1], &d[2], &dum);
		else if(col==5) k = sscanf(t1,"%lf %lf %lf %lf %lf", &d[0], &d[1], &d[2], &d[3], &dum);
		else if(col==6) k = sscanf(t1,"%lf %lf %lf %lf %lf %lf", &d[0], &d[1], &d[2], &d[3], &d[4], &dum);
		else if(col==7) k = sscanf(t1,"%lf %lf %lf %lf %lf %lf %lf", &d[0], &d[1], &d[2], &d[3], &d[4], &d[5],&dum);
		else if(col==8) k = sscanf(t1,"%lf %lf %lf %lf %lf %lf %lf %lf", &d[0], &d[1], &d[2], &d[3], &d[4], &d[5], &d[6], &dum);
		if(k!=col) break;
		tab[n] = dum;
		n++;
		t2 = strstr(t1+1,"\n");
		t1 = t2;
	}while(k==col);
	return n;
}
/********************************************************************************/
void applyAnharmonicAll(GtkWidget *dlgWin, gchar *data)
{
	GtkWidget *text = (GtkWidget*)g_object_get_data(G_OBJECT(dlgWin),"TxtWidget");
        GtkTextBuffer *buffer;
        GtkTextIter start;
        GtkTextIter end;
        GtkTextIter iterStart;
        GtkTextIter iterEnd;
	gchar* str;
	gchar* t1=NULL;
	gchar* t2=NULL;
	gchar* t3=NULL;
	gint i;
	gint nFund;
	gchar* tFund=NULL;
	gdouble* fundF = NULL;
	gdouble* fundI = NULL;
	gint nOver = 0;
	gchar* tOver=NULL;
	gdouble* overF = NULL;
	gdouble* overI = NULL;
	gint nComb = 0;
	gchar* tComb=NULL;
	gdouble* combF = NULL;
	gdouble* combI = NULL;
	gint k;
	gint nFreqs;
	gdouble* freqs = NULL;
	gdouble* intensities = NULL;
	gint numCols[4];

        if(!GTK_IS_TEXT_VIEW (text)) return;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	gtk_text_buffer_get_bounds (buffer, &start, &end);
        iterStart = start;
        iterEnd = end;

        str = gtk_text_iter_get_text(&iterStart, &iterEnd);
	tFund = searchPosTxt(str, "Fundamental",2,numCols);
	nFund = getNumberOfLinesInBlock(tFund);
	if(nFund>0) 
	{
		fundF = g_malloc(nFund*sizeof(gdouble));
		fundI = g_malloc(nFund*sizeof(gdouble));
		getValuesOneColInBlock(tFund, numCols[2], fundF);
		getValuesOneColInBlock(tFund, numCols[3], fundI);
	}
	tOver = searchPosTxt(str, "Overtones",2, numCols);
	nOver = getNumberOfLinesInBlock(tOver);
	if(nOver>0) 
	{
		overF = g_malloc(nOver*sizeof(gdouble));
		overI = g_malloc(nOver*sizeof(gdouble));
		getValuesOneColInBlock(tOver, numCols[2], overF);
		getValuesOneColInBlock(tOver, numCols[3], overI);
	}
	tComb = searchPosTxt(str, "Combination Bands",2, numCols);
	nComb = getNumberOfLinesInBlock(tComb);
	if(nComb>0) 
	{
		combF = g_malloc(nComb*sizeof(gdouble));
		combI = g_malloc(nComb*sizeof(gdouble));
		getValuesOneColInBlock(tComb, numCols[2], overF);
		getValuesOneColInBlock(tComb, numCols[3], overI);
	}
	/*
	printf("nFund = %d\n",nFund);
	printf("nOver = %d\n",nOver);
	printf("nComb = %d\n",nComb);
	*/


	nFreqs = nFund + nOver + nComb;
	if(nFreqs>0)
	{
		freqs = g_malloc(nFreqs*sizeof(gdouble));
		intensities = g_malloc(nFreqs*sizeof(gdouble));
	}
	k = 0;
	for(i=0;i<nFund;i++)
	{
		freqs[k] = fundF[i];
		intensities[k] = fundI[i];
		k++;
	}
	for(i=0;i<nOver;i++)
	{
		freqs[k] = overF[i];
		intensities[k] = overI[i];
		k++;
	}
	for(i=0;i<nComb;i++)
	{
		freqs[k] = combF[i];
		intensities[k] = combI[i];
		k++;
	}
	if(fundF) g_free(fundF);
	if(fundI) g_free(fundI);
	if(overF) g_free(overF);
	if(overI) g_free(overI);
	if(combF) g_free(combF);
	if(combI) g_free(combI);

	createIRSpectrumWin(nFreqs, freqs, intensities);
	if(freqs) g_free(freqs);
	if(intensities) g_free(intensities);

}
/********************************************************************************/
void applyAnharmonic(GtkWidget *dlgWin, gchar *data)
{
	GtkWidget *text = (GtkWidget*)g_object_get_data(G_OBJECT(dlgWin),"TxtWidget");
        GtkTextBuffer *buffer;
        GtkTextIter start;
        GtkTextIter end;
        GtkTextIter iterStart;
        GtkTextIter iterEnd;
	gchar* str;
	gchar* t1=NULL;
	gchar* t2=NULL;
	gchar* t3=NULL;
	gdouble* fundF = NULL;
	gdouble* fundI = NULL;
	gint i;
	gint n;
	gint numCols[4];

        if(!GTK_IS_TEXT_VIEW (text)) return;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	gtk_text_buffer_get_bounds (buffer, &start, &end);
        iterStart = start;
        iterEnd = end;

        str = gtk_text_iter_get_text(&iterStart, &iterEnd);
	t2 = searchPosTxt(str, "Fundamental",2, numCols);
	n = getNumberOfLinesInBlock(t2);
	if(n>0) 
	{
		fundF = g_malloc(n*sizeof(gdouble));
		fundI = g_malloc(n*sizeof(gdouble));
		getValuesOneColInBlock(t2, numCols[2], fundF);
		getValuesOneColInBlock(t2, numCols[3], fundI);
		createIRSpectrumWin(n, fundF, fundI);
	}
	if(fundF) g_free(fundF);
	if(fundI) g_free(fundI);

}
/********************************************************************************/
void applyHarmonic(GtkWidget *dlgWin, gchar *data)
{
	GtkWidget *text = (GtkWidget*)g_object_get_data(G_OBJECT(dlgWin),"TxtWidget");
        GtkTextBuffer *buffer;
        GtkTextIter start;
        GtkTextIter end;
        GtkTextIter iterStart;
        GtkTextIter iterEnd;
	gchar* str;
	gchar* t1=NULL;
	gchar* t2=NULL;
	gchar* t3=NULL;
	gdouble* fundF = NULL;
	gdouble* fundI = NULL;
	gint i;
	gint n;
	gint numCols[4];

        if(!GTK_IS_TEXT_VIEW (text)) return;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	gtk_text_buffer_get_bounds (buffer, &start, &end);
        iterStart = start;
        iterEnd = end;

        str = gtk_text_iter_get_text(&iterStart, &iterEnd);
	t2 = searchPosTxt(str, "Fundamental",2, numCols);
	n = getNumberOfLinesInBlock(t2);
	if(n>0) 
	{
		fundF = g_malloc(n*sizeof(gdouble));
		fundI = g_malloc(n*sizeof(gdouble));
		getValuesOneColInBlock(t2, numCols[0], fundF);
		getValuesOneColInBlock(t2, numCols[1], fundI);
		createIRSpectrumWin(n, fundF, fundI);
	}
	if(fundF) g_free(fundF);
	if(fundI) g_free(fundI);
}
/********************************************************************************/
GtkWidget* AnharmonicResultTxt(gchar *message,gchar *title)
{
	GtkWidget *dlgWin = NULL;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *txtWid;
	GtkWidget *button;


	dlgWin = gtk_dialog_new();
	gtk_widget_realize(GTK_WIDGET(dlgWin));

	gtk_window_set_title(GTK_WINDOW(dlgWin),title);
	gtk_window_set_transient_for(GTK_WINDOW(dlgWin),GTK_WINDOW(Fenetre));
	gtk_window_set_position(GTK_WINDOW(dlgWin),GTK_WIN_POS_CENTER);

	g_signal_connect(G_OBJECT(dlgWin), "delete_event", (GCallback)destroy_button_windows, NULL);
	g_signal_connect(G_OBJECT(dlgWin), "delete_event", (GCallback)gtk_widget_destroy, NULL);
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(dlgWin)->vbox), frame,TRUE,TRUE,0);

	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	txtWid = create_text_widget(vboxframe,NULL,&frame);
	if(message) 
	{
		gabedit_text_insert (GABEDIT_TEXT(txtWid), NULL, NULL, NULL,message,-1);   
		gtk_text_view_set_editable((GtkTextView *)txtWid, TRUE);
	}

	gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), FALSE);
  

	button = create_button(dlgWin,_("Harmonic spectrum"));
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), button, FALSE, TRUE, 5);  
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)applyHarmonic, GTK_OBJECT(dlgWin));

	button = create_button(dlgWin,_("Anharmonic spectrum/Fundamentals"));
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), button, FALSE, TRUE, 5);  
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)applyAnharmonic, GTK_OBJECT(dlgWin));

	button = create_button(dlgWin,_("Anharmonic spectrum/All"));
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), button, FALSE, TRUE, 5);  
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)applyAnharmonicAll, GTK_OBJECT(dlgWin));

	button = create_button(dlgWin,_("Close"));
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), button, FALSE, TRUE, 5);  
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)destroy_button_windows, GTK_OBJECT(dlgWin));
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(dlgWin));

	add_button_windows(title,dlgWin);
	gtk_window_set_default_size (GTK_WINDOW(dlgWin), (gint)(ScreenHeight*0.4), (gint)(ScreenHeight*0.4));
	gtk_widget_show_all(dlgWin);
  	g_object_set_data(G_OBJECT (dlgWin), "TxtWidget", txtWid);
	return dlgWin;
}
/********************************************************************************/
static void save_igvpt2(gchar* fileNameBas, FILE* file, G_CONST_RETURN gchar* keys, G_CONST_RETURN gchar* harmonicFileName)
{
	fprintf(file,"%s\n",keys);
	fprintf(file,"Geometry\n");
	fprintf(file,"%s\n", harmonicFileName);
	fprintf(file,"\n");
}
/********************************************************************************/
static void save_igvpt2_win(GtkWidget* Win, gpointer data)
{
	gint i;
	gint j;
	gchar* fileName = NULL;
	FILE* file;
	GtkWidget* buttonDirSelector = (GtkWidget*)g_object_get_data(G_OBJECT(Win), "ButtonDirSelector");
	GtkWidget* entryFileName = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryFileName"));
	GtkWidget* entryHarmonicFileName = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryHarmonicFileName"));
	GtkWidget* comboModel = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"ComboModel"));
	GtkWidget* entryModel = NULL;
	G_CONST_RETURN gchar* fileNameStr = NULL;
	G_CONST_RETURN gchar* dirNameStr = NULL;
	G_CONST_RETURN gchar* modelStr = NULL;
	gchar* allKeys = NULL;
	gchar* fileNameBas = NULL;
	G_CONST_RETURN gchar* harmonicFileName = NULL;

	if(buttonDirSelector) dirNameStr = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
	if(entryHarmonicFileName) harmonicFileName = gtk_entry_get_text (GTK_ENTRY(entryHarmonicFileName));
	if(!dirNameStr) return;
	if(entryFileName) fileNameStr = gtk_entry_get_text(GTK_ENTRY(entryFileName));
	printf("fileName = %s\n", fileNameStr);
	if(!fileNameStr) return;
	if(!comboModel) printf("!comboModel\n");
	if(!comboModel) return;
	entryModel = (GtkWidget*)(g_object_get_data(G_OBJECT(comboModel),"Entry"));
	if(!entryModel) return;
	modelStr = gtk_entry_get_text(GTK_ENTRY(entryModel));
	if(!modelStr) return;

	allKeys = g_strdup_printf(
	"RunType=HybridMM\n"
	"VPT2Model=%s\n"
	"PropModel=GVPT2\n"
	"maxFrequencyDifferenceFermi=200\n"
	"MartinCutOff1=1.0\n"
	"MartinCutOff2=1.0\n"
	"QFFnModes=3\n"
	"mmff94Charges=TRUE\n"
	"\n",
	modelStr);

	fileName = g_strdup_printf("%s%s%s",dirNameStr,G_DIR_SEPARATOR_S,fileNameStr);
 	file = fopen(fileName, "w");
	if(!file)
	{
		gchar* t = g_strdup_printf(_("Sorry\n I can not create %s file"),fileName); 
		Message(t,_("Error"),TRUE);
		if(fileName) g_free(fileName);
		if(t)g_free(t);
		return;
	}
	fileNameBas = g_path_get_basename(fileNameStr);
	for(i=strlen(fileNameBas);i>0;i--)
	if(fileNameBas[i]=='.')
	{
		fileNameBas[i]='\0';
		break;
	}
	save_igvpt2(fileNameBas, file, allKeys, harmonicFileName);
	fclose(file);
	if(fileNameBas) g_free(fileNameBas);
	gtk_widget_destroy(Win);
	{
		gchar* t = g_strdup_printf(_("The %s file was created"),fileName); 
		Message(t,_("Error"),TRUE);
		if(t)g_free(t);
	}
}
/********************************************************************************/
static void run_igvpt2_win(GtkWidget* Win, gpointer data)
{
	gint i;
	gint j;
	gchar* fileName = NULL;
	FILE* file;
	GtkWidget* buttonDirSelector = (GtkWidget*)g_object_get_data(G_OBJECT(Win), "ButtonDirSelector");
	GtkWidget* entryFileName = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryFileName"));
	GtkWidget* entryHarmonicFileName = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryHarmonicFileName"));
	GtkWidget* comboModel = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"ComboModel"));
	GtkWidget* entryModel = NULL;
	G_CONST_RETURN gchar* fileNameStr = NULL;
	G_CONST_RETURN gchar* dirNameStr = NULL;
	G_CONST_RETURN gchar* modelStr = NULL;
	gchar* allKeys = NULL;
	gchar* fileNameBas = NULL;
	gchar* fileNameRes = NULL;
	gchar* command = NULL;
	gchar* old = NULL;
	gchar* result = NULL;
	G_CONST_RETURN gchar* harmonicFileName = NULL;

	if(buttonDirSelector) dirNameStr = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
	if(entryHarmonicFileName) harmonicFileName = gtk_entry_get_text (GTK_ENTRY(entryHarmonicFileName));
	if(!dirNameStr) return;
	if(entryFileName) fileNameStr = gtk_entry_get_text(GTK_ENTRY(entryFileName));
	printf("fileName = %s\n", fileNameStr);
	if(!fileNameStr) return;
	if(!comboModel) printf("!comboModel\n");
	if(!comboModel) return;
	entryModel = (GtkWidget*)(g_object_get_data(G_OBJECT(comboModel),"Entry"));
	if(!entryModel) return;
	modelStr = gtk_entry_get_text(GTK_ENTRY(entryModel));
	if(!modelStr) return;

	allKeys = g_strdup_printf(
	"RunType=HybridMM\n"
	"VPT2Model=%s\n"
	"PropModel=GVPT2\n"
	"maxFrequencyDifferenceFermi=200\n"
	"MartinCutOff1=1.0\n"
	"MartinCutOff2=1.0\n"
	"QFFnModes=3\n"
	"mmff94Charges=TRUE\n"
	"\n",
	modelStr);

	fileName = g_strdup_printf("%s%s%s",dirNameStr,G_DIR_SEPARATOR_S,fileNameStr);
 	file = fopen(fileName, "w");
	if(!file)
	{
		gchar* t = g_strdup_printf(_("Sorry\n I can not create %s file"),fileName); 
		Message(t,_("Error"),TRUE);
		if(fileName) g_free(fileName);
		if(t)g_free(t);
		return;
	}
	fileNameBas = g_path_get_basename(fileNameStr);
	for(i=strlen(fileNameBas);i>0;i--)
	if(fileNameBas[i]=='.')
	{
		fileNameBas[i]='\0';
		break;
	}
	save_igvpt2(fileNameBas, file, allKeys, harmonicFileName);
	fclose(file);
	fileNameRes = g_strdup_printf("%s.out", fileNameBas);
	command = g_strdup_printf("%s%siGVPT2%sbin%sigvpt2 %s > %s", g_get_home_dir(), G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S, fileName, fileNameRes);
	system(command);
 	file = fopen(fileNameRes, "r");
	if(file)
	{
		gchar* t = g_malloc(BSIZE*sizeof(gchar));
		result = g_strdup_printf("%s","");
		while(!feof(file))
        	{
                	if(!fgets(t,BSIZE,file))break;
			old = result;
			result = g_strdup_printf("%s%s",old,t);
                        if(old) g_free(old);
        	}
        	if(result)
        	{
                	GtkWidget* message = AnharmonicResultTxt(result,"iGVPT2 result");
                	gtk_window_set_default_size (GTK_WINDOW(message),(gint)(ScreenWidth*0.8),-1);
                	gtk_widget_set_size_request(message,(gint)(ScreenWidth*0.45),-1);
                	/* gtk_window_set_modal (GTK_WINDOW (message), TRUE);*/
        	}
		fclose(file);
		g_free(t);
	}


	if(fileNameRes) g_free(fileNameRes);
	if(fileNameBas) g_free(fileNameBas);
	gtk_widget_destroy(Win);
	{
		gchar* t = g_strdup_printf(_("The %s file was created"),fileName); 
		Message(t,_("Error"),TRUE);
		if(t)g_free(t);
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
static GtkWidget *addVPT2ModelsToTable(GtkWidget *table, gint i)
{
	GtkWidget* entry = NULL;
	GtkWidget* combo = NULL;
	gint nlist = 2;
	gchar* list[] = {"HDCPT2","GVPT2"};

	add_label_table(table,_("Model"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entry = addComboListToATable(table, list, nlist, i, 2, 1);
	combo  = g_object_get_data(G_OBJECT (entry), "Combo");
	g_object_set_data(G_OBJECT(combo), "Entry", entry);
	gtk_widget_set_sensitive(entry, FALSE);

	return combo;
}
/********************************************************************************/
static void set_entry_freqs_file_name(GtkWidget* SelFile, gint response_id)
{
  GtkWidget *entry = NULL;
  gchar *longfile = NULL;
   if(response_id != GTK_RESPONSE_OK) return;

  entry = (GtkWidget*)(g_object_get_data(G_OBJECT(SelFile),"EntryFile"));
  longfile = gabedit_file_chooser_get_current_file(GABEDIT_FILE_CHOOSER(SelFile));
  gtk_entry_set_text(GTK_ENTRY(entry),longfile);

}
/********************************************************************************/
static void set_entry_freqs_file_name_selection(GtkWidget* entry)
{
  GtkWidget *SelFile;

  SelFile = gabedit_file_chooser_new(_("File chooser"), GTK_FILE_CHOOSER_ACTION_OPEN);
  gtk_window_set_modal (GTK_WINDOW (SelFile), TRUE);
  gabedit_file_chooser_hide_hidden(GABEDIT_FILE_CHOOSER(SelFile));
  g_signal_connect(G_OBJECT(SelFile),"delete_event", (GCallback)gtk_widget_destroy,NULL);

  g_object_set_data(G_OBJECT (SelFile), "EntryFile", entry);

  g_signal_connect (SelFile, "response",  G_CALLBACK (set_entry_freqs_file_name), GTK_OBJECT(SelFile));
  g_signal_connect (SelFile, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(SelFile));

  /* g_signal_connect (SelFile, "close",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(SelFile));*/
  gtk_widget_show(SelFile);
}
/***********************************************************************************************/
static GtkWidget *addHarmonicFileNameToTable(GtkWidget *Wins, GtkWidget *table, gint i)
{
	GtkWidget* entry;
	GtkWidget* button;
	add_label_table(table,_("File name of harmonic modes"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
  	entry = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entry),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"OrcaHessian.hess");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);
	button = create_button(Wins,"Change");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(GTK_OBJECT (button), "clicked", G_CALLBACK(set_entry_freqs_file_name_selection), GTK_OBJECT(entry));
	add_widget_table(table,button,i,3);
  	/* g_signal_connect(G_OBJECT (entrybabel), "changed", (GCallback)changed_ffn, NULL);*/
	return entry;
}
/********************************************************************************/
static GtkWidget*   add_inputiGVPT2(GtkWidget *Wins,GtkWidget *vbox)
{
	GtkWidget* entry;
	GtkWidget* sep;
  	GtkWidget *table = gtk_table_new(8,4,FALSE);
	GtkWidget* comboModel = NULL;
	GtkWidget* buttonDirSelector = NULL;
	GtkWidget* entryFileName = NULL;
	GtkWidget* label = NULL;
	gint i;
	gint j;

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	add_label_table(table,_("Working Folder"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),

                  1,1);
	j = 2;
	buttonDirSelector = gabedit_dir_button();
	gtk_widget_set_size_request(GTK_WIDGET(buttonDirSelector),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),buttonDirSelector,
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "ButtonDirSelector", buttonDirSelector);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("File name"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	entryFileName = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryFileName),"iGVPT2.ici");
	gtk_widget_set_size_request(GTK_WIDGET(entryFileName),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryFileName, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "EntryFileName", entryFileName);
/*----------------------------------------------------------------------------------*/
	i++;
	entry = addHarmonicFileNameToTable(Wins, table, i);
	g_object_set_data(G_OBJECT(Wins), "EntryHarmonicFileName", entry);
/*----------------------------------------------------------------------------------*/
	i++;
	comboModel = addVPT2ModelsToTable(table, i);
	g_object_set_data(G_OBJECT(Wins), "ComboModel", comboModel);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  2,2);
/*----------------------------------------------------------------------------------*/
	gtk_widget_show_all(table);
	return entryFileName;
}
/********************************************************************************************************/
void create_igvpt2_file_dlg(gboolean run)
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget* vbox;

	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),"iGVPT2 input file for an anharmonic calculation");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 2);
	/*gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));*/
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)delete_child,NULL);

	vboxall = create_vbox(Win);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
	gtk_box_pack_start(GTK_BOX(vboxall), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);
  	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

  	gtk_widget_realize(Win);
	
	add_inputiGVPT2(Win,vbox);
	if(!run) add_cancel_ok_buttons(Win,vbox,(GCallback)save_igvpt2_win);
	else add_cancel_ok_buttons(Win,vbox,(GCallback)run_igvpt2_win);

	/* Show all */
	gtk_widget_show_all (Win);
}
/********************************************************************************/
