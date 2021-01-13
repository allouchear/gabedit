/* Printer.c */
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/GabeditTextEdit.h"
#include "Printer.h"
#include "../Files/FileChooser.h"
#include "Windows.h"

#ifndef G_OS_WIN32


static  GtkWidget *FrameOptions = NULL;
static  GtkWidget *VboxOptions = NULL;
static  GtkWidget *Wins;
static  gchar *ProgName = NULL;
static  GtkWidget *ButtonYes;
static  GtkWidget *ButtonNo;
static  EntryPrint entrys;
static  GtkWidget *ButtonDisplay;

void create_print_page();
/********************************************************************************/
void change_of_chars(GtkWidget* w,gpointer data)
{
 gchar *orientation;
 gchar *sheets;

  orientation = g_strdup(gtk_entry_get_text(GTK_ENTRY(entrys.Orientation))); 
  lowercase(orientation);
  sheets = g_strdup(gtk_entry_get_text(GTK_ENTRY(entrys.NbSheets))); 
  if (strstr((gchar *)orientation,_("landscape")) && strstr((gchar *)sheets,"1") )
  		gtk_entry_set_text(GTK_ENTRY(entrys.Format),"140");
  if (strstr((gchar *)orientation,_("landscape")) && strstr((gchar *)sheets,"2") )
  		gtk_entry_set_text(GTK_ENTRY(entrys.Format),"80");
  if ( !strstr((gchar *)orientation,_("landscape")) && strstr((gchar *)sheets,"1") )
  		gtk_entry_set_text(GTK_ENTRY(entrys.Format),"100");
  if ( !strstr((gchar *)orientation,_("landscape")) && strstr((gchar *)sheets,"2") )
  		gtk_entry_set_text(GTK_ENTRY(entrys.Format),"80");
  if(orientation) g_free(orientation);
  if(sheets) g_free(sheets);
}
/********************************************************************************/
void print_file(GtkWidget* w,gpointer data)
{
 G_CONST_RETURN gchar *filename;
 G_CONST_RETURN gchar *printname;
 G_CONST_RETURN gchar *nbcopies;
 gchar *orientation;
 G_CONST_RETURN gchar *format;
 G_CONST_RETURN gchar *sheets;
 G_CONST_RETURN gchar *psfile = NULL;
 gchar *command = NULL;
 gchar *t = NULL;

 filename = gtk_entry_get_text(GTK_ENTRY(entrys.FileToPrint)); 
 printname = gtk_entry_get_text(GTK_ENTRY(entrys.PrinterName)); 
 nbcopies = gtk_entry_get_text(GTK_ENTRY(entrys.NbCopies)); 

 if (!strcmp((gchar *)ProgName,"a2ps") )
 {
  	orientation = g_strdup(gtk_entry_get_text(GTK_ENTRY(entrys.Orientation))); 
  	lowercase(orientation);
  	format = gtk_entry_get_text(GTK_ENTRY(entrys.Format)); 
  	sheets = gtk_entry_get_text(GTK_ENTRY(entrys.NbSheets)); 

  	if (GTK_TOGGLE_BUTTON (ButtonYes)->active || data) 
  	{
        	if(!data)
		{
  			psfile = gtk_entry_get_text(GTK_ENTRY(entrys.FileToCreate)); 
			t = g_strdup_printf("-o%s",psfile);
		}
        	else
		t = g_strdup("-Pdisplay");
  	}
  	else
  	{
   		if (strstr((gchar *)printname,_("Default")) )
			t = g_strdup("   ");
		else
			t = g_strdup_printf("-P%s",printname);
  	}
        if (strstr((gchar *)orientation,_("landscape")) && strstr((gchar *)sheets,"1") )
   		command = g_strdup_printf("a2ps -%s  --chars-per-line=%s -n%s --%s %s %s",sheets,format,nbcopies,orientation,t,filename);
        else
   		command = g_strdup_printf("a2ps -%s --chars-per-line=%s -n%s --%s %s %s",sheets,format,nbcopies,orientation,t,filename);

        g_free(t);
   	t = run_command(command);
   	if(t)
	{
	if(!this_is_a_backspace(t))
	  Message(t,"Info",TRUE);
	 g_free(t);
	}
        g_free(command);
	if(orientation) g_free(orientation);
 }
 else
 {
   if (strstr((gchar *)printname,_("Default")) )
   	command = g_strdup_printf("lpr -#%s %s",nbcopies,filename);
   else
   	command = g_strdup_printf("lpr -P%s -#%s %s",printname,nbcopies,filename);

   t = run_command(command);
   if(t)
   {
	printf("t=%s",t);
	if(!this_is_a_backspace(t))
  		Message(t,_("Info"),TRUE);
  	g_free(t);
   }
   g_free(command);
 }
}
/********************************************************************************/
GtkWidget *file_to_print(GtkWidget* box)
{
  GtkWidget *Entry = NULL;
  GtkWidget *hbox = box;
  GtkWidget *Frame;
  GtkWidget *vbox;
  static gchar* patterns[] = {"*.com *.xyz *.gzmt *.zmt","*.com","*.xyz",
	  			"*.gzmt","*.zmt","*.log","*.out","*",NULL};
  gchar *filename  = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);

  Frame = create_frame(Wins,box,_("File to print")); 
  vbox = create_vbox(Frame);
  hbox = create_hbox(vbox);

  hbox = create_hbox_browser(Wins,vbox,_("File Name :"),filename,patterns);
  Entry = (GtkWidget*)(g_object_get_data(G_OBJECT(hbox),"Entry"));	

  g_free(filename);

  return Entry;
}
/********************************************************************************/
GtkWidget *create_frame_with_list(GtkWidget* box,gchar *title,gchar **liste,gint n)
{
  GtkWidget *Frame;
  GtkWidget *vbox;
  GtkWidget *Entry;

  Frame = create_frame(Wins,box,title); 
  vbox = create_vbox(Frame);
  Entry = create_combo_box_entry_liste(Wins,vbox,NULL,liste,n);
  return Entry;
}
/********************************************************************************/
GtkWidget *create_orientation(GtkWidget* box)
{
  GtkWidget *Entry;
  gchar *liste[2] = {N_("Portrait"),N_("Landscape")};

  Entry = create_frame_with_list(box,_(" Orientation "),liste,2);  
  gtk_widget_set_sensitive(Entry,FALSE); 
  g_signal_connect(G_OBJECT(Entry), "changed",
                             G_CALLBACK(change_of_chars),
                             NULL);

  return Entry;
}
/********************************************************************************/
GtkWidget *create_format(GtkWidget* box)
{
  GtkWidget *Entry;
  gchar *liste[9] = {"70","80","90","100","110","120","130","140","150"};

  Entry = create_frame_with_list(box,_(" chars by line "),liste,9);  
  gtk_widget_set_sensitive(Entry,FALSE); 
  gtk_entry_set_text(GTK_ENTRY(Entry),"100");

  return Entry;
}
/********************************************************************************/
GtkWidget *create_page_by_papier(GtkWidget* box)
{
  GtkWidget *Entry;
  gchar *liste[2] = {"1","2"};

  Entry = create_frame_with_list(box,_(" pages by paper "),liste,2);  
  gtk_widget_set_sensitive(Entry,FALSE); 
  g_signal_connect(G_OBJECT(Entry), "changed", G_CALLBACK(change_of_chars), NULL);

  return Entry;
}
/********************************************************************************/
void show_hbox_file(GtkWidget* win,gpointer hbox)
{
  gtk_widget_show_all(GTK_WIDGET(hbox));
  gtk_widget_set_sensitive(hbox, TRUE);
}
/********************************************************************************/
void hide_hbox_file(GtkWidget* win,gpointer hbox)
{
  gtk_widget_show_all(GTK_WIDGET(hbox));
  gtk_widget_set_sensitive(hbox, FALSE);
}
/********************************************************************************/
GtkWidget *create_print_in_file(GtkWidget* box)
{
  GtkWidget *Entry = NULL;
  GtkWidget *hbox = box;
  GtkWidget *Frame;
  GtkWidget *vbox;
  static gchar* patterns[] = {"*.ps *.eps","*.ps","*.eps","*",NULL};

  Frame = create_frame(Wins,box,_("Print in file")); 
  vbox = create_vbox(Frame);
  hbox = create_hbox(vbox);

  ButtonYes = gtk_radio_button_new_with_label( NULL,_("Yes"));
  gtk_box_pack_start (GTK_BOX (hbox), ButtonYes, TRUE, TRUE, 0);
  gtk_widget_show (ButtonYes);

  ButtonNo = gtk_radio_button_new_with_label(
                       gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonYes)),
                       "No"); 
   gtk_box_pack_start (GTK_BOX (hbox), ButtonNo, TRUE, TRUE, 0);
   gtk_widget_show (ButtonNo);
   hbox = create_hbox_browser(Wins,vbox,_(" File Name :"),"gabedit.ps",patterns);
   Entry = (GtkWidget*)(g_object_get_data(G_OBJECT(hbox),"Entry"));	
  g_signal_connect(G_OBJECT (ButtonYes), "clicked",
                                     G_CALLBACK(show_hbox_file),
                                     hbox);
  g_signal_connect(G_OBJECT (ButtonNo), "clicked",
                                     G_CALLBACK(hide_hbox_file),
                                     hbox);
   gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonNo), TRUE);


  return Entry;
}
/********************************************************************************/
void create_frame_options_a2ps(gchar *title)
{
  GtkWidget *vbox;
  GtkWidget *hbox;

  FrameOptions = create_frame(Wins,VboxOptions,title); 
  vbox = create_vbox(FrameOptions);
  hbox = create_hbox(vbox);
  entrys.Orientation = create_orientation(hbox);
  entrys.NbSheets = create_page_by_papier(hbox);
  entrys.Format = create_format(hbox);
  hbox = create_hbox(vbox);
  entrys.FileToCreate = create_print_in_file(hbox);
}
/********************************************************************************************************/
static void traite_option(GtkComboBox *combobox, gpointer d)
{
	GtkTreeIter iter;
	gchar* data = NULL;
	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	if(ProgName) g_free(ProgName);
	ProgName = g_strdup((gchar *)data);
 
	if(FrameOptions) gtk_widget_set_sensitive(FrameOptions, FALSE);

	if (!strcmp((gchar *)data,"a2ps") )
	{
		if(!FrameOptions) create_frame_options_a2ps(_("a2ps options"));
		else
		gtk_widget_set_sensitive(FrameOptions, TRUE);
		gtk_widget_set_sensitive(ButtonDisplay, TRUE);
	}
	else gtk_widget_set_sensitive(ButtonDisplay, FALSE);
}
/********************************************************************************************************/
static GtkWidget *CreateListeProg(GtkWidget* box)
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;

	store = gtk_tree_store_new (1,G_TYPE_STRING);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "a2ps", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "lpr", -1);

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	g_object_unref (model);
	gtk_box_pack_start (GTK_BOX (box), combobox, TRUE, TRUE, 1);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traite_option), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	return combobox;
}
/********************************************************************************/
GtkWidget *create_prog_frame(GtkWidget* box)
{
  GtkWidget *Frame;
  GtkWidget *vbox;
  GtkWidget *combobox;

  Frame = create_frame(Wins,box," Program "); 
  vbox = create_vbox(Frame);
  combobox = CreateListeProg(vbox);
  g_object_set_data(G_OBJECT(Frame),"ComboBox",combobox);
  return Frame;
}
/********************************************************************************/
GtkWidget *create_name_print_frame(GtkWidget* box)
{
  GtkWidget *Entry;
  gchar *liste[3] = {N_("Default"),"lp0","lp1"};

  Entry = create_frame_with_list(box,_(" Printer Name "),liste,3);

  return Entry;
}
/********************************************************************************/
GtkWidget *create_number_of_copies(GtkWidget* box)
{
  GtkWidget *Entry;
  gchar *liste[10] = {"1","2","3","4","5","6","7","8","9","10"};

  Entry = create_frame_with_list(box,_(" Number of copies "),liste,10);  
  gtk_widget_set_sensitive(Entry,FALSE); 

  return Entry;
}
/********************************************************************************/
void create_print_page()
{
  GtkWidget *Frame;
  GtkWidget *button;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *combobox;

  /*
  Wins =  gtk_print_unix_dialog_new("Gabedit : Print",Fenetre);
  gtk_widget_show(Wins);
  return;
  */



  ProgName = g_strdup("a2ps");
  Wins= gtk_dialog_new ();
  gtk_window_set_position(GTK_WINDOW(Wins),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Wins),GTK_WINDOW(Fenetre));
  gtk_window_set_title(&GTK_DIALOG(Wins)->window,_("Gabedit : Print"));
  gtk_widget_realize(Wins);

  init_child(Wins,gtk_widget_destroy,_(" Print "));
  g_signal_connect(G_OBJECT(Wins),"delete_event",(GCallback)destroy_children,NULL);
 
  Frame = create_frame(Wins,GTK_DIALOG(Wins)->vbox,NULL); 
  vbox = create_vbox(Frame);

  entrys.FileToPrint = file_to_print(vbox);
  hbox = create_hbox(vbox);

  Frame = create_prog_frame(hbox);
  combobox = g_object_get_data(G_OBJECT(Frame),"ComboBox");
  entrys.PrinterName = create_name_print_frame(hbox);
  entrys.NbCopies = create_number_of_copies(hbox);
  create_hseparator(vbox);

  VboxOptions = GTK_DIALOG(Wins)->vbox;
  FrameOptions = NULL;

  button = create_button(Wins,_("Cancel"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Wins)->action_area), button, TRUE, TRUE, 0);
  g_signal_connect_swapped(GTK_OBJECT(button), "clicked",G_CALLBACK(destroy_children),GTK_OBJECT(Wins));
  gtk_widget_show (button);

  button = create_button(Wins,_("Display"));
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(print_file),&button);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Wins)->action_area), button, TRUE, TRUE, 0);
  gtk_widget_show (button);
  ButtonDisplay = button;

  button = create_button(Wins,_("Print"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Wins)->action_area), button, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(print_file),NULL);
  g_signal_connect_swapped(GTK_OBJECT(button), "clicked",G_CALLBACK(destroy_children),GTK_OBJECT(Wins));
  gtk_widget_grab_default(button);
  gtk_widget_show (button);

  gtk_widget_show_all(Wins);
  create_frame_options_a2ps(_("a2ps options"));
  gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 0);
}
/********************************************************************************/
#else /* G_OS_WIN32 */
#include <windows.h>

