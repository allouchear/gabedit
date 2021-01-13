/* Preferences.c */
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
#include <GL/gl.h>
#include <GL/glu.h>
#include "Global.h"
#include "../Common/Help.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "Windows.h"
#include "TextEdit.h"
#include "../Utils/Vector3d.h"
#include "../Display/PreferencesOrb.h"
#include "../Display/GLArea.h"
#include "../Display/UtilsOrb.h"
#include "../Geometry/ResultsAnalise.h"
#include "../Geometry/GeomGlobal.h"
#include "../Files/FolderChooser.h"
#include "../Files/GabeditFolderChooser.h"
void rafresh_drawing();
void set_color_surface(gint num,gdouble v[]);
#include "../../pixmaps/Open.xpm"
static GtkWidget* entrybabel = NULL;

static GtkWidget *Wins = NULL;
static GtkWidget *EntryDeMon = NULL;
static GtkWidget *EntryGamess = NULL;
static GtkWidget *EntryGaussian = NULL;
static GtkWidget *EntryMolcas = NULL;
static GtkWidget *EntryMolpro = NULL;
static GtkWidget *EntryMopac = NULL;
static GtkWidget *EntryPovray = NULL;
static GtkWidget *EntryMPQC = NULL;
static GtkWidget *EntryFireFly = NULL;
static GtkWidget *EntryQChem = NULL;
static GtkWidget *EntryOrca = NULL;
static GtkWidget *EntryNWChem = NULL;
static GtkWidget *EntryPsicode = NULL;

static GtkWidget *ComboDeMon = NULL;
static GtkWidget *ComboGamess = NULL;
static GtkWidget *ComboGaussian = NULL;
static GtkWidget *ComboMolcas = NULL;
static GtkWidget *ComboMolpro = NULL;
static GtkWidget *ComboMopac = NULL;
static GtkWidget *ComboPovray = NULL;
static GtkWidget *ComboMPQC = NULL;
static GtkWidget *ComboFireFly = NULL;
static GtkWidget *ComboQChem = NULL;
static GtkWidget *ComboOrca = NULL;
static GtkWidget *ComboNWChem = NULL;
static GtkWidget *ComboPsicode = NULL;

static GtkWidget *ButtonDeMon = NULL;
static GtkWidget *ButtonGamess = NULL;
static GtkWidget *ButtonGaussian = NULL;
static GtkWidget *ButtonMolcas = NULL;
static GtkWidget *ButtonMolpro = NULL;
static GtkWidget *ButtonMopac = NULL;
static GtkWidget *ButtonPovray = NULL;
static GtkWidget *ButtonMPQC = NULL;
static GtkWidget *ButtonFireFly = NULL;
static GtkWidget *ButtonQChem = NULL;
static GtkWidget *ButtonOrca = NULL;
static GtkWidget *ButtonNWChem = NULL;
static GtkWidget *ButtonPsicode = NULL;

static GtkWidget *EntryBatchType = NULL;
static GtkWidget *selectors[3];
static GdkColor ColorTemp;
static gchar *FontTemp;
static gboolean instal = FALSE;

