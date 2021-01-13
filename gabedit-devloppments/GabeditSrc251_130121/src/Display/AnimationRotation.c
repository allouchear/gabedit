/* AnimationRotation.c */
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
#include "GlobalOrb.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "../Utils/UtilsInterface.h"
#include "../Display/StatusOrb.h"
#include "../Display/GLArea.h"
#include "../Display/AnimationRotation.h"
#include "../Files/FolderChooser.h"
#include "../Files/GabeditFolderChooser.h"
#include "../Common/Help.h"
#include "../Display/Images.h"
#include "../Display/PovrayGL.h"

static	GtkWidget *WinDlg = NULL;
static	GtkWidget *EntryVelocity = NULL;
static	GtkWidget *EntryNRotation = NULL;
static	GtkWidget *LabelNRotation = NULL;
static	GtkWidget *PlayButton = NULL;
static	GtkWidget *StopButton = NULL;
static gboolean play = FALSE;
static gdouble velocity = 0.1;
static gint nRotation = 10;
static	GtkWidget *buttonXAxis = NULL;
static	GtkWidget *buttonYAxis = NULL;
static	GtkWidget *buttonZAxis = NULL;

static GtkWidget *buttonCheckFilm = NULL;
static GtkWidget *buttonDirFilm = NULL;
static GtkWidget* comboListFilm = NULL;
static gboolean createFilm = FALSE;
static gint numFileFilm = 0;
static gchar formatFilm[100] = "BMP";

typedef enum
{
 XAXIS = 0,
 YAXIS = 1,
 ZAXIS = 2
} GabEditAxis;
GabEditAxis selectedAxis = YAXIS;

