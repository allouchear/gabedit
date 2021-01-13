/* OrbitalsDalton.c */
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
#include "GlobalOrb.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "GeomDraw.h"
#include "GLArea.h"
#include "UtilsOrb.h"
#include "Basis.h"
#include "GeomOrbXYZ.h"
#include "AtomicOrbitals.h"
#include "StatusOrb.h"
#include "Basis.h"
#include "Orbitals.h"

#define MAXSYM 8

/*****************************************/
typedef struct _OneBasisSymmetry
{
 gint numsym;
 gchar *sym;

 gint nOrbs;
 gint* numbOrbs;
 gint** numOrbs;
 gint** signe;
}OneBasisSymmetry;
/*****************************************/
static void DefineType()
{
 gint i,j;
 gint OK;
 Ntype =1;
 GeomOrb[0].NumType=0;
 for(i=1;i<nCenters;i++)
 {
   OK =1;
   for(j=0;j<i;j++)
   {
   	if(!strcmp(GeomOrb[i].Symb,GeomOrb[j].Symb)) 
   	{
                GeomOrb[i].NumType= GeomOrb[j].NumType;
        	OK =0;
		break;
   	}
   }
   if(OK)
   {
        GeomOrb[i].NumType= Ntype;
	Ntype++;
   }
 }
 /*
 Debug("Types = ");
 for(i=0;i<nCenters;i++)
	 Debug("%d ", GeomOrb[i].NumType);
 Debug("\n");
 */
}
/********************************************************************************/
static gboolean read_dalton_geom(gchar *FileName)
{
 	gchar t[BSIZE];
 	gchar *tmp = NULL;
 	FILE *file;
 	guint taille=BSIZE;
 	guint idummy;
 	guint i;
 	gint j=0;
 	gint l;
	gint kk;
	gchar dum[100];
	gchar symb[100];
	gchar cor[100];
	gint natoms = 0;

  
 	file = FOpen(FileName, "rb");
 	if(file ==NULL)
 	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  		return FALSE;
 	}

	free_data_all();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Dalton");
	set_status_label_info(_("Geometry"),_("Reading"));
	natoms = 0;
 	while(!feof(file))
	{
		if(!fgets(t,taille,file))break;
		if (strstr(t,"Cartesian Coordinates"))
		{
 			while(!feof(file))
			{
				if(!fgets(t,taille,file))break;
				if (strstr(t,"Total number of coordinates:"))
				{
					gchar* p = strstr(t,":")+1;
					natoms = atoi(p);
					natoms /= 3;
					break;
				}
			}
	  		break;
	  	}
        }
 	if(natoms<1)
	{
  		Message(_("Sorry\nI can not read geometry in this file"),_("Error"),TRUE);
 		fclose(file);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Geometry"),_("Nothing"));
		return FALSE;
    	}
	printf("natoms = %d\n",natoms);
	if(natoms>0)
	{
	       	GeomOrb=g_malloc(natoms*sizeof(TypeGeomOrb));
	     	fgets(t,taille,file);
	}
	for(j=0;j<natoms;j++)
	{
		if(!fgets(t,taille,file))break;
		kk = sscanf(t,"%d %s %s %s %s",&idummy, symb,dum, cor, dum);
		if(kk==5) sscanf(t,"%d %s %s %s %s",&idummy, symb,dum,dum, cor);
		for(i=0;i<(gint)strlen(symb);i++) if(isdigit(symb[i])) symb[i] = ' ';
		delete_all_spaces(symb);
		symb[0]=toupper(symb[0]);
 		l=strlen(symb);
          	if (l==2) symb[1]=tolower(symb[1]);
    		GeomOrb[j].Symb=g_strdup(symb);
		GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb[j].partialCharge = 0.0;
		GeomOrb[j].variable = TRUE;
		GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
		GeomOrb[j].C[0]=atof(cor);

		if(!fgets(t,taille,file))break;
		kk = sscanf(t,"%d %s %s",&idummy, dum,cor);
		GeomOrb[j].C[1]=atof((cor));

		if(!fgets(t,taille,file))break;
		kk = sscanf(t,"%d %s %s",&idummy, dum,cor);
		GeomOrb[j].C[2]=atof(cor);

		if(!fgets(t,taille,file))break; /* backspace */
		printf("%s %lf %lf %lf\n", GeomOrb[j].Symb, GeomOrb[j].C[0], GeomOrb[j].C[1], GeomOrb[j].C[2]);
	}
	printf("j= %d\n",j);
 	fclose(file);
	if(j!=natoms)
	{
  		g_free(GeomOrb);
		GeomOrb = 0;
		nCenters = 0;
	}
 	else
	{
 		nCenters = natoms;
  		DefineType();
	}
	buildBondsOrb();
	RebuildGeomD = TRUE;
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	return TRUE;
}
/********************************************************************************/
static gchar** read_basis_from_a_dalton_output_file(gchar *FileName, gint* nRows)
{
 	gchar **strbasis;
 	gchar *t;
 	FILE *file;
 	gint nrows=0;
 	gint i=0;
	gboolean OK;
	gboolean begin;
	gboolean backspace;
	
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return NULL;
 	}

 	file = FOpen(FileName, "rb");
 	if(file ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
  		return NULL;
 	}

 	t=g_malloc(BSIZE*sizeof(gchar));
	OK = FALSE;
 	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file)) break;
		if(strstr(t,"Orbital exponents and contraction coefficients"))
		{
			OK = TRUE;
			break;
		}
        }
	if(!OK)
	{
		g_free(t);
  		Message(_("Sorry I can read basis from this file\n"),_("Error"),TRUE);
  		return NULL;
	}
	fgets(t,BSIZE,file); /* -------- */

	strbasis=g_malloc(sizeof(gchar*));
	begin = TRUE;
	backspace = FALSE;
 	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file)) break;
		if(strstr(t,"Contracted Orbitals")) break;
		OK = this_is_a_backspace(t);
		if(OK && backspace ) break;
		if(begin && OK) continue;
		begin = FALSE;
		backspace = OK;
		nrows++;
		strbasis = g_realloc(strbasis,nrows*sizeof(gchar*));
		strbasis[nrows-1] = g_strdup(t);
	}
	if(nrows == 0)
	{
		g_free(t);
		g_free(strbasis);
  		Message(_("Sorry I can read basis from this file\n"),_("Error"),TRUE);
  		return NULL;
	}
	
	/*
	Debug("End of read \n");
	Debug("Atomic basis nrows = %d \n",nrows);
	for(i=0;i<nrows;i++)
	{
		Debug("%s",strbasis[i]);
	}
	*/

	/*
	for(i=0;i<nrows;i++) g_free(strbasis[i]);
	g_free(strbasis);
	*/
 	fclose(file);
 	g_free(t);
	*nRows = nrows;
	return strbasis;
}
/********************************************************************************/
static OneBasisSymmetry* read_symmetry_basis_from_a_dalton_output_file(gchar *FileName, gint* nSym)
{
 	gchar *t;
 	FILE *file;
 	gint nsym=0;
 	gint i=0;
 	gint j=0;
 	gint k=0;
	gint nO[MAXSYM];
	OneBasisSymmetry* basisSymmetry = NULL;
	gchar* p = NULL;
	gchar dum[100];
	
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return NULL;
 	}

 	file = FOpen(FileName, "rb");
 	if(file ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
  		return NULL;
 	}

 	t=g_malloc(BSIZE*sizeof(gchar));
	nsym = 0;
 	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file)) break;
		if(strstr(t,"Symmetry Orbitals"))
		{
 			while(!feof(file))
			{
				if(!fgets(t,BSIZE,file)) break;
				if(strstr(t,"Number of orbitals in each symmetry:"))
				{
					gchar* p = strstr(t,":")+1;
					nsym = sscanf(p, "%d %d %d %d %d %d %d %d",&nO[0], &nO[1], &nO[2], &nO[3], &nO[4], &nO[5], &nO[6], &nO[7]);
					break;
				}
			}
			break;
		}
        }
	if(nsym==0)
	{
		g_free(t);
  		Message(_("Sorry I can read basis from this file\n"),_("Error"),TRUE);
  		return NULL;
	}
	printf("nsym = %d\n", nsym);
	basisSymmetry = g_malloc(nsym*sizeof(OneBasisSymmetry));
	for(i=0;i<nsym;i++)
	{
		if(!fgets(t,BSIZE,file)) break; /* backspace */
		if(!fgets(t,BSIZE,file)) break; /* backspace */
		if(!fgets(t,BSIZE,file)) break; /* Symmetry */
		if(!strstr(t,"ymmetry")) break;
		if(strstr(t,"No orbitals"))
		{
			basisSymmetry[i].numsym = i;
			basisSymmetry[i].nOrbs = 0;
			basisSymmetry[i].numbOrbs = NULL;
			basisSymmetry[i].numOrbs = NULL;
			basisSymmetry[i].signe = NULL;
			p  = strstr(t,"ymmetry")+8;
			if(p) basisSymmetry[i].sym = g_strdup(p);
			else basisSymmetry[i].sym = g_strdup("UNKNOWN");
			str_delete_n(basisSymmetry[i].sym);
			printf("sym = %s\n", basisSymmetry[i].sym);
			continue;
		}
		basisSymmetry[i].numsym = i;
		basisSymmetry[i].nOrbs = nO[i];
		basisSymmetry[i].numbOrbs = NULL;
		basisSymmetry[i].numOrbs = NULL;
		basisSymmetry[i].signe = NULL;
		p  = strstr(t,"ymmetry")+8;
		if(p) basisSymmetry[i].sym = g_strdup(p);
		else basisSymmetry[i].sym = g_strdup("UNKNOWN");
		str_delete_n(basisSymmetry[i].sym);
		printf("sym = %s\n", basisSymmetry[i].sym);
		if(!fgets(t,BSIZE,file)) break; /* backspace */
		if(nO[i]>0)
		{
			basisSymmetry[i].numbOrbs = g_malloc(nO[i]*sizeof(gint));
			basisSymmetry[i].numOrbs = g_malloc(nO[i]*sizeof(gint*));
			basisSymmetry[i].signe = g_malloc(nO[i]*sizeof(gint*));
		}
				
		for(j=0;j<nO[i];j++)
		{
			gchar* tmp;
			gchar* t2;
			gint kk;
			if(!fgets(t,BSIZE,file)) break; /* Symmetry */
			sscanf(t,"%s %s %s",dum, dum, dum);
			p = strstr(t,dum)+strlen(dum)+1;
			tmp = g_strdup(p);
			delete_first_spaces(tmp);
			delete_last_spaces(tmp);
			str_delete_n(tmp);
			basisSymmetry[i].numbOrbs[j] = 1;	
			for(k = 0;k<strlen(tmp);k++)
			{
				if(tmp[k]=='-') basisSymmetry[i].numbOrbs[j]++;
				if(tmp[k]=='+') basisSymmetry[i].numbOrbs[j]++;
			}
			if(basisSymmetry[i].numbOrbs[j]<1) continue;
			 basisSymmetry[i].numOrbs[j] = g_malloc(basisSymmetry[i].numbOrbs[j]*sizeof(gint));
			 basisSymmetry[i].signe[j] = g_malloc(basisSymmetry[i].numbOrbs[j]*sizeof(gint));

			kk = 0;
			basisSymmetry[i].signe[j][kk] = 1.0;
			sscanf(tmp,"%d",&basisSymmetry[i].numOrbs[j][kk]);
			t2 = tmp;
			for(k = 0;k<strlen(tmp);k++)
			{
				t2++;
				if(tmp[k]=='-')
				{
					kk++;
					t2++;
					basisSymmetry[i].signe[j][kk] = -1.0;
					sscanf(t2,"%d",&basisSymmetry[i].numOrbs[j][kk]);
					t2--;
				}
				if(tmp[k]=='+')
				{
					kk++;
					t2++;
					basisSymmetry[i].signe[j][kk] = +1.0;
					sscanf(t2,"%d",&basisSymmetry[i].numOrbs[j][kk]);
					t2--;
				}
			}
			for(kk=0;kk<basisSymmetry[i].numbOrbs[j];kk++)
			{
				printf("%d %d ", basisSymmetry[i].signe[j][kk], basisSymmetry[i].numOrbs[j][kk]);
			}
			printf("\n");

		}
	}
	printf("t=%s\n",t);
 	fclose(file);
 	g_free(t);
	*nSym = nsym;
	return basisSymmetry;
}
/********************************************************************************/
static gboolean good_sym_orb(gchar* str)
{
	static gchar* orbTypes[] = {"1s","2p","3d", "4f", "5g", "6h", "7i", "8j", "9k","10l"};
	static gint nTypes = G_N_ELEMENTS (orbTypes);
	gint i;
	for(i=0; i<nTypes; i++)
		if(strstr(str,orbTypes[i]))return TRUE;
	return FALSE;
}
/********************************************************************************/
static gboolean set_basis_from_a_strbasis(gchar** strbasis, gint nrows)
{
	CGTF *tmpBasis;
	gint row = 0;
	static gchar dum[100];
	static gchar numC[100];
	static gchar symOrb[100];
	static gchar numOrb[30];
	static gchar expStr[30];
	static gchar symb[30];
	gint nBlock = 0;
	gint* nExp = NULL;
	gint* nOrbs = NULL;
	gint* nCents = NULL;
	gint nE = 0;
	gint nO = 0;
	gint i;
	gint nC = 0;
	static gchar symbOld[30];
	static gchar numCOld[100];

	if(nrows<1 || strbasis == NULL) return FALSE;
	if(this_is_a_backspace(strbasis[0])) return FALSE;

	nE = 0;
	nO = 0;
	nC = 0;
	for(row = 0; row<nrows; row++)
	{
		if(this_is_a_backspace(strbasis[row]))
		{
			nBlock++;
			nExp = g_realloc(nExp,nBlock*sizeof(gint));
			nOrbs = g_realloc(nOrbs,nBlock*sizeof(gint));
			nCents = g_realloc(nCents,nBlock*sizeof(gint));
			nExp[nBlock-1] = nE;
			nOrbs[nBlock-1] = nO;
			nCents[nBlock-1] = nC;
			nE = 0;
			nO = 0;
			continue;
		}
		if(nE==0)
		{
			gchar* t;
			nE++;
			sscanf(strbasis[row],"%s %s %s %s %s", symb, numC, symOrb, numOrb, expStr);
			if(!good_sym_orb(symOrb))
			{
				sprintf(numC,"#0");
				sscanf(strbasis[row],"%s %s %s %s", symb, symOrb, numOrb, expStr);
				if(!good_sym_orb(symOrb)) return FALSE;
			}
			t = strstr(strbasis[row], expStr) + strlen( expStr)+1;
			nO += sscanf(t,"%s %s %s %s",dum, dum, dum, dum);
			if(row ==0) nC = 0;
			else
			{
				printf("symb = %s symbOld = %s\n",symb, symbOld);
				printf("numC = %s numCOld = %s\n",numC, numCOld);
				if(strcmp(symb, symbOld)!=0) nC++;
				else
				{
					if(strcmp(numCOld, numC)!=0) nC++;
				}
			}
			sprintf(symbOld,"%s",symb);
			sprintf(numCOld,"%s",numC);
		}
		else
		{
			if(strstr(strbasis[row],"cont.")) nE++;
			else 
			{
				sscanf(strbasis[row],"%s", dum);
				if(!strstr(dum,".")) nE++;
				else if(nE==1)
				{

					printf("------->%s nOAvant = %d\n",strbasis[row], nO);
					nO += sscanf(strbasis[row],"%s %s %s %s", dum, dum, dum, dum);
					printf("------->%s nOApres = %d\n",strbasis[row], nO);
				}
			}
		}
	}
	printf("nBlock = %d\n", nBlock);
	for(i = 0; i< nBlock; i++)
	{
		printf("nE = %d nO = %d numCenter = %d\n", nExp[i], nOrbs[i], nCents[i]);
	}
	printf("\n");

	for(row = 0; row<nrows; row++)
	{
		if(this_is_a_backspace(strbasis[row])) continue;
		sscanf(strbasis[row],"%s %s %s %s %s", dum, numC, symOrb, numOrb, expStr);
		if(!good_sym_orb(symOrb))
		{
			sprintf(numC,"#0");
			sscanf(strbasis[row],"%s %s %s %s", dum, symOrb, numOrb, expStr);
			if(!good_sym_orb(symOrb)) return FALSE;
		}
	}

	
	return TRUE;
}
/********************************************************************************/
void read_dalton_orbitals(gchar* FileName)
{
	gint nRows = 0;
	gint nSym = 0;
	gchar** strbasis = NULL;

	set_status_label_info(_("File name"),FileName);
	set_status_label_info(_("File type"),"Dalton");
	set_status_label_info(_("Mol. Orb."),_("Nothing"));

	read_dalton_geom(FileName);
	strbasis = read_basis_from_a_dalton_output_file(FileName, &nRows);
	set_basis_from_a_strbasis(strbasis, nRows);
	/* read_symmetry_basis_from_a_dalton_output_file(FileName, &nSym);*/

} 