/********************************************************************************/
void destroy_preferences_window(GtkWidget*Win)
{
  destroy_children(Wins);
  Wins = NULL;
}
/********************************************************************************/
void  modify_color_surfaces()
{
	gdouble Col[4];
	GdkColor color;

	/* positive value */
	gtk_color_button_get_color ((GtkColorButton*)selectors[0], &color);
	Col[0] = color.red/65535.0;
	Col[1] = color.green/65535.0;
	Col[2] = color.blue/65535.0;
	set_color_surface(0,Col);

	/* negative value */
	gtk_color_button_get_color ((GtkColorButton*)selectors[1], &color);
	Col[0] = color.red/65535.0;
	Col[1] = color.green/65535.0;
	Col[2] = color.blue/65535.0;
	set_color_surface(1,Col);
	/* density surface */
	gtk_color_button_get_color ((GtkColorButton*)selectors[2], &color);
	Col[0] = color.red/65535.0;
	Col[1] = color.green/65535.0;
	Col[2] = color.blue/65535.0;
	set_color_surface(2,Col);
	rafresh_window_orb();
}
/********************************************************************************/
void  create_color_surfaces(GtkWidget *Wins,GtkWidget *Frame)
{
  GtkWidget *vbox;

  vbox = create_vbox(Frame);

  create_colorsel_frame(vbox,NULL,selectors);
}
/********************************************************************************/
static void  remove_gamess_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;
  gint inList = -1;

  if(gamessCommands.numberOfCommands<2)
	  return;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryGamess));

  for(i=0;i<gamessCommands.numberOfCommands;i++)
  {
	  if(strcmp(strcom,gamessCommands.commands[i])==0)
	  {
		  inList = i;
		  break;
	  }
  }
  if(inList == -1)
	  return;
  for(i=inList;i<gamessCommands.numberOfCommands-1;i++)
	  gamessCommands.commands[i] = gamessCommands.commands[i+1];

  gamessCommands.numberOfCommands--;
  gamessCommands.commands = g_realloc(
		   gamessCommands.commands,
		   gamessCommands.numberOfCommands*sizeof(gchar*));

  for(i=0;i<gamessCommands.numberOfCommands;i++)
	glist = g_list_append(glist,gamessCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboGamess, glist) ;

  g_list_free(glist);

  if(gamessCommands.numberOfCommands<2) gtk_widget_set_sensitive(ButtonGamess, FALSE);
  else gtk_widget_set_sensitive(ButtonGamess, TRUE);

  NameCommandGamess = g_strdup(gamessCommands.commands[0]);

  str_delete_n(NameCommandGamess);
  delete_last_spaces(NameCommandGamess);
  delete_first_spaces(NameCommandGamess);
}
/********************************************************************************/
void  modify_gamess_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryGamess));
  if(strcmp(strcom,""))
      NameCommandGamess = g_strdup(strcom);

  str_delete_n(NameCommandGamess);
  delete_last_spaces(NameCommandGamess);
  delete_first_spaces(NameCommandGamess);

  for(i=0;i<gamessCommands.numberOfCommands;i++)
  {
	  if(strcmp(NameCommandGamess,gamessCommands.commands[i])==0)
		  return;
  }
  gamessCommands.numberOfCommands++;
  gamessCommands.commands = g_realloc(
		   gamessCommands.commands,
		   gamessCommands.numberOfCommands*sizeof(gchar*));
  gamessCommands.commands[gamessCommands.numberOfCommands-1] = g_strdup(NameCommandGamess);

  for(i=gamessCommands.numberOfCommands-1;i>=0;i--)
	glist = g_list_append(glist,gamessCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboGamess, glist) ;

  g_list_free(glist);
  if(gamessCommands.numberOfCommands<2) gtk_widget_set_sensitive(ButtonGamess, FALSE);
  else gtk_widget_set_sensitive(ButtonGamess, TRUE);
}
/********************************************************************************/
static void  remove_gaussian_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;
  gint inList = -1;

  if(gaussianCommands.numberOfCommands<2)
	  return;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryGaussian));

  for(i=0;i<gaussianCommands.numberOfCommands;i++)
  {
	  if(strcmp(strcom,gaussianCommands.commands[i])==0)
	  {
		  inList = i;
		  break;
	  }
  }
  if(inList == -1)
	  return;
  for(i=inList;i<gaussianCommands.numberOfCommands-1;i++)
	  gaussianCommands.commands[i] = gaussianCommands.commands[i+1];

  gaussianCommands.numberOfCommands--;
  gaussianCommands.commands = g_realloc(
		   gaussianCommands.commands,
		   gaussianCommands.numberOfCommands*sizeof(gchar*));

  for(i=0;i<gaussianCommands.numberOfCommands;i++)
	glist = g_list_append(glist,gaussianCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboGaussian, glist) ;

  g_list_free(glist);

  if(gaussianCommands.numberOfCommands<2) gtk_widget_set_sensitive(ButtonGaussian, FALSE);
  else gtk_widget_set_sensitive(ButtonGaussian, TRUE);

  NameCommandGaussian = g_strdup(gaussianCommands.commands[0]);

  str_delete_n(NameCommandGaussian);
  delete_last_spaces(NameCommandGaussian);
  delete_first_spaces(NameCommandGaussian);
}
/********************************************************************************/
void  modify_gaussian_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryGaussian));
  if(strcmp(strcom,""))
      NameCommandGaussian = g_strdup(strcom);

  str_delete_n(NameCommandGaussian);
  delete_last_spaces(NameCommandGaussian);
  delete_first_spaces(NameCommandGaussian);

  for(i=0;i<gaussianCommands.numberOfCommands;i++)
  {
	  if(strcmp(NameCommandGaussian,gaussianCommands.commands[i])==0)
		  return;
  }
  gaussianCommands.numberOfCommands++;
  gaussianCommands.commands = g_realloc(
		   gaussianCommands.commands,
		   gaussianCommands.numberOfCommands*sizeof(gchar*));
  gaussianCommands.commands[gaussianCommands.numberOfCommands-1] = g_strdup(NameCommandGaussian);

  for(i=gaussianCommands.numberOfCommands-1;i>=0;i--)
	glist = g_list_append(glist,gaussianCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboGaussian, glist) ;

  g_list_free(glist);
  if(gaussianCommands.numberOfCommands<2) gtk_widget_set_sensitive(ButtonGaussian, FALSE);
  else gtk_widget_set_sensitive(ButtonGaussian, TRUE);
}
/********************************************************************************/
static void  remove_molcas_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;
  gint inList = -1;

  if(molcasCommands.numberOfCommands<2)
	  return;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryMolcas));

  for(i=0;i<molcasCommands.numberOfCommands;i++)
  {
	  if(strcmp(strcom,molcasCommands.commands[i])==0)
	  {
		  inList = i;
		  break;
	  }
  }
  if(inList == -1)
	  return;
  for(i=inList;i<molcasCommands.numberOfCommands-1;i++)
	  molcasCommands.commands[i] = molcasCommands.commands[i+1];

  molcasCommands.numberOfCommands--;
  molcasCommands.commands = g_realloc(
		   molcasCommands.commands,
		   molcasCommands.numberOfCommands*sizeof(gchar*));

  for(i=0;i<molcasCommands.numberOfCommands;i++)
	glist = g_list_append(glist,molcasCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboMolcas, glist) ;

  g_list_free(glist);

  if(molcasCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonMolcas, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonMolcas, TRUE);

  NameCommandMolcas = g_strdup(molcasCommands.commands[0]);

  str_delete_n(NameCommandMolcas);
  delete_last_spaces(NameCommandMolcas);
  delete_first_spaces(NameCommandMolcas);
}
/********************************************************************************/
void  modify_molcas_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryMolcas));
  if(strcmp(strcom,""))
      NameCommandMolcas = g_strdup(strcom);

  str_delete_n(NameCommandMolcas);
  delete_last_spaces(NameCommandMolcas);
  delete_first_spaces(NameCommandMolcas);

  for(i=0;i<molcasCommands.numberOfCommands;i++)
  {
	  if(strcmp(NameCommandMolcas,molcasCommands.commands[i])==0)
		  return;
  }
  molcasCommands.numberOfCommands++;
  molcasCommands.commands = g_realloc(
		   molcasCommands.commands,
		   molcasCommands.numberOfCommands*sizeof(gchar*));
  molcasCommands.commands[molcasCommands.numberOfCommands-1] = g_strdup(NameCommandMolcas);

  for(i=molcasCommands.numberOfCommands-1;i>=0;i--)
	glist = g_list_append(glist,molcasCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboMolcas, glist) ;

  g_list_free(glist);
  if(molcasCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonMolcas, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonMolcas, TRUE);
}
/********************************************************************************/
static void  remove_mpqc_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;
  gint inList = -1;

  if(mpqcCommands.numberOfCommands<2)
	  return;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryMPQC));

  for(i=0;i<mpqcCommands.numberOfCommands;i++)
  {
	  if(strcmp(strcom,mpqcCommands.commands[i])==0)
	  {
		  inList = i;
		  break;
	  }
  }
  if(inList == -1)
	  return;
  for(i=inList;i<mpqcCommands.numberOfCommands-1;i++)
	  mpqcCommands.commands[i] = mpqcCommands.commands[i+1];

  mpqcCommands.numberOfCommands--;
  mpqcCommands.commands = g_realloc(
		   mpqcCommands.commands,
		   mpqcCommands.numberOfCommands*sizeof(gchar*));

  for(i=0;i<mpqcCommands.numberOfCommands;i++)
	glist = g_list_append(glist,mpqcCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboMPQC, glist) ;

  g_list_free(glist);

  if(mpqcCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonMPQC, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonMPQC, TRUE);

  NameCommandMPQC = g_strdup(mpqcCommands.commands[0]);

  str_delete_n(NameCommandMPQC);
  delete_last_spaces(NameCommandMPQC);
  delete_first_spaces(NameCommandMPQC);
}
/********************************************************************************/
void  modify_mpqc_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryMPQC));
  if(strcmp(strcom,""))
      NameCommandMPQC = g_strdup(strcom);

  str_delete_n(NameCommandMPQC);
  delete_last_spaces(NameCommandMPQC);
  delete_first_spaces(NameCommandMPQC);

  for(i=0;i<mpqcCommands.numberOfCommands;i++)
  {
	  if(strcmp(NameCommandMPQC,mpqcCommands.commands[i])==0)
		  return;
  }
  mpqcCommands.numberOfCommands++;
  mpqcCommands.commands = g_realloc(
		   mpqcCommands.commands,
		   mpqcCommands.numberOfCommands*sizeof(gchar*));
  mpqcCommands.commands[mpqcCommands.numberOfCommands-1] = g_strdup(NameCommandMPQC);

  for(i=mpqcCommands.numberOfCommands-1;i>=0;i--)
	glist = g_list_append(glist,mpqcCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboMPQC, glist) ;

  g_list_free(glist);
  if(mpqcCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonMPQC, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonMPQC, TRUE);
}
/********************************************************************************/
static void  remove_molpro_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;
  gint inList = -1;

  if(molproCommands.numberOfCommands<2)
	  return;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryMolpro));

  for(i=0;i<molproCommands.numberOfCommands;i++)
  {
	  if(strcmp(strcom,molproCommands.commands[i])==0)
	  {
		  inList = i;
		  break;
	  }
  }
  if(inList == -1)
	  return;
  for(i=inList;i<molproCommands.numberOfCommands-1;i++)
	  molproCommands.commands[i] = molproCommands.commands[i+1];

  molproCommands.numberOfCommands--;
  molproCommands.commands = g_realloc(
		   molproCommands.commands,
		   molproCommands.numberOfCommands*sizeof(gchar*));

  for(i=0;i<molproCommands.numberOfCommands;i++)
	glist = g_list_append(glist,molproCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboMolpro, glist) ;

  g_list_free(glist);

  if(molproCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonMolpro, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonMolpro, TRUE);

  NameCommandMolpro = g_strdup(molproCommands.commands[0]);

  str_delete_n(NameCommandMolpro);
  delete_last_spaces(NameCommandMolpro);
  delete_first_spaces(NameCommandMolpro);
}
/********************************************************************************/
void  modify_molpro_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryMolpro));
  if(strcmp(strcom,""))
      NameCommandMolpro = g_strdup(strcom);

  str_delete_n(NameCommandMolpro);
  delete_last_spaces(NameCommandMolpro);
  delete_first_spaces(NameCommandMolpro);

  for(i=0;i<molproCommands.numberOfCommands;i++)
  {
	  if(strcmp(NameCommandMolpro,molproCommands.commands[i])==0)
		  return;
  }
  molproCommands.numberOfCommands++;
  molproCommands.commands = g_realloc(
		   molproCommands.commands,
		   molproCommands.numberOfCommands*sizeof(gchar*));
  molproCommands.commands[molproCommands.numberOfCommands-1] = g_strdup(NameCommandMolpro);

  for(i=molproCommands.numberOfCommands-1;i>=0;i--)
	glist = g_list_append(glist,molproCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboMolpro, glist) ;

  g_list_free(glist);
  if(molproCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonMolpro, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonMolpro, TRUE);
}
/********************************************************************************/
static void  remove_demon_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;
  gint inList = -1;

  if(demonCommands.numberOfCommands<2)
	  return;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryDeMon));

  for(i=0;i<demonCommands.numberOfCommands;i++)
  {
	  if(strcmp(strcom,demonCommands.commands[i])==0)
	  {
		  inList = i;
		  break;
	  }
  }
  if(inList == -1)
	  return;
  for(i=inList;i<demonCommands.numberOfCommands-1;i++)
	  demonCommands.commands[i] = demonCommands.commands[i+1];

  demonCommands.numberOfCommands--;
  demonCommands.commands = g_realloc(
		   demonCommands.commands,
		   demonCommands.numberOfCommands*sizeof(gchar*));

  for(i=0;i<demonCommands.numberOfCommands;i++)
	glist = g_list_append(glist,demonCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboDeMon, glist) ;

  g_list_free(glist);

  if(demonCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonDeMon, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonDeMon, TRUE);

  NameCommandDeMon = g_strdup(demonCommands.commands[0]);

  str_delete_n(NameCommandDeMon);
  delete_last_spaces(NameCommandDeMon);
  delete_first_spaces(NameCommandDeMon);
}
/********************************************************************************/
void  modify_demon_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryDeMon));
  if(strcmp(strcom,""))
      NameCommandDeMon = g_strdup(strcom);

  str_delete_n(NameCommandDeMon);
  delete_last_spaces(NameCommandDeMon);
  delete_first_spaces(NameCommandDeMon);

  for(i=0;i<demonCommands.numberOfCommands;i++)
  {
	  if(strcmp(NameCommandDeMon,demonCommands.commands[i])==0)
		  return;
  }
  demonCommands.numberOfCommands++;
  demonCommands.commands = g_realloc(
		   demonCommands.commands,
		   demonCommands.numberOfCommands*sizeof(gchar*));
  demonCommands.commands[demonCommands.numberOfCommands-1] = g_strdup(NameCommandDeMon);

  for(i=demonCommands.numberOfCommands-1;i>=0;i--)
	glist = g_list_append(glist,demonCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboDeMon, glist) ;

  g_list_free(glist);
  if(demonCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonDeMon, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonDeMon, TRUE);
}
/********************************************************************************/
/********************************************************************************/
static void  remove_firefly_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;
  gint inList = -1;

  if(fireflyCommands.numberOfCommands<2)
	  return;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryFireFly));

  for(i=0;i<fireflyCommands.numberOfCommands;i++)
  {
	  if(strcmp(strcom,fireflyCommands.commands[i])==0)
	  {
		  inList = i;
		  break;
	  }
  }
  if(inList == -1)
	  return;
  for(i=inList;i<fireflyCommands.numberOfCommands-1;i++)
	  fireflyCommands.commands[i] = fireflyCommands.commands[i+1];

  fireflyCommands.numberOfCommands--;
  fireflyCommands.commands = g_realloc(
		   fireflyCommands.commands,
		   fireflyCommands.numberOfCommands*sizeof(gchar*));

  for(i=0;i<fireflyCommands.numberOfCommands;i++)
	glist = g_list_append(glist,fireflyCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboFireFly, glist) ;

  g_list_free(glist);

  if(fireflyCommands.numberOfCommands<2) gtk_widget_set_sensitive(ButtonFireFly, FALSE);
  else gtk_widget_set_sensitive(ButtonFireFly, TRUE);

  NameCommandFireFly = g_strdup(fireflyCommands.commands[0]);

  str_delete_n(NameCommandFireFly);
  delete_last_spaces(NameCommandFireFly);
  delete_first_spaces(NameCommandFireFly);
}
/********************************************************************************/
void  modify_firefly_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryFireFly));
  if(strcmp(strcom,""))
      NameCommandFireFly = g_strdup(strcom);

  str_delete_n(NameCommandFireFly);
  delete_last_spaces(NameCommandFireFly);
  delete_first_spaces(NameCommandFireFly);

  for(i=0;i<fireflyCommands.numberOfCommands;i++)
  {
	  if(strcmp(NameCommandFireFly,fireflyCommands.commands[i])==0)
		  return;
  }
  fireflyCommands.numberOfCommands++;
  fireflyCommands.commands = g_realloc(
		   fireflyCommands.commands,
		   fireflyCommands.numberOfCommands*sizeof(gchar*));
  fireflyCommands.commands[fireflyCommands.numberOfCommands-1] = g_strdup(NameCommandFireFly);

  for(i=fireflyCommands.numberOfCommands-1;i>=0;i--)
	glist = g_list_append(glist,fireflyCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboFireFly, glist) ;

  g_list_free(glist);
  if(fireflyCommands.numberOfCommands<2) gtk_widget_set_sensitive(ButtonFireFly, FALSE);
  else gtk_widget_set_sensitive(ButtonFireFly, TRUE);
}
/********************************************************************************/
static void  remove_mopac_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;
  gint inList = -1;

  if(mopacCommands.numberOfCommands<2)
	  return;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryMopac));

  for(i=0;i<mopacCommands.numberOfCommands;i++)
  {
	  if(strcmp(strcom,mopacCommands.commands[i])==0)
	  {
		  inList = i;
		  break;
	  }
  }
  if(inList == -1)
	  return;
  for(i=inList;i<mopacCommands.numberOfCommands-1;i++)
	  mopacCommands.commands[i] = mopacCommands.commands[i+1];

  mopacCommands.numberOfCommands--;
  mopacCommands.commands = g_realloc(
		   mopacCommands.commands,
		   mopacCommands.numberOfCommands*sizeof(gchar*));

  for(i=0;i<mopacCommands.numberOfCommands;i++)
	glist = g_list_append(glist,mopacCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboMopac, glist) ;

  g_list_free(glist);

  if(mopacCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonMopac, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonMopac, TRUE);

  NameCommandMopac = g_strdup(mopacCommands.commands[0]);

  str_delete_n(NameCommandMopac);
  delete_last_spaces(NameCommandMopac);
  delete_first_spaces(NameCommandMopac);
}
/********************************************************************************/
void  modify_mopac_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryMopac));
  if(strcmp(strcom,""))
      NameCommandMopac = g_strdup(strcom);

  str_delete_n(NameCommandMopac);
  delete_last_spaces(NameCommandMopac);
  delete_first_spaces(NameCommandMopac);

  for(i=0;i<mopacCommands.numberOfCommands;i++)
  {
	  if(strcmp(NameCommandMopac,mopacCommands.commands[i])==0)
		  return;
  }
  mopacCommands.numberOfCommands++;
  mopacCommands.commands = g_realloc(
		   mopacCommands.commands,
		   mopacCommands.numberOfCommands*sizeof(gchar*));
  mopacCommands.commands[mopacCommands.numberOfCommands-1] = g_strdup(NameCommandMopac);

  for(i=mopacCommands.numberOfCommands-1;i>=0;i--)
	glist = g_list_append(glist,mopacCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboMopac, glist) ;

  g_list_free(glist);
  if(mopacCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonMopac, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonMopac, TRUE);
}
/********************************************************************************/
static void  remove_qchem_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;
  gint inList = -1;

  if(qchemCommands.numberOfCommands<2)
	  return;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryQChem));

  for(i=0;i<qchemCommands.numberOfCommands;i++)
  {
	  if(strcmp(strcom,qchemCommands.commands[i])==0)
	  {
		  inList = i;
		  break;
	  }
  }
  if(inList == -1)
	  return;
  for(i=inList;i<qchemCommands.numberOfCommands-1;i++)
	  qchemCommands.commands[i] = qchemCommands.commands[i+1];

  qchemCommands.numberOfCommands--;
  qchemCommands.commands = g_realloc(
		   qchemCommands.commands,
		   qchemCommands.numberOfCommands*sizeof(gchar*));

  for(i=0;i<qchemCommands.numberOfCommands;i++)
	glist = g_list_append(glist,qchemCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboQChem, glist) ;

  g_list_free(glist);

  if(qchemCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonQChem, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonQChem, TRUE);

  NameCommandQChem = g_strdup(qchemCommands.commands[0]);

  str_delete_n(NameCommandQChem);
  delete_last_spaces(NameCommandQChem);
  delete_first_spaces(NameCommandQChem);
}
/********************************************************************************/
void  modify_qchem_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryQChem));
  if(strcmp(strcom,""))
      NameCommandQChem = g_strdup(strcom);

  str_delete_n(NameCommandQChem);
  delete_last_spaces(NameCommandQChem);
  delete_first_spaces(NameCommandQChem);

  for(i=0;i<qchemCommands.numberOfCommands;i++)
  {
	  if(strcmp(NameCommandQChem,qchemCommands.commands[i])==0)
		  return;
  }
  qchemCommands.numberOfCommands++;
  qchemCommands.commands = g_realloc(
		   qchemCommands.commands,
		   qchemCommands.numberOfCommands*sizeof(gchar*));
  qchemCommands.commands[qchemCommands.numberOfCommands-1] = g_strdup(NameCommandQChem);

  for(i=qchemCommands.numberOfCommands-1;i>=0;i--)
	glist = g_list_append(glist,qchemCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboQChem, glist) ;

  g_list_free(glist);
  if(qchemCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonQChem, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonQChem, TRUE);
}
/********************************************************************************/
static void  remove_orca_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;
  gint inList = -1;

  if(orcaCommands.numberOfCommands<2)
	  return;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryOrca));

  for(i=0;i<orcaCommands.numberOfCommands;i++)
  {
	  if(strcmp(strcom,orcaCommands.commands[i])==0)
	  {
		  inList = i;
		  break;
	  }
  }
  if(inList == -1)
	  return;
  for(i=inList;i<orcaCommands.numberOfCommands-1;i++)
	  orcaCommands.commands[i] = orcaCommands.commands[i+1];

  orcaCommands.numberOfCommands--;
  orcaCommands.commands = g_realloc(
		   orcaCommands.commands,
		   orcaCommands.numberOfCommands*sizeof(gchar*));

  for(i=0;i<orcaCommands.numberOfCommands;i++)
	glist = g_list_append(glist,orcaCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboOrca, glist) ;

  g_list_free(glist);

  if(orcaCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonOrca, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonOrca, TRUE);

  NameCommandOrca = g_strdup(orcaCommands.commands[0]);

  str_delete_n(NameCommandOrca);
  delete_last_spaces(NameCommandOrca);
  delete_first_spaces(NameCommandOrca);
}
/********************************************************************************/
void  modify_orca_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryOrca));
  if(strcmp(strcom,""))
      NameCommandOrca = g_strdup(strcom);

  str_delete_n(NameCommandOrca);
  delete_last_spaces(NameCommandOrca);
  delete_first_spaces(NameCommandOrca);

  for(i=0;i<orcaCommands.numberOfCommands;i++)
  {
	  if(strcmp(NameCommandOrca,orcaCommands.commands[i])==0)
		  return;
  }
  orcaCommands.numberOfCommands++;
  orcaCommands.commands = g_realloc(
		   orcaCommands.commands,
		   orcaCommands.numberOfCommands*sizeof(gchar*));
  orcaCommands.commands[orcaCommands.numberOfCommands-1] = g_strdup(NameCommandOrca);

  for(i=orcaCommands.numberOfCommands-1;i>=0;i--)
	glist = g_list_append(glist,orcaCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboOrca, glist) ;

  g_list_free(glist);
  if(orcaCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonOrca, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonOrca, TRUE);
}
/********************************************************************************/
static void  remove_nwchem_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;
  gint inList = -1;

  if(nwchemCommands.numberOfCommands<2)
	  return;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryNWChem));

  for(i=0;i<nwchemCommands.numberOfCommands;i++)
  {
	  if(strcmp(strcom,nwchemCommands.commands[i])==0)
	  {
		  inList = i;
		  break;
	  }
  }
  if(inList == -1)
	  return;
  for(i=inList;i<nwchemCommands.numberOfCommands-1;i++)
	  nwchemCommands.commands[i] = nwchemCommands.commands[i+1];

  nwchemCommands.numberOfCommands--;
  nwchemCommands.commands = g_realloc(
		   nwchemCommands.commands,
		   nwchemCommands.numberOfCommands*sizeof(gchar*));

  for(i=0;i<nwchemCommands.numberOfCommands;i++)
	glist = g_list_append(glist,nwchemCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboNWChem, glist) ;

  g_list_free(glist);

  if(nwchemCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonNWChem, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonNWChem, TRUE);

  NameCommandNWChem = g_strdup(nwchemCommands.commands[0]);

  str_delete_n(NameCommandNWChem);
  delete_last_spaces(NameCommandNWChem);
  delete_first_spaces(NameCommandNWChem);
}
/********************************************************************************/
void  modify_nwchem_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryNWChem));
  if(strcmp(strcom,""))
      NameCommandNWChem = g_strdup(strcom);

  str_delete_n(NameCommandNWChem);
  delete_last_spaces(NameCommandNWChem);
  delete_first_spaces(NameCommandNWChem);

  for(i=0;i<nwchemCommands.numberOfCommands;i++)
  {
	  if(strcmp(NameCommandNWChem,nwchemCommands.commands[i])==0)
		  return;
  }
  nwchemCommands.numberOfCommands++;
  nwchemCommands.commands = g_realloc(
		   nwchemCommands.commands,
		   nwchemCommands.numberOfCommands*sizeof(gchar*));
  nwchemCommands.commands[nwchemCommands.numberOfCommands-1] = g_strdup(NameCommandNWChem);

  for(i=nwchemCommands.numberOfCommands-1;i>=0;i--)
	glist = g_list_append(glist,nwchemCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboNWChem, glist) ;

  g_list_free(glist);
  if(nwchemCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonNWChem, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonNWChem, TRUE);
}
/********************************************************************************/
static void  remove_psicode_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;
  gint inList = -1;

  if(psicodeCommands.numberOfCommands<2)
	  return;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryPsicode));

  for(i=0;i<psicodeCommands.numberOfCommands;i++)
  {
	  if(strcmp(strcom,psicodeCommands.commands[i])==0)
	  {
		  inList = i;
		  break;
	  }
  }
  if(inList == -1)
	  return;
  for(i=inList;i<psicodeCommands.numberOfCommands-1;i++)
	  psicodeCommands.commands[i] = psicodeCommands.commands[i+1];

  psicodeCommands.numberOfCommands--;
  psicodeCommands.commands = g_realloc(
		   psicodeCommands.commands,
		   psicodeCommands.numberOfCommands*sizeof(gchar*));

  for(i=0;i<psicodeCommands.numberOfCommands;i++)
	glist = g_list_append(glist,psicodeCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboPsicode, glist) ;

  g_list_free(glist);

  if(psicodeCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonPsicode, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonPsicode, TRUE);

  NameCommandPsicode = g_strdup(psicodeCommands.commands[0]);

  str_delete_n(NameCommandPsicode);
  delete_last_spaces(NameCommandPsicode);
  delete_first_spaces(NameCommandPsicode);
}
/********************************************************************************/
void  modify_psicode_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryPsicode));
  if(strcmp(strcom,""))
      NameCommandPsicode = g_strdup(strcom);

  str_delete_n(NameCommandPsicode);
  delete_last_spaces(NameCommandPsicode);
  delete_first_spaces(NameCommandPsicode);

  for(i=0;i<psicodeCommands.numberOfCommands;i++)
  {
	  if(strcmp(NameCommandPsicode,psicodeCommands.commands[i])==0)
		  return;
  }
  psicodeCommands.numberOfCommands++;
  psicodeCommands.commands = g_realloc(
		   psicodeCommands.commands,
		   psicodeCommands.numberOfCommands*sizeof(gchar*));
  psicodeCommands.commands[psicodeCommands.numberOfCommands-1] = g_strdup(NameCommandPsicode);

  for(i=psicodeCommands.numberOfCommands-1;i>=0;i--)
	glist = g_list_append(glist,psicodeCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboPsicode, glist) ;

  g_list_free(glist);
  if(psicodeCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonPsicode, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonPsicode, TRUE);
}
/********************************************************************************/
static void  remove_povray_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;
  gint inList = -1;

  if(povrayCommands.numberOfCommands<2)
	  return;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryPovray));

  for(i=0;i<povrayCommands.numberOfCommands;i++)
  {
	  if(strcmp(strcom,povrayCommands.commands[i])==0)
	  {
		  inList = i;
		  break;
	  }
  }
  if(inList == -1)
	  return;
  for(i=inList;i<povrayCommands.numberOfCommands-1;i++)
	  povrayCommands.commands[i] = povrayCommands.commands[i+1];

  povrayCommands.numberOfCommands--;
  povrayCommands.commands = g_realloc(
		   povrayCommands.commands,
		   povrayCommands.numberOfCommands*sizeof(gchar*));

  for(i=0;i<povrayCommands.numberOfCommands;i++)
	glist = g_list_append(glist,povrayCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboPovray, glist) ;

  g_list_free(glist);

  if(povrayCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonPovray, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonPovray, TRUE);

  NameCommandPovray = g_strdup(povrayCommands.commands[0]);

  str_delete_n(NameCommandPovray);
  delete_last_spaces(NameCommandPovray);
  delete_first_spaces(NameCommandPovray);
}
/********************************************************************************/
void  modify_povray_command()
{
  G_CONST_RETURN gchar *strcom;
  GList *glist = NULL;
  gint i;

  strcom = gtk_entry_get_text (GTK_ENTRY (EntryPovray));
  if(strcmp(strcom,""))
      NameCommandPovray = g_strdup(strcom);

  str_delete_n(NameCommandPovray);
  delete_last_spaces(NameCommandPovray);
  delete_first_spaces(NameCommandPovray);

  for(i=0;i<povrayCommands.numberOfCommands;i++)
  {
	  if(strcmp(NameCommandPovray,povrayCommands.commands[i])==0)
		  return;
  }
  povrayCommands.numberOfCommands++;
  povrayCommands.commands = g_realloc(
		   povrayCommands.commands,
		   povrayCommands.numberOfCommands*sizeof(gchar*));
  povrayCommands.commands[povrayCommands.numberOfCommands-1] = g_strdup(NameCommandPovray);

  for(i=povrayCommands.numberOfCommands-1;i>=0;i--)
	glist = g_list_append(glist,povrayCommands.commands[i]);

  gtk_combo_box_entry_set_popdown_strings( ComboPovray, glist) ;

  g_list_free(glist);
  if(povrayCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(ButtonPovray, FALSE);
  else
  	gtk_widget_set_sensitive(ButtonPovray, TRUE);
}
/********************************************************************************/
void apply_all()
{
  modify_gaussian_command();
  modify_molpro_command();
  modify_demon_command();
  modify_color_surfaces();
}
/********************************************************************************/
void set_temp_font(GtkFontSelectionDialog *Sel,gpointer *d)
{
        FontTemp =  gtk_font_selection_dialog_get_font_name(Sel);
}
/********************************************************************************/
void set_font_other (gchar *fontname)
{
        GtkStyle *style;
	PangoFontDescription *font_desc = NULL;
 
        style = gtk_widget_get_default_style ();
  	font_desc = pango_font_description_from_string (fontname);
 
	if (font_desc)
        {
    		style->font_desc = font_desc;
		/*
		gtk_widget_set_default_style(style);
		*/
        }
	else
	{
		gchar* temp= NULL;
                temp = g_strdup_printf(N_("Unknown font,\n%s\nPlease select a other\n"),fontname);
		Message(temp,"ERROR",TRUE);
		g_free(temp);
	}
 
}
/********************************************************************************/
static void set_button_font(GtkWidget *button,gpointer *data)
{
  GtkWidget *hbox;
  GtkWidget *label;
  gchar *Type = g_object_get_data(G_OBJECT (button), "Type");

  gtk_widget_hide (GTK_WIDGET(button));
  hbox = g_object_get_data(G_OBJECT (button), "Hbox");  
  gtk_widget_destroy(GTK_WIDGET(hbox));

  hbox = gtk_hbox_new (TRUE, 0);
  label = gtk_label_new(FontTemp); 
  gtk_container_add(GTK_CONTAINER(hbox),label);

  gtk_container_add(GTK_CONTAINER(button),hbox);
  g_object_set_data(G_OBJECT (button), "Hbox", hbox);
  gtk_widget_show_all (GTK_WIDGET(button));

  if(strstr(Type,"Data"))
  {
	set_font (text, FontTemp);
        if( FontsStyleData.fontname )
            g_free(FontsStyleData.fontname);
        FontsStyleData.fontname = g_strdup(FontTemp);
  }
  else 
  if(strstr(Type,"Result"))
  {
	set_font (textresult, FontTemp);
	set_font (TextOutput, FontTemp);
	set_font (TextError, FontTemp);
       	if( FontsStyleResult.fontname )
       		g_free(FontsStyleResult.fontname);
       	FontsStyleResult.fontname = g_strdup(FontTemp);
  }
  else 
  if(strstr(Type,"Label"))
  {
       	if( FontsStyleLabel.fontname )
       		g_free(FontsStyleLabel.fontname);
       	FontsStyleLabel.fontname = g_strdup(FontTemp);
 	if(GeomDrawingArea)
  		rafresh_drawing();
  }
  else
  {
	set_font_other (FontTemp);
       	if( FontsStyleOther.fontname )
       		g_free(FontsStyleOther.fontname);
       	FontsStyleOther.fontname = g_strdup(FontTemp);
  }

}
/********************************************************************************/
static void open_font_dlg(GtkWidget *button,gpointer tdata)
{

	GtkFontSelectionDialog *FontDlg;
	FontDlg = (GtkFontSelectionDialog *)gtk_font_selection_dialog_new(_("Font selection"));
        gtk_window_set_position(GTK_WINDOW(FontDlg),GTK_WIN_POS_CENTER);

	if(!instal)
	{
		add_child(Wins, GTK_WIDGET(FontDlg), gtk_widget_destroy, _(" Font selection "));
		g_signal_connect(G_OBJECT(FontDlg),"delete_event",(GCallback)delete_child,NULL);
	}
	else
	{
		gtk_window_set_modal (GTK_WINDOW (FontDlg), TRUE);
		gtk_window_set_transient_for(GTK_WINDOW(FontDlg),GTK_WINDOW(Wins));
	}

  	gtk_font_selection_dialog_set_font_name    ((GtkFontSelectionDialog *)FontDlg,(gchar*)tdata);

/*  	gtk_widget_hide(FontDlg->help_button);*/
	g_signal_connect_swapped(GTK_OBJECT(FontDlg->ok_button),"clicked", (GCallback)set_temp_font,GTK_OBJECT(FontDlg));

	g_signal_connect_swapped(GTK_OBJECT(FontDlg->ok_button),"clicked", (GCallback)set_button_font,GTK_OBJECT(button));

	if(!instal)
	{
 		g_signal_connect_swapped(GTK_OBJECT(FontDlg->ok_button),"clicked",G_CALLBACK(delete_child),GTK_OBJECT(FontDlg)); 
 		g_signal_connect_swapped(GTK_OBJECT(FontDlg->cancel_button),"clicked",G_CALLBACK(delete_child),GTK_OBJECT(FontDlg)); 
	}
	else
	{
 		g_signal_connect_swapped(GTK_OBJECT(FontDlg->ok_button),"clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(FontDlg)); 
 		g_signal_connect_swapped(GTK_OBJECT(FontDlg->cancel_button),"clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(FontDlg)); 
	}
	gtk_widget_show(GTK_WIDGET(FontDlg));

}
/********************************************************************************/
void set_temp_color(GtkColorSelection *Sel,gpointer *d)
{
	gtk_color_selection_get_current_color(Sel, &ColorTemp);
}
/********************************************************************************/
static void set_button_color(GtkObject *b,gpointer *data)
{
  GtkWidget *hbox;
  GtkWidget *button = GTK_WIDGET(b);
  gchar *Type = g_object_get_data(G_OBJECT (button), "Type");

  gtk_widget_hide (GTK_WIDGET(button));
  hbox = g_object_get_data(G_OBJECT (button), "Hbox");  
  gtk_widget_destroy(GTK_WIDGET(hbox));

  hbox = create_hbox_pixmap_color(Wins,ColorTemp.red,ColorTemp.green,ColorTemp.blue);

  gtk_container_add(GTK_CONTAINER(button),hbox);
  g_object_set_data(G_OBJECT (button), "Hbox", hbox);
  gtk_widget_show (hbox);
  gtk_widget_show_all (GTK_WIDGET(button));
  if(strstr(Type,"Data _Back"))
  {
  	set_base_style(text,ColorTemp.red,ColorTemp.green,ColorTemp.blue);
        FontsStyleData.BaseColor  = ColorTemp;
  }
  else if(strstr(Type,"Data _Fore"))
  {
  	set_text_style(text,ColorTemp.red,ColorTemp.green,ColorTemp.blue);
        FontsStyleData.TextColor  = ColorTemp;
  }
  if(strstr(Type,"Result _Back"))
  {
  	set_base_style(textresult,ColorTemp.red,ColorTemp.green,ColorTemp.blue);
  	set_base_style(TextOutput,ColorTemp.red,ColorTemp.green,ColorTemp.blue);
  	set_base_style(TextError,ColorTemp.red,ColorTemp.green,ColorTemp.blue);
        FontsStyleResult.BaseColor  = ColorTemp;
  }
  else if(strstr(Type,"Result _Fore"))
  {
  	set_text_style(textresult,ColorTemp.red,ColorTemp.green,ColorTemp.blue);
  	set_text_style(TextOutput,ColorTemp.red,ColorTemp.green,ColorTemp.blue);
  	set_text_style(TextError,ColorTemp.red,ColorTemp.green,ColorTemp.blue);
        FontsStyleResult.TextColor  = ColorTemp;
  }
  else if(strstr(Type,"Label_Fore"))
  {
        FontsStyleLabel.TextColor  = ColorTemp;
 	if(GeomDrawingArea)
  		rafresh_drawing();
  }
}
/********************************************************************************/
static void open_color_dlg(GtkWidget *button,gpointer tcolor)
{

	GtkColorSelectionDialog *ColorDlg;
	ColorDlg = (GtkColorSelectionDialog *)gtk_color_selection_dialog_new(_("Set Atom Color"));
	if(tcolor)
	{
		GdkColor* color = (GdkColor*)tcolor;
		gtk_color_selection_set_current_color (GTK_COLOR_SELECTION (ColorDlg->colorsel), color);
	}
        gtk_window_set_position(GTK_WINDOW(ColorDlg),GTK_WIN_POS_CENTER);

	if(!instal)
	{
		add_child(Wins,GTK_WIDGET(ColorDlg),gtk_widget_destroy,_(" Set Color "));
		g_signal_connect(G_OBJECT(ColorDlg),"delete_event",(GCallback)delete_child,NULL);
	}
	else
	{
		gtk_window_set_modal (GTK_WINDOW (ColorDlg), TRUE);
		gtk_window_set_transient_for(GTK_WINDOW(ColorDlg),GTK_WINDOW(Wins));
	}

  	gtk_widget_hide(ColorDlg->help_button);

	g_signal_connect_swapped(GTK_OBJECT(ColorDlg->ok_button),"clicked",
		(GCallback)set_temp_color,GTK_OBJECT(ColorDlg->colorsel));

	g_signal_connect_swapped(GTK_OBJECT(ColorDlg->ok_button),"clicked",
		(GCallback)set_button_color,GTK_OBJECT(button));

	if(!instal)
	{
 		g_signal_connect_swapped(GTK_OBJECT(ColorDlg->ok_button),"clicked",G_CALLBACK(delete_child),GTK_OBJECT(ColorDlg)); 
 		g_signal_connect_swapped(GTK_OBJECT(ColorDlg->cancel_button),"clicked",G_CALLBACK(delete_child),GTK_OBJECT(ColorDlg)); 
	}
	else
	{
 		g_signal_connect_swapped(GTK_OBJECT(ColorDlg->ok_button),"clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(ColorDlg)); 
 		g_signal_connect_swapped(GTK_OBJECT(ColorDlg->cancel_button),"clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(ColorDlg)); 
	}

	gtk_widget_show(GTK_WIDGET(ColorDlg));

}
/********************************************************************************/
GtkWidget*  add_button_color(GtkWidget *hbox,gushort red,gushort green,gushort blue)
{
  GtkWidget *button;
  GdkColor* color = g_malloc(sizeof(GdkColor));

  color->red = red;
  color->green = green;
  color->blue = blue;

  button = create_button_pixmap_color(Wins,red,green,blue);
  g_object_set_data_full (G_OBJECT (button), "InitialCOlor", color,g_free);
  g_signal_connect(G_OBJECT(button), "clicked",(GCallback)open_color_dlg,color);

  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 1);

  return button;
}
/********************************************************************************/
GtkWidget*  add_label_color(GtkWidget *hbox,gchar *tlabel)
{
  GtkWidget *label;

  label = gtk_label_new (tlabel);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 1);

  return label;
}
/********************************************************************************/
void add_frame_label(GtkWidget *hboxall)
{
  GtkWidget *vbox;
  GtkWidget *frame;
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *hboxbutton;
  gchar *type;
  gchar *tfont;
  gchar *tlabel = g_strdup("Label");
  GtkWidget *table = gtk_table_new(2,3,FALSE);


  frame = create_frame(Wins,hboxall,tlabel); 
  vbox = create_vbox(frame);
  gtk_container_add(GTK_CONTAINER(vbox),table);

  add_label_table(table,_(" Default font          "),0,0);
  add_label_table(table," : ",0,1);

  hboxbutton = gtk_hbox_new (TRUE, 0);
  gtk_widget_set_size_request(GTK_WIDGET(hboxbutton), -1,  (gint)(ScreenHeight/50));

  tfont = g_strdup(FontsStyleLabel.fontname);
  label = gtk_label_new(tfont);
  button = gtk_button_new();
  gtk_container_add(GTK_CONTAINER(hboxbutton),label);
  gtk_container_add(GTK_CONTAINER(button),hboxbutton);
  g_object_set_data(G_OBJECT (button), "Hbox", hboxbutton);

  add_widget_table(table,button,0,2);
  g_signal_connect(G_OBJECT(button), "clicked",(GCallback)open_font_dlg,(gpointer)tfont);
  type = g_strdup_printf("%s",tlabel);
  g_object_set_data(G_OBJECT (button), "Type", type);                                                                                    

  add_label_table(table,_(" Foreground color "),1,0);
  add_label_table(table," : ",1,1);

  hboxbutton = gtk_hbox_new (FALSE, 0);
  button = add_button_color(hboxbutton,
  FontsStyleLabel.TextColor.red,FontsStyleLabel.TextColor.green,FontsStyleLabel.TextColor.blue);

  add_widget_table(table,hboxbutton,1,2);
  type = g_strdup_printf("%s_Fore",tlabel);
  g_object_set_data(G_OBJECT (button), "Type", type);

}
/********************************************************************************/
void add_frame_data_result(GtkWidget *hboxall,gchar *tlabel)
{
  GtkWidget *hboxbutton;
  GtkWidget *vbox;
  GtkWidget *frame;
  GtkWidget *button;
  GtkWidget *label;
  gchar *type;
  gchar *tfont;
  GtkWidget *table = gtk_table_new(3,3,FALSE);



  frame = create_frame(Wins,hboxall,tlabel); 
  vbox = create_vbox(frame);
  gtk_container_add(GTK_CONTAINER(vbox),table);

  add_label_table(table," Default font ",0,0);
  add_label_table(table," : ",0,1);
  hboxbutton = gtk_hbox_new (TRUE, 0);
  gtk_widget_set_size_request(GTK_WIDGET(hboxbutton), -1,  (gint)(ScreenHeight/50));
  if(strstr(tlabel,"Data"))
        tfont = g_strdup(FontsStyleData.fontname);
  else
        tfont = g_strdup(FontsStyleResult.fontname);

  label = gtk_label_new(tfont);
  button = gtk_button_new();
  gtk_container_add(GTK_CONTAINER(hboxbutton),label);
  gtk_container_add(GTK_CONTAINER(button),hboxbutton);
  g_object_set_data(G_OBJECT (button), "Hbox", hboxbutton);
  add_widget_table(table,button,0,2);
  g_signal_connect(G_OBJECT(button), "clicked",(GCallback)open_font_dlg,(gpointer)tfont);
  type = g_strdup_printf("%s",tlabel);
  g_object_set_data(G_OBJECT (button), "Type", type);
                                                                                    

  add_label_table(table,_(" Background color "),1,0);
  add_label_table(table," : ",1,1);
  if(strstr(tlabel,"Data"))
  {
  	hboxbutton = gtk_hbox_new (FALSE, 0);
  	button = add_button_color(hboxbutton,
	FontsStyleData.BaseColor.red,FontsStyleData.BaseColor.green,FontsStyleData.BaseColor.blue);
  }
  else
  {
  	hboxbutton = gtk_hbox_new (FALSE, 0);
  	button = add_button_color(hboxbutton,
	FontsStyleResult.BaseColor.red,FontsStyleResult.BaseColor.green,FontsStyleResult.BaseColor.blue);
  }
  add_widget_table(table,hboxbutton,1,2);
  type = g_strdup_printf("%s_Back",tlabel);
  g_object_set_data(G_OBJECT (button), "Type", type);

  add_label_table(table,_(" Foreground color "),2,0);
  add_label_table(table," : ",2,1);

  if(strstr(tlabel,"Data"))
  {
  	hboxbutton = gtk_hbox_new (FALSE, 0);
  	button = add_button_color(hboxbutton,
	FontsStyleData.TextColor.red,FontsStyleData.TextColor.green,FontsStyleData.TextColor.blue);
  }
  else
  {
  	hboxbutton = gtk_hbox_new (FALSE, 0);
  	button = add_button_color(hboxbutton,
	FontsStyleResult.TextColor.red,FontsStyleResult.TextColor.green,FontsStyleResult.TextColor.blue);
  }
  add_widget_table(table,hboxbutton,2,2);
  type = g_strdup_printf("%s_Fore",tlabel);
  g_object_set_data(G_OBJECT (button), "Type", type);
}
/********************************************************************************/
void  create_font_color(GtkWidget *Wins,GtkWidget *Frame)
{
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *hboxall;

  gtk_widget_realize(Wins);
  vbox = create_vbox(Frame);
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 1);

  hboxall = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), hboxall, FALSE, FALSE, 1);
  add_frame_data_result(hboxall,_(" Data "));

  hboxall = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), hboxall, FALSE, FALSE, 1);
  add_frame_data_result(hboxall,_(" Result "));

  hboxall = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), hboxall, FALSE, FALSE, 1);
  add_frame_label(hboxall);

  /* add_frame_other(hboxall);*/

}
/********************************************************************************/
void  create_font_color_in_box(GtkWidget *Win,GtkWidget *Box)
{
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *Frame;
  GtkWidget *hboxall;

  Frame= gtk_frame_new(NULL);
  gtk_container_set_border_width(GTK_CONTAINER(Frame), 10);
  gtk_container_add(GTK_CONTAINER(Box),Frame);

  Wins = Win;
  instal = TRUE;
  gtk_widget_realize(Wins);
  vbox = create_vbox(Frame);
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 1);

  hboxall = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), hboxall, FALSE, FALSE, 1);
  add_frame_data_result(hboxall,_(" Data "));

  hboxall = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), hboxall, FALSE, FALSE, 1);
  add_frame_data_result(hboxall,_(" Result "));

  hboxall = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), hboxall, FALSE, FALSE, 1);
  add_frame_label(hboxall);

  hboxall = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), hboxall, FALSE, FALSE, 1);
  /* add_frame_other(hboxall);*/

  gtk_widget_show_all(Frame);

}
/********************************************************************************/
static void  modify_batch_command(GtkWidget *Entry,gpointer data)
{
	GtkWidget* entryType   = GTK_WIDGET(data);
  	G_CONST_RETURN gchar*     textType  =  gtk_entry_get_text (GTK_ENTRY (entryType));
  	G_CONST_RETURN gchar*     textEntry  =  gtk_entry_get_text (GTK_ENTRY (Entry));
  	GtkWidget* entryAll   = g_object_get_data(G_OBJECT (entryType), "EntryAll");
  	GtkWidget* entryUser  = g_object_get_data(G_OBJECT (entryType), "EntryUser");
  	GtkWidget* entryKill  = g_object_get_data(G_OBJECT (entryType), "EntryKill");
  	GtkWidget* entryJobId = g_object_get_data(G_OBJECT (entryType), "EntryJobId");
	gchar* temp = g_strdup(textEntry);


  	gint n=-1;
  	gint i;

	if(!temp) return;
	if(strlen(temp)<1) return;

  	for(i=0;i<batchCommands.numberOfTypes;i++)
		if(strcmp(textType,batchCommands.types[i])==0)
	  	{
			n = i;
		  	break;
	  	}
	if(n<0) return;

	if(NameTypeBatch) g_free(NameTypeBatch);
	NameTypeBatch = g_strdup(batchCommands.types[n]);

	if(Entry==entryAll)
	{
		if(batchCommands.commandListAll[n]) g_free(batchCommands.commandListAll[n]);
		batchCommands.commandListAll[n] = g_strdup(temp);
		if(NameCommandBatchAll) g_free(NameCommandBatchAll);
		NameCommandBatchAll = g_strdup(temp);
	}
	else
	if(Entry==entryUser)
	{
		if(batchCommands.commandListUser[n]) g_free(batchCommands.commandListUser[n]);
		batchCommands.commandListUser[n] = g_strdup(temp);
		if(NameCommandBatchUser) g_free(NameCommandBatchUser);
		NameCommandBatchUser = g_strdup(temp);
	}
	else
	if(Entry==entryKill)
	{
		if(batchCommands.commandKill[n]) g_free(batchCommands.commandKill[n]);
		batchCommands.commandKill[n] = g_strdup(temp);
		if(NameCommandBatchKill) g_free(NameCommandBatchKill);
		NameCommandBatchKill = g_strdup(temp);
	}
	else
	if(Entry==entryJobId)
	{
		if(batchCommands.jobIdTitle[n]) g_free(batchCommands.jobIdTitle[n]);
		batchCommands.jobIdTitle[n] = g_strdup(temp);
		if(NamejobIdTitleBatch) g_free(NamejobIdTitleBatch);
		NamejobIdTitleBatch = g_strdup(temp);
	}
	g_free(temp);


}
/********************************************************************************/
static void  modify_batch_entrys(GtkWidget *Entry,gpointer data)
{
  GtkWidget* entryAll   = g_object_get_data(G_OBJECT (Entry), "EntryAll");
  GtkWidget* entryUser  = g_object_get_data(G_OBJECT (Entry), "EntryUser");
  GtkWidget* entryKill  = g_object_get_data(G_OBJECT (Entry), "EntryKill");
  GtkWidget* entryJobId = g_object_get_data(G_OBJECT (Entry), "EntryJobId");
  G_CONST_RETURN gchar*     textType  =  gtk_entry_get_text (GTK_ENTRY (Entry));
  gint n=-1;
  gint i;
  for(i=0;i<batchCommands.numberOfTypes;i++)
	  if(strcmp(textType,batchCommands.types[i])==0)
	  {
		  n = i;
		  break;
	  }
  if(n<0) return;
  gtk_entry_set_text (GTK_ENTRY (entryAll),batchCommands.commandListAll[n]);
  gtk_entry_set_text (GTK_ENTRY (entryUser),batchCommands.commandListUser[n]);
  gtk_entry_set_text (GTK_ENTRY (entryKill),batchCommands.commandKill[n]);
  gtk_entry_set_text (GTK_ENTRY (entryJobId),batchCommands.jobIdTitle[n]);
	
  if(n==0 || n == 1 || n == 2)
  {
  	gtk_editable_set_editable((GtkEditable*)entryAll,FALSE);
  	gtk_editable_set_editable((GtkEditable*)entryUser,FALSE);
  	gtk_editable_set_editable((GtkEditable*)entryKill,FALSE);
  	gtk_editable_set_editable((GtkEditable*)entryJobId,FALSE);

	gtk_widget_set_sensitive(entryAll, FALSE);
	gtk_widget_set_sensitive(entryUser, FALSE);
	gtk_widget_set_sensitive(entryKill, FALSE);
	gtk_widget_set_sensitive(entryJobId, FALSE);
  }
  else
  {
  	gtk_editable_set_editable((GtkEditable*)entryAll,TRUE);
  	gtk_editable_set_editable((GtkEditable*)entryUser,TRUE);
  	gtk_editable_set_editable((GtkEditable*)entryKill,TRUE);
  	gtk_editable_set_editable((GtkEditable*)entryJobId,TRUE);

	gtk_widget_set_sensitive(entryAll, TRUE);
	gtk_widget_set_sensitive(entryUser, TRUE);
	gtk_widget_set_sensitive(entryKill, TRUE);
	gtk_widget_set_sensitive(entryJobId, TRUE);

  }

}
/********************************************************************************/
void  create_batch_commands(GtkWidget *Wins,GtkWidget *vbox,gboolean expand)
{
  GtkWidget *frame;
  GtkWidget *combo;
  GtkWidget *entry;
  GtkWidget *table = gtk_table_new(4,3,FALSE);
  gint i;


  frame = gtk_frame_new (_("Batch Commands"));
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox), frame, expand, expand, 0);
  gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);