/********************************************************************************/
static void animate_rotation();
static void stop_animation(GtkWidget *win, gpointer data);
static void play_animation(GtkWidget *win, gpointer data);
/********************************************************************************/
static void reset_last_directory(GtkWidget *dirSelector, gpointer data)
{
	gchar* dirname = gabedit_folder_chooser_get_current_folder(GABEDIT_FOLDER_CHOOSER(dirSelector));
	gchar* filename = NULL;

	if(dirname && strlen(dirname)>0)
	{
		if(dirname[strlen(dirname)-1] != G_DIR_SEPARATOR)
			filename = g_strdup_printf("%s%sdump.txt",dirname,G_DIR_SEPARATOR_S);
		else
			filename = g_strdup_printf("%sdump.txt",dirname);
	}
	else
	{
		dirname = g_strdup(g_get_home_dir());
		filename = g_strdup_printf("%s%sdump.txt",dirname,G_DIR_SEPARATOR_S);
	}
	if(dirname) g_free(dirname);
	if(filename)
	{
		set_last_directory(filename);
		g_free(filename);
	}
}
/********************************************************************************/
static void set_directory(GtkWidget *win, gpointer data)
{
	GtkWidget *dirSelector;
	dirSelector = selectionOfDir(reset_last_directory, _("Set folder"), GABEDIT_TYPEWIN_ORB);
	gtk_window_set_modal (GTK_WINDOW (dirSelector), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(dirSelector),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_transient_for(GTK_WINDOW(dirSelector),GTK_WINDOW(WinDlg));
}
/********************************************************************************/
static void reset_parameters(GtkWidget *win, gpointer data)
{
	gdouble velo     = atof(gtk_entry_get_text(GTK_ENTRY(EntryVelocity)));
	gint nRot     = atoi(gtk_entry_get_text(GTK_ENTRY(EntryNRotation)));
	gchar t[BSIZE];

	if(velo<0)
	{
		velo = -velo;
		sprintf(t,"%lf",velo);
		gtk_entry_set_text(GTK_ENTRY(EntryVelocity),t);
	}
	velocity = velo;

	if(nRot==0)
	{
		nRot = 10;
		sprintf(t,"%d",nRot);
		gtk_entry_set_text(GTK_ENTRY(EntryNRotation),t);
	}
	if(nRot<0)
	{
		nRot = -nRot;
		sprintf(t,"%d",nRot);
		gtk_entry_set_text(GTK_ENTRY(EntryNRotation),t);
	}
	nRotation = nRot;
	if(!play)
		stop_animation(NULL,NULL);

}
/********************************************************************************/
static void play_animation(GtkWidget *win, gpointer data)
{
	play = TRUE;
	gtk_widget_set_sensitive(PlayButton, FALSE); 
	gtk_widget_set_sensitive(StopButton, TRUE); 
	gtk_window_set_modal (GTK_WINDOW (WinDlg), TRUE);
	animate_rotation();

}
/********************************************************************************/
static void stop_animation(GtkWidget *win, gpointer data)
{
	play = FALSE;
	if(GTK_IS_WIDGET(PlayButton)) gtk_widget_set_sensitive(PlayButton, TRUE); 

	if(GTK_IS_WIDGET(StopButton)) gtk_widget_set_sensitive(StopButton, FALSE); 

	if(GTK_IS_WIDGET(WinDlg)) gtk_window_set_modal (GTK_WINDOW (WinDlg), FALSE);

	while( gtk_events_pending() )
		gtk_main_iteration();

	if(this_is_an_object((GtkObject*)GLArea))
		glarea_rafresh(GLArea);
}
/********************************************************************************/
static void destroyAnimationDlg(GtkWidget *win)
{
	createFilm = FALSE;
	numFileFilm = 0;
	stop_animation(NULL, NULL);
	gtk_widget_destroy(WinDlg);
	WinDlg = NULL;
}
/********************************************************************************/
static gchar* get_format_image_from_option()
{
	if(strcmp(formatFilm,"BMP")==0) return "bmp";
	if(strcmp(formatFilm,"PPM")==0) return "ppm";
	if(strcmp(formatFilm,"PNG")==0) return "png";
	if(strcmp(formatFilm,"JPEG")==0) return "jpg";
	if(strcmp(formatFilm,"PNG transparent")==0) return "png";
	if(strcmp(formatFilm,"Povray")==0) return "pov";
	return "UNK";
}
/********************************************************************************/
static void filmSelected(GtkWidget *widget)
{
	if(GTK_IS_WIDGET(buttonCheckFilm)&& GTK_TOGGLE_BUTTON (buttonCheckFilm)->active)
	{
		createFilm = TRUE;
		if(GTK_IS_WIDGET(buttonDirFilm))  gtk_widget_set_sensitive(buttonDirFilm, TRUE);
		if(GTK_IS_WIDGET(comboListFilm))  gtk_widget_set_sensitive(comboListFilm, TRUE);
	}
	else
	{
		createFilm = FALSE;
		if(GTK_IS_WIDGET(buttonDirFilm))  gtk_widget_set_sensitive(buttonDirFilm, FALSE);
		if(GTK_IS_WIDGET(comboListFilm))  gtk_widget_set_sensitive(comboListFilm, FALSE);
	}
}
/********************************************************************************/
static void showMessageEnd()
{
	gchar* format =get_format_image_from_option();
	gchar* message = messageAnimatedImage(format);
	gchar* t = g_strdup_printf(_("\nA seriess of gab*.%s files was created in \"%s\" directeory.\n\n\n%s") , format, get_last_directory(),message);
	GtkWidget* winDlg = Message(t,_("Info"),TRUE);
	g_free(message);
	gtk_window_set_modal (GTK_WINDOW (winDlg), TRUE);
	g_free(t);
}
/********************************************************************************/
static void unActivateFilm()
{
	createFilm = FALSE;
	numFileFilm = 0;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheckFilm), FALSE);
	if(GTK_IS_WIDGET(buttonDirFilm))  gtk_widget_set_sensitive(buttonDirFilm, FALSE);
	if(GTK_IS_WIDGET(comboListFilm))  gtk_widget_set_sensitive(comboListFilm, FALSE);
}
/********************************************************************************************************/
static void set_format_film(GtkComboBox *combobox, gpointer d)
{
	GtkTreeIter iter;
	gchar* data = NULL;
	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	if(!data) return;
	sprintf(formatFilm ,"%s",(gchar*)data);
}
/********************************************************************************************************/
static GtkWidget *create_list_of_formats()
{
        GtkTreeIter iter;
        GtkListStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
	gint k;
	gchar* options[] = {"BMP","PPM", "JPEG", "PNG", "PNG transparent", "Povray"};
	guint numberOfElements = G_N_ELEMENTS (options);


	k = 0;
	store = gtk_list_store_new (1,G_TYPE_STRING);
	for(i=0;i<numberOfElements; i++)
	{
        	gtk_list_store_append (store, &iter);
        	gtk_list_store_set (store, &iter, 0, options[i], -1);
		if(strcmp(options[i],formatFilm)==0) k = i;
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(set_format_film), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);
  	gtk_combo_box_set_active(GTK_COMBO_BOX (combobox), k);
	return combobox;
}
static void buttonAxisSelected(GtkWidget *widget)
{
	if(GTK_IS_WIDGET(buttonXAxis) && GTK_TOGGLE_BUTTON (buttonXAxis)->active) selectedAxis = XAXIS;
	if(GTK_IS_WIDGET(buttonYAxis) && GTK_TOGGLE_BUTTON (buttonYAxis)->active) selectedAxis = YAXIS;
	if(GTK_IS_WIDGET(buttonZAxis) && GTK_TOGGLE_BUTTON (buttonZAxis)->active) selectedAxis = ZAXIS;

}
/********************************************************************************/
static void addEntriesButtons(GtkWidget* box)
{
	GtkWidget *Button;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *table;
	gchar t[BSIZE];
	gint i;
	GtkWidget *separator;
	GtkWidget* formatBox;

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start(GTK_BOX(box), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);

  	table = gtk_table_new(11,3,FALSE);
	gtk_box_pack_start(GTK_BOX(vboxframe), table,TRUE,TRUE,0);

	i = 0;
	buttonXAxis = gtk_radio_button_new_with_label( NULL,_("Rotation about X axis"));
	gtk_table_attach(GTK_TABLE(table),buttonXAxis,0,3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);

	i++;
	buttonYAxis = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonXAxis)), _("Rotation about Y axis ")); 
	gtk_table_attach(GTK_TABLE(table),buttonYAxis,0,3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
	i++;
	buttonZAxis = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonXAxis)), _("Rotation about Z axis ")); 
	gtk_table_attach(GTK_TABLE(table),buttonZAxis,0,3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
  	g_signal_connect (G_OBJECT(buttonXAxis), "toggled", G_CALLBACK (buttonAxisSelected), NULL);  
  	g_signal_connect (G_OBJECT(buttonYAxis), "toggled", G_CALLBACK (buttonAxisSelected), NULL);  
  	g_signal_connect (G_OBJECT(buttonZAxis), "toggled", G_CALLBACK (buttonAxisSelected), NULL);  
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonYAxis), TRUE);

	i++;
	separator = gtk_hseparator_new();
	gtk_table_attach(GTK_TABLE(table),separator,0,3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);


	i++;
	add_label_table(table,_(" Time step(s) "),(gushort)i,0);
	add_label_table(table," : ",(gushort)i,1); 
	EntryVelocity = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table),EntryVelocity,2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
	gtk_editable_set_editable((GtkEditable*) EntryVelocity,TRUE);
	sprintf(t,"%lf",velocity);
	gtk_entry_set_text(GTK_ENTRY(EntryVelocity),t);

	i++;
	LabelNRotation = add_label_table(table,_(" Number of Rotation by cycle"),(gushort)i,0);
	add_label_table(table," : ",(gushort)i,1); 
	EntryNRotation = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table),EntryNRotation,2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
	gtk_editable_set_editable((GtkEditable*) EntryNRotation,TRUE);
	sprintf(t,"%d",nRotation);
	gtk_entry_set_text(GTK_ENTRY(EntryNRotation),t);

	i++;
	separator = gtk_hseparator_new();
	gtk_table_attach(GTK_TABLE(table),separator,0,3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);

	i++;
	buttonCheckFilm = gtk_check_button_new_with_label (_("Create a film"));
	createFilm = FALSE;
	numFileFilm = 0;
	gtk_table_attach(GTK_TABLE(table),buttonCheckFilm,0,1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
  	g_signal_connect (G_OBJECT(buttonCheckFilm), "toggled", G_CALLBACK (filmSelected), NULL);  

	formatBox = create_list_of_formats();
	gtk_table_attach(GTK_TABLE(table),formatBox,1,1+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);

	buttonDirFilm = create_button(WinDlg,_("Folder"));
	gtk_table_attach(GTK_TABLE(table),buttonDirFilm,2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
  	g_signal_connect(G_OBJECT(buttonDirFilm), "clicked",(GCallback)set_directory,NULL);
	comboListFilm = formatBox;

	if(GTK_IS_WIDGET(buttonDirFilm))  gtk_widget_set_sensitive(buttonDirFilm, FALSE);
	if(GTK_IS_WIDGET(comboListFilm))  gtk_widget_set_sensitive(comboListFilm, FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheckFilm), FALSE);


	i++;
	separator = gtk_hseparator_new();
	gtk_table_attach(GTK_TABLE(table),separator,0,3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);


  	table = gtk_table_new(1,2,TRUE);
	gtk_box_pack_start (GTK_BOX (vboxframe), table, TRUE, TRUE, 0);
	i=0;
	Button = create_button(WinDlg,_("Play"));
	gtk_table_attach(GTK_TABLE(table),Button,0,0+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
	PlayButton = Button;

	Button = create_button(WinDlg,_("Stop"));
	gtk_table_attach(GTK_TABLE(table),Button,1,1+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
	StopButton = Button;

  	g_signal_connect(G_OBJECT(PlayButton), "clicked",(GCallback)play_animation,NULL);
  	g_signal_connect(G_OBJECT(StopButton), "clicked",(GCallback)stop_animation,NULL);
  	g_signal_connect_swapped (G_OBJECT (EntryVelocity), "activate", (GCallback)reset_parameters, NULL);
  	g_signal_connect_swapped (G_OBJECT (EntryNRotation), "activate", (GCallback)reset_parameters, NULL);
}
/*****************************************************************************/
static gboolean createImagesFile()
{
	gchar* message = NULL;
	gchar* t;
	gchar* format;
	if(!createFilm)
	{
		setTextInProgress(" ");
		return FALSE;
	}
	format =get_format_image_from_option();
	t = g_strdup_printf(_("The %s%sgab%d.%s file was created"), get_last_directory(),G_DIR_SEPARATOR_S,numFileFilm, format);

	if(!strcmp(formatFilm,"BMP")) message = new_bmp(get_last_directory(), ++numFileFilm);
	if(!strcmp(formatFilm,"PPM")) message = new_ppm(get_last_directory(), ++numFileFilm);
	if(!strcmp(formatFilm,"JPEG")) message = new_jpeg(get_last_directory(), ++numFileFilm);
	if(!strcmp(formatFilm,"PNG")) message = new_png(get_last_directory(), ++numFileFilm);
	if(!strcmp(formatFilm,"Povray")) message = new_pov(get_last_directory(), ++numFileFilm);
	if(!strcmp(formatFilm,"PNG transparent")) message = new_png_without_background(get_last_directory(), ++numFileFilm);

	if(message == NULL) setTextInProgress(t);
	else
	{
    		GtkWidget* m;
		createFilm = FALSE;
		numFileFilm = 0;
    		m = Message(message,_("Error"),TRUE);
		gtk_window_set_modal (GTK_WINDOW (m), TRUE);
	}
	g_free(t);
	return TRUE;
}
/********************************************************************************/
static void animate_rotation()
{

	gdouble  phi = -360.0/nRotation;
	gint i = 0;

	reset_parameters(NULL, NULL);
	numFileFilm = 0;

	if(GTK_IS_WIDGET(buttonCheckFilm)) gtk_widget_set_sensitive(buttonCheckFilm, FALSE);
	if(GTK_IS_WIDGET(comboListFilm)) gtk_widget_set_sensitive(comboListFilm, FALSE);
	if(GTK_IS_WIDGET(buttonDirFilm))  gtk_widget_set_sensitive(buttonDirFilm, FALSE);

	if(GTK_IS_WIDGET(buttonXAxis)) gtk_widget_set_sensitive(buttonXAxis, FALSE);
	if(GTK_IS_WIDGET(buttonYAxis)) gtk_widget_set_sensitive(buttonYAxis, FALSE);
	if(GTK_IS_WIDGET(buttonZAxis)) gtk_widget_set_sensitive(buttonZAxis, FALSE);
	if(GTK_IS_WIDGET(EntryNRotation)) gtk_widget_set_sensitive(EntryNRotation, FALSE);
	if(GTK_IS_WIDGET(LabelNRotation)) gtk_widget_set_sensitive(LabelNRotation, FALSE);

	while(play)
	{
		rotationAboutAnAxis(GLArea, phi,selectedAxis);
		while( gtk_events_pending() ) gtk_main_iteration();
		glarea_rafresh(GLArea);
		createImagesFile();
		Waiting(velocity);
		i++;
	
		if(i==nRotation)
		{
			i = 0;
			if(numFileFilm>0) showMessageEnd();
			unActivateFilm();
			while( gtk_events_pending() ) gtk_main_iteration();
		}


	}

	if(numFileFilm>0) showMessageEnd();
	unActivateFilm();

	if(GTK_IS_WIDGET(buttonCheckFilm)) gtk_widget_set_sensitive(buttonCheckFilm, TRUE);
	if(GTK_IS_WIDGET(comboListFilm)) gtk_widget_set_sensitive(comboListFilm, FALSE);
	if(GTK_IS_WIDGET(buttonDirFilm))  gtk_widget_set_sensitive(buttonDirFilm, FALSE);

	if(GTK_IS_WIDGET(buttonXAxis)) gtk_widget_set_sensitive(buttonXAxis, TRUE);
	if(GTK_IS_WIDGET(buttonYAxis)) gtk_widget_set_sensitive(buttonYAxis, TRUE);
	if(GTK_IS_WIDGET(buttonZAxis)) gtk_widget_set_sensitive(buttonZAxis, TRUE);
	if(GTK_IS_WIDGET(EntryNRotation)) gtk_widget_set_sensitive(EntryNRotation, TRUE);
	if(GTK_IS_WIDGET(LabelNRotation)) gtk_widget_set_sensitive(LabelNRotation, TRUE);
	stop_animation(NULL, NULL);
}
/***********************************************************************/
void animationRotationDlg()
{
	GtkWidget *Win;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *parentWindow = PrincipalWindow;

	if(WinDlg) return;
	if(nCenters <1 ) 
	{
		Message(_("Sorry, the number of atoms can not <1"),_("Error"),TRUE);
		return;
	}

	Win= gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(parentWindow));
	/* gtk_window_set_default_size (GTK_WINDOW(Win),-1,(gint)(ScreenHeightD*0.69));*/
	gtk_window_set_title(GTK_WINDOW(Win),_("Rotation animation"));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	WinDlg = Win;

  	add_child(PrincipalWindow,Win,destroyAnimationDlg,"Anim. Rot.");
  	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)delete_child,NULL);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_container_add (GTK_CONTAINER (Win), vbox);
	hbox = create_hbox_false(vbox);
	gtk_widget_realize(Win);

	addEntriesButtons(vbox);
	gtk_widget_show_all(vbox);

	gtk_widget_show_now(Win);

	/* fit_windows_position(PrincipalWindow, Win);*/

	stop_animation(NULL, NULL);
}
