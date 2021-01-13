/* PovrayUtils.c */
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

#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "../Common/Windows.h"

typedef enum
{
  SKY_WATER = 0,
  SKY_CHEKER,
  ROOM,
  BALCK,
  WHITE
} TypeOfBackground;
static gchar* types[] = {N_("sky & water"), N_("sky & cheker"), N_("room"),  N_("black"), N_("white") };
static gint typeOfBackground[] = {SKY_WATER, SKY_CHEKER, ROOM, BALCK, WHITE };
static gint backgroundType = BALCK;
static gint tmpBackgoundType = BALCK;

/**************************************************************************************************************************************/
void applyPovrayOptions(GtkWidget *win, gpointer data)
{
	backgroundType = tmpBackgoundType;
}
/**************************************************************************************************************************************/
static void activateRadioButton(GtkWidget *button, gpointer data)
{
	gint* type = NULL;
	 
	if(!GTK_IS_WIDGET(button)) return;

	type = g_object_get_data(G_OBJECT (button), "Type");
	tmpBackgoundType = *type;
}
/**************************************************************************************************************************************/
static GtkWidget* addRadioButtonToATable(GtkWidget* table, GtkWidget* friendButton, gchar* label, gint i, gint j, gint k)
{
	GtkWidget *newButton;

	if(friendButton)
		newButton = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (friendButton)), label);
	else
		newButton = gtk_radio_button_new_with_label( NULL, label);

	gtk_table_attach(GTK_TABLE(table),newButton,j,j+k,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);

	g_object_set_data(G_OBJECT (newButton), "Type",NULL);
	return newButton;
}
/**************************************************************************************************************************************/
void createPOVBackgroundFrame(GtkWidget *box)
{
	GtkWidget* button;
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget *table = gtk_table_new(5,2,TRUE);
	gint i;

	frame = gtk_frame_new (_("Type of background"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	button = NULL;
	for(i=0;i<=WHITE;i++)
	{
		button = addRadioButtonToATable(table, button, types[i], i, 0,2);
		g_object_set_data(G_OBJECT (button), "Type",&typeOfBackground[i]);
		g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
		if(backgroundType== typeOfBackground[i]) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
	}
	tmpBackgoundType = backgroundType;
}
/****************************************************************************************************/
void createPovrayOptionsWindow(GtkWidget* win)
{
	GtkWidget *dialogWindow = NULL;
	GtkWidget *button;
	GtkWidget *frame;
	GtkWidget *hbox;
	gchar title[BSIZE];
	 
	dialogWindow = gtk_dialog_new();
	gtk_widget_realize(GTK_WIDGET(dialogWindow));
	sprintf(title, _("Povray options"));
	gtk_window_set_title(GTK_WINDOW(dialogWindow),title);

	gtk_window_set_modal (GTK_WINDOW (dialogWindow), TRUE);
	gtk_window_set_position(GTK_WINDOW(dialogWindow),GTK_WIN_POS_CENTER);

	g_signal_connect(G_OBJECT(dialogWindow), "delete_event", (GCallback)destroy_button_windows, NULL);
	g_signal_connect(G_OBJECT(dialogWindow), "delete_event", (GCallback)gtk_widget_destroy, NULL);

	frame = gtk_frame_new (NULL);
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (GTK_WIDGET(GTK_DIALOG(dialogWindow)->vbox)), frame, TRUE, TRUE, 3);

	hbox = gtk_hbox_new (FALSE, 3);
	gtk_widget_show (hbox);
	gtk_container_add (GTK_CONTAINER (frame), hbox);

	createPOVBackgroundFrame(hbox);
	gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(dialogWindow)->action_area), TRUE);

	button = create_button(dialogWindow,"Cancel");
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(dialogWindow)->action_area), button, FALSE, TRUE, 5);	
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)destroy_button_windows, GTK_OBJECT(dialogWindow));
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(dialogWindow));

	button = create_button(dialogWindow,"OK");
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(dialogWindow)->action_area), button, FALSE, TRUE, 5);	
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)applyPovrayOptions, GTK_OBJECT(dialogWindow));
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)destroy_button_windows, GTK_OBJECT(dialogWindow));
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(dialogWindow));
	

	add_button_windows(title,dialogWindow);

	gtk_widget_show_all(dialogWindow);
	if(GTK_IS_WIDGET(win)) gtk_window_set_transient_for(GTK_WINDOW(dialogWindow),GTK_WINDOW(win));
}
/********************************************************************************/
static gchar *get_pov_sky()
{
     gchar *temp;
     temp = g_strdup(
		"//  Sky with white clouds  \n"
		"object \n"
		"{\n"
		"\tsphere { <0, 0, 0> 200000}\n" 
		"\ttexture {Blue_Sky scale <50000, 6000, 50000>}\n" 
		"\tfinish { ambient 0.9 diffuse 0.0 }\n" 
		"}\n"
		);
	 return temp;
}
/********************************************************************************/
static gchar *get_pov_water(gdouble x, gdouble y, gdouble z, gdouble scale)
{
     gchar *temp;
     temp = g_strdup_printf(
		"//  water\n"
		"plane \n"
		"{\n"
		"\t<%f, %f, %f>,%f\n" 
		"\ttexture\n" 
		"\t{\n" 
		"\t\tpigment{ rgb <0.2, 0.2, 0.2> }\n" 
		"\t\tnormal { bumps 0.08 scale <1,0.25,0.35>*1 turbulence 0.6 }\n" 
		"\t\tfinish { ambient 0.05 diffuse 0.55 brilliance 6.0 phong 0.8 phong_size 120 reflection 0.6 }\n" 
		"\t}\n" 
		"}\n",
		x, y, z, scale
		);
	 return temp;
}
/********************************************************************************/
static gchar *get_pov_checker(gdouble x, gdouble y, gdouble z, gdouble scale)
{
     gchar *temp;
     temp = g_strdup_printf(
		"// checker\n"
		"plane \n"
		"{\n"
		"\t<%f, %f, %f>,%f\n" 
		"\thollow on\n" 
		"\tpigment { checker rgb <1,0,0>, rgb <1,1,0> scale 10 }\n" 
		"\tfinish { ambient 0.3 diffuse 0.4 }\n" 
		"}\n",
		x, y, z, scale
		);
	 return temp;
}
/********************************************************************************/
static gchar *get_pov_hexagon(gdouble x, gdouble y, gdouble z, gdouble scale)
{
     gchar *temp;
     temp = g_strdup_printf(
		"// Hexagon\n"
		"plane \n"
		"{\n"
		"\t<%f, %f, %f>,%f\n" 
		"\tpigment { hexagon rgb <0.5,0.5,0.5>, rgb <0,0,0>, rgb <1,1,1> scale 5}\n" 
		"\tfinish { ambient 0.3 diffuse 0.4 }\n" 
		"}\n",
		x, y, z, scale
		);
	 return temp;
}
/********************************************************************************/
static gchar *get_pov_stone(gdouble x, gdouble y, gdouble z, gdouble scale, gint numStone)
{
     gchar *temp;
     temp = g_strdup_printf(
		"//  Stone\n"
		"plane \n"
		"{\n"
		"\t<%f, %f, %f>,%f\n" 
		"\ttexture {T_Stone%d}\n" 
		"\tfinish { ambient 0.5 diffuse 0.4 }\n" 
		"}\n",
		x, y, z, scale, numStone
		);
	 return temp;
}
/********************************************************************************/
static gchar *get_pov_backgrond_unicolor(gdouble red, gdouble green, gdouble blue)
{
     gchar *temp;
     temp = g_strdup_printf(
		"// BACKGROUND \n"
		"background \n"
		"{\n"
		"\tcolor rgb < %f, %f, %f >\n" 
		"}\n",
		red, green, blue
		);
     return temp;
}
/********************************************************************************/
static gchar *get_pov_sky_water(gdouble x, gdouble y, gdouble z, gdouble scale)
{
	gchar *temp = NULL;
	gchar *sky;
	gchar *water;

     	sky = get_pov_sky();
	water = get_pov_water(x, y, z, scale);
	if(sky)
	{
		temp = g_strdup(sky);
		g_free(sky);
	}
	if(water)
	{
		gchar* dump;
		dump =temp;
		temp = g_strdup_printf("%s%s",dump,water);
		g_free(dump);
		g_free(water);
	}
	return temp;
}
/********************************************************************************/
static gchar *get_pov_sky_checker(gdouble x, gdouble y, gdouble z, gdouble scale)
{
	gchar *temp = NULL;
	gchar *sky;
	gchar *checker;

     	sky = get_pov_sky();
	checker = get_pov_checker(x, y, z, scale);
	if(sky)
	{
		temp = g_strdup(sky);
		g_free(sky);
	}
	if(checker)
	{
		gchar* dump;
		dump =temp;
		temp = g_strdup_printf("%s%s",dump,checker);
		g_free(dump);
		g_free(checker);
	}
	return temp;
}
/********************************************************************************/
static gchar *get_pov_cube(gdouble xScale, gdouble yScale, gdouble zScale)
{
	gchar *temp = NULL;
	gchar *bottom;
	gchar *left;
	gchar *ground;

	bottom = get_pov_hexagon(0, 1, 0, yScale);
	if(bottom)
	{
		temp = g_strdup(bottom);
		g_free(bottom);
	}
	left = get_pov_stone(1, 0, 0, xScale*2,21);
	if(left)
	{
		gchar* dump;
		dump =temp;
		temp = g_strdup_printf("%s%s",dump,left);
		g_free(dump);
		g_free(left);
	}
	ground = get_pov_stone(0, 0, 1, zScale*8,21);
	if(ground)
	{
		gchar* dump;
		dump =temp;
		temp = g_strdup_printf("%s%s",dump,ground);
		g_free(dump);
		g_free(ground);
	}
	return temp;
}
/********************************************************************************/
gchar *get_pov_background(gdouble xScale, gdouble yScale, gdouble zScale)
{
	switch(backgroundType)
	{
		case SKY_WATER : return get_pov_sky_water(0,1,0, yScale);
		case SKY_CHEKER : return get_pov_sky_checker(0, 1, 0, yScale);
		case ROOM : return get_pov_cube(xScale, yScale, zScale);
		case  BALCK : return get_pov_backgrond_unicolor(0,0,0);
		case  WHITE :return get_pov_backgrond_unicolor(1,1,1);
		default: return NULL;
	}
}
/********************************************************************************/
