/* Run.c */
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
#include <glib.h>

#ifdef G_OS_WIN32
#include <winsock.h>
#else
#endif /* G_OS_WIN32 */

#include <unistd.h>

#include <ctype.h>

#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Common/Windows.h"
#include "../Files/ListeFiles.h"
#include "../Files/FileChooser.h"
#include "../Files/GabeditFolderChooser.h"
#include "../NetWork/RemoteCommand.h"
#include "../NetWork/FilesTransfer.h"
#include "../Molcas/MolcasVariables.h"

static GtkWidget* ButtonLocal = NULL;
static GtkWidget* ButtonRemote = NULL;

static GtkWidget* ButtonDeMon = NULL;
static GtkWidget* ButtonGamess = NULL;
static GtkWidget* ButtonGauss = NULL;
static GtkWidget* ButtonMolcas = NULL;
static GtkWidget* ButtonMolpro = NULL;
static GtkWidget* ButtonMPQC = NULL;
static GtkWidget* ButtonOrca = NULL;
static GtkWidget* ButtonNWChem = NULL;
static GtkWidget* ButtonPsicode = NULL;
static GtkWidget* ButtonFireFly = NULL;
static GtkWidget* ButtonQChem = NULL;
static GtkWidget* ButtonMopac = NULL;
static GtkWidget* ButtonOther = NULL;
static GtkWidget* EntryFileData = NULL;
static GtkWidget* EntryCommand = NULL;
static GtkWidget* ComboCommand = NULL;
static GtkWidget* FrameRemote = NULL;
static GtkWidget* FrameNetWork = NULL;
static GtkWidget* LabelDataFile = NULL;
static GtkWidget* LabelExtFile = NULL;
static GtkWidget *buttonDirSelector = NULL;
static GtkWidget* LabelPassWord1 = NULL;
static GtkWidget* LabelPassWord2 = NULL;
static GtkWidget* EntryPassWord = NULL;
static gint typeButton[4] = {0, 1, 2, 3}; /* local, remote, ftp, ssh */
/********************************************************************************/
static void changedEntryFileData(GtkWidget *entry,gpointer data);
/********************************************************************************/
static gboolean save_local_doc(gchar *NomFichier)
{
	gchar *temp;
	FILE *fd;
	gint i;
 
	fd = FOpen(NomFichier, "wb");
	if(fd == NULL)
	{
		Message("Sorry, I can not save file","Error",TRUE);
		return FALSE;
	}
	temp=gabedit_text_get_chars(text,0,-1);
	for(i=0;i<strlen(temp);i++)
		if(temp[i]=='\r') temp[i] = ' ';
	fprintf(fd,"%s",temp);
	fclose(fd);
	g_free(temp);
	return TRUE;
}
/********************************************************************************/
void set_frame_remote_visibility(GtkWidget *button,gpointer data)
{
	gint* type = NULL;

	if(!button) return;
	if ((GTK_OBJECT_FLAGS((GtkObject*)button) & GTK_IN_DESTRUCTION)) return;
	if ((GTK_OBJECT_FLAGS((GtkObject*)FrameRemote) & GTK_IN_DESTRUCTION)) return;
	if ((GTK_OBJECT_FLAGS((GtkObject*)FrameNetWork) & GTK_IN_DESTRUCTION)) return;

	type = g_object_get_data (G_OBJECT (button), "TypeButton");
	if(!type) return;

	if(*type == typeButton[0] )
	{
		if(GTK_IS_WIDGET(FrameRemote)) gtk_widget_set_sensitive(FrameRemote, FALSE);
		if(GTK_IS_WIDGET(FrameNetWork)) gtk_widget_set_sensitive(FrameNetWork, FALSE);
	}
	else
	if(*type == typeButton[1] )
	{
		if(GTK_IS_WIDGET(FrameRemote)) gtk_widget_set_sensitive(FrameRemote, TRUE);
		if(GTK_IS_WIDGET(FrameNetWork)) gtk_widget_set_sensitive(FrameNetWork, TRUE);
	}
	else
	if(*type == typeButton[2] )
	{
		if(GTK_IS_WIDGET(LabelPassWord1)) gtk_widget_set_sensitive(LabelPassWord1, TRUE);
		if(GTK_IS_WIDGET(LabelPassWord2)) gtk_widget_set_sensitive(LabelPassWord2, TRUE);
		if(GTK_IS_WIDGET(EntryPassWord)) gtk_widget_set_sensitive(EntryPassWord, TRUE);
	}
	else
	if(*type == typeButton[3] )
	{
#ifdef G_OS_WIN32
#else
		if(GTK_IS_WIDGET(LabelPassWord1)) gtk_widget_set_sensitive(LabelPassWord1, FALSE);
		if(GTK_IS_WIDGET(LabelPassWord2)) gtk_widget_set_sensitive(LabelPassWord2, FALSE);
		if(GTK_IS_WIDGET(EntryPassWord)) gtk_widget_set_sensitive(EntryPassWord, FALSE);
#endif
	}
}
#ifdef G_OS_WIN32
/*************************************************************************************************************/
static gchar* listLocalGamess[] = {
        "gamess.11-64.exe 1",
        "gamess.11-32.exe 1",
	"gamess.00.exe", "gamess.01.exe", "gamess.02.exe", "gamess.03.exe", "gamess.04.exe", "gamess.05.exe",
	"gamess.06.exe", "gamess.07.exe", "gamess.08.exe", "gamess.09.exe"
};
static guint numberOfLocalGamess = G_N_ELEMENTS (listLocalGamess);
#endif
/********************************************************************************/
void set_default_entrys(GtkWidget *button,gpointer data)
{
	GList *glist = NULL;
	gint i;

	if (GTK_TOGGLE_BUTTON (button)->active)
	{
		if(button == ButtonGamess )
		{
#ifdef G_OS_WIN32
			if(ButtonLocal && GTK_TOGGLE_BUTTON (ButtonLocal)->active)
			{
  				for(i=0;i<numberOfLocalGamess;i++)
					glist = g_list_append(glist,listLocalGamess[i]);
				if(fileopen.command && !strstr(fileopen.command,"exe"))
				{
					if(fileopen.command) g_free(fileopen.command);
					fileopen.command = g_strdup("gamess.11-64.exe 1");
				}
			}
			else
#endif
  			for(i=0;i<gamessCommands.numberOfCommands;i++)
				glist = g_list_append(glist,gamessCommands.commands[i]);

  			gtk_combo_box_entry_set_popdown_strings( ComboCommand, glist) ;

  			g_list_free(glist);
			gtk_entry_set_text (GTK_ENTRY (EntryCommand), NameCommandGamess);
			if(ButtonLocal && GTK_TOGGLE_BUTTON (ButtonLocal)->active)
			{
			if(fileopen.command && strlen(fileopen.command)>0)
			       	gtk_entry_set_text (GTK_ENTRY (EntryCommand), fileopen.command);
			}
			else
			if(fileopen.command && !strstr(fileopen.command,"gamess.") && strlen(fileopen.command)>0)
			       	gtk_entry_set_text (GTK_ENTRY (EntryCommand), fileopen.command);

			gtk_label_set_text(GTK_LABEL(LabelExtFile), ".inp");
			gtk_widget_show(LabelDataFile);
  			iprogram = PROG_IS_GAMESS;
		}
		else
		if(button == ButtonDeMon )
		{
  			for(i=0;i<demonCommands.numberOfCommands;i++)
				glist = g_list_append(glist,demonCommands.commands[i]);

  			gtk_combo_box_entry_set_popdown_strings( ComboCommand, glist) ;

  			g_list_free(glist);
			gtk_entry_set_text (GTK_ENTRY (EntryCommand), NameCommandDeMon);
			if(fileopen.command && !strstr(fileopen.command,"gamess.") && strlen(fileopen.command)>0)
			       	gtk_entry_set_text (GTK_ENTRY (EntryCommand), fileopen.command);
			gtk_label_set_text(GTK_LABEL(LabelExtFile), ".inp");
			gtk_widget_show(LabelDataFile);
  			iprogram = PROG_IS_DEMON;
		}
		else
		if(button == ButtonGauss )
		{
  			for(i=0;i<gaussianCommands.numberOfCommands;i++)
				glist = g_list_append(glist,gaussianCommands.commands[i]);

  			gtk_combo_box_entry_set_popdown_strings( ComboCommand, glist) ;

  			g_list_free(glist);
			gtk_entry_set_text (GTK_ENTRY (EntryCommand), NameCommandGaussian);
			if(fileopen.command && !strstr(fileopen.command,"gamess.") && strlen(fileopen.command)>0)
			       	gtk_entry_set_text (GTK_ENTRY (EntryCommand), fileopen.command);
			gtk_label_set_text(GTK_LABEL(LabelExtFile), ".com");
			gtk_widget_show(LabelDataFile);
  			iprogram = PROG_IS_GAUSS;
		}
		else
		if(button == ButtonMPQC)
		{
  			for(i=0;i<mpqcCommands.numberOfCommands;i++)
				glist = g_list_append(glist,mpqcCommands.commands[i]);

  			gtk_combo_box_entry_set_popdown_strings( ComboCommand, glist) ;

  			g_list_free(glist);
			gtk_entry_set_text (GTK_ENTRY (EntryCommand), NameCommandMPQC);
			if(fileopen.command && !strstr(fileopen.command,"gamess.") && strlen(fileopen.command)>0)
			       	gtk_entry_set_text (GTK_ENTRY (EntryCommand), fileopen.command);
			gtk_label_set_text(GTK_LABEL(LabelExtFile), ".com");
			gtk_widget_show(LabelDataFile);
  			iprogram = PROG_IS_MPQC;
		}
		else
		if(button == ButtonMolcas)
		{
  			for(i=0;i<molcasCommands.numberOfCommands;i++)
				glist = g_list_append(glist,molcasCommands.commands[i]);

  			gtk_combo_box_entry_set_popdown_strings( ComboCommand, glist) ;

  			g_list_free(glist);
			gtk_entry_set_text (GTK_ENTRY (EntryCommand), NameCommandMolcas);
			if(fileopen.command && !strstr(fileopen.command,"gamess.") && strlen(fileopen.command)>0)
			       	gtk_entry_set_text (GTK_ENTRY (EntryCommand), fileopen.command);
			gtk_label_set_text(GTK_LABEL(LabelExtFile), ".com");
			gtk_widget_show(LabelDataFile);
  			iprogram = PROG_IS_MOLCAS;
		}
		else
		if(button == ButtonMolpro )
		{
  			for(i=0;i<molproCommands.numberOfCommands;i++)
				glist = g_list_append(glist,molproCommands.commands[i]);

  			gtk_combo_box_entry_set_popdown_strings( ComboCommand, glist) ;

  			g_list_free(glist);
			gtk_entry_set_text (GTK_ENTRY (EntryCommand), NameCommandMolpro);
			if(fileopen.command && !strstr(fileopen.command,"gamess.") && strlen(fileopen.command)>0)
			       	gtk_entry_set_text (GTK_ENTRY (EntryCommand), fileopen.command);
			gtk_label_set_text(GTK_LABEL(LabelExtFile), ".com");
			gtk_widget_show(LabelDataFile);
  			iprogram = PROG_IS_MOLPRO;
		}
		else
		if(button == ButtonFireFly )
		{
  			for(i=0;i<fireflyCommands.numberOfCommands;i++)
				glist = g_list_append(glist,fireflyCommands.commands[i]);

  			gtk_combo_box_entry_set_popdown_strings( ComboCommand, glist) ;

  			g_list_free(glist);
			gtk_entry_set_text (GTK_ENTRY (EntryCommand), NameCommandFireFly);
			if(fileopen.command && !strstr(fileopen.command,"gamess.") && strlen(fileopen.command)>0)
			       	gtk_entry_set_text (GTK_ENTRY (EntryCommand), fileopen.command);
			gtk_label_set_text(GTK_LABEL(LabelExtFile), ".inp");
			gtk_widget_show(LabelDataFile);
  			iprogram = PROG_IS_FIREFLY;
		}
		else
		if(button == ButtonOrca )
		{
  			for(i=0;i<orcaCommands.numberOfCommands;i++)
				glist = g_list_append(glist,orcaCommands.commands[i]);

  			gtk_combo_box_entry_set_popdown_strings( ComboCommand, glist) ;

  			g_list_free(glist);
			gtk_entry_set_text (GTK_ENTRY (EntryCommand), NameCommandOrca);
			if(fileopen.command && !strstr(fileopen.command,"gamess.") && strlen(fileopen.command)>0)
			       	gtk_entry_set_text (GTK_ENTRY (EntryCommand), fileopen.command);
			gtk_label_set_text(GTK_LABEL(LabelExtFile), ".inp");
			gtk_widget_show(LabelDataFile);
  			iprogram = PROG_IS_ORCA;
		}
		else
		if(button == ButtonNWChem )
		{
  			for(i=0;i<nwchemCommands.numberOfCommands;i++)
				glist = g_list_append(glist,nwchemCommands.commands[i]);

  			gtk_combo_box_entry_set_popdown_strings( ComboCommand, glist) ;

  			g_list_free(glist);
			gtk_entry_set_text (GTK_ENTRY (EntryCommand), NameCommandNWChem);
			if(fileopen.command && !strstr(fileopen.command,"gamess.") && strlen(fileopen.command)>0)
			       	gtk_entry_set_text (GTK_ENTRY (EntryCommand), fileopen.command);
			gtk_label_set_text(GTK_LABEL(LabelExtFile), ".nw");
			gtk_widget_show(LabelDataFile);
  			iprogram = PROG_IS_NWCHEM;
		}
		else
		if(button == ButtonPsicode )
		{
  			for(i=0;i<nwchemCommands.numberOfCommands;i++)
				glist = g_list_append(glist,nwchemCommands.commands[i]);

  			gtk_combo_box_entry_set_popdown_strings( ComboCommand, glist) ;

  			g_list_free(glist);
			gtk_entry_set_text (GTK_ENTRY (EntryCommand), NameCommandPsicode);
			if(fileopen.command && !strstr(fileopen.command,"gamess.") && strlen(fileopen.command)>0)
			       	gtk_entry_set_text (GTK_ENTRY (EntryCommand), fileopen.command);
			gtk_label_set_text(GTK_LABEL(LabelExtFile), ".psi");
			gtk_widget_show(LabelDataFile);
  			iprogram = PROG_IS_PSICODE;
		}
		else
		if(button == ButtonQChem )
		{
  			for(i=0;i<qchemCommands.numberOfCommands;i++)
				glist = g_list_append(glist,qchemCommands.commands[i]);

  			gtk_combo_box_entry_set_popdown_strings( ComboCommand, glist) ;

  			g_list_free(glist);
			gtk_entry_set_text (GTK_ENTRY (EntryCommand), NameCommandQChem);
			if(fileopen.command && !strstr(fileopen.command,"gamess.") && strlen(fileopen.command)>0)
			       	gtk_entry_set_text (GTK_ENTRY (EntryCommand), fileopen.command);
			gtk_label_set_text(GTK_LABEL(LabelExtFile), ".inp");
			gtk_widget_show(LabelDataFile);
  			iprogram = PROG_IS_QCHEM;
		}
		else
		if(button == ButtonMopac )
		{
  			for(i=0;i<mopacCommands.numberOfCommands;i++)
				glist = g_list_append(glist,mopacCommands.commands[i]);

  			gtk_combo_box_entry_set_popdown_strings( ComboCommand, glist) ;

  			g_list_free(glist);
			gtk_entry_set_text (GTK_ENTRY (EntryCommand), NameCommandMopac);
			if(fileopen.command && !strstr(fileopen.command,"gamess.") && strlen(fileopen.command)>0)
			       	gtk_entry_set_text (GTK_ENTRY (EntryCommand), fileopen.command);
			gtk_label_set_text(GTK_LABEL(LabelExtFile), ".mop");
			gtk_widget_show(LabelDataFile);
  			iprogram = PROG_IS_MOPAC;
		}
		else
		{
#ifdef G_OS_WIN32
			gtk_entry_set_text (GTK_ENTRY (EntryCommand), "type");
			glist = g_list_append(glist,"type");
			glist = g_list_append(glist,"ls -l");
#else
			gtk_entry_set_text (GTK_ENTRY (EntryCommand), "ls -l");
			glist = g_list_append(glist,"ls -l");
#endif
  			gtk_combo_box_entry_set_popdown_strings( ComboCommand, glist) ;
			if(fileopen.command && strlen(fileopen.command)>0) gtk_entry_set_text (GTK_ENTRY (EntryCommand), fileopen.command);
			gtk_widget_hide(LabelDataFile);
  			g_list_free(glist);
  			iprogram = PROG_IS_OTHER;
		}

	}
	changedEntryFileData(EntryFileData,NULL);

}
/********************************************************************************/
GtkWidget* create_text_result_command(GtkWidget* Text[],GtkWidget* Frame[],gchar* title)
{
  GtkWidget *Win;
  GtkWidget *frame;
  GtkWidget *hbox;
  GtkWidget *vboxall;
  GtkWidget *vboxwin;
  GtkWidget *button;


  /* Principal Window */
  Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(Win),title);
  gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(Fenetre));
  gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

  gtk_widget_realize(Win);
  init_child(Win,gtk_widget_destroy," Run ");
  g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)destroy_children,NULL);

  gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
  vboxall = create_vbox(Win);
  vboxwin = vboxall;

  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_add(GTK_CONTAINER(vboxall),frame);
  gtk_widget_show (frame);
  vboxall = create_vbox(frame);

  Text[0] = create_text_widget(vboxall,_("Output"),&Frame[0]);
  set_font (Text[0],FontsStyleResult.fontname);
  set_base_style(Text[0],FontsStyleResult.BaseColor.red ,FontsStyleResult.BaseColor.green ,FontsStyleResult.BaseColor.blue);
  set_text_style(Text[0],FontsStyleResult.TextColor.red ,FontsStyleResult.TextColor.green ,FontsStyleResult.TextColor.blue);
  Text[1] = create_text_widget(vboxall,_("Error"),&Frame[1]);
  set_font (Text[1],FontsStyleResult.fontname);
  set_base_style(Text[1],FontsStyleResult.BaseColor.red ,FontsStyleResult.BaseColor.green ,FontsStyleResult.BaseColor.blue);
  set_text_style(Text[1],FontsStyleResult.TextColor.red ,0,0);
  /* boutons box */
  hbox = gtk_hbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX(vboxwin), hbox, FALSE, FALSE, 5);
  gtk_box_set_homogeneous(GTK_BOX(hbox), FALSE);
  gtk_widget_realize(Win);
  button = create_button(Win,_("OK"));
  gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, FALSE, 5);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)destroy_children,G_OBJECT(Win));
  gtk_widget_show (button);
  gtk_window_set_default_size (GTK_WINDOW(Win), 3*ScreenWidth/5, 3*ScreenHeight/5);
  gtk_widget_set_size_request(GTK_WIDGET(Text[0]),-1,1*ScreenHeight/5);
  return Win;
}
/********************************************************************************/
void put_text_in_texts_widget(GtkWidget* Text[],gchar* fout,gchar* ferr)
{  
  gchar *strout;
  gchar *strerr;
  strout = cat_file(fout,TRUE);
  strerr = cat_file(ferr,TRUE);
  if(strout)
 {
	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,strout,-1);   
	g_free(strout);
 }
 if(strerr)
 {
 	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,strerr,-1);   
	g_free(strerr);
 }
 else
 {
 	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,"Ok",-1);   
 }
}
/********************************************************************************/
void get_file_frome_remote_host(GtkWidget* wid,gpointer data)
{
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);

  GtkWidget* Text[2];
  gchar* temp = NULL;
  gint type = *((gint*)data);

  Text[0] = TextOutput;
  Text[1] = TextError;
  gtk_notebook_set_current_page((GtkNotebook*)NoteBookInfo,1);     
  ClearText(Text[0]);
  ClearText(Text[1]);

/*  Debug("Begin get_file_freom\n");*/
  while( gtk_events_pending() )
          gtk_main_iteration();


  if(fileopen.remotepass)
	g_free(fileopen.remotepass);
  fileopen.remotepass = g_strdup(gtk_entry_get_text(GTK_ENTRY(ResultEntryPass)));
  switch(type)
  {
  case ALLFILES :
  case LOGFILE :
  	/* get file.log */
/*	Debug("Get File frome remote : %s %s %s %s \n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir,fileopen.logfile);*/
  	get_file(fout,ferr,fileopen.logfile,fileopen.localdir,fileopen.remotedir,
  		fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
/*	Debug("End get_file\n");*/
/*	Debug("End Get File frome remote : %s %s %s %s \n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir,fileopen.logfile);*/
	temp = g_strdup_printf(_("\nGet %s file from remote host :\n"),fileopen.logfile);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,temp,-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,temp,-1);   
	g_free(temp);
  	put_text_in_texts_widget(Text,fout,ferr);
  	view_result(NULL,NULL);
	if(type==LOGFILE || iprogram == PROG_IS_GAUSS) break;
	if(type==LOGFILE || iprogram == PROG_IS_GAMESS) break;
  case OUTFILE :
  	/* get file.out */
/*	Debug("Get File frome remote : %s %s %s %s \n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir,fileopen.outputfile);*/
  	get_file(fout,ferr,fileopen.outputfile,fileopen.localdir,fileopen.remotedir,
  		fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
/*	Debug("End Get File frome remote : %s %s %s %s \n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir,fileopen.outputfile);*/
	temp = g_strdup_printf(_("\nGet %s file from remote host :\n"),fileopen.outputfile);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,temp,-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,temp,-1);   
	g_free(temp);
  	put_text_in_texts_widget(Text,fout,ferr);
  	view_result(NULL,NULL);
	if(type==OUTFILE) break;
  case MOLDENFILE :
  	/* get file.molden */
/*	Debug("Get File frome remote : %s %s %s %s \n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir,fileopen.moldenfile);*/
	
	if(iprogram == PROG_IS_GAUSS)
  	get_file(fout,ferr,fileopen.moldenfile,fileopen.localdir,fileopen.remotedir,
  		fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
	else
	if(iprogram == PROG_IS_MOLCAS)
	{
#ifdef G_OS_WIN32
		{
			gchar filename[BSIZE];
			sprintf(filename,"%s.scf.molden",fileopen.projectname);
  			get_file(fout,ferr,filename,fileopen.localdir,fileopen.remotedir,
  				fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);

			temp = g_strdup_printf(_("\nGet %s file from remote host :\n"),filename);
  			gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,temp,-1);   
  			gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,temp,-1);
			g_free(temp);
			put_text_in_texts_widget(Text,fout,ferr);



			sprintf(filename,"%s.geo.molden",fileopen.projectname);
  			get_file(fout,ferr,filename,fileopen.localdir,fileopen.remotedir,
  				fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
			temp = g_strdup_printf(_("\nGet %s file from remote host :\n"),filename);
  			gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,temp,-1);   
  			gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,temp,-1);
			g_free(temp);
			put_text_in_texts_widget(Text,fout,ferr);

			sprintf(filename,"%s.freq.molden",fileopen.projectname);
  			get_file(fout,ferr,filename,fileopen.localdir,fileopen.remotedir,
  				fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
			temp = g_strdup_printf(_("\nGet %s file from remote host :\n"),filename);
  			gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,temp,-1);   
  			gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,temp,-1);
			g_free(temp);
			put_text_in_texts_widget(Text,fout,ferr);

			sprintf(filename,"%s.grid",fileopen.projectname);
  			get_file(fout,ferr,filename,fileopen.localdir,fileopen.remotedir,
  				fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
			temp = g_strdup_printf(_("\nGet %s file from remote host :\n"),filename);
  			gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,temp,-1);   
  			gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,temp,-1);
			g_free(temp);
			put_text_in_texts_widget(Text,fout,ferr);
		}

