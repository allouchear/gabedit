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
#include "../Utils/Constants.h"
/************************************************************/
static void freeOneList(gchar** t, gint nl)
{
	gint i;
	if(nl<1)
		return;
	for(i=0;i<nl;i++)
		if(t[i])
			g_free(t[i]);
	if(t)
		g_free(t);
}
/************************************************************/
static gchar** getOneList(FILE* file,gint* nl, gchar* str, gboolean reading)
{
	gchar** t = (gchar**)g_malloc(sizeof(gchar*));
	
	gchar dump[BSIZE];
	gint len = BSIZE;
	gint n;
	gboolean Ok = FALSE;

	*nl = 0;
	fseek(file, 0L, SEEK_SET);
    	{ char* e = fgets(dump,len,file);}
	while(!feof(file))
	{
		if(fgets(dump,len,file))
		{
			if(strstr(dump,str))
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
		if(reading)
		{
			t[n] = (gchar*)g_malloc(BSIZE*sizeof(gchar));
			sscanf(dump,"%s",t[n]);
		}
		else
			t[n] = g_strdup(dump);

		n++;
	}
	if(n==0)
	{
		free(t);
		return NULL;
	}
	*nl = n;

	return t;
}
/************************************************************/
static void loadGroupesList(PersonalFragments* personalFragments, FILE* file)
{
	gint i;
	gint numberOfGroupes;
	gchar** groupes = getOneList(file,&numberOfGroupes,"Begin Groupes List",TRUE);

	personalFragments->numberOfGroupes = numberOfGroupes;
	personalFragments->personalGroupes= NULL;

	if(numberOfGroupes<1)
		return;

	personalFragments->personalGroupes = g_malloc(numberOfGroupes*sizeof(PersonalGroupe));
	for(i=0;i<personalFragments->numberOfGroupes;i++)
	{
		personalFragments->personalGroupes[i].groupName = g_strdup(groupes[i]);
		personalFragments->personalGroupes[i].numberOfFragments = 0;
		personalFragments->personalGroupes[i].fragments = NULL;
	}
	freeOneList(groupes,numberOfGroupes);

}
/**********************************************************************/
static void loadOneFragmentsList(PersonalFragments* personalFragments, FILE* file,gint groupeNumber)
{
	PersonalGroupe* personalGroupes = personalFragments->personalGroupes;
	gint numberOfFragments = 0;
	OnePersonalFragment* fragments = NULL;
	gint i;
	gchar* str = g_strdup_printf("Begin %s Groupe",personalGroupes[groupeNumber].groupName);
	gchar** fragmentsList = getOneList(file,&numberOfFragments,str,TRUE);

	if(numberOfFragments<1)
	{
		g_free(str);
		return;
	}

	fragments = g_malloc(numberOfFragments*sizeof(OnePersonalFragment));

	for(i=0;i<numberOfFragments;i++)
	{
		fragments[i].name = g_strdup(fragmentsList[i]);
	}

	personalGroupes[groupeNumber].numberOfFragments = numberOfFragments;
	personalGroupes[groupeNumber].fragments = fragments;

	freeOneList(fragmentsList,numberOfFragments);
	g_free(str);
}
/**********************************************************************/
static void loadAllFragmentsList(PersonalFragments* personalFragments, FILE* file)
{
	gint numberOfGroupes =  personalFragments->numberOfGroupes;
	gint i;

	for(i=0;i<numberOfGroupes;i++)
		loadOneFragmentsList(personalFragments,file,i);
}
/**********************************************************************/
static void loadOneFragment(PersonalFragments* personalFragments, FILE* file,
		gint groupeNumber, gint fragmentNumber)
{
	PersonalGroupe* personalGroupes = personalFragments->personalGroupes;
	OnePersonalFragment* fragments = personalGroupes[groupeNumber].fragments;
	Fragment f = fragments[fragmentNumber].f;
	gint i;
	gchar* str = g_strdup_printf("Begin %s %s Fragment",
			personalGroupes[groupeNumber].groupName,
			fragments[fragmentNumber].name
			);
	gint nlines;
	gchar** list = getOneList(file,&nlines,str,FALSE);
	gchar dump1[BSIZE];
	gchar dump2[BSIZE];
	gchar dump3[BSIZE];
	gchar dump4[BSIZE];

	f.NAtoms = 0;
	f.Atoms = NULL;
	if(nlines<1)
	{
		g_free(str);
		return;
	}
	f.NAtoms = atoi(list[0]);
	if(f.NAtoms<1)
		return;
	f.Atoms = g_malloc(f.NAtoms*sizeof(Atom));
	for(i=0;i<f.NAtoms;i++)
	{
		f.Atoms[i].Coord[0] = 0;
		f.Atoms[i].Coord[1] = 0;
		f.Atoms[i].Coord[2] = 0;
		f.Atoms[i].Charge = 0;
		sprintf(dump1,"DUM");
		sprintf(dump2,"X");
		sprintf(dump3,"X");
		sscanf(list[i+1],"%s %s %s %s %lf %lf %lf %lf",
			dump1,dump2,dump3,dump4,
			&f.Atoms[i].Coord[0],
			&f.Atoms[i].Coord[1],
			&f.Atoms[i].Coord[2],
			&f.Atoms[i].Charge
				);
		f.Atoms[i].Residue = g_strdup(dump1);
		f.Atoms[i].Symb = g_strdup(dump2);
		f.Atoms[i].pdbType = g_strdup(dump3);
		f.Atoms[i].mmType = g_strdup(dump4);
		/*
		printf("%s %s %s %f %f %f %f\n",
				f.Atoms[i].Residue,
				f.Atoms[i].Symb,
				f.Atoms[i].pdbType,
				f.Atoms[i].mmType,
				f.Atoms[i].Coord[0],
				f.Atoms[i].Coord[1],
				f.Atoms[i].Coord[2],
				f.Atoms[i].Charge);
				*/
	}
	sscanf(list[nlines-1],"%d %d %d",&f.atomToDelete,&f.atomToBondTo,&f.angleAtom);
	fragments[fragmentNumber].f = f;

	freeOneList(list,nlines);
	g_free(str);
}
/**********************************************************************/
static void loadAllFragments(PersonalFragments* personalFragments, FILE* file)
{
	gint numberOfGroupes =  personalFragments->numberOfGroupes;
	gint numberOfFragments;
	gint i;
	gint j;

	for(i=0;i<numberOfGroupes;i++)
	{
		numberOfFragments = personalFragments->personalGroupes[i].numberOfFragments;
		for(j=0;j<numberOfFragments;j++)
			loadOneFragment(personalFragments,file,i,j);
	}
}
/**********************************************************************/
PersonalFragments* loadAllPersonalFragments(gchar* filename)
{
	FILE* file;
	PersonalFragments* personalFragments;

	file = FOpen(filename,"rb");

	if(file == NULL)
		return NULL;

	personalFragments = g_malloc(sizeof(PersonalFragments));
	personalFragments->numberOfGroupes = 0;
	personalFragments->personalGroupes= NULL;

	loadGroupesList(personalFragments,file);
	
	if(personalFragments->numberOfGroupes == 0)
	{
		g_free(personalFragments);
		personalFragments = NULL;
		if(file)
			fclose(file);
		return NULL;
	}
	loadAllFragmentsList(personalFragments,file);
	loadAllFragments(personalFragments,file);
	fclose(file);

	return personalFragments;
}
/**********************************************************************/
