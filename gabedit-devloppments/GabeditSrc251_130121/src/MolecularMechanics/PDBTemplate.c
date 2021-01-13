/* PDBTemplate.c */
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
#include <stdio.h>
#include <math.h>

#include "../Common/Global.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../MolecularMechanics/PDBTemplate.h"
#include "../MolecularMechanics/LoadPDBTemplate.h"
#include "../MolecularMechanics/CreateDefaultPDBTpl.h"
#include "../MolecularMechanics/SavePDBTemplate.h"
static	PDBTemplate* staticPDBTemplate = NULL;
/************************************************************/
PDBTemplate* freePDBTpl(PDBTemplate* pdbTemplate)
{
	gint i;
	gint j;
	if(!pdbTemplate)
		return NULL;
	for(i=0;i<pdbTemplate->numberOfResidues;i++)
	{
		if(pdbTemplate->residueTemplates[i].residueName)
			g_free(pdbTemplate->residueTemplates[i].residueName);

		for(j=0;j<pdbTemplate->residueTemplates[i].numberOfTypes;j++)
		{
			if(pdbTemplate->residueTemplates[i].typeTemplates[j].pdbType)
				g_free(pdbTemplate->residueTemplates[i].typeTemplates[j].pdbType);
			if(pdbTemplate->residueTemplates[i].typeTemplates[j].mmType)
				g_free(pdbTemplate->residueTemplates[i].typeTemplates[j].mmType);
		}
		if(pdbTemplate->residueTemplates[i].typeTemplates)
			g_free(pdbTemplate->residueTemplates[i].typeTemplates);
	}
	if(pdbTemplate->residueTemplates)
		g_free(pdbTemplate->residueTemplates);
	g_free(pdbTemplate);
	return NULL;
}
/************************************************************/
PDBTemplate* LoadPersonalPDBTpl()
{
	gchar* filename = g_strdup_printf("%s%sPersonalPDBTemplate.tpl",
			gabedit_directory(), G_DIR_SEPARATOR_S);
	PDBTemplate* pdbTemplate = NULL;

	pdbTemplate = g_malloc(sizeof(PDBTemplate));
	if(!readPDBTemplate(pdbTemplate,filename))
	{
		g_free(pdbTemplate);
		pdbTemplate = NULL;
	}

	g_free(filename);
	return pdbTemplate;
}
/************************************************************/
PDBTemplate* LoadDefaultPDBTpl()
{
	PDBTemplate* pdbTemplate = NULL;
	gchar* filename = g_strdup_printf("%s%sDefaultPDBTemplate.tpl",
			gabedit_directory(), G_DIR_SEPARATOR_S);

	pdbTemplate = g_malloc(sizeof(PDBTemplate));
	if(!readPDBTemplate(pdbTemplate,filename))
	{
		g_free(pdbTemplate);
		pdbTemplate = NULL;
	}

	g_free(filename);
	return pdbTemplate;
}
/************************************************************/
void LoadPDBTpl()
{
	if(staticPDBTemplate)
		staticPDBTemplate = freePDBTpl(staticPDBTemplate);
	staticPDBTemplate = LoadPersonalPDBTpl();
	if(!staticPDBTemplate)
	{
		staticPDBTemplate = LoadDefaultPDBTpl();
		if(!staticPDBTemplate)
		{

			if(CreateDefaultPDBTpl())
				staticPDBTemplate = LoadDefaultPDBTpl();
		}
	}
}
/************************************************************/
static gint getResiduePDBTplNumber(gchar* residueName)
{
	gint i;
	if(!staticPDBTemplate)
		return -1;

	for(i=0;i<staticPDBTemplate->numberOfResidues;i++)
	{
		if(!strcmp(staticPDBTemplate->residueTemplates[i].residueName,residueName))
			return i;
	}
	return -1;
}
/************************************************************/
static gchar* getmmType(gint residueNumber, gchar* pdbType,gdouble* charge)
{
	gint j;
	PDBTypeTemplate* typeTemplates = 
		staticPDBTemplate->residueTemplates[residueNumber].typeTemplates;
	gint numberOfTypes = staticPDBTemplate->residueTemplates[residueNumber].numberOfTypes;
	gchar* mmType = g_strdup("UNK");
	for(j=0;j<numberOfTypes;j++)
	{
		if(!strcmp(pdbType,typeTemplates[j].pdbType))
		{
			g_free(mmType);
			mmType = g_strdup(typeTemplates[j].mmType);
			*charge = typeTemplates[j].charge;
			return mmType;
		}
	}
	return mmType;
}
/************************************************************/
gchar* getMMTypeFromPDBTpl(gchar* residueName,gchar* pdbType,gdouble* charge)
{
	gchar* mmType = g_strdup("UNK");
	gint residueNumber = -1;
	*charge = 0;

	if(!staticPDBTemplate)
		return mmType;
	residueNumber = getResiduePDBTplNumber(residueName);
	if(residueNumber==-1)
	{
		residueNumber = getResiduePDBTplNumber("ALLRESIDUE");
		if(residueNumber==-1)
			return mmType;
		else
			return getmmType(residueNumber,pdbType,charge);
	}
	else
	{
		mmType = getmmType(residueNumber,pdbType,charge);
		if(!strcmp(mmType,"UNK"))
		{
			residueNumber = getResiduePDBTplNumber("ALLRESIDUE");
			if(residueNumber==-1)
				return mmType;
			else
				return getmmType(residueNumber,pdbType,charge);
		}
		else
			return mmType;
	}
}
/************************************************************/
static gint getHydrogens(gint residueNumber, gchar* pdbType, gchar** hAtoms)
{
	gint j;
	gint k;
	PDBTypeTemplate* typeTemplates = 
		staticPDBTemplate->residueTemplates[residueNumber].typeTemplates;
	gint numberOfTypes = staticPDBTemplate->residueTemplates[residueNumber].numberOfTypes;
	gint nH = 0;
	for(j=0;j<numberOfTypes;j++)
	{
		if(!strcmp(pdbType,typeTemplates[j].pdbType))
		{
			for(k=j+1;k<numberOfTypes;k++)
			{
				if(!typeTemplates[k].pdbType) break;
				if(typeTemplates[k].pdbType[0]!='H') break;
				sprintf(hAtoms[nH],"%s",typeTemplates[k].pdbType);
				nH++;
				if(nH>10) break;
			}
			return nH;
		}
	}
	return nH;
}
/************************************************************/
gint getHydrogensFromPDBTpl(gchar* residueName,gchar* pdbType, gchar** hAtoms)
{
	gint nH = 0;
	gint residueNumber = -1;

	if(!pdbType) return nH;
	if(pdbType && (pdbType[0]=='H' || pdbType[0]=='h')) return nH;
	if(!staticPDBTemplate) return nH;
	residueNumber = getResiduePDBTplNumber(residueName);
	if(residueNumber==-1)
	{
		residueNumber = getResiduePDBTplNumber("ALLRESIDUE");
		if(residueNumber==-1)
			return nH;
		else
			return getHydrogens(residueNumber,pdbType, hAtoms);
	}
	else
	{
		nH = getHydrogens(residueNumber,pdbType, hAtoms);
		if(nH == 0)
		{
			residueNumber = getResiduePDBTplNumber("ALLRESIDUE");
			if(residueNumber==-1)
				return nH;
			else
				return getHydrogens(residueNumber,pdbType, hAtoms);
		}
		else
			return nH;
	}
	return nH;
}
/************************************************************/
PDBTemplate* getPointerPDBTemplate()
{
	return staticPDBTemplate;
}
/************************************************************/
void setPointerPDBTemplate(PDBTemplate* ptr)
{
	staticPDBTemplate = ptr;
}
/************************************************************/
void savePersonalPDBTpl(GtkWidget* win)
{
	gchar* filename = g_strdup_printf("%s%sPersonalPDBTemplate.tpl",
			gabedit_directory(), G_DIR_SEPARATOR_S);


	if(!savePDBTemplate(staticPDBTemplate,filename) && win)
	{
		gchar* t = g_strdup_printf("Sorry, I can not create \"%s\" file",filename);
		GtkWidget* w = Message(t,"Error",TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(win));
		g_free(t);
		return;
	}
	g_free(filename);
}
/************************************************************/
gchar** getListPDBTypes(gchar* residueName, gint* nlist)
{
	gchar** t = NULL;
	gint j;
	gint residueNumber = getResiduePDBTplNumber(residueName);
	PDBTypeTemplate* typeTemplates = NULL;
	gint numberOfTypes = 0;

	*nlist = 0;

	if(residueNumber==-1) residueNumber = getResiduePDBTplNumber("ALLRESIDUE");
	if(residueNumber==-1) return NULL;

	typeTemplates = staticPDBTemplate->residueTemplates[residueNumber].typeTemplates;
	numberOfTypes = staticPDBTemplate->residueTemplates[residueNumber].numberOfTypes;
	t = g_malloc(numberOfTypes*sizeof(gchar*));
	for(j=0;j<numberOfTypes;j++)
		t[j] = g_strdup(typeTemplates[j].pdbType);
	*nlist = numberOfTypes;
	return t;
}
/********************************************************************************/