#else
		if(!strstr(fileopen.moldenfile,"*") || fileopen.netWorkProtocol != GABEDIT_NETWORK_FTP_RSH)
		{
			gchar filename[BSIZE];

  			get_file(fout,ferr,fileopen.moldenfile,fileopen.localdir,fileopen.remotedir,
  				fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
			temp = g_strdup_printf(_("\nGet %s file from remote host :\n"),fileopen.moldenfile);
  			gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,temp,-1);   
  			gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,temp,-1);
			g_free(temp);
			put_text_in_texts_widget(Text,fout,ferr);

			sprintf(filename,"%s.grid",fileopen.projectname);
  			get_file(fout,ferr,filename,fileopen.localdir,fileopen.remotedir,
  				fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
			temp = g_strdup_printf(_("\nGet %s file from remote host :\n"),filename);
  			gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,temp,-1);   
  			gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,temp,-1);
			g_free(temp);
			put_text_in_texts_widget(Text,fout,ferr);


		}
		else
		{
			gchar filename[BSIZE];
			sprintf(filename,"%s.scf.molden",fileopen.projectname);
  			get_file(fout,ferr,filename,fileopen.localdir,fileopen.remotedir,
  				fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);

			temp = g_strdup_printf(_("\nGet %s file from remote host :\n"),filename);
  			gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,temp,-1);   
  			gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,temp,-1);
			g_free(temp);
			put_text_in_texts_widget(Text,fout,ferr);

			sprintf(filename,"%s.geo.molden",fileopen.projectname);
  			get_file(fout,ferr,filename,fileopen.localdir,fileopen.remotedir,
  				fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);

			temp = g_strdup_printf(_("\nGet %s file from remote host :\n"),filename);
			gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,temp,-1);   
  			gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,temp,-1);
			g_free(temp);
			put_text_in_texts_widget(Text,fout,ferr);


			sprintf(filename,"%s.freq.molden",fileopen.projectname);
  			get_file(fout,ferr,filename,fileopen.localdir,fileopen.remotedir,
  				fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
			temp = g_strdup_printf(_("\nGet %s file from remote host :\n"),filename);
  			gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,temp,-1);   
  			gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,temp,-1);
			g_free(temp);
			put_text_in_texts_widget(Text,fout,ferr);

			sprintf(filename,"%s.grid",fileopen.projectname);
  			get_file(fout,ferr,filename,fileopen.localdir,fileopen.remotedir,
  				fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
			temp = g_strdup_printf(_("\nGet %s file from remote host :\n"),filename);
  			gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,temp,-1);   
  			gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,temp,-1);
			g_free(temp);
			put_text_in_texts_widget(Text,fout,ferr);

		}
#endif
	}
	else
	if(iprogram == PROG_IS_MOLPRO)
	{
		lowercase(fileopen.moldenfile);
  		get_file(fout,ferr,fileopen.moldenfile,fileopen.localdir,fileopen.remotedir,
  		fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
		temp = g_strdup_printf(_("\nGet %s file from remote host :\n"),fileopen.moldenfile);
  		gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,temp,-1);   
  		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,temp,-1);   
		g_free(temp);
  		put_text_in_texts_widget(Text,fout,ferr);

	}
