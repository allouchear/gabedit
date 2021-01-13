/* ExportGeom.c */
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
#include "../Geometry/InterfaceGeom.h"
#include "../Utils/Utils.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Vector3d.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/Measure.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/ExportGeom.h"
#include "../Common/Windows.h"
#include "../Files/FileChooser.h"

#ifdef DRAWGEOMGL
/********************************************************************************/
static void export_file(GabeditFileChooser *SelecteurFichier , gint response_id)
{
 

  
	gchar *fileName;
	FILE* file;
	gint buffsize = 0;
	gint state = GL2PS_OVERFLOW;
  	gchar* type = NULL;
	gint gl2ps_type = GL2PS_PS;

 	if(response_id != GTK_RESPONSE_OK) return;

	if(!GeomDrawingArea) return;
	if(!SelecteurFichier) return;
	type = (gchar*)g_object_get_data(G_OBJECT (SelecteurFichier), "Type");
	if(!type) type = g_strdup("eps");
 	fileName = gabedit_file_chooser_get_current_file(SelecteurFichier);
	if(strstr(type,"eps")) gl2ps_type = GL2PS_EPS;
	if(strstr(type,"tex")) gl2ps_type = GL2PS_TEX;
	if(strstr(type,"pdf")) gl2ps_type = GL2PS_PDF;
	if(strstr(type,"svg")) gl2ps_type = GL2PS_SVG;
	if(strstr(type,"pgf")) gl2ps_type = GL2PS_PGF;

    	file = FOpen(fileName, "wb");
	printf("Writing '%s file'. Please wait... \n",type);
	while(state == GL2PS_OVERFLOW)
	{
		buffsize += 1024*1024;
      		gl2psBeginPage("Gabedit", "Gabedit", NULL, gl2ps_type, GL2PS_BSP_SORT, 
                     GL2PS_OCCLUSION_CULL | GL2PS_USE_CURRENT_VIEWPORT | GL2PS_NO_BLENDING, 
                     GL_RGBA, 0, NULL, 0, 0, 0, buffsize, file,fileName);
		redrawGeomGL2PS();
		state = gl2psEndPage();
	}
    	fclose(file);
	printf("End Writing '%s file'... \n",type);
}
/********************************************************************************/
void export_geometry_dlg(gchar* type)
{
	gchar* title = g_strdup_printf("Create a %s file",type);
  	gchar* patternsfiles[] = {"*","*.eps","*.ps","*.pdf","*.svg",NULL};
	gchar* fileName = g_strdup_printf("gabedit.%s",type);
	gchar* filter = g_strdup_printf("*.%s",type);

	GtkWidget* win = choose_file_to_create(title, G_CALLBACK(export_file));
	g_object_set_data(G_OBJECT (win), "Type",type);
	gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(win), patternsfiles);
	gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(win),filter);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(win),fileName);
	g_free(title);
	g_free(fileName);
	g_free(filter);
}
#else
/*****************************************************************************/
static void export_geometry_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;
	gchar* type = (gchar*)g_object_get_data(G_OBJECT (SelecFile), "Type");
	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	export_geometry(fileName, type);
} 

/********************************************************************************/
void export_geometry_dlg(gchar* type)
{
	gchar* title = g_strdup_printf("Create a %s file",type);
  	gchar* patternsfiles[] = {"*","*.eps","*.ps","*.pdf","*.svg","*.pov",NULL};
	gchar* fileName = g_strdup_printf("gabedit.%s",type);
	gchar* filter = g_strdup_printf("*.%s",type);

	GtkWidget* win = choose_file_to_create(title, G_CALLBACK(export_geometry_file));
	g_object_set_data(G_OBJECT (win), "Type",type);
	gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(win), patternsfiles);
	gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(win),filter);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(win),fileName);
	g_free(title);
	g_free(fileName);
	g_free(filter);
}
#endif