void create_print_page()
{
	PRINTDLG	pd;
	DOCINFO		di;
	gchar*		szMessage;
	gchar*		temp;
	gint nchar;
	GtkWidget* TextWid;
	guint nlignes;
	guint RowByPage = 50;
	gchar *curent;
	gchar *next ;
	gchar tab[5] = "     " ;
	guint i;
	guint j;
	guint k;

	if( gtk_notebook_get_current_page(GTK_NOTEBOOK(NoteBookText))==0)
		TextWid = text;
	else
		TextWid = textresult;

	
	temp= gabedit_text_get_chars(TextWid,0,-1);

	if(!temp)
	{
		Message(_("Error, No text to print\n"),_("Error"),TRUE);
		return;
	}
	nchar =  gabedit_text_get_length(GABEDIT_TEXT(TextWid));
	if(nchar<1)
	{
		Message(_("Error, No text to print\n"),_("Error"),TRUE);
		return;
	}
	next = temp;
/*	Debug("Nombre Total de characters = %d\n",nchar);*/
	nlignes = 0;
	for(i=0;i<(guint)nchar;i++)
	{
			if(next[i] == '\n')
				nlignes ++;
	}
/*	Debug("Nombre Total de lignes = %d\n",nlignes);*/
	memset (&pd, 0, sizeof(PRINTDLG));
	memset (&di, 0, sizeof(DOCINFO));

	di.cbSize = sizeof(DOCINFO);
	if( gtk_notebook_get_current_page(GTK_NOTEBOOK(NoteBookText))==0)
		di.lpszDocName =   fileopen.datafile;
	else
		di.lpszDocName =   fileopen.outputfile;
	
	pd.lStructSize = sizeof(PRINTDLG);
	pd.Flags = PD_PAGENUMS | PD_RETURNDC;
	pd.nFromPage = 1;
	pd.nToPage = nlignes/RowByPage  +1;
	pd.nMinPage = 1;
	pd.nMaxPage = nlignes/RowByPage +1;

	szMessage = 0;

	if (PrintDlg (&pd))
	{
		if (pd.hDC)
		{
			if (StartDoc (pd.hDC, &di) != SP_ERROR)
			{
				for(i =0 ;i<pd.nMaxPage;i++)
				{
					/* Debug("Page number %d\n",i+1);*/
					curent = next;
					j = 0;
					k = 0;
					StartPage (pd.hDC);
					while(next[0] != '\0' && j<RowByPage)
					{
						if(next[0] == '\n')
						{
							j++;
							k = 0;
							next++;
							continue;
						}
						if(next[0] == '\t')
						{
							TextOut (pd.hDC, 30+k*30, 60+j*60, tab, 5);
							k+=5;
							next++;
							continue;
						}

						TextOut (pd.hDC, 30+k*30, 60+j*60, next, 1);
						k++;
						next++;
					}
					EndPage (pd.hDC);
				}

				EndDoc (pd.hDC);

				szMessage = g_strdup_printf(_("Printed.%d charaters\n"),nchar);
			}
			else
			{
				szMessage = g_strdup(_("Could not start document."));
			}
		}
		else
		{
			szMessage = g_strdup(_("Could not create device context."));
		}
	}
	else
	{
		szMessage = g_strdup(_("Canceled or printer could not be setup."));
	}
	g_free(temp);
	if (szMessage)
	{
		Message(szMessage, _("Info"),TRUE);
		g_free(szMessage);
	}

}
#endif