/*	Debug("End Get File frome remote : %s %s %s %s \n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir,fileopen.moldenfile);*/
  }

  g_free(fout);
  g_free(ferr);
}
#ifdef G_OS_WIN32
gboolean createGamessCsh(gchar* filename)
{
	FILE* file = fopen(filename,"wb");

	if(file==NULL)
	{
		return FALSE;
	}
	fprintf(file,"#!./csh -f\n");
	fprintf(file,"#  03 Feb 05 - A.R. ALLOUCHE\n");
	fprintf(file,"#\n");
	fprintf(file,"set TARGET=sockets\n");
	fprintf(file,"#\n");
	fprintf(file,"set JOB=$1\n");
	fprintf(file,"set VERNO=$2\n");
	fprintf(file,"set NCPUS=1\n");
	fprintf(file,"set NNODES=1\n");
	fprintf(file,"\n");
	fprintf(file,"set SCR=\"$PWD\\\\scratch\"\n");
	fprintf(file,"set SCK=\"$PWD\\\\temp\"\n");
	fprintf(file,"set HOSTLIST=$3\n");
	fprintf(file,"\n");
	fprintf(file,"if ((-x \"$PWD/temp\") && (-x \"$PWD/scratch\")) then\n");
	fprintf(file,"else\n");
	fprintf(file,"echo \" Error: /temp and /scratch folders are missing \"\n");
	fprintf(file,"echo \"  please create them in WinGAMESS' root folder;\"\n");
	fprintf(file,"echo \"   gabeditRunGms.csh exiting ..     \"\n");
	fprintf(file,"exit\n");
	fprintf(file,"endif\n");
	fprintf(file,"\n");
	fprintf(file,"if (null$VERNO == null) set VERNO=05\n");
	fprintf(file,"if (null$NNODES == null) set NNODES=1\n");
	fprintf(file,"\n");
	fprintf(file,"if ($JOB:r.inp == $JOB) set JOB=$JOB:r\n");
	fprintf(file,"\n");
	fprintf(file,"#  file assignments.\n");
	fprintf(file,"unset echo\n");
	fprintf(file,"setenv GMSPATH \"$PWD\"\n");
	fprintf(file,"setenv GMSJOBNAME $JOB\n");
	fprintf(file,"setenv  master $HOSTLIST\n");
	fprintf(file,"setenv ERICFMT \"$GMSPATH/ericfmt.dat\"\n");
	fprintf(file,"setenv  EXTBAS /dev/null\n");
	fprintf(file,"setenv IRCDATA \"$SCK/$JOB.irc\"\n");
	fprintf(file,"setenv   INPUT \"$SCR/$JOB.F05\"\n");
	fprintf(file,"setenv   PUNCH \"$SCK/$JOB.dat\"\n");
	fprintf(file,"setenv  AOINTS \"$SCR/$JOB.F08\"\n");
	fprintf(file,"setenv  MOINTS \"$SCR/$JOB.F09\"\n");
	fprintf(file,"setenv DICTNRY \"$SCR/$JOB.F10\"\n");
	fprintf(file,"setenv DRTFILE \"$SCR/$JOB.F11\"\n");
	fprintf(file,"setenv CIVECTR \"$SCR/$JOB.F12\"\n");
	fprintf(file,"setenv CASINTS \"$SCR/$JOB.F13\"\n");
	fprintf(file,"setenv  CIINTS \"$SCR/$JOB.F14\"\n");
	fprintf(file,"setenv  WORK15 \"$SCR/$JOB.F15\"\n");
	fprintf(file,"setenv  WORK16 \"$SCR/$JOB.F16\"\n");
	fprintf(file,"setenv CSFSAVE \"$SCR/$JOB.F17\"\n");
	fprintf(file,"setenv FOCKDER \"$SCR/$JOB.F18\"\n");
	fprintf(file,"setenv  WORK19 \"$SCR/$JOB.F19\"\n");
	fprintf(file,"setenv  DASORT \"$SCR/$JOB.F20\"\n");
	fprintf(file,"setenv DFTINTS \"$SCR/$JOB.F21\"\n");
	fprintf(file,"setenv DFTGRID \"$SCR/$JOB.F22\"\n");
	fprintf(file,"setenv  JKFILE \"$SCR/$JOB.F23\"\n");
	fprintf(file,"setenv  ORDINT \"$SCR/$JOB.F24\"\n");
	fprintf(file,"setenv  EFPIND \"$SCR/$JOB.F25\"\n");
	fprintf(file,"setenv PCMDATA \"$SCR/$JOB.F26\"\n");
	fprintf(file,"setenv PCMINTS \"$SCR/$JOB.F27\"\n");
	fprintf(file,"setenv SVPWRK1 \"$SCR/$JOB.F26\"\n");
	fprintf(file,"setenv SVPWRK2 \"$SCR/$JOB.F27\"\n");
	fprintf(file,"setenv  MLTPL  \"$SCR/$JOB.F28\"\n");
	fprintf(file,"setenv  MLTPLT \"$SCR/$JOB.F29\"\n");
	fprintf(file,"setenv  DAFL30 \"$SCR/$JOB.F30\"\n");
	fprintf(file,"setenv  SOINTX \"$SCR/$JOB.F31\"\n");
	fprintf(file,"setenv  SOINTY \"$SCR/$JOB.F32\"\n");
	fprintf(file,"setenv  SOINTZ \"$SCR/$JOB.F33\"\n");
	fprintf(file,"setenv  SORESC \"$SCR/$JOB.F34\"\n");
	fprintf(file,"setenv   SIMEN \"$SCK/$JOB.simen\"\n");
	fprintf(file,"setenv  SIMCOR \"$SCK/$JOB.simcor\"\n");
	fprintf(file,"setenv GCILIST \"$SCR/$JOB.F37\"\n");
	fprintf(file,"setenv HESSIAN \"$SCR/$JOB.F38\"\n");
	fprintf(file,"setenv SOCCDAT \"$SCR/$JOB.F40\"\n");
	fprintf(file,"setenv  AABB41 \"$SCR/$JOB.F41\"\n");
	fprintf(file,"setenv  BBAA42 \"$SCR/$JOB.F42\"\n");
	fprintf(file,"setenv  BBBB43 \"$SCR/$JOB.F43\"\n");
	fprintf(file,"setenv  MCQD50 \"$SCR/$JOB.F50\"\n");
	fprintf(file,"setenv  MCQD51 \"$SCR/$JOB.F51\"\n");
	fprintf(file,"setenv  MCQD52 \"$SCR/$JOB.F52\"\n");
	fprintf(file,"setenv  MCQD53 \"$SCR/$JOB.F53\"\n");
	fprintf(file,"setenv  MCQD54 \"$SCR/$JOB.F54\"\n");
	fprintf(file,"setenv  MCQD55 \"$SCR/$JOB.F55\"\n");
	fprintf(file,"setenv  MCQD56 \"$SCR/$JOB.F56\"\n");
	fprintf(file,"setenv  MCQD57 \"$SCR/$JOB.F57\"\n");
	fprintf(file,"setenv  MCQD58 \"$SCR/$JOB.F58\"\n");
	fprintf(file,"setenv  MCQD59 \"$SCR/$JOB.F59\"\n");
	fprintf(file,"setenv  MCQD60 \"$SCR/$JOB.F60\"\n");
	fprintf(file,"setenv  MCQD61 \"$SCR/$JOB.F61\"\n");
	fprintf(file,"setenv  MCQD62 \"$SCR/$JOB.F62\"\n");
	fprintf(file,"setenv  MCQD63 \"$SCR/$JOB.F63\"\n");
	fprintf(file,"setenv  MCQD64 \"$SCR/$JOB.F64\"\n");
	fprintf(file,"setenv NMRINT1 \"$SCR/$JOB.F61\"\n");
	fprintf(file,"setenv NMRINT2 \"$SCR/$JOB.F62\"\n");
	fprintf(file,"setenv NMRINT3 \"$SCR/$JOB.F63\"\n");
	fprintf(file,"setenv NMRINT4 \"$SCR/$JOB.F64\"\n");
	fprintf(file,"setenv NMRINT5 \"$SCR/$JOB.F65\"\n");
	fprintf(file,"setenv NMRINT6 \"$SCR/$JOB.F66\"\n");
	fprintf(file,"setenv DCPHFH2 \"$SCR/$JOB.F67\"\n");
	fprintf(file,"setenv DCPHF21 \"$SCR/$JOB.F68\"\n");
	fprintf(file,"setenv   GVVPT \"$SCR/$JOB.F69\"\n");
	fprintf(file,"setenv  CCREST \"$SCR/$JOB.F70\"\n");
	fprintf(file,"setenv  CCDIIS \"$SCR/$JOB.F71\"\n");
	fprintf(file,"setenv  CCINTS \"$SCR/$JOB.F72\"\n");
	fprintf(file,"setenv CCT1AMP \"$SCR/$JOB.F73\"\n");
	fprintf(file,"setenv CCT2AMP \"$SCR/$JOB.F74\"\n");
	fprintf(file,"setenv CCT3AMP \"$SCR/$JOB.F75\"\n");
	fprintf(file,"setenv    CCVM \"$SCR/$JOB.F76\"\n");
	fprintf(file,"setenv    CCVE \"$SCR/$JOB.F77\"\n");
	fprintf(file,"setenv CCQUADS \"$SCR/$JOB.F78\"\n");
	fprintf(file,"setenv QUADSVO \"$SCR/$JOB.F79\"\n");
	fprintf(file,"setenv EOMSTAR \"$SCR/$JOB.F80\"\n");
	fprintf(file,"setenv EOMVEC1 \"$SCR/$JOB.F81\"\n");
	fprintf(file,"setenv EOMVEC2 \"$SCR/$JOB.F82\"\n");
	fprintf(file,"setenv  EOMHC1 \"$SCR/$JOB.F83\"\n");
	fprintf(file,"setenv  EOMHC2 \"$SCR/$JOB.F84\"\n");
	fprintf(file,"setenv EOMHHHH \"$SCR/$JOB.F85\"\n");
	fprintf(file,"setenv EOMPPPP \"$SCR/$JOB.F86\"\n");
	fprintf(file,"setenv EOMRAMP \"$SCR/$JOB.F87\"\n");
	fprintf(file,"setenv EOMRTMP \"$SCR/$JOB.F88\"\n");
	fprintf(file,"setenv EOMDG12 \"$SCR/$JOB.F89\"\n");
	fprintf(file,"setenv    MMPP \"$SCR/$JOB.F90\"\n");
	fprintf(file,"setenv   MMHPP \"$SCR/$JOB.F91\"\n");
	fprintf(file,"setenv MMCIVEC \"$SCR/$JOB.F92\"\n");
	fprintf(file,"setenv MMCIVC1 \"$SCR/$JOB.F93\"\n");
	fprintf(file,"setenv MMCIITR \"$SCR/$JOB.F94\"\n");
	fprintf(file,"setenv  EOMVL1 \"$SCR/$JOB.F95\"\n");
	fprintf(file,"setenv  EOMVL2 \"$SCR/$JOB.F96\"\n");
	fprintf(file,"setenv EOMLVEC \"$SCR/$JOB.F97\"\n");
	fprintf(file,"setenv  EOMHL1 \"$SCR/$JOB.F98\"\n");
	fprintf(file,"setenv  EOMHL2 \"$SCR/$JOB.F99\"\n");
	fprintf(file,"setenv  OLI201 \"$SCR/$JOB.F201\"\n");
	fprintf(file,"setenv  OLI202 \"$SCR/$JOB.F202\"\n");
	fprintf(file,"setenv  OLI203 \"$SCR/$JOB.F203\"\n");
	fprintf(file,"setenv  OLI204 \"$SCR/$JOB.F204\"\n");
	fprintf(file,"setenv  OLI205 \"$SCR/$JOB.F205\"\n");
	fprintf(file,"setenv  OLI206 \"$SCR/$JOB.F206\"\n");
	fprintf(file,"setenv  OLI207 \"$SCR/$JOB.F207\"\n");
	fprintf(file,"setenv  OLI208 \"$SCR/$JOB.F208\"\n");
	fprintf(file,"setenv  OLI209 \"$SCR/$JOB.F209\"\n");
	fprintf(file,"setenv  OLI210 \"$SCR/$JOB.F210\"\n");
	fprintf(file,"setenv  OLI211 \"$SCR/$JOB.F211\"\n");
	fprintf(file,"setenv  OLI212 \"$SCR/$JOB.F212\"\n");
	fprintf(file,"setenv  OLI213 \"$SCR/$JOB.F213\"\n");
	fprintf(file,"setenv  OLI214 \"$SCR/$JOB.F214\"\n");
	fprintf(file,"setenv  OLI215 \"$SCR/$JOB.F215\"\n");
	fprintf(file,"setenv  OLI216 \"$SCR/$JOB.F216\"\n");
	fprintf(file,"setenv  OLI217 \"$SCR/$JOB.F217\"\n");
	fprintf(file,"setenv  OLI218 \"$SCR/$JOB.F218\"\n");
	fprintf(file,"setenv  OLI219 \"$SCR/$JOB.F219\"\n");
	fprintf(file,"setenv  OLI220 \"$SCR/$JOB.F220\"\n");
	fprintf(file,"setenv  OLI221 \"$SCR/$JOB.F221\"\n");
	fprintf(file,"setenv  OLI222 \"$SCR/$JOB.F222\"\n");
	fprintf(file,"setenv  OLI223 \"$SCR/$JOB.F223\"\n");
	fprintf(file,"setenv  OLI224 \"$SCR/$JOB.F224\"\n");
	fprintf(file,"setenv  OLI225 \"$SCR/$JOB.F225\"\n");
	fprintf(file,"setenv  OLI226 \"$SCR/$JOB.F226\"\n");
	fprintf(file,"setenv  OLI227 \"$SCR/$JOB.F227\"\n");
	fprintf(file,"setenv  OLI228 \"$SCR/$JOB.F228\"\n");
	fprintf(file,"setenv  OLI229 \"$SCR/$JOB.F229\"\n");
	fprintf(file,"setenv  OLI230 \"$SCR/$JOB.F230\"\n");
	fprintf(file,"setenv  OLI231 \"$SCR/$JOB.F231\"\n");
	fprintf(file,"setenv  OLI232 \"$SCR/$JOB.F232\"\n");
	fprintf(file,"setenv  OLI233 \"$SCR/$JOB.F233\"\n");
	fprintf(file,"setenv  OLI234 \"$SCR/$JOB.F234\"\n");
	fprintf(file,"setenv  OLI235 \"$SCR/$JOB.F235\"\n");
	fprintf(file,"setenv  OLI236 \"$SCR/$JOB.F236\"\n");
	fprintf(file,"setenv  OLI237 \"$SCR/$JOB.F237\"\n");
	fprintf(file,"setenv  OLI238 \"$SCR/$JOB.F238\"\n");
	fprintf(file,"setenv  OLI239 \"$SCR/$JOB.F239\"\n");
	fprintf(file,"setenv  VB2000PATH \"$GMSPATH/VB2000\"\n");
	fprintf(file,"\n");
	fprintf(file,"#\n");
	fprintf(file,"if ((-x \"$GMSPATH/gamess.$VERNO.exe\") && (-x \"$GMSPATH/ddikick.exe\")) then\n");
	fprintf(file,"else\n");
	fprintf(file,"echo The GAMESS executable gamess.$VERNO.exe or else\n");
	fprintf(file,"echo the DDIKICK executable ddikick.exe\n");
	fprintf(file,"echo could not be found in directory $GMSPATH,\n");
	fprintf(file,"echo or else they did not properly link to executable permission.\n");
	fprintf(file,"exit 8\n");
	fprintf(file,"endif\n");
	fprintf(file,"#\n");
	fprintf(file,"#        OK, now we are ready to execute!\n");
	fprintf(file,"#    The kickoff program initiates GAMESS process(es) on all CPUs/nodes.\n");
	fprintf(file,"#\n");
	fprintf(file,"if ((-x \"$GMSPATH/gamess.$VERNO.exe\") && (-x \"$GMSPATH/ddikick.exe\")) then\n");
	fprintf(file,"set echo\n");
	fprintf(file,"\"$GMSPATH/ddikick.exe\" \"$GMSPATH/gamess.$VERNO.exe\" $JOB ");
	fprintf(file,"-ddi $NNODES $NCPUS $HOSTLIST ");
	fprintf(file,"-scr \"$SCR\" < /dev/null\n");
	fprintf(file,"unset echo\n");
	fprintf(file,"else\n");
	fprintf(file,"echo The GAMESS executable gamess.$VERNO.exe or else\n");
	fprintf(file,"echo the DDIKICK executable ddikick.exe\n");
	fprintf(file,"echo could not be found in directory $GMSPATH,\n");
	fprintf(file,"echo or else they did not properly link to executable permission.\n");
	fprintf(file,"exit 8\n");
	fprintf(file,"endif\n");
	fprintf(file,"echo ----- accounting info -----\n");
	fprintf(file,"exit\n");
	fclose(file);
	return TRUE;
}
gboolean createGamessBat(gchar* filename, gchar* command, gint nproc)
{
	FILE* file = fopen(filename,"wb");

	if(file==NULL)
	{
		return FALSE;
	}
	fprintf(file,"rem @echo off\n");
	fprintf(file,"set nameLog=%c~n1.log\n",'%');
	fprintf(file,"set nameFile=%c~n1\n",'%');

	fflush(file);
	fprintf(file,"set PATH=%s;%s\n",gamessDirectory,g_getenv("PATH"));
	fflush(file);
	fprintf(file,"set WDIR=%c~dp1\n",'%');
	fprintf(file,"set GAMESSDIR=%s\n",gamessDirectory);
	fprintf(file,"set AUXDATADIR=%cGAMESSDIR%c\\auxdata\n",'%','%');
	fprintf(file,"set RESTARTDIR=%cWDIR%c\\scr%cnameFile%c\n",'%','%','%','%');
	fprintf(file,"set SCRATCHDIR=%cWDIR%c\\tmp%cnameFile%c\n",'%','%','%','%');
	fprintf(file,"cd %cDIR%c\n",'%','%');
	fprintf(file,"del rungms.gms\n");
	fprintf(file,"echo GAMESSDIR=%cGAMESSDIR%c> rungms.gms\n",'%','%');
	fprintf(file,"echo AUXDATADIR=%cAUXDATADIR%c>> rungms.gms\n",'%','%');
	fprintf(file,"echo RESTARTDIR=%cRESTARTDIR%c>> rungms.gms\n",'%','%');
	fprintf(file,"echo SCRARTCHDIR=%cSCRATCHDIR%c>> rungms.gms\n",'%','%');
	fflush(file);

	fprintf(file,"mkdir %cRESTARTDIR%c\n",'%','%');
	fprintf(file,"mkdir %cSCRATCHDIR%c\n",'%','%');
	fflush(file);

	if(strstr(command,"11-64"))
	fprintf(file,"%s\\rungms.bat %c~n1.inp 11-64 %d 0 %cnameLog%c\n",gamessDirectory,'%',nproc,'%','%');
	else
	fprintf(file,"%s\\rungms.bat %c~n1.inp 11-32 %d 0 %cnameLog%c\n",gamessDirectory,'%',nproc,'%','%');

	fprintf(file,"del/S/F/P %cRESTARTDIR%c\n",'%','%');
	fprintf(file,"rmdir %cRESTARTDIR%c\n",'%','%');
	fprintf(file,"del/S/F/P %cSCRATCHDIR%c\n",'%','%');
	fprintf(file,"rmdir %cSCRATCHDIR%c\n",'%','%');

	fclose(file);
	return TRUE;
}
#endif
/***********************************************************************************************************/
static gboolean create_cmd_firefly(G_CONST_RETURN gchar* command, gboolean local, gchar* cmddir, gchar* cmdfile, gchar* cmdall)
{
	gchar* commandStr = g_strdup(command);
        FILE* fcmd = NULL;
#ifndef G_OS_WIN32
	gchar buffer[BSIZE];
#endif
	delete_last_spaces(commandStr);
	delete_first_spaces(commandStr);

	if(local)
  		sprintf(cmddir,"%s", fileopen.localdir);
	else
		sprintf(cmddir,"%s%stmp", gabedit_directory(), G_DIR_SEPARATOR_S);

#ifndef G_OS_WIN32
	sprintf(cmdfile,"%s.cmd", fileopen.projectname);
#else
	if(!local)
		sprintf(cmdfile,"%s.cmd", fileopen.projectname);
	else
		sprintf(cmdfile,"%s.bat", fileopen.projectname);
#endif
  	sprintf(cmdall,"%s%s%s",cmddir,G_DIR_SEPARATOR_S,cmdfile);


  	fcmd = FOpen(cmdall, "w");
	if(!fcmd)
	{
		if(local)
  			Message(_("\nI can not create cmd file\n"),_("Error"),TRUE);   
		return FALSE;
	}
#ifndef G_OS_WIN32
	fprintf(fcmd,"#!/bin/sh\n");
#endif
	if(local)
	{
#ifdef G_OS_WIN32
		addUnitDisk(fcmd, fileopen.localdir);
#endif
		fprintf(fcmd,"cd %s\n", fileopen.localdir);
	}


#ifdef G_OS_WIN32
	if(local)
	{
		if(
			!strcmp(commandStr,"pcgamess") || !strcmp(commandStr,"nohup pcgamess")
	   	     || !strcmp(commandStr,"firefly") || !strcmp(commandStr,"nohup firefly")
		)
		{
			fprintf(fcmd,"set RND=%cRANDOM%c\n",'%','%');
			fprintf(fcmd,"mkdir \"%s\\tmp\"\n",g_get_home_dir());
			fprintf(fcmd,"mkdir \"%s\\tmp\\%cRND%c%s\"\n",g_get_home_dir(),'%','%',fileopen.projectname);
			addUnitDisk(fcmd, g_get_home_dir());
			fprintf(fcmd,"cd \"%s\\tmp\\%cRND%c%s\"\n",g_get_home_dir(),'%','%',fileopen.projectname);
			fprintf(fcmd,"copy \"%s\\%s\" \"%s\\tmp\\%cRND%c%s\\input\"\n",fileopen.localdir,fileopen.datafile,g_get_home_dir(),'%','%',fileopen.projectname);
			fprintf(fcmd,"%s -o \"%s\\%s.log\"\n",commandStr,fileopen.localdir,fileopen.projectname);
			fprintf(fcmd,"move \"%s\\tmp\\%cRND%c%s\\PUNCH\" \"%s\\%s.pun\"\n",g_get_home_dir(),'%','%',fileopen.projectname, fileopen.localdir,fileopen.projectname);
			fprintf(fcmd,"move \"%s\\tmp\\%cRND%c%s\\IRCDATA\" \"%s\\%s.irc\"\n",g_get_home_dir(),'%','%',fileopen.projectname, fileopen.localdir,fileopen.projectname);
			addUnitDisk(fcmd, fileopen.localdir);
			fprintf(fcmd,"cd \"%s\"\n",fileopen.localdir);
			fprintf(fcmd,"del /Q \"%s\\tmp\\%cRND%c%s\\*\"\n",g_get_home_dir(),'%','%',fileopen.projectname);
			fprintf(fcmd,"rmdir \"%s\\tmp\\%cRND%c%s\"\n",g_get_home_dir(),'%','%',fileopen.projectname);
			fprintf(fcmd,"exit\n");
		}
		else
		{
			fprintf(fcmd,"%s -i %s\\%s\n",command,fileopen.localdir,fileopen.datafile);
			fprintf(fcmd,"exit\n");
		}
	}
	else
	{
		if(
			!strcmp(commandStr,"pcgamess") || !strcmp(commandStr,"nohup pcgamess")
	   	     || !strcmp(commandStr,"firefly") || !strcmp(commandStr,"nohup firefly")
		)
		{
			fprintf(fcmd,"#!/bin/sh\n");
			if(fileopen.remotedir[0]!='/') 
				fprintf(fcmd,"export DEFAULTDIR=$HOME/%s\n",fileopen.remotedir);
			else 
				fprintf(fcmd,"export DEFAULTDIR=%s\n",fileopen.remotedir);
			fprintf(fcmd,"cd $DEFAULTDIR\n");
			fprintf(fcmd,"export fileinput=%s\n",fileopen.datafile);
			fprintf(fcmd,"export filename=%s\n",fileopen.projectname);
			fprintf(fcmd,"testTMPDIR=\"x$FIREFLY_TMPDIR\"\n");
			fprintf(fcmd,"if [ $testTMPDIR = \"x\" ]\n");
			fprintf(fcmd,"then\n");
			fprintf(fcmd,"export FIREFLY_TMPDIR=$HOME/tmp\n");
			fprintf(fcmd,"fi\n");
			fprintf(fcmd,"export FIREFLYDIR=$FIREFLY_TMPDIR/$RANDOM\n");
			fprintf(fcmd,"if [ ! -s \"$FIREFLY_TMPDIR\" ]\n");
			fprintf(fcmd,"then\n");
			fprintf(fcmd,"mkdir $FIREFLY_TMPDIR\n");
			fprintf(fcmd,"fi\n");
			fprintf(fcmd,"mkdir $FIREFLYDIR\n");
			fprintf(fcmd,"cd $FIREFLYDIR\n");
			fprintf(fcmd,"cp $DEFAULTDIR/$fileinput input\n");
			fprintf(fcmd,"%s -o $DEFAULTDIR/$filename.log\n",commandStr);
			fprintf(fcmd,"mv -f $FIREFLYDIR/PUNCH $DEFAULTDIR/$filename.pun\n");
			fprintf(fcmd,"if [ -s \"$FIREFLYDIR/IRCDATA\" ]\n");
			fprintf(fcmd,"then\n");
			fprintf(fcmd,"mv -f $FIREFLYDIR/IRCDATA $DEFAULTDIR/$filename.irc\n");
			fprintf(fcmd,"fi\n");
			fprintf(fcmd,"cd $DEFAULTDIR\n");
			fprintf(fcmd,"/bin/rm -r $FIREFLYDIR\n");
		}
		else
		{
			fprintf(fcmd,"%s %s &\n",command,fileopen.datafile);
			fprintf(fcmd,"exit\n");
	 	}
	}
#else
	if(
		!strcmp(commandStr,"pcgamess") || !strcmp(commandStr,"nohup pcgamess")
   	     || !strcmp(commandStr,"firefly") || !strcmp(commandStr,"nohup firefly")
	)
	{
		if(local) fprintf(fcmd,"export DEFAULTDIR=%s\n",fileopen.localdir);
		else
		{
			if(fileopen.remotedir[0]!='/') 
				fprintf(fcmd,"export DEFAULTDIR=$HOME/%s\n",fileopen.remotedir);
			else 
				fprintf(fcmd,"export DEFAULTDIR=%s\n",fileopen.remotedir);
		}
		fprintf(fcmd,"cd $DEFAULTDIR\n");
		fprintf(fcmd,"export fileinput=%s\n",fileopen.datafile);
		fprintf(fcmd,"export filename=%s\n",fileopen.projectname);
		fprintf(fcmd,"testTMPDIR=\"x$FIREFLY_TMPDIR\"\n");
		fprintf(fcmd,"if [ $testTMPDIR = \"x\" ]\n");
		fprintf(fcmd,"then\n");
		fprintf(fcmd,"export FIREFLY_TMPDIR=$HOME/tmp\n");
		fprintf(fcmd,"fi\n");
		fprintf(fcmd,"export FIREFLYDIR=$FIREFLY_TMPDIR/$RANDOM\n");
		fprintf(fcmd,"if [ ! -s \"$FIREFLY_TMPDIR\" ]\n");
		fprintf(fcmd,"then\n");
		fprintf(fcmd,"mkdir $FIREFLY_TMPDIR\n");
		fprintf(fcmd,"fi\n");
		fprintf(fcmd,"mkdir $FIREFLYDIR\n");
		fprintf(fcmd,"cd $FIREFLYDIR\n");
		fprintf(fcmd,"cp $DEFAULTDIR/$fileinput input\n");
		fprintf(fcmd,"%s -o $DEFAULTDIR/$filename.log\n",commandStr);
		fprintf(fcmd,"mv -f $FIREFLYDIR/PUNCH $DEFAULTDIR/$filename.pun\n");
		fprintf(fcmd,"if [ -s \"$FIREFLYDIR/IRCDATA\" ]\n");
		fprintf(fcmd,"then\n");
		fprintf(fcmd,"mv -f $FIREFLYDIR/IRCDATA $DEFAULTDIR/$filename.irc\n");
		fprintf(fcmd,"fi\n");
		fprintf(fcmd,"cd $DEFAULTDIR\n");
		fprintf(fcmd,"/bin/rm -r $FIREFLYDIR\n");
	}
	else
	{
	 if(!local)
	 {
		 fprintf(fcmd,"%s %s &\n",command,fileopen.datafile);
		 fprintf(fcmd,"exit\n");
	 }
	 else
	 {
		 fprintf(fcmd,"%s %s\n",command,fileopen.datafile);
		 fprintf(fcmd,_("echo Job %s.inp finished.\n"),fileopen.projectname);
	 }
	}

#endif
	fclose(fcmd);
#ifndef G_OS_WIN32
  	sprintf(buffer,"chmod u+x %s",cmdall);
	{ int i = system(buffer);}
#endif
	if(commandStr) g_free(commandStr);
	return TRUE;
}
/***********************************************************************************************************/
static gboolean create_runDeMon_bash()
{
  	FILE* file = FOpen("runDeMon", "w");
	if(!file) return FALSE;
	fprintf(file,"#!/bin/bash\n");
	fprintf(file,"filename=$1 ; [ 0 = $# ] || shift\n");
	fprintf(file,"\n");
	fprintf(file,"if [ -z \"$filename\" ]\n");
	fprintf(file,"then\n");
	fprintf(file,"   clear\n");
	fprintf(file,"   echo \" \"\n");
	fprintf(file,"   echo \"Usage Example:\"\n");
	fprintf(file,"   echo \"       runDeMon h2o.inp \"\n");
	fprintf(file,"   echo \"       input file : h2o.inp\"\n");
	fprintf(file,"   echo \"===========================================\"\n");
	fprintf(file,"   echo \" \"\n");
	fprintf(file,"   exit 1\n");
	fprintf(file,"fi\n");
	fprintf(file,"fileinp=$filename\n");
	fprintf(file,"filename=${fileinp%c.inp}\n",'%');
	fprintf(file,"filename=${filename%c.com}\n",'%');
	fprintf(file,"\n");
	fprintf(file,"[ -z \"$CREX_ROOT\" ] && echo \"Error you must define CREX_ROOT global variable\" && echo \"deMon must be installed in \\$CREX_ROOT/DeMon\" && echo \"The name of executable must be binary\" && exit 1\n");
	fprintf(file,"\n");
	fprintf(file,"[ -z \"$TMPDIR\" ] && export TMPDIR=/tmp\n");
	fprintf(file,"\n");
	fprintf(file,"DEFAULTDIR=`pwd`\n");
	fprintf(file,"fn=`echo \"$filename\" | sed \"s/.*\\///\" | sed \"s/\\..*//\"`\n");
	fprintf(file,"cpid=$$\n");
	fprintf(file,"export DEMONTMP=$TMPDIR/$fn_$cpid\n");
	fprintf(file,"mkdir $DEMONTMP\n");
	fprintf(file,"\n");
	fprintf(file,"# echo $DEMONTMP\n");
	fprintf(file,"\n");
	fprintf(file,"# echo Begin $filename\n");
	fprintf(file,"# echo DEMONTMP=$DEMONTMP\n");
	fprintf(file,"cp  $fileinp  $DEMONTMP/deMon.inp\n");
	fprintf(file,"cp $CREX_ROOT/deMon/AUXIS $DEMONTMP/.\n");
	fprintf(file,"cp $CREX_ROOT/deMon/BASIS $DEMONTMP/.\n");
	fprintf(file,"cp $CREX_ROOT/deMon/ECPS $DEMONTMP/. \n");
	fprintf(file,"cp $CREX_ROOT/deMon/MECPS $DEMONTMP/. 2>/dev/null\n");
	fprintf(file,"\n");
	fprintf(file,"cd  $DEMONTMP\n");
	fprintf(file,"touch deMon.out\n");
	fprintf(file,"touch deMon.mol\n");
	fprintf(file,"rm $DEFAULTDIR/${fn}.out $DEFAULTDIR/${fn}.molden 2>/dev/null\n");
	fprintf(file,"ln -s $DEMONTMP/deMon.out  $DEFAULTDIR/${fn}.out\n");
	fprintf(file,"ln -s $DEMONTMP/deMon.mol  $DEFAULTDIR/${fn}.molden\n");
	fprintf(file,"$CREX_ROOT/deMon/binary \n");
	fprintf(file,"rm $DEFAULTDIR/${fn}.out $DEFAULTDIR/${fn}.molden 2>/dev/null\n");
	fprintf(file,"cp $DEMONTMP/deMon.out $DEFAULTDIR/${fn}.out\n");
	fprintf(file,"cp $DEMONTMP/deMon.mol $DEFAULTDIR/${fn}.molden\n");
	fprintf(file,"cd $DEFAULTDIR\n");
	fprintf(file,"#echo End $filename\n");
	fprintf(file,"\n");
	fprintf(file,"/bin/rm -r $DEMONTMP\n");
	fclose(file);
	return TRUE;
}
/***********************************************************************************************************/
static gboolean create_cmd_demon(G_CONST_RETURN gchar* command, gboolean local, gchar* cmddir, gchar* cmdfile, gchar* cmdall)
{
        FILE* fcmd = NULL;
	gchar* commandStr = g_strdup(command);
#ifndef G_OS_WIN32
	gchar buffer[BSIZE];
#endif
	delete_last_spaces(commandStr);
	delete_first_spaces(commandStr);

	if(local)
  		sprintf(cmddir,"%s", fileopen.localdir);
	else
		sprintf(cmddir,"%s%stmp", gabedit_directory(), G_DIR_SEPARATOR_S);

#ifndef G_OS_WIN32
	sprintf(cmdfile,"%s.cmd", fileopen.projectname);
#else
	if(!local) sprintf(cmdfile,"%s.cmd", fileopen.projectname);
	else sprintf(cmdfile,"%s.bat", fileopen.projectname);
#endif
  	sprintf(cmdall,"%s%s%s",cmddir,G_DIR_SEPARATOR_S,cmdfile);

  	fcmd = FOpen(cmdall, "w");
	if(!fcmd)
	{
		if(local) Message(_("\nI can not create cmd file\n"),_("Error"),TRUE);   
		return FALSE;
	}
#ifndef G_OS_WIN32
	fprintf(fcmd,"#!/bin/sh\n");
#else
	if(local)
	{
		if(strstr(demonDirectory,"\"")) fprintf(fcmd,"set PATH=%s;%cPATH%c\n",demonDirectory,'%','%');
		else fprintf(fcmd,"set PATH=\"%s\";%cPATH%c\n",demonDirectory,'%','%');
	}
#endif

	if(local) 
	{
#ifdef G_OS_WIN32
		addUnitDisk(fcmd, fileopen.localdir);
#endif
		fprintf(fcmd,"cd %s\n", fileopen.localdir);
	}
	else fprintf(fcmd,"cd %s\n", fileopen.remotedir);


	if(!strcmp(commandStr,"default"))
	{
		create_runDeMon_bash();
		fprintf(fcmd,"chmod u+x ./runDeMon ; nohup ./runDeMon %s &\n",fileopen.datafile);
		fprintf(fcmd,"exit\n");
	}
	else
	{
		 fprintf(fcmd,"%s %s &\n",command,fileopen.datafile);
		 fprintf(fcmd,"exit\n");
	}
	fclose(fcmd);
#ifndef G_OS_WIN32
  	sprintf(buffer,"chmod u+x %s",cmdall);
	{ int i = system(buffer);}
#endif
	if(commandStr) g_free(commandStr);
	return TRUE;
}
/***********************************************************************************************************/
static gboolean create_cmd_orca(G_CONST_RETURN gchar* command, gboolean local, gchar* cmddir, gchar* cmdfile, gchar* cmdall)
{
        FILE* fcmd = NULL;
	gchar* commandStr = g_strdup(command);
#ifndef G_OS_WIN32
	gchar buffer[BSIZE];
#endif
	delete_last_spaces(commandStr);
	delete_first_spaces(commandStr);

	if(local)
  		sprintf(cmddir,"%s", fileopen.localdir);
	else
		sprintf(cmddir,"%s%stmp", gabedit_directory(), G_DIR_SEPARATOR_S);

#ifndef G_OS_WIN32
	sprintf(cmdfile,"%s.cmd", fileopen.projectname);
#else
	if(!local) sprintf(cmdfile,"%s.cmd", fileopen.projectname);
	else sprintf(cmdfile,"%s.bat", fileopen.projectname);
#endif
  	sprintf(cmdall,"%s%s%s",cmddir,G_DIR_SEPARATOR_S,cmdfile);

  	fcmd = FOpen(cmdall, "w");
	if(!fcmd)
	{
		if(local) Message(_("\nI can not create cmd file\n"),_("Error"),TRUE);   
		return FALSE;
	}
#ifndef G_OS_WIN32
	fprintf(fcmd,"#!/bin/sh\n");
#else
	if(local)
	{
		if(strstr(orcaDirectory,"\"")) fprintf(fcmd,"set PATH=%s;%cPATH%c\n",orcaDirectory,'%','%');
		else fprintf(fcmd,"set PATH=\"%s\";%cPATH%c\n",orcaDirectory,'%','%');
	}
#endif

	if(local) 
	{
#ifdef G_OS_WIN32
		addUnitDisk(fcmd, fileopen.localdir);
#endif
		fprintf(fcmd,"cd %s\n", fileopen.localdir);
	}
	else fprintf(fcmd,"cd %s\n", fileopen.remotedir);


	if(!strcmp(commandStr,"orca") || !strcmp(commandStr,"nohup orca"))
	{
		fprintf(fcmd,"%s %s > %s.out &\n",command,fileopen.datafile,fileopen.projectname);
		fprintf(fcmd,"exit\n");
	}
	else
	{
		 fprintf(fcmd,"%s %s &\n",command,fileopen.datafile);
		 fprintf(fcmd,"exit\n");
	}
	fclose(fcmd);
#ifndef G_OS_WIN32
  	sprintf(buffer,"chmod u+x %s",cmdall);
	{ int i = system(buffer);}
#endif
	if(commandStr) g_free(commandStr);
	return TRUE;
}
/***********************************************************************************************************/
static gboolean create_cmd_nwchem(G_CONST_RETURN gchar* command, gboolean local, gchar* cmddir, gchar* cmdfile, gchar* cmdall)
{
        FILE* fcmd = NULL;
	gchar* commandStr = g_strdup(command);
#ifndef G_OS_WIN32
	gchar buffer[BSIZE];
#endif
	delete_last_spaces(commandStr);
	delete_first_spaces(commandStr);

	if(local)
  		sprintf(cmddir,"%s", fileopen.localdir);
	else
		sprintf(cmddir,"%s%stmp", gabedit_directory(), G_DIR_SEPARATOR_S);

#ifndef G_OS_WIN32
	sprintf(cmdfile,"%s.cmd", fileopen.projectname);
#else
	if(!local) sprintf(cmdfile,"%s.cmd", fileopen.projectname);
	else sprintf(cmdfile,"%s.bat", fileopen.projectname);
#endif
  	sprintf(cmdall,"%s%s%s",cmddir,G_DIR_SEPARATOR_S,cmdfile);

  	fcmd = FOpen(cmdall, "w");
	if(!fcmd)
	{
		if(local) Message(_("\nI can not create cmd file\n"),_("Error"),TRUE);   
		return FALSE;
	}
#ifndef G_OS_WIN32
	fprintf(fcmd,"#!/bin/sh\n");
#else
	if(local)
	{
		if(strstr(nwchemDirectory,"\"")) fprintf(fcmd,"set PATH=%s;%cPATH%c\n",nwchemDirectory,'%','%');
		else fprintf(fcmd,"set PATH=\"%s\";%cPATH%c\n",nwchemDirectory,'%','%');
	}
#endif

	if(local) 
	{
#ifdef G_OS_WIN32
		addUnitDisk(fcmd, fileopen.localdir);
#endif
		fprintf(fcmd,"cd %s\n", fileopen.localdir);
	}
	else fprintf(fcmd,"cd %s\n", fileopen.remotedir);


	if(!strcmp(commandStr,"nwchem") || !strcmp(commandStr,"nohup nwchem"))
	{
#ifdef G_OS_WIN32
		fprintf(fcmd,"del %s.db\n",fileopen.projectname);
#else
		fprintf(fcmd,"rm -f %s.db\n",fileopen.projectname);
#endif
		fprintf(fcmd,"%s %s > %s.out &\n",command,fileopen.datafile,fileopen.projectname);
		fprintf(fcmd,"exit\n");
	}
	else
	{
		 fprintf(fcmd,"%s %s &\n",command,fileopen.datafile);
		 fprintf(fcmd,"exit\n");
	}
	fclose(fcmd);
#ifndef G_OS_WIN32
  	sprintf(buffer,"chmod u+x %s",cmdall);
	{ int i = system(buffer);}
#endif
	if(commandStr) g_free(commandStr);
	return TRUE;
}
/***********************************************************************************************************/
static gboolean create_cmd_psicode(G_CONST_RETURN gchar* command, gboolean local, gchar* cmddir, gchar* cmdfile, gchar* cmdall)
{
        FILE* fcmd = NULL;
	gchar* commandStr = g_strdup(command);
#ifndef G_OS_WIN32
	gchar buffer[BSIZE];
#endif
	delete_last_spaces(commandStr);
	delete_first_spaces(commandStr);

	if(local)
  		sprintf(cmddir,"%s", fileopen.localdir);
	else
		sprintf(cmddir,"%s%stmp", gabedit_directory(), G_DIR_SEPARATOR_S);

#ifndef G_OS_WIN32
	sprintf(cmdfile,"%s.cmd", fileopen.projectname);
#else
	if(!local) sprintf(cmdfile,"%s.cmd", fileopen.projectname);
	else sprintf(cmdfile,"%s.bat", fileopen.projectname);
#endif
  	sprintf(cmdall,"%s%s%s",cmddir,G_DIR_SEPARATOR_S,cmdfile);

  	fcmd = FOpen(cmdall, "w");
	if(!fcmd)
	{
		if(local) Message(_("\nI can not create cmd file\n"),_("Error"),TRUE);   
		return FALSE;
	}
#ifndef G_OS_WIN32
	fprintf(fcmd,"#!/bin/sh\n");
#else
	if(local)
	{
		if(strstr(psicodeDirectory,"\"")) fprintf(fcmd,"set PATH=%s;%cPATH%c\n",psicodeDirectory,'%','%');
		else fprintf(fcmd,"set PATH=\"%s\";%cPATH%c\n",psicodeDirectory,'%','%');
	}
#endif

	if(local) 
	{
#ifdef G_OS_WIN32
		addUnitDisk(fcmd, fileopen.localdir);
#endif
		fprintf(fcmd,"cd %s\n", fileopen.localdir);
	}
	else fprintf(fcmd,"cd %s\n", fileopen.remotedir);


	if(!strcmp(commandStr,"psi4") || !strcmp(commandStr,"nohup psi4"))
	{
#ifdef G_OS_WIN32
		fprintf(fcmd,"del %s.db\n",fileopen.projectname);
#else
		fprintf(fcmd,"rm -f %s.db\n",fileopen.projectname);
#endif
		fprintf(fcmd,"%s -i %s -o %s.out &\n",command,fileopen.datafile,fileopen.projectname);
		fprintf(fcmd,"exit\n");
	}
	else
	{
		 fprintf(fcmd,"%s %s &\n",command,fileopen.datafile);
		 fprintf(fcmd,"exit\n");
	}
	fclose(fcmd);
#ifndef G_OS_WIN32
  	sprintf(buffer,"chmod u+x %s",cmdall);
	{ int i = system(buffer);}
#endif
	if(commandStr) g_free(commandStr);
	return TRUE;
}
/***********************************************************************************************************/
static gboolean create_cmd_qchem(G_CONST_RETURN gchar* command, gboolean local, gchar* cmddir, gchar* cmdfile, gchar* cmdall)
{
        FILE* fcmd = NULL;
	gchar* commandStr = g_strdup(command);
#ifndef G_OS_WIN32
	gchar buffer[BSIZE];
#endif
	delete_last_spaces(commandStr);
	delete_first_spaces(commandStr);

	if(local)
  		sprintf(cmddir,"%s", fileopen.localdir);
	else
		sprintf(cmddir,"%s%stmp", gabedit_directory(), G_DIR_SEPARATOR_S);

#ifndef G_OS_WIN32
	sprintf(cmdfile,"%s.cmd", fileopen.projectname);
#else
	if(!local)
		sprintf(cmdfile,"%s.cmd", fileopen.projectname);
	else
		sprintf(cmdfile,"%s.bat", fileopen.projectname);
#endif
  	sprintf(cmdall,"%s%s%s",cmddir,G_DIR_SEPARATOR_S,cmdfile);


  	fcmd = FOpen(cmdall, "w");
	if(!fcmd)
	{
		if(local)
  			Message(_("\nI can not create cmd file\n"),_("Error"),TRUE);   
		return FALSE;
	}
#ifndef G_OS_WIN32
	fprintf(fcmd,"#!/bin/sh\n");
#endif

	if(local)
	{
#ifdef G_OS_WIN32
		addUnitDisk(fcmd, fileopen.localdir);
#endif
		fprintf(fcmd,"cd %s\n", fileopen.localdir);
	}
	else
		fprintf(fcmd,"cd %s\n", fileopen.remotedir);


#ifdef G_OS_WIN32
	fprintf(fcmd,"%s %s &\n",command,fileopen.datafile);
	fprintf(fcmd,"exit\n");
#else
	if(!strcmp(commandStr,"qchem") || !strcmp(commandStr,"nohup qchem"))
	{
		if(local) fprintf(fcmd,"export DEFAULTDIR=%s\n",fileopen.localdir);
		else fprintf(fcmd,"export DEFAULTDIR=%s\n",fileopen.remotedir);
		fprintf(fcmd,"cd $DEFAULTDIR\n");
		fprintf(fcmd,"export fileinput=%s\n",fileopen.datafile);
		fprintf(fcmd,"export filename=%s\n",fileopen.projectname);
		fprintf(fcmd,"testTMPDIR=\"x$QCSCRATCH\"\n");
		fprintf(fcmd,"if [ $testTMPDIR = \"x\" ]\n");
		fprintf(fcmd,"then\n");
		fprintf(fcmd,"export QCSCRATCH=$HOME/tmp\n");
		fprintf(fcmd,"fi\n");
		fprintf(fcmd,"export QCHEMTMPDIR=$QCSCRATCH/$RANDOM\n");
		fprintf(fcmd,"if [ ! -s \"$QCSCRATCH\" ]\n");
		fprintf(fcmd,"then\n");
		fprintf(fcmd,"mkdir $QCSCRATCH\n");
		fprintf(fcmd,"fi\n");
		fprintf(fcmd,"mkdir $QCHEMTMPDIR\n");
		fprintf(fcmd,"qchem $DEFAULTDIR/$fileinput $DEFAULTDIR/$filename.out\n");
		fprintf(fcmd,"cd $DEFAULTDIR\n");
		fprintf(fcmd,"/bin/rm -r $QCHEMTMPDIR\n");
	}
	else
	{
	 if(!local)
	 {
		 fprintf(fcmd,"%s %s &\n",command,fileopen.datafile);
		 fprintf(fcmd,"exit\n");
	 }
	 else
	 {
		 fprintf(fcmd,"%s %s\n",command,fileopen.datafile);
		 fprintf(fcmd,_("echo Job %s.inp finished.\n"),fileopen.projectname);
	 }
	}

#endif
	fclose(fcmd);
#ifndef G_OS_WIN32
  	sprintf(buffer,"chmod u+x %s",cmdall);
	{ int i = system(buffer);}
#endif
	if(commandStr) g_free(commandStr);
	return TRUE;
}
/***********************************************************************************************************/
static gboolean create_cmd_mopac(G_CONST_RETURN gchar* command, gboolean local, gchar* cmddir, gchar* cmdfile, gchar* cmdall)
{
        FILE* fcmd = NULL;
	gchar* commandStr = g_strdup(command);
#ifndef G_OS_WIN32
	gchar buffer[BSIZE];
#endif
	delete_last_spaces(commandStr);
	delete_first_spaces(commandStr);

	if(local)
  		sprintf(cmddir,"%s", fileopen.localdir);
	else
		sprintf(cmddir,"%s%stmp", gabedit_directory(), G_DIR_SEPARATOR_S);

#ifndef G_OS_WIN32
	sprintf(cmdfile,"%s.cmd", fileopen.projectname);
#else
	if(!local)
		sprintf(cmdfile,"%s.cmd", fileopen.projectname);
	else
		sprintf(cmdfile,"%s.bat", fileopen.projectname);
#endif
  	sprintf(cmdall,"%s%s%s",cmddir,G_DIR_SEPARATOR_S,cmdfile);


  	fcmd = FOpen(cmdall, "w");
	if(!fcmd)
	{
		if(local)
  			Message(_("\nI can not create cmd file\n"),_("Error"),TRUE);   
		return FALSE;
	}
#ifndef G_OS_WIN32
	fprintf(fcmd,"#!/bin/sh\n");
#endif

	if(local)
	{
#ifdef G_OS_WIN32
		addUnitDisk(fcmd, fileopen.localdir);
#endif
		fprintf(fcmd,"cd %s\n", fileopen.localdir);
	}
	else
		fprintf(fcmd,"cd %s\n", fileopen.remotedir);


#ifdef G_OS_WIN32
	if(strstr(mopacDirectory,"\""))
		fprintf(fcmd,"set PATH=%s;%cPATH%c\n",mopacDirectory,'%','%');
	else
		fprintf(fcmd,"set PATH=\"%s\";%cPATH%c\n",mopacDirectory,'%','%');
	fprintf(fcmd,"%s %s &\n",command,fileopen.datafile);
	fprintf(fcmd,"exit\n");
#else
	 if(!local)
	 {
		 fprintf(fcmd,"%s %s &\n",command,fileopen.datafile);
		 fprintf(fcmd,"exit\n");
	 }
	 else
	 {
		 fprintf(fcmd,"%s %s&\n",command,fileopen.datafile);
		 fprintf(fcmd,"exit\n");
	 }

#endif
	fclose(fcmd);
#ifndef G_OS_WIN32
  	sprintf(buffer,"chmod u+x %s",cmdall);
	{ int i = system(buffer);}
#endif
	if(commandStr) g_free(commandStr);
	return TRUE;
}
/***********************************************************************************************************/
static gboolean create_cmd_gamess(G_CONST_RETURN gchar* command, gboolean local, gchar* cmddir, gchar* cmdfile, gchar* cmdall)
{
        FILE* fcmd = NULL;
#ifndef G_OS_WIN32
	gchar buffer[BSIZE];
#endif

	if(local)
  		sprintf(cmddir,"%s", fileopen.localdir);
	else
		sprintf(cmddir,"%s%stmp", gabedit_directory(), G_DIR_SEPARATOR_S);

#ifndef G_OS_WIN32
	sprintf(cmdfile,"%s.cmd", fileopen.projectname);
#else
	if(!local)
		sprintf(cmdfile,"%s.cmd", fileopen.projectname);
	else
		sprintf(cmdfile,"%s.bat", fileopen.projectname);
#endif
  	sprintf(cmdall,"%s%s%s",cmddir,G_DIR_SEPARATOR_S,cmdfile);


  	fcmd = FOpen(cmdall, "w");
	if(!fcmd)
	{
		if(local)
  			Message(_("\nI can not create cmd file\n"),_("Error"),TRUE);   
		return FALSE;
	}

#ifndef G_OS_WIN32
	if(strcmp(command,"submitGMS")==0) 
	{
		fprintf(fcmd,"#!/bin/sh\n");
		if(local)
		{
			if(fileopen.localdir[0]!='/') 
				fprintf(fcmd,"export DEFAULTDIR=$HOME/%s\n",fileopen.localdir);
			else 
				fprintf(fcmd,"export DEFAULTDIR=%s\n",fileopen.localdir);
		}
		else
		{
			if(fileopen.remotedir[0]!='/') 
				fprintf(fcmd,"export DEFAULTDIR=$HOME/%s\n",fileopen.remotedir);
			else 
				fprintf(fcmd,"export DEFAULTDIR=%s\n",fileopen.remotedir);
		}
	}
#endif
	if(local)
	{
#ifdef G_OS_WIN32
		addUnitDisk(fcmd, fileopen.localdir);
#endif
		fprintf(fcmd,"cd %s\n", fileopen.localdir);
	}
	else
		fprintf(fcmd,"cd %s\n", fileopen.remotedir);


#ifndef G_OS_WIN32
	if(strcmp(command,"submitGMS")==0)
	{
			fprintf(fcmd,"export fileinput=%s\n",fileopen.datafile);
			fprintf(fcmd,"export filename=%s\n",fileopen.projectname);
			fprintf(fcmd,"export RUNGAMESSDIR=`which rungms`\n");
			fprintf(fcmd,"if [ x$RUNGAMESSDIR = \"x\" ]\n");
			fprintf(fcmd,"then\n");
			fprintf(fcmd,"   echo 'Sorry, the Gamess directory is not in you path'>$DEFAULTDIR/$filename.log\n");
			fprintf(fcmd,"   echo 'You should add the gamess directory to you path (.login or .cshrc or .basshrc)'>>$DEFAULTDIR/$filename.log\n");
			fprintf(fcmd,"exit\n");
			fprintf(fcmd,"fi\n");
			fprintf(fcmd,"export GAMESSDIR=`dirname $RUNGAMESSDIR`\n");
			fprintf(fcmd,"testTMPDIR=\"x$TGAMESSUS_TMPDIR\"\n");
			fprintf(fcmd,"if [ $testTMPDIR = \"x\" ]\n");
			fprintf(fcmd,"then\n");
			fprintf(fcmd,"export TGAMESSUS_TMPDIR=$HOME/tmp\n");
			fprintf(fcmd,"fi\n");
			fprintf(fcmd,"export TGAMESSUSDIR=$TGAMESSUS_TMPDIR/tmpGamess_$$\n");
			fprintf(fcmd,"if [ ! -s \"$TGAMESSUS_TMPDIR\" ]\n");
			fprintf(fcmd,"then\n");
			fprintf(fcmd,"mkdir $TGAMESSUS_TMPDIR\n");
			fprintf(fcmd,"fi\n");

			fprintf(fcmd,"mkdir $TGAMESSUSDIR\n");
			fprintf(fcmd,"cd $TGAMESSUSDIR\n");
			fprintf(fcmd,"cp $DEFAULTDIR/$fileinput .\n");
			fprintf(fcmd,"cp -r $GAMESSDIR/* .\n");
			fprintf(fcmd,"mv gamess.*.x gamess.00.x\n");
			fprintf(fcmd,"VERNO=00\n");
			fprintf(fcmd,"NNODES=1\n");
			fprintf(fcmd,"sh -c './rungms $filename $VERNO $NNODES > $DEFAULTDIR/$filename.log' 2>err.log\n");
			fprintf(fcmd,"cd $DEFAULTDIR\n");
			fprintf(fcmd,"cp $TGAMESSUSDIR/$filename.dat $DEFAULTDIR/.\n");
			fprintf(fcmd,"chmod -R u+w $TGAMESSUSDIR\n");
			fprintf(fcmd,"/bin/rm -r $TGAMESSUSDIR\n");
	}
	else
		fprintf(fcmd,"%s %s &\n",command,fileopen.projectname);
	fprintf(fcmd,"exit\n");
#else
	 if(!local)
	 {
		 fprintf(fcmd,"%s %s &\n",command,fileopen.datafile);
		 fprintf(fcmd,"exit\n");
	 }
	 else
	 {
		 gchar localhost[100];
		 gchar versionNumber[10]="05";
		 gchar* begin= strstr(command,".");
		 if(begin)
		 {
		 	gchar* end;
		 	gchar* k;
		 	gint i = 0;
			end= strstr(begin+1,".");
			if(end)
		 		for(k=begin+1;k<end;k++)
		 		{
			 		versionNumber[i] = *k;
			 		i++;
		 		}
			 	versionNumber[i] = '\0';
		 }
		 {
			 gchar buffer[BSIZE];
			 sprintf(buffer,"%s\\gabeditRunGms.csh",gamessDirectory);
			if(!createGamessCsh(buffer)) return FALSE;
		 }
		 
		if(!strstr(command,"11-"))
		{
		 winsockCheck(stderr);
  		 gethostname(localhost,100);
		 fprintf(fcmd,"@echo off\n");
		 fprintf(fcmd,_("echo Running %s.inp ....\n"),fileopen.projectname);
		 begin = strstr(gamessDirectory,":");
		 if(begin) fprintf(fcmd,"%c:\n",gamessDirectory[0]);
		 else fprintf(fcmd,"C:\n");
		 fprintf(fcmd,"mkdir \"%s\\temp\"\n",gamessDirectory);
		 fprintf(fcmd,"mkdir \"%s\\scratch\"\n",gamessDirectory);
		 addUnitDisk(fcmd, gamessDirectory);
		 fprintf(fcmd,"cd \"%s\\temp\"\n",gamessDirectory);
		 fprintf(fcmd,"del %s.*\n",fileopen.projectname);
		 fprintf(fcmd,"cd \"%s\"\n",gamessDirectory);
		 fprintf(fcmd,"copy \"%s\\%s\" \"%s\\scratch\\%s.F05\"\n",
				 fileopen.localdir,fileopen.datafile,gamessDirectory,fileopen.projectname);
		 fprintf(fcmd,"csh -f gabeditRunGms.csh %s %s %s > \"%s\\%s\" \n",
				 fileopen.projectname, versionNumber, localhost,fileopen.localdir,fileopen.logfile);
		 fprintf(fcmd,"copy \"%s\\temp\\%s.dat\" \"%s\\%s.dat\"\n",
				 gamessDirectory,fileopen.projectname,
				 fileopen.localdir,fileopen.projectname);
		 fprintf(fcmd,"copy \"%s\\temp\\%s.dat\" \"%s\\%s.pun\"\n",
				 gamessDirectory,fileopen.projectname,
				 fileopen.localdir,fileopen.projectname);
		 fprintf(fcmd,_("echo Job %s.inp finished.\n"),fileopen.projectname);
		}
		else
		{
			gchar com[BSIZE];
			gint nproc = 0;
			 gchar buffer[BSIZE];
			 sprintf(buffer,"%s\\gabeditRunGms.csh",gamessDirectory);
  			sprintf(buffer,"%s%sgabeditRun.bat",cmddir,G_DIR_SEPARATOR_S);
			sscanf(command,"%s %d",com,&nproc);
			if(nproc<1) nproc = 1;
			if(!createGamessBat(buffer,com,nproc)) return FALSE;
		 	fprintf(fcmd,"%s %s\n",buffer,fileopen.datafile);
		}
	 }

#endif
	fclose(fcmd);
#ifndef G_OS_WIN32
  	sprintf(buffer,"chmod u+x %s",cmdall);
	{ int i = system(buffer);}
#endif
	return TRUE;
}
/***********************************************************************************************************/
static gboolean create_cmd_gaussian(G_CONST_RETURN gchar* command, gboolean local, gchar* cmddir, gchar* cmdfile, gchar* cmdall)
{
        FILE* fcmd = NULL;
#ifndef G_OS_WIN32
	gchar buffer[BSIZE];
#endif

	if(local)
  		sprintf(cmddir,"%s", fileopen.localdir);
	else
		sprintf(cmddir,"%s%stmp", gabedit_directory(), G_DIR_SEPARATOR_S);

#ifndef G_OS_WIN32
	sprintf(cmdfile,"%s.cmd", fileopen.projectname);
#else
	if(!local)
		sprintf(cmdfile,"%s.cmd", fileopen.projectname);
	else
		sprintf(cmdfile,"%s.bat", fileopen.projectname);
#endif
  	sprintf(cmdall,"%s%s%s",cmddir,G_DIR_SEPARATOR_S,cmdfile);


  	fcmd = FOpen(cmdall, "w");
	if(!fcmd)
	{
		if(local)
  			Message(_("\nI can not create cmd file\n"),_("Error"),TRUE);   
		return FALSE;
	}

	if(local)
	{
#ifdef G_OS_WIN32
		fprintf(fcmd,"@echo off\n");
		if(fileopen.localdir && 
		strlen(fileopen.localdir)>2 &&
		fileopen.localdir[1]==':'
		)
		fprintf(fcmd,"%c%c\n", fileopen.localdir[0],fileopen.localdir[1]);
		else
		if(fileopen.localdir && 
		strlen(fileopen.localdir)>3 &&
		fileopen.localdir[0]=='"' &&
		fileopen.localdir[2]==':'
		)
		fprintf(fcmd,"%c%c\n", fileopen.localdir[1],fileopen.localdir[2]);
		fprintf(fcmd,"cd \"%s\"\n", fileopen.localdir);
#else

		fprintf(fcmd,"cd %s\n", fileopen.localdir);
#endif
	}
	else
		fprintf(fcmd,"cd %s\n", fileopen.remotedir);


#ifndef G_OS_WIN32
	fprintf(fcmd,"%s %s &\n",command,fileopen.datafile);
	fprintf(fcmd,"exit\n");
#else
	 if(!local)
	 {
		 fprintf(fcmd,"%s %s &\n",command,fileopen.datafile);
		 fprintf(fcmd,"exit\n");
	 }
	 else
	 {
		if(strstr(gaussDirectory,"\""))
			fprintf(fcmd,"set PATH=%s;%cPATH%c\n",gaussDirectory,'%','%');
		else
			fprintf(fcmd,"set PATH=\"%s\";%cPATH%c\n",gaussDirectory,'%','%');
		if(!strcmp(command,"g03.exe")||!strcmp(command,"g98.exe"))
		{
		    fprintf(fcmd,"set GAUSS_EXEDIR=%s\n",gaussDirectory);
		    fprintf(fcmd,"del/Q %s 2>nul\n",fileopen.logfile);
		    fprintf(fcmd,"%s%s%s %s %s\n",gaussDirectory,G_DIR_SEPARATOR_S,command,fileopen.datafile, fileopen.logfile);
		}
		else
		 fprintf(fcmd,"%s %s \n",command,fileopen.datafile);
	 }
#endif
	fclose(fcmd);
#ifndef G_OS_WIN32
  	sprintf(buffer,"chmod u+x %s",cmdall);
	{ int i = system(buffer);}
#endif
	return TRUE;
}
/***********************************************************************************************************/
static gboolean create_cmd_molcas(G_CONST_RETURN gchar* command, gboolean local, gchar* cmddir, gchar* cmdfile, gchar* cmdall)
{
        FILE* fcmd = NULL;
	gchar buffer[BSIZE];
	gint i;

	if(local)
  		sprintf(cmddir,"%s", fileopen.localdir);
	else
		sprintf(cmddir,"%s%stmp", gabedit_directory(), G_DIR_SEPARATOR_S);

#ifndef G_OS_WIN32
	sprintf(cmdfile,"%s.cmd", fileopen.projectname);
#else
	if(!local)
		sprintf(cmdfile,"%s.cmd", fileopen.projectname);
	else
		sprintf(cmdfile,"%s.bat", fileopen.projectname);
#endif

  	sprintf(cmdall,"%s%s%s",cmddir,G_DIR_SEPARATOR_S,cmdfile);


  	fcmd = FOpen(cmdall, "w");
	if(!fcmd)
	{
		if(local)
  			Message(_("\nI can not create cmd file\n"),_("Error"),TRUE);   
		return FALSE;
	}

#ifndef G_OS_WIN32
	fprintf(fcmd,"#! /bin/sh\n");
	fprintf(fcmd,"#********************************************\n");
	fprintf(fcmd,"# file created by Gabedit\n");
	fprintf(fcmd,"#********************************************\n");
	if(!molcasTrap())
	{
		fprintf(fcmd,"MOLCAS_TRAP='OFF'\n");
		fprintf(fcmd,"export MOLCAS_TRAP\n");
	}
	i = molcasMem();
	if(i>=0)
	{
		fprintf(fcmd,"MOLCASMEM=%d\n",i);
		fprintf(fcmd,"export MOLCASMEM\n");
	}
	i = molcasDisk();
	if(i>=0)
	{
		fprintf(fcmd,"MOLCASDISK=%d\n",i);
		fprintf(fcmd,"export MOLCASDISK\n");
	}
	i = molcasRamd();
	if(i>=0)
	{
		fprintf(fcmd,"MOLCASRAMD=%d\n",i);
		fprintf(fcmd,"export MOLCASRAMD\n");
	}

	molcasWorkDir(buffer);
	if(strcmp(buffer,"Default") != 0)
	{
		fprintf(fcmd,"WorkDir=%s\n",buffer);
		fprintf(fcmd,"export WorkDir\n");
	}

	fprintf(fcmd,"#--------------------------------------------\n");
	fprintf(fcmd,"Project=%s\n", fileopen.projectname);
	fprintf(fcmd,"export Project\n");
	fprintf(fcmd,"#--------------------------------------------\n");
	if(local)
	{
		fprintf(fcmd,"cd %s\n", fileopen.localdir);
	}
	else
		fprintf(fcmd,"cd %s\n", fileopen.remotedir);

	fprintf(fcmd,"%s %s > %s&\n", command, fileopen.datafile, fileopen.outputfile);
	fprintf(fcmd,"exit\n");
	fclose(fcmd);

  	sprintf(buffer,"chmod u+x %s",cmdall);
	{ int i = system(buffer);}
#else
	if(!local)
	{
		fprintf(fcmd,"#! /bin/sh\n");
		fprintf(fcmd,"#********************************************\n");
		fprintf(fcmd,"# file created by Gabedit\n");
		fprintf(fcmd,"#********************************************\n");
		if(!molcasTrap())
		{
			fprintf(fcmd,"MOLCAS_TRAP='OFF'\n");
			fprintf(fcmd,"export MOLCAS_TRAP\n");
		}
		i = molcasMem();
		if(i>=0)
		{
			fprintf(fcmd,"MOLCASMEM=%d\n",i);
			fprintf(fcmd,"export MOLCASMEM\n");
		}
		i = molcasDisk();
		if(i>=0)
		{
			fprintf(fcmd,"MOLCASDISK=%d\n",i);
			fprintf(fcmd,"export MOLCASDISK\n");
		}
		i = molcasRamd();
		if(i>=0)
		{
			fprintf(fcmd,"MOLCASRAMD=%d\n",i);
			fprintf(fcmd,"export MOLCASRAMD\n");
		}

		molcasWorkDir(buffer);
		if(strcmp(buffer,"Default") != 0)
		{
			fprintf(fcmd,"WorkDir=%s\n",buffer);
			fprintf(fcmd,"export WorkDir\n");
		}

		fprintf(fcmd,"#--------------------------------------------\n");
		fprintf(fcmd,"Project=%s\n", fileopen.projectname);
		fprintf(fcmd,"export Project\n");
		fprintf(fcmd,"#--------------------------------------------\n");
		if(local)
			fprintf(fcmd,"cd %s\n", fileopen.localdir);
		else
			fprintf(fcmd,"cd %s\n", fileopen.remotedir);

		fprintf(fcmd,"%s %s > %s&\n", command, fileopen.datafile, fileopen.outputfile);
		fprintf(fcmd,"exit\n");
		fclose(fcmd);
	}
	else
	{
		fprintf(fcmd,"@echo off\n");
		fprintf(fcmd,"@rem ********************************************\n");
		fprintf(fcmd,"@rem  file created by Gabedit\n");
		fprintf(fcmd,"@rem  *******************************************\n");
		if(!molcasTrap())
		{
			fprintf(fcmd,"set MOLCAS_TRAP='OFF'\n");
			
		}
		i = molcasMem();
		if(i>=0)
		{
			fprintf(fcmd,"set MOLCASMEM=%d\n",i);
		}
		i = molcasDisk();
		if(i>=0)
		{
			fprintf(fcmd,"set MOLCASDISK=%d\n",i);
		}
		i = molcasRamd();
		if(i>=0)
		{
			fprintf(fcmd,"set MOLCASRAMD=%d\n",i);
		}

		molcasWorkDir(buffer);
		if(strcmp(buffer,"Default") != 0)
		{
			fprintf(fcmd,"set WorkDir=%s\n",buffer);
		}

		fprintf(fcmd,"@rem --------------------------------------------\n");
		fprintf(fcmd,"set Project=%s\n", fileopen.projectname);
		fprintf(fcmd,"@rem --------------------------------------------\n");
		if(local)
		{
			addUnitDisk(fcmd, fileopen.localdir);
			fprintf(fcmd,"cd %s\n", fileopen.localdir);
		}
		else
			fprintf(fcmd,"cd %s\n", fileopen.remotedir);

		fprintf(fcmd,"%s %s > %s\n", command, fileopen.datafile, fileopen.outputfile);
		fclose(fcmd);

	}

#endif

	return TRUE;
}
/***********************************************************************************************************/
static gboolean create_cmd_molpro(G_CONST_RETURN gchar* command, gboolean local, gchar* cmddir, gchar* cmdfile, gchar* cmdall)
{
        FILE* fcmd = NULL;
#ifndef G_OS_WIN32
	gchar buffer[BSIZE];
#endif

	if(local)
  		sprintf(cmddir,"%s", fileopen.localdir);
	else
		sprintf(cmddir,"%s%stmp", gabedit_directory(), G_DIR_SEPARATOR_S);

#ifndef G_OS_WIN32
	sprintf(cmdfile,"%s.cmd", fileopen.projectname);
#else
	if(!local)
		sprintf(cmdfile,"%s.cmd", fileopen.projectname);
	else
		sprintf(cmdfile,"%s.bat", fileopen.projectname);
#endif

  	sprintf(cmdall,"%s%s%s",cmddir,G_DIR_SEPARATOR_S,cmdfile);


  	fcmd = FOpen(cmdall, "w");
	if(!fcmd)
	{
		if(local)
  			Message(_("\nI can not create cmd file\n"),_("Error"),TRUE);   
		return FALSE;
	}

	if(local)
	{
#ifdef G_OS_WIN32
		addUnitDisk(fcmd, fileopen.localdir);
#endif
		fprintf(fcmd,"cd %s\n", fileopen.localdir);
	}
	else
		fprintf(fcmd,"cd %s\n", fileopen.remotedir);

#ifndef G_OS_WIN32
	fprintf(fcmd,"%s %s &\n",command,fileopen.datafile);
	fprintf(fcmd,"exit\n");
	fclose(fcmd);
  	sprintf(buffer,"chmod u+x %s",cmdall);
	{ int i = system(buffer);}
#else
	if(!local)
	{
		fprintf(fcmd,"%s %s &\n",command,fileopen.datafile);
		fprintf(fcmd,"exit\n");
		fclose(fcmd);
	}
	else
	{
		fprintf(fcmd,"%s %s \n",command,fileopen.datafile);
		fclose(fcmd);
	}
#endif
	return TRUE;
}
/***********************************************************************************************************/
static gboolean create_cmd_mpqc(G_CONST_RETURN gchar* command, gboolean local, gchar* cmddir, gchar* cmdfile, gchar* cmdall)
{
        FILE* fcmd = NULL;
#ifndef G_OS_WIN32
	gchar buffer[BSIZE];
#endif

	if(local)
  		sprintf(cmddir,"%s", fileopen.localdir);
	else
		sprintf(cmddir,"%s%stmp", gabedit_directory(), G_DIR_SEPARATOR_S);

#ifndef G_OS_WIN32
	sprintf(cmdfile,"%s.cmd", fileopen.projectname);
#else
	if(!local)
		sprintf(cmdfile,"%s.cmd", fileopen.projectname);
	else
		sprintf(cmdfile,"%s.bat", fileopen.projectname);
#endif
  	sprintf(cmdall,"%s%s%s",cmddir,G_DIR_SEPARATOR_S,cmdfile);


  	fcmd = FOpen(cmdall, "w");
	if(!fcmd)
	{
		if(local)
  			Message(_("\nI can not create cmd file\n"),_("Error"),TRUE);   
		return FALSE;
	}

	if(local)
	{
#ifdef G_OS_WIN32
		addUnitDisk(fcmd, fileopen.localdir);
#endif
		fprintf(fcmd,"cd %s\n", fileopen.localdir);
	}
	else
		fprintf(fcmd,"cd %s\n", fileopen.remotedir);


#ifndef G_OS_WIN32
	fprintf(fcmd,"%s %s > %s &\n",command,fileopen.datafile, fileopen.outputfile);
	fprintf(fcmd,"exit\n");
#else
	 if(!local)
	 {
		 fprintf(fcmd,"%s %s > %s &\n",command,fileopen.datafile, fileopen.outputfile);
		 fprintf(fcmd,"exit\n");
	 }
	 else
		 fprintf(fcmd,"%s %s > %s\n",command,fileopen.datafile, fileopen.outputfile);

#endif
	fclose(fcmd);
#ifndef G_OS_WIN32
  	sprintf(buffer,"chmod u+x %s",cmdall);
	{ int i = system(buffer);}
#endif
	return TRUE;
}
/***********************************************************************************************************/
static gboolean create_cmd_other(G_CONST_RETURN gchar* command, gboolean local, gchar* cmddir, gchar* cmdfile, gchar* cmdall)
{
        FILE* fcmd = NULL;
#ifndef G_OS_WIN32
	gchar buffer[BSIZE];
#endif

	if(local)
  		sprintf(cmddir,"%s", fileopen.localdir);
	else
		sprintf(cmddir,"%s%stmp", gabedit_directory(), G_DIR_SEPARATOR_S);

  	sprintf(cmdfile,"%s.cmd", fileopen.projectname);
  	sprintf(cmdall,"%s%s%s",cmddir,G_DIR_SEPARATOR_S,cmdfile);


  	fcmd = FOpen(cmdall, "w");
	if(!fcmd)
	{
		if(local)
  			Message(_("\nI can not create cmd file\n"),_("Error"),TRUE);   
		return FALSE;
	}

	if(local)
	{
#ifdef G_OS_WIN32
		addUnitDisk(fcmd, fileopen.localdir);
		fprintf(fcmd,"cd %s\n", fileopen.localdir);
#endif
	}
	else
		fprintf(fcmd,"cd %s\n", fileopen.remotedir);

#ifndef G_OS_WIN32
	fprintf(fcmd,"%s\n",command);
	fprintf(fcmd,"exit\n");
	fclose(fcmd);
  	sprintf(buffer,"chmod u+x %s",cmdall);
	{ int i = system(buffer);}
#else
	if(!local)
	{
		fprintf(fcmd,"%s\n",command);
		fprintf(fcmd,"exit\n");
		fclose(fcmd);
	}
	else
	{
		fprintf(fcmd,"%s\n",command);
		fclose(fcmd);
	}
#endif
	return TRUE;
}
/********************************************************************************/
static void run_remote_demon(GtkWidget *b,gpointer data)
{  
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);

  gchar cmdfile[BSIZE];
  gchar cmddir[BSIZE];
  gchar cmdall[BSIZE];

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *temp;
  gchar *NomFichier;
  gchar *Command;
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  gint code = 0;
  G_CONST_RETURN gchar *localdir;


  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);

  fileopen.remotehost = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[2])));
  fileopen.remoteuser = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[3])));
  fileopen.remotepass  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[4])));
  fileopen.remotedir  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[5])));
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));

  /* fileopen.netWorkProtocol Deja defini dans run_program*/
  
