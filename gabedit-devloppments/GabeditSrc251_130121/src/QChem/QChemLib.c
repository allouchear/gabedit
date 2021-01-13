/* QChemLib.c */
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
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "../Common/GabeditType.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "../QChem/QChemLib.h"
/********************************************************************************/
GList* getQChemBasisList(gchar* Symb, gchar* message)
{
	GList *glist = NULL;
	gchar *filename = g_strdup_printf("%s%sqchemBasis",gabedit_directory(), G_DIR_SEPARATOR_S);
	gchar* basisName= NULL;
	gchar symb[6];
	gint nbas = 0;
	gint n = 0;
	gint j;
	gchar* t = NULL;
	gchar* dump = NULL;
	gint k;

	FILE* fin = FOpen(filename,"r");
	if(!fin)
	{
		create_qchem_basis_file();
		fin = FOpen(filename,"r");
	}
	if(!fin)
	{
		sprintf(message,_("Sorry the qchemBasis is corrupted.\nPlease reinstall gabedit\n"));
		return NULL;
	}
  

	t = g_malloc(BSIZE*sizeof(gchar));
	dump = g_malloc(BSIZE*sizeof(gchar));
	basisName= g_malloc(BSIZE*sizeof(gchar));

    	{ char* e = fgets(t,BSIZE,fin);} /* number of atoms */
	while(!feof(fin))
	{
		if(!fgets(t,BSIZE,fin)) break;
		if(strstr(t,"Atom") && strstr(t,Symb) && sscanf(t,"%s %s",dump,symb)==2 && strcmp(Symb,symb) ==0 )
		{
			if(!fgets(t,BSIZE,fin)) break;
			sscanf(t,"%d",&nbas);
			for(j=0;j<nbas;j++)
			{
				if(!fgets(t,BSIZE,fin)) break;
				n = sscanf(t,"%s %s",symb, basisName);
				if(n<2)
				{
					nbas = 0;
		       			g_list_free(glist);
					glist = NULL;
					sprintf(message,_("Sorry the qchemBasis is corrupted.\nPlease reinstall gabedit\n"));
					break;
				}
				for(k=0;k<(gint)strlen(basisName);k++) if (t[k]=='#') t[k] = ' ';
		       		glist = g_list_append(glist,g_strdup(basisName));
			}
			break;
		}
	}

	fclose(fin);
	g_free(t);
	g_free(dump);
	g_free(basisName);
	return glist;
}
/********************************************************************************/
GList* getQChemBasisListOfAtoms(GList* atomsList, gchar* message)
{
	GList *basisList = NULL;
	gchar *filename = g_strdup_printf("%s%sqchemBasis",gabedit_directory(), G_DIR_SEPARATOR_S);
	gchar* basisName= NULL;
	gchar symb[6];
	gint nbas = 0;
	gint n = 0;
	gint i;
	gint j;
	gint k;
	gchar* t = NULL;
	gchar* dump = NULL;
	gboolean Ok;
	gboolean first = TRUE;
	gchar** list1 = NULL;
	gchar** list2 = NULL;
	gint n1 = 0;
	gint n2 = 0;

	FILE* fin = FOpen(filename,"r");

	sprintf(message,"No problem");

	if(!fin)
	{
		create_qchem_basis_file();
		fin = FOpen(filename,"r");
	}
	if(!fin)
	{
		sprintf(message,_("Sorry the qchemBasis is corrupted.\nPlease reinstall gabedit\n"));
		return NULL;
	}

	t = g_malloc(BSIZE*sizeof(gchar));
	dump = g_malloc(BSIZE*sizeof(gchar));
	basisName= g_malloc(BSIZE*sizeof(gchar));

    	{ char* e = fgets(t,BSIZE,fin);} /* number of atoms */
	while(!feof(fin))
	{
		if(!fgets(t,BSIZE,fin)) break;
		Ok = FALSE;
		if(strstr(t,"Atom") && sscanf(t,"%s %s",dump,symb)==2)
		{
			GList* l = atomsList;
			while(l)
			{
				if(strcmp(l->data,symb)==0)
				{
					Ok = TRUE;
					break;
				}
				l = l->next;
			}

		}
		if(Ok)
		{
			if(!fgets(t,BSIZE,fin)) break;
			sscanf(t,"%d",&nbas);
			if(nbas>0)
			{
				if(first)
				{
					list1 = g_malloc(nbas*sizeof(gchar*));
					n1 = nbas;
				}
				else
				{
					list2 = g_malloc(nbas*sizeof(gchar*));
					n2 = nbas;
				}
			}
			for(j=0;j<nbas;j++)
			{
				if(!fgets(t,BSIZE,fin)) break;
				n = sscanf(t,"%s %s",symb, basisName);
				for(k=0;k<(gint)strlen(basisName);k++) if (basisName[k]=='#') basisName[k] = ' ';
				if(n<2)
				{
					nbas = -1;
					basisList = NULL;
					sprintf(message,_("Sorry the qchemBasis is corrupted.\nPlease reinstall gabedit\n"));
					break;
				}
				if(first)
				{
		       			list1[j] =g_strdup(basisName);
				}
				else
				{
		       			list2[j] =g_strdup(basisName);
				}
			}
			if(nbas == -1) break;
			if(!first)
			{
				for(i=0;i<n1;i++)
				{
					if(!list1[i]) continue;
					Ok = FALSE;
					for(j=0;j<n2;j++)
					{
						if(strcmp(list1[i],list2[j])==0)
						{
							Ok = TRUE;
							break;
						}
					}
					if(!Ok)
					{
						if(list1[i]) g_free(list1[i]);
						list1[i] = NULL;
					}
				}
				if(list2)
				{
					for(j=0;j<n2;j++) if(list2[j])g_free(list2[j]);
					g_free(list2);
					n2 = 0;
				}

			}
			if(nbas>0) first = FALSE;
		}
		else
		{
			if(!fgets(t,BSIZE,fin)) break;
			sscanf(t,"%d",&nbas);
			for(j=0;j<nbas;j++) if(!fgets(t,BSIZE,fin)) break;
		}
	}
	for(i=0;i<n1;i++)
	{
		if(list1[i]) basisList = g_list_append(basisList,list1[i]);
	}
	if(list1) g_free(list1); /* but not for(...) g_free(list1[i]);*/

	fclose(fin);
	g_free(t);
	g_free(dump);
	g_free(basisName);
	return basisList;
}
/********************************************************************************/
gboolean create_qchem_basis_file()
{
        gchar *fileName =  g_strdup_printf("%s/qchemBasis",gabedit_directory());  
	FILE* file = fopen(fileName,"w");

	g_free(fileName);
	if(!file) return FALSE;

	fprintf(file,"NAtoms = 104\n");
	fprintf(file,"Atom  H\n");
	fprintf(file,"54\n");
	fprintf(file,"H STO-2G \n");
	fprintf(file,"H STO-3G \n");
	fprintf(file,"H STO-6G \n");
	fprintf(file,"H 3-21G \n");
	fprintf(file,"H 4-31G \n");
	fprintf(file,"H 6-311G \n");
	fprintf(file,"H 6-311G(2d,2p) \n");
	fprintf(file,"H 6-311G(2df,2pd) \n");
	fprintf(file,"H 6-311G(3d,3p) \n");
	fprintf(file,"H 6-311G(3df,3pd) \n");
	fprintf(file,"H 6-311G(df,pd) \n");
	fprintf(file,"H 6-311G** \n");
	fprintf(file,"H 6-31G \n");
	fprintf(file,"H 6-31G(2d,2p) \n");
	fprintf(file,"H 6-31G(2df,2pd) \n");
	fprintf(file,"H 6-31G(3d,3p) \n");
	fprintf(file,"H 6-31G(3df,3pd) \n");
	fprintf(file,"H 6-31G** \n");
	fprintf(file,"H Ahlrichs \n");
	fprintf(file,"H Ahlrichs_RIMP2_SVP \n");
	fprintf(file,"H aug-cc-pVDZ \n");
	fprintf(file,"H aug-cc-pVTZ \n");
	fprintf(file,"H aug-cc-pVQZ \n");
	fprintf(file,"H aug-cc-pV5Z \n");
	fprintf(file,"H cc-pVDZ \n");
	fprintf(file,"H cc-pVTZ \n");
	fprintf(file,"H cc-pVQZ \n");
	fprintf(file,"H cc-pV5Z \n");
	fprintf(file,"H CRENBL \n");
	fprintf(file,"H DGauss \n");
	fprintf(file,"H Diffuse \n");
	fprintf(file,"H DZ \n");
	fprintf(file,"H DZP \n");
	fprintf(file,"H G3large \n");
	fprintf(file,"H G3MP2large \n");
	fprintf(file,"H Hay-Wadt \n");
	fprintf(file,"H LACVP \n");
	fprintf(file,"H LANL2DZ \n");
	fprintf(file,"H Pol \n");
	fprintf(file,"H Pople-style \n");
	fprintf(file,"H rcc-pVTZ \n");
	fprintf(file,"H rcc-pVQZ \n");
	fprintf(file,"H rimp2-aug-cc-pVDZ \n");
	fprintf(file,"H rimp2-aug-cc-pVTZ \n");
	fprintf(file,"H rimp2-aug-cc-pVQZ \n");
	fprintf(file,"H rimp2-cc-pVDZ \n");
	fprintf(file,"H rimp2-cc-pVTZ \n");
	fprintf(file,"H rimp2-cc-pVQZ \n");
	fprintf(file,"H rimp2-SVP \n");
	fprintf(file,"H rimp2-TZVPP \n");
	fprintf(file,"H SBKJC \n");
	fprintf(file,"H Stuttgart \n");
	fprintf(file,"H SV \n");
	fprintf(file,"H TZ \n");
	fprintf(file,"Atom  He\n");
	fprintf(file,"41\n");
	fprintf(file,"He STO-2G \n");
	fprintf(file,"He STO-3G \n");
	fprintf(file,"He STO-6G \n");
	fprintf(file,"He 3-21G \n");
	fprintf(file,"He 4-31G \n");
	fprintf(file,"He 6-311G \n");
	fprintf(file,"He 6-311G(2d,2p) \n");
	fprintf(file,"He 6-311G(2df,2pd) \n");
	fprintf(file,"He 6-311G(3d,3p) \n");
	fprintf(file,"He 6-311G(3df,3pd) \n");
	fprintf(file,"He 6-311G(df,pd) \n");
	fprintf(file,"He 6-311G** \n");
	fprintf(file,"He 6-31G \n");
	fprintf(file,"He 6-31G** \n");
	fprintf(file,"He Ahlrichs \n");
	fprintf(file,"He aug-cc-pVDZ \n");
	fprintf(file,"He aug-cc-pVTZ \n");
	fprintf(file,"He aug-cc-pVQZ \n");
	fprintf(file,"He aug-cc-pV5Z \n");
	fprintf(file,"He cc-pVDZ \n");
	fprintf(file,"He cc-pVTZ \n");
	fprintf(file,"He cc-pVQZ \n");
	fprintf(file,"He cc-pV5Z \n");
	fprintf(file,"He CRENBL \n");
	fprintf(file,"He DGauss \n");
	fprintf(file,"He G3large \n");
	fprintf(file,"He G3MP2large \n");
	fprintf(file,"He Hay-Wadt \n");
	fprintf(file,"He LACVP \n");
	fprintf(file,"He LANL2DZ \n");
	fprintf(file,"He rcc-pVTZ \n");
	fprintf(file,"He rcc-pVQZ \n");
	fprintf(file,"He rimp2-aug-cc-pVDZ \n");
	fprintf(file,"He rimp2-aug-cc-pVTZ \n");
	fprintf(file,"He rimp2-aug-cc-pVQZ \n");
	fprintf(file,"He rimp2-cc-pVDZ \n");
	fprintf(file,"He rimp2-cc-pVTZ \n");
	fprintf(file,"He rimp2-cc-pVQZ \n");
	fprintf(file,"He rimp2-SVP \n");
	fprintf(file,"He SBKJC \n");
	fprintf(file,"He Stuttgart \n");
	fprintf(file,"Atom  Li\n");
	fprintf(file,"34\n");
	fprintf(file,"Li STO-2G \n");
	fprintf(file,"Li STO-3G \n");
	fprintf(file,"Li STO-6G \n");
	fprintf(file,"Li 3-21G \n");
	fprintf(file,"Li 4-31G \n");
	fprintf(file,"Li 6-311G \n");
	fprintf(file,"Li 6-311G(2d,2p) \n");
	fprintf(file,"Li 6-311G(2df,2pd) \n");
	fprintf(file,"Li 6-311G(3d,3p) \n");
	fprintf(file,"Li 6-311G(3df,3pd) \n");
	fprintf(file,"Li 6-311G(df,pd) \n");
	fprintf(file,"Li 6-311G** \n");
	fprintf(file,"Li 6-31G \n");
	fprintf(file,"Li 6-31G** \n");
	fprintf(file,"Li Ahlrichs \n");
	fprintf(file,"Li cc-pVDZ \n");
	fprintf(file,"Li cc-pVTZ \n");
	fprintf(file,"Li cc-pVQZ \n");
	fprintf(file,"Li cc-pV5Z \n");
	fprintf(file,"Li CRENBL \n");
	fprintf(file,"Li DGauss \n");
	fprintf(file,"Li G3large \n");
	fprintf(file,"Li G3MP2large \n");
	fprintf(file,"Li Hay-Wadt \n");
	fprintf(file,"Li LACVP \n");
	fprintf(file,"Li LANL2DZ \n");
	fprintf(file,"Li rcc-pVTZ \n");
	fprintf(file,"Li rcc-pVQZ \n");
	fprintf(file,"Li rimp2-cc-pVDZ \n");
	fprintf(file,"Li rimp2-cc-pVTZ \n");
	fprintf(file,"Li rimp2-cc-pVQZ \n");
	fprintf(file,"Li rimp2-SVP \n");
	fprintf(file,"Li SBKJC \n");
	fprintf(file,"Li Stuttgart \n");
	fprintf(file,"Atom  Be\n");
	fprintf(file,"34\n");
	fprintf(file,"Be STO-2G \n");
	fprintf(file,"Be STO-3G \n");
	fprintf(file,"Be STO-6G \n");
	fprintf(file,"Be 3-21G \n");
	fprintf(file,"Be 4-31G \n");
	fprintf(file,"Be 6-311G \n");
	fprintf(file,"Be 6-311G(2d,2p) \n");
	fprintf(file,"Be 6-311G(2df,2pd) \n");
	fprintf(file,"Be 6-311G(3d,3p) \n");
	fprintf(file,"Be 6-311G(3df,3pd) \n");
	fprintf(file,"Be 6-311G(df,pd) \n");
	fprintf(file,"Be 6-311G** \n");
	fprintf(file,"Be 6-31G \n");
	fprintf(file,"Be 6-31G** \n");
	fprintf(file,"Be Ahlrichs \n");
	fprintf(file,"Be cc-pVDZ \n");
	fprintf(file,"Be cc-pVTZ \n");
	fprintf(file,"Be cc-pVQZ \n");
	fprintf(file,"Be cc-pV5Z \n");
	fprintf(file,"Be CRENBL \n");
	fprintf(file,"Be DGauss \n");
	fprintf(file,"Be G3large \n");
	fprintf(file,"Be G3MP2large \n");
	fprintf(file,"Be Hay-Wadt \n");
	fprintf(file,"Be LACVP \n");
	fprintf(file,"Be LANL2DZ \n");
	fprintf(file,"Be rcc-pVTZ \n");
	fprintf(file,"Be rcc-pVQZ \n");
	fprintf(file,"Be rimp2-cc-pVDZ \n");
	fprintf(file,"Be rimp2-cc-pVTZ \n");
	fprintf(file,"Be rimp2-cc-pVQZ \n");
	fprintf(file,"Be rimp2-SVP \n");
	fprintf(file,"Be SBKJC \n");
	fprintf(file,"Be Stuttgart \n");
	fprintf(file,"Atom  B\n");
	fprintf(file,"34\n");
	fprintf(file,"B STO-2G \n");
	fprintf(file,"B STO-3G \n");
	fprintf(file,"B STO-6G \n");
	fprintf(file,"B 3-21G \n");
	fprintf(file,"B 4-31G \n");
	fprintf(file,"B 6-311G \n");
	fprintf(file,"B 6-311G(2d,2p) \n");
	fprintf(file,"B 6-311G(2df,2pd) \n");
	fprintf(file,"B 6-311G(3d,3p) \n");
	fprintf(file,"B 6-311G(3df,3pd) \n");
	fprintf(file,"B 6-311G(df,pd) \n");
	fprintf(file,"B 6-311G** \n");
	fprintf(file,"B 6-31G \n");
	fprintf(file,"B 6-31G** \n");
	fprintf(file,"B Ahlrichs \n");
	fprintf(file,"B cc-pVDZ \n");
	fprintf(file,"B cc-pVTZ \n");
	fprintf(file,"B cc-pVQZ \n");
	fprintf(file,"B cc-pV5Z \n");
	fprintf(file,"B CRENBL \n");
	fprintf(file,"B DGauss \n");
	fprintf(file,"B G3large \n");
	fprintf(file,"B G3MP2large \n");
	fprintf(file,"B Hay-Wadt \n");
	fprintf(file,"B LACVP \n");
	fprintf(file,"B LANL2DZ \n");
	fprintf(file,"B rcc-pVTZ \n");
	fprintf(file,"B rcc-pVQZ \n");
	fprintf(file,"B rimp2-cc-pVDZ \n");
	fprintf(file,"B rimp2-cc-pVTZ \n");
	fprintf(file,"B rimp2-cc-pVQZ \n");
	fprintf(file,"B rimp2-SVP \n");
	fprintf(file,"B SBKJC \n");
	fprintf(file,"B Stuttgart \n");
	fprintf(file,"Atom  C\n");
	fprintf(file,"34\n");
	fprintf(file,"C STO-2G \n");
	fprintf(file,"C STO-3G \n");
	fprintf(file,"C STO-6G \n");
	fprintf(file,"C 3-21G \n");
	fprintf(file,"C 4-31G \n");
	fprintf(file,"C 6-311G \n");
	fprintf(file,"C 6-311G(2d,2p) \n");
	fprintf(file,"C 6-311G(2df,2pd) \n");
	fprintf(file,"C 6-311G(3d,3p) \n");
	fprintf(file,"C 6-311G(3df,3pd) \n");
	fprintf(file,"C 6-311G(df,pd) \n");
	fprintf(file,"C 6-311G** \n");
	fprintf(file,"C 6-31G \n");
	fprintf(file,"C 6-31G** \n");
	fprintf(file,"C Ahlrichs \n");
	fprintf(file,"C cc-pVDZ \n");
	fprintf(file,"C cc-pVTZ \n");
	fprintf(file,"C cc-pVQZ \n");
	fprintf(file,"C cc-pV5Z \n");
	fprintf(file,"C CRENBL \n");
	fprintf(file,"C DGauss \n");
	fprintf(file,"C G3large \n");
	fprintf(file,"C G3MP2large \n");
	fprintf(file,"C Hay-Wadt \n");
	fprintf(file,"C LACVP \n");
	fprintf(file,"C LANL2DZ \n");
	fprintf(file,"C rcc-pVTZ \n");
	fprintf(file,"C rcc-pVQZ \n");
	fprintf(file,"C rimp2-cc-pVDZ \n");
	fprintf(file,"C rimp2-cc-pVTZ \n");
	fprintf(file,"C rimp2-cc-pVQZ \n");
	fprintf(file,"C rimp2-SVP \n");
	fprintf(file,"C SBKJC \n");
	fprintf(file,"C Stuttgart \n");
	fprintf(file,"Atom  N\n");
	fprintf(file,"34\n");
	fprintf(file,"N STO-2G \n");
	fprintf(file,"N STO-3G \n");
	fprintf(file,"N STO-6G \n");
	fprintf(file,"N 3-21G \n");
	fprintf(file,"N 4-31G \n");
	fprintf(file,"N 6-311G \n");
	fprintf(file,"N 6-311G(2d,2p) \n");
	fprintf(file,"N 6-311G(2df,2pd) \n");
	fprintf(file,"N 6-311G(3d,3p) \n");
	fprintf(file,"N 6-311G(3df,3pd) \n");
	fprintf(file,"N 6-311G(df,pd) \n");
	fprintf(file,"N 6-311G** \n");
	fprintf(file,"N 6-31G \n");
	fprintf(file,"N 6-31G** \n");
	fprintf(file,"N Ahlrichs \n");
	fprintf(file,"N cc-pVDZ \n");
	fprintf(file,"N cc-pVTZ \n");
	fprintf(file,"N cc-pVQZ \n");
	fprintf(file,"N cc-pV5Z \n");
	fprintf(file,"N CRENBL \n");
	fprintf(file,"N DGauss \n");
	fprintf(file,"N G3large \n");
	fprintf(file,"N G3MP2large \n");
	fprintf(file,"N Hay-Wadt \n");
	fprintf(file,"N LACVP \n");
	fprintf(file,"N LANL2DZ \n");
	fprintf(file,"N rcc-pVTZ \n");
	fprintf(file,"N rcc-pVQZ \n");
	fprintf(file,"N rimp2-cc-pVDZ \n");
	fprintf(file,"N rimp2-cc-pVTZ \n");
	fprintf(file,"N rimp2-cc-pVQZ \n");
	fprintf(file,"N rimp2-SVP \n");
	fprintf(file,"N SBKJC \n");
	fprintf(file,"N Stuttgart \n");
	fprintf(file,"Atom  O\n");
	fprintf(file,"34\n");
	fprintf(file,"O STO-2G \n");
	fprintf(file,"O STO-3G \n");
	fprintf(file,"O STO-6G \n");
	fprintf(file,"O 3-21G \n");
	fprintf(file,"O 4-31G \n");
	fprintf(file,"O 6-311G \n");
	fprintf(file,"O 6-311G(2d,2p) \n");
	fprintf(file,"O 6-311G(2df,2pd) \n");
	fprintf(file,"O 6-311G(3d,3p) \n");
	fprintf(file,"O 6-311G(3df,3pd) \n");
	fprintf(file,"O 6-311G(df,pd) \n");
	fprintf(file,"O 6-311G** \n");
	fprintf(file,"O 6-31G \n");
	fprintf(file,"O 6-31G** \n");
	fprintf(file,"O Ahlrichs \n");
	fprintf(file,"O cc-pVDZ \n");
	fprintf(file,"O cc-pVTZ \n");
	fprintf(file,"O cc-pVQZ \n");
	fprintf(file,"O cc-pV5Z \n");
	fprintf(file,"O CRENBL \n");
	fprintf(file,"O DGauss \n");
	fprintf(file,"O G3large \n");
	fprintf(file,"O G3MP2large \n");
	fprintf(file,"O Hay-Wadt \n");
	fprintf(file,"O LACVP \n");
	fprintf(file,"O LANL2DZ \n");
	fprintf(file,"O rcc-pVTZ \n");
	fprintf(file,"O rcc-pVQZ \n");
	fprintf(file,"O rimp2-cc-pVDZ \n");
	fprintf(file,"O rimp2-cc-pVTZ \n");
	fprintf(file,"O rimp2-cc-pVQZ \n");
	fprintf(file,"O rimp2-SVP \n");
	fprintf(file,"O SBKJC \n");
	fprintf(file,"O Stuttgart \n");
	fprintf(file,"Atom  F\n");
	fprintf(file,"34\n");
	fprintf(file,"F STO-2G \n");
	fprintf(file,"F STO-3G \n");
	fprintf(file,"F STO-6G \n");
	fprintf(file,"F 3-21G \n");
	fprintf(file,"F 4-31G \n");
	fprintf(file,"F 6-311G \n");
	fprintf(file,"F 6-311G(2d,2p) \n");
	fprintf(file,"F 6-311G(2df,2pd) \n");
	fprintf(file,"F 6-311G(3d,3p) \n");
	fprintf(file,"F 6-311G(3df,3pd) \n");
	fprintf(file,"F 6-311G(df,pd) \n");
	fprintf(file,"F 6-311G** \n");
	fprintf(file,"F 6-31G \n");
	fprintf(file,"F 6-31G** \n");
	fprintf(file,"F Ahlrichs \n");
	fprintf(file,"F cc-pVDZ \n");
	fprintf(file,"F cc-pVTZ \n");
	fprintf(file,"F cc-pVQZ \n");
	fprintf(file,"F cc-pV5Z \n");
	fprintf(file,"F CRENBL \n");
	fprintf(file,"F DGauss \n");
	fprintf(file,"F G3large \n");
	fprintf(file,"F G3MP2large \n");
	fprintf(file,"F Hay-Wadt \n");
	fprintf(file,"F LACVP \n");
	fprintf(file,"F LANL2DZ \n");
	fprintf(file,"F rcc-pVTZ \n");
	fprintf(file,"F rcc-pVQZ \n");
	fprintf(file,"F rimp2-cc-pVDZ \n");
	fprintf(file,"F rimp2-cc-pVTZ \n");
	fprintf(file,"F rimp2-cc-pVQZ \n");
	fprintf(file,"F rimp2-SVP \n");
	fprintf(file,"F SBKJC \n");
	fprintf(file,"F Stuttgart \n");
	fprintf(file,"Atom  Ne\n");
	fprintf(file,"34\n");
	fprintf(file,"Ne STO-2G \n");
	fprintf(file,"Ne STO-3G \n");
	fprintf(file,"Ne STO-6G \n");
	fprintf(file,"Ne 3-21G \n");
	fprintf(file,"Ne 4-31G \n");
	fprintf(file,"Ne 6-311G \n");
	fprintf(file,"Ne 6-311G(2d,2p) \n");
	fprintf(file,"Ne 6-311G(2df,2pd) \n");
	fprintf(file,"Ne 6-311G(3d,3p) \n");
	fprintf(file,"Ne 6-311G(3df,3pd) \n");
	fprintf(file,"Ne 6-311G(df,pd) \n");
	fprintf(file,"Ne 6-311G** \n");
	fprintf(file,"Ne 6-31G \n");
	fprintf(file,"Ne 6-31G** \n");
	fprintf(file,"Ne Ahlrichs \n");
	fprintf(file,"Ne cc-pVDZ \n");
	fprintf(file,"Ne cc-pVTZ \n");
	fprintf(file,"Ne cc-pVQZ \n");
	fprintf(file,"Ne cc-pV5Z \n");
	fprintf(file,"Ne CRENBL \n");
	fprintf(file,"Ne DGauss \n");
	fprintf(file,"Ne G3large \n");
	fprintf(file,"Ne G3MP2large \n");
	fprintf(file,"Ne Hay-Wadt \n");
	fprintf(file,"Ne LACVP \n");
	fprintf(file,"Ne LANL2DZ \n");
	fprintf(file,"Ne rcc-pVTZ \n");
	fprintf(file,"Ne rcc-pVQZ \n");
	fprintf(file,"Ne rimp2-cc-pVDZ \n");
	fprintf(file,"Ne rimp2-cc-pVTZ \n");
	fprintf(file,"Ne rimp2-cc-pVQZ \n");
	fprintf(file,"Ne rimp2-SVP \n");
	fprintf(file,"Ne SBKJC \n");
	fprintf(file,"Ne Stuttgart \n");
	fprintf(file,"Atom  Na\n");
	fprintf(file,"33\n");
	fprintf(file,"Na STO-2G \n");
	fprintf(file,"Na STO-3G \n");
	fprintf(file,"Na STO-6G \n");
	fprintf(file,"Na 3-21G \n");
	fprintf(file,"Na 6-311G \n");
	fprintf(file,"Na 6-311G(2d,2p) \n");
	fprintf(file,"Na 6-311G(2df,2pd) \n");
	fprintf(file,"Na 6-311G(3d,3p) \n");
	fprintf(file,"Na 6-311G(3df,3pd) \n");
	fprintf(file,"Na 6-311G(df,pd) \n");
	fprintf(file,"Na 6-311G** \n");
	fprintf(file,"Na 6-31G \n");
	fprintf(file,"Na 6-31G** \n");
	fprintf(file,"Na Ahlrichs \n");
	fprintf(file,"Na cc-pVDZ \n");
	fprintf(file,"Na cc-pVTZ \n");
	fprintf(file,"Na cc-pVQZ \n");
	fprintf(file,"Na cc-pV5Z \n");
	fprintf(file,"Na CRENBL \n");
	fprintf(file,"Na DGauss \n");
	fprintf(file,"Na G3large \n");
	fprintf(file,"Na G3MP2large \n");
	fprintf(file,"Na Hay-Wadt \n");
	fprintf(file,"Na LACVP \n");
	fprintf(file,"Na LANL2DZ \n");
	fprintf(file,"Na rcc-pVTZ \n");
	fprintf(file,"Na rcc-pVQZ \n");
	fprintf(file,"Na rimp2-cc-pVDZ \n");
	fprintf(file,"Na rimp2-cc-pVTZ \n");
	fprintf(file,"Na rimp2-cc-pVQZ \n");
	fprintf(file,"Na rimp2-SVP \n");
	fprintf(file,"Na SBKJC \n");
	fprintf(file,"Na Stuttgart \n");
	fprintf(file,"Atom  Mg\n");
	fprintf(file,"31\n");
	fprintf(file,"Mg STO-2G \n");
	fprintf(file,"Mg STO-3G \n");
	fprintf(file,"Mg STO-6G \n");
	fprintf(file,"Mg 3-21G \n");
	fprintf(file,"Mg 6-311G \n");
	fprintf(file,"Mg 6-311G(2d,2p) \n");
	fprintf(file,"Mg 6-311G(2df,2pd) \n");
	fprintf(file,"Mg 6-311G(3d,3p) \n");
	fprintf(file,"Mg 6-311G(3df,3pd) \n");
	fprintf(file,"Mg 6-311G(df,pd) \n");
	fprintf(file,"Mg 6-311G** \n");
	fprintf(file,"Mg 6-31G \n");
	fprintf(file,"Mg 6-31G** \n");
	fprintf(file,"Mg Ahlrichs \n");
	fprintf(file,"Mg cc-pVDZ \n");
	fprintf(file,"Mg cc-pVTZ \n");
	fprintf(file,"Mg cc-pVQZ \n");
	fprintf(file,"Mg cc-pV5Z \n");
	fprintf(file,"Mg CRENBL \n");
	fprintf(file,"Mg DGauss \n");
	fprintf(file,"Mg G3large \n");
	fprintf(file,"Mg G3MP2large \n");
	fprintf(file,"Mg Hay-Wadt \n");
	fprintf(file,"Mg LACVP \n");
	fprintf(file,"Mg LANL2DZ \n");
	fprintf(file,"Mg rimp2-cc-pVDZ \n");
	fprintf(file,"Mg rimp2-cc-pVTZ \n");
	fprintf(file,"Mg rimp2-cc-pVQZ \n");
	fprintf(file,"Mg rimp2-SVP \n");
	fprintf(file,"Mg SBKJC \n");
	fprintf(file,"Mg Stuttgart \n");
	fprintf(file,"Atom  Al\n");
	fprintf(file,"31\n");
	fprintf(file,"Al STO-2G \n");
	fprintf(file,"Al STO-3G \n");
	fprintf(file,"Al STO-6G \n");
	fprintf(file,"Al 3-21G \n");
	fprintf(file,"Al 6-311G \n");
	fprintf(file,"Al 6-311G(2d,2p) \n");
	fprintf(file,"Al 6-311G(2df,2pd) \n");
	fprintf(file,"Al 6-311G(3d,3p) \n");
	fprintf(file,"Al 6-311G(3df,3pd) \n");
	fprintf(file,"Al 6-311G(df,pd) \n");
	fprintf(file,"Al 6-311G** \n");
	fprintf(file,"Al 6-31G \n");
	fprintf(file,"Al 6-31G** \n");
	fprintf(file,"Al Ahlrichs \n");
	fprintf(file,"Al cc-pVDZ \n");
	fprintf(file,"Al cc-pVTZ \n");
	fprintf(file,"Al cc-pVQZ \n");
	fprintf(file,"Al cc-pV5Z \n");
	fprintf(file,"Al CRENBL \n");
	fprintf(file,"Al DGauss \n");
	fprintf(file,"Al G3large \n");
	fprintf(file,"Al G3MP2large \n");
	fprintf(file,"Al Hay-Wadt \n");
	fprintf(file,"Al LACVP \n");
	fprintf(file,"Al LANL2DZ \n");
	fprintf(file,"Al rimp2-cc-pVDZ \n");
	fprintf(file,"Al rimp2-cc-pVTZ \n");
	fprintf(file,"Al rimp2-cc-pVQZ \n");
	fprintf(file,"Al rimp2-SVP \n");
	fprintf(file,"Al SBKJC \n");
	fprintf(file,"Al Stuttgart \n");
	fprintf(file,"Atom  Si\n");
	fprintf(file,"31\n");
	fprintf(file,"Si STO-2G \n");
	fprintf(file,"Si STO-3G \n");
	fprintf(file,"Si STO-6G \n");
	fprintf(file,"Si 3-21G \n");
	fprintf(file,"Si 6-311G \n");
	fprintf(file,"Si 6-311G(2d,2p) \n");
	fprintf(file,"Si 6-311G(2df,2pd) \n");
	fprintf(file,"Si 6-311G(3d,3p) \n");
	fprintf(file,"Si 6-311G(3df,3pd) \n");
	fprintf(file,"Si 6-311G(df,pd) \n");
	fprintf(file,"Si 6-311G** \n");
	fprintf(file,"Si 6-31G \n");
	fprintf(file,"Si 6-31G** \n");
	fprintf(file,"Si Ahlrichs \n");
	fprintf(file,"Si cc-pVDZ \n");
	fprintf(file,"Si cc-pVTZ \n");
	fprintf(file,"Si cc-pVQZ \n");
	fprintf(file,"Si cc-pV5Z \n");
	fprintf(file,"Si CRENBL \n");
	fprintf(file,"Si DGauss \n");
	fprintf(file,"Si G3large \n");
	fprintf(file,"Si G3MP2large \n");
	fprintf(file,"Si Hay-Wadt \n");
	fprintf(file,"Si LACVP \n");
	fprintf(file,"Si LANL2DZ \n");
	fprintf(file,"Si rimp2-cc-pVDZ \n");
	fprintf(file,"Si rimp2-cc-pVTZ \n");
	fprintf(file,"Si rimp2-cc-pVQZ \n");
	fprintf(file,"Si rimp2-SVP \n");
	fprintf(file,"Si SBKJC \n");
	fprintf(file,"Si Stuttgart \n");
	fprintf(file,"Atom  P\n");
	fprintf(file,"31\n");
	fprintf(file,"P STO-2G \n");
	fprintf(file,"P STO-3G \n");
	fprintf(file,"P STO-6G \n");
	fprintf(file,"P 3-21G \n");
	fprintf(file,"P 6-311G \n");
	fprintf(file,"P 6-311G(2d,2p) \n");
	fprintf(file,"P 6-311G(2df,2pd) \n");
	fprintf(file,"P 6-311G(3d,3p) \n");
	fprintf(file,"P 6-311G(3df,3pd) \n");
	fprintf(file,"P 6-311G(df,pd) \n");
	fprintf(file,"P 6-311G** \n");
	fprintf(file,"P 6-31G \n");
	fprintf(file,"P 6-31G** \n");
	fprintf(file,"P Ahlrichs \n");
	fprintf(file,"P cc-pVDZ \n");
	fprintf(file,"P cc-pVTZ \n");
	fprintf(file,"P cc-pVQZ \n");
	fprintf(file,"P cc-pV5Z \n");
	fprintf(file,"P CRENBL \n");
	fprintf(file,"P DGauss \n");
	fprintf(file,"P G3large \n");
	fprintf(file,"P G3MP2large \n");
	fprintf(file,"P Hay-Wadt \n");
	fprintf(file,"P LACVP \n");
	fprintf(file,"P LANL2DZ \n");
	fprintf(file,"P rimp2-cc-pVDZ \n");
	fprintf(file,"P rimp2-cc-pVTZ \n");
	fprintf(file,"P rimp2-cc-pVQZ \n");
	fprintf(file,"P rimp2-SVP \n");
	fprintf(file,"P SBKJC \n");
	fprintf(file,"P Stuttgart \n");
	fprintf(file,"Atom  S\n");
	fprintf(file,"31\n");
	fprintf(file,"S STO-2G \n");
	fprintf(file,"S STO-3G \n");
	fprintf(file,"S STO-6G \n");
	fprintf(file,"S 3-21G \n");
	fprintf(file,"S 6-311G \n");
	fprintf(file,"S 6-311G(2d,2p) \n");
	fprintf(file,"S 6-311G(2df,2pd) \n");
	fprintf(file,"S 6-311G(3d,3p) \n");
	fprintf(file,"S 6-311G(3df,3pd) \n");
	fprintf(file,"S 6-311G(df,pd) \n");
	fprintf(file,"S 6-311G** \n");
	fprintf(file,"S 6-31G \n");
	fprintf(file,"S 6-31G** \n");
	fprintf(file,"S Ahlrichs \n");
	fprintf(file,"S cc-pVDZ \n");
	fprintf(file,"S cc-pVTZ \n");
	fprintf(file,"S cc-pVQZ \n");
	fprintf(file,"S cc-pV5Z \n");
	fprintf(file,"S CRENBL \n");
	fprintf(file,"S DGauss \n");
	fprintf(file,"S G3large \n");
	fprintf(file,"S G3MP2large \n");
	fprintf(file,"S Hay-Wadt \n");
	fprintf(file,"S LACVP \n");
	fprintf(file,"S LANL2DZ \n");
	fprintf(file,"S rimp2-cc-pVDZ \n");
	fprintf(file,"S rimp2-cc-pVTZ \n");
	fprintf(file,"S rimp2-cc-pVQZ \n");
	fprintf(file,"S rimp2-SVP \n");
	fprintf(file,"S SBKJC \n");
	fprintf(file,"S Stuttgart \n");
	fprintf(file,"Atom  Cl\n");
	fprintf(file,"31\n");
	fprintf(file,"Cl STO-2G \n");
	fprintf(file,"Cl STO-3G \n");
	fprintf(file,"Cl STO-6G \n");
	fprintf(file,"Cl 3-21G \n");
	fprintf(file,"Cl 6-311G \n");
	fprintf(file,"Cl 6-311G(2d,2p) \n");
	fprintf(file,"Cl 6-311G(2df,2pd) \n");
	fprintf(file,"Cl 6-311G(3d,3p) \n");
	fprintf(file,"Cl 6-311G(3df,3pd) \n");
	fprintf(file,"Cl 6-311G(df,pd) \n");
	fprintf(file,"Cl 6-311G** \n");
	fprintf(file,"Cl 6-31G \n");
	fprintf(file,"Cl 6-31G** \n");
	fprintf(file,"Cl Ahlrichs \n");
	fprintf(file,"Cl cc-pVDZ \n");
	fprintf(file,"Cl cc-pVTZ \n");
	fprintf(file,"Cl cc-pVQZ \n");
	fprintf(file,"Cl cc-pV5Z \n");
	fprintf(file,"Cl CRENBL \n");
	fprintf(file,"Cl DGauss \n");
	fprintf(file,"Cl G3large \n");
	fprintf(file,"Cl G3MP2large \n");
	fprintf(file,"Cl Hay-Wadt \n");
	fprintf(file,"Cl LACVP \n");
	fprintf(file,"Cl LANL2DZ \n");
	fprintf(file,"Cl rimp2-cc-pVDZ \n");
	fprintf(file,"Cl rimp2-cc-pVTZ \n");
	fprintf(file,"Cl rimp2-cc-pVQZ \n");
	fprintf(file,"Cl rimp2-SVP \n");
	fprintf(file,"Cl SBKJC \n");
	fprintf(file,"Cl Stuttgart \n");
	fprintf(file,"Atom  Ar\n");
	fprintf(file,"31\n");
	fprintf(file,"Ar STO-2G \n");
	fprintf(file,"Ar STO-3G \n");
	fprintf(file,"Ar STO-6G \n");
	fprintf(file,"Ar 3-21G \n");
	fprintf(file,"Ar 6-311G \n");
	fprintf(file,"Ar 6-311G(2d,2p) \n");
	fprintf(file,"Ar 6-311G(2df,2pd) \n");
	fprintf(file,"Ar 6-311G(3d,3p) \n");
	fprintf(file,"Ar 6-311G(3df,3pd) \n");
	fprintf(file,"Ar 6-311G(df,pd) \n");
	fprintf(file,"Ar 6-311G** \n");
	fprintf(file,"Ar 6-31G \n");
	fprintf(file,"Ar 6-31G** \n");
	fprintf(file,"Ar Ahlrichs \n");
	fprintf(file,"Ar cc-pVDZ \n");
	fprintf(file,"Ar cc-pVTZ \n");
	fprintf(file,"Ar cc-pVQZ \n");
	fprintf(file,"Ar cc-pV5Z \n");
	fprintf(file,"Ar CRENBL \n");
	fprintf(file,"Ar DGauss \n");
	fprintf(file,"Ar G3large \n");
	fprintf(file,"Ar G3MP2large \n");
	fprintf(file,"Ar Hay-Wadt \n");
	fprintf(file,"Ar LACVP \n");
	fprintf(file,"Ar LANL2DZ \n");
	fprintf(file,"Ar rimp2-cc-pVDZ \n");
	fprintf(file,"Ar rimp2-cc-pVTZ \n");
	fprintf(file,"Ar rimp2-cc-pVQZ \n");
	fprintf(file,"Ar rimp2-SVP \n");
	fprintf(file,"Ar SBKJC \n");
	fprintf(file,"Ar Stuttgart \n");
	fprintf(file,"Atom  K\n");
	fprintf(file,"21\n");
	fprintf(file,"K STO-2G \n");
	fprintf(file,"K STO-3G \n");
	fprintf(file,"K STO-6G \n");
	fprintf(file,"K 3-21G \n");
	fprintf(file,"K 6-311G \n");
	fprintf(file,"K 6-311G(2d,2p) \n");
	fprintf(file,"K 6-311G(2df,2pd) \n");
	fprintf(file,"K 6-311G** \n");
	fprintf(file,"K 6-31G \n");
	fprintf(file,"K 6-31G** \n");
	fprintf(file,"K Ahlrichs \n");
	fprintf(file,"K CRENBL \n");
	fprintf(file,"K DGauss \n");
	fprintf(file,"K G3large \n");
	fprintf(file,"K G3MP2large \n");
	fprintf(file,"K Hay-Wadt \n");
	fprintf(file,"K LACVP \n");
	fprintf(file,"K LANL2DZ \n");
	fprintf(file,"K rimp2-SVP \n");
	fprintf(file,"K SBKJC \n");
	fprintf(file,"K Stuttgart \n");
	fprintf(file,"Atom  Ca\n");
	fprintf(file,"21\n");
	fprintf(file,"Ca STO-2G \n");
	fprintf(file,"Ca STO-3G \n");
	fprintf(file,"Ca STO-6G \n");
	fprintf(file,"Ca 3-21G \n");
	fprintf(file,"Ca 6-311G \n");
	fprintf(file,"Ca 6-311G(2d,2p) \n");
	fprintf(file,"Ca 6-311G(2df,2pd) \n");
	fprintf(file,"Ca 6-311G** \n");
	fprintf(file,"Ca 6-31G \n");
	fprintf(file,"Ca 6-31G** \n");
	fprintf(file,"Ca Ahlrichs \n");
	fprintf(file,"Ca CRENBL \n");
	fprintf(file,"Ca DGauss \n");
	fprintf(file,"Ca G3large \n");
	fprintf(file,"Ca G3MP2large \n");
	fprintf(file,"Ca Hay-Wadt \n");
	fprintf(file,"Ca LACVP \n");
	fprintf(file,"Ca LANL2DZ \n");
	fprintf(file,"Ca rimp2-SVP \n");
	fprintf(file,"Ca SBKJC \n");
	fprintf(file,"Ca Stuttgart \n");
	fprintf(file,"Atom  Sc\n");
	fprintf(file,"15\n");
	fprintf(file,"Sc STO-3G \n");
	fprintf(file,"Sc STO-6G \n");
	fprintf(file,"Sc 3-21G \n");
	fprintf(file,"Sc 6-31G \n");
	fprintf(file,"Sc 6-31G** \n");
	fprintf(file,"Sc Ahlrichs \n");
	fprintf(file,"Sc CRENBL \n");
	fprintf(file,"Sc DGauss \n");
	fprintf(file,"Sc G3MP2large \n");
	fprintf(file,"Sc Hay-Wadt \n");
	fprintf(file,"Sc LACVP \n");
	fprintf(file,"Sc LANL2DZ \n");
	fprintf(file,"Sc rimp2-SVP \n");
	fprintf(file,"Sc SBKJC \n");
	fprintf(file,"Sc Stuttgart \n");
	fprintf(file,"Atom  Ti\n");
	fprintf(file,"15\n");
	fprintf(file,"Ti STO-3G \n");
	fprintf(file,"Ti STO-6G \n");
	fprintf(file,"Ti 3-21G \n");
	fprintf(file,"Ti 6-31G \n");
	fprintf(file,"Ti 6-31G** \n");
	fprintf(file,"Ti Ahlrichs \n");
	fprintf(file,"Ti CRENBL \n");
	fprintf(file,"Ti DGauss \n");
	fprintf(file,"Ti G3MP2large \n");
	fprintf(file,"Ti Hay-Wadt \n");
	fprintf(file,"Ti LACVP \n");
	fprintf(file,"Ti LANL2DZ \n");
	fprintf(file,"Ti rimp2-SVP \n");
	fprintf(file,"Ti SBKJC \n");
	fprintf(file,"Ti Stuttgart \n");
	fprintf(file,"Atom  V\n");
	fprintf(file,"15\n");
	fprintf(file,"V STO-3G \n");
	fprintf(file,"V STO-6G \n");
	fprintf(file,"V 3-21G \n");
	fprintf(file,"V 6-31G \n");
	fprintf(file,"V 6-31G** \n");
	fprintf(file,"V Ahlrichs \n");
	fprintf(file,"V CRENBL \n");
	fprintf(file,"V DGauss \n");
	fprintf(file,"V G3MP2large \n");
	fprintf(file,"V Hay-Wadt \n");
	fprintf(file,"V LACVP \n");
	fprintf(file,"V LANL2DZ \n");
	fprintf(file,"V rimp2-SVP \n");
	fprintf(file,"V SBKJC \n");
	fprintf(file,"V Stuttgart \n");
	fprintf(file,"Atom  Cr\n");
	fprintf(file,"15\n");
	fprintf(file,"Cr STO-3G \n");
	fprintf(file,"Cr STO-6G \n");
	fprintf(file,"Cr 3-21G \n");
	fprintf(file,"Cr 6-31G \n");
	fprintf(file,"Cr 6-31G** \n");
	fprintf(file,"Cr Ahlrichs \n");
	fprintf(file,"Cr CRENBL \n");
	fprintf(file,"Cr DGauss \n");
	fprintf(file,"Cr G3MP2large \n");
	fprintf(file,"Cr Hay-Wadt \n");
	fprintf(file,"Cr LACVP \n");
	fprintf(file,"Cr LANL2DZ \n");
	fprintf(file,"Cr rimp2-SVP \n");
	fprintf(file,"Cr SBKJC \n");
	fprintf(file,"Cr Stuttgart \n");
	fprintf(file,"Atom  Mn\n");
	fprintf(file,"15\n");
	fprintf(file,"Mn STO-3G \n");
	fprintf(file,"Mn STO-6G \n");
	fprintf(file,"Mn 3-21G \n");
	fprintf(file,"Mn 6-31G \n");
	fprintf(file,"Mn 6-31G** \n");
	fprintf(file,"Mn Ahlrichs \n");
	fprintf(file,"Mn CRENBL \n");
	fprintf(file,"Mn DGauss \n");
	fprintf(file,"Mn G3MP2large \n");
	fprintf(file,"Mn Hay-Wadt \n");
	fprintf(file,"Mn LACVP \n");
	fprintf(file,"Mn LANL2DZ \n");
	fprintf(file,"Mn rimp2-SVP \n");
	fprintf(file,"Mn SBKJC \n");
	fprintf(file,"Mn Stuttgart \n");
	fprintf(file,"Atom  Fe\n");
	fprintf(file,"15\n");
	fprintf(file,"Fe STO-3G \n");
	fprintf(file,"Fe STO-6G \n");
	fprintf(file,"Fe 3-21G \n");
	fprintf(file,"Fe 6-31G \n");
	fprintf(file,"Fe 6-31G** \n");
	fprintf(file,"Fe Ahlrichs \n");
	fprintf(file,"Fe CRENBL \n");
	fprintf(file,"Fe DGauss \n");
	fprintf(file,"Fe G3MP2large \n");
	fprintf(file,"Fe Hay-Wadt \n");
	fprintf(file,"Fe LACVP \n");
	fprintf(file,"Fe LANL2DZ \n");
	fprintf(file,"Fe rimp2-SVP \n");
	fprintf(file,"Fe SBKJC \n");
	fprintf(file,"Fe Stuttgart \n");
	fprintf(file,"Atom  Co\n");
	fprintf(file,"15\n");
	fprintf(file,"Co STO-3G \n");
	fprintf(file,"Co STO-6G \n");
	fprintf(file,"Co 3-21G \n");
	fprintf(file,"Co 6-31G \n");
	fprintf(file,"Co 6-31G** \n");
	fprintf(file,"Co Ahlrichs \n");
	fprintf(file,"Co CRENBL \n");
	fprintf(file,"Co DGauss \n");
	fprintf(file,"Co G3MP2large \n");
	fprintf(file,"Co Hay-Wadt \n");
	fprintf(file,"Co LACVP \n");
	fprintf(file,"Co LANL2DZ \n");
	fprintf(file,"Co rimp2-SVP \n");
	fprintf(file,"Co SBKJC \n");
	fprintf(file,"Co Stuttgart \n");
	fprintf(file,"Atom  Ni\n");
	fprintf(file,"15\n");
	fprintf(file,"Ni STO-3G \n");
	fprintf(file,"Ni STO-6G \n");
	fprintf(file,"Ni 3-21G \n");
	fprintf(file,"Ni 6-31G \n");
	fprintf(file,"Ni 6-31G** \n");
	fprintf(file,"Ni Ahlrichs \n");
	fprintf(file,"Ni CRENBL \n");
	fprintf(file,"Ni DGauss \n");
	fprintf(file,"Ni G3MP2large \n");
	fprintf(file,"Ni Hay-Wadt \n");
	fprintf(file,"Ni LACVP \n");
	fprintf(file,"Ni LANL2DZ \n");
	fprintf(file,"Ni rimp2-SVP \n");
	fprintf(file,"Ni SBKJC \n");
	fprintf(file,"Ni Stuttgart \n");
	fprintf(file,"Atom  Cu\n");
	fprintf(file,"15\n");
	fprintf(file,"Cu STO-3G \n");
	fprintf(file,"Cu STO-6G \n");
	fprintf(file,"Cu 3-21G \n");
	fprintf(file,"Cu 6-31G \n");
	fprintf(file,"Cu 6-31G** \n");
	fprintf(file,"Cu Ahlrichs \n");
	fprintf(file,"Cu CRENBL \n");
	fprintf(file,"Cu DGauss \n");
	fprintf(file,"Cu G3MP2large \n");
	fprintf(file,"Cu Hay-Wadt \n");
	fprintf(file,"Cu LACVP \n");
	fprintf(file,"Cu LANL2DZ \n");
	fprintf(file,"Cu rimp2-SVP \n");
	fprintf(file,"Cu SBKJC \n");
	fprintf(file,"Cu Stuttgart \n");
	fprintf(file,"Atom  Zn\n");
	fprintf(file,"15\n");
	fprintf(file,"Zn STO-3G \n");
	fprintf(file,"Zn STO-6G \n");
	fprintf(file,"Zn 3-21G \n");
	fprintf(file,"Zn 6-31G \n");
	fprintf(file,"Zn 6-31G** \n");
	fprintf(file,"Zn Ahlrichs \n");
	fprintf(file,"Zn CRENBL \n");
	fprintf(file,"Zn DGauss \n");
	fprintf(file,"Zn G3MP2large \n");
	fprintf(file,"Zn Hay-Wadt \n");
	fprintf(file,"Zn LACVP \n");
	fprintf(file,"Zn LANL2DZ \n");
	fprintf(file,"Zn rimp2-SVP \n");
	fprintf(file,"Zn SBKJC \n");
	fprintf(file,"Zn Stuttgart \n");
	fprintf(file,"Atom  Ga\n");
	fprintf(file,"14\n");
	fprintf(file,"Ga STO-3G \n");
	fprintf(file,"Ga STO-6G \n");
	fprintf(file,"Ga 3-21G \n");
	fprintf(file,"Ga 6-31G \n");
	fprintf(file,"Ga 6-31G** \n");
	fprintf(file,"Ga Ahlrichs \n");
	fprintf(file,"Ga CRENBL \n");
	fprintf(file,"Ga DGauss \n");
	fprintf(file,"Ga G3MP2large \n");
	fprintf(file,"Ga Hay-Wadt \n");
	fprintf(file,"Ga LACVP \n");
	fprintf(file,"Ga LANL2DZ \n");
	fprintf(file,"Ga rimp2-SVP \n");
	fprintf(file,"Ga SBKJC \n");
	fprintf(file,"Atom  Ge\n");
	fprintf(file,"14\n");
	fprintf(file,"Ge STO-3G \n");
	fprintf(file,"Ge STO-6G \n");
	fprintf(file,"Ge 3-21G \n");
	fprintf(file,"Ge 6-31G \n");
	fprintf(file,"Ge 6-31G** \n");
	fprintf(file,"Ge Ahlrichs \n");
	fprintf(file,"Ge CRENBL \n");
	fprintf(file,"Ge DGauss \n");
	fprintf(file,"Ge G3MP2large \n");
	fprintf(file,"Ge Hay-Wadt \n");
	fprintf(file,"Ge LACVP \n");
	fprintf(file,"Ge LANL2DZ \n");
	fprintf(file,"Ge rimp2-SVP \n");
	fprintf(file,"Ge SBKJC \n");
	fprintf(file,"Atom  As\n");
	fprintf(file,"14\n");
	fprintf(file,"As STO-3G \n");
	fprintf(file,"As STO-6G \n");
	fprintf(file,"As 3-21G \n");
	fprintf(file,"As 6-31G \n");
	fprintf(file,"As 6-31G** \n");
	fprintf(file,"As Ahlrichs \n");
	fprintf(file,"As CRENBL \n");
	fprintf(file,"As DGauss \n");
	fprintf(file,"As G3MP2large \n");
	fprintf(file,"As Hay-Wadt \n");
	fprintf(file,"As LACVP \n");
	fprintf(file,"As LANL2DZ \n");
	fprintf(file,"As rimp2-SVP \n");
	fprintf(file,"As SBKJC \n");
	fprintf(file,"Atom  Se\n");
	fprintf(file,"14\n");
	fprintf(file,"Se STO-3G \n");
	fprintf(file,"Se STO-6G \n");
	fprintf(file,"Se 3-21G \n");
	fprintf(file,"Se 6-31G \n");
	fprintf(file,"Se 6-31G** \n");
	fprintf(file,"Se Ahlrichs \n");
	fprintf(file,"Se CRENBL \n");
	fprintf(file,"Se DGauss \n");
	fprintf(file,"Se G3MP2large \n");
	fprintf(file,"Se Hay-Wadt \n");
	fprintf(file,"Se LACVP \n");
	fprintf(file,"Se LANL2DZ \n");
	fprintf(file,"Se rimp2-SVP \n");
	fprintf(file,"Se SBKJC \n");
	fprintf(file,"Atom  Br\n");
	fprintf(file,"14\n");
	fprintf(file,"Br STO-3G \n");
	fprintf(file,"Br STO-6G \n");
	fprintf(file,"Br 3-21G \n");
	fprintf(file,"Br 6-31G \n");
	fprintf(file,"Br 6-31G** \n");
	fprintf(file,"Br Ahlrichs \n");
	fprintf(file,"Br CRENBL \n");
	fprintf(file,"Br DGauss \n");
	fprintf(file,"Br G3MP2large \n");
	fprintf(file,"Br Hay-Wadt \n");
	fprintf(file,"Br LACVP \n");
	fprintf(file,"Br LANL2DZ \n");
	fprintf(file,"Br rimp2-SVP \n");
	fprintf(file,"Br SBKJC \n");
	fprintf(file,"Atom  Kr\n");
	fprintf(file,"14\n");
	fprintf(file,"Kr STO-3G \n");
	fprintf(file,"Kr STO-6G \n");
	fprintf(file,"Kr 3-21G \n");
	fprintf(file,"Kr 6-31G \n");
	fprintf(file,"Kr 6-31G** \n");
	fprintf(file,"Kr Ahlrichs \n");
	fprintf(file,"Kr CRENBL \n");
	fprintf(file,"Kr DGauss \n");
	fprintf(file,"Kr G3MP2large \n");
	fprintf(file,"Kr Hay-Wadt \n");
	fprintf(file,"Kr LACVP \n");
	fprintf(file,"Kr LANL2DZ \n");
	fprintf(file,"Kr rimp2-SVP \n");
	fprintf(file,"Kr SBKJC \n");
	fprintf(file,"Atom  Rb\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Sr\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Y\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Zr\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Nb\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Mo\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Tc\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Ru\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Rh\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Pd\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Ag\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Cd\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  In\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Sn\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Sb\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Te\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  I\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Xe\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Cs\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Ba\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  La\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Hf\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Ta\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  W\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Re\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Os\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Ir\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Pt\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Au\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Hg\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Tl\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Pb\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Bi\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Po\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  At\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Rn\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Fr\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Ra\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Ac\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Ce\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Pr\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Nd\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Pm\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Sm\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Eu\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Gd\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Tb\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Dy\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Ho\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Er\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Tm\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Yb\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Lu\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  X\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Th\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Pa\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  U\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Np\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Pu\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Am\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Cm\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Bk\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Cf\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Es\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Fm\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Md\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  No\n");
	fprintf(file,"0\n");
	fprintf(file,"Atom  Lr\n");
	fprintf(file,"0\n");
	fclose(file);
	return TRUE;
}