/* ------------------------------------------------------------------*/
  i = 0;
  add_label_table(table,_(" Batch Type "),(gushort)i,0);
  add_label_table(table," : ",(gushort)i,1);

  combo = create_combo_box_entry(batchCommands.types,batchCommands.numberOfTypes,TRUE,-1,-1);
  EntryBatchType = GTK_BIN(combo)->child;
  add_widget_table(table,combo,(gushort)i,2);
  gtk_editable_set_editable((GtkEditable*)EntryBatchType,FALSE);
  gtk_entry_set_text (GTK_ENTRY (EntryBatchType),NameTypeBatch);
/* ------------------------------------------------------------------*/
  i = 1;
  add_label_table(table,_(" Command for list of all Job "),(gushort)i,0);
  add_label_table(table," : ",(gushort)i,1);
  entry = gtk_entry_new();
  add_widget_table(table,entry,(gushort)i,2);
  gtk_entry_set_text (GTK_ENTRY (entry),NameCommandBatchAll);
  g_object_set_data(G_OBJECT (EntryBatchType), "EntryAll", entry);
  gtk_editable_set_editable((GtkEditable*)entry,FALSE);
  gtk_widget_set_sensitive(entry, FALSE);
  g_signal_connect(G_OBJECT (entry), "changed", (GCallback)modify_batch_command,EntryBatchType);