/*   Debug("remote demon : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_ORCA],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_ORCA, fileopen.command, fileopen.netWorkProtocol); 
  add_host(fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir);

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  title = g_strdup_printf(_("Run DeMon at host :%s, Login : %s"),fileopen.remotehost,fileopen.remoteuser); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  gtk_widget_show_all(Win);
  while( gtk_events_pending() ) gtk_main_iteration();
  gtk_widget_set_sensitive(Win, FALSE);

  if(!this_is_a_backspace(fileopen.remotedir))
  {
	/* Make Working directory */
	/*  Debug("Make dir remote demon : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	Command = g_strdup_printf("mkdir %s",fileopen.remotedir);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	g_free(Command);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }

  if(code == 0)
  {
	/* put file.com */
	/*  Debug("Put File remote demon : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	code = put_file(fout,ferr,fileopen.datafile,fileopen.localdir,fileopen.remotedir,
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if( code==0 )
  {
        if(!create_cmd_demon(entrytext0, FALSE, cmddir, cmdfile, cmdall))
	{
  		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nI can not create cmd file\n"),-1);   
  		gtk_widget_set_sensitive(Win, TRUE);
		return;
	}
	code = 0;
	  
  }
  if(code == 0)
  {
  	code = put_file(fout,ferr,cmdfile,cmddir,"./",
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1); 
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1);
  	put_text_in_texts_widget(Text,fout,ferr);
	unlink(cmdall);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("chmod u+x %s",cmdfile);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("./%s>/dev/null&",cmdfile);
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nRun DeMon at remote host :\n"),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nRun DeMon at remote host :\n"),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  gtk_widget_set_sensitive(Win, TRUE);

  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_remote_orca(GtkWidget *b,gpointer data)
{  
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);

  gchar cmdfile[BSIZE];
  gchar cmddir[BSIZE];
  gchar cmdall[BSIZE];

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *temp;
  gchar *NomFichier;
  gchar *Command;
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  gint code = 0;
  G_CONST_RETURN gchar *localdir;


  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);

  fileopen.remotehost = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[2])));
  fileopen.remoteuser = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[3])));
  fileopen.remotepass  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[4])));
  fileopen.remotedir  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[5])));
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));

  /* fileopen.netWorkProtocol Deja defini dans run_program*/
  
