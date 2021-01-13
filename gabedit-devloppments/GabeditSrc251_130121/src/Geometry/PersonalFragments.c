/*PersonalFragments.c  */
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
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "../Geometry/Fragments.h"
#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Utils/Utils.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/PersonalFragments.h"
#include "../Geometry/SavePersonalFragments.h"
#include "../Geometry/LoadPersonalFragments.h"
#include "../Geometry/MenuToolBarGeom.h"
#include "../Geometry/FragmentsSelector.h"

#define NENTRYS 5
typedef enum
{
  E_GROUPE=0,
  E_FRAGMENT,
  E_TODELETE,
  E_TOBONDTO,
  E_ANGLE
} SetEntryType;

static GtkWidget* Entries[NENTRYS];
static gdouble labelWidth = 0.15;
static gdouble entryWidth = 0.15;

static PersonalFragments* personalFragments = NULL;
/*****************************************************************************/
gint getNumberOfGroups()
{
	if(personalFragments) return personalFragments->numberOfGroupes;
	else return 0;
}
/*****************************************************************************/
static gint getIndexGeom(gint n)
{
	gint k;
	if(n<1)
		return -1;
	for (k=0;k<(gint)Natoms;k++)
		if(geometry[k].N == (guint)n)
			return k;
        return -1;
}
/********************************************************************************/
static gint numGroupe(gchar* groupName)
{
	gint i;
	
	for(i=0;i<personalFragments->numberOfGroupes;i++)
	{
		if(!strcmp(groupName,personalFragments->personalGroupes[i].groupName))
			return i;
	}
	return -1;
}
/*****************************************************************/
static gchar** getListGroupes(gint*n)
{
	gchar**t = NULL;
	gint i;

	if(!personalFragments) 
	{
		*n = 0;
		return NULL;
	}
	*n = personalFragments->numberOfGroupes;
	if(*n<1)
		return t;

	t = g_malloc((*n)*sizeof(gchar*));
	for(i=0;i<personalFragments->numberOfGroupes;i++)
		t[i] = g_strdup(personalFragments->personalGroupes[i].groupName);

	return t;
}
/*****************************************************************/
static gchar** getListFragments(gint*n, gchar* groupName)
{
	gchar**t = NULL;
	gint i;
	gint k;
	PersonalGroupe* personalGroupes = NULL;

	k = numGroupe(groupName);
	if(k<0)
	{
		*n = 0;
		return NULL;
	}

	personalGroupes = personalFragments->personalGroupes;

	*n = personalGroupes[k].numberOfFragments;
	t = g_malloc((*n)*sizeof(gchar*));

	for(i=0;i<personalGroupes[k].numberOfFragments;i++)
		t[i] = g_strdup(personalGroupes[k].fragments[i].name);

	return t;
}
/*****************************************************************/
static gchar** getListGroupesWithFragments(gint*n)
{
	gint k;
	gint i;
	gint ng;
	gint nf;
	gchar** groups = getListGroupes(&ng);
	gchar** fragments;
	if(ng<1)
	{
		*n = 0;
		return NULL;
	}

	k= 0;
	while(k<ng)
	{
		fragments = getListFragments(&nf,groups[k]);
		if(nf==0)
		{
			g_free(groups[k]);
			for(i=k;i<ng-1;i++)
			{
				groups[i] = groups[i+1];
			}
			ng--;
			if(ng==0)
			{
				*n = 0;
				return NULL;
			}
			groups = g_realloc(groups,ng*sizeof(gchar*));
		}
		else
			k++;

  		for(i=0;i<nf;i++)
	  		g_free(fragments[i]);
	}
	*n = ng;
	return groups;
}
/********************************************************************************/
static void addGroupe(GtkWidget* win, gpointer d)
{
	gchar *groupName;
	PersonalGroupe* personalGroupes = NULL;
	gint numberOfGroupes = 0;
	gint i;

	groupName = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entries[E_GROUPE])));

	if(strlen(groupName)<1)
		return;

	for(i=0;i<(gint)strlen(groupName);i++)
		if(groupName[i] == '/' || groupName[i] == '_')
			groupName[i] = ' ';

	delete_all_spaces(groupName);

	if(strlen(groupName)<1)
		return;

	if(!personalFragments)
	{
		personalFragments = g_malloc(sizeof(PersonalFragments));
		personalFragments->numberOfGroupes = 0;
		personalFragments->personalGroupes = NULL;
	}

	if(numGroupe(groupName)!=-1)
	{
		gchar* t = g_strdup_printf(_("Sorry, I can not add this group\n%s is available."),groupName);
		GtkWidget* w = Message(t,_("Error"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(win));
		g_free(groupName);
		g_free(t);
		return;
	}
	numberOfGroupes = personalFragments->numberOfGroupes;
	personalGroupes = personalFragments->personalGroupes;

	numberOfGroupes++;
	if(!personalGroupes)
		personalGroupes = g_malloc(sizeof(PersonalGroupe));
	else
		personalGroupes = 
			g_realloc(personalGroupes,numberOfGroupes*sizeof(PersonalGroupe));

	personalGroupes[numberOfGroupes-1].groupName = g_strdup(groupName);
	personalGroupes[numberOfGroupes-1].numberOfFragments = 0;
	personalGroupes[numberOfGroupes-1].fragments = NULL;

	personalFragments->numberOfGroupes = numberOfGroupes;
	personalFragments->personalGroupes = personalGroupes;

	/* add_a_personal_group_to_menu(groupName);*/

	g_free(groupName);
	gtk_widget_destroy(win);
}
/*****************************************************************/
void newGroupeDlg(gpointer data, guint Operation,GtkWidget* wid)
{
	GtkWidget *WinDlg;
	GtkWidget *Button;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *vboxframe;

	
	WinDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(WinDlg),_("New Group"));
	gtk_window_set_position(GTK_WINDOW(WinDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(WinDlg),GTK_WINDOW(GeomDlg));
	gtk_window_set_modal(GTK_WINDOW(WinDlg),TRUE);

	g_signal_connect(G_OBJECT(WinDlg),"delete_event",(GCallback)gtk_widget_destroy,NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->vbox), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	hbox=create_hbox_false(vboxframe);

	Entries[E_GROUPE] = create_label_entry(hbox,_("Group Name : "),
		  (gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));

	gtk_widget_realize(WinDlg);

	Button = create_button(WinDlg,_("Cancel"));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(Button), "clicked", (GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);

	Button = create_button(WinDlg,_("OK"));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)addGroupe,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Button);
    

	gtk_widget_show_all(WinDlg);
}
/*****************************************************************/
static void deleteOneGroupe(GtkWidget*win, gpointer data)
{
	PersonalGroupe* personalGroupes = NULL;
	gint numberOfGroupes = 0;
	gint i;
	gint k;
	gchar* groupName;

	groupName = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entries[E_GROUPE])));
	/* delete_a_personal_group_from_menu(groupName);*/

	if(!personalFragments)
		return;

	k = numGroupe(groupName);
	if(k<0)
		return;

	numberOfGroupes = personalFragments->numberOfGroupes;
	personalGroupes = personalFragments->personalGroupes;

	if(personalGroupes[k].groupName)
	{
		g_free(personalGroupes[k].groupName);
		if(personalGroupes[k].fragments)
		{
			for(i=0;i<personalGroupes[k].numberOfFragments;i++)
			{
				if(personalGroupes[k].fragments[i].name)
					g_free(personalGroupes[k].fragments[i].name);
				FreeFragment(&personalGroupes[k].fragments[i].f);
			}
			g_free(personalGroupes[k].fragments);
		}
	}
	for(i=k;i<numberOfGroupes-1;i++)
		personalGroupes[i] = personalGroupes[i+1];
	

	numberOfGroupes--;
	if(numberOfGroupes>0)
		personalGroupes = g_realloc(personalGroupes,numberOfGroupes*sizeof(PersonalGroupe));

	personalFragments->numberOfGroupes = numberOfGroupes;
	personalFragments->personalGroupes = personalGroupes;
	g_free(groupName);
	if(numberOfGroupes <1)
	{
		/*
		set_sensitive_personal("/Add/Personal/Edit/Delete a Groupe",FALSE);
		set_sensitive_personal("/Add/Personal/Edit/Add this molecule to personal Fragments",FALSE);
		set_sensitive_personal("/Add/Personal/Edit/Remove a Fragment",FALSE);
		*/
	}
	{
		gint ng;
		gchar** groups = getListGroupesWithFragments(&ng);
		if(ng==0)
		{
			/*
			set_sensitive_personal("/Add/Personal/Edit/Remove a Fragment", FALSE);
			*/
		}
		else
		{
  			for(i=0;i<ng;i++)
				g_free(groups[i]);
		}
	}

	gtk_widget_destroy(win);
	savePersonalFragments(NULL);
	rafresh_fragments_selector();
}
/*****************************************************************/
void deleteGroupeDlg(gpointer data, guint Operation,GtkWidget* wid)
{
	GtkWidget *WinDlg;
	GtkWidget *Button;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gint n;
	gchar** tlist;
	gint i;

	
	tlist=getListGroupes(&n);
	if(n<1)
	{
		gchar* t = g_strdup(_("Sorry, no group to delete."));
		GtkWidget* w = Message(t,_("Error"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(GeomDlg));
		g_free(t);
		return;
	}
	WinDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(WinDlg),_("Delete a Group"));
	gtk_window_set_position(GTK_WINDOW(WinDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(WinDlg),GTK_WINDOW(GeomDlg));
	gtk_window_set_modal(GTK_WINDOW(WinDlg),TRUE);

	g_signal_connect(G_OBJECT(WinDlg),"delete_event",(GCallback)gtk_widget_destroy,NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->vbox), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);


	hbox=create_hbox_false(vboxframe);
	Entries[E_GROUPE] = create_label_combo(hbox,_("Group to delete  : "),tlist,n,
			TRUE,(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	gtk_editable_set_editable((GtkEditable*) Entries[E_GROUPE],FALSE);

  	for(i=0;i<n;i++)
	  	g_free(tlist[i]);

	gtk_widget_realize(WinDlg);

	Button = create_button(WinDlg,_("Cancel"));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(Button), "clicked", (GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);

	Button = create_button(WinDlg,_("OK"));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)deleteOneGroupe,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Button);
    

	gtk_widget_show_all(WinDlg);
}
/*****************************************************************/
static void addFragment(GtkWidget* win, gpointer data)
{
	gchar* name;
	gchar* groupName = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entries[E_GROUPE])));
	gchar* fragmentName = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entries[E_FRAGMENT])));
	G_CONST_RETURN gchar* todelete = gtk_entry_get_text(GTK_ENTRY(Entries[E_TODELETE]));
	G_CONST_RETURN gchar* tobondto = gtk_entry_get_text(GTK_ENTRY(Entries[E_TOBONDTO]));
	G_CONST_RETURN gchar* angle    = gtk_entry_get_text(GTK_ENTRY(Entries[E_ANGLE]));
	gint i;
	gint itodelete = atoi(todelete);
	gint itobondto = atoi(tobondto);
	gint iangle = atoi(angle);
	PersonalGroupe* personalGroupes = personalFragments->personalGroupes;
	gint numberOfFragments = 0;
	OnePersonalFragment* fragments = NULL;
	gint k;
	Fragment f;

	if(strlen(fragmentName)<1)
		return;

	for(i=0;i<(gint)strlen(fragmentName);i++)
		if(fragmentName[i] == '/' || fragmentName[i] == '_')
			fragmentName[i] = ' ';

	delete_all_spaces(fragmentName);

	if(strlen(fragmentName)<1)
		return;

	if(itodelete==-1)
		itobondto = -1;
	if(itobondto==-1)
		iangle = -1;

	itodelete = getIndexGeom(itodelete);
	itobondto = getIndexGeom(itobondto);
	iangle    = getIndexGeom(iangle);

	if(itodelete >=0)
	{
		if(itobondto==itodelete)
		{
			gchar* t = 
				g_strdup(_("Sorry, number of atom to delete can not = number of atom to bond to."));
			GtkWidget* w = Message(t,_("Error"),TRUE);
			gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  			gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(win));
			g_free(t);
			return;
		}
		if(iangle==itodelete)
		{
			gchar* t = 
				g_strdup(_("Sorry, number of atom to delete can not = number of atom used for set angle."));
			GtkWidget* w = Message(t,_("Error"),TRUE);
			gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  			gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(win));
			g_free(t);
			return;
		}
		if(itobondto>0)
		{
			if(itobondto==iangle)
			{
				gchar* t = 
				g_strdup(_("Sorry, number of atom to bond to can not =  number of atom used for set angle."));
				GtkWidget* w = Message(t,_("Error"),TRUE);
				gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  				gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(win));
				g_free(t);
				return;
			}
		}
	}


	k = numGroupe(groupName);

	if(k<0)
		return;
	
	numberOfFragments = personalGroupes[k].numberOfFragments;
	fragments = personalGroupes[k].fragments;

	numberOfFragments++;
	if(numberOfFragments==1)
		fragments = g_malloc(sizeof(OnePersonalFragment));
	else
		fragments = g_realloc(fragments, numberOfFragments*sizeof(OnePersonalFragment));

	fragments[numberOfFragments-1].name = g_strdup(fragmentName);
	fragments[numberOfFragments-1].f.NAtoms = 0;


	f.NAtoms = Natoms ;
	f.Atoms = g_malloc(f.NAtoms*sizeof(Atom));
	for(i=0;i<(gint)Natoms;i++)
	{
		f.Atoms[i].Residue = g_strdup(geometry[i].Residue);
		f.Atoms[i].Symb = g_strdup(geometry[i].Prop.symbol);
		f.Atoms[i].mmType = g_strdup(geometry[i].mmType);
		f.Atoms[i].pdbType = g_strdup(geometry[i].pdbType);
		f.Atoms[i].Coord[0] = geometry[i].X;
		f.Atoms[i].Coord[1] = geometry[i].Y;
		f.Atoms[i].Coord[2] = geometry[i].Z;
		f.Atoms[i].Charge = geometry[i].Charge;
	}

	f.atomToDelete = itodelete;
	f.atomToBondTo = itobondto;
	f.angleAtom    = iangle;

	fragments[numberOfFragments-1].f = f;




	personalGroupes[k].fragments = fragments;
	personalGroupes[k].numberOfFragments = numberOfFragments;

	
	name = g_strdup_printf("%s/%s",groupName,fragmentName); 

	/* add_a_personal_fragement_to_menu(groupName,fragmentName);*/
	savePersonalFragments(NULL);
	rafresh_fragments_selector();
	gtk_widget_destroy(win);
}
/*****************************************************************/
static gchar** getListNumbers(gint*n)
{
	gchar**t = NULL;
	gint i;
	if(Natoms<=0)
	{
		t = g_malloc(sizeof(gchar*));
		t[0] = g_strdup("-1");
		*n = 1;
		return t;
	}
	t = g_malloc((Natoms+1)*sizeof(gchar*));
	t[0] = g_strdup("-1");
	for(i=0;i<(gint)Natoms;i++)
		t[i+1] = g_strdup_printf("%d",i+1);
	*n = Natoms+1;
	return t;
}
/*****************************************************************/
void addFragmentDlg(gpointer data, guint Operation,GtkWidget* wid)
{
	GtkWidget *WinDlg;
	GtkWidget *Button;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *table;
	GtkWidget *combo;
	gint nn = 0;
	gint ng = 0;
	gchar** numbers = NULL;
	gchar** groups = NULL;
	gint i;

	groups = getListGroupes(&ng);
	if(ng<1)
	{
		gchar* t = g_strdup(_("Sorry, no group available."));
		GtkWidget* w = Message(t,_("Error"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(GeomDlg));
		g_free(t);
		return;
	}

	if(Natoms<1)
	{
		gchar* t = g_strdup(_("Sorry, number of atoms = 0."));
		GtkWidget* w = Message(t,_("Error"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(GeomDlg));
		g_free(t);
		return;
	}
	
	WinDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(WinDlg),_("New Fragment"));
	gtk_window_set_position(GTK_WINDOW(WinDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(WinDlg),GTK_WINDOW(GeomDlg));
	gtk_window_set_modal(GTK_WINDOW(WinDlg),TRUE);

	g_signal_connect(G_OBJECT(WinDlg),"delete_event",(GCallback)gtk_widget_destroy,NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->vbox), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);

  	table = gtk_table_new(5,3,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);

	i=0;
	add_label_table(table,_(" Group Name "),(gushort)i,0); 
	add_label_table(table," : ",(gushort)i,1); 
	combo = create_combo_box_entry(groups,ng,FALSE,-1,-1);
	add_widget_table(table,combo,(gushort)i,2);
	Entries[E_GROUPE] = GTK_BIN (combo)->child;
	gtk_editable_set_editable((GtkEditable*) Entries[E_GROUPE],FALSE);

	i=1;
	add_label_table(table,_(" Fragment Name "),(gushort)i,0); 
	add_label_table(table," : ",(gushort)i,1); 
	Entries[E_FRAGMENT] = gtk_entry_new();
	add_widget_table(table,Entries[E_FRAGMENT],(gushort)i,2);

	numbers=getListNumbers(&nn);

	i=2;
	add_label_table(table,_(" Number of atoms to delete "),(gushort)i,0); 
	add_label_table(table," : ",(gushort)i,1); 
	combo = create_combo_box_entry(numbers,nn,FALSE,-1,-1);
	Entries[E_TODELETE] = GTK_BIN (combo)->child;
	add_widget_table(table,combo,(gushort)i,2);
	gtk_editable_set_editable((GtkEditable*) Entries[E_TODELETE],FALSE);

	i=3;
	add_label_table(table,_(" Number of atoms to bond to "),(gushort)i,0); 
	add_label_table(table," : ",(gushort)i,1); 
	combo = create_combo_box_entry(numbers,nn,FALSE,-1,-1);
	Entries[E_TOBONDTO] = GTK_BIN (combo)->child;
	add_widget_table(table,combo,(gushort)i,2);
	gtk_editable_set_editable((GtkEditable*) Entries[E_TOBONDTO],FALSE);

	i=4;
	add_label_table(table,_(" Number of atoms for set angle "),(gushort)i,0); 
	add_label_table(table," : ",(gushort)i,1); 
	combo = create_combo_box_entry(numbers,nn,FALSE,-1,-1);
	Entries[E_ANGLE] = GTK_BIN (combo)->child;
	add_widget_table(table,combo,(gushort)i,2);
	gtk_editable_set_editable((GtkEditable*) Entries[E_ANGLE],FALSE);


  	for(i=0;i<nn;i++)
	  	g_free(numbers[i]);
	g_free(numbers);

  	for(i=0;i<ng;i++)
	  	g_free(groups[i]);
	g_free(groups);
  
	gtk_widget_realize(WinDlg);

	Button = create_button(WinDlg,_("Cancel"));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(Button), "clicked", (GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);

	Button = create_button(WinDlg,_("OK"));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)addFragment,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Button);
    

	gtk_widget_show_all(WinDlg);
}
/*****************************************************************/
static void deleteOneFragment(GtkWidget* win, gpointer data)
{
	gchar* groupName = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entries[E_GROUPE])));
	gchar* fragmentName = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entries[E_FRAGMENT])));
	PersonalGroupe* personalGroupes = personalFragments->personalGroupes;
	gint numberOfFragments = 0;
	OnePersonalFragment* fragments = NULL;
	gint k;
	gint i;
	gint j;

	k = numGroupe(groupName);

	if(k<0)
		return;
	
	numberOfFragments = personalGroupes[k].numberOfFragments;

	if(numberOfFragments<1)
		return;

	fragments = personalGroupes[k].fragments;

	for(i=0;i<numberOfFragments;i++)
	{
		if(!strcmp(fragmentName,fragments[i].name))
		{
			FreeFragment(&fragments[i].f);
			g_free(fragments[i].name);
			for(j=i;j<numberOfFragments-1;j++)
			{
				fragments[j].name = fragments[j+1].name;
				fragments[j].f = fragments[j+1].f;
			}
			numberOfFragments--;
			if(numberOfFragments>0)
			{
				fragments = g_realloc(fragments,
						numberOfFragments*sizeof(OnePersonalFragment));
			}
			else
			{
				g_free(fragments);
				fragments = NULL;
			}
			break;
		}
	}

	personalGroupes[k].fragments = fragments;
	personalGroupes[k].numberOfFragments = numberOfFragments;

	{
		gint ng;
		gchar** groups = getListGroupesWithFragments(&ng);
		if(ng==0)
		{
			/*
			set_sensitive_personal("/Add/Personal/Edit/Remove a Fragment", FALSE);
			*/
		}
		else
		{
  			for(i=0;i<ng;i++)
				g_free(groups[i]);
		}
	}
	/* delete_a_personal_fragment_from_menu(groupName,fragmentName);*/
	savePersonalFragments(NULL);
	rafresh_fragments_selector();
	gtk_widget_destroy(win);
}
/*****************************************************************/
static void resetFragmentCombo(GtkWidget* combo,gpointer data)
{
	gchar* groupName = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entries[E_GROUPE])));
	gchar** fragments = NULL;
  	GList *list=NULL;
	gint i;
	gint nf = 0;

	fragments = getListFragments(&nf,groupName);
	if(nf<1) return;
	for (i=0;i<nf;i++)
		list=g_list_append(list,fragments[i]);
	gtk_combo_box_entry_set_popdown_strings(combo,list);

  	for(i=0;i<nf;i++) g_free(fragments[i]);

}
/*****************************************************************/
void deleteFragmentDlg(gpointer data, guint Operation,GtkWidget* wid)
{
	GtkWidget *WinDlg;
	GtkWidget *Button;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *combo;
	GtkWidget *label;
	gint nf = 0;
	gint ng = 0;
	gchar** fragments = NULL;
	gchar** groups = NULL;
  	GList *list=NULL;
	gint i;

	groups = getListGroupes(&ng);
	if(ng<1)
	{
		gchar* t = g_strdup(_("Sorry, no group available."));
		GtkWidget* w = Message(t,_("Error"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(GeomDlg));
		g_free(t);
		return;
	}
  	for(i=0;i<ng;i++)
		g_free(groups[i]);

	groups = getListGroupesWithFragments(&ng);
	if(ng==0)
	{
		gchar* t = g_strdup(_("Sorry, no fragment available."));
		GtkWidget* w = Message(t,_("Error"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(GeomDlg));
		g_free(t);
		return;
	}
	fragments = getListFragments(&nf,groups[0]);
	
	WinDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(WinDlg),_("Remove a Fragment"));
	gtk_window_set_position(GTK_WINDOW(WinDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(WinDlg),GTK_WINDOW(GeomDlg));
	gtk_window_set_modal(GTK_WINDOW(WinDlg),TRUE);

	g_signal_connect(G_OBJECT(WinDlg),"delete_event",(GCallback)gtk_widget_destroy,NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->vbox), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);

	hbox=create_hbox_false(vboxframe);
	Entries[E_GROUPE] = create_label_combo(hbox,_("Group Name  : "),groups,ng,
			TRUE,(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	gtk_editable_set_editable((GtkEditable*) Entries[E_GROUPE],FALSE);

	hbox=create_hbox_false(vboxframe);

  	label = gtk_label_new(_("Fragment to delete : "));
	gtk_widget_set_size_request(GTK_WIDGET(label),(gint)(ScreenHeight*labelWidth),-1);
	gtk_box_pack_start (GTK_BOX(hbox), label, TRUE, TRUE, 1);

	for (i=0;i<nf;i++)
		list=g_list_append(list,fragments[i]);
	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),(gint)(ScreenHeight*entryWidth),-1);
	gtk_box_pack_start (GTK_BOX(hbox), combo, TRUE, TRUE, 1);

	Entries[E_FRAGMENT] = GTK_BIN (combo)->child;
	gtk_editable_set_editable((GtkEditable*) Entries[E_FRAGMENT],FALSE);
	g_signal_connect_swapped(G_OBJECT(Entries[E_GROUPE]), "changed",
			G_CALLBACK(resetFragmentCombo),GTK_OBJECT(combo));

  	for(i=0;i<ng;i++)
	  	g_free(groups[i]);
  	for(i=0;i<nf;i++)
	  	g_free(fragments[i]);

	gtk_widget_realize(WinDlg);

	Button = create_button(WinDlg,_("Cancel"));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(Button), "clicked", (GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);

	Button = create_button(WinDlg,_("OK"));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)deleteOneFragment,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Button);
    

	gtk_widget_show_all(WinDlg);
}
/*****************************************************************/
void addPersonalFragment(gpointer data, guint Operation,GtkWidget* wid)
{
	gchar* name = (gchar*)data;
	gchar* groupName = NULL;
	gchar* fragmentName = NULL;
	gchar* slash = strstr(name,"/");
	gint i;
	gint len;
	gint k;
	OnePersonalFragment* fragments;
	gint numberOfFragments;
	PersonalGroupe* personalGroupes = NULL;

	personalGroupes = personalFragments->personalGroupes;
	
	if(slash == NULL)
	{
		return;
	}
	fragmentName = g_strdup(slash+1);
	groupName = g_malloc(strlen(name)*sizeof(gchar));

	len = 0;
	for(i=0;i<(gint)strlen(name);i++)
	{
		if(name[i] == '/')
		{
			len = i;
			break;
		}
		groupName[i] = name[i];
	}
	groupName[len] = '\0';

	k = numGroupe(groupName);

	if(k<0)
		return;
	
	numberOfFragments = personalGroupes[k].numberOfFragments;
	fragments = personalGroupes[k].fragments;
	for(i=0;i<numberOfFragments;i++)
	{
		if(!strcmp(fragments[i].name,fragmentName))
		{
			setPersonalFragment(fragments[i].f);
		}
	}

	g_free(groupName);
	g_free(fragmentName);
}
/*****************************************************************/
/*
void addGroupesToMenu()
{
	gint i;
	if(personalFragments)
	for(i=0;i<personalFragments->numberOfGroupes;i++)
	{
		add_a_personal_group_to_menu(personalFragments->personalGroupes[i].groupName);
	}
}
void addFragmentsToMenu()
{
	PersonalGroupe* personalGroupes;
	gint numberOfGroupes;
	gint i;
	gint j;
	gchar* name;

	if(!personalFragments)
		return;

	personalGroupes = personalFragments->personalGroupes;
	numberOfGroupes = personalFragments->numberOfGroupes;
	
	for(i=0;i<numberOfGroupes;i++)
	{
		for(j=0;j<personalGroupes[i].numberOfFragments;j++)
		{
			name = g_strdup_printf("%s/%s",
					personalGroupes[i].groupName,
					personalGroupes[i].fragments[j].name
					);
			add_a_personal_fragement_to_menu(personalGroupes[i].groupName, personalGroupes[i].fragments[j].name);
			g_free(name);


		}
	}
}
*/
/*****************************************************************/
void savePersonalFragments(GtkWidget* win)
{
	gchar* filename = g_strdup_printf("%s%sPersonalFragments.frg",
			gabedit_directory(), G_DIR_SEPARATOR_S);


	if(!saveAllPersonalFragments(personalFragments,filename) && win)
	{
		gchar* t = g_strdup_printf(_("Sorry, I can not create \"%s\" file"),filename);
		GtkWidget* w = Message(t,_("Error"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(win));
		g_free(t);
		return;
	}
	g_free(filename);
}
/************************************************************/
void loadPersonalFragments(GtkWidget* win)
{
	gchar* filename = g_strdup_printf("%s%sPersonalFragments.frg",
			gabedit_directory(), G_DIR_SEPARATOR_S);

	personalFragments = loadAllPersonalFragments(filename);
	if(!personalFragments && win)
	{
		gchar* t = g_strdup_printf(_("Sorry, I can not read \"%s\" file"),filename);
		GtkWidget* w = Message(t,_("Error"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(win));
		g_free(t);
		return;
	}
	g_free(filename);
}
/*****************************************************************/