/* ------------------------------------------------------------------*/
  i = 2;
  add_label_table(table,N_(" Command for list of user Job "),(gushort)i,0);
  add_label_table(table," : ",(gushort)i,1);
  entry = gtk_entry_new();
  add_widget_table(table,entry,(gushort)i,2);
  gtk_entry_set_text (GTK_ENTRY (entry),NameCommandBatchUser);
  g_object_set_data(G_OBJECT (EntryBatchType), "EntryUser", entry);
  gtk_editable_set_editable((GtkEditable*)entry,FALSE);
  gtk_widget_set_sensitive(entry, FALSE);
  g_signal_connect(G_OBJECT (entry), "changed", (GCallback)modify_batch_command,EntryBatchType);
/* ------------------------------------------------------------------*/
  i = 3;
  add_label_table(table,_(" Command for kill a Job "),(gushort)i,0);
  add_label_table(table," : ",(gushort)i,1);
  entry = gtk_entry_new();
  add_widget_table(table,entry,(gushort)i,2);
  gtk_entry_set_text (GTK_ENTRY (entry),NameCommandBatchKill);
  g_object_set_data(G_OBJECT (EntryBatchType), "EntryKill", entry);
  gtk_editable_set_editable((GtkEditable*)entry,FALSE);
  gtk_widget_set_sensitive(entry, FALSE);
  g_signal_connect(G_OBJECT (entry), "changed", (GCallback)modify_batch_command,EntryBatchType);
/* ------------------------------------------------------------------*/
  i = 4;
  add_label_table(table,_(" Title for Job id "),(gushort)i,0);
  add_label_table(table," : ",(gushort)i,1);
  entry = gtk_entry_new();
  add_widget_table(table,entry,(gushort)i,2);
  gtk_entry_set_text (GTK_ENTRY (entry),NamejobIdTitleBatch);
  g_object_set_data(G_OBJECT (EntryBatchType), "EntryJobId", entry);
  gtk_editable_set_editable((GtkEditable*)entry,FALSE);
  gtk_widget_set_sensitive(entry, FALSE);
  g_signal_connect(G_OBJECT (entry), "changed", (GCallback)modify_batch_command,EntryBatchType);
