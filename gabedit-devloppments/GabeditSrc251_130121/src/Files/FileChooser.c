/* FileChooser.c */
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
#include <GL/gl.h>
#include <GL/glu.h>
#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Common/Windows.h"
#include "../Common/Exit.h"
#include "../Display/UtilsOrb.h"


/***************************************************************************************************************************/
GtkWidget* file_chooser(gpointer data,gchar* title,GabEditTypeFile type,GabEditTypeWin typewin, GtkFileChooserAction action) 
{
  GtkWidget *gabeditFileChooser;
  gchar* patternsfiles[] = {	"*",
			    	"*.inp","*.com","*.mop","*.nw","*.psi",
	  			"*.log","*.out","*.fchk", "*.aux","*.wfx", "*.gab","*.ici","*.xyz","*.mol2","*.mol","*.tnk","*.pdb","*.hin","*.zmt","*.gzmt",
	  		    	"*.hf","*.gcube","*.cube","*.CUBE","*.grid","*.M2Msi","*.t41","*.dx","*.trj","*.irc","*.txt","*.xml","*.cif","*",
			    	NULL};
  GCallback *func = (GCallback *)data;
  gchar* temp = NULL;

  if(title)
  	gabeditFileChooser = gabedit_file_chooser_new(title, action);
  else
  {
	  if(action == GTK_FILE_CHOOSER_ACTION_OPEN) gabeditFileChooser = gabedit_file_chooser_new(_("Open file"), action);
	  else gabeditFileChooser = gabedit_file_chooser_new(_("Save file"), action);
  }

  if(type== GABEDIT_TYPEFILE_CUBEMOLPRO)
  {
	  temp=patternsfiles[6];
	  patternsfiles[6]=patternsfiles[7];
	  patternsfiles[7]=temp;
	  temp = NULL;
  };
  gabedit_file_chooser_hide_hidden(GABEDIT_FILE_CHOOSER(gabeditFileChooser));

   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(gabeditFileChooser), patternsfiles);
   switch(type)
   {
	   case GABEDIT_TYPEFILE_DALTON : 
					   gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.out");
					   temp = g_strdup_printf("%s.out",fileopen.projectname);
					   break;
	   case GABEDIT_TYPEFILE_DEMON : 
					   gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.out");
					   temp = g_strdup_printf("%s.out",fileopen.projectname);
					   break;
	   case GABEDIT_TYPEFILE_GAMESS : 
					   gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.log");
					   temp = g_strdup_printf("%s.log",fileopen.projectname);
					   break;
	   case GABEDIT_TYPEFILE_FIREFLY : 
					   gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.log");
					   temp = g_strdup_printf("%s.log",fileopen.projectname);
					   break;
	   case GABEDIT_TYPEFILE_GAUSSIAN : 
		   				gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.log");
						temp = g_strdup_printf("%s.log",fileopen.projectname);
					  	break;
	   case GABEDIT_TYPEFILE_MOLCAS : 
					   gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.out");
						temp = g_strdup_printf("%s.out",fileopen.projectname);
					   break;
	   case GABEDIT_TYPEFILE_MOLPRO : 
					    if(strstr(title,"geometry"))
					    	gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.out");
					    else
					    	gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.out");

					    temp = g_strdup_printf("%s.out",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_MOLPRO_LOG : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.log");

					    temp = g_strdup_printf("%s.log",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_ORCA : 
		   				gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.out");
						temp = g_strdup_printf("%s.out",fileopen.projectname);
						break;
	   case GABEDIT_TYPEFILE_VASPOUTCAR : 
		   				gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*");
						temp = g_strdup_printf("OUTCAR");
						break;
	   case GABEDIT_TYPEFILE_VASPXML : 
		   				gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.xml");
						temp = g_strdup_printf("%s.xml",fileopen.projectname);
						break;
	   case GABEDIT_TYPEFILE_VASPPOSCAR : 
		   				gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*");
						temp = g_strdup_printf("POSCAR");
						break;
	   case GABEDIT_TYPEFILE_CIF : 
		   				gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.cif");
						temp = g_strdup_printf("%s.cif", fileopen.projectname);
						break;
	   case GABEDIT_TYPEFILE_QCHEM : 
		   				gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.out");
						temp = g_strdup_printf("%s.out",fileopen.projectname);
					  	break;
	   case GABEDIT_TYPEFILE_NWCHEM : 
		   				gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.out");
						temp = g_strdup_printf("%s.out",fileopen.projectname);
					  	break;
	   case GABEDIT_TYPEFILE_PSICODE : 
		   				gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.out");
						temp = g_strdup_printf("%s.out",fileopen.projectname);
					  	break;
	   case GABEDIT_TYPEFILE_MOPAC : 
		   				gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.out");
						temp = g_strdup_printf("%s.out",fileopen.projectname);
					  	break;
	   case GABEDIT_TYPEFILE_MOPAC_AUX : 
		   				gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.aux");
						temp = g_strdup_printf("%s.aux",fileopen.projectname);
					  	break;
	   case GABEDIT_TYPEFILE_WFX : 
		   				gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.wfx");
						temp = g_strdup_printf("%s.wfx",fileopen.projectname);
					  	break;
	   case GABEDIT_TYPEFILE_ADF : 
		   				gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.out");
						temp = g_strdup_printf("%s.out",fileopen.projectname);
					  	break;
	   case GABEDIT_TYPEFILE_MOLDEN : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.molden");
					    temp = g_strdup_printf("%s.molden",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_GABEDIT : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.gab");
					    temp = g_strdup_printf("%s.gab",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_MOBCAL : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.mfj");
					    temp = g_strdup_printf("%s.mfj",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_MPQC : 
		   				gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.out");
						temp = g_strdup_printf("%s.out",fileopen.projectname);
					  	break;
	   case GABEDIT_TYPEFILE_XYZ : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.xyz");
					    temp = g_strdup_printf("%s.xyz",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_MOL2 : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.mol2");
					    temp = g_strdup_printf("%s.mol2",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_MOL : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.mol");
					    temp = g_strdup_printf("%s.mol",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_TINKER : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.tnk");
					    temp = g_strdup_printf("%s.tnk",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_PDB : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.pdb");
					    temp = g_strdup_printf("%s.pdb",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_HIN : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.hin");
					    temp = g_strdup_printf("%s.hin",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_AIMALL : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.sum");
					    temp = g_strdup_printf("%s.sum",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_GZMAT : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.gzmt");
					    temp = g_strdup_printf("%s.gzmt",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_MZMAT : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.zmt");
					    temp = g_strdup_printf("%s.zmt",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_CCHEMI : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.ici");
					    temp = g_strdup_printf("%s.ici",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_DEMONINPUT : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.inp");
					    temp = g_strdup_printf("%s.inp",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_GAMESSINPUT : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.inp");
					    temp = g_strdup_printf("%s.inp",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_FIREFLYINPUT : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.inp");
					    temp = g_strdup_printf("%s.inp",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_GAUSSIANINPUT : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.com");
					    temp = g_strdup_printf("%s.com",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_GAUSSIAN_FCHK : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.fchk");
					    temp = g_strdup_printf("%s.fchk",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_MOLCASINPUT : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.com");
					    temp = g_strdup_printf("%s.com",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_MOLPROINPUT : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.com");
					    temp = g_strdup_printf("%s.com",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_ORCAINPUT : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.inp");
					    temp = g_strdup_printf("%s.inp",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_QCHEMINPUT : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.inp");
					    temp = g_strdup_printf("%s.inp",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_NWCHEMINPUT : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.nw");
					    temp = g_strdup_printf("%s.nw",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_PSICODEINPUT : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.psi");
					    temp = g_strdup_printf("%s.psi",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_MOPACINPUT : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.mop");
					    temp = g_strdup_printf("%s.mop",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_JPEG : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.jpg");
					    temp = g_strdup_printf("%s.jpg",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_PPM : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.ppm");
					    temp = g_strdup_printf("%s.ppm",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_BMP : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.bmp");
					    temp = g_strdup_printf("%s.bmp",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_PNG : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.png");
					    temp = g_strdup_printf("%s.png",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_TIF : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.tif");
					    temp = g_strdup_printf("%s.tif",fileopen.projectname);
					    break;
	   case GABEDIT_TYPEFILE_PS : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.ps");
					    temp = g_strdup_printf("%s.ps",fileopen.projectname);
				      	    break;
	   case GABEDIT_TYPEFILE_CUBEGAUSS : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.cube");
					    temp = g_strdup_printf("%s.cube",fileopen.projectname);
				      	    break;
	   case GABEDIT_TYPEFILE_CUBEMOLPRO : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.CUBE");
					    temp = g_strdup_printf("%s.CUBE",fileopen.projectname);
				      	    break;
	   case GABEDIT_TYPEFILE_CUBEADF : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.t41");
					    temp = g_strdup_printf("%s.t41",fileopen.projectname);
				      	    break;
	   case GABEDIT_TYPEFILE_MOLCASGRID : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.grid");
					    temp = g_strdup_printf("%s.grid",fileopen.projectname);
				      	    break;
	   case GABEDIT_TYPEFILE_CUBEMOLCAS : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.cube");
					    temp = g_strdup_printf("%s.cube",fileopen.projectname);
				      	    break;
	   case GABEDIT_TYPEFILE_CUBEQCHEM : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.hf");
					    temp = g_strdup_printf("%s.hf",fileopen.projectname);
				      	    break;
	   case GABEDIT_TYPEFILE_CUBEGABEDIT : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.gcube");
					    temp = g_strdup_printf("%s.gcube",fileopen.projectname);
				      	    break;
	   case GABEDIT_TYPEFILE_CUBEDX : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.dx");
					    temp = g_strdup_printf("%s.dx",fileopen.projectname);
				      	    break;
	   case GABEDIT_TYPEFILE_TRJ : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.trj");
					    temp = g_strdup_printf("%s.trj",fileopen.projectname);
				      	    break;
	   case GABEDIT_TYPEFILE_GAMESSIRC : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.irc");
					    temp = g_strdup_printf("%s.irc",fileopen.projectname);
				      	    break;
	   case GABEDIT_TYPEFILE_XML : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.xml");
					    temp = g_strdup_printf("%s.xml",fileopen.projectname);
				      	    break;
	   case GABEDIT_TYPEFILE_TXT : 
					    gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.txt");
					    temp = g_strdup_printf("%s.txt",fileopen.projectname);
				      	    break;
	default :break;
   }
  if(temp != NULL)
  {
	if(lastdirectory)
	{
		gchar* t = g_strdup_printf("%s%s%s",lastdirectory,G_DIR_SEPARATOR_S,temp);
		gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(gabeditFileChooser),t);
		g_free(t);
		g_free(temp);
	}
	else
	{
		if(fileopen.localdir)
		{
			gchar* t = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,temp);
			gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(gabeditFileChooser),t);
			g_free(t);
			g_free(temp);
		}
		else
		{
			gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(gabeditFileChooser),temp);
			g_free(temp);
		}
	}
  }
  else
  {
	if(lastdirectory)
	{
		gchar* t = g_strdup_printf("%s%sdump.txt",lastdirectory,G_DIR_SEPARATOR_S);
		gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(gabeditFileChooser),t);
		g_free(t);
	}
	else
		if(fileopen.localdir)
		{
			gchar* t = g_strdup_printf("%s%sdump.txt",fileopen.localdir,G_DIR_SEPARATOR_S);
			gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(gabeditFileChooser),t);
			g_free(t);
		}
  }
  	if(typewin==GABEDIT_TYPEWIN_ORB)
	{
	  if(action == GTK_FILE_CHOOSER_ACTION_OPEN) add_glarea_child(gabeditFileChooser,_("Open file "));
	  else add_glarea_child(gabeditFileChooser,_("Save file "));
	}
	else
	{
  		add_button_windows(_(" Open file "),gabeditFileChooser);
  		g_signal_connect(G_OBJECT(gabeditFileChooser), "delete_event",(GCallback)destroy_button_windows,NULL);
	}

  g_signal_connect (gabeditFileChooser, "response",  G_CALLBACK (func),GTK_OBJECT(gabeditFileChooser));

  if(typewin==GABEDIT_TYPEWIN_ORB)
  	g_signal_connect (gabeditFileChooser, "response", G_CALLBACK (delete_child), GTK_OBJECT(gabeditFileChooser));
  else
  	g_signal_connect (gabeditFileChooser, "response", G_CALLBACK (destroy_button_windows),GTK_OBJECT(gabeditFileChooser));
  g_signal_connect_after (gabeditFileChooser, "response", G_CALLBACK (gtk_widget_destroy),GTK_OBJECT(gabeditFileChooser));
  
  gtk_widget_show(gabeditFileChooser);

  return gabeditFileChooser;

}
/***************************************************************************************************************************/
GtkWidget* file_chooser_open(gpointer data,gchar* title,GabEditTypeFile type,GabEditTypeWin typewin) 
{
	return file_chooser(data, title, type, typewin, GTK_FILE_CHOOSER_ACTION_OPEN);
}
/***************************************************************************************************************************/
GtkWidget* file_chooser_save(gpointer data,gchar* title,GabEditTypeFile type,GabEditTypeWin typewin) 
{
	return file_chooser(data, title, type, typewin, GTK_FILE_CHOOSER_ACTION_SAVE);
}
/***************************************************************************************************************************/
void choose_file_to_open()
{
  GtkWidget *gabeditFileChooser;
  gchar* patternsfiles[] = {"*.com","*.mop","*.inp","*.nw","*.psi","*.xyz","*.zmt","*.gzmt","*",NULL};

  gabeditFileChooser = gabedit_file_chooser_new(_("Open file"), GTK_FILE_CHOOSER_ACTION_OPEN);
  gabedit_file_chooser_hide_hidden(GABEDIT_FILE_CHOOSER(gabeditFileChooser));

  if(strcmp(fileopen.datafile,"NoName"))
  {
	  if(fileopen.localdir)
	  {
		gchar* t = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
		gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(gabeditFileChooser),t);
		g_free(t);

	  }
	  else
		gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(gabeditFileChooser), fileopen.datafile);
  }

   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(gabeditFileChooser), patternsfiles);
    if(iprogram == PROG_IS_GAMESS  || iprogram == PROG_IS_FIREFLY || iprogram == PROG_IS_DEMON || iprogram == PROG_IS_ORCA  || iprogram == PROG_IS_OTHER)
   	gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.inp");
    else if(iprogram == PROG_IS_MOPAC) gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.mop");
    else if(iprogram == PROG_IS_PSICODE) gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.psi");
    else if(iprogram == PROG_IS_NWCHEM) gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.nw");
    else gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.com");
  


  g_signal_connect(G_OBJECT(gabeditFileChooser), "delete_event",(GCallback)destroy_button_windows,NULL);

  g_signal_connect (gabeditFileChooser, "response", G_CALLBACK (show_doc), GTK_OBJECT(gabeditFileChooser));
  g_signal_connect (gabeditFileChooser, "response", G_CALLBACK (destroy_button_windows), GTK_OBJECT(gabeditFileChooser));
  g_signal_connect (gabeditFileChooser, "response", G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(gabeditFileChooser));
  
  add_button_windows(_(" Open file "),gabeditFileChooser);
  gtk_widget_show(gabeditFileChooser);
}
/********************************************************************************/
void choose_file_to_insert()
{
  GtkWidget *gabeditFileChooser;

  gabeditFileChooser = gabedit_file_chooser_new(_("Include file"), GTK_FILE_CHOOSER_ACTION_OPEN);
  gabedit_file_chooser_hide_hidden(GABEDIT_FILE_CHOOSER(gabeditFileChooser));
  g_signal_connect(G_OBJECT(gabeditFileChooser), "delete_event",(GCallback)destroy_button_windows,NULL);
  if(fileopen.localdir)
  {
	gchar* t = g_strdup_printf("%s%s",fileopen.localdir,G_DIR_SEPARATOR_S);
	gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(gabeditFileChooser),t);
	g_free(t);
  }

  g_signal_connect (gabeditFileChooser, "response",  G_CALLBACK (insert_doc), GTK_OBJECT(gabeditFileChooser));
  g_signal_connect (gabeditFileChooser, "response", G_CALLBACK (data_modify),NULL);
  g_signal_connect (gabeditFileChooser, "response",  G_CALLBACK (destroy_button_windows), GTK_OBJECT(gabeditFileChooser));
  g_signal_connect (gabeditFileChooser, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(gabeditFileChooser));
  
  add_button_windows(_(" Include file "),gabeditFileChooser);
  gtk_widget_show(gabeditFileChooser);
}
/********************************************************************************/
void choose_file_to_save()
{
  GtkWidget *gabeditFileChooser;

  gabeditFileChooser = gabedit_file_chooser_new("Save file", GTK_FILE_CHOOSER_ACTION_SAVE);
  gabedit_file_chooser_hide_hidden(GABEDIT_FILE_CHOOSER(gabeditFileChooser));

  if(strcmp(fileopen.datafile,"NoName"))
  {
	  if(fileopen.localdir)
	  {
		gchar* t = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
		gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(gabeditFileChooser),t);
		g_free(t);

	  }
	  else
		gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(gabeditFileChooser),fileopen.datafile);
  }
  g_signal_connect(G_OBJECT(gabeditFileChooser), "delete_event",(GCallback)destroy_button_windows,NULL);

  g_signal_connect (gabeditFileChooser, "response", G_CALLBACK(enreg_selec_doc) , GTK_OBJECT(gabeditFileChooser));
  g_signal_connect (gabeditFileChooser, "response",  G_CALLBACK (destroy_button_windows), GTK_OBJECT(gabeditFileChooser));
  g_signal_connect (gabeditFileChooser, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(gabeditFileChooser));
  
  add_button_windows(" Save file ",gabeditFileChooser);
  gtk_widget_show(gabeditFileChooser);
}
/********************************************************************************/
void choose_file_to_save_end()
{
  GtkWidget *gabeditFileChooser;

  gabeditFileChooser = gabedit_file_chooser_new(_("Save file"), GTK_FILE_CHOOSER_ACTION_SAVE);
  gabedit_file_chooser_hide_hidden(GABEDIT_FILE_CHOOSER(gabeditFileChooser));
  gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(gabeditFileChooser),fileopen.datafile);

  g_signal_connect (gabeditFileChooser, "response", G_CALLBACK(enreg_selec_doc) , GTK_OBJECT(gabeditFileChooser));
  g_signal_connect (gabeditFileChooser, "response", G_CALLBACK(ExitDlg), GTK_OBJECT(gabeditFileChooser));
  g_signal_connect (gabeditFileChooser, "response", G_CALLBACK(gtk_widget_destroy) , GTK_OBJECT(gabeditFileChooser));
  
  gtk_widget_show(gabeditFileChooser);
}
/********************************************************************************/
void Save_YesNo()
{
    GtkWidget *DialogueMessage = NULL;
    GtkWidget *Label, *Bouton;
    GtkWidget *frame, *vboxframe;
    gchar *t;


    DialogueMessage = gtk_dialog_new();
   

    gtk_window_set_title(GTK_WINDOW(DialogueMessage),_("Question"));
    gtk_window_set_modal (GTK_WINDOW (DialogueMessage), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(DialogueMessage),GTK_WINDOW(Fenetre));
    gtk_window_set_position(GTK_WINDOW(DialogueMessage),GTK_WIN_POS_CENTER);


    g_signal_connect(G_OBJECT(DialogueMessage), "delete_event", (GCallback)gtk_widget_destroy, NULL);
   frame = gtk_frame_new (NULL);
   gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

   g_object_ref (frame);
   g_object_set_data_full (G_OBJECT (DialogueMessage), "frame", frame,(GDestroyNotify) g_object_unref);
   gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start(GTK_BOX(GTK_DIALOG(DialogueMessage)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

      gtk_widget_realize(DialogueMessage);

    t=get_name_file(fileopen.datafile);
    t=g_strdup_printf(_("\"%s\" has been modified. Do you want to save it ?"),t);
    Label = create_label_with_pixmap(DialogueMessage,t,_("Question"));
    g_free(t);
    gtk_box_pack_start( GTK_BOX(vboxframe), Label,TRUE,TRUE,0);
    
    /* the Cancel button */
    gtk_widget_realize(DialogueMessage);
    Bouton = create_button(DialogueMessage,_("Cancel"));
    gtk_box_pack_start( GTK_BOX(GTK_DIALOG(DialogueMessage)->action_area), Bouton,TRUE,TRUE,0);
    g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(DialogueMessage));
    GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
    gtk_widget_grab_default(Bouton);

    /* the No button */
    Bouton = create_button(DialogueMessage,_("No"));
    gtk_box_pack_start( GTK_BOX(GTK_DIALOG(DialogueMessage)->action_area), Bouton,TRUE,TRUE,0);
    g_signal_connect(G_OBJECT(Bouton), "clicked", (GCallback)ExitDlg,NULL);
    GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
    g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(DialogueMessage));

    /* the Yes button */
    Bouton = create_button(DialogueMessage,"Yes");
    gtk_box_pack_start( GTK_BOX(GTK_DIALOG(DialogueMessage)->action_area), Bouton,TRUE,TRUE,0);
    g_signal_connect(G_OBJECT(Bouton), "clicked", (GCallback)choose_file_to_save_end,NULL);
    g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(DialogueMessage));
    GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);



    gtk_widget_show_all(DialogueMessage);
}
/********************************************************************************/
void set_entry_file(GtkWidget* gabeditFileChooser, gint response_id)
{
  GtkWidget *entry;
  gchar *file;
  if( response_id != GTK_RESPONSE_OK) return;
  entry = (GtkWidget*)(g_object_get_data(G_OBJECT(gabeditFileChooser),"Entry"));	
  file = gabedit_file_chooser_get_current_file(GABEDIT_FILE_CHOOSER(gabeditFileChooser)); 
  gtk_entry_set_text(GTK_ENTRY(entry),file);
}
/********************************************************************************/
void set_entry_selected_file(GtkWidget* hbox)
{
  GtkWidget *gabeditFileChooser;
  GtkWidget *entry = (GtkWidget*)(g_object_get_data(G_OBJECT(hbox),"Entry"));	
  GtkWidget *Win = (GtkWidget*)(g_object_get_data(G_OBJECT(hbox),"Window"));	
  gchar** patterns = (gchar**)(g_object_get_data(G_OBJECT(hbox),"Patterns")); 
  
  gabeditFileChooser = gabedit_file_chooser_new(_("File selection"), GTK_FILE_CHOOSER_ACTION_OPEN);
  gtk_window_set_modal (GTK_WINDOW (gabeditFileChooser), TRUE);
  gabedit_file_chooser_hide_hidden(GABEDIT_FILE_CHOOSER(gabeditFileChooser));
  gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(gabeditFileChooser), patterns);
  add_child(Win,gabeditFileChooser,gtk_widget_destroy,_(" File selection "));
  g_signal_connect(G_OBJECT(gabeditFileChooser),"delete_event",(GCallback)delete_child,NULL);



  g_object_set_data(G_OBJECT (gabeditFileChooser), "Entry", entry);
  g_signal_connect (gabeditFileChooser, "response",  G_CALLBACK (set_entry_file), GTK_OBJECT(gabeditFileChooser));
  g_signal_connect (gabeditFileChooser, "response",  G_CALLBACK (delete_child), GTK_OBJECT(gabeditFileChooser));

  gtk_widget_show(gabeditFileChooser);
}
/********************************************************************************/
GtkWidget* choose_file_to_create(gchar* title, GCallback applyFunc)
{
  GtkWidget *gabeditFileChooser;

  gabeditFileChooser = gabedit_file_chooser_new(title, GTK_FILE_CHOOSER_ACTION_SAVE);
  gabedit_file_chooser_hide_hidden(GABEDIT_FILE_CHOOSER(gabeditFileChooser));

  if(strcmp(fileopen.datafile,"NoName"))
  {
	  if(fileopen.localdir)
	  {
		gchar* t = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.datafile);
		gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(gabeditFileChooser),t);
		g_free(t);

	  }
	  else
		gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(gabeditFileChooser),fileopen.datafile);
  }
  g_signal_connect(G_OBJECT(gabeditFileChooser), "delete_event",(GCallback)destroy_button_windows,NULL);

  g_signal_connect (gabeditFileChooser, "response", G_CALLBACK(applyFunc) , GTK_OBJECT(gabeditFileChooser));
  g_signal_connect (gabeditFileChooser, "response",  G_CALLBACK (destroy_button_windows), GTK_OBJECT(gabeditFileChooser));
  g_signal_connect (gabeditFileChooser, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(gabeditFileChooser));
  
  add_button_windows(title,gabeditFileChooser);
  gtk_widget_show(gabeditFileChooser);
  return gabeditFileChooser;
}