/*   Debug("remote orca : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_ORCA],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_ORCA, fileopen.command, fileopen.netWorkProtocol); 
  add_host(fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir);

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  title = g_strdup_printf(_("Run Orca at host :%s, Login : %s"),fileopen.remotehost,fileopen.remoteuser); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  gtk_widget_show_all(Win);
  while( gtk_events_pending() ) gtk_main_iteration();
  gtk_widget_set_sensitive(Win, FALSE);

  if(!this_is_a_backspace(fileopen.remotedir))
  {
	/* Make Working directory */
	/*  Debug("Make dir remote orca : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	Command = g_strdup_printf("mkdir %s",fileopen.remotedir);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	g_free(Command);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }

  if(code == 0)
  {
	/* put file.com */
	/*  Debug("Put File remote orca : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	code = put_file(fout,ferr,fileopen.datafile,fileopen.localdir,fileopen.remotedir,
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if( code==0 )
  {
        if(!create_cmd_orca(entrytext0, FALSE, cmddir, cmdfile, cmdall))
	{
  		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nI can not create cmd file\n"),-1);   
  		gtk_widget_set_sensitive(Win, TRUE);
		return;
	}
	code = 0;
	  
  }
  if(code == 0)
  {
  	code = put_file(fout,ferr,cmdfile,cmddir,"./",
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1); 
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1);
  	put_text_in_texts_widget(Text,fout,ferr);
	unlink(cmdall);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("chmod u+x %s",cmdfile);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("./%s>/dev/null&",cmdfile);
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nRun Orca at remote host :\n"),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nRun Orca at remote host :\n"),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  gtk_widget_set_sensitive(Win, TRUE);

  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_remote_nwchem(GtkWidget *b,gpointer data)
{  
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);

  gchar cmdfile[BSIZE];
  gchar cmddir[BSIZE];
  gchar cmdall[BSIZE];

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *temp;
  gchar *NomFichier;
  gchar *Command;
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  gint code = 0;
  G_CONST_RETURN gchar *localdir;


  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.nw",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);

  fileopen.remotehost = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[2])));
  fileopen.remoteuser = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[3])));
  fileopen.remotepass  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[4])));
  fileopen.remotedir  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[5])));
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));

  /* fileopen.netWorkProtocol Deja defini dans run_program*/
  
/*   Debug("remote nwchem : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_NWCHEM],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_NWCHEM, fileopen.command, fileopen.netWorkProtocol); 
  add_host(fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir);

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  title = g_strdup_printf(_("Run NWChem at host :%s, Login : %s"),fileopen.remotehost,fileopen.remoteuser); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  gtk_widget_show_all(Win);
  while( gtk_events_pending() ) gtk_main_iteration();
  gtk_widget_set_sensitive(Win, FALSE);

  if(!this_is_a_backspace(fileopen.remotedir))
  {
	/* Make Working directory */
	/*  Debug("Make dir remote nwchem : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	Command = g_strdup_printf("mkdir %s",fileopen.remotedir);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	g_free(Command);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }

  if(code == 0)
  {
	/* put file.com */
	/*  Debug("Put File remote nwchem : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	code = put_file(fout,ferr,fileopen.datafile,fileopen.localdir,fileopen.remotedir,
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if( code==0 )
  {
        if(!create_cmd_nwchem(entrytext0, FALSE, cmddir, cmdfile, cmdall))
	{
  		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nI can not create cmd file\n"),-1);   
  		gtk_widget_set_sensitive(Win, TRUE);
		return;
	}
	code = 0;
	  
  }
  if(code == 0)
  {
  	code = put_file(fout,ferr,cmdfile,cmddir,"./",
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1); 
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1);
  	put_text_in_texts_widget(Text,fout,ferr);
	unlink(cmdall);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("chmod u+x %s",cmdfile);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("./%s>/dev/null&",cmdfile);
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nRun NWChem at remote host :\n"),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nRun NWChem at remote host :\n"),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  gtk_widget_set_sensitive(Win, TRUE);

  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_remote_psicode(GtkWidget *b,gpointer data)
{  
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);

  gchar cmdfile[BSIZE];
  gchar cmddir[BSIZE];
  gchar cmdall[BSIZE];

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *temp;
  gchar *NomFichier;
  gchar *Command;
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  gint code = 0;
  G_CONST_RETURN gchar *localdir;


  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.psi",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);

  fileopen.remotehost = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[2])));
  fileopen.remoteuser = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[3])));
  fileopen.remotepass  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[4])));
  fileopen.remotedir  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[5])));
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));

  /* fileopen.netWorkProtocol Deja defini dans run_program*/
  
/*   Debug("remote psicode : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_PSICODE],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_PSICODE, fileopen.command, fileopen.netWorkProtocol); 
  add_host(fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir);

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  title = g_strdup_printf(_("Run Psicode at host :%s, Login : %s"),fileopen.remotehost,fileopen.remoteuser); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  gtk_widget_show_all(Win);
  while( gtk_events_pending() ) gtk_main_iteration();
  gtk_widget_set_sensitive(Win, FALSE);

  if(!this_is_a_backspace(fileopen.remotedir))
  {
	/* Make Working directory */
	/*  Debug("Make dir remote psicode : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	Command = g_strdup_printf("mkdir %s",fileopen.remotedir);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	g_free(Command);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }

  if(code == 0)
  {
	/* put file.com */
	/*  Debug("Put File remote psicode : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	code = put_file(fout,ferr,fileopen.datafile,fileopen.localdir,fileopen.remotedir,
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if( code==0 )
  {
        if(!create_cmd_psicode(entrytext0, FALSE, cmddir, cmdfile, cmdall))
	{
  		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nI can not create cmd file\n"),-1);   
  		gtk_widget_set_sensitive(Win, TRUE);
		return;
	}
	code = 0;
	  
  }
  if(code == 0)
  {
  	code = put_file(fout,ferr,cmdfile,cmddir,"./",
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1); 
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1);
  	put_text_in_texts_widget(Text,fout,ferr);
	unlink(cmdall);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("chmod u+x %s",cmdfile);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("./%s>/dev/null&",cmdfile);
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nRun Psicode at remote host :\n"),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nRun Psicode at remote host :\n"),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  gtk_widget_set_sensitive(Win, TRUE);

  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_remote_firefly(GtkWidget *b,gpointer data)
{  
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);

  gchar cmdfile[BSIZE];
  gchar cmddir[BSIZE];
  gchar cmdall[BSIZE];

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *temp;
  gchar *NomFichier;
  gchar *Command;
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  gint code = 0;
  G_CONST_RETURN gchar *localdir;


  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.log",fileopen.projectname);

  fileopen.remotehost = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[2])));
  fileopen.remoteuser = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[3])));
  fileopen.remotepass  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[4])));
  fileopen.remotedir  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[5])));
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));

  /* fileopen.netWorkProtocol Deja defini dans run_program*/
  
/*   Debug("remote pcgamess : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_FIREFLY],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_FIREFLY, fileopen.command, fileopen.netWorkProtocol); 
  add_host(fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir);

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  title = g_strdup_printf(_("Run FireFly at host :%s, Login : %s"),fileopen.remotehost,fileopen.remoteuser); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  gtk_widget_show_all(Win);
  while( gtk_events_pending() ) gtk_main_iteration();
  gtk_widget_set_sensitive(Win, FALSE);

  if(!this_is_a_backspace(fileopen.remotedir))
  {
	/* Make Working directory */
	/*  Debug("Make dir remote pcgamess : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	Command = g_strdup_printf("mkdir %s",fileopen.remotedir);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	g_free(Command);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }

  if(code == 0)
  {
	/* put file.com */
	/*  Debug("Put File remote pcgamess : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	code = put_file(fout,ferr,fileopen.datafile,fileopen.localdir,fileopen.remotedir,
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if( code==0 )
  {
        if(!create_cmd_firefly(entrytext0, FALSE, cmddir, cmdfile, cmdall))
	{
  		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL, _("\nI can not create cmd file\n "),-1);   
  		gtk_widget_set_sensitive(Win, TRUE);
		return;
	}
	code = 0;
	  
  }
  if(code == 0)
  {
  	code = put_file(fout,ferr,cmdfile,cmddir,"./",
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1); 
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1);
  	put_text_in_texts_widget(Text,fout,ferr);
	unlink(cmdall);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("chmod u+x %s",cmdfile);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nchmod for cmd file :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nchmod for cmd file :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("$HOME/%s>/dev/null&",cmdfile);
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nRun FireFly at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nRun FireFly at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  gtk_widget_set_sensitive(Win, TRUE);

  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_remote_qchem(GtkWidget *b,gpointer data)
{  
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);

  gchar cmdfile[BSIZE];
  gchar cmddir[BSIZE];
  gchar cmdall[BSIZE];

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *temp;
  gchar *NomFichier;
  gchar *Command;
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  gint code = 0;
  G_CONST_RETURN gchar *localdir;


  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);

  fileopen.remotehost = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[2])));
  fileopen.remoteuser = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[3])));
  fileopen.remotepass  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[4])));
  fileopen.remotedir  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[5])));
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));

  /* fileopen.netWorkProtocol Deja defini dans run_program*/
  
/*   Debug("remote qchem : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_QCHEM],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_QCHEM, fileopen.command, fileopen.netWorkProtocol); 
  add_host(fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir);

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  title = g_strdup_printf(_("Run Q-Chem at host :%s, Login : %s"),fileopen.remotehost,fileopen.remoteuser); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  gtk_widget_show_all(Win);
  while( gtk_events_pending() ) gtk_main_iteration();
  gtk_widget_set_sensitive(Win, FALSE);

  if(!this_is_a_backspace(fileopen.remotedir))
  {
	/* Make Working directory */
	/*  Debug("Make dir remote qchem : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	Command = g_strdup_printf("mkdir %s",fileopen.remotedir);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	g_free(Command);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }

  if(code == 0)
  {
	/* put file.com */
	/*  Debug("Put File remote qchem : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	code = put_file(fout,ferr,fileopen.datafile,fileopen.localdir,fileopen.remotedir,
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if( code==0 )
  {
        if(!create_cmd_qchem(entrytext0, FALSE, cmddir, cmdfile, cmdall))
	{
  		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL, _("\nI can not create cmd file\n "),-1);   
  		gtk_widget_set_sensitive(Win, TRUE);
		return;
	}
	code = 0;
	  
  }
  if(code == 0)
  {
  	code = put_file(fout,ferr,cmdfile,cmddir,"./",
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1); 
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1);
  	put_text_in_texts_widget(Text,fout,ferr);
	unlink(cmdall);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("chmod u+x %s",cmdfile);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nchmod for cmd file :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nchmod for cmd file :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("./%s>/dev/null&",cmdfile);
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nRun Q-Chem at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nRun Q-Chem at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  gtk_widget_set_sensitive(Win, TRUE);

  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_remote_mopac(GtkWidget *b,gpointer data)
{  
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);

  gchar cmdfile[BSIZE];
  gchar cmddir[BSIZE];
  gchar cmdall[BSIZE];

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *temp;
  gchar *NomFichier;
  gchar *Command;
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  gint code = 0;
  G_CONST_RETURN gchar *localdir;


  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.mop",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.aux",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);

  fileopen.remotehost = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[2])));
  fileopen.remoteuser = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[3])));
  fileopen.remotepass  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[4])));
  fileopen.remotedir  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[5])));
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));

  /* fileopen.netWorkProtocol Deja defini dans run_program*/
  
/*   Debug("remote mopac : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_MOPAC],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_MOPAC, fileopen.command, fileopen.netWorkProtocol); 
  add_host(fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir);

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  title = g_strdup_printf(_("Run Mopac at host :%s, Login : %s"),fileopen.remotehost,fileopen.remoteuser); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  gtk_widget_show_all(Win);
  while( gtk_events_pending() ) gtk_main_iteration();
  gtk_widget_set_sensitive(Win, FALSE);

  if(!this_is_a_backspace(fileopen.remotedir))
  {
	/* Make Working directory */
	/*  Debug("Make dir remote mopac : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	Command = g_strdup_printf("mkdir %s",fileopen.remotedir);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	g_free(Command);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }

  if(code == 0)
  {
	/* put file.com */
	/*  Debug("Put File remote mopac : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	code = put_file(fout,ferr,fileopen.datafile,fileopen.localdir,fileopen.remotedir,
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if( code==0 )
  {
        if(!create_cmd_mopac(entrytext0, FALSE, cmddir, cmdfile, cmdall))
	{
  		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL, _("\nI can not create cmd file\n "),-1);   
  		gtk_widget_set_sensitive(Win, TRUE);
		return;
	}
	code = 0;
	  
  }
  if(code == 0)
  {
  	code = put_file(fout,ferr,cmdfile,cmddir,"./",
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1); 
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1);
  	put_text_in_texts_widget(Text,fout,ferr);
	unlink(cmdall);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("chmod u+x %s",cmdfile);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("./%s>/dev/null&",cmdfile);
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nRun Mopac at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nRun Mopac at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  gtk_widget_set_sensitive(Win, TRUE);

  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_remote_gamess(GtkWidget *b,gpointer data)
{  
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);

  gchar cmdfile[BSIZE];
  gchar cmddir[BSIZE];
  gchar cmdall[BSIZE];

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *temp;
  gchar *NomFichier;
  gchar *Command;
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  gint code = 0;
  G_CONST_RETURN gchar *localdir;


  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.log",fileopen.projectname);

  fileopen.remotehost = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[2])));
  fileopen.remoteuser = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[3])));
  fileopen.remotepass  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[4])));
  fileopen.remotedir  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[5])));
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));

  /* fileopen.netWorkProtocol Deja defini dans run_program*/
  
/*   Debug("remote gauss : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_GAMESS],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_GAMESS, fileopen.command, fileopen.netWorkProtocol); 
  add_host(fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir);

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  title = g_strdup_printf(_("Run Gamess at host :%s, Login : %s"),fileopen.remotehost,fileopen.remoteuser); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  gtk_widget_show_all(Win);
  while( gtk_events_pending() ) gtk_main_iteration();
  gtk_widget_set_sensitive(Win, FALSE);

  if(!this_is_a_backspace(fileopen.remotedir))
  {
	/* Make Working directory */
	/*  Debug("Make dir remote gauss : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	Command = g_strdup_printf("mkdir %s",fileopen.remotedir);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	g_free(Command);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }

  if(code == 0)
  {
	/* put file.com */
	/*  Debug("Put File remote gauss : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	code = put_file(fout,ferr,fileopen.datafile,fileopen.localdir,fileopen.remotedir,
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if( code==0 )
  {
        if(!create_cmd_gamess(entrytext0, FALSE, cmddir, cmdfile, cmdall))
	{
  		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nI can not create cmd file\n"),-1);   
  		gtk_widget_set_sensitive(Win, TRUE);
		return;
	}
	code = 0;
	  
  }
  if(code == 0)
  {
  	code = put_file(fout,ferr,cmdfile,cmddir,"./",
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1); 
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1);
  	put_text_in_texts_widget(Text,fout,ferr);
	unlink(cmdall);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("chmod u+x %s",cmdfile);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("./%s>/dev/null&",cmdfile);
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nRun gamess at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nRun gamess at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  gtk_widget_set_sensitive(Win, TRUE);

  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_remote_gaussian(GtkWidget *b,gpointer data)
{  
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);

  gchar cmdfile[BSIZE];
  gchar cmddir[BSIZE];
  gchar cmdall[BSIZE];

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *temp;
  gchar *NomFichier;
  gchar *Command;
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  gint code = 0;
  G_CONST_RETURN gchar *localdir;


  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.log",fileopen.projectname);

  fileopen.remotehost = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[2])));
  fileopen.remoteuser = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[3])));
  fileopen.remotepass  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[4])));
  fileopen.remotedir  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[5])));
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));

  /* fileopen.netWorkProtocol Deja defini dans run_program*/
  