/* ------------------------------------------------------------------*/

  g_signal_connect(G_OBJECT (EntryBatchType), "changed", (GCallback)modify_batch_entrys, NULL);
  gtk_entry_set_text (GTK_ENTRY (EntryBatchType)," ");
  gtk_entry_set_text (GTK_ENTRY (EntryBatchType),NameTypeBatch);
  gtk_widget_show_all(frame);

}
/********************************************************************************/
/*
static void changed_babel(GtkWidget* wid)
{
  	if(babelCommand) g_free(babelCommand);
	babelCommand = g_strdup(gtk_entry_get_text(GTK_ENTRY(entrybabel)));
}
static void set_entry_babel(GtkWidget* SelFile, gint response_id)
{
  GtkWidget *entry = NULL;
  gchar *longfile = NULL;
   if(response_id != GTK_RESPONSE_OK) return;

  entry = (GtkWidget*)(g_object_get_data(G_OBJECT(SelFile),"EntryFile"));	
  longfile = gabedit_file_chooser_get_current_file(GABEDIT_FILE_CHOOSER(SelFile));
  gtk_entry_set_text(GTK_ENTRY(entry),longfile);

  if(babelCommand) g_free(babelCommand);
  babelCommand = g_strdup(gtk_entry_get_text(GTK_ENTRY(entrybabel)));

}
static void set_entry_babel_selection(GtkWidget* entry)
{
  GtkWidget *SelFile;

  SelFile = gabedit_file_chooser_new(_("File chooser"), GTK_FILE_CHOOSER_ACTION_OPEN);
  gtk_window_set_modal (GTK_WINDOW (SelFile), TRUE);
  gabedit_file_chooser_hide_hidden(GABEDIT_FILE_CHOOSER(SelFile));
  g_signal_connect(G_OBJECT(SelFile),"delete_event", (GCallback)gtk_widget_destroy,NULL);

  g_object_set_data(G_OBJECT (SelFile), "EntryFile", entry);

  g_signal_connect (SelFile, "response",  G_CALLBACK (set_entry_babel), GTK_OBJECT(SelFile));
  g_signal_connect (SelFile, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(SelFile));

  // g_signal_connect (SelFile, "close",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(SelFile));
  gtk_widget_show(SelFile);
}
*/
/********************************************************************************/
#ifdef G_OS_WIN32
static void set_entry_gamessdir(GtkWidget* dirSelector, gint response_id)
{
	gchar* dirname = NULL;
	GtkWidget *entry;
	if(response_id != GTK_RESPONSE_OK) return;
	dirname = gabedit_folder_chooser_get_current_folder(GABEDIT_FOLDER_CHOOSER(dirSelector));


	entry = (GtkWidget*)(g_object_get_data(G_OBJECT(dirSelector),"EntryFile"));	
	gtk_entry_set_text(GTK_ENTRY(entry),dirname);

	if(gamessDirectory) g_free(gamessDirectory);
	gamessDirectory = g_strdup(dirname);
}
/********************************************************************************/
static void set_entry_gamessDir_selection(GtkWidget* entry)
{
	GtkWidget *dirSelector;
	dirSelector = selectionOfDir(set_entry_gamessdir, _("Select Gamess folder"), GABEDIT_TYPEWIN_ORB); 
  	gtk_window_set_modal (GTK_WINDOW (dirSelector), TRUE);
  	g_signal_connect(G_OBJECT(dirSelector),"delete_event", (GCallback)gtk_widget_destroy,NULL);

	g_object_set_data(G_OBJECT (dirSelector), "EntryFile", entry);

	g_signal_connect (dirSelector, "response",  G_CALLBACK (set_entry_gamessdir), GTK_OBJECT(dirSelector));
	g_signal_connect (dirSelector, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(dirSelector));

	gtk_widget_show(dirSelector);
}
/********************************************************************************/
static void set_entry_demondir(GtkWidget* dirSelector, gint response_id)
{
	gchar* dirname = NULL;
	gchar* t = NULL;
	GtkWidget *entry;
	if(response_id != GTK_RESPONSE_OK) return;
	dirname = gabedit_folder_chooser_get_current_folder(GABEDIT_FOLDER_CHOOSER(dirSelector));


	entry = (GtkWidget*)(g_object_get_data(G_OBJECT(dirSelector),"EntryFile"));	
	gtk_entry_set_text(GTK_ENTRY(entry),dirname);

	if(demonDirectory) g_free(demonDirectory);
	demonDirectory = g_strdup(dirname);
	t = g_strdup_printf("%s;%s",demonDirectory,g_getenv("PATH"));
	g_setenv("PATH",t,TRUE);
	g_free(t);
}
/********************************************************************************/
static void set_entry_demonDir_selection(GtkWidget* entry)
{
	GtkWidget *dirSelector;
	dirSelector = selectionOfDir(set_entry_demondir, _("Select Demon folder"), GABEDIT_TYPEWIN_ORB); 
  	gtk_window_set_modal (GTK_WINDOW (dirSelector), TRUE);
  	g_signal_connect(G_OBJECT(dirSelector),"delete_event", (GCallback)gtk_widget_destroy,NULL);

	g_object_set_data(G_OBJECT (dirSelector), "EntryFile", entry);

	g_signal_connect (dirSelector, "response",  G_CALLBACK (set_entry_demondir), GTK_OBJECT(dirSelector));
	g_signal_connect (dirSelector, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(dirSelector));

	gtk_widget_show(dirSelector);
}
/********************************************************************************/
static void set_entry_orcadir(GtkWidget* dirSelector, gint response_id)
{
	gchar* dirname = NULL;
	gchar* t = NULL;
	GtkWidget *entry;
	if(response_id != GTK_RESPONSE_OK) return;
	dirname = gabedit_folder_chooser_get_current_folder(GABEDIT_FOLDER_CHOOSER(dirSelector));


	entry = (GtkWidget*)(g_object_get_data(G_OBJECT(dirSelector),"EntryFile"));	
	gtk_entry_set_text(GTK_ENTRY(entry),dirname);

	if(orcaDirectory) g_free(orcaDirectory);
	orcaDirectory = g_strdup(dirname);
	t = g_strdup_printf("%s;%s",orcaDirectory,g_getenv("PATH"));
	g_setenv("PATH",t,TRUE);
	g_free(t);
}
/********************************************************************************/
static void set_entry_orcaDir_selection(GtkWidget* entry)
{
	GtkWidget *dirSelector;
	dirSelector = selectionOfDir(set_entry_orcadir, _("Select Orca folder"), GABEDIT_TYPEWIN_ORB); 
  	gtk_window_set_modal (GTK_WINDOW (dirSelector), TRUE);
  	g_signal_connect(G_OBJECT(dirSelector),"delete_event", (GCallback)gtk_widget_destroy,NULL);

	g_object_set_data(G_OBJECT (dirSelector), "EntryFile", entry);

	g_signal_connect (dirSelector, "response",  G_CALLBACK (set_entry_orcadir), GTK_OBJECT(dirSelector));
	g_signal_connect (dirSelector, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(dirSelector));

	gtk_widget_show(dirSelector);
}
/********************************************************************************/
static void set_entry_nwchemdir(GtkWidget* dirSelector, gint response_id)
{
	gchar* dirname = NULL;
	gchar* t = NULL;
	GtkWidget *entry;
	if(response_id != GTK_RESPONSE_OK) return;
	dirname = gabedit_folder_chooser_get_current_folder(GABEDIT_FOLDER_CHOOSER(dirSelector));


	entry = (GtkWidget*)(g_object_get_data(G_OBJECT(dirSelector),"EntryFile"));	
	gtk_entry_set_text(GTK_ENTRY(entry),dirname);

	if(nwchemDirectory) g_free(nwchemDirectory);
	nwchemDirectory = g_strdup(dirname);
	t = g_strdup_printf("%s;%s",nwchemDirectory,g_getenv("PATH"));
	g_setenv("PATH",t,TRUE);
	g_free(t);
}
/********************************************************************************/
static void set_entry_nwchemDir_selection(GtkWidget* entry)
{
	GtkWidget *dirSelector;
	dirSelector = selectionOfDir(set_entry_nwchemdir, _("Select NWChem folder"), GABEDIT_TYPEWIN_ORB); 
  	gtk_window_set_modal (GTK_WINDOW (dirSelector), TRUE);
  	g_signal_connect(G_OBJECT(dirSelector),"delete_event", (GCallback)gtk_widget_destroy,NULL);

	g_object_set_data(G_OBJECT (dirSelector), "EntryFile", entry);

	g_signal_connect (dirSelector, "response",  G_CALLBACK (set_entry_nwchemdir), GTK_OBJECT(dirSelector));
	g_signal_connect (dirSelector, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(dirSelector));

	gtk_widget_show(dirSelector);
}
/********************************************************************************/
static void set_entry_psicodedir(GtkWidget* dirSelector, gint response_id)
{
	gchar* dirname = NULL;
	gchar* t = NULL;
	GtkWidget *entry;
	if(response_id != GTK_RESPONSE_OK) return;
	dirname = gabedit_folder_chooser_get_current_folder(GABEDIT_FOLDER_CHOOSER(dirSelector));


	entry = (GtkWidget*)(g_object_get_data(G_OBJECT(dirSelector),"EntryFile"));	
	gtk_entry_set_text(GTK_ENTRY(entry),dirname);

	if(psicodeDirectory) g_free(psicodeDirectory);
	psicodeDirectory = g_strdup(dirname);
	t = g_strdup_printf("%s;%s",psicodeDirectory,g_getenv("PATH"));
	g_setenv("PATH",t,TRUE);
	g_free(t);
}
/********************************************************************************/
static void set_entry_psicodeDir_selection(GtkWidget* entry)
{
	GtkWidget *dirSelector;
	dirSelector = selectionOfDir(set_entry_psicodedir, _("Select Psicode folder"), GABEDIT_TYPEWIN_ORB); 
  	gtk_window_set_modal (GTK_WINDOW (dirSelector), TRUE);
  	g_signal_connect(G_OBJECT(dirSelector),"delete_event", (GCallback)gtk_widget_destroy,NULL);

	g_object_set_data(G_OBJECT (dirSelector), "EntryFile", entry);

	g_signal_connect (dirSelector, "response",  G_CALLBACK (set_entry_psicodedir), GTK_OBJECT(dirSelector));
	g_signal_connect (dirSelector, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(dirSelector));

	gtk_widget_show(dirSelector);
}
/********************************************************************************/
static void set_entry_fireflydir(GtkWidget* dirSelector, gint response_id)
{
	gchar* dirname = NULL;
	gchar* t = NULL;
	GtkWidget *entry;
	if(response_id != GTK_RESPONSE_OK) return;
	dirname = gabedit_folder_chooser_get_current_folder(GABEDIT_FOLDER_CHOOSER(dirSelector));


	entry = (GtkWidget*)(g_object_get_data(G_OBJECT(dirSelector),"EntryFile"));	
	gtk_entry_set_text(GTK_ENTRY(entry),dirname);

	if(fireflyDirectory) g_free(fireflyDirectory);
	fireflyDirectory = g_strdup(dirname);
	t = g_strdup_printf("%s;%s",fireflyDirectory,g_getenv("PATH"));
	g_setenv("PATH",t,TRUE);
	g_free(t);
}
/********************************************************************************/
static void set_entry_fireflyDir_selection(GtkWidget* entry)
{
	GtkWidget *dirSelector;
	dirSelector = selectionOfDir(set_entry_fireflydir, _("Select FireFly folder"), GABEDIT_TYPEWIN_ORB); 
  	gtk_window_set_modal (GTK_WINDOW (dirSelector), TRUE);
  	g_signal_connect(G_OBJECT(dirSelector),"delete_event", (GCallback)gtk_widget_destroy,NULL);

	g_object_set_data(G_OBJECT (dirSelector), "EntryFile", entry);

	g_signal_connect (dirSelector, "response",  G_CALLBACK (set_entry_fireflydir), GTK_OBJECT(dirSelector));
	g_signal_connect (dirSelector, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(dirSelector));

	gtk_widget_show(dirSelector);
}
/********************************************************************************/
static void set_entry_mopacdir(GtkWidget* dirSelector, gint response_id)
{
	gchar* dirname = NULL;
	gchar* t = NULL;
	GtkWidget *entry;
	if(response_id != GTK_RESPONSE_OK) return;
	dirname = gabedit_folder_chooser_get_current_folder(GABEDIT_FOLDER_CHOOSER(dirSelector));


	entry = (GtkWidget*)(g_object_get_data(G_OBJECT(dirSelector),"EntryFile"));	
	gtk_entry_set_text(GTK_ENTRY(entry),dirname);

	if(mopacDirectory) g_free(mopacDirectory);
	mopacDirectory = g_strdup(dirname);
	t = g_strdup_printf("%s;%s",mopacDirectory,g_getenv("PATH"));
	g_setenv("PATH",t,TRUE);
	g_free(t);
}
/********************************************************************************/
static void set_entry_mopacDir_selection(GtkWidget* entry)
{
	GtkWidget *dirSelector;
	dirSelector = selectionOfDir(set_entry_mopacdir, _("Select Mopac folder"), GABEDIT_TYPEWIN_ORB); 
  	gtk_window_set_modal (GTK_WINDOW (dirSelector), TRUE);
  	g_signal_connect(G_OBJECT(dirSelector),"delete_event", (GCallback)gtk_widget_destroy,NULL);

	g_object_set_data(G_OBJECT (dirSelector), "EntryFile", entry);

	g_signal_connect (dirSelector, "response",  G_CALLBACK (set_entry_mopacdir), GTK_OBJECT(dirSelector));
	g_signal_connect (dirSelector, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(dirSelector));

	gtk_widget_show(dirSelector);
}
/********************************************************************************/
static void set_entry_povraydir(GtkWidget* dirSelector, gint response_id)
{
	gchar* dirname = NULL;
	gchar* t = NULL;
	GtkWidget *entry;
	if(response_id != GTK_RESPONSE_OK) return;
	dirname = gabedit_folder_chooser_get_current_folder(GABEDIT_FOLDER_CHOOSER(dirSelector));


	entry = (GtkWidget*)(g_object_get_data(G_OBJECT(dirSelector),"EntryFile"));	
	gtk_entry_set_text(GTK_ENTRY(entry),dirname);

	if(povrayDirectory) g_free(povrayDirectory);
	povrayDirectory = g_strdup(dirname);
	t = g_strdup_printf("%s;%s",povrayDirectory,g_getenv("PATH"));
	g_setenv("PATH",t,TRUE);
	g_free(t);
}
/********************************************************************************/
static void set_entry_povrayDir_selection(GtkWidget* entry)
{
	GtkWidget *dirSelector;
	dirSelector = selectionOfDir(set_entry_povraydir, _("Select PovRay folder"), GABEDIT_TYPEWIN_ORB); 
  	gtk_window_set_modal (GTK_WINDOW (dirSelector), TRUE);
  	g_signal_connect(G_OBJECT(dirSelector),"delete_event", (GCallback)gtk_widget_destroy,NULL);

	g_object_set_data(G_OBJECT (dirSelector), "EntryFile", entry);

	g_signal_connect (dirSelector, "response",  G_CALLBACK (set_entry_povraydir), GTK_OBJECT(dirSelector));
	g_signal_connect (dirSelector, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(dirSelector));

	gtk_widget_show(dirSelector);
}
/********************************************************************************/
static void set_entry_gaussdir(GtkWidget* dirSelector, gint response_id)
{
	gchar* dirname = NULL;
	gchar* t = NULL;
	GtkWidget *entry;
	if(response_id != GTK_RESPONSE_OK) return;
	dirname = gabedit_folder_chooser_get_current_folder(GABEDIT_FOLDER_CHOOSER(dirSelector));


	entry = (GtkWidget*)(g_object_get_data(G_OBJECT(dirSelector),"EntryFile"));	
	gtk_entry_set_text(GTK_ENTRY(entry),dirname);

	if(gaussDirectory) g_free(gaussDirectory);
	gaussDirectory = g_strdup(dirname);
	t = g_strdup_printf("%s;%s",gaussDirectory,g_getenv("PATH"));
	g_setenv("PATH",t,TRUE);
	g_free(t);
}
/********************************************************************************/
static void set_entry_gaussDir_selection(GtkWidget* entry)
{
	GtkWidget *dirSelector;
	dirSelector = selectionOfDir(set_entry_gaussdir, _("Select Gaussian folder"), GABEDIT_TYPEWIN_ORB); 
  	gtk_window_set_modal (GTK_WINDOW (dirSelector), TRUE);
  	g_signal_connect(G_OBJECT(dirSelector),"delete_event", (GCallback)gtk_widget_destroy,NULL);

	g_object_set_data(G_OBJECT (dirSelector), "EntryFile", entry);

	g_signal_connect (dirSelector, "response",  G_CALLBACK (set_entry_gaussdir), GTK_OBJECT(dirSelector));
	g_signal_connect (dirSelector, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(dirSelector));

	gtk_widget_show(dirSelector);
}
#endif
/********************************************************************************/
static void set_entry_openbabeldir(GtkWidget* dirSelector, gint response_id)
{
	gchar* dirname = NULL;
	gchar* t = NULL;
	GtkWidget *entry;
	if(response_id != GTK_RESPONSE_OK) return;
	dirname = gabedit_folder_chooser_get_current_folder(GABEDIT_FOLDER_CHOOSER(dirSelector));


	entry = (GtkWidget*)(g_object_get_data(G_OBJECT(dirSelector),"EntryFile"));	
	gtk_entry_set_text(GTK_ENTRY(entry),dirname);

	if(openbabelDirectory) g_free(openbabelDirectory);
	openbabelDirectory = g_strdup(dirname);
	t = g_strdup_printf("%s;%s",openbabelDirectory,g_getenv("PATH"));
	g_setenv("PATH",t,TRUE);
	g_free(t);
	if(babelCommand) g_free(babelCommand);
	babelCommand = g_strdup_printf("%s%sobabel",openbabelDirectory,G_DIR_SEPARATOR_S);
}
/********************************************************************************/
static void set_entry_openbabelDir_selection(GtkWidget* entry)
{
	GtkWidget *dirSelector;
	dirSelector = selectionOfDir(set_entry_openbabeldir, _("Select PovRay folder"), GABEDIT_TYPEWIN_ORB); 
  	gtk_window_set_modal (GTK_WINDOW (dirSelector), TRUE);
  	g_signal_connect(G_OBJECT(dirSelector),"delete_event", (GCallback)gtk_widget_destroy,NULL);

	g_object_set_data(G_OBJECT (dirSelector), "EntryFile", entry);

	g_signal_connect (dirSelector, "response",  G_CALLBACK (set_entry_openbabeldir), GTK_OBJECT(dirSelector));
	g_signal_connect (dirSelector, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(dirSelector));

	gtk_widget_show(dirSelector);
}
/********************************************************************************/
void  create_execucte_commands(GtkWidget *Wins,GtkWidget *vbox,gboolean expand)
{
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *frame;
  GtkWidget *combo;
  GtkWidget *button;

  frame = gtk_frame_new (_("Commands to execute Computational Chemistry Packages"));
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox), frame, expand, expand, 0);
  gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);


/* ------------------------------------------------------------------*/
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);
  label = gtk_label_new (_("Command for execute Gamess : "));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 3);

  combo = create_combo_box_entry(gamessCommands.commands,gamessCommands.numberOfCommands,TRUE,-1,-1);
  ComboGamess = combo;
  EntryGamess = GTK_BIN(combo)->child;
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 3);
  gtk_entry_set_text (GTK_ENTRY (EntryGamess),NameCommandGamess );
  g_signal_connect(G_OBJECT (EntryGamess), "activate",
			(GCallback)modify_gamess_command,
			NULL);

  button = create_button(Wins,_("  Remove from list  "));
  ButtonGamess = button;
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  if(gamessCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(button, FALSE);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(remove_gamess_command),NULL);

  button = create_button(Wins,_("  Help  "));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(help_commands),NULL);
/* ------------------------------------------------------------------*/
  gtk_widget_realize(Wins);

  create_hseparator(vbox);

/* ------------------------------------------------------------------*/
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);
  label = gtk_label_new (_("Command for execute DeMon    : "));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 3);

  combo = create_combo_box_entry(demonCommands.commands,demonCommands.numberOfCommands,TRUE,-1,-1);
  ComboDeMon = combo;
  EntryDeMon =  GTK_BIN(combo)->child;
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 3);
  gtk_entry_set_text (GTK_ENTRY (EntryDeMon),NameCommandDeMon);
  g_signal_connect(G_OBJECT (EntryDeMon), "activate",
			(GCallback)modify_demon_command,
			NULL);
  button = create_button(Wins,_("  Remove from list  "));
  ButtonDeMon = button;
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  if(demonCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(button, FALSE);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(remove_demon_command),NULL);

  button = create_button(Wins,_("  Help  "));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(help_commands),NULL);
/* ------------------------------------------------------------------*/

  create_hseparator(vbox);


/* ------------------------------------------------------------------*/
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);
  label = gtk_label_new (_("Command for execute Gaussian : "));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 3);

  combo = create_combo_box_entry(gaussianCommands.commands,gaussianCommands.numberOfCommands,TRUE,-1,-1);
  ComboGaussian = combo;
  EntryGaussian = GTK_BIN(combo)->child;
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 3);
  gtk_entry_set_text (GTK_ENTRY (EntryGaussian),NameCommandGaussian );
  g_signal_connect(G_OBJECT (EntryGaussian), "activate",
			(GCallback)modify_gaussian_command,
			NULL);

  button = create_button(Wins,_("  Remove from list  "));
  ButtonGaussian = button;
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  if(gaussianCommands.numberOfCommands<2) gtk_widget_set_sensitive(button, FALSE);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(remove_gaussian_command),NULL);

  button = create_button(Wins,_("  Help  "));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(help_commands),NULL);
/* ------------------------------------------------------------------*/

  create_hseparator(vbox);

/* ------------------------------------------------------------------*/
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);
  label = gtk_label_new (_("Command for execute Molpro    : "));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 3);

  combo = create_combo_box_entry(molproCommands.commands,molproCommands.numberOfCommands,TRUE,-1,-1);
  ComboMolpro = combo;
  EntryMolpro =  GTK_BIN(combo)->child;
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 3);
  gtk_entry_set_text (GTK_ENTRY (EntryMolpro),NameCommandMolpro);
  g_signal_connect(G_OBJECT (EntryMolpro), "activate",
			(GCallback)modify_molpro_command,
			NULL);
  button = create_button(Wins,_("  Remove from list  "));
  ButtonMolpro = button;
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  if(molproCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(button, FALSE);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(remove_molpro_command),NULL);

  button = create_button(Wins,_("  Help  "));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(help_commands),NULL);
/* ------------------------------------------------------------------*/

  create_hseparator(vbox);

