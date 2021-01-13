/* LoadPDBTemplate.c */
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
#include "../MolecularMechanics/PDBTemplate.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
/**********************************************************************/
gchar** getResiduesList(FILE* file,gint* nResidue)
{
	gchar** t = (gchar**)g_malloc(sizeof(gchar*));
	
	gchar dump[BSIZE];
	gint len = BSIZE;
	gint n;
	gboolean Ok = FALSE;

	*nResidue = 0;
	fseek(file, 0L, SEEK_SET);
    	{ char* e = fgets(dump,len,file);}
	while(!feof(file))
	{
		if(fgets(dump,len,file))
		{
			if(strstr(dump,"Begin Residue List"))
			{
				Ok = TRUE;
				break;
			}
		}
	}
	if(!Ok)
		return NULL;
	n = 0;
	while(!feof(file))
	{
		if(fgets(dump,len,file))
		{
			if(strstr(dump,"End"))
				break;
		}
		t = g_realloc(t,(n+1)*sizeof(gchar*));
		t[n] = (gchar*)g_malloc(BSIZE*sizeof(gchar));
		sscanf(dump,"%s",t[n]);
		n++;
	}
	if(n==0)
	{
		free(t);
		return NULL;
	}
	*nResidue = n;

	return t;
}
/**********************************************************************/
void setResiduesList(PDBTemplate* pdbTemplate, FILE* file)
{
	gint nResidue;
	gchar** t = getResiduesList(file,&nResidue);

	pdbTemplate->numberOfResidues = nResidue;
	pdbTemplate->residueTemplates = NULL;
	if(nResidue>0 && t)
	{
		gint i;
		pdbTemplate->residueTemplates = g_malloc(nResidue*sizeof(PDBResidueTemplate));
		for(i=0;i<nResidue;i++)
		{
			pdbTemplate->residueTemplates[i].residueName = g_strdup(t[i]);
			pdbTemplate->residueTemplates[i].numberOfTypes = 0;
			pdbTemplate->residueTemplates[i].typeTemplates = NULL;
			g_free(t[i]);
		}
		g_free(t);
	}
}
/**********************************************************************/
void setOneResidue(PDBTemplate* pdbTemplate, FILE* file,gint residueNumber)
{
	gchar title[BSIZE];
	gchar pdbType[BSIZE];
	gchar mmType[BSIZE];
	gchar charge[BSIZE];
	gchar dump[BSIZE];
	gint len = BSIZE;
	gint n = 0;
	PDBTypeTemplate *typeTemplates = NULL;
	gboolean Ok = FALSE;

	sprintf(title,"Begin %s Residue",pdbTemplate->residueTemplates[residueNumber].residueName);
	fseek(file, 0L, SEEK_SET);
	while(!feof(file))
	{
		if(fgets(dump,len,file))
		{
			if(strstr(dump,title))
			{
				Ok = TRUE;
				break;
			}
		}
	}
	if(!Ok)
		return;
	n = 0;
	typeTemplates = g_malloc(sizeof(PDBTypeTemplate));
	while(!feof(file))
	{
		if(fgets(dump,len,file))
		{
			if(strstr(dump,"End"))
				break;
		}
		sscanf(dump,"%s %s %s",pdbType, mmType, charge);
		/*printf("pdbType = %s mmType = %s charge = %s\n",pdbType, mmType, charge);*/
		typeTemplates[n].pdbType = g_strdup(pdbType);
		typeTemplates[n].mmType = g_strdup(mmType);
		typeTemplates[n].charge = atof(charge);

		n++;
		typeTemplates = g_realloc(typeTemplates,(n+1)*sizeof(PDBTypeTemplate));
	}
	if(n==0)
	{
		g_free(typeTemplates);
		pdbTemplate->residueTemplates[residueNumber].numberOfTypes = 0;
		pdbTemplate->residueTemplates[residueNumber].typeTemplates = NULL;
		return;
	}
	typeTemplates = g_realloc(typeTemplates,n*sizeof(PDBTypeTemplate));

	pdbTemplate->residueTemplates[residueNumber].numberOfTypes = n;
	pdbTemplate->residueTemplates[residueNumber].typeTemplates = typeTemplates;
}
/**********************************************************************/
void setAllResidues(PDBTemplate* pdbTemplate, FILE* file)
{
	gint i;
	gint n = pdbTemplate->numberOfResidues;
	/* printf("numberOfResidues = %d\n",pdbTemplate->numberOfResidues);*/
	for(i=0;i<n;i++)
	{
		/* printf("i = %d\n",i);*/
		setOneResidue(pdbTemplate,file,i);
	}
}
/**********************************************************************/
gboolean readPDBTemplate(PDBTemplate* pdbTemplate,gchar* filename)
{
	FILE* file;
	file = FOpen(filename,"rb");

	/* printf("Read Default TPL file %s \n",filename);*/
	if(file == NULL)
		return FALSE;
	else
	{
		/* printf("Read List Of residue\n");*/
		setResiduesList(pdbTemplate,file);
		/* printf("End Read List Of residue\n");*/
		setAllResidues(pdbTemplate,file);
		/* printf("End Set All Residue\n");*/
		fclose(file);
	}
	return TRUE;
}
/**********************************************************************/
