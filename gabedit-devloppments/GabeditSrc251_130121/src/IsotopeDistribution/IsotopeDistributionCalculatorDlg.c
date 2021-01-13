/* IsotopeDistributionCalculatorDlg.c */
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
#include "../IsotopeDistribution/IsotopeDistributionCalculator.h"
#include "../IsotopeDistribution/IsotopeDistributionCalculatorDlg.h"
#include "../Utils/GabeditXYPlot.h"

#define DEBUGFLAG 0

/*****************************************************************************/
static GtkWidget* getPointsFromList(GtkWidget* list, gdouble** pX, gdouble** pY, gint* pn)
{
        GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
	gchar* dataMass = NULL;
	gchar* dataAbund = NULL;
	gint i;
	gint n = 0;
	gchar* path_string = NULL;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	GtkWidget* xyplot = g_object_get_data(G_OBJECT(list),"XYPlot");
	GtkWidget* entryCharge = g_object_get_data(G_OBJECT(list),"EntryCharge");
	gdouble charge = 1.0;

	if(!pX || !pY) return NULL;
	*pX = NULL;
	*pY = NULL;
	if(!xyplot) return NULL;
	if(!GTK_IS_WIDGET(xyplot)) return NULL;
	if(entryCharge)
	{
		G_CONST_RETURN gchar* tentry;
		tentry = gtk_entry_get_text(GTK_ENTRY(entryCharge));
		charge = atof(tentry);
		if(charge<1e-12) charge = 1.0;
	}
	do{

	  	path_string = g_strdup_printf("%d",n);
		if(!gtk_tree_model_get_iter_from_string (model, &iter, path_string))break;
		n++;
	}while(TRUE);
	/* printf("Number of ligne = %d\n",n);*/
	if(n<1) return NULL;
	X = g_malloc(n*sizeof(gdouble));
	Y = g_malloc(n*sizeof(gdouble));
	for(i=0;i<n;i++) X[i] = Y[i] = 0;
	i = 0;
	do{

	  	path_string = g_strdup_printf("%d",i);
		if(!gtk_tree_model_get_iter_from_string (model, &iter, path_string))break;
		dataMass = NULL;
		dataAbund = NULL;
		gtk_tree_model_get (model, &iter, 0, &dataMass, 4, &dataAbund, -1);
		if(dataMass && dataAbund)
		{
			X[i] = atof(dataMass)/charge;
			Y[i] = atof(dataAbund);
		}
		i++;
	}while(i<n);
	*pX = X;
	*pY = Y;
	*pn = n;
	return xyplot;

}
/*****************************************************************************/
static void addPeaksFromList(GtkWidget* list)
{
	gint n = 0;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	GtkWidget* xyplot = getPointsFromList(list, &X, &Y, &n);

	if(!xyplot) return;
	gabedit_xyplot_add_data_peaks(GABEDIT_XYPLOT(xyplot), n, X,  Y, NULL);
	gabedit_xyplot_set_range_ymin (GABEDIT_XYPLOT(xyplot), 0);
	gabedit_xyplot_set_range_ymax (GABEDIT_XYPLOT(xyplot), 100);

	if(X)
	{
		gabedit_xyplot_set_range_xmin (GABEDIT_XYPLOT(xyplot), (gint)(X[0]-1));
		gabedit_xyplot_set_range_xmax (GABEDIT_XYPLOT(xyplot), (gint)(X[n-1]+1));
	}
	if(X) g_free(X);
	if(Y) g_free(Y);
}
/*****************************************************************************/
static void addConvFromList(GtkWidget* list)
{
	gint n = 0;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	GdkColor color;
	GtkWidget* xyplot = getPointsFromList(list, &X, &Y, &n);
	GtkWidget* entryHalfWidth = NULL;
	gdouble halfWidth = 0.05;
	GabeditXYPlotConvType convType = GABEDIT_XYPLOT_CONV_LORENTZ;
  	GtkWidget* toggleLorentzian = g_object_get_data(G_OBJECT (list), "ToggleLorentzian");

	if(!xyplot) return;

	entryHalfWidth = g_object_get_data(G_OBJECT(list),"EntryHalfWidth");
	if(entryHalfWidth)
	{
		G_CONST_RETURN gchar* tentry;
		tentry = gtk_entry_get_text(GTK_ENTRY(entryHalfWidth));
		halfWidth = atof(tentry);
		if(halfWidth<1e-12) 
		{
			gtk_entry_set_text(GTK_ENTRY(entryHalfWidth),"0.05");
			halfWidth = 0.05;
		}
	}
	if(!GTK_TOGGLE_BUTTON (toggleLorentzian)->active) convType = GABEDIT_XYPLOT_CONV_GAUSS;
	color.red = 65000;
	color.green = 0;
	color.blue = 0;
	gabedit_xyplot_add_data_conv(GABEDIT_XYPLOT(xyplot), n, X,  Y, halfWidth, convType, &color);
	gabedit_xyplot_set_range_ymin (GABEDIT_XYPLOT(xyplot), 0);
	gabedit_xyplot_set_range_ymax (GABEDIT_XYPLOT(xyplot), 100);
	if(X)
	{
		gabedit_xyplot_set_range_xmin (GABEDIT_XYPLOT(xyplot), (gint)(X[0]-10*halfWidth));
		gabedit_xyplot_set_range_xmax (GABEDIT_XYPLOT(xyplot), (gint)(X[n-1]+10*halfWidth+0.5));
	}
	if(X) g_free(X);
	if(Y) g_free(Y);
}
/********************************************************************************/
static void clearText(GtkWidget* myText)
{
	ClearText(myText);

}
/********************************************************************************/
static void clearList(GtkWidget* myList)
{
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(myList));
        GtkListStore *store = GTK_LIST_STORE (model);
	gtk_list_store_clear(store);
}
/*****************************************************************************/
static void appendList(GtkWidget* list, GList* peaks, gdouble massPrecision, gdouble abdunPrecision)
{
	GList* p = peaks;
#define NC 3
	gchar *texts[NC];
	gint k;
        GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
        GtkListStore *store = GTK_LIST_STORE (model);
	gdouble max;
	gdouble min;
	gint nMass = 13;
	gint nAbund = 13;
	gdouble log10 = log(10.0);
	gdouble absMass = fabs(massPrecision);
	gdouble absAbund = fabs(abdunPrecision);
	gchar* formatMass = NULL;
	gchar* formatAbundFrac = NULL;
	gchar* formatAbund = NULL;

	if(absMass>1e-13)
	{
		nMass = (gint)(-log(absMass)/log10+0.5);
		if(nMass<0) nMass = 1;
	}
	if(absAbund>1e-13)
	{
		nAbund = (gint)(-log(absAbund)/log10+0.5);
		if(nAbund<0) nAbund = 1;
	}

	g_object_set_data(G_OBJECT(list),"Peaks", peaks);
  
        model = GTK_TREE_MODEL (store);
	max = 0;
	for(p = peaks; p != NULL; p = p->next)
	{
		IsotopeData* is = (IsotopeData*) (p->data);
		if(max<is->abundance) max = is->abundance;
	}
	if(max<1e-10) max = 1.0;
	min = max;
	for(p = peaks; p != NULL; p = p->next)
	{
		IsotopeData* is = (IsotopeData*) (p->data);
		if(min>is->abundance) min = is->abundance;
	}
	if(min<1e-7) min = 1e-7;

	formatMass = g_strdup_printf("%%0.%df",nMass);
	if(min<1e-7) formatAbundFrac = g_strdup_printf("%%0.%de",nAbund+2);
	else formatAbundFrac = g_strdup_printf("%%0.%df",nAbund+2);
	if(min/max*100<1e-7) formatAbund = g_strdup_printf("%%0.%de",nAbund);
	else formatAbund = g_strdup_printf("%%0.%df",nAbund);
	

	for(p = peaks; p != NULL; p = p->next)
	{
		IsotopeData* is = (IsotopeData*) (p->data);
 	 	texts[0] = g_strdup_printf(formatMass,is->mass);
		texts[1] = g_strdup_printf(formatAbundFrac,is->abundance);
		texts[2] = g_strdup_printf(formatAbund,is->abundance/max*100);
        	gtk_list_store_append (store, &iter);
		for(k=0;k<NC;k++)
		{
			gboolean ed = TRUE;
       			gtk_list_store_set (store, &iter, k+k, texts[k],k+k+1,ed, -1);
			g_free(texts[k]);
		}
	}
	/*
 	texts[0] = g_strdup_printf(" ");
	texts[1] = g_strdup_printf(" ");
	texts[2] = g_strdup_printf(" ");
        gtk_list_store_append (store, &iter);
	for(k=0;k<NC;k++)
	{
		gboolean ed = TRUE;
       		gtk_list_store_set (store, &iter, k+k, texts[k],k+k+1,ed, -1);
		g_free(texts[k]);
	}
	*/
	if(formatMass) g_free(formatMass);
	if(formatAbundFrac) g_free(formatAbundFrac);
	if(formatAbund) g_free(formatAbund);
#undef NC
}
/********************************************************************************/
static void editedPeak (GtkCellRendererText *cell, gchar  *path_string, gchar *new_text, gpointer data)
{
	GtkTreeModel *model = GTK_TREE_MODEL (data);
	GtkTreeIter iter;
	GtkTreePath *path = NULL;
	gint numCol = 0;
	gint Nc = -1;
	GList* peaks;
	if(!new_text) return;
	numCol = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell),"NumColumn"));
	peaks = g_object_get_data(G_OBJECT(cell),"Peaks");
	/* mass */
	if(numCol==0)
	{
		gint nc = strlen(new_text);
		if(nc<1)return;
		if(!test(new_text)) return;
		if(atof(new_text)<0) return;
	  	Nc = atoi(path_string);
		path = gtk_tree_path_new_from_string (path_string);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, new_text, -1);
		gtk_tree_path_free (path);
		return;
	}
	/* abundance Frac*/
	if(numCol==1)
	{
		gint nc = strlen(new_text);
		if(nc<1)return;
		if(!test(new_text)) return;
		if(atof(new_text)<0) return;
		if(atof(new_text)>1) return;
	  	Nc = atoi(path_string);
		path = gtk_tree_path_new_from_string (path_string);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, new_text, -1);
		gtk_tree_path_free (path);
		return;
	}
	/* Rel. abundance(%)*/
	if(numCol==2)
	{
		gint nc = strlen(new_text);
		if(nc<1)return;
		if(!test(new_text)) return;
		if(atof(new_text)<0) return;
		if(atof(new_text)>100) return;
	  	Nc = atoi(path_string);
		path = gtk_tree_path_new_from_string (path_string);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, new_text, -1);
		gtk_tree_path_free (path);
		return;
	}
}
/************************************************************/
static GtkWidget* create_peaks_list(GtkWidget *vbox, GList* peaks)
{
        GtkListStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkWidget *list = NULL;

	GtkWidget *scr;
	guint i;
	guint Factor=7;
	guint widall=0;
#define  NC 3
	gchar *titres[NC]={ N_("Mass"), N_("Frac. Abund."), N_("Rel. Abund.(%)")};
	gint width[NC]={14,12,12}; 
  
	for(i=0;(gint)i<NC;i++) widall+=width[i];
	widall=widall*Factor+50;

	scr=gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_set_size_request(scr,widall,(gint)(ScreenHeight*0.4));
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 
	gtk_box_pack_start(GTK_BOX (vbox), scr,TRUE, TRUE, 2);
	gtk_widget_set_size_request(vbox,widall+10,-1);

	store = gtk_list_store_new (NC*2,
		       	G_TYPE_STRING, G_TYPE_BOOLEAN,
		       	G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_BOOLEAN
			);
        model = GTK_TREE_MODEL (store);

	list = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (list), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (list), TRUE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW (list), TRUE);
	for (i=0;(gint)i<NC;i++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, titres[i]);
		gtk_tree_view_column_set_min_width(column, width[i]*Factor);
		gtk_tree_view_column_set_reorderable(column, TRUE);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, TRUE);
		gtk_tree_view_column_set_attributes (column, renderer, "text", i+i, "editable",i+i+1,NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
		g_object_set_data(G_OBJECT(renderer),"NumColumn", GINT_TO_POINTER(i));
		g_object_set_data(G_OBJECT(renderer),"Peaks", peaks);
		g_signal_connect (renderer, "edited", G_CALLBACK (editedPeak), model);
	}
	gtk_container_add(GTK_CONTAINER(scr),list);
  
	set_base_style(list,55000,55000,55000);
	gtk_widget_show (list);