/* ------------------------------------------------------------------*/
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);
  label = gtk_label_new (_("Command for execute Molcas    : "));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 3);

  combo = create_combo_box_entry(molcasCommands.commands,molcasCommands.numberOfCommands,TRUE,-1,-1);
  ComboMolcas = combo;
  EntryMolcas =  GTK_BIN(combo)->child;
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 3);
  gtk_entry_set_text (GTK_ENTRY (EntryMolcas),NameCommandMolcas);
  g_signal_connect(G_OBJECT (EntryMolcas), "activate",
			(GCallback)modify_molcas_command,
			NULL);
  button = create_button(Wins,_("  Remove from list  "));
  ButtonMolcas = button;
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  if(molcasCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(button, FALSE);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(remove_molcas_command),NULL);

  button = create_button(Wins,_("  Help  "));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(help_commands),NULL);
/* ------------------------------------------------------------------*/

  create_hseparator(vbox);

/* ------------------------------------------------------------------*/
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);
  label = gtk_label_new (_("Command for execute MPQC    : "));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 3);

  combo = create_combo_box_entry(mpqcCommands.commands,mpqcCommands.numberOfCommands,TRUE,-1,-1);
  ComboMPQC = combo;
  EntryMPQC =  GTK_BIN(combo)->child;
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 3);
  gtk_entry_set_text (GTK_ENTRY (EntryMPQC),NameCommandMPQC);
  g_signal_connect(G_OBJECT (EntryMPQC), "activate",
			(GCallback)modify_mpqc_command,
			NULL);
  button = create_button(Wins,_("  Remove from list  "));
  ButtonMPQC = button;
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  if(mpqcCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(button, FALSE);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(remove_mpqc_command),NULL);

  button = create_button(Wins,_("  Help  "));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(help_commands),NULL);

/* ------------------------------------------------------------------*/
  create_hseparator(vbox);
/* ------------------------------------------------------------------*/
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);
  label = gtk_label_new (_("Command for execute Orca       : "));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 3);

  combo = create_combo_box_entry(orcaCommands.commands,orcaCommands.numberOfCommands,TRUE,-1,-1);
  ComboOrca = combo;
  EntryOrca =  GTK_BIN(combo)->child;
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 3);
  gtk_entry_set_text (GTK_ENTRY (EntryOrca),NameCommandOrca);
  g_signal_connect(G_OBJECT (EntryOrca), "activate", (GCallback)modify_orca_command, NULL);
  button = create_button(Wins,_("  Remove from list  "));
  ButtonOrca = button;
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  if(orcaCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(button, FALSE);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(remove_orca_command),NULL);

  button = create_button(Wins,_("  Help  "));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(help_commands),NULL);

  create_hseparator(vbox);
/* ------------------------------------------------------------------*/
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);
  label = gtk_label_new (_("Command for execute NWChem       : "));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 3);

  combo = create_combo_box_entry(nwchemCommands.commands,nwchemCommands.numberOfCommands,TRUE,-1,-1);
  ComboNWChem = combo;
  EntryNWChem =  GTK_BIN(combo)->child;
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 3);
  gtk_entry_set_text (GTK_ENTRY (EntryNWChem),NameCommandNWChem);
  g_signal_connect(G_OBJECT (EntryNWChem), "activate", (GCallback)modify_nwchem_command, NULL);
  button = create_button(Wins,_("  Remove from list  "));
  ButtonNWChem = button;
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  if(nwchemCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(button, FALSE);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(remove_nwchem_command),NULL);

  button = create_button(Wins,_("  Help  "));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(help_commands),NULL);

  create_hseparator(vbox);
/* ------------------------------------------------------------------*/
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);
  label = gtk_label_new (_("Command for execute Psicode       : "));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 3);

  combo = create_combo_box_entry(psicodeCommands.commands,psicodeCommands.numberOfCommands,TRUE,-1,-1);
  ComboPsicode = combo;
  EntryPsicode =  GTK_BIN(combo)->child;
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 3);
  gtk_entry_set_text (GTK_ENTRY (EntryPsicode),NameCommandPsicode);
  g_signal_connect(G_OBJECT (EntryPsicode), "activate", (GCallback)modify_psicode_command, NULL);
  button = create_button(Wins,_("  Remove from list  "));
  ButtonPsicode = button;
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  if(psicodeCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(button, FALSE);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(remove_psicode_command),NULL);

  button = create_button(Wins,_("  Help  "));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(help_commands),NULL);

  create_hseparator(vbox);
/* ------------------------------------------------------------------*/
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);
  label = gtk_label_new (_("Command for execute FireFly    : "));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 3);

  combo = create_combo_box_entry(fireflyCommands.commands,fireflyCommands.numberOfCommands,TRUE,-1,-1);
  ComboFireFly = combo;
  EntryFireFly =  GTK_BIN(combo)->child;
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 3);
  gtk_entry_set_text (GTK_ENTRY (EntryFireFly),NameCommandFireFly);
  g_signal_connect(G_OBJECT (EntryFireFly), "activate", (GCallback)modify_firefly_command, NULL);
  button = create_button(Wins,_("  Remove from list  "));
  ButtonFireFly = button;
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  if(fireflyCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(button, FALSE);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(remove_firefly_command),NULL);

  button = create_button(Wins,_("  Help  "));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(help_commands),NULL);
/* ------------------------------------------------------------------*/
  create_hseparator(vbox);
/* ------------------------------------------------------------------*/
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);
  label = gtk_label_new (_("Command for execute Q-Chem    : "));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 3);

  combo = create_combo_box_entry(qchemCommands.commands,qchemCommands.numberOfCommands,TRUE,-1,-1);
  ComboQChem = combo;
  EntryQChem =  GTK_BIN(combo)->child;
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 3);
  gtk_entry_set_text (GTK_ENTRY (EntryQChem),NameCommandQChem);
  g_signal_connect(G_OBJECT (EntryQChem), "activate", (GCallback)modify_qchem_command, NULL);
  button = create_button(Wins,_("  Remove from list  "));
  ButtonQChem = button;
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  if(qchemCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(button, FALSE);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(remove_qchem_command),NULL);

  button = create_button(Wins,_("  Help  "));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(help_commands),NULL);
/* ------------------------------------------------------------------*/
  create_hseparator(vbox);
/* ------------------------------------------------------------------*/
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);
  label = gtk_label_new (_("Command for execute Mopac    : "));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 3);

  combo = create_combo_box_entry(mopacCommands.commands,mopacCommands.numberOfCommands,TRUE,-1,-1);
  ComboMopac = combo;
  EntryMopac =  GTK_BIN(combo)->child;
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 3);
  gtk_entry_set_text (GTK_ENTRY (EntryMopac),NameCommandMopac);
  g_signal_connect(G_OBJECT (EntryMopac), "activate", (GCallback)modify_mopac_command, NULL);
  button = create_button(Wins,_("  Remove from list  "));
  ButtonMopac = button;
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  if(mopacCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(button, FALSE);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(remove_mopac_command),NULL);

  button = create_button(Wins,_("  Help  "));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(help_commands),NULL);
/* ------------------------------------------------------------------*/
  create_hseparator(vbox);
/* ------------------------------------------------------------------*/
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);
  label = gtk_label_new (_("Command for execute Povray    : "));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 3);

  combo = create_combo_box_entry(povrayCommands.commands,povrayCommands.numberOfCommands,TRUE,-1,-1);
  ComboPovray = combo;
  EntryPovray =  GTK_BIN(combo)->child;
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 3);
  gtk_entry_set_text (GTK_ENTRY (EntryPovray),NameCommandPovray);
  g_signal_connect(G_OBJECT (EntryPovray), "activate", (GCallback)modify_povray_command, NULL);
  button = create_button(Wins,_("  Remove from list  "));
  ButtonPovray = button;
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  if(povrayCommands.numberOfCommands<2)
  	gtk_widget_set_sensitive(button, FALSE);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(remove_povray_command),NULL);

  button = create_button(Wins,_("  Help  "));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 3);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(help_commands),NULL);
/* ------------------------------------------------------------------*/
/* ------------------------------------------------------------------*/
  create_hseparator(vbox);

