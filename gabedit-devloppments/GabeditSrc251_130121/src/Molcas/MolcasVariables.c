/* MolcasVariables.c */
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

#include <stdlib.h>

#include "../../Config.h"
#include "../Common/Global.h"
#include "../Molcas/MolcasTypes.h"
#include "../Molcas/MolcasGlobal.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/GeomConversion.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Symmetry/MoleculeSymmetry.h"
#include "../Symmetry/MoleculeSymmetryInterface.h"

/************************************************************************************************************/
typedef enum
{
  MOLCAS_TYPE_VARIABLE_MEM = 0,
  MOLCAS_TYPE_VARIABLE_DISK,
  MOLCAS_TYPE_VARIABLE_RAMD,
  MOLCAS_TYPE_VARIABLE_TRAP,
  MOLCAS_TYPE_VARIABLE_WORKDIR
} MolcasTypeVariable;

static MolcasTypeVariable typeOfVariables[] = { 
	MOLCAS_TYPE_VARIABLE_MEM, 
	MOLCAS_TYPE_VARIABLE_DISK, 
	MOLCAS_TYPE_VARIABLE_RAMD, 
	MOLCAS_TYPE_VARIABLE_TRAP,
	MOLCAS_TYPE_VARIABLE_WORKDIR
};
static MolcasSystemVariables molcasSystemVariablesTmp;
/************************************************************************************************************/
gint molcasMem()
{
	if(strstr(molcasSystemVariables.mem,_("Default"))) 
		return -1;
	else
	if(strstr(molcasSystemVariables.mem,_("largest possible"))) 
		return 0;
	else
	if(atoi(molcasSystemVariables.mem)>0) 
		return atoi(molcasSystemVariables.mem);
	else
		return -1;  
}
/************************************************************************************************************/
gint molcasDisk()
{
	if(strstr(molcasSystemVariables.disk,_("Default"))) 
		return -1;
	else
	if(strstr(molcasSystemVariables.disk,_("Limited to 2 GBytes"))) 
		return 0;
	else
	if(atoi(molcasSystemVariables.disk)>0) 
		return atoi(molcasSystemVariables.disk);
	else
		return -1;  
}
/************************************************************************************************************/
gint molcasRamd()
{
	if(strstr(molcasSystemVariables.ramd,_("Default"))) 
		return -1;
	else
	if(atoi(molcasSystemVariables.ramd)>0) 
		return atoi(molcasSystemVariables.ramd);
	else
		return -1;  
}
/************************************************************************************************************/
gboolean molcasTrap()
{
	if(strstr(molcasSystemVariables.trap,"ON")) 
		return TRUE;
	else
		return FALSE;
}
/************************************************************************************************************/
void molcasWorkDir(gchar* workDir)
{
	sprintf(workDir,"%s", molcasSystemVariables.workDir);
}
/************************************************************************************************************/
static void initMolcasSystemVariables(MolcasSystemVariables* mSystemVariables)
{
	sprintf(mSystemVariables->mem,_("Default"));
	sprintf(mSystemVariables->disk,_("Default"));
	sprintf(mSystemVariables->ramd,_("Default"));
	sprintf(mSystemVariables->trap,"ON");
	sprintf(mSystemVariables->workDir,_("Default"));
}
/************************************************************************************************************/
static void initMolcasSystemVariablesTmp()
{
	initMolcasSystemVariables(&molcasSystemVariablesTmp);
}
/************************************************************************************************************/
static void copySystemVariablesParameters(MolcasSystemVariables* newCopy, MolcasSystemVariables* toCopy)
{

	sprintf( newCopy->mem, "%s", toCopy->mem);
	sprintf( newCopy->disk, "%s", toCopy->disk);
	sprintf( newCopy->ramd, "%s",toCopy->ramd);
	sprintf( newCopy->trap, "%s",toCopy->trap);
	sprintf( newCopy->workDir, "%s",toCopy->workDir);
}
/************************************************************************************************************/
static void copySystemVariablesParametersFromTmp(GtkWidget *win, gpointer data)
{
	copySystemVariablesParameters(&molcasSystemVariables, &molcasSystemVariablesTmp);
}
/**********************************************************************/
static void changedEntriesVariable(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entrytext = NULL;
	MolcasTypeVariable* type;
	 
	if(!GTK_IS_WIDGET(entry)) return;


	type = g_object_get_data(G_OBJECT (entry), "Type");
	if(type == NULL) return ;
	entrytext = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entrytext)<1)return;

	switch(*type)
	{
		case MOLCAS_TYPE_VARIABLE_MEM : 
			if(strstr(entrytext,_("Default")) || strstr(entrytext,_("largest possible")) )
			{
				gtk_widget_set_sensitive(entry, FALSE);
			}
			else
			{
				gtk_widget_set_sensitive(entry, TRUE);
			}

			if(strstr(entrytext,_("Default")) || strstr(entrytext,_("largest possible")) || atof(entrytext) != 0)
				sprintf(molcasSystemVariablesTmp.mem,"%s",entrytext);

			break;
		case MOLCAS_TYPE_VARIABLE_DISK :
			if(strstr(entrytext,_("Default")) || strstr(entrytext,_("Limited to 2 GBytes")) )
			{
				gtk_widget_set_sensitive(entry, FALSE);
			}
			else
			{
				gtk_widget_set_sensitive(entry, TRUE);
			}

			if(strstr(entrytext,_("Default")) || strstr(entrytext,_("Limited to 2 GBytes")) || atof(entrytext) != 0)
				sprintf(molcasSystemVariablesTmp.disk, "%s", entrytext);
			break;
		case MOLCAS_TYPE_VARIABLE_RAMD :
			if(strstr(entrytext,_("Default")))
			{
				gtk_widget_set_sensitive(entry, FALSE);
			}
			else
			{
				gtk_widget_set_sensitive(entry, TRUE);
			}
			if(strstr(entrytext,_("Default")) || atof(entrytext) != 0)
			sprintf(molcasSystemVariablesTmp.ramd, "%s",entrytext);
			break;
		case MOLCAS_TYPE_VARIABLE_TRAP :
			gtk_widget_set_sensitive(entry, FALSE);
			sprintf(molcasSystemVariablesTmp.trap, "%s",entrytext);
			break;
		case MOLCAS_TYPE_VARIABLE_WORKDIR :
			if(strstr(entrytext,_("Default")))
			{
				gtk_widget_set_sensitive(entry, FALSE);
			}
			else
			{
				gtk_widget_set_sensitive(entry, TRUE);
			}
			sprintf(molcasSystemVariablesTmp.workDir, "%s",entrytext);
			break;
	}
}
/*********************************************************************************************/
static GtkWidget* addComboListToATable(GtkWidget* table,
		gchar** list, gint nlist, gint i, gint j, gint k)
{
	GtkWidget *entry = NULL;
	GtkWidget *combo = NULL;

	combo = create_combo_box_entry(list, nlist, TRUE, -1, -1);

	gtk_table_attach(GTK_TABLE(table),combo,j,j+k,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);
	entry = GTK_BIN (combo)->child;

	return entry;
}
/************************************************************/
void createSystemVariablesFrame(GtkWidget *win, GtkWidget *box, GtkWidget *OkButton)
{
	gushort i;
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget *table = gtk_table_new(5,2,FALSE);

	GtkWidget* entryMem = NULL;
	GtkWidget* labelMem = gtk_label_new("MOLCASMEM : ");

	GtkWidget* entryDisk = NULL;
	GtkWidget* labelDisk = gtk_label_new("MOLCASDISK : ");

	GtkWidget* entryRamd = NULL;
	GtkWidget* labelRamd = gtk_label_new("MOLCASRAMD : ");

	GtkWidget* entryTrap = NULL;
	GtkWidget* labelTrap = gtk_label_new("MOLCAS_TRAP : ");

	GtkWidget* entryWorkDir = NULL;
	GtkWidget* labelWorkDir = gtk_label_new("WorkDir : ");

	gint nlistMem = 3;
	gchar* listMem[]  = { _("Default"), _("The largest possible"), "64 MB" };

	gint nlistDisk = 3;
	gchar* listDisk[] = { _("Default"), _("Limited to 2 GBytes"), "1000 MB" };

	gint nlistRamd = 2;
	gchar* listRamd[] = { _("Default"), "100 MB" };

	gint nlistTrap = 2;
	gchar* listTrap[] = { "ON", "OFF" };

	gint nlistWorkDir = 2;
	gchar* listWorkDir[] = { _("Default"), "$HOME/tmp" };

	initMolcasSystemVariablesTmp();
	initMolcasSystemVariables(&molcasSystemVariables);

	frame = gtk_frame_new (_("Molcas system variables"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	i = 0;
	add_widget_table(table, labelMem, i, 0);
	entryMem = addComboListToATable(table, listMem, nlistMem, i, 1, 1);
	gtk_widget_set_sensitive(entryMem, FALSE);
	g_object_set_data(G_OBJECT (entryMem), "Type", &typeOfVariables[MOLCAS_TYPE_VARIABLE_MEM]);
	g_signal_connect(G_OBJECT(entryMem),"changed", G_CALLBACK(changedEntriesVariable),NULL);

	i = 1;
	add_widget_table(table, labelDisk, i, 0);
	entryDisk = addComboListToATable(table, listDisk, nlistDisk, i, 1, 1);
	gtk_widget_set_sensitive(entryDisk, FALSE);
	g_object_set_data(G_OBJECT (entryDisk), "Type",&typeOfVariables[MOLCAS_TYPE_VARIABLE_DISK]);
	g_signal_connect(G_OBJECT(entryDisk),"changed", G_CALLBACK(changedEntriesVariable),NULL);

	i = 2;
	add_widget_table(table, labelRamd, i, 0);
	entryRamd = addComboListToATable(table, listRamd, nlistRamd, i, 1, 1);
	gtk_widget_set_sensitive(entryRamd, FALSE);
	g_object_set_data(G_OBJECT (entryRamd), "Type",&typeOfVariables[MOLCAS_TYPE_VARIABLE_RAMD]);
	g_signal_connect(G_OBJECT(entryRamd),"changed", G_CALLBACK(changedEntriesVariable),NULL);

	i = 3;
	add_widget_table(table, labelTrap, i, 0);
	entryTrap = addComboListToATable(table, listTrap, nlistTrap, i, 1, 1);
	gtk_widget_set_sensitive(entryTrap, FALSE);
	g_object_set_data(G_OBJECT (entryTrap), "Type",&typeOfVariables[MOLCAS_TYPE_VARIABLE_TRAP]);
	g_signal_connect(G_OBJECT(entryTrap),"changed", G_CALLBACK(changedEntriesVariable),NULL);

	i = 4;
	add_widget_table(table, labelWorkDir, i, 0);
	entryWorkDir = addComboListToATable(table, listWorkDir, nlistWorkDir, i, 1, 1);
	gtk_widget_set_sensitive(entryWorkDir, FALSE);
	g_object_set_data(G_OBJECT (entryWorkDir), "Type",&typeOfVariables[MOLCAS_TYPE_VARIABLE_WORKDIR]);
	g_signal_connect(G_OBJECT(entryWorkDir),"changed", G_CALLBACK(changedEntriesVariable),NULL);

	if(OkButton)
		g_signal_connect(G_OBJECT(OkButton), "clicked",G_CALLBACK(copySystemVariablesParametersFromTmp),NULL);
}
/************************************************************************************************************/
void putVariablesInTextEditor()
{
        gchar buffer[BSIZE];
	gboolean trap = molcasTrap();
	gint mem = molcasMem();
	gint disk = molcasDisk();
	gint ramd = molcasRamd();
        gchar workDir[BSIZE];

	molcasWorkDir(workDir);

	if( trap && mem <0 && disk <0 && ramd <0 && strcmp(workDir,_("Default"))==0) return;

	sprintf(buffer,"*----------------------------------------------------------------\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	sprintf(buffer,"*              Molcas Variables\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);

	if(!molcasTrap())
	{
		sprintf(buffer,"*              MOLCAS_TRAP='OFF'\n");
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.program, NULL, buffer,-1);
	}
	if(mem>=0)
	{
		sprintf(buffer,"*              MOLCASMEM=%d\n",mem);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.program, NULL, buffer,-1);
	}
	if(disk>=0)
	{
		sprintf(buffer,"*              MOLCASDISK=%d\n",disk);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.program, NULL, buffer,-1);
	}
	if(ramd>=0)
	{
		sprintf(buffer,"*              MOLCASRAMD=%d\n",ramd);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.program, NULL, buffer,-1);
	}
	if(strcmp(workDir,_("Default")) !=0)
	{
		sprintf(buffer,"*              MOLCASWORKDIR=%s\n",workDir);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.program, NULL, buffer,-1);
	}
	sprintf(buffer,"*----------------------------------------------------------------\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
}
/************************************************************************************************************/
static void setOneValue(gchar* t, gchar* value)
{
	gint i;
	gint k;
	gchar* p;

	p = strstr(t,"=");
	if(p)
	{
		i = sscanf(p+1,"%d",&k);
		if(i<1) 
			sprintf(value,_("Default"));
		else
			sprintf(value,"%d",k);
	}

}
/************************************************************************************************************/
static void setTrap(gchar* t, gchar* value)
{
	gint i;
	gchar* p;

	p = strstr(t,"=");
	if(p)
	{
		i = sscanf(p+1,"%s",value);
		if(i<1) sprintf(value,"ON");
		if(strstr(value,"ON")) return;
		if(strstr(value,"OFF")) return;
		sprintf(value,"ON");
	}
}
/************************************************************************************************************/
static void setWorkDir(gchar* t, gchar* value)
{
	gint i;
	gchar* p;

	p = strstr(t,"=");
	if(p)
	{
		i = sscanf(p+1,"%s",value);
		if(i<1) sprintf(value,_("Default"));
	}
}
/************************************************************************************************************/
void setMolcasVariablesFromInputFile(gchar* fileName)
{
	gchar t[BSIZE];
	guint taille=BSIZE;
	FILE* file = NULL;

	sprintf(molcasSystemVariables.mem,_("Default"));
	sprintf(molcasSystemVariables.disk,_("Default"));
	sprintf(molcasSystemVariables.ramd,_("Default"));
	sprintf(molcasSystemVariables.trap,"ON");
	sprintf(molcasSystemVariables.workDir,_("Default"));

	file = FOpen(fileName, "r");
	if(!file) return;

	while(!feof(file))
	{
		if(!fgets(t, taille, file)) break;
		if( strstr(t, "MOLCASMEM")) setOneValue(t,molcasSystemVariables.mem);
		if( strstr(t, "MOLCASDISK")) setOneValue(t,molcasSystemVariables.disk);
		if( strstr(t, "MOLCASRAMD")) setOneValue(t,molcasSystemVariables.ramd);
		if( strstr(t, "MOLCAS_TRAP")) setTrap(t,molcasSystemVariables.trap);
		if( strstr(t, "MOLCASWORKDIR")) setWorkDir(t,molcasSystemVariables.workDir);
	}
	fclose(file);
	return;
}