#undef  NC
	appendList(list,peaks,1e-8,1e-3);
	return list;
}
/********************************************************************/
static void cancel_calculation(GtkWidget* cancelButton, gpointer data)
{
	cancelIsotopeDistribution = TRUE;
	gtk_widget_set_sensitive(cancelButton,FALSE);
	if(data && GTK_IS_WIDGET((GtkWidget*)data))
		gtk_widget_set_sensitive((GtkWidget*)data,FALSE);/* calculate button*/

}
/********************************************************************/
static void calculate_spectrum(GtkWidget* calculateButton, gpointer data)
{
	GtkWidget** entrys;
	GtkWidget* cancelButton;
	G_CONST_RETURN gchar* tentry;
	gdouble* pMass = NULL;
	gdouble* pAbund = NULL;
	gchar* t;
	GtkWidget* treeViewPeaks = NULL;
	GtkWidget* textWidget = NULL;
	GList* newPeaks = NULL;
	gchar* error = NULL;
	gchar* info = NULL;

	cancelButton = g_object_get_data(G_OBJECT (calculateButton), "CancelButton");
	if(!cancelButton) return;
	if(cancelButton && !GTK_IS_WIDGET(cancelButton) ) return;

	if(!data) return;
	entrys = (GtkWidget**) data;
	if(entrys[0] && !GTK_IS_WIDGET(entrys[0])) return;
	if(entrys[1] && !GTK_IS_WIDGET(entrys[1])) return;
	if(entrys[2] && !GTK_IS_WIDGET(entrys[2])) return;

	pMass = g_object_get_data(G_OBJECT (entrys[1]), "MassPrecision");
	if(!pMass) return;
	pAbund = g_object_get_data(G_OBJECT (entrys[2]), "AbundPrecision");
	if(!pAbund) return;
  	

	tentry = gtk_entry_get_text(GTK_ENTRY(entrys[1]));
	*pMass = atof(tentry);
	if(*pMass<1e-14) *pMass = 0.01;

	if(*pMass<1e-6) t = g_strdup_printf("%0.8e",*pMass);
	else t = g_strdup_printf("%0.8f",*pMass);
	gtk_entry_set_text(GTK_ENTRY(entrys[1]),t);
	g_free(t);

	tentry = gtk_entry_get_text(GTK_ENTRY(entrys[2]));
	*pAbund = atof(tentry);
	if(*pAbund<1e-14) *pAbund = 0.0;
	if(*pAbund>100) *pAbund = 100.0;
	if(*pAbund<1e-6) t = g_strdup_printf("%0.8e",*pAbund);
	else t = g_strdup_printf("%0.8f",*pAbund);
	gtk_entry_set_text(GTK_ENTRY(entrys[2]),t);
	g_free(t);

	tentry = gtk_entry_get_text(GTK_ENTRY(entrys[0]));
	if(!tentry) return;
	if(tentry && strlen(tentry)<1) return;
	gtk_widget_set_sensitive(calculateButton,FALSE);
	cancelIsotopeDistribution = FALSE;
	gtk_widget_set_sensitive(cancelButton,TRUE);

	t = g_strdup(tentry);
	newPeaks = compute_isotope_distribution_from_formula(t, *pMass, *pAbund, &error,&info);
	g_free(t);
 	textWidget = g_object_get_data(G_OBJECT (calculateButton), "TextWidget");
	if(textWidget) clearText(textWidget);
	if(error) 
	{
		if(textWidget) 
			gabedit_text_insert (GABEDIT_TEXT(textWidget), NULL, NULL, NULL,error,-1);
		printf("%s\n",error);
	}
	else if(newPeaks)
	{
		if(textWidget) 
		{
			gchar* t = NULL;
			if(info) gabedit_text_insert (GABEDIT_TEXT(textWidget), NULL, NULL, NULL,info,-1);
			t = g_strdup_printf(_("Sum of frac. abund -1 = \n  %0.8e\n"),
			get_sum_abundance_from_list(newPeaks)-1);
			gabedit_text_insert (GABEDIT_TEXT(textWidget), NULL, NULL, NULL,t,-1);
			g_free(t);
		}
	}
	if(error) g_free(error);
	if(info) g_free(info);
	/*newPeaks = get_example_isotope_distribution();*/


 	treeViewPeaks = g_object_get_data(G_OBJECT (calculateButton), "ListView");
	if(treeViewPeaks) clearList(treeViewPeaks);
	appendList(treeViewPeaks, newPeaks,*pMass, *pAbund);
	if(newPeaks) free_isotope_distribution(newPeaks);
	g_object_set_data(G_OBJECT(treeViewPeaks),"Peaks", NULL);

	gtk_widget_set_sensitive(calculateButton,TRUE);
	cancelIsotopeDistribution = TRUE;
	gtk_widget_set_sensitive(cancelButton,FALSE);
}
/********************************************************************/
static void delete_distribution_dlg(GtkWidget* winDlg, gpointer data)
{
	GtkWidget** entrys =  g_object_get_data(G_OBJECT (winDlg), "Entries");
	if(entrys) g_free(entrys);
	gtk_widget_destroy(winDlg);
}
/********************************************************************/
void compute_distribution_dlg(GtkWidget* winParent, gchar* formula)
{
	GtkWidget *winDlg;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget *vboxframe;
	GtkWidget *hbox;
	GtkWidget *hboxall;
	GtkWidget *vboxLeft;
	GtkWidget *vboxRight;
	GtkWidget *button;
	GtkWidget *hseparator;
	GtkWidget *calculateButton;
	GtkWidget *cancelButton;
	GtkWidget *clearButton;
	/* GtkWidget *label;*/
	GtkWidget *xyplot;
	GtkWidget *textWidget;
	GtkWidget *addPeaksButton;
	GtkWidget *addConvolutionButton;
	GtkWidget *toggleLorentzian;
	GtkWidget *toggleGaussian;

	gchar* tlabel[]={N_("Chemical formula"),N_("Mass precision (Da)"),N_("Abondance precision (%)")};
	GtkWidget** entrys = g_malloc(5*sizeof(GtkWidget*));
	gint i;
	GtkWidget* table;
	gchar* t = NULL;
	static gdouble massPrecision = 0.1;
	static gdouble abundPrecision = 0.01;
	GtkWidget* treeViewPeaks = NULL;

	if(!winParent || !GTK_IS_WIDGET(winParent)) return;

	entrys = (GtkWidget**) g_malloc(5*sizeof(GtkWidget*));
	for(i=0;i<5;i++) entrys[i] = NULL;

	winDlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	/* gtk_window_set_modal(GTK_WINDOW(winDlg),TRUE);*/
	gtk_window_set_title(GTK_WINDOW(winDlg),_("Isotope distribution calculation"));
	gtk_container_set_border_width (GTK_CONTAINER (winDlg), 5);

	gtk_window_set_position(GTK_WINDOW(winDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(winDlg),GTK_WINDOW(winParent));

	g_signal_connect(G_OBJECT(winDlg),"delete_event",(GCallback)delete_distribution_dlg,NULL);

	vboxall = create_vbox(winDlg);
	hboxall = gtk_hbox_new(0,FALSE);
	gtk_box_pack_start (GTK_BOX (vboxall), hboxall, TRUE, TRUE, 0);

	vboxLeft = gtk_vbox_new(0,FALSE);
	gtk_box_pack_start (GTK_BOX (hboxall), vboxLeft, FALSE, FALSE, 0);
	vboxRight = gtk_hbox_new(0,FALSE);
	gtk_box_pack_start (GTK_BOX (hboxall), vboxRight, TRUE, TRUE, 0);

	frame = gtk_frame_new (_("Input"));
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxLeft), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	table = gtk_table_new(5,3,FALSE);
	gtk_box_pack_start(GTK_BOX(vboxframe), table,TRUE,TRUE,0);

	for(i=0;i<3;i++)
	{
		add_label_table(table,tlabel[i],(gushort)i,0);
		add_label_table(table," : ",(gushort)i,1); 

		entrys[i] = gtk_entry_new ();
		gtk_table_attach(GTK_TABLE(table),entrys[i],2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	}
	if(formula) t = g_strdup_printf("%s",formula);
	else t = g_strdup_printf("C6H6");
	gtk_entry_set_text(GTK_ENTRY(entrys[0]),t);
	g_free(t);
	t = g_strdup_printf("%f",massPrecision);
	gtk_entry_set_text(GTK_ENTRY(entrys[1]),t);
	g_free(t);
  	g_object_set_data(G_OBJECT (entrys[1]), "MassPrecision", &massPrecision);
	t = g_strdup_printf("%f",abundPrecision);
	gtk_entry_set_text(GTK_ENTRY(entrys[2]),t);
	g_free(t);
  	g_object_set_data(G_OBJECT (entrys[2]), "AbundPrecision", &abundPrecision);

	i = 3;
	hseparator = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),hseparator,0,0+3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);

	i = 4;
	button = create_button(winParent,_("Calculate"));
	gtk_table_attach(GTK_TABLE(table),button,0,0+2,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(calculate_spectrum),(gpointer)entrys);
	gtk_widget_show (button);
	calculateButton = button;

	button = create_button(winParent,_("Cancel"));
	gtk_table_attach(GTK_TABLE(table),button,2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(cancel_calculation),(gpointer)calculateButton);
	gtk_widget_show (button);
	cancelButton = button;

  	g_object_set_data(G_OBJECT (button), "CalculateButton", calculateButton);
  	g_object_set_data(G_OBJECT (calculateButton), "CancelButton", button);
	gtk_widget_set_sensitive(button,FALSE);


	frame = gtk_frame_new (_("Output"));
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxLeft), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	table = gtk_table_new(10,4,FALSE);
	gtk_box_pack_start(GTK_BOX(vboxframe), table,TRUE,TRUE,0);


	i = 0;
	hbox = gtk_hbox_new(0,FALSE);
	gtk_table_attach(GTK_TABLE(table),hbox,0,0+4,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);

	textWidget = create_text_widget(hbox,NULL,&frame);
	gtk_widget_set_size_request(textWidget,-1,(gint)(ScreenHeight*0.15));
  	gabedit_text_set_editable (GABEDIT_TEXT (textWidget), TRUE); 
	set_font (textWidget,FontsStyleResult.fontname);
	set_base_style(textWidget,FontsStyleResult.BaseColor.red ,FontsStyleResult.BaseColor.green ,FontsStyleResult.BaseColor.blue);
	set_text_style(textWidget,FontsStyleResult.TextColor.red ,FontsStyleResult.TextColor.green ,FontsStyleResult.TextColor.blue);

	i = 1;
  	hbox = gtk_hbox_new (TRUE, 0);
	gtk_table_attach(GTK_TABLE(table),hbox,0,0+4,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	treeViewPeaks = create_peaks_list(hbox, NULL);

	i = 2;
	button = create_button(winParent,_("Clear"));
	gtk_table_attach(GTK_TABLE(table),button,0,0+3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(clearList),(gpointer)treeViewPeaks);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(clearText),(gpointer)textWidget);
	gtk_widget_show (button);
	clearButton = button;

	button = create_button(winParent,_("Close"));
	gtk_table_attach(GTK_TABLE(table),button,3,3+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
		  1,1);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_distribution_dlg),(gpointer)winDlg);
	gtk_widget_show (button);

	i = 3;
	hseparator = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),hseparator,0,0+4,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);


	i=4;
	add_label_table(table,_("Charge"),(gushort)i,0);
	add_label_table(table," : ",(gushort)i,1); 
	entrys[3] = gtk_entry_new ();
	gtk_entry_set_text(GTK_ENTRY(entrys[3]),"1.0");
	gtk_widget_set_size_request(entrys[3],30,-1);
	gtk_table_attach(GTK_TABLE(table),entrys[3],2,2+1,i,i+1,
	  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
	  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);

	button = create_button(winParent,_("Add peaks"));
	gtk_table_attach(GTK_TABLE(table),button,3,3+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(addPeaksFromList),(gpointer)treeViewPeaks);
	gtk_widget_show (button);
	addPeaksButton = button;

	i=5;
	add_label_table(table,_("Half-Width"),(gushort)i,0);
	add_label_table(table," : ",(gushort)i,1); 
	entrys[4] = gtk_entry_new ();

	gtk_entry_set_text(GTK_ENTRY(entrys[4]),"0.05");

	gtk_widget_set_size_request(entrys[4],30,-1);
	gtk_table_attach(GTK_TABLE(table),entrys[4],2,2+1,i,i+1,
	  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
	  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);

	button = create_button(winParent,_("Add convoluted curve"));
	gtk_table_attach(GTK_TABLE(table),button,3,3+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(addConvFromList),(gpointer)treeViewPeaks);
	gtk_widget_show (button);
	addConvolutionButton = button;

	i = 6;
	toggleLorentzian = gtk_radio_button_new_with_label( NULL,_("Lorentzian lineshape") );
	gtk_table_attach(GTK_TABLE(table),toggleLorentzian,0,0+3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	gtk_widget_show(toggleLorentzian); 

	toggleGaussian = gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (toggleLorentzian)),_("Gaussian lineshape") );
	gtk_table_attach(GTK_TABLE(table),toggleGaussian,3,3+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	gtk_widget_show(toggleGaussian); 

  	g_object_set_data(G_OBJECT (clearButton), "ListView", treeViewPeaks);
  	g_object_set_data(G_OBJECT (calculateButton), "ListView", treeViewPeaks);
  	g_object_set_data(G_OBJECT (cancelButton), "ListView", treeViewPeaks);
  	g_object_set_data(G_OBJECT (winDlg), "ListView", treeViewPeaks);
  	g_object_set_data(G_OBJECT (addPeaksButton), "ListView", treeViewPeaks);
  	g_object_set_data(G_OBJECT (toggleLorentzian), "ListView", treeViewPeaks);
  	g_object_set_data(G_OBJECT (toggleGaussian), "ListView", treeViewPeaks);
  	g_object_set_data(G_OBJECT (addConvolutionButton), "ListView", treeViewPeaks);
  	g_object_set_data(G_OBJECT (calculateButton), "WinDlg", winDlg);
  	g_object_set_data(G_OBJECT (cancelButton), "WinDlg", winDlg);
  	g_object_set_data(G_OBJECT (winDlg), "TextWidget", textWidget);
  	g_object_set_data(G_OBJECT (treeViewPeaks), "TextWidget", textWidget);
  	g_object_set_data(G_OBJECT (calculateButton), "TextWidget", textWidget);
  	g_object_set_data(G_OBJECT (addPeaksButton), "TextWidget", textWidget);
  	g_object_set_data(G_OBJECT (toggleGaussian), "TextWidget", textWidget);
  	g_object_set_data(G_OBJECT (toggleLorentzian), "TextWidget", textWidget);
  	g_object_set_data(G_OBJECT (addConvolutionButton), "TextWidget", textWidget);
  	g_object_set_data(G_OBJECT (addConvolutionButton), "ToggleLorentzian", toggleLorentzian);
  	g_object_set_data(G_OBJECT (addConvolutionButton), "EntryHalfWidth", entrys[4]);
  	g_object_set_data(G_OBJECT (treeViewPeaks), "EntryCharge", entrys[3]);
  	g_object_set_data(G_OBJECT (treeViewPeaks), "EntryHalfWidth", entrys[4]);
  	g_object_set_data(G_OBJECT (treeViewPeaks), "ToggleLorentzian", toggleLorentzian);
  	g_object_set_data(G_OBJECT (treeViewPeaks), "ToggleGaussian", toggleGaussian);

	frame = gtk_frame_new (NULL);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxRight), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	table = gtk_table_new(1,1,FALSE);
	gtk_box_pack_start(GTK_BOX(vboxframe), table,TRUE,TRUE,0);

        xyplot = gabedit_xyplot_new();
  	g_object_set_data(G_OBJECT (treeViewPeaks), "XYPlot", xyplot);
        gabedit_xyplot_set_range(GABEDIT_XYPLOT(xyplot),  0.0,  10,  0,  20);
	gabedit_xyplot_set_x_legends_digits(GABEDIT_XYPLOT(xyplot), 5);
	gabedit_xyplot_set_y_legends_digits(GABEDIT_XYPLOT(xyplot), 5);


	i = 0;
	gtk_table_attach(GTK_TABLE(table),xyplot,0,0+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
        gtk_widget_show (xyplot);
        g_object_set_data (G_OBJECT (winDlg), "XYPLOT",xyplot);
	gtk_window_set_default_size (GTK_WINDOW(winDlg),4*gdk_screen_width()/5,4*gdk_screen_height()/5);

	gtk_widget_show_all(winDlg);
	gabedit_xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_HMAJOR_GRID, FALSE);
	gabedit_xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_VMAJOR_GRID, FALSE);
	gabedit_xyplot_show_top_legends (GABEDIT_XYPLOT(xyplot),FALSE);
	gabedit_xyplot_show_right_legends (GABEDIT_XYPLOT(xyplot),FALSE);
  	g_object_set_data(G_OBJECT (winDlg), "Entries", entrys);
}
