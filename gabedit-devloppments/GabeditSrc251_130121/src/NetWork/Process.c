/* Process.c */
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
#include <unistd.h>

#include "../Common/Global.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Common/Run.h"
#include "../Common/Windows.h"
#include "../NetWork/RemoteCommand.h"
#include "../NetWork/Process.h"
#ifndef G_OS_WIN32
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h> 
#include <fcntl.h>
#endif /* G_OS_WIN32*/

static gint Nlist = 0;
static gint NlistTitle = 0;
static gchar** Titles = NULL;
static gchar*** List = NULL;
static GtkWidget* KillButton1 = NULL;
static GtkWidget* KillButton2 = NULL;
static GtkWidget* KillAllButton = NULL;
static GtkWidget* WinUserProcess = NULL;
static gchar selectedRow[100] = "-1";
static GtkWidget* EntryRemote = NULL;
static GtkWidget* EntryLogin = NULL;
static GtkWidget* EntryPassWord = NULL;
static gboolean Remote = FALSE;
static gchar* RemoteHost = NULL;
static gchar* RemoteUser = NULL;
static gchar* RemotePassWord = NULL;

/********************************************************************************/
static void destroy_win_user_process(GtkWidget* Win, gpointer data)
{
	destroy_children(Win);
	WinUserProcess = NULL;
}
/********************************************************************************/
static gboolean authorized_type(gchar* type)
{
	gchar *Atypes[]={"UID","PID","PPID","CMD"};
	gint i;
  	for(i=0;i<4;i++)
		if(strstr(type,Atypes[i]))
			return TRUE;
	return FALSE;
}
/********************************************************************************/
static gchar* get_title_bordure()
{
  gchar *str = NULL;
  gchar *dump = NULL;
  gchar *temp = NULL;
  gint i = 0;
 
  str = g_strdup_printf(" ");
  for(i=0;i<NlistTitle;i++)
  {
	if(authorized_type(Titles[i]))
	{
		dump = str;
		if(strstr(Titles[i],"CMD"))
		{
			temp = get_line_chars('=',20);
  			str = g_strdup_printf("%s   %s ",dump,temp);
		}
		else
		{
			temp = get_line_chars('=',10);
  			str = g_strdup_printf("%s%10s ",dump,temp);
		}
		g_free(temp);
		g_free(dump);
	}
  }

  return str;
}
/********************************************************************************/
static gchar* get_title_process()
{
  gchar *str = NULL;
  gchar *dump = NULL;
  gchar *temp = NULL;
  gint i = 0;
 
  if(authorized_type(Titles[0]))
  {
  	str = g_strdup_printf("%10s ",Titles[0]);
  }
  else
  	str = g_strdup_printf(" ");
  for(i=1;i<NlistTitle;i++)
  {
	if(authorized_type(Titles[i]))
	{
		dump = str;
		if(strstr(Titles[i],"CMD"))
  			str = g_strdup_printf("%s   %s ",dump,Titles[i]);
		else
  			str = g_strdup_printf("%s%10s ",dump,Titles[i]);
		g_free(dump);
	}
  }
  temp = get_title_bordure();
  dump = str;
  str = g_strdup_printf("%s\n%s\n%s\n",temp,dump,temp);
  g_free(dump);
  g_free(temp);

  return str;
}
/********************************************************************************/
static gchar* get_row_process(gint row)
{
  gchar *str = NULL;
  gchar *dump = NULL;
  gint i = 0;
 
  str = g_strdup_printf(" ");
  for(i=0;i<NlistTitle;i++)
  {
	if(authorized_type(Titles[i]))
	{
		dump = str;
		if(strstr(Titles[i],"CMD"))
  			str = g_strdup_printf("%s   %s ",dump,List[row][i]);
		else
  			str = g_strdup_printf("%s%10s ",dump,List[row][i]);
		g_free(dump);
	}
  }

  return str;
}
/********************************************************************************/
static gchar *get_pid(gint row)
{
	gchar *pid = NULL;
	gint i;

  	for(i=0;i<NlistTitle;i++)
  	{
		if(strcmp(Titles[i],"PID") == 0)
		{
  			pid =List[row][i];
			break;
		}
	}
	return pid;

}
/********************************************************************************/
static gint get_num_child(gint row)
{
	gchar *pid = get_pid(row);
	gint i;
	gint j;

	if(!pid)
		return -1;
  	for(i=0;i<NlistTitle;i++)
	{
		if(strcmp(Titles[i],"PPID") == 0)
		{
 			for(j=0;j<Nlist;j++)
 			{
				if(atoi(pid) ==atoi(List[j][i]))
				{
				  return j;
				}
 			}
		}
	}
	return -1;

}
/********************************************************************************/
static gint* get_all_children(gint row,gint* Numb)
{
	gint *NumChildren = NULL;
	gint num = -1;

	*Numb = 0;


	num = get_num_child(row);
	if(num>-1)
	{
		NumChildren = g_malloc(sizeof(gint));
		NumChildren[0] = num;
		(*Numb)++;
	}
	else
		return NULL;

	while(num>-1)
	{
		num = get_num_child(num);
		if(num>-1)
		{
			NumChildren = g_realloc(NumChildren,(*Numb+1)*sizeof(gint));
			NumChildren[*Numb] = num;
			(*Numb)++;
		}
	}
	return NumChildren;
	

}
/********************************************************************************/
static gchar *get_string_all_children(gint row)
{
	gint Numb = 0;
	gint *NumChildren = NULL;
	gint j;
  	gchar* rowprocess = NULL;
  	gchar* str = NULL;
  	gchar* dump = NULL;

	NumChildren = get_all_children(row,&Numb);
	if(Numb>0)
  		str = get_title_process();

 	for(j=0;j<Numb;j++)
 	{
  		rowprocess = get_row_process(NumChildren[j]);
		dump = str;
		if(j==0)
  			str = g_strdup_printf("%s%s",dump,rowprocess);
		else
  			str = g_strdup_printf("%s\n%s",dump,rowprocess);
		g_free(dump);
		if(rowprocess)
		{
			g_free(rowprocess);
			rowprocess = NULL;
		}
 	}
	if(NumChildren)
		g_free(NumChildren);
	return str;

}
/********************************************************************************/
static void run_process_remote_all(GtkWidget*win,gpointer data)
{  
	run_process_all(TRUE);
}
/********************************************************************************/
static void run_process_remote_user(GtkWidget*win,gpointer data)
{  
	if(RemoteUser)
		g_free(RemoteUser);
        RemoteUser = NULL;
	if(RemoteHost)
		g_free(RemoteHost);
	RemoteHost = NULL;
	if(RemotePassWord)
		g_free(RemotePassWord);
	RemotePassWord = NULL;
	
	run_process_user(TRUE,NULL,NULL,NULL);
}
/********************************************************************************/
static void changed_host(GtkWidget *combo,gpointer data)
{
	GtkWidget **entry = (GtkWidget **)data;
	gint nlistuser = 1;
	G_CONST_RETURN gchar *hostname;
	gint numhost = -1;
	GtkWidget* combouser = NULL;
	gint i;
        GList *glist = NULL;

	hostname = gtk_entry_get_text(GTK_ENTRY(entry[0]));
        if(!this_is_an_object((GtkObject*)entry[1]))
		return;
	combouser = g_object_get_data (G_OBJECT (entry[1]), "Combo");
        if(!combouser)
		return;
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


  	for (i=0;i<nlistuser;i++)
        	gtk_combo_box_entry_set_popdown_strings( combouser, glist) ;


	g_list_free(glist);
}
/********************************************************************************/
static GtkWidget *create_process_remote_frame( GtkWidget *vboxall,GtkWidget **entry,gboolean all)
{
  GtkWidget *frame;
  GtkWidget *combo;
  GtkWidget *vboxframe;
  gushort i;
  GtkWidget *Table;
  gchar      *tlisthost[NHOSTMAX];
  gchar      *tlistuser[NHOSTMAX];
  gint nlisthost = 1;
  gint nlistuser = 1;
  G_CONST_RETURN gchar *localuser;

  if(recenthosts.nhosts>0)
  {
  	nlisthost = recenthosts.nhosts;
	for(i=0;i<nlisthost;i++)
  		tlisthost[i] = g_strdup(recenthosts.hosts[i].hostname);
        nlistuser = recenthosts.hosts[0].nusers;
	for(i=0;i<recenthosts.hosts[0].nusers;i++)
  		tlistuser[i] = g_strdup(recenthosts.hosts[0].users[i].username);
  }
  else
  {
   	localuser = get_local_user();
  	tlisthost[0] = g_strdup("hostname");
  	if(localuser)
  		tlistuser[0] = g_strdup(localuser);
  	else
  		tlistuser[0] = g_strdup("login");
  }

  frame = gtk_frame_new (_("Remote host"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  Table = gtk_table_new(2,3,FALSE);
  gtk_container_add(GTK_CONTAINER(vboxframe),Table);

	i = 0;
	add_label_table(Table,_("Host name "),(gushort)(i),0);
	add_label_table(Table," : ",(gushort)(i),1);
	combo = create_combo_box_entry(tlisthost,nlisthost,TRUE,-1,-1);
	gtk_table_attach(GTK_TABLE(Table),combo,2,3,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);
	gtk_widget_show (combo);
	entry[0] = GTK_BIN(combo)->child;
        g_object_set_data (G_OBJECT (entry[0]), "Combo",combo);
        g_signal_connect(G_OBJECT(GTK_COMBO_BOX(combo)), "changed",G_CALLBACK(changed_host),entry);

	i = 1;
	add_label_table(Table,_("Login "),(gushort)(i),0);
	add_label_table(Table," : ",(gushort)(i),1);
	combo = create_combo_box_entry(tlistuser,nlistuser,TRUE,-1,-1);
	gtk_table_attach(GTK_TABLE(Table),combo,2,3,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);
	entry[1] = GTK_BIN(combo)->child;
    g_object_set_data (G_OBJECT (entry[1]), "Combo",combo);

	i = 2;
	if(fileopen.netWorkProtocol==GABEDIT_NETWORK_SSH)
	{
#ifdef G_OS_WIN32
		add_label_table(Table,_("Password "),(gushort)(i),0);
		add_label_table(Table," : ",(gushort)(i),1);
		entry[i] = gtk_entry_new ();
		gtk_entry_set_visibility(GTK_ENTRY (entry[i]),FALSE);
		gtk_table_attach(GTK_TABLE(Table),entry[i],2,3,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);
#else
		entry[i] = gtk_entry_new ();
#endif
	}
	else
	{
		entry[i] = gtk_entry_new ();
	}


	if(fileopen.remotehost)
		gtk_entry_set_text(GTK_ENTRY(entry[0]),fileopen.remotehost);
	if(fileopen.remoteuser)
		gtk_entry_set_text(GTK_ENTRY(entry[1]),fileopen.remoteuser);
	if(fileopen.remotepass)
		gtk_entry_set_text(GTK_ENTRY(entry[2]),fileopen.remotepass);


  for (i=0;i<nlisthost;i++)
        g_free(tlisthost[i]);
  for (i=0;i<nlistuser;i++)
        g_free(tlistuser[i]);

  return frame;
}
/********************************************************************************/
void create_process_remote(gboolean all)
{
  GtkWidget *fp;
  GtkWidget *frame;
  GtkWidget *vboxall;
  GtkWidget *vboxwin;
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget **entry;
  gchar *title = g_strdup_printf(_("Process in remote host "));

  entry=g_malloc(3*sizeof(GtkWidget *));

  /* Principal Window */
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(fp),title);
  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Fenetre));

  gtk_widget_realize(fp);
  init_child(fp,gtk_widget_destroy,_(" Remote Process "));
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

  frame = create_process_remote_frame(hbox,entry,all);

  EntryRemote = entry[0];
  EntryLogin  = entry[1];
  EntryPassWord = entry[2];
  /* boutons box */
  hbox = create_hbox(vboxwin);
  gtk_widget_realize(fp);

  button = create_button(fp,_("Cancel"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)destroy_children,GTK_OBJECT(fp));
  gtk_widget_show (button);

  button = create_button(fp,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);
  g_signal_connect_swapped(G_OBJECT (EntryPassWord ), "activate", (GCallback) gtk_button_clicked, GTK_OBJECT (button));
  if(!all) g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(run_process_remote_user),(gpointer)NULL);
  else g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(run_process_remote_all),(gpointer)NULL);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)destroy_children,GTK_OBJECT(fp));
  

  /* Show all */
  gtk_widget_show_all(fp);
}
/********************************************************************************/
static GtkWidget* create_children_frame(GtkWidget *box,gint row)
{
  GtkWidget *frame;
  GtkWidget *vboxframe;
  GtkWidget *Label;
  gchar *str = NULL;
 
  str = get_string_all_children(row);

  frame = gtk_frame_new (_("Children Process"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start (GTK_BOX( box), frame, TRUE, TRUE, 3);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  Label = gtk_label_new(str);
  set_font (Label,FontsStyleResult.fontname);
  gtk_label_set_justify(GTK_LABEL(Label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(vboxframe), Label, FALSE, FALSE, 3);
  g_free(str);
  
  return frame;
}
/********************************************************************************/
static GtkWidget* create_label_frame(GtkWidget *box,gint row)
{
  GtkWidget *frame;
  GtkWidget *vboxframe;
  GtkWidget *Label;
  gchar *title = NULL;
  gchar *rowprocess = NULL;
  gchar *str = NULL;
 
  title = get_title_process();
  rowprocess = get_row_process(row);
  str = g_strdup_printf("%s%s",title,rowprocess);
  g_free(title);
  g_free(rowprocess);

  frame = gtk_frame_new (_("Process to kill"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start (GTK_BOX( box), frame, TRUE, TRUE, 3);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  Label = gtk_label_new(str);
  set_font (Label,FontsStyleResult.fontname);
  gtk_label_set_justify(GTK_LABEL(Label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(vboxframe), Label, FALSE, FALSE, 3);
  g_free(str);
  
  return frame;
}
/********************************************************************************/
static void kill_process(GtkWidget *Win,gpointer data)
{
  	gchar *command = NULL;
  	gchar *scom = NULL;
  	gchar *t = NULL;
	gint Numb = 0;
	gint *NumChildren = NULL;
	gint i;
	gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);


  	if (GTK_TOGGLE_BUTTON (KillButton1)->active) 
		scom = g_strdup("kill");
  	else
		scom = g_strdup("kill -KILL");

/* killing of children beforee */
	if(GTK_TOGGLE_BUTTON (KillAllButton)->active)
	{
  		NumChildren = get_all_children(atoi(selectedRow),&Numb);
  		if(Numb>0)
		{
			for(i=Numb-1;i>=0;i--)
			{
  				command = g_strdup_printf("%s %s",scom,get_pid(NumChildren[i]));
				if(Remote)
				{
  					/*rsh (fout,ferr,command, RemoteUser,RemoteHost);*/
					remote_command (fout,ferr,command,RemoteHost,RemoteUser,RemotePassWord);
  					t = cat_file(ferr,FALSE);
				}
				else
				{
  					t = run_command(command);
				}
  				if(t)
  				{
 					Message(t,_("Info"),TRUE);
					g_free(t);
				}
				g_free(command);
			}
  			if(NumChildren)
  				g_free(NumChildren);
		}
	}
/* killing of parent */
  	command = g_strdup_printf("%s %s",scom,get_pid(atoi(selectedRow)));
	if(Remote)
	{
  	/*	rsh (fout,ferr,command, RemoteUser,RemoteHost);*/
		remote_command (fout,ferr,command,RemoteHost,RemoteUser,RemotePassWord);
  		t = cat_file(ferr,FALSE);
	}
	else
	{
  		t = run_command(command);
	}
	if(WinUserProcess)
	{
  		destroy_children(WinUserProcess);
		WinUserProcess = NULL;
		run_process_user(Remote,RemoteUser,RemoteHost,RemotePassWord);
	}
  	if(t)
  	{
		if(!GTK_TOGGLE_BUTTON (KillAllButton)->active)
			Message(t,_("Error"),TRUE);
		g_free(t);
  	}

  	g_free(scom);
  	g_free(command);
  	g_free(ferr);
  	g_free(fout);
  
}
/********************************************************************************/
static GtkWidget* create_options_frame(GtkWidget *hbox)
{
  GtkWidget *frame;
  GtkWidget *vboxframe;
  GtkWidget *Table;
  GtkWidget *button1;
  GtkWidget *button2;
  GtkWidget *button3;
 

  frame = gtk_frame_new (_("Options"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start (GTK_BOX( hbox), frame, TRUE, TRUE, 3);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  Table = gtk_table_new(2,2,FALSE);
  gtk_container_add(GTK_CONTAINER(vboxframe),Table);

  button1 = gtk_radio_button_new_with_label( NULL,"kill " );
  add_widget_table(Table,button1,0,0);

  button2 = gtk_radio_button_new_with_label(
                       gtk_radio_button_get_group (GTK_RADIO_BUTTON (button1)),
                       "kill -KILL "); 
  add_widget_table(Table,button2,0,1);
  button3 = gtk_check_button_new_with_label(_("Kill all children process "));
  add_widget_table(Table,button3,1,0);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button2), TRUE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button3), TRUE);

  KillButton1 = button1;
  KillButton2 = button2;
  KillAllButton = button3;

  return frame;
}
/********************************************************************************/
static void create_kill_process(GtkWidget*Win,gint row)
{
  GtkWidget *fp;
  GtkWidget *frame;
  GtkWidget *vboxall;
  GtkWidget *vboxwin;
  GtkWidget *hbox;
  GtkWidget *button;
  gchar *title = g_strdup_printf(_("Kill a process"));

  /* Principal Window */
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(fp),title);
  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Fenetre));

  gtk_widget_realize(fp);
  init_child(fp,gtk_widget_destroy,_(" Kill "));
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
  frame = create_label_frame(hbox,row);

  hbox = create_hbox(vboxall);
  frame = create_children_frame(hbox,row);

  hbox = create_hbox(vboxall);
  frame = create_options_frame(hbox);

  /* boutons box */
  hbox = create_hbox(vboxwin);
  gtk_widget_realize(fp);

  button = create_button(fp,_("Cancel"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)destroy_children,GTK_OBJECT(fp));
  gtk_widget_show (button);

  button = create_button(fp,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)kill_process,GTK_OBJECT(Win));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)destroy_children,GTK_OBJECT(fp));
  

  gtk_widget_show_all(fp);
}
/*************************************************************************************************/
static void eventDispatcher(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{

	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (!event) return;
	if (event->window == gtk_tree_view_get_bin_window (GTK_TREE_VIEW (widget))
	    && !gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget), event->x, event->y, NULL, NULL, NULL, NULL)) {
		gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)));
	}
	if(gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget), event->x, event->y, &path, NULL, NULL, NULL))
	{
		if(path)
		{
			model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
			gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)), path);
			sprintf(selectedRow ,"%s",gtk_tree_path_to_string(path));
			gtk_tree_model_get_iter (model, &iter, path);
			gtk_tree_path_free(path);
  			if (event->type == GDK_2BUTTON_PRESS && ((GdkEventButton *) event)->button == 1)
				create_kill_process(widget,atoi(selectedRow));
		}
		else sprintf(selectedRow,"-1");
	}
	GTK_WIDGET_GET_CLASS(widget)->button_press_event(widget, event);
}
/********************************************************************************/
static void init_list()
{
	Nlist = 0;
	NlistTitle = 0;
	Titles = NULL;
	List = NULL;
}
/********************************************************************************/
static void free_list()
{
	gint i;
	gint j;
 	if(Nlist == 0 || NlistTitle == 0)
 	{
		init_list();
		return;
 	}
 	if(Titles)
	{
		for(i=0;i<NlistTitle;i++)
			if(Titles[i]) g_free(Titles[i]);
		g_free(Titles);
	}

	if(List)
	{
		for(i=0;i<Nlist;i++)
		{
			for(j=0;j<NlistTitle;j++)
					if(List[i][j]) g_free(List[i][j]);
			if(List[i]) g_free(List[i]);
		}
		g_free(List);
	}

	init_list();
 
}
/********************************************************************************/
static GtkWidget* create_gtk_list_process()
{
	gint i;
	gint j;
	GtkWidget* gtklist = NULL;
	gint *Width = NULL;
	GtkListStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *select;
	GtkTreeIter iter;
	GType* types = NULL;

	if(NlistTitle<1) return gtklist;
	Width = g_malloc(NlistTitle*sizeof(gint));
	for (j=0;j<NlistTitle;j++)
	{
  		Width[j] = strlen(Titles[j]);
  		for(i=0;i<Nlist;i++) if(Width[j]<(gint)strlen(List[i][j]) ) Width[j] = strlen(List[i][j]);
	}

	types = g_malloc(NlistTitle*sizeof(GType));
	for (i=0;i<NlistTitle;i++) types[i] = G_TYPE_STRING;
  	store = gtk_list_store_newv (NlistTitle, types);
	g_free(types);
	model = GTK_TREE_MODEL (store);

	gtklist = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (gtklist), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (gtklist), TRUE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW (gtklist), TRUE);
	for (j=0;j<NlistTitle;j++) Width[j] = (gint)(Width[j]*8);

	for (i=0;i<NlistTitle;i++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, Titles[i]);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, TRUE);
		gtk_tree_view_column_set_min_width(column, Width[i]);
		gtk_tree_view_column_set_attributes (column, renderer, "text", i, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (gtklist), column);
	}
  	g_free( Width);
  
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (gtklist));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	for(i=0;i<Nlist;i++)
	{
		gtk_list_store_append(store, &iter);
		for(j=0;j<NlistTitle;j++) gtk_list_store_set (store, &iter, j, List[i][j], -1);
	}
	g_signal_connect(gtklist, "button_press_event", G_CALLBACK(eventDispatcher), NULL);

	return gtklist;
}
/********************************************************************************/
static void reduce_list()
{
	gint i;
	gint j;
	gint k;
	gchar** titles = NULL;
	gchar*** listOfRows = NULL;
	static gint max = 4;
	gint n[] = {-1,-1,-1,-1}; /* PID  PPID TIME CMD */

	if(Nlist<1) return;
	if(NlistTitle<max) return;

	for(j=0;j<NlistTitle;j++)
	{
		if(strstr(Titles[j],"PID") && !strstr(Titles[j],"PPID")) { n[0] = j; continue;}
		if(strstr(Titles[j],"PPID")) { n[1] = j; continue;}
		if(strstr(Titles[j],"TIME") && !strstr(Titles[j],"STIME")) { n[2] = j; continue;}
		if(strstr(Titles[j],"CMD")) { n[3] = j; continue;}
	}
	for(i=0;i<max;i++)
		if(n[i]<0) return;

	titles = g_malloc(max*sizeof(gchar*));

	for(i=0; i<max; i++)
		titles[i] = g_strdup(Titles[n[i]]);

	if(Titles)
	{
		for(j=0;j<NlistTitle;j++) if(Titles[j]) g_free(Titles[j]);
		g_free(Titles);
		Titles = NULL;
	}

	
	listOfRows = g_malloc(Nlist*sizeof(gchar**));
	for(k=0;k<Nlist;k++)
		listOfRows[k] = g_malloc(max*sizeof(gchar*));
	
	for(k=0;k<Nlist;k++)
	{
		for(i=0; i<max; i++)
			listOfRows[k][i] = g_strdup(List[k][n[i]]);
	}
	
	if(List)
	{
		for(k=0;k<Nlist;k++)
		{
			for(j=0;j<NlistTitle;j++) if(List[k][j]) g_free(List[k][j]);
			if(List[k]) g_free(List[k]);
		}
		g_free(List);
	}
	
	NlistTitle = max;
	Titles = titles;
	List = listOfRows;
	
}
/********************************************************************************/
static void get_list_from_file(gchar* namefile)
{
 FILE *fd;
 gchar *t = NULL;
#ifndef G_OS_WIN32
 gchar *dump = NULL;
#endif
 gint taille = BSIZE;
 gint i;
 gchar tmp[BSIZE];
 gint nListAll = 0;

#define NMAXTITLES 20

 free_list();
 t=g_malloc(taille);

 fd = FOpen(namefile, "r");
 if(fd)
 {
	Titles = g_malloc(NMAXTITLES*sizeof(gchar*));
	for(i=0;i<NMAXTITLES;i++)
	{
		Titles[i] = g_malloc(100*sizeof(gchar));
		sprintf(Titles[i]," ");
	}
	NlistTitle = NMAXTITLES;

	while(!feof(fd))
	{
    		if(fgets(t,taille, fd))
		{
			sscanf(t,"%s",tmp);
			if(!strstr(t,"UID")) continue;
			/* if(strcmp(tmp,"UID")!=0) continue; */
				
			NlistTitle = sscanf(t,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
			Titles[0],Titles[1],Titles[2],Titles[3],Titles[4],Titles[5],Titles[6],
			Titles[7],Titles[8],Titles[9],Titles[10],Titles[11],Titles[12],Titles[13],
			Titles[14],Titles[15],Titles[16],Titles[17],Titles[18],Titles[19]
			);
			break;
		}
	}
	if(NlistTitle==0)	
	{
		free_list();
		return;
	}
  	while(!feof(fd))
  	{
    		if(!fgets(t,taille, fd)) break;
		if(List == NULL)
		{
			List = g_malloc(sizeof(gchar**));
			List[0] = g_malloc(NMAXTITLES*sizeof(gchar*));
			for(i=0;i<NMAXTITLES;i++)
				List[0][i] = g_malloc(BSIZE*sizeof(gchar));
		}
		else
		{
			List = g_realloc(List,(Nlist+1)*sizeof(gchar**));
			List[Nlist] = g_malloc(NMAXTITLES*sizeof(gchar*));
			for(i=0;i<NMAXTITLES;i++)
				List[Nlist][i] = g_malloc(BSIZE*sizeof(gchar));
		}
		nListAll = sscanf(t,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
			List[Nlist][0],List[Nlist][1],List[Nlist][2],List[Nlist][3],
			List[Nlist][4],List[Nlist][5],List[Nlist][6],List[Nlist][7],
			List[Nlist][8],List[Nlist][9],List[Nlist][10],List[Nlist][11],
			List[Nlist][12],List[Nlist][13],List[Nlist][14],List[Nlist][15],
			List[Nlist][16],List[Nlist][17],List[Nlist][18],List[Nlist][19]
			);
		if(nListAll>NlistTitle)
		{
			gchar buffer[BSIZE];
			gint i;

			for(i=1; i<=nListAll-NlistTitle;i++)
			{
				if(strlen(List[Nlist][NlistTitle-1+i])<1) continue;
				sprintf(buffer,"%s",List[Nlist][NlistTitle-1]);
				sprintf(List[Nlist][NlistTitle-1],"%s %s",buffer,List[Nlist][NlistTitle-1+i]);
			}
		}
		Nlist++;
  	}
 	fclose(fd);
#ifdef G_OS_WIN32
	unlink (namefile);
#else
 	dump = g_strdup_printf("rm %s",namefile);
	{int ierre = system(dump);}
	g_free(dump);

#endif
 }
 g_free(t);
}
/********************************************************************************/
static GtkWidget* create_list_result_command(GtkWidget* gtklist,gchar* strerr,gchar* title)
{
  GtkWidget *Win;
  GtkWidget *frame;
  GtkWidget *scr;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *vboxall;
  GtkWidget *vboxwin;
  GtkWidget *button;
  GtkWidget* Text;
  GtkWidget* Frame[2];


  Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(Win),title);
  gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);

  gtk_widget_realize(Win);

  init_child(Win,gtk_widget_destroy,_(" List of process "));

  
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

  Frame[0] = NULL;
  Frame[1] = NULL;

  if(gtklist)
  {
  	frame = gtk_frame_new (_("Output"));
  	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  	gtk_container_add(GTK_CONTAINER(vboxall),frame);
  	gtk_widget_show (frame);
  	vbox = create_vbox(frame);
  	scr=gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 

  	gtk_box_pack_start(GTK_BOX (vbox), scr,TRUE, TRUE, 2);
  	gtk_container_add(GTK_CONTAINER(scr),gtklist);
        set_base_style(gtklist,50000,50000,50000);
  
	Frame[0] = frame;
  }

  Text = NULL;
  if(strerr)
  {
  Text = create_text_widget(vboxall,_("Error"),&Frame[1]);
  set_font (Text,FontsStyleResult.fontname);
  set_base_style(Text,FontsStyleResult.BaseColor.red ,FontsStyleResult.BaseColor.green ,FontsStyleResult.BaseColor.blue);
  set_text_style(Text,FontsStyleResult.TextColor.red ,0,0);
  }


  /* boutons box */
  hbox = gtk_hbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX(vboxwin), hbox, FALSE, FALSE, 5);
  gtk_box_set_homogeneous(GTK_BOX(hbox), FALSE);
  gtk_widget_realize(Win);
  button = create_button(Win,_("OK"));
  gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, FALSE, 5);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)destroy_win_user_process,GTK_OBJECT(Win));
  gtk_widget_show (button);
  gtk_window_set_default_size (GTK_WINDOW(Win), 4*ScreenWidth/5, 4*ScreenHeight/5);
  if(Frame[0])
  {
  	gtk_widget_set_size_request(GTK_WIDGET(Frame[0]),-1,3*ScreenHeight/5);
	if(Frame[1])
  		gtk_widget_set_size_request(GTK_WIDGET(Frame[1]),-1,1*ScreenHeight/10);
  }
  
  if(Text && strerr)
	gabedit_text_insert (GABEDIT_TEXT(Text), NULL, NULL, NULL,strerr,-1);   
  return Win;
}
/********************************************************************************/
void run_process_all(gboolean remote)
{  
	gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);


  gchar *command;
  GtkWidget* Text[2];
  GtkWidget* Frame[2];
  GtkWidget* Win;
  gchar *strout;
  gchar *Uidstrout;
  gchar *strerr;
  G_CONST_RETURN gchar *remoteuser = NULL;
  G_CONST_RETURN gchar *remotehost = NULL;
  G_CONST_RETURN gchar *remotepassword = NULL;
  gchar *title = NULL;

	Remote = remote;