/*   Debug("remote gauss : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_GAUSSIAN],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_GAUSSIAN, fileopen.command, fileopen.netWorkProtocol); 
  add_host(fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir);

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  title = g_strdup_printf(_("Run Gaussian at host :%s, Login : %s"),fileopen.remotehost,fileopen.remoteuser); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  gtk_widget_show_all(Win);
  while( gtk_events_pending() ) gtk_main_iteration();
  gtk_widget_set_sensitive(Win, FALSE);

  if(!this_is_a_backspace(fileopen.remotedir))
  {
	/* Make Working directory */
	/*  Debug("Make dir remote gauss : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	Command = g_strdup_printf("mkdir %s",fileopen.remotedir);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	g_free(Command);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }

  if(code == 0)
  {
	/* put file.com */
	/*  Debug("Put File remote gauss : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	code = put_file(fout,ferr,fileopen.datafile,fileopen.localdir,fileopen.remotedir,
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if( code==0 )
  {
        if(!create_cmd_gaussian(entrytext0, FALSE, cmddir, cmdfile, cmdall))
	{
  		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nI can not create cmd file\n"),-1);   
  		gtk_widget_set_sensitive(Win, TRUE);
		return;
	}
	code = 0;
	  
  }
  if(code == 0)
  {
  	code = put_file(fout,ferr,cmdfile,cmddir,"./",
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1); 
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1);
  	put_text_in_texts_widget(Text,fout,ferr);
	unlink(cmdall);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("chmod u+x %s",cmdfile);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("./%s>/dev/null&",cmdfile);
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nRun gaussian at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nRun gaussian at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  gtk_widget_set_sensitive(Win, TRUE);

  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_remote_molcas(GtkWidget *b,gpointer data)
{  
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);

  gchar cmdfile[BSIZE];
  gchar cmddir[BSIZE];
  gchar cmdall[BSIZE];

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *temp;
  gchar *NomFichier;
  gchar *Command;
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  gint code = 0;
  G_CONST_RETURN gchar *localdir;


  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  /* fileopen.moldenfile=g_strdup_printf("%s.molden",fileopen.projectname);*/
  fileopen.moldenfile=g_strdup_printf("'%s.*.molden'", fileopen.projectname);

  fileopen.remotehost = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[2])));
  fileopen.remoteuser = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[3])));
  fileopen.remotepass  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[4])));
  fileopen.remotedir  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[5])));
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));
  /* fileopen.netWorkProtocol Deja defini dans run_program*/
  
/*   Debug("remote molcas : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_MOLCAS],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_MOLCAS, fileopen.command, fileopen.netWorkProtocol); 
  add_host(fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir);

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  title = g_strdup_printf(_("Run molcas at host :%s, Login : %s"),fileopen.remotehost,fileopen.remoteuser); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  gtk_widget_show_all(Win);
  while( gtk_events_pending() ) gtk_main_iteration();
  gtk_widget_set_sensitive(Win, FALSE);

  if(!this_is_a_backspace(fileopen.remotedir))
  {
	/* Make Working directory */
	/*  Debug("Make dir remote molcas : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	Command = g_strdup_printf("mkdir %s",fileopen.remotedir);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	g_free(Command);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }

  if(code == 0)
  {
	/* put file.com */
	/*  Debug("Put File remote molcas : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	code = put_file(fout,ferr,fileopen.datafile,fileopen.localdir,fileopen.remotedir,
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if( code==0 )
  {
        if(!create_cmd_molcas(entrytext0, FALSE, cmddir, cmdfile, cmdall))
	{
  		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nI can not create cmd file\n"),-1);   
  		gtk_widget_set_sensitive(Win, TRUE);
		return;
	}
	code = 0;
  }
  if(code == 0)
  {
  	code = put_file(fout,ferr,cmdfile,cmddir,"./",
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1); 
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1);
  	put_text_in_texts_widget(Text,fout,ferr);
	unlink(cmdall);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("chmod u+x %s",cmdfile);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("./%s>/dev/null&",cmdfile);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nRun molcas at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nRun molcas at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
 }
  gtk_widget_set_sensitive(Win, TRUE);

  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_remote_molpro(GtkWidget *b,gpointer data)
{  
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);

  gchar cmdfile[BSIZE];
  gchar cmddir[BSIZE];
  gchar cmdall[BSIZE];

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *temp;
  gchar *Tompon;
  gchar *pdest;
  gchar *t;
  gchar *NomFichier;
  gchar *Command;
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  gint code = 0;
  G_CONST_RETURN gchar *localdir;


  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  /*Debug("localdir = %s\n",fileopen.localdir);*/
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.molden",fileopen.projectname);
  lowercase(fileopen.moldenfile);

  fileopen.remotehost = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[2])));
  fileopen.remoteuser = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[3])));
  fileopen.remotepass  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[4])));
  fileopen.remotedir  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[5])));
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));
  /* fileopen.netWorkProtocol Deja defini dans run_program*/
  
/*   Debug("remote molpro : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/

  Tompon=gabedit_text_get_chars(text,0,-1);
  pdest = strstr( Tompon, fileopen.moldenfile);
  if( pdest == NULL )
  {
  	pdest = strstr( Tompon,"put,molden");
	if(pdest)
	{
		temp = strstr(pdest,";");
		t = strstr(pdest,"\n");
		if(temp && t && t<temp)
			temp = t;
		if(!temp)
			temp = t;
		if(temp)
		{
/*			Debug("%d\n",(guint)(pdest-Tompon));*/
/*			Debug("%d\n",(guint)(temp-pdest));*/
        		gabedit_text_set_point(GABEDIT_TEXT(text),(guint)(pdest-Tompon));
        		gabedit_text_forward_delete(GABEDIT_TEXT(text),(guint)(temp-pdest+1));
		}
		
	}
	temp = g_strdup_printf("put,molden,%s",fileopen.moldenfile);
	gabedit_text_set_point(GABEDIT_TEXT(text),gabedit_text_get_length(GABEDIT_TEXT(text)));  
  	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,temp,-1);
	g_free(temp);
   	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
  }
  if(Tompon != NULL) 
	g_free(Tompon);

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_MOLPRO],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_MOLPRO, fileopen.command, fileopen.netWorkProtocol); 
  add_host(fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir);

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  title = g_strdup_printf(_("Run molpro at host :%s, Login : %s"),fileopen.remotehost,fileopen.remoteuser); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  gtk_widget_show_all(Win);
  while( gtk_events_pending() ) gtk_main_iteration();
  gtk_widget_set_sensitive(Win, FALSE);

  if(!this_is_a_backspace(fileopen.remotedir))
  {
	/* Make Working directory */
	/*  Debug("Make dir remote molpro : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	Command = g_strdup_printf("mkdir %s",fileopen.remotedir);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	g_free(Command);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }

  if(code == 0)
  {
	/* put file.com */
	/*  Debug("Put File remote molpro : %s %s %s\n",fileopen.remotehost,fileopen.remoteuser,fileopen.remotedir);*/
  	code = put_file(fout,ferr,fileopen.datafile,fileopen.localdir,fileopen.remotedir,
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if( code==0 )
  {
        if(!create_cmd_molpro(entrytext0, FALSE, cmddir, cmdfile, cmdall))
	{
  		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nI can not create cmd file\n"),-1);   
  		gtk_widget_set_sensitive(Win, TRUE);
		return;
	}
	code = 0;
	  
  }
  if(code == 0)
  {
  	code = put_file(fout,ferr,cmdfile,cmddir,"./",
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1); 
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1);
  	put_text_in_texts_widget(Text,fout,ferr);
	unlink(cmdall);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("chmod u+x %s",cmdfile);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("./%s>/dev/null&",cmdfile);
  	/*rsh (fout,ferr,Command,fileopen.remoteuser,fileopen.remotehost);*/
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nRun molpro at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nRun molpro at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
 }
  gtk_widget_set_sensitive(Win, TRUE);

  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_remote_mpqc(GtkWidget *b,gpointer data)
{  
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);

  gchar cmdfile[BSIZE];
  gchar cmddir[BSIZE];
  gchar cmdall[BSIZE];

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *temp;
  gchar *NomFichier;
  gchar *Command;
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  gint code = 0;
  G_CONST_RETURN gchar *localdir;


  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.molden",fileopen.projectname);

  fileopen.remotehost = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[2])));
  fileopen.remoteuser = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[3])));
  fileopen.remotepass  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[4])));
  fileopen.remotedir  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[5])));
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_MPQC],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_MPQC, fileopen.command, fileopen.netWorkProtocol); 
  add_host(fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir);

  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  title = g_strdup_printf(_("Run MPQC at host :%s, Login : %s"),fileopen.remotehost,fileopen.remoteuser); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  gtk_widget_show_all(Win);
  while( gtk_events_pending() ) gtk_main_iteration();
  gtk_widget_set_sensitive(Win, FALSE);

  if(!this_is_a_backspace(fileopen.remotedir))
  {
	/* Make Working directory */
  	Command = g_strdup_printf("mkdir %s",fileopen.remotedir);
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	g_free(Command);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }

  if(code == 0)
  {
	/* put file.com */
  	code = put_file(fout,ferr,fileopen.datafile,fileopen.localdir,fileopen.remotedir,
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if( code==0 )
  {
        if(!create_cmd_mpqc(entrytext0, FALSE, cmddir, cmdfile, cmdall))
	{
  		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nI can not create cmd file\n"),-1);   
  		gtk_widget_set_sensitive(Win, TRUE);
		return;
	}
	code = 0;
	  
  }
  if(code == 0)
  {
  	code = put_file(fout,ferr,cmdfile,cmddir,"./",
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1); 
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1);
  	put_text_in_texts_widget(Text,fout,ferr);
	unlink(cmdall);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("chmod u+x %s",cmdfile);
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("./%s>/dev/null&",cmdfile);
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nRun MPQC at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nRun MPQC at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  gtk_widget_set_sensitive(Win, TRUE);
  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_remote_other(GtkWidget *b,gpointer data)
{  
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);

  gchar cmdfile[BSIZE];
  gchar cmddir[BSIZE];
  gchar cmdall[BSIZE];

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *temp;
  gchar *NomFichier;
  gchar *Command;
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  gint code = 0;
  G_CONST_RETURN gchar *localdir;


  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.log",fileopen.projectname);

  fileopen.remotehost = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[2])));
  fileopen.remoteuser = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[3])));
  fileopen.remotepass  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[4])));
  fileopen.remotedir  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[5])));
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));
  /* fileopen.netWorkProtocol Deja defini dans run_program*/
  

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_OTHER],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_OTHER, fileopen.command, fileopen.netWorkProtocol); 
  add_host(fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir);


  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  title = g_strdup_printf(_("Run \"%s %s\" Command  at host : %s, Login : %s"), 
		  entrytext0, fileopen.datafile, fileopen.remotehost,fileopen.remoteuser); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  gtk_widget_show_all(Win);
  while( gtk_events_pending() ) gtk_main_iteration();
  gtk_widget_set_sensitive(Win, FALSE);

  if(!this_is_a_backspace(fileopen.remotedir))
  {
	/* Make Working directory */
  	Command = g_strdup_printf("mkdir %s",fileopen.remotedir);
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	g_free(Command);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nMake Working Directory remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }

  if(code == 0)
  {
	/* put file.com */
  	code = put_file(fout,ferr,fileopen.datafile,fileopen.localdir,fileopen.remotedir,
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut Data File at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if( code==0 )
  {
        if(!create_cmd_other(entrytext0, FALSE, cmddir, cmdfile, cmdall))
	{
  		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nI can not create cmd file\n"),-1);   
  		gtk_widget_set_sensitive(Win, TRUE);
		return;
	}
	code = 0;
	  
  }
  if(code == 0)
  {
  	code = put_file(fout,ferr,cmdfile,cmddir,"./",
		  	fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1); 
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nPut CMD File at remote host :\n "),-1);
  	put_text_in_texts_widget(Text,fout,ferr);
	unlink(cmdall);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("chmod u+x %s",cmdfile);
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nchmod for cmd file :\n"),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  if(code == 0)
  {
  	Command = g_strdup_printf("./%s>/dev/null&",cmdfile);
  	remote_command (fout,ferr,Command,fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass);
  	gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,_("\nRun gaussian at remote host :\n "),-1);   
  	gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,_("\nRun gaussian at remote host :\n "),-1);   
  	put_text_in_texts_widget(Text,fout,ferr);
  	while( gtk_events_pending() )
          gtk_main_iteration();
  }
  gtk_widget_set_sensitive(Win, TRUE);
  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_local_demon(GtkWidget *b,gpointer data)
{  
#ifdef G_OS_WIN32
  gchar *fout =  g_strdup_printf("\"%s%stmp%sfout\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("\"%s%stmp%sferr\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#else
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#endif

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *strout;
  gchar *strerr;
  gchar *temp;
  gchar *NomFichier;
  gchar Command[BSIZE];
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  G_CONST_RETURN gchar *localdir;
  gchar cmdFileAllName[BSIZE];
  gchar cmdDir[BSIZE];
  gchar cmdFile[BSIZE];


  unlink(fout);
  unlink(ferr);

  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotedir = NULL;

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotepass = NULL;
  fileopen.remotedir = NULL;
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_ORCA],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_ORCA, fileopen.command, fileopen.netWorkProtocol); 

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  if(!create_cmd_demon(entrytext0, TRUE, cmdDir, cmdFile, cmdFileAllName)) return;
#ifdef G_OS_WIN32
  sprintf(Command ,"\"%s\"",cmdFileAllName);
#else
  sprintf(Command ,"%s",cmdFileAllName);
#endif

  run_local_command(fout,ferr,Command,TRUE);
  title = g_strdup_printf(_("Run DeMon in local : %s"),Command); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  strout = cat_file(fout,FALSE);
  strerr = cat_file(ferr,FALSE);
  if(!strout && !strerr)
  	destroy_children(Win);
  else
  {
  	if(strout)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,strout,-1);   
		g_free(strout);
	}
  	if(strerr)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,strerr,-1);   
		g_free(strerr);
	}
  	gtk_widget_show_all(Win);
  	if(!strout)
  		gtk_widget_hide(Frame[0]);
  }
  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_local_orca(GtkWidget *b,gpointer data)
{  
#ifdef G_OS_WIN32
  gchar *fout =  g_strdup_printf("\"%s%stmp%sfout\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("\"%s%stmp%sferr\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#else
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#endif

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *strout;
  gchar *strerr;
  gchar *temp;
  gchar *NomFichier;
  gchar Command[BSIZE];
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  G_CONST_RETURN gchar *localdir;
  gchar cmdFileAllName[BSIZE];
  gchar cmdDir[BSIZE];
  gchar cmdFile[BSIZE];


  unlink(fout);
  unlink(ferr);

  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotedir = NULL;

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotepass = NULL;
  fileopen.remotedir = NULL;
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_ORCA],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_ORCA, fileopen.command, fileopen.netWorkProtocol); 

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  if(!create_cmd_orca(entrytext0, TRUE, cmdDir, cmdFile, cmdFileAllName)) return;
#ifdef G_OS_WIN32
  sprintf(Command ,"\"%s\"",cmdFileAllName);
#else
  sprintf(Command ,"%s",cmdFileAllName);
#endif

  run_local_command(fout,ferr,Command,TRUE);
  title = g_strdup_printf(_("Run Orca in local : %s"),Command); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  strout = cat_file(fout,FALSE);
  strerr = cat_file(ferr,FALSE);
  if(!strout && !strerr)
  	destroy_children(Win);
  else
  {
  	if(strout)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,strout,-1);   
		g_free(strout);
	}
  	if(strerr)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,strerr,-1);   
		g_free(strerr);
	}
  	gtk_widget_show_all(Win);
  	if(!strout)
  		gtk_widget_hide(Frame[0]);
  }
  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_local_nwchem(GtkWidget *b,gpointer data)
{  
#ifdef G_OS_WIN32
  gchar *fout =  g_strdup_printf("\"%s%stmp%sfout\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("\"%s%stmp%sferr\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#else
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#endif

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *strout;
  gchar *strerr;
  gchar *temp;
  gchar *NomFichier;
  gchar Command[BSIZE];
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  G_CONST_RETURN gchar *localdir;
  gchar cmdFileAllName[BSIZE];
  gchar cmdDir[BSIZE];
  gchar cmdFile[BSIZE];


  unlink(fout);
  unlink(ferr);

  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.nw",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotedir = NULL;

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotepass = NULL;
  fileopen.remotedir = NULL;
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_NWCHEM],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_NWCHEM, fileopen.command, fileopen.netWorkProtocol); 

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  if(!create_cmd_nwchem(entrytext0, TRUE, cmdDir, cmdFile, cmdFileAllName)) return;
#ifdef G_OS_WIN32
  sprintf(Command ,"\"%s\"",cmdFileAllName);
#else
  sprintf(Command ,"%s",cmdFileAllName);
#endif

  run_local_command(fout,ferr,Command,TRUE);
  title = g_strdup_printf(_("Run NWChem in local : %s"),Command); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  strout = cat_file(fout,FALSE);
  strerr = cat_file(ferr,FALSE);
  if(!strout && !strerr)
  	destroy_children(Win);
  else
  {
  	if(strout)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,strout,-1);   
		g_free(strout);
	}
  	if(strerr)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,strerr,-1);   
		g_free(strerr);
	}
  	gtk_widget_show_all(Win);
  	if(!strout)
  		gtk_widget_hide(Frame[0]);
  }
  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_local_psicode(GtkWidget *b,gpointer data)
{  
#ifdef G_OS_WIN32
  gchar *fout =  g_strdup_printf("\"%s%stmp%sfout\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("\"%s%stmp%sferr\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#else
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#endif

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *strout;
  gchar *strerr;
  gchar *temp;
  gchar *NomFichier;
  gchar Command[BSIZE];
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  G_CONST_RETURN gchar *localdir;
  gchar cmdFileAllName[BSIZE];
  gchar cmdDir[BSIZE];
  gchar cmdFile[BSIZE];


  unlink(fout);
  unlink(ferr);

  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.psi",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotedir = NULL;

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotepass = NULL;
  fileopen.remotedir = NULL;
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_PSICODE],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_PSICODE, fileopen.command, fileopen.netWorkProtocol); 

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  if(!create_cmd_psicode(entrytext0, TRUE, cmdDir, cmdFile, cmdFileAllName)) return;
#ifdef G_OS_WIN32
  sprintf(Command ,"\"%s\"",cmdFileAllName);
#else
  sprintf(Command ,"%s",cmdFileAllName);
#endif

  run_local_command(fout,ferr,Command,TRUE);
  title = g_strdup_printf(_("Run Psicode in local : %s"),Command); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  strout = cat_file(fout,FALSE);
  strerr = cat_file(ferr,FALSE);
  if(!strout && !strerr)
  	destroy_children(Win);
  else
  {
  	if(strout)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,strout,-1);   
		g_free(strout);
	}
  	if(strerr)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,strerr,-1);   
		g_free(strerr);
	}
  	gtk_widget_show_all(Win);
  	if(!strout)
  		gtk_widget_hide(Frame[0]);
  }
  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_local_firefly(GtkWidget *b,gpointer data)
{  
#ifdef G_OS_WIN32
  gchar *fout =  g_strdup_printf("\"%s%stmp%sfout\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("\"%s%stmp%sferr\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#else
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#endif

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *strout;
  gchar *strerr;
  gchar *temp;
  gchar *NomFichier;
  gchar Command[BSIZE];
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  G_CONST_RETURN gchar *localdir;
  gchar cmdFileAllName[BSIZE];
  gchar cmdDir[BSIZE];
  gchar cmdFile[BSIZE];

  unlink(fout);
  unlink(ferr);

  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotedir = NULL;

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotepass = NULL;
  fileopen.remotedir = NULL;
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_FIREFLY],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_FIREFLY, fileopen.command, fileopen.netWorkProtocol); 

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  if(!create_cmd_firefly(entrytext0, TRUE, cmdDir, cmdFile, cmdFileAllName)) return;
#ifdef G_OS_WIN32
  sprintf(Command ,"\"%s\"",cmdFileAllName);
#else
  sprintf(Command ,"%s",cmdFileAllName);
#endif

  run_local_command(fout,ferr,Command,TRUE);
  title = g_strdup_printf(_("Run FireFly in local : %s"),Command); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  strout = cat_file(fout,FALSE);
  strerr = cat_file(ferr,FALSE);
  if(!strout && !strerr)
  	destroy_children(Win);
  else
  {
  	if(strout)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,strout,-1);   
		g_free(strout);
	}
  	if(strerr)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,strerr,-1);   
		g_free(strerr);
	}
  	gtk_widget_show_all(Win);
  	if(!strout)
  		gtk_widget_hide(Frame[0]);
  }
  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_local_qchem(GtkWidget *b,gpointer data)
{  
#ifdef G_OS_WIN32
  gchar *fout =  g_strdup_printf("\"%s%stmp%sfout\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("\"%s%stmp%sferr\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#else
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#endif

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *strout;
  gchar *strerr;
  gchar *temp;
  gchar *NomFichier;
  gchar Command[BSIZE];
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  G_CONST_RETURN gchar *localdir;
  gchar cmdFileAllName[BSIZE];
  gchar cmdDir[BSIZE];
  gchar cmdFile[BSIZE];


  unlink(fout);
  unlink(ferr);

  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotedir = NULL;

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotepass = NULL;
  fileopen.remotedir = NULL;
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_QCHEM],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_QCHEM, fileopen.command, fileopen.netWorkProtocol); 

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  if(!create_cmd_qchem(entrytext0, TRUE, cmdDir, cmdFile, cmdFileAllName)) return;
#ifdef G_OS_WIN32
  sprintf(Command ,"\"%s\"",cmdFileAllName);
#else
  sprintf(Command ,"%s",cmdFileAllName);
#endif

  run_local_command(fout,ferr,Command,TRUE);
  title = g_strdup_printf(_("Run Q-Chem in local : %s"),Command); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  strout = cat_file(fout,FALSE);
  strerr = cat_file(ferr,FALSE);
  if(!strout && !strerr)
  	destroy_children(Win);
  else
  {
  	if(strout)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,strout,-1);   
		g_free(strout);
	}
  	if(strerr)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,strerr,-1);   
		g_free(strerr);
	}
  	gtk_widget_show_all(Win);
  	if(!strout)
  		gtk_widget_hide(Frame[0]);
  }
  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_local_mopac(GtkWidget *b,gpointer data)
{  
#ifdef G_OS_WIN32
  gchar *fout =  g_strdup_printf("\"%s%stmp%sfout\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("\"%s%stmp%sferr\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#else
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#endif

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *strout;
  gchar *strerr;
  gchar *temp;
  gchar *NomFichier;
  gchar Command[BSIZE];
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  G_CONST_RETURN gchar *localdir;
  gchar cmdFileAllName[BSIZE];
  gchar cmdDir[BSIZE];
  gchar cmdFile[BSIZE];


  unlink(fout);
  unlink(ferr);

  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.mop",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.aux",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotedir = NULL;

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotepass = NULL;
  fileopen.remotedir = NULL;
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_MOPAC],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_MOPAC, fileopen.command, fileopen.netWorkProtocol); 

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  if(!create_cmd_mopac(entrytext0, TRUE, cmdDir, cmdFile, cmdFileAllName)) return;
#ifdef G_OS_WIN32
  sprintf(Command ,"\"%s\"",cmdFileAllName);
#else
  sprintf(Command ,"%s",cmdFileAllName);
