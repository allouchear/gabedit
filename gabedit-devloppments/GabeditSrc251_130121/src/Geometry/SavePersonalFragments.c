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
#include "../Geometry/Fragments.h"
#include "../Geometry/PersonalFragments.h"
#include "../Utils/Utils.h"
/************************************************************/
static void saveTitlePesoFrag(FILE* file)
{
	fprintf(file,"Begin Title\n");
	fprintf(file,"	Please not edit this file\n");
	fprintf(file,"End\n");
}
/**********************************************************************/
static void saveGroupesList(PersonalFragments* personalFragments, FILE* file)
{
	gint i;

	fprintf(file,"Begin Groupes List\n");

	for(i=0;i<personalFragments->numberOfGroupes;i++)
		fprintf(file,"%s\n",personalFragments->personalGroupes[i].groupName);

	fprintf(file,"End\n");
}
/**********************************************************************/
static void saveOneFragmentsList(PersonalFragments* personalFragments, FILE* file,gint groupeNumber)
{
	PersonalGroupe* personalGroupes = personalFragments->personalGroupes;
	gint numberOfFragments =  personalFragments->personalGroupes[groupeNumber].numberOfFragments;
	OnePersonalFragment* fragments = personalGroupes[groupeNumber].fragments;
	gint i;

	fprintf(file,"Begin %s Groupe\n",personalGroupes[groupeNumber].groupName);

	for(i=0;i<numberOfFragments;i++)
		fprintf(file,"%s\n",fragments[i].name);

	fprintf(file,"End\n");
			
}
/**********************************************************************/
static void saveAllFragmentsList(PersonalFragments* personalFragments, FILE* file)
{
	gint numberOfGroupes =  personalFragments->numberOfGroupes;
	gint i;

	for(i=0;i<numberOfGroupes;i++)
		saveOneFragmentsList(personalFragments,file,i);

}
/**********************************************************************/
static void saveOneFragment(PersonalFragments* personalFragments, FILE* file,
		gint groupeNumber, gint fragmentNumber)
{
	PersonalGroupe* personalGroupes = personalFragments->personalGroupes;
	OnePersonalFragment* fragments = personalGroupes[groupeNumber].fragments;
	Fragment f;
	gint i;

	fprintf(file,"Begin %s %s Fragment\n",
			personalGroupes[groupeNumber].groupName,
			fragments[fragmentNumber].name
			);

	f = fragments[fragmentNumber].f;
	fprintf(file,"%d\n",f.NAtoms);
	for(i=0;i<f.NAtoms;i++)
	{
		fprintf(file,"%s %s %s %s %f %f %f %f\n",
			f.Atoms[i].Residue,
			f.Atoms[i].Symb,
			f.Atoms[i].pdbType,
			f.Atoms[i].mmType,
			f.Atoms[i].Coord[0],
			f.Atoms[i].Coord[1],
			f.Atoms[i].Coord[2],
			f.Atoms[i].Charge
			);

	}
	fprintf(file,"%d %d %d\n",f.atomToDelete,f.atomToBondTo,f.angleAtom);
	fprintf(file,"End\n");
}
/**********************************************************************/
static void saveAllFragments(PersonalFragments* personalFragments, FILE* file)
{
	gint numberOfGroupes =  personalFragments->numberOfGroupes;
	gint numberOfFragments;
	gint i;
	gint j;

	for(i=0;i<numberOfGroupes;i++)
	{
		numberOfFragments = personalFragments->personalGroupes[i].numberOfFragments;
		for(j=0;j<numberOfFragments;j++)
			saveOneFragment(personalFragments,file,i,j);
	}
}
/**********************************************************************/
gboolean saveAllPersonalFragments(PersonalFragments* personalFragments,gchar* filename)
{
	FILE* file;
	file = FOpen(filename,"w");

	if(personalFragments == NULL)
		return FALSE;
	if(file == NULL)
		return FALSE;
	else
	{
		saveTitlePesoFrag(file);
		saveGroupesList(personalFragments,file);
		saveAllFragmentsList(personalFragments,file);
		saveAllFragments(personalFragments,file);
		fclose(file);
	}
	return TRUE;
}
/**********************************************************************/