#ifdef G_OS_WIN32
	if(!remote)
	{
		Message(_("Sorry, This option is available for unix system only"),_("Error"),TRUE);
		g_free(fout);
		g_free(ferr);
		return;
	}
#endif
	if(remote)
	{
  		remotehost = gtk_entry_get_text(GTK_ENTRY(EntryRemote));
  		remoteuser = gtk_entry_get_text(GTK_ENTRY(EntryLogin));
		remotepassword = gtk_entry_get_text(GTK_ENTRY(EntryPassWord));
  		command = g_strdup_printf("ps -lef");
  		title = g_strdup_printf(_("All process in %s host "),remotehost);
  		/*rsh (fout,ferr,command, remoteuser,remotehost);*/
		remote_command (fout,ferr,command,remotehost,remoteuser,remotepassword);
		g_free(command);
  		add_host(remotehost,remoteuser,"","tmp");
	}
	else
	{
		command = g_strdup("ps -lef");
  		title = g_strdup_printf(_("All process in local host "));
  		run_local_command(fout,ferr,command,FALSE);
  		g_free(command);
	}
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
  		Uidstrout = strout;
		while(Uidstrout[0] != '\0' && Uidstrout[0] != 'U' && Uidstrout[0] != 'I' && Uidstrout[0] != 'D')
			Uidstrout++;
 		gabedit_text_insert (GABEDIT_TEXT(Text[0]), NULL, NULL, NULL,Uidstrout,-1);   
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
void run_process_user(gboolean remote,gchar *remoteuser,gchar *remotehost,gchar *remotepassword)
{  
	gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);


  gchar *command;
  GtkWidget* gtklist;
  GtkWidget* Win;
  gchar *strerr;
  G_CONST_RETURN gchar *localuser = NULL;
  gchar localhost[100];
  gchar *title = NULL;

	Remote = remote;