/*#ifdef G_OS_WIN32*/
/*
  // Remove it. Now babelCommand=openbabelDirectory/obabel
  {
	GtkWidget* entry;
  	GtkWidget *table = gtk_table_new(1,3,FALSE);

	if(!babelCommand) babelCommand = g_strdup_printf("%s%sbabel.exe",g_get_current_dir(),G_DIR_SEPARATOR_S);

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

	add_label_table(table,_("Command for Open Babel         : "),0,0);
  	entry = gtk_entry_new ();
	entrybabel = entry;
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,1,1+1,0,0+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),babelCommand);
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);
	button = create_button_pixmap(Wins,open_xpm,NULL);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(GTK_OBJECT (button), "clicked",
                                     G_CALLBACK(set_entry_babel_selection),
                                     GTK_OBJECT(entry));
	add_widget_table(table,button,0,2);
  	g_signal_connect(G_OBJECT (entrybabel), "changed",
			(GCallback)changed_babel,
			NULL);
  }
*/
/*#endif*/
  create_hseparator(vbox);
  gtk_widget_show_all(frame);

}
#ifdef G_OS_WIN32
/********************************************************************************/
void  create_gamess_directory(GtkWidget *Wins,GtkWidget *vbox,gboolean expand)
{
  GtkWidget *frame;
  GtkWidget *button;

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox), frame, expand, expand, 0);
  gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);


  {
	GtkWidget* entry;
  	GtkWidget *table = gtk_table_new(1,3,FALSE);

	if(!gamessDirectory) gamessDirectory = g_strdup_printf("%s",g_get_home_dir());

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

	add_label_table(table,_("Gamess directory                        : "),0,0);
  	entry = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,1,1+1,0,0+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),gamessDirectory);
	gtk_editable_set_editable((GtkEditable*)entry,FALSE);
	gtk_widget_set_sensitive(entry, FALSE);
	button = create_button_pixmap(Wins,open_xpm,NULL);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(GTK_OBJECT (button), "clicked",
                                     G_CALLBACK(set_entry_gamessDir_selection),
                                     GTK_OBJECT(entry));
	add_widget_table(table,button,0,2);
  }
  gtk_widget_show_all(frame);
}
#endif
#ifdef G_OS_WIN32
/********************************************************************************/
void  create_demon_directory(GtkWidget *Wins,GtkWidget *vbox,gboolean expand)
{
  GtkWidget *frame;
  GtkWidget *button;

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox), frame, expand, expand, 0);
  gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);


  {
	GtkWidget* entry;
  	GtkWidget *table = gtk_table_new(1,3,FALSE);

	if(!demonDirectory) demonDirectory = g_strdup_printf("%s",g_get_home_dir());

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

	add_label_table(table,_("Demon directory                        : "),0,0);
  	entry = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,1,1+1,0,0+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),demonDirectory);
	gtk_editable_set_editable((GtkEditable*)entry,FALSE);
	gtk_widget_set_sensitive(entry, FALSE);
	button = create_button_pixmap(Wins,open_xpm,NULL);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(GTK_OBJECT (button), "clicked",
                                     G_CALLBACK(set_entry_demonDir_selection),
                                     GTK_OBJECT(entry));
	add_widget_table(table,button,0,2);
  }
  gtk_widget_show_all(frame);
}
#endif
#ifdef G_OS_WIN32
/********************************************************************************/
void  create_orca_directory(GtkWidget *Wins,GtkWidget *vbox,gboolean expand)
{
  GtkWidget *frame;
  GtkWidget *button;

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox), frame, expand, expand, 0);
  gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);


  {
	GtkWidget* entry;
  	GtkWidget *table = gtk_table_new(1,3,FALSE);

	if(!orcaDirectory) orcaDirectory = g_strdup_printf("%s",g_get_home_dir());

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

	add_label_table(table,_("Orca directory                        : "),0,0);
  	entry = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,1,1+1,0,0+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),orcaDirectory);
	gtk_editable_set_editable((GtkEditable*)entry,FALSE);
	gtk_widget_set_sensitive(entry, FALSE);
	button = create_button_pixmap(Wins,open_xpm,NULL);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(GTK_OBJECT (button), "clicked",
                                     G_CALLBACK(set_entry_orcaDir_selection),
                                     GTK_OBJECT(entry));
	add_widget_table(table,button,0,2);
  }
  gtk_widget_show_all(frame);
}
#endif
#ifdef G_OS_WIN32
/********************************************************************************/
void  create_nwchem_directory(GtkWidget *Wins,GtkWidget *vbox,gboolean expand)
{
  GtkWidget *frame;
  GtkWidget *button;

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox), frame, expand, expand, 0);
  gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);


  {
	GtkWidget* entry;
  	GtkWidget *table = gtk_table_new(1,3,FALSE);

	if(!nwchemDirectory) nwchemDirectory = g_strdup_printf("%s",g_get_home_dir());

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

	add_label_table(table,_("NWChem directory                        : "),0,0);
  	entry = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,1,1+1,0,0+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),nwchemDirectory);
	gtk_editable_set_editable((GtkEditable*)entry,FALSE);
	gtk_widget_set_sensitive(entry, FALSE);
	button = create_button_pixmap(Wins,open_xpm,NULL);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(GTK_OBJECT (button), "clicked",
                                     G_CALLBACK(set_entry_nwchemDir_selection),
                                     GTK_OBJECT(entry));
	add_widget_table(table,button,0,2);
  }
  gtk_widget_show_all(frame);
}
#endif
#ifdef G_OS_WIN32
/********************************************************************************/
void  create_psicode_directory(GtkWidget *Wins,GtkWidget *vbox,gboolean expand)
{
  GtkWidget *frame;
  GtkWidget *button;

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox), frame, expand, expand, 0);
  gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);


  {
	GtkWidget* entry;
  	GtkWidget *table = gtk_table_new(1,3,FALSE);

	if(!psicodeDirectory) psicodeDirectory = g_strdup_printf("%s",g_get_home_dir());

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

	add_label_table(table,_("Psicode directory                        : "),0,0);
  	entry = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,1,1+1,0,0+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),psicodeDirectory);
	gtk_editable_set_editable((GtkEditable*)entry,FALSE);
	gtk_widget_set_sensitive(entry, FALSE);
	button = create_button_pixmap(Wins,open_xpm,NULL);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(GTK_OBJECT (button), "clicked",
                                     G_CALLBACK(set_entry_psicodeDir_selection),
                                     GTK_OBJECT(entry));
	add_widget_table(table,button,0,2);
  }
  gtk_widget_show_all(frame);
}
#endif
#ifdef G_OS_WIN32
/********************************************************************************/
void  create_firefly_directory(GtkWidget *Wins,GtkWidget *vbox,gboolean expand)
{
  GtkWidget *frame;
  GtkWidget *button;

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox), frame, expand, expand, 0);
  gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);


  {
	GtkWidget* entry;
  	GtkWidget *table = gtk_table_new(1,3,FALSE);

	if(!fireflyDirectory) fireflyDirectory = g_strdup_printf("%s",g_get_home_dir());

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

	add_label_table(table,_("FireFly directory                        : "),0,0);
  	entry = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,1,1+1,0,0+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),fireflyDirectory);
	gtk_editable_set_editable((GtkEditable*)entry,FALSE);
	gtk_widget_set_sensitive(entry, FALSE);
	button = create_button_pixmap(Wins,open_xpm,NULL);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(GTK_OBJECT (button), "clicked",
                                     G_CALLBACK(set_entry_fireflyDir_selection),
                                     GTK_OBJECT(entry));
	add_widget_table(table,button,0,2);
  }
  gtk_widget_show_all(frame);
}
#endif
#ifdef G_OS_WIN32
/********************************************************************************/
void  create_mopac_directory(GtkWidget *Wins,GtkWidget *vbox,gboolean expand)
{
  GtkWidget *frame;
  GtkWidget *button;

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox), frame, expand, expand, 0);
  gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);


  {
	GtkWidget* entry;
  	GtkWidget *table = gtk_table_new(1,3,FALSE);

	if(!mopacDirectory) mopacDirectory = g_strdup_printf("%s",g_get_home_dir());

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

	add_label_table(table,_("Mopac directory                         : "),0,0);
  	entry = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,1,1+1,0,0+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),mopacDirectory);
	gtk_editable_set_editable((GtkEditable*)entry,FALSE);
	gtk_widget_set_sensitive(entry, FALSE);
	button = create_button_pixmap(Wins,open_xpm,NULL);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(GTK_OBJECT (button), "clicked",
                                     G_CALLBACK(set_entry_mopacDir_selection),
                                     GTK_OBJECT(entry));
	add_widget_table(table,button,0,2);
  }
  gtk_widget_show_all(frame);
}
/********************************************************************************/
void  create_povray_directory(GtkWidget *Wins,GtkWidget *vbox,gboolean expand)
{
  GtkWidget *frame;
  GtkWidget *button;

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox), frame, expand, expand, 0);
  gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);


  {
	GtkWidget* entry;
  	GtkWidget *table = gtk_table_new(1,3,FALSE);

	if(!povrayDirectory) povrayDirectory = g_strdup_printf("%s",g_get_home_dir());

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

	add_label_table(table,_("PovRay directory                         : "),0,0);
  	entry = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,1,1+1,0,0+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),povrayDirectory);
	gtk_editable_set_editable((GtkEditable*)entry,FALSE);
	gtk_widget_set_sensitive(entry, FALSE);
	button = create_button_pixmap(Wins,open_xpm,NULL);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(GTK_OBJECT (button), "clicked",
                                     G_CALLBACK(set_entry_povrayDir_selection),
                                     GTK_OBJECT(entry));
	add_widget_table(table,button,0,2);
  }
  gtk_widget_show_all(frame);
}
/********************************************************************************/
void  create_gauss_directory(GtkWidget *Wins,GtkWidget *vbox,gboolean expand)
{
  GtkWidget *frame;
  GtkWidget *button;

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox), frame, expand, expand, 0);
  gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);


  {
	GtkWidget* entry;
  	GtkWidget *table = gtk_table_new(1,3,FALSE);

	if(!gaussDirectory) gaussDirectory = g_strdup_printf("%s",g_get_home_dir());

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

	add_label_table(table,_("Gaussian directory                      : "),0,0);
  	entry = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,1,1+1,0,0+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),gaussDirectory);
	gtk_editable_set_editable((GtkEditable*)entry,FALSE);
	gtk_widget_set_sensitive(entry, FALSE);
	button = create_button_pixmap(Wins,open_xpm,NULL);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(GTK_OBJECT (button), "clicked",
                                     G_CALLBACK(set_entry_gaussDir_selection),
                                     GTK_OBJECT(entry));
	add_widget_table(table,button,0,2);
  }
  gtk_widget_show_all(frame);
}
#endif
/********************************************************************************/
void  create_openbabel_directory(GtkWidget *Wins,GtkWidget *vbox,gboolean expand)
{
  GtkWidget *frame;
  GtkWidget *button;

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox), frame, expand, expand, 0);
  gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);


  {
	GtkWidget* entry;
  	GtkWidget *table = gtk_table_new(1,3,FALSE);

	if(!openbabelDirectory) openbabelDirectory = g_strdup_printf("%s",g_get_home_dir());

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

	add_label_table(table,_("Open babel directory                         : "),0,0);
  	entry = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,1,1+1,0,0+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),openbabelDirectory);
	gtk_editable_set_editable((GtkEditable*)entry,FALSE);
	gtk_widget_set_sensitive(entry, FALSE);
	button = create_button_pixmap(Wins,open_xpm,NULL);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(GTK_OBJECT (button), "clicked",
                                     G_CALLBACK(set_entry_openbabelDir_selection),
                                     GTK_OBJECT(entry));
	add_widget_table(table,button,0,2);
  }
  gtk_widget_show_all(frame);
}
/********************************************************************************/
void AddPageColorSurf(GtkWidget *NoteBook)
{
  GtkWidget *Frame;
  GtkWidget *LabelOnglet;
  GtkWidget *LabelMenu;
  
  Frame= gtk_frame_new(NULL);
  gtk_container_set_border_width(GTK_CONTAINER(Frame), 10);
  LabelOnglet = gtk_label_new(_(" Surface colors "));
  LabelMenu = gtk_label_new(_("   Surface colors  "));
  gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),
                                Frame,
                                LabelOnglet, LabelMenu);
  create_color_surfaces(Wins,Frame);
  gtk_widget_show(Frame);
}
/********************************************************************************/
static void  set_network(GtkWidget *Button, gpointer data)
{
	if(!data)
  		defaultNetWorkProtocol = GABEDIT_NETWORK_FTP_RSH;
	else
  		defaultNetWorkProtocol = GABEDIT_NETWORK_SSH;

	set_sensitive_remote_frame(TRUE);
}
/********************************************************************************/
void AddPageFont(GtkWidget *NoteBook)
{
  GtkWidget *Frame;
  GtkWidget *LabelOnglet;
  GtkWidget *LabelMenu;
  
  Frame= gtk_frame_new(NULL);
  gtk_container_set_border_width(GTK_CONTAINER(Frame), 10);
  LabelOnglet = gtk_label_new(_(" Fonts/Colors "));
  LabelMenu = gtk_label_new(_(" Fonts/Colors "));
  gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),
                                Frame,
                                LabelOnglet, LabelMenu);
  create_font_color(Wins,Frame);
  gtk_widget_show_all(Frame);
}
/********************************************************************************/
void AddPageProp(GtkWidget *NoteBook)
{
  GtkWidget *Frame;
  GtkWidget *LabelOnglet;
  GtkWidget *LabelMenu;
  
  Frame= gtk_frame_new(NULL);
  gtk_container_set_border_width(GTK_CONTAINER(Frame), 10);
  gtk_widget_set_size_request(GTK_WIDGET(Frame), (gint)(ScreenHeight*0.6),  (gint)(ScreenHeight*0.3));
  LabelOnglet = gtk_label_new(_(" Properties of atoms "));
  LabelMenu = gtk_label_new(_(" Properties of atoms "));
  gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),
                                Frame,
                                LabelOnglet, LabelMenu);
  create_table_prop_in_window(Wins,Frame);
  gtk_widget_show_all(Frame);
}
#ifdef G_OS_WIN32
/********************************************************************************/
static void set_entry_pscpplinkdir(GtkWidget* dirSelector, gint response_id)
{
	gchar* dirname = NULL;
	GtkWidget *entry;
	if(response_id != GTK_RESPONSE_OK) return;
	dirname = gabedit_folder_chooser_get_current_folder(GABEDIT_FOLDER_CHOOSER(dirSelector));


	entry = (GtkWidget*)(g_object_get_data(G_OBJECT(dirSelector),"EntryFile"));	
	gtk_entry_set_text(GTK_ENTRY(entry),dirname);

	/*if(strcmp(dirname,pscpplinkDirectory)!=0)*/
	{
		gchar* t = g_strdup_printf("%s;%s",dirname,g_getenv("PATH"));
		g_setenv("PATH",t,TRUE);
		g_free(t);
	}

	if(pscpplinkDirectory) g_free(pscpplinkDirectory);
	pscpplinkDirectory = g_strdup(dirname);
}
/********************************************************************************/
static void set_entry_pscpplinkDir_selection(GtkWidget* entry)
{
	GtkWidget *dirSelector;
	dirSelector = selectionOfDir(set_entry_pscpplinkdir, _("Select pscp & plink folder"), GABEDIT_TYPEWIN_ORB); 
  	gtk_window_set_modal (GTK_WINDOW (dirSelector), TRUE);
  	g_signal_connect(G_OBJECT(dirSelector),"delete_event", (GCallback)gtk_widget_destroy,NULL);

	g_object_set_data(G_OBJECT (dirSelector), "EntryFile", entry);

	g_signal_connect (dirSelector, "response",  G_CALLBACK (set_entry_pscpplinkdir), GTK_OBJECT(dirSelector));
	g_signal_connect (dirSelector, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(dirSelector));

	gtk_widget_show(dirSelector);
}
/********************************************************************************/
void  create_pscpplink_directory(GtkWidget *Wins,GtkWidget *vbox,gboolean expand)
{
  GtkWidget *frame;
  GtkWidget *button;

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox), frame, expand, expand, 0);
  gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);


  {
	GtkWidget* entry;
  	GtkWidget *table = gtk_table_new(1,3,FALSE);

	if(!pscpplinkDirectory) pscpplinkDirectory = g_strdup_printf("%s",g_get_home_dir());

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

	add_label_table(table,_("pscp & plink directory                    : "),0,0);
  	entry = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,1,1+1,0,0+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),pscpplinkDirectory);
	gtk_editable_set_editable((GtkEditable*)entry,FALSE);
	gtk_widget_set_sensitive(entry, FALSE);
	button = create_button_pixmap(Wins,open_xpm,NULL);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(GTK_OBJECT (button), "clicked",
                                     G_CALLBACK(set_entry_pscpplinkDir_selection),
                                     GTK_OBJECT(entry));
	add_widget_table(table,button,0,2);
  }
  gtk_widget_show_all(frame);
}
#endif /* and G_OS_WIN32 for pscp and plink */
/********************************************************************************/
void  create_network_protocols(GtkWidget* Win,GtkWidget *vbox,gboolean expand)
{
  GtkWidget *frame;
  GtkWidget *ButtonFtpRsh;
  GtkWidget *ButtonSsh;
  GtkWidget *button;
  GtkWidget *vboxframe;
  GtkWidget *table = gtk_table_new(2,2,TRUE);
  gchar ftprsh[] = N_("FTP and Rsh protocols");
  gchar ssh[]    = N_("ssh protocol            ");

  frame = gtk_frame_new (_("Default NetWork protocol"));
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (vbox), frame, expand, expand, 0);
  gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

  vboxframe = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vboxframe);
  gtk_container_add (GTK_CONTAINER (frame), vboxframe);

  gtk_box_pack_start (GTK_BOX (vboxframe), table, expand, expand, 0);


  ButtonFtpRsh = gtk_radio_button_new_with_label( NULL,ftprsh);
  add_widget_table(table,ButtonFtpRsh,0,0);
  gtk_widget_show (ButtonFtpRsh);
  if(defaultNetWorkProtocol==GABEDIT_NETWORK_FTP_RSH)
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonFtpRsh), TRUE);
  else
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonFtpRsh), FALSE);

  button = create_button(Win,"  Help  ");
  add_widget_table(table,button,0,1);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(help_ftp_rsh),NULL);

  ButtonSsh = gtk_radio_button_new_with_label(
                       gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonFtpRsh)),
                       ssh); 
  add_widget_table(table,ButtonSsh,1,0);
  gtk_widget_show (ButtonSsh);
  if(defaultNetWorkProtocol==GABEDIT_NETWORK_SSH)
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonSsh), TRUE);
  else
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonSsh), FALSE);

  g_signal_connect(G_OBJECT (ButtonSsh), "clicked", G_CALLBACK(set_network), ButtonSsh);
  g_signal_connect(G_OBJECT (ButtonFtpRsh), "clicked", G_CALLBACK(set_network), NULL);
  button = create_button(Win,_("  Help  "));
  add_widget_table(table,button,1,1);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(help_ssh),NULL);

#ifdef G_OS_WIN32
#endif

  gtk_widget_show_all(frame);
}
/********************************************************************************/
void AddPageOthers(GtkWidget *NoteBook)
{
  GtkWidget *Frame;
  GtkWidget *LabelOnglet;
  GtkWidget *LabelMenu;
  GtkWidget *vbox;
  
  Frame= gtk_frame_new(NULL);
  gtk_container_set_border_width(GTK_CONTAINER(Frame), 10);
  /* gtk_widget_set_size_request(GTK_WIDGET(Frame), (gint)(ScreenHeight*0.6),  (gint)(ScreenHeight*0.3));*/
  LabelOnglet = gtk_label_new(_(" Others "));
  LabelMenu = gtk_label_new(_(" Others "));
  gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook), Frame, LabelOnglet, LabelMenu);
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (Frame), vbox);

#ifdef G_OS_WIN32
  create_demon_directory(Wins,vbox,FALSE);
  create_gamess_directory(Wins,vbox,FALSE);
  create_orca_directory(Wins,vbox,FALSE);
  create_nwchem_directory(Wins,vbox,FALSE);
  create_psicode_directory(Wins,vbox,FALSE);
  create_firefly_directory(Wins,vbox,FALSE);
  create_mopac_directory(Wins,vbox,FALSE);
  create_gauss_directory(Wins,vbox,FALSE);
  create_povray_directory(Wins,vbox,FALSE);
#endif
  create_openbabel_directory(Wins,vbox,FALSE);

  create_opengl_frame(Wins,vbox);
  gtk_widget_show_all(Frame);
}
/********************************************************************************/
void AddPageCommands(GtkWidget *NoteBook)
{
  GtkWidget *Frame;
  GtkWidget *LabelOnglet;
  GtkWidget *LabelMenu;
  GtkWidget *vbox;
  
  Frame= gtk_frame_new(NULL);
  gtk_container_set_border_width(GTK_CONTAINER(Frame), 10);
  /* gtk_widget_set_size_request(GTK_WIDGET(Frame), (gint)(ScreenHeight*0.6),  (gint)(ScreenHeight*0.3));*/
  LabelOnglet = gtk_label_new(_(" Commands "));
  LabelMenu = gtk_label_new(_(" Commands "));
  gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),
                                Frame,
                                LabelOnglet, LabelMenu);
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (Frame), vbox);

  create_execucte_commands(Wins,vbox,FALSE);
  gtk_widget_show_all(Frame);
}
/********************************************************************************/
void AddPageBatch(GtkWidget *NoteBook)
{
  GtkWidget *Frame;
  GtkWidget *LabelOnglet;
  GtkWidget *LabelMenu;
  GtkWidget *vbox;
  
  Frame= gtk_frame_new(NULL);
  gtk_container_set_border_width(GTK_CONTAINER(Frame), 10);
  /* gtk_widget_set_size_request(GTK_WIDGET(Frame), (gint)(ScreenHeight*0.6),  (gint)(ScreenHeight*0.3));*/
  LabelOnglet = gtk_label_new(_(" Batch "));
  LabelMenu = gtk_label_new(_(" Batch "));
  gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),
                                Frame,
                                LabelOnglet, LabelMenu);
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (Frame), vbox);

  create_batch_commands(Wins,vbox,FALSE);
  gtk_widget_show_all(Frame);
}
/********************************************************************************/
void AddPageNetWork(GtkWidget *NoteBook)
{
  GtkWidget *Frame;
  GtkWidget *LabelOnglet;
  GtkWidget *LabelMenu;
  GtkWidget *vbox;
  
  Frame= gtk_frame_new(NULL);
  gtk_container_set_border_width(GTK_CONTAINER(Frame), 10);
  /* gtk_widget_set_size_request(GTK_WIDGET(Frame), (gint)(ScreenHeight*0.6),  (gint)(ScreenHeight*0.3));*/
  LabelOnglet = gtk_label_new(_(" NetWork "));
  LabelMenu = gtk_label_new(_(" NetWork "));
  gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),
                                Frame,
                                LabelOnglet, LabelMenu);
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (Frame), vbox);

#ifdef G_OS_WIN32
  create_pscpplink_directory(Wins,vbox,FALSE);
#endif
  create_network_protocols(Wins,vbox,FALSE);
  gtk_widget_show_all(Frame);
}
/********************************************************************************/
void create_preferences()
{
GtkWidget *NoteBook;
  GtkWidget *button;

  if(instal)
  	Wins= gtk_dialog_new ();
  else
  {
  	if(!Wins)
  		Wins= gtk_dialog_new ();
  	else
  	{
	  	gtk_widget_hide(Wins);
	 	 gtk_widget_show(Wins);
	  	return;
  	}
  }
  instal = FALSE;
  gtk_window_set_position(GTK_WINDOW(Wins),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Wins),GTK_WINDOW(Fenetre));
  gtk_window_set_default_size (GTK_WINDOW(Wins), (gushort)(2.9*ScreenWidth/5), (gushort)(2.9*ScreenHeight/5));