#endif

  run_local_command(fout,ferr,Command,TRUE);
  title = g_strdup_printf(_("Run Mopac in local : %s"),Command); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  strout = cat_file(fout,FALSE);
  strerr = cat_file(ferr,FALSE);
  if(!strout && !strerr)
  	destroy_children(Win);
  else
  {
  	if(strout)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,strout,-1);   
		g_free(strout);
	}
  	if(strerr)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,strerr,-1);   
		g_free(strerr);
	}
  	gtk_widget_show_all(Win);
  	if(!strout)
  		gtk_widget_hide(Frame[0]);
  }
  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_local_gamess(GtkWidget *b,gpointer data)
{  
#ifdef G_OS_WIN32
  gchar *fout =  g_strdup_printf("\"%s%stmp%sfout\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("\"%s%stmp%sferr\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#else
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#endif

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *strout;
  gchar *strerr;
  gchar *temp;
  gchar *NomFichier;
  gchar Command[BSIZE];
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  G_CONST_RETURN gchar *localdir;
  gchar cmdFileAllName[BSIZE];
  gchar cmdDir[BSIZE];
  gchar cmdFile[BSIZE];

  unlink(fout);
  unlink(ferr);

  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.inp",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotedir = NULL;

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotepass = NULL;
  fileopen.remotedir = NULL;
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_GAMESS],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_GAMESS, fileopen.command, fileopen.netWorkProtocol); 

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  if(!create_cmd_gamess(entrytext0, TRUE, cmdDir, cmdFile, cmdFileAllName)) return;
#ifdef G_OS_WIN32
  sprintf(Command ,"\"%s\"",cmdFileAllName);
#else
  sprintf(Command ,"%s&",cmdFileAllName);
#endif

  run_local_command(fout,ferr,Command,TRUE);
  title = g_strdup_printf(_("Run gamess in local : %s"),Command); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  strout = cat_file(fout,FALSE);
  strerr = cat_file(ferr,FALSE);
  if(!strout && !strerr)
  	destroy_children(Win);
  else
  {
  	if(strout)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,strout,-1);   
		g_free(strout);
	}
  	if(strerr)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,strerr,-1);   
		g_free(strerr);
	}
  	gtk_widget_show_all(Win);
  	if(!strout)
  		gtk_widget_hide(Frame[0]);
  }
  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_local_gaussian(GtkWidget *b,gpointer data)
{  
#ifdef G_OS_WIN32
  gchar *fout =  g_strdup_printf("\"%s%stmp%sfout\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("\"%s%stmp%sferr\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#else
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#endif

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *strout;
  gchar *strerr;
  gchar *temp;
  gchar *NomFichier;
  gchar Command[BSIZE];
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  G_CONST_RETURN gchar *localdir;
  gchar cmdFileAllName[BSIZE];
  gchar cmdDir[BSIZE];
  gchar cmdFile[BSIZE];

  unlink(fout);
  unlink(ferr);

  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotedir = NULL;

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotepass = NULL;
  fileopen.remotedir = NULL;
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_GAUSSIAN],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_GAUSSIAN, fileopen.command, fileopen.netWorkProtocol); 

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  if(!create_cmd_gaussian(entrytext0, TRUE, cmdDir, cmdFile, cmdFileAllName)) return;
  sprintf(Command ,"%s",cmdFileAllName);

  run_local_command(fout,ferr,Command,TRUE);
  title = g_strdup_printf(_("Run gaussian in local : %s"),Command); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  strout = cat_file(fout,FALSE);
  strerr = cat_file(ferr,FALSE);
  if(!strout && !strerr)
  	destroy_children(Win);
  else
  {
  	if(strout)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,strout,-1);   
		g_free(strout);
	}
  	if(strerr)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,strerr,-1);   
		g_free(strerr);
	}
  	gtk_widget_show_all(Win);
  	if(!strout)
  		gtk_widget_hide(Frame[0]);
  }
  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_local_molcas(GtkWidget *b,gpointer data)
{  
#ifdef G_OS_WIN32
  gchar *fout =  g_strdup_printf("\"%s%stmp%sfout\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("\"%s%stmp%sferr\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#else
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#endif

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *strout;
  gchar *strerr;
  gchar *temp;
  gchar *NomFichier;
  gchar *Command;
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  G_CONST_RETURN gchar *localdir;
  gchar cmdFileAllName[BSIZE];
  gchar cmdDir[BSIZE];
  gchar cmdFile[BSIZE];

  unlink(fout);
  unlink(ferr);

  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);


  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  /* fileopen.moldenfile=g_strdup_printf("%s.molden",fileopen.projectname);*/
  fileopen.moldenfile=g_strdup_printf("'%s.*.molden'", fileopen.projectname);
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotedir = NULL;

  /* save file */
  NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotepass = NULL;
  fileopen.remotedir = NULL;
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_MOLCAS],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_MOLCAS, fileopen.command, fileopen.netWorkProtocol); 

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  if(!create_cmd_molcas(entrytext0, TRUE, cmdDir, cmdFile, cmdFileAllName)) return;
  
  Command = g_strdup_printf("%s ",cmdFileAllName);
 


  create_popup_win(_("\nPlease Wait\n"));
  run_local_command(fout,ferr,Command,TRUE);

  title = g_strdup_printf(_("Run molcas in local : %s"),Command); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  strout = cat_file(fout,FALSE);
  strerr = cat_file(ferr,FALSE);
  if(!strout && !strerr) destroy_children(Win);
  else
  {
  	if(strout)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,strout,-1);   
		g_free(strout);
	}
  	if(strerr)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,strerr,-1);   
		g_free(strerr);
	}
  	gtk_widget_show_all(Win);
  	if(!strout)
  		gtk_widget_hide(Frame[0]);
  }
  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_local_molpro(GtkWidget *b,gpointer data)
{  
#ifdef G_OS_WIN32
  gchar *fout =  g_strdup_printf("\"%s%stmp%sfout\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("\"%s%stmp%sferr\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#else
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#endif

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *strout;
  gchar *strerr;
  gchar *temp;
  gchar *t;
  gchar *NomFichier;
  gchar Command[BSIZE];
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  gchar *pdest;
  gchar *Tompon;
  G_CONST_RETURN gchar *localdir;
  gchar cmdFileAllName[BSIZE];
  gchar cmdDir[BSIZE];
  gchar cmdFile[BSIZE];

  unlink(fout);
  unlink(ferr);

  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);


  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.molden",fileopen.projectname);
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotedir = NULL;

  Tompon=gabedit_text_get_chars(text,0,-1);
  pdest = strstr( Tompon, fileopen.moldenfile);
  if( pdest == NULL )
  {
  	pdest = strstr( Tompon,"put,molden");
	if(pdest)
	{
		temp = strstr(pdest,";");
		t = strstr(pdest,"\n");
		if(temp && t && t<temp)
			temp = t;
		if(!temp)
			temp = t;
		if(temp)
		{
/*			Debug("%d\n",(guint)(pdest-Tompon));*/
/*			Debug("%d\n",(guint)(temp-pdest));*/
        		gabedit_text_set_point(GABEDIT_TEXT(text),(guint)(pdest-Tompon));
        		gabedit_text_forward_delete(GABEDIT_TEXT(text),(guint)(temp-pdest+1));
		}
		
	}
	temp = g_strdup_printf("put,molden,%s",fileopen.moldenfile);
	gabedit_text_set_point(GABEDIT_TEXT(text), gabedit_text_get_length(GABEDIT_TEXT(text)));  
  	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,temp,-1);
	g_free(temp);
   	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
  }
  if(Tompon != NULL) 
	g_free(Tompon);

  /* save file */
  NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotepass = NULL;
  fileopen.remotedir = NULL;
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_MOLPRO],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_MOLPRO, fileopen.command, fileopen.netWorkProtocol); 

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  if(!create_cmd_molpro(entrytext0, TRUE, cmdDir, cmdFile, cmdFileAllName)) return;
  sprintf(Command ,"%s",cmdFileAllName);


  create_popup_win(_("\nPlease Wait\n"));
  run_local_command(fout,ferr,Command,TRUE);
  title = g_strdup_printf(_("Run molpro in local : %s"),Command); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  strout = cat_file(fout,FALSE);
  strerr = cat_file(ferr,FALSE);
  if(!strout && !strerr)
  	destroy_children(Win);
  else
  {
  	if(strout)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,strout,-1);   
		g_free(strout);
	}
  	if(strerr)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,strerr,-1);   
		g_free(strerr);
	}
  	gtk_widget_show_all(Win);
  	if(!strout)
  		gtk_widget_hide(Frame[0]);
  }
  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_local_mpqc(GtkWidget *b,gpointer data)
{  
#ifdef G_OS_WIN32
  gchar *fout =  g_strdup_printf("\"%s%stmp%sfout\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("\"%s%stmp%sferr\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#else
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#endif

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *strout;
  gchar *strerr;
  gchar *temp;
  gchar *NomFichier;
  gchar *Command;
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  G_CONST_RETURN gchar *localdir;
  gchar cmdFileAllName[BSIZE];
  gchar cmdDir[BSIZE];
  gchar cmdFile[BSIZE];

  unlink(fout);
  unlink(ferr);

  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);


  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.out",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.out",fileopen.projectname);
  /* fileopen.moldenfile=g_strdup_printf("%s.molden",fileopen.projectname);*/
  fileopen.moldenfile=g_strdup_printf("'%s.*.molden'", fileopen.projectname);
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotedir = NULL;

  /* save file */
  NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotepass = NULL;
  fileopen.remotedir = NULL;
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_MPQC],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_MPQC, fileopen.command, fileopen.netWorkProtocol); 

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  if(!create_cmd_mpqc(entrytext0, TRUE, cmdDir, cmdFile, cmdFileAllName)) return;
  
  Command = g_strdup_printf("%s ",cmdFileAllName);
 


  create_popup_win(_("\nPlease Wait\n"));
  run_local_command(fout,ferr,Command,TRUE);

  title = g_strdup_printf(_("Run MPQC in local : %s"),Command); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  strout = cat_file(fout,FALSE);
  strerr = cat_file(ferr,FALSE);
  if(!strout && !strerr) destroy_children(Win);
  else
  {
  	if(strout)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,strout,-1);   
		g_free(strout);
	}
  	if(strerr)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,strerr,-1);   
		g_free(strerr);
	}
  	gtk_widget_show_all(Win);
  	if(!strout)
  		gtk_widget_hide(Frame[0]);
  }
  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
static void run_local_other(GtkWidget *b,gpointer data)
{  
#ifdef G_OS_WIN32
  gchar *fout =  g_strdup_printf("\"%s%stmp%sfout\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("\"%s%stmp%sferr\"",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#else
  gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
#endif

  GtkWidget* Win;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  gchar *strout;
  gchar *strerr;
  gchar *temp;
  gchar *NomFichier;
  gchar Command[BSIZE];
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext0;
  gchar *entrytext;
  gchar* title;
  G_CONST_RETURN gchar *localdir;
  gchar cmdFileAllName[BSIZE];
  gchar cmdDir[BSIZE];
  gchar cmdFile[BSIZE];

  unlink(fout);
  unlink(ferr);

  entryall=(GtkWidget **)data;
  entry=entryall[0];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));
  localdir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  entrytext = get_dir_file_name(localdir,entrytext0);

  temp = get_suffix_name_file(entrytext);
  fileopen.projectname = get_name_file(temp);
  fileopen.localdir = get_name_dir(temp);
  g_free(temp);
  fileopen.datafile = g_strdup_printf("%s.com",fileopen.projectname);
  fileopen.outputfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.logfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.moldenfile=g_strdup_printf("%s.log",fileopen.projectname);
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotedir = NULL;

  /* save file */
   NomFichier = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
  
  fileopen.remotehost = NULL;
  fileopen.remoteuser = NULL;
  fileopen.remotepass = NULL;
  fileopen.remotedir = NULL;
  fileopen.command  = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryall[1])));
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_OTHER],fileopen.projectname,fileopen.datafile,fileopen.localdir,
			fileopen.remotehost,fileopen.remoteuser,fileopen.remotepass,fileopen.remotedir,GABEDIT_TYPENODE_OTHER, fileopen.command, fileopen.netWorkProtocol); 

