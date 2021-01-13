/* SavePDBTemplate.c */
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
/************************************************************/
static void saveTitleResidueTpl(FILE* file)
{
	fprintf(file,"Begin Title\n");
	fprintf(file,"	Residue        : PDB type atom  Amber type atom  charge of atom\n");
	fprintf(file,"End\n");
}
/**********************************************************************/
static void saveResiduesList(PDBTemplate* pdbTemplate, FILE* file)
{
	gint i;

	fprintf(file,"Begin Residue List\n");

	for(i=0;i<pdbTemplate->numberOfResidues;i++)
		fprintf(file,"%s\n",pdbTemplate->residueTemplates[i].residueName);

	fprintf(file,"End\n");
}
/**********************************************************************/
static void saveOneResidue(PDBTemplate* pdbTemplate, FILE* file,gint residueNumber)
{
	gint i;
	gint numberOfTypes = pdbTemplate->residueTemplates[residueNumber].numberOfTypes;
	PDBTypeTemplate *typeTemplates = pdbTemplate->residueTemplates[residueNumber].typeTemplates;

	fprintf(file,"Begin %s Residue\n",pdbTemplate->residueTemplates[residueNumber].residueName);
	for(i=0;i<numberOfTypes;i++)
		fprintf(file,"%s %s %f\n",
				typeTemplates[i].pdbType,
				typeTemplates[i].mmType,
				typeTemplates[i].charge
				);
	fprintf(file,"End\n");
}
/**********************************************************************/
static void saveAllResidues(PDBTemplate* pdbTemplate, FILE* file)
{
	gint i;
	gint n = pdbTemplate->numberOfResidues;
	for(i=0;i<n;i++)
		saveOneResidue(pdbTemplate,file,i);
}
/**********************************************************************/
gboolean savePDBTemplate(PDBTemplate* pdbTemplate,gchar* filename)
{
	FILE* file;
	file = FOpen(filename,"w");

	if(file == NULL)
		return FALSE;
	else
	{
		saveTitleResidueTpl(file);
		saveResiduesList(pdbTemplate,file);
		saveAllResidues(pdbTemplate,file);
		fclose(file);
	}
	return TRUE;
}
/**********************************************************************/