/*  gtk_window_set_modal (GTK_WINDOW (Wins), TRUE);*/

  /* Connection des signaux "delete" et "destroy" */
  init_child(Wins,gtk_widget_destroy,_("Preferences "));
  g_signal_connect(G_OBJECT(Wins),"delete_event",(GCallback)destroy_preferences_window,NULL);

  gtk_window_set_title(&GTK_DIALOG(Wins)->window,_("Preferences"));
 
  /* NoteBook Options */
  NoteBook = gtk_notebook_new();
  gtk_box_pack_start(GTK_BOX (GTK_DIALOG(Wins)->vbox), NoteBook,TRUE, TRUE, 0);
  AddPageProp(NoteBook);
  AddPageFont(NoteBook);
  gtk_widget_show(NoteBook);
  gtk_widget_show(GTK_WIDGET(GTK_DIALOG(Wins)->vbox));

  AddPageColorSurf(NoteBook);

  AddPageCommands(NoteBook);
  AddPageNetWork(NoteBook);
  AddPageBatch(NoteBook);

  AddPageOthers(NoteBook);
  
  gtk_widget_realize(Wins);

  button = create_button(Wins,"Close");
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Wins)->action_area), button, TRUE, TRUE, 0);
  g_signal_connect_swapped(GTK_OBJECT(button), "clicked",(GCallback)destroy_preferences_window,GTK_OBJECT(Wins));
  gtk_widget_show_all (button);

  button = create_button(Wins,_("Save&Apply&Close"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Wins)->action_area), button, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(apply_all),NULL);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(create_ressource_file),NULL);
  g_signal_connect_swapped(GTK_OBJECT(button), "clicked",(GCallback)destroy_preferences_window,GTK_OBJECT(Wins));
  gtk_widget_grab_default(button);
  gtk_widget_show_all (button);

  button = create_button(Wins,_("Apply&Close"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Wins)->action_area), button, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(apply_all),NULL);
  g_signal_connect_swapped(GTK_OBJECT(button), "clicked",(GCallback)destroy_preferences_window,GTK_OBJECT(Wins));
  gtk_widget_grab_default(button);
  gtk_widget_show_all (button);

  button = create_button(Wins,_("Apply"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Wins)->action_area), button, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(apply_all),NULL);
  gtk_widget_grab_default(button);
  gtk_widget_show_all (button);


  gtk_widget_show(Wins);
  
}
/********************************************************************************/
static void  setNumberOfSubdivisions(GtkWidget *Entry, gpointer data)
{
	static gchar* typeEntry[] = {"CYLINDER","SPHERE"};
	G_CONST_RETURN gchar* textEntry = gtk_entry_get_text(GTK_ENTRY(Entry)); 
	gint value = 0;
	if(textEntry && strlen(textEntry)>0) value = atof(textEntry);
	if(value<1)return;
	if(strcmp((gchar*)data,typeEntry[0])==0)
	{
		openGLOptions.numberOfSubdivisionsCylindre = value;
	}
	else
	{
		openGLOptions.numberOfSubdivisionsSphere = value;
	}
}
/********************************************************************************/
static void  set_opengl(GtkWidget *Button, gpointer data)
{
	gchar* typeButton[] = {"RGBA","DOUBLEBUFFER" ,"ALPHASIZE","DEPTHSIZE","Activate Text"};
	gint i;
	for(i=0;i<5;i++)
		if(strcmp((gchar*)data,typeButton[i])==0)
		{
			switch(i)
			{
				case 0: 
					if(openGLOptions.rgba==0) 
						openGLOptions.rgba=1;
					else 
						openGLOptions.rgba=0;
					break;
				case 1: 
					if(openGLOptions.doubleBuffer==0)
						openGLOptions.doubleBuffer = 1;
					else
						openGLOptions.doubleBuffer = 0;
					break;
				case 2: 
					if(openGLOptions.alphaSize==0)
						openGLOptions.alphaSize = 1;
					else
						openGLOptions.alphaSize = 0;
					break;
				case 3: 
					if(openGLOptions.depthSize==0)
						openGLOptions.depthSize = 1;
					else
						openGLOptions.depthSize = 0;
					break;
				case 4: 
					if(openGLOptions.activateText==0)
						openGLOptions.activateText = 1;
					else
						openGLOptions.activateText = 0;
					break;
			}
			break;
		}
}
/********************************************************************************/
void  create_opengl_frame(GtkWidget* Win,GtkWidget *vbox)
{
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget* buttonRGBA;
	GtkWidget* buttonALPHASIZE;
	GtkWidget* buttonDEPTHSIZE;
	GtkWidget* buttonDOUBLEBUFFER;
	GtkWidget* buttonActivateText;
	GtkWidget* EntryCylinder;
	GtkWidget* EntrySphere;
	GtkWidget* combo;
	GtkWidget *table = gtk_table_new(2,6,FALSE);
	static gchar* typeButton[] = {"RGBA","DOUBLEBUFFER" ,"ALPHASIZE","DEPTHSIZE","Activate Text"};
	static gchar* values[] = {"5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24","25","26","27","28","29","30","40","50","60","80","100"};
	static gchar* typeEntry[] = {"CYLINDER","SPHERE"};
	gushort i;
	gint nv = sizeof(values)/sizeof(gchar*);


	frame = gtk_frame_new (_("OpenGL Options"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxframe = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vboxframe);
	gtk_container_add (GTK_CONTAINER (frame), vboxframe);

	gtk_box_pack_start (GTK_BOX (vboxframe), table, TRUE, TRUE, 0);

/* ------------------------------------------------------------------*/
	i = 0;
	add_label_table(table,_(" Number Of Subdivisions for a Cylinder "),i,0);
	add_label_table(table," : ",i,1);
	combo = create_combo_box_entry(values,nv,TRUE,-1,-1);
	EntryCylinder = GTK_BIN(combo)->child;
	add_widget_table(table,combo,i,2);
	gtk_editable_set_editable((GtkEditable*)EntryCylinder,FALSE);
	gtk_entry_set_text (GTK_ENTRY (EntryCylinder),g_strdup_printf("%d",openGLOptions.numberOfSubdivisionsCylindre));
	g_signal_connect (G_OBJECT (EntryCylinder), "changed",(GCallback)setNumberOfSubdivisions,typeEntry[0]);
/* ------------------------------------------------------------------*/
	i = 1;
	add_label_table(table,_(" Number Of Subdivisions for a Sphere "),i,0);
	add_label_table(table," : ",i,1);
	combo = create_combo_box_entry(values,nv,TRUE,-1,-1);
	EntrySphere = GTK_BIN(combo)->child;
	add_widget_table(table,combo,i,2);
	gtk_editable_set_editable((GtkEditable*)EntrySphere,FALSE);
	gtk_entry_set_text (GTK_ENTRY (EntrySphere),g_strdup_printf("%d",openGLOptions.numberOfSubdivisionsSphere));
	g_signal_connect (G_OBJECT (EntrySphere), "changed",(GCallback)setNumberOfSubdivisions,typeEntry[1]);
/* ------------------------------------------------------------------*/

	buttonRGBA = gtk_check_button_new_with_label("RGBA" );
	add_widget_table(table,buttonRGBA,0,3);
	gtk_widget_show (buttonRGBA);
	if(openGLOptions.rgba!=0)
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonRGBA), TRUE);
	else
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonRGBA), FALSE);

	buttonDOUBLEBUFFER = gtk_check_button_new_with_label("DOUBLEBUFFER" );
	add_widget_table(table,buttonDOUBLEBUFFER,1,3);
	gtk_widget_show (buttonDOUBLEBUFFER);
	if(openGLOptions.doubleBuffer!=0)
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonDOUBLEBUFFER), TRUE);
	else
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonDOUBLEBUFFER), FALSE);

	buttonALPHASIZE = gtk_check_button_new_with_label("ALPHASIZE=1(0 if not)" );
	add_widget_table(table,buttonALPHASIZE,0,4);
	gtk_widget_show (buttonALPHASIZE);
	if(openGLOptions.alphaSize!=0)
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonALPHASIZE), TRUE);
	else
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonALPHASIZE), FALSE);

	buttonDEPTHSIZE = gtk_check_button_new_with_label("DEPTHSIZE=1(0 if not)" );
	add_widget_table(table,buttonDEPTHSIZE,1,4);
	gtk_widget_show (buttonDEPTHSIZE);
	if(openGLOptions.depthSize!=0)
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonDEPTHSIZE), TRUE);
	else
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonDEPTHSIZE), FALSE);

	buttonActivateText = gtk_check_button_new_with_label("Activate Text" );
	add_widget_table(table,buttonActivateText,0,5);
	gtk_widget_show (buttonActivateText);
	if(openGLOptions.activateText!=0)
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonActivateText), TRUE);
	else
  		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonActivateText), FALSE);

	g_signal_connect (G_OBJECT (buttonRGBA), "clicked", G_CALLBACK(set_opengl), typeButton[0]);
	g_signal_connect (G_OBJECT (buttonDOUBLEBUFFER), "clicked", G_CALLBACK(set_opengl), typeButton[1]);
	g_signal_connect (G_OBJECT (buttonALPHASIZE), "clicked", G_CALLBACK(set_opengl), typeButton[2]);
	g_signal_connect (G_OBJECT (buttonDEPTHSIZE), "clicked", G_CALLBACK(set_opengl), typeButton[3]);
	g_signal_connect (G_OBJECT (buttonActivateText), "clicked", G_CALLBACK(set_opengl), typeButton[4]);
	gtk_widget_show_all(frame);
}
/********************************************************************************/
static void apply_set_opacity(GtkWidget *Win,gpointer data)
{
	GtkWidget* opacitySpinButton = NULL;
	gdouble a = 0;

	if(!GTK_IS_WIDGET(Win)) return;

	opacitySpinButton = g_object_get_data (G_OBJECT (Win), "OpacitySpinButton");

	a = gtk_spin_button_get_value (GTK_SPIN_BUTTON(opacitySpinButton));
	if(a>=0 && a<=100) alpha_opacity = a/100;
	rafresh_window_orb();
}
/********************************************************************************/
static void apply_set_opacity_close(GtkWidget *Win,gpointer data)
{
	apply_set_opacity(Win,data);
	delete_child(Win);
}
/********************************************************************************/
static GtkWidget *add_spin_button( GtkWidget *table, gchar* strLabel, gint il)
{
	gushort i;
	gushort j;
	GtkWidget *spinButton;
	GtkWidget *label;

/*----------------------------------------------------------------------------------*/
	i = il;
	j = 0;
	label = gtk_label_new(strLabel);
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = il;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = il;
	j = 2;
	spinButton =  gtk_spin_button_new_with_range (0, 100, 1);

	gtk_table_attach(GTK_TABLE(table),spinButton,
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);

  	return spinButton;
}
/********************************************************************************/
void set_opacity_dlg()
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *hbox;
	GtkWidget *table;
	GtkWidget *vboxall;
	GtkWidget *opacitySpinButton;
	GtkWidget *button;

	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),_("Set opacity"));
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Opacity ");

	vboxall = create_vbox(Win);
	frame = gtk_frame_new (NULL);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	table = gtk_table_new(5,3,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);

	opacitySpinButton = add_spin_button( table, _("Opacity coefficient : "), 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(opacitySpinButton),(gint)(alpha_opacity*100));
	g_object_set_data (G_OBJECT (Win), "OpacitySpinButton",opacitySpinButton);

	hbox = create_hbox_false(vboxall);
	gtk_widget_realize(Win);

	button = create_button(Win,_("OK"));
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_set_opacity_close,G_OBJECT(Win));

	button = create_button(Win,_("Apply"));
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_set_opacity,G_OBJECT(Win));

	button = create_button(Win,_("Cancel"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, G_OBJECT(Win));
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,G_OBJECT(Win));
	gtk_widget_show (button);

	gtk_widget_show_all (Win);
}
/*********************************************************************************************************************/
static void applyColorMapOptions(GtkWidget *dialogWindow, gpointer data)
{
	GtkWidget* buttonMultiColor;
	GtkWidget* button2Colors;
	/* GtkWidget* buttonUniColor;*/
	GtkWidget* selectorUniColor;
	GtkWidget* selector2Colors1;
	GtkWidget* selector2Colors2;
	 
	if(!GTK_IS_WIDGET(dialogWindow)) return;
	buttonMultiColor = g_object_get_data(G_OBJECT (dialogWindow), "ButtonMultiColor");
	button2Colors = g_object_get_data(G_OBJECT (dialogWindow), "Button2Colors");
	/* buttonUniColor = g_object_get_data(G_OBJECT (dialogWindow), "ButtonUniColor");*/
	selectorUniColor = g_object_get_data(G_OBJECT (dialogWindow), "SelectorUniColor");
	selector2Colors1 = g_object_get_data(G_OBJECT (dialogWindow), "Selector2Colors1");
	selector2Colors2 = g_object_get_data(G_OBJECT (dialogWindow), "Selector2Colors2");

	if(!buttonMultiColor) return;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(buttonMultiColor)))
	{

		colorMapType = 1;
	}
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button2Colors)) && selector2Colors1 && selector2Colors2)
	{
		GdkColor color;
		colorMapType = 2;
		gtk_color_button_get_color (GTK_COLOR_BUTTON(selector2Colors1), &color);
		colorMapColors[0][0] = color.red/65535.0;
		colorMapColors[0][1] = color.green/65535.0;
		colorMapColors[0][2] = color.blue/65535.0;
		gtk_color_button_get_color (GTK_COLOR_BUTTON(selector2Colors2), &color);
		colorMapColors[1][0] = color.red/65535.0;
		colorMapColors[1][1] = color.green/65535.0;
		colorMapColors[1][2] = color.blue/65535.0;

	}
	else if(selectorUniColor)
	{
		GdkColor color;
		colorMapType = 3;
		gtk_color_button_get_color (GTK_COLOR_BUTTON(selectorUniColor), &color);
		colorMapColors[2][0] = color.red/65535.0;
		colorMapColors[2][1] = color.green/65535.0;
		colorMapColors[2][2] = color.blue/65535.0;
	}
	resetAllColorMapOrb();
}
/*********************************************************************************************************************/
static GtkWidget* addRadioButtonColorMapToATable(GtkWidget* table, GtkWidget* friendButton, gchar* label, gint i, gint j, gint k)
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
/*********************************************************************************************************************/
static void createColorMapOptionsFrame(GtkWidget* dialogWindow, GtkWidget *box)
{
	GtkWidget* button;
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget *table = gtk_table_new(3,3,TRUE);
	gint i;
	GtkWidget *selector;
	GdkColor color;


	color.red = 65535;
	color.green = 65535;
	color.blue = 65535;
	frame = gtk_frame_new (_("Color mapping type"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	i = 0;
	button =  NULL;
	button = addRadioButtonColorMapToATable(table, button, _("Multi color"), i, 0,1);
	if(colorMapType == 1) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
	g_object_set_data(G_OBJECT (dialogWindow), "ButtonMultiColor",button);

	i = 1;
	button = addRadioButtonColorMapToATable(table, button, _("2 colors"), i, 0,1);
	if(colorMapType == 2) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
	g_object_set_data(G_OBJECT (dialogWindow), "Button2Colors",button);

	color.red = (gushort)(colorMapColors[0][0]*65535);
	color.green = (gushort)(colorMapColors[0][1]*65535);
	color.blue = (gushort)(colorMapColors[0][2]*65535);
	selector = gtk_color_button_new_with_color (&color);
	gtk_table_attach(GTK_TABLE(table),selector,1,2,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);
	g_object_set_data(G_OBJECT (dialogWindow), "Selector2Colors1",selector);

	color.red = (gushort)(colorMapColors[1][0]*65535);
	color.green = (gushort)(colorMapColors[1][1]*65535);
	color.blue = (gushort)(colorMapColors[1][2]*65535);
	selector = gtk_color_button_new_with_color (&color);
	gtk_table_attach(GTK_TABLE(table),selector,2,3,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);
	g_object_set_data(G_OBJECT (dialogWindow), "Selector2Colors2",selector);

	i = 2;
	button = addRadioButtonColorMapToATable(table, button, _("Unicolor"), i, 0,1);
	if(colorMapType == 3) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);

	color.red = (gushort)(colorMapColors[2][0]*65535);
	color.green = (gushort)(colorMapColors[2][1]*65535);
	color.blue = (gushort)(colorMapColors[2][2]*65535);
	selector = gtk_color_button_new_with_color (&color);
	gtk_table_attach(GTK_TABLE(table),selector,1,2,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);
	g_object_set_data(G_OBJECT (dialogWindow), "SelectorUniColor",selector);
	g_object_set_data(G_OBJECT (dialogWindow), "ButtonUniColor",button);

}
/****************************************************************************************************/
void createColorMapOptionsWindow(GtkWidget* win)
{
	GtkWidget *dialogWindow = NULL;
	GtkWidget *button;
	GtkWidget *frame;
	GtkWidget *hbox;
	gchar title[BSIZE];
	 
	dialogWindow = gtk_dialog_new();
	gtk_widget_realize(GTK_WIDGET(dialogWindow));
	sprintf(title, _("Color Mapping options"));
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

	createColorMapOptionsFrame(dialogWindow,hbox);
	gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(dialogWindow)->action_area), TRUE);

	button = create_button(dialogWindow,_("Close"));
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(dialogWindow)->action_area), button, FALSE, TRUE, 5);	
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)destroy_button_windows, GTK_OBJECT(dialogWindow));
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(dialogWindow));

	button = create_button(dialogWindow,_("Apply"));
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(dialogWindow)->action_area), button, FALSE, TRUE, 5);	
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)applyColorMapOptions, GTK_OBJECT(dialogWindow));

	button = create_button(dialogWindow,_("OK"));
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(dialogWindow)->action_area), button, FALSE, TRUE, 5);	
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)applyColorMapOptions, GTK_OBJECT(dialogWindow));
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)destroy_button_windows, GTK_OBJECT(dialogWindow));
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(dialogWindow));
	

	add_button_windows(title,dialogWindow);

	gtk_widget_show_all(dialogWindow);
	if(GTK_IS_WIDGET(win)) gtk_window_set_transient_for(GTK_WINDOW(dialogWindow),GTK_WINDOW(win));
}