/* Save file in local host */
  if(!save_local_doc(NomFichier)) return;

  data_modify(FALSE);

  entry=entryall[1];
  entrytext0 = gtk_entry_get_text(GTK_ENTRY(entry));

  /*
  printf("entrytext = %s\n",entrytext);
  */

  if(!create_cmd_other(entrytext0, TRUE, cmdDir, cmdFile, cmdFileAllName)) return;

  sprintf(Command ,"%s",cmdFileAllName);
  

  run_local_command(fout,ferr,Command,TRUE);
  title = g_strdup_printf(_("Run \"%s %s\" command in local host"), entrytext0, fileopen.datafile); 
  Win = create_text_result_command(Text,Frame,title);
  g_free(title);
  strout = cat_file(fout,FALSE);
  strerr = cat_file(ferr,FALSE);
  if(!strout && !strerr)
  	destroy_children(Win);
  else
  {
  	if(strout)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,strout,-1);   
		g_free(strout);
	}
  	if(strerr)
	{
 		gabedit_text_insert (GABEDIT_TEXT(Text[1]), NULL, NULL, NULL,strerr,-1);   
		g_free(strerr);
	}
  	gtk_widget_show_all(Win);
  	if(!strout)
  		gtk_widget_hide(Frame[0]);
  }
  g_free(fout);
  g_free(ferr);
}
/********************************************************************************/
void run_program(GtkWidget *button,gpointer data)
{
  	GtkWidget* ButtonSsh = g_object_get_data(G_OBJECT (button),"ButtonSsh");
	/*
  	GtkWidget* ButtonFtpRsh = g_object_get_data(G_OBJECT (button),"ButtonFtpRsh");
	*/

	if(ButtonSsh && GTK_TOGGLE_BUTTON (ButtonSsh)->active)
	{
		fileopen.netWorkProtocol = GABEDIT_NETWORK_SSH;
	}
	else
	{
		fileopen.netWorkProtocol = GABEDIT_NETWORK_FTP_RSH;
	}

	if(GTK_TOGGLE_BUTTON (ButtonLocal)->active)
	{
		if (GTK_TOGGLE_BUTTON (ButtonGamess)->active) run_local_gamess(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonGauss)->active) run_local_gaussian(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonMolcas)->active) run_local_molcas(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonMPQC)->active) run_local_mpqc(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonMolpro)->active) run_local_molpro(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonOrca)->active) run_local_orca(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonDeMon)->active) run_local_demon(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonNWChem)->active) run_local_nwchem(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonPsicode)->active) run_local_psicode(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonFireFly)->active) run_local_firefly(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonQChem)->active) run_local_qchem(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonMopac)->active) run_local_mopac(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonOther)->active) run_local_other(NULL,data);
	}
	else
	{
		if (GTK_TOGGLE_BUTTON (ButtonGamess)->active) run_remote_gamess(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonGauss)->active) run_remote_gaussian(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonMolcas)->active) run_remote_molcas(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonMPQC)->active) run_remote_mpqc(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonMolpro)->active) run_remote_molpro(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonOrca)->active) run_remote_orca(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonDeMon)->active) run_remote_demon(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonNWChem)->active) run_remote_nwchem(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonPsicode)->active) run_remote_psicode(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonFireFly)->active) run_remote_firefly(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonQChem)->active) run_remote_qchem(NULL,data);
		else if (GTK_TOGGLE_BUTTON (ButtonMopac)->active) run_remote_mopac(NULL,data);
		else run_remote_other(NULL,data);
	}
	gtk_notebook_set_current_page((GtkNotebook*)NoteBookText,0);
}
/********************************************************************************/
/*
static void set_entrys_folder(GtkWidget* selFolder)
{
  GtkWidget *entry = (GtkWidget*)(g_object_get_data(G_OBJECT(selFolder),"EntryDirectory"));	
  gchar *dirname = gabedit_folder_chooser_get_current_folder((GabeditFolderChooser *)selFolder);
  gchar *longfile = g_strdup_printf("%s%sdump.txt",dirname, G_DIR_SEPARATOR_S);
  gtk_entry_set_text(GTK_ENTRY(entry),dirname);
  set_last_directory(longfile);
}
*/
/********************************************************************************/
/*
static void set_entry_directory_selection(GtkWidget* table)
{
  GtkWidget *selFolder;
  GtkWidget *Win = (GtkWidget*)(g_object_get_data(G_OBJECT(table),"Window"));
  GtkWidget *entrydirectory = (GtkWidget*)(g_object_get_data(G_OBJECT(table),"Entry"));

  selFolder = gabedit_folder_chooser_new("Folder chooser");
  add_child(Win,selFolder,gtk_widget_destroy," Folder selection ");
  g_signal_connect(G_OBJECT(selFolder),"delete_event",(GCallback)delete_child,NULL);

  g_object_set_data (G_OBJECT (selFolder), "EntryDirectory", entrydirectory);

  g_signal_connect (selFolder, "response",  G_CALLBACK (set_entrys_folder), G_OBJECT(selFolder));
  g_signal_connect (selFolder, "response",  G_CALLBACK (delete_child), G_OBJECT(selFolder));

  gtk_widget_show(selFolder);
}
*/
/********************************************************************************/
static GtkWidget *create_hbox_browser_run(GtkWidget* Wins,GtkWidget* Table,
							   gchar *LabelRight,gchar *LabelLeft,
							   gchar *deffile,gushort row)
{
  GtkWidget *Entry = NULL;
  GtkWidget* Label;

  Label = add_label_table(Table,LabelLeft,row,0);
  Label = add_label_table(Table,":",row,1);
  Entry = gtk_entry_new();
  if(deffile) gtk_entry_set_text(GTK_ENTRY(Entry),deffile);
  gtk_table_attach(GTK_TABLE(Table),Entry,2,3,row,row+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);

  {
	GtkWidget *Label;
	GtkWidget *hbox = gtk_hbox_new(0,FALSE);
	
	Label = gtk_label_new (LabelRight);
   	gtk_label_set_justify(GTK_LABEL(Label),GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX (hbox), Label, FALSE, FALSE, 0);
	add_widget_table(Table,hbox,row,3);
	LabelExtFile = Label;
  }



  g_object_set_data (G_OBJECT (Table), "Entry", Entry);
  g_object_set_data (G_OBJECT (Table), "Label", Label);
  g_object_set_data (G_OBJECT (Table), "Window", Wins);
  return Entry;
}
/********************************************************************************/
GtkWidget* create_server_frame(GtkWidget* hbox)
{
  GtkWidget *frame;
  GtkWidget *Table;
  GtkWidget *vboxframe;

  frame = gtk_frame_new ("Server");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start (GTK_BOX( hbox), frame, TRUE, TRUE, 5);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  Table = gtk_table_new(2,1,FALSE);
  gtk_container_add(GTK_CONTAINER(vboxframe),Table);

  ButtonLocal = gtk_radio_button_new_with_label( NULL,"Local" );
  add_widget_table(Table,ButtonLocal,0,0);
  ButtonRemote = gtk_radio_button_new_with_label(
                       gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonLocal)),
                       "Remote host "); 
  add_widget_table(Table,ButtonRemote,1,0);
  return frame;
}
/********************************************************************************/
GtkWidget* create_programs_frame(GtkWidget *hbox)
{
  GtkWidget *frame;
  GtkWidget *vboxframe;
  GtkWidget *Table;

  frame = gtk_frame_new ("Program");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start (GTK_BOX( hbox), frame, TRUE, TRUE, 5);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  Table = gtk_table_new(4,4,FALSE);
  gtk_container_add(GTK_CONTAINER(vboxframe),Table);

  ButtonDeMon = gtk_radio_button_new_with_label( NULL,"DeMon" );
  add_widget_table(Table,ButtonDeMon,0,0);

  ButtonGamess = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonDeMon)), "Gamess "); 
  add_widget_table(Table,ButtonGamess,0,1);

  ButtonGauss = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonGamess)), "Gaussian "); 
  add_widget_table(Table,ButtonGauss,0,2);

  ButtonMolcas = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonGauss)), "Molcas "); 
  add_widget_table(Table,ButtonMolcas,0,3);

  ButtonMolpro = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonGauss)), "Molpro "); 
  add_widget_table(Table,ButtonMolpro,1,0);

  ButtonMopac = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonGauss)), "Mopac "); 
  add_widget_table(Table,ButtonMopac,1,1);

  ButtonMPQC = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonGauss)), "MPQC "); 
  add_widget_table(Table,ButtonMPQC,1,2);

  ButtonOrca = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonGauss)), "Orca "); 
  add_widget_table(Table,ButtonOrca,1,3);

  ButtonNWChem = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonGauss)), "NWChem "); 
  add_widget_table(Table,ButtonNWChem,2,0);



  ButtonFireFly = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonGauss)), "FireFly "); 
  add_widget_table(Table,ButtonFireFly,2,1);

  ButtonQChem = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonGauss)), "Q-Chem "); 
  add_widget_table(Table,ButtonQChem,2,2);

  ButtonPsicode = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonGauss)), "Psicode "); 
  add_widget_table(Table,ButtonPsicode,2,3);

  ButtonOther = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonGauss)), "Other "); 
  add_widget_table(Table,ButtonOther,3,0);
  return frame;
}
/********************************************************************************/
static GtkWidget *create_local_frame( GtkWidget *vboxall)
{
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gushort i;
	gushort j;
	GtkWidget *table;
	GtkWidget *label;
	gchar* title = "Local";

	frame = gtk_frame_new (title);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	table = gtk_table_new(1,3,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);

/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	label = gtk_label_new("Folder");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 2;
	buttonDirSelector =  gabedit_dir_button();
	gtk_table_attach(GTK_TABLE(table),buttonDirSelector,
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	if(fileopen.localdir && strcmp(fileopen.localdir,"NoName")!=0) gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER(buttonDirSelector), fileopen.localdir);
	gtk_widget_show_all(frame);
	g_object_set_data (G_OBJECT (frame), "ButtonDirSelector",buttonDirSelector);

  	return frame;
}
/********************************************************************************/
GtkWidget *create_local_remote_frame(GtkWidget *Window, GtkWidget *vboxall,GtkWidget **entry,gchar* type)
{
	GtkWidget* frame;
	GtkWidget* vboxframe;
	GtkWidget* Table;
	GtkWidget* Label;
	GtkWidget* combo;
#define NLAB 2
  	gchar      *LabelRight[NLAB];
  	gchar      *LabelLeft[NLAB];
  	gchar      *liste[NLAB];
	gint i;
	GtkWidget *hbox = gtk_hbox_new(0,FALSE);

  	LabelLeft[0] = g_strdup("Save data in file");
  	LabelLeft[1] = g_strdup("Command to execute");

  	LabelRight[0] = g_strdup(".com");
  	LabelRight[1] = g_strdup(" DataFile.com ");

  	liste[0]  = g_strdup(fileopen.projectname);
  	if(strstr(type,"Molpro")) liste[1]  = g_strdup(NameCommandMolpro);
  	else if(strstr(type,"Molcas")) liste[1]  = g_strdup(NameCommandMolcas);
	else liste[1]  = g_strdup(NameCommandGaussian);

  	frame = gtk_frame_new ("Local/Remote");
  	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  	gtk_container_add (GTK_CONTAINER (vboxall), frame);
  	gtk_widget_show (frame);

  	vboxframe = create_vbox(frame);
  	Table = gtk_table_new(2,5,FALSE);
  	gtk_container_add(GTK_CONTAINER(vboxframe),Table);

  	entry[0] = create_hbox_browser_run(Window,Table,
			  LabelRight[0],LabelLeft[0],
		          liste[0],0);

	add_label_table(Table,LabelLeft[1],1,0);
	add_label_table(Table,":",1,1);
  	if(strstr(type,"Molpro")) combo = create_combo_box_entry(molproCommands.commands,molproCommands.numberOfCommands,TRUE,-1,-1);
	else if(strstr(type,"Molcas")) combo = create_combo_box_entry(molcasCommands.commands,molcasCommands.numberOfCommands,TRUE,-1,-1);
	else combo = create_combo_box_entry(gaussianCommands.commands,gaussianCommands.numberOfCommands,TRUE,-1,-1);

	ComboCommand = combo;
	entry[1] = GTK_BIN(combo)->child;
	gtk_entry_set_text (GTK_ENTRY (entry[1]), liste[1]);
	if(fileopen.command && strlen(fileopen.command)>0) gtk_entry_set_text (GTK_ENTRY (entry[1]), fileopen.command);
	add_widget_table(Table,combo,1,2);

	Label = gtk_label_new(LabelRight[1]);
	LabelDataFile = Label;
   	gtk_label_set_justify(GTK_LABEL(Label),GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX (hbox), Label, FALSE, FALSE, 0);
	gtk_table_attach(GTK_TABLE(Table),hbox,3,5,1,1+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  1,1);
  	for (i=0;i<NLAB;i++)
        	g_free(LabelLeft[i]);
  	for (i=0;i<NLAB;i++)
        	g_free(LabelRight[i]);
	return frame;
}
/********************************************************************************/
static void changedEntryFileData(GtkWidget *entry,gpointer data)
{
	G_CONST_RETURN gchar* entrytext = gtk_entry_get_text(GTK_ENTRY(entry));
	if(entrytext)
	{
		gchar buffer[BSIZE];

		if (ButtonGamess && GTK_TOGGLE_BUTTON (ButtonGamess)->active)
		sprintf(buffer,"%s.inp",entrytext);
		else if (ButtonFireFly && GTK_TOGGLE_BUTTON (ButtonFireFly)->active)
		sprintf(buffer,"%s.inp",entrytext);
		else if (ButtonQChem && GTK_TOGGLE_BUTTON (ButtonQChem)->active)
		sprintf(buffer,"%s.inp",entrytext);
		else if (ButtonOrca && GTK_TOGGLE_BUTTON (ButtonOrca)->active)
		sprintf(buffer,"%s.inp",entrytext);
		else if (ButtonNWChem && GTK_TOGGLE_BUTTON (ButtonNWChem)->active)
		sprintf(buffer,"%s.nw",entrytext);
		else if (ButtonPsicode && GTK_TOGGLE_BUTTON (ButtonPsicode)->active)
		sprintf(buffer,"%s.psi",entrytext);
		else if (ButtonMopac && GTK_TOGGLE_BUTTON (ButtonMopac)->active)
		sprintf(buffer,"%s.mop",entrytext);
		else sprintf(buffer,"%s.com",entrytext);
		gtk_label_set_text(GTK_LABEL(LabelDataFile), buffer);
	}
}
/********************************************************************************/
void changed_user(GtkWidget *combo,gpointer data)
{
	GtkWidget **entry = (GtkWidget **)data;
	gint nlistdir = 1;
	G_CONST_RETURN gchar *hostname;
	G_CONST_RETURN gchar *username;
	gint numhost = -1;
	gint numuser = -1;
	GtkWidget* combodir = NULL;
	gint i;
	gint j;
        GList *glist = NULL;

	if(!entry[1]) return;
	if(!entry[2]) return;
	if(!entry[3]) return;
	if(!entry[5]) return;
	if (!GTK_IS_WIDGET((GtkWidget*)entry[1])) return;
	if (!GTK_IS_WIDGET((GtkWidget*)entry[2])) return;
	if (!GTK_IS_WIDGET((GtkWidget*)entry[3])) return;
	if (!GTK_IS_WIDGET((GtkWidget*)entry[5])) return;
	if ((GTK_OBJECT_FLAGS((GtkObject*)entry[1]) & GTK_IN_DESTRUCTION)) return;
	if ((GTK_OBJECT_FLAGS((GtkObject*)entry[2]) & GTK_IN_DESTRUCTION)) return;
	if ((GTK_OBJECT_FLAGS((GtkObject*)entry[3]) & GTK_IN_DESTRUCTION)) return;
	if ((GTK_OBJECT_FLAGS((GtkObject*)entry[5]) & GTK_IN_DESTRUCTION)) return;

	hostname = gtk_entry_get_text(GTK_ENTRY(entry[2]));

	username = gtk_entry_get_text(GTK_ENTRY(entry[3]));

	combodir = g_object_get_data (G_OBJECT (entry[5]), "Combo");


        if(!combodir) return;

	if(recenthosts.nhosts>0)
  	{
		for(i=0;i<recenthosts.nhosts;i++)
  			if(strcmp(hostname,recenthosts.hosts[i].hostname) == 0)
			{
				numhost = i;
				for(j=0;j<recenthosts.hosts[numhost].nusers;j++)
  					if(strcmp(username,recenthosts.hosts[i].users[j].username) == 0)
					{
						numuser = j;
						break;
					}
				
				break;
			}
		if(numhost<0) return;
		if(numuser<0) return;

        	nlistdir = recenthosts.hosts[numhost].users[numuser].ndirs;
		for(i=nlistdir-1;i>=0;i--)
  			glist = g_list_append(glist,recenthosts.hosts[numhost].users[numuser].dirs[i]);
  	}
  	else
		return;


	if (!(GTK_OBJECT_FLAGS((GtkObject*)combodir) & GTK_IN_DESTRUCTION))
        	gtk_combo_box_entry_set_popdown_strings( combodir, glist) ;


	g_list_free(glist);
}
/********************************************************************************/
void changed_host(GtkWidget *combo,gpointer data)
{
	GtkWidget **entry = (GtkWidget **)data;
	gint nlistuser = 1;
	G_CONST_RETURN gchar *hostname;
	gint numhost = -1;
	GtkWidget* combouser = NULL;
	gint i;
        GList *glist = NULL;

	if(!entry[1]) return;
	if(!entry[2]) return;
	if(!entry[3]) return;
	if(!entry[5]) return;
	if (!GTK_IS_WIDGET((GtkWidget*)entry[1])) return;
	if (!GTK_IS_WIDGET((GtkWidget*)entry[2])) return;
	if (!GTK_IS_WIDGET((GtkWidget*)entry[3])) return;
	if (!GTK_IS_WIDGET((GtkWidget*)entry[5])) return;
	if ((GTK_OBJECT_FLAGS((GtkObject*)entry[1]) & GTK_IN_DESTRUCTION)) return;
	if ((GTK_OBJECT_FLAGS((GtkObject*)entry[2]) & GTK_IN_DESTRUCTION)) return;
	if ((GTK_OBJECT_FLAGS((GtkObject*)entry[3]) & GTK_IN_DESTRUCTION)) return;
	if ((GTK_OBJECT_FLAGS((GtkObject*)entry[5]) & GTK_IN_DESTRUCTION)) return;
	hostname = gtk_entry_get_text(GTK_ENTRY(entry[2]));
	combouser = g_object_get_data (G_OBJECT (entry[3]), "Combo");
        if(!combouser) return;
	if(recenthosts.nhosts>0)
  	{
		for(i=0;i<recenthosts.nhosts;i++)
  			if(strcmp(hostname,recenthosts.hosts[i].hostname) == 0)
			{
				numhost = i;
				break;
			}
		if(numhost<0)
			return;

        	nlistuser = recenthosts.hosts[numhost].nusers;
		for(i=0;i<recenthosts.hosts[numhost].nusers;i++)
		glist = g_list_append(glist,recenthosts.hosts[numhost].users[i].username);
  	}
  	else
		return;


	if (!(GTK_OBJECT_FLAGS((GtkObject*)combouser) & GTK_IN_DESTRUCTION))
  	for (i=0;i<nlistuser;i++)
        	gtk_combo_box_entry_set_popdown_strings( combouser, glist) ;


	g_list_free(glist);
}
/********************************************************************************/
static  GtkWidget* create_network_protocols(GtkWidget* Win,GtkWidget *vbox,gboolean expand)
{
	GtkWidget *frame;
	GtkWidget *ButtonFtpRsh;
	GtkWidget *ButtonSsh;
	GtkWidget *vboxframe;
	GtkWidget *table = gtk_table_new(1,2,FALSE);
	gchar ftprsh[] = "FTP and rsh protocols";
	gchar ssh[]    = "ssh/scp protocols    ";

	frame = gtk_frame_new ("NetWork protocols");
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (vbox), frame, expand, expand, 0);

	vboxframe = gtk_vbox_new (TRUE, 0);
	gtk_widget_show (vboxframe);
	gtk_container_add (GTK_CONTAINER (frame), vboxframe);

	gtk_box_pack_start (GTK_BOX (vboxframe), table, expand, expand, 0);

	ButtonFtpRsh = gtk_radio_button_new_with_label( NULL,ftprsh);
	gtk_table_attach(GTK_TABLE(table),ButtonFtpRsh,0,1,0,1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);
	gtk_widget_show (ButtonFtpRsh);

	ButtonSsh = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonFtpRsh)), ssh); 
	gtk_table_attach(GTK_TABLE(table),ButtonSsh,1,2,0,1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);
	gtk_widget_show (ButtonSsh);


	g_object_set_data(G_OBJECT (Win),"ButtonSsh", ButtonSsh);
	g_object_set_data(G_OBJECT (Win),"ButtonFtpRsh",ButtonFtpRsh);

	g_object_set_data (G_OBJECT (ButtonFtpRsh), "TypeButton", &typeButton[2]);
	g_signal_connect(G_OBJECT(ButtonFtpRsh), "clicked",G_CALLBACK(set_frame_remote_visibility),NULL);

	g_object_set_data (G_OBJECT (ButtonSsh), "TypeButton", &typeButton[3]);
	g_signal_connect(G_OBJECT(ButtonSsh), "clicked",G_CALLBACK(set_frame_remote_visibility),NULL);

  	FrameNetWork = frame;
	gtk_widget_show_all(frame);
	return frame;
}
/********************************************************************************/
GtkWidget *create_remote_frame( GtkWidget *vboxall,GtkWidget **entry)
{
  GtkWidget *frame;
  GtkWidget *combo;
  GtkWidget *vboxframe;
#define NL 4
  gchar      *LabelLeft[NL];
  gushort i;
  GtkWidget *Table;
  gchar      *tlisthost[NHOSTMAX];
  gchar      *tlistuser[NHOSTMAX];
  gchar      *tlistdir[NHOSTMAX];
  gint nlisthost = 1;
  gint nlistuser = 1;
  gint nlistdir  = 1;

  if(recenthosts.nhosts>0)
  {
  	nlisthost = recenthosts.nhosts;
	for(i=0;i<nlisthost;i++)
  		tlisthost[i] = g_strdup(recenthosts.hosts[i].hostname);
        nlistuser = recenthosts.hosts[0].nusers;
	for(i=0;i<recenthosts.hosts[0].nusers;i++)
  		tlistuser[i] = g_strdup(recenthosts.hosts[0].users[i].username);
        nlistdir = recenthosts.hosts[0].users[0].ndirs;
	for(i=0;i<recenthosts.hosts[0].users[0].ndirs;i++)
  		tlistdir[i] = g_strdup(recenthosts.hosts[0].users[0].dirs[i]);
  }
  else
  {
  	tlisthost[0] = g_strdup("hostname");
  	tlistuser[0] = g_strdup("login");
  	tlistdir[0] = g_strdup("tmp");
  }

  LabelLeft[0] = g_strdup(_("Host name"));
  LabelLeft[1] = g_strdup(_("Login"));
  LabelLeft[2] = g_strdup(_("Password"));
  LabelLeft[3] = g_strdup(_("Working Directory"));

  frame = gtk_frame_new (_("Remote host"));
  FrameRemote = frame;
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  Table = gtk_table_new(3,3,FALSE);
  gtk_container_add(GTK_CONTAINER(vboxframe),Table);

	i = 2;
	add_label_table(Table,LabelLeft[i-2],(gushort)(i-2),0);
	add_label_table(Table,":",(gushort)(i-2),1);
	combo = create_combo_box_entry(tlisthost,nlisthost,TRUE,-1,-1);

	gtk_table_attach(GTK_TABLE(Table),combo,2,3,i-2,i+1-2,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);
	gtk_widget_show (combo);
	entry[i] = GTK_BIN(combo)->child;
        g_object_set_data (G_OBJECT (entry[i]), "Combo",combo);
        g_signal_connect(G_OBJECT(GTK_COMBO_BOX(combo)), "changed",G_CALLBACK(changed_host),entry);

	i = 3;
	add_label_table(Table,LabelLeft[i-2],(gushort)(i-2),0);
	add_label_table(Table,":",(gushort)(i-2),1);
	combo = create_combo_box_entry(tlistuser,nlistuser,TRUE,-1,-1);
	add_widget_table(Table,combo,(gushort)(i-2),2);
	entry[i] = GTK_BIN(combo)->child;
        g_object_set_data (G_OBJECT (entry[i]), "Combo",combo);
        g_signal_connect(G_OBJECT(GTK_COMBO_BOX(combo)), "changed",G_CALLBACK(changed_user),entry);

	i = 4;
	{
		LabelPassWord1 = add_label_table(Table,LabelLeft[i-2],(gushort)(i-2),0);
		LabelPassWord2 = add_label_table(Table,":",(gushort)(i-2),1);
		entry[i] = gtk_entry_new ();
		gtk_entry_set_visibility(GTK_ENTRY (entry[i]),FALSE);
		add_widget_table(Table,entry[i],(gushort)(i-2),2);
		EntryPassWord = entry[i];
	}

	i = 5;
	add_label_table(Table,LabelLeft[i-2],(gushort)(i-2),0);
	add_label_table(Table,":",(gushort)(i-2),1);
	combo = create_combo_box_entry(tlistdir,nlistdir,TRUE,-1,-1);
	add_widget_table(Table,combo,(gushort)(i-2),2);
	entry[i] = GTK_BIN(combo)->child;
        g_object_set_data (G_OBJECT (entry[i]), "Combo",combo);

	gtk_widget_show_all(frame);
  
	if(fileopen.remotehost)
		gtk_entry_set_text(GTK_ENTRY(entry[2]),fileopen.remotehost);
	if(fileopen.remoteuser)
		gtk_entry_set_text(GTK_ENTRY(entry[3]),fileopen.remoteuser);
	if(fileopen.remotepass)
		gtk_entry_set_text(GTK_ENTRY(entry[4]),fileopen.remotepass);

	if(fileopen.remotedir && !this_is_a_backspace(fileopen.remotedir))
		gtk_entry_set_text(GTK_ENTRY(entry[5]),fileopen.remotedir);
	else
		gtk_entry_set_text(GTK_ENTRY(entry[5]),"tmp");

  for (i=0;i<NL;i++)
        g_free(LabelLeft[i]);
  for (i=0;i<nlisthost;i++)
        g_free(tlisthost[i]);
  for (i=0;i<nlistuser;i++)
        g_free(tlistuser[i]);
  for (i=0;i<nlistdir;i++)
        g_free(tlistdir[i]);

  return frame;
}
/********************************************************************************/
void create_run_dialogue_box(GtkWidget *w,gchar *type,GCallback func)
{
  GtkWidget *fp;
  GtkWidget *sep;
  GtkWidget *frame;
  GtkWidget *vboxall;
  GtkWidget *vboxwin;
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget **entry;
  gchar *title = g_strdup_printf(_("Run "));
  GtkWidget* ButtonSsh = NULL;
  GtkWidget* ButtonFtpRsh = NULL;

  entry=g_malloc(6*sizeof(GtkWidget *));

  /* Principal Window */
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(fp),title);
  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Fenetre));

  gtk_widget_realize(fp);
  init_child(fp,gtk_widget_destroy," Run ");
  g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)destroy_children,NULL);

  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);
  vboxall = create_vbox(fp);
  vboxwin = vboxall;

  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_add(GTK_CONTAINER(vboxall),frame);
  gtk_widget_show (frame);

  vboxall = create_vbox(frame);

  hbox = create_hbox(vboxall);

  frame = create_programs_frame(hbox);

  frame = create_server_frame(hbox);

  frame = create_local_frame(vboxall);

  frame = create_local_remote_frame(fp,vboxall,entry,type);

  frame = create_network_protocols(fp,vboxall,TRUE);

  frame = create_remote_frame(vboxall,entry);

  EntryFileData = entry[0];


  g_signal_connect(G_OBJECT(EntryFileData),"changed", G_CALLBACK(changedEntryFileData),NULL);
  /* pour appeler changedEntryFileData */
  gtk_entry_set_text(GTK_ENTRY(EntryFileData)," ");
  gtk_entry_set_text(GTK_ENTRY(EntryFileData),fileopen.projectname);

  EntryCommand  = entry[1];
  sep = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(vboxwin), sep, FALSE, FALSE, 2);
  gtk_widget_show(sep);
  /* boutons box */
  hbox = create_hbox(vboxwin);
  gtk_widget_realize(fp);

  button = create_button(fp,"Cancel");
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 5);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)destroy_children,G_OBJECT(fp));
  gtk_widget_show (button);

  button = create_button(fp,"OK");
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 5);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);

  ButtonSsh = g_object_get_data(G_OBJECT (fp),"ButtonSsh");
  ButtonFtpRsh = g_object_get_data(G_OBJECT (fp),"ButtonFtpRsh");
  g_object_set_data(G_OBJECT (button),"ButtonSsh", ButtonSsh);
  g_object_set_data(G_OBJECT (button),"ButtonFtpRsh",ButtonFtpRsh);

  if(fileopen.netWorkProtocol==GABEDIT_NETWORK_SSH)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonSsh), TRUE);
  else
 	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonSsh), FALSE);


  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(func),(gpointer)entry);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)destroy_children,G_OBJECT(fp));
  g_signal_connect_swapped (G_OBJECT (entry[4]), "activate", (GCallback) gtk_button_clicked, G_OBJECT (button));
  
  g_signal_connect(G_OBJECT(ButtonDeMon), "clicked",G_CALLBACK(set_default_entrys),NULL);
  g_signal_connect(G_OBJECT(ButtonGamess), "clicked",G_CALLBACK(set_default_entrys),NULL);
  g_signal_connect(G_OBJECT(ButtonGauss), "clicked",G_CALLBACK(set_default_entrys),NULL);
  g_signal_connect(G_OBJECT(ButtonMolcas), "clicked",G_CALLBACK(set_default_entrys),NULL);
  g_signal_connect(G_OBJECT(ButtonMolpro), "clicked",G_CALLBACK(set_default_entrys),NULL);
  g_signal_connect(G_OBJECT(ButtonMPQC), "clicked",G_CALLBACK(set_default_entrys),NULL);
  g_signal_connect(G_OBJECT(ButtonFireFly), "clicked",G_CALLBACK(set_default_entrys),NULL);
  g_signal_connect(G_OBJECT(ButtonQChem), "clicked",G_CALLBACK(set_default_entrys),NULL);
  g_signal_connect(G_OBJECT(ButtonOrca), "clicked",G_CALLBACK(set_default_entrys),NULL);
  g_signal_connect(G_OBJECT(ButtonNWChem), "clicked",G_CALLBACK(set_default_entrys),NULL);
  g_signal_connect(G_OBJECT(ButtonPsicode), "clicked",G_CALLBACK(set_default_entrys),NULL);
  g_signal_connect(G_OBJECT(ButtonMopac), "clicked",G_CALLBACK(set_default_entrys),NULL);
  g_signal_connect(G_OBJECT(ButtonOther), "clicked",G_CALLBACK(set_default_entrys),NULL);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonDeMon), FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonMolpro), FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonGamess), FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonGauss), FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonMolcas), FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonMPQC), FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonFireFly), FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonOrca), FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonNWChem), FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonPsicode), FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonQChem), FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonMopac), FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonOther), FALSE); 

  if(strstr(type,"FireFly")) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonFireFly), TRUE);
  else if(strstr(type,"DeMon")) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonDeMon), TRUE);
  else if(strstr(type,"Gamess")) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonGamess), TRUE);
  else if(strstr(type,"Molpro")) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonMolpro), TRUE);
  else if(strstr(type,"Gaussian")) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonGauss), TRUE);
  else if(strstr(type,"Molcas")) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonMolcas), TRUE);
  else if(strstr(type,"MPQC")) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonMPQC), TRUE);
  else if(strstr(type,"Orca")) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonOrca), TRUE);
  else if(strstr(type,"NWChem")) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonNWChem), TRUE);
  else if(strstr(type,"Psicode")) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonPsicode), TRUE);
  else if(strstr(type,"Q-Chem")) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonQChem), TRUE);
  else if(strstr(type,"Mopac")) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonMopac), TRUE);
  else gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonOther), TRUE); 


  g_object_set_data (G_OBJECT (ButtonLocal), "TypeButton", &typeButton[0]);
  g_signal_connect(G_OBJECT(ButtonLocal), "clicked",G_CALLBACK(set_frame_remote_visibility),NULL);
#ifdef G_OS_WIN32
  g_signal_connect_swapped(G_OBJECT(ButtonLocal), "clicked",G_CALLBACK(set_default_entrys),ButtonGamess);
#endif

  g_object_set_data (G_OBJECT (ButtonRemote), "TypeButton", &typeButton[1]);
  g_signal_connect(G_OBJECT(ButtonRemote), "clicked",G_CALLBACK(set_frame_remote_visibility),NULL);
  
#ifdef G_OS_WIN32
  if(iprogram == PROG_IS_ORCA || iprogram == PROG_IS_GAMESS || iprogram == PROG_IS_FIREFLY || iprogram == PROG_IS_MOPAC||  iprogram == PROG_IS_GAUSS)
  {
  	if(fileopen.remotedir && !this_is_a_backspace(fileopen.remotedir))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonRemote), TRUE);
	else
	{
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonLocal), TRUE);
		gtk_widget_set_sensitive(FrameRemote, FALSE);
		gtk_widget_set_sensitive(FrameNetWork, FALSE);
	}
  }
  else
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonRemote), TRUE);
  /*gtk_widget_set_sensitive(ButtonLocal, FALSE);*/

#else
  
  if(fileopen.remotedir && !this_is_a_backspace(fileopen.remotedir))
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonRemote), TRUE);
  else
  {
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonLocal), TRUE);
	gtk_widget_set_sensitive(FrameRemote, FALSE);
	gtk_widget_set_sensitive(FrameNetWork, FALSE);
  }
#endif

  /* Show all */
  gtk_widget_show_all(fp);
  if(!strstr(type,"Gamess")  && !strstr(type,"Gaussian") && !strstr(type,"Molcas") && !strstr(type,"Molpro") && !strstr(type,"MPQC"))
  	gtk_widget_hide(LabelDataFile);
}
/********************************************************************************/
void create_run ()
{
	switch(iprogram)
	{
		case PROG_IS_DEMON :
		create_run_dialogue_box(NULL,"DeMon",(GCallback)run_program);
		break;

		case PROG_IS_GAMESS :
		create_run_dialogue_box(NULL,"Gamess",(GCallback)run_program);
		break;

		case PROG_IS_GAUSS :
		create_run_dialogue_box(NULL,"Gaussian",(GCallback)run_program);
		break;

		case PROG_IS_MOLCAS :
		create_run_dialogue_box(NULL,"Molcas",(GCallback)run_program);
		break;

		case PROG_IS_MOLPRO :
		create_run_dialogue_box(NULL,"Molpro",(GCallback)run_program);
		break;
		case PROG_IS_MPQC :
		create_run_dialogue_box(NULL,"MPQC",(GCallback)run_program);
		break;
		case PROG_IS_ORCA :
		create_run_dialogue_box(NULL,"Orca",(GCallback)run_program);
		break;
		case PROG_IS_NWCHEM :
		create_run_dialogue_box(NULL,"NWChem",(GCallback)run_program);
		break;
		case PROG_IS_PSICODE :
		create_run_dialogue_box(NULL,"Psicode",(GCallback)run_program);
		break;
		case PROG_IS_FIREFLY :
		create_run_dialogue_box(NULL,"FireFly",(GCallback)run_program);
		break;
		case PROG_IS_QCHEM :
		create_run_dialogue_box(NULL,"Q-Chem",(GCallback)run_program);
		break;
		case PROG_IS_MOPAC :
		create_run_dialogue_box(NULL,"Mopac",(GCallback)run_program);
		break;
	default :
		create_run_dialogue_box(NULL,"Other",(GCallback)run_program);
	}
}
/********************************************************************************/