#ifdef G_OS_WIN32
	if(!remote)
	{
		Message(_("Sorry, This option is available for unix system only"),_("Error"),TRUE);
		g_free(fout);
		g_free(ferr);
		return;
	}
#else
	localuser = get_local_user();
  	gethostname(localhost,100);
#endif /* G_OS_WIN32 */
  	if(!localuser && !remote)
  	{
		Message(_("Sorry, I can not obtain user name"),_("Error"),TRUE);
		g_free(fout);
		g_free(ferr);
		return;
  	}

	if(remote)
	{
		if(!remotehost)
		{
  			G_CONST_RETURN gchar* remotehost0 = gtk_entry_get_text(GTK_ENTRY(EntryRemote));
			if(RemoteHost) g_free(RemoteHost);
			RemoteHost = g_strdup(remotehost0);
			remotehost = g_strdup(remotehost0);
	 	}	
  		if(!remoteuser)
		{
			G_CONST_RETURN gchar* remoteuser0 = gtk_entry_get_text(GTK_ENTRY(EntryLogin));
			if(RemoteUser) g_free(RemoteUser);
			RemoteUser = g_strdup(remoteuser0);
			remoteuser = g_strdup(remoteuser0);
		}
  		if(!remotepassword)
		{
			G_CONST_RETURN gchar* remotepassword0 = gtk_entry_get_text(GTK_ENTRY(EntryPassWord));
			if(RemotePassWord) g_free(RemotePassWord);
			RemotePassWord = g_strdup(remotepassword0);
			remotepassword = g_strdup(remotepassword0);
		}

		command = g_strdup_printf("ps -fu %s",remoteuser);
		remote_command (fout,ferr,command,remotehost,remoteuser,remotepassword);
		g_free(command);
  		add_host(remotehost,remoteuser,"","tmp");
  		title = g_strdup_printf(_("Process in host : \"%s\" ;  for user : \"%s\" "),remotehost,remoteuser);
	}
	else
	{
  		command = g_strdup_printf("ps -lfu %s",localuser);
  		run_local_command(fout,ferr,command,FALSE);
  		g_free(command);
  		title = g_strdup_printf(_("Process in host : \"%s\" ;  for user : \"%s\" "),localhost,localuser);
	}
	if(WinUserProcess)
		destroy_children(WinUserProcess);

  	get_list_from_file(fout);

	reduce_list();

  	gtklist = create_gtk_list_process();
  	strerr = cat_file(ferr,FALSE);

  	Win =  create_list_result_command(gtklist,strerr,title);

	g_free(title);
  	WinUserProcess = Win;
  	gtk_widget_show_all(Win);

  	g_free(fout);
  	g_free(ferr);

}
/********************************************************************************/
