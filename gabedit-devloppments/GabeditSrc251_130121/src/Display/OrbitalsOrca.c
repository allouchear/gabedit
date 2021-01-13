/* OrbitalsOrca.c */
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
#include "../Common/Global.h"
#include "GlobalOrb.h"
#include "../Utils/Constants.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/Zlm.h"
#include "GeomDraw.h"
#include "GLArea.h"
#include "GeomOrbXYZ.h"
#include "UtilsOrb.h"
#include "StatusOrb.h"
#include "Basis.h"
#include "Orbitals.h"

/********************************************************************************/
void read_orca_orbitals_using_orca_2mkl(gchar* fileName)
{
	gchar* fileNameBas = NULL;
	gchar* convertOrcaMolden = NULL;
	gchar* moldenFile = NULL;
	FILE* file= NULL;
	gchar* t = NULL;
	gint l = 0;
	gint i = 0;

	fileNameBas = g_strdup(fileName);
	l = strlen(fileNameBas)-1;
	for(i=l;i>=0;i--)
	{
		if(fileNameBas[i]=='.')
		{
			fileNameBas[i]='\0';
			break;
		}
	}
	if(!fileNameBas || strlen(fileNameBas)<1)
	{
		gchar buffer[BSIZE];
		sprintf(buffer, _("Sorry, I can not read orbitals from the orca file\n"));
  		Message(buffer,_("Error"),TRUE);
		return;
	}
#ifdef G_OS_WIN32
	convertOrcaMolden = g_strdup_printf("%s%sorca_2mkl %s -molden",orcaDirectory,G_DIR_SEPARATOR_S,fileNameBas);
#else
	convertOrcaMolden = g_strdup_printf("orca_2mkl %s -molden",fileNameBas);
#endif

	t = get_name_file(fileName);
	set_status_label_info(_("File name"),t);
	g_free(t);
	set_status_label_info(_("File type"),"Orca");
	set_status_label_info(_("Mol. Orb."),"Reading");

	{int ierr = system(convertOrcaMolden);}
	g_free(convertOrcaMolden);

	moldenFile = g_strdup_printf("%s.molden.input",fileNameBas);
 	file = FOpen(moldenFile, "rb");
	if(file == NULL)
	{
		gchar buffer[BSIZE];
		sprintf(buffer, _("Sorry, I cannot create the molden file using orca_2mkl"));
  		Message(buffer,_("Error"),TRUE);
		return;
	}
	fclose(file);
	read_molden_orbitals(moldenFile);
	g_free(moldenFile);
} 
/**********************************************/
static void DefineOrcaSphericalBasis()
{
	gint i,j,k;
	gint c;
	gint kl;
	gint L,M;
	CGTF *temp;
	Zlm Stemp;
	gint N,Nc,n;
	gint inc;
	gint  klbeg;
	gint  klend;
	gint  klinc;


	NOrb = 0;
	for(i=0;i<nCenters;i++)
	{
	 	for(j=0;j<Type[GeomOrb[i].NumType].Norb;j++)
	 	{
			L=Type[GeomOrb[i].NumType].Ao[j].L;
			NOrb += 2*L+1;
	 	}
	}

	temp  = g_malloc(NOrb*sizeof(CGTF));

	k=-1;
	for(i=0;i<nCenters;i++)
	for(j=0;j<Type[GeomOrb[i].NumType].Norb;j++)
	{
	 	L =Type[GeomOrb[i].NumType].Ao[j].L;
		/* Debug("L=%d \n",L);*/

		/*Debug("L =%d \n",L);*/
		if(L==1)
		{
                	klbeg = 0;
                	klend = L;
                	klinc = +1;
		}
		else
		{
                	klbeg = 0;
                	klend = L;
                	klinc = +1;
		}
		for(kl = klbeg;(klbeg == 0 && kl<=klend) || (klbeg == L && kl>=klend);kl +=klinc)
		{
			if(kl!=0) inc = 2*kl;	
			else inc = 1;
			for(M=kl;M>=-kl;M -=inc)
    			{
				/*Debug("L =%d kl=%d M=%d \n",L,kl,M);*/
	 			k++;
	 	   		Stemp =  getZlm(L,M);

	 			temp[k].numberOfFunctions=Stemp.numberOfCoefficients*Type[GeomOrb[i].NumType].Ao[j].N;
		    		temp[k].NumCenter=i;
				/* Debug("M=%d N=%d\n",M,temp[k].N);*/
	 			temp[k].Gtf =g_malloc(temp[k].numberOfFunctions*sizeof(GTF));
          			Nc=-1;
	 			for(N=0;N<Type[GeomOrb[i].NumType].Ao[j].N;N++)
	 			for(n=0;n<Stemp.numberOfCoefficients;n++)
	 			{
	 			   	Nc++;
	   				temp[k].Gtf[Nc].Ex   = Type[GeomOrb[i].NumType].Ao[j].Ex[N];
	   				temp[k].Gtf[Nc].Coef = Type[GeomOrb[i].NumType].Ao[j].Coef[N]*Stemp.lxyz[n].Coef;
	   				for(c=0;c<3;c++)
	   				{
	   					temp[k].Gtf[Nc].C[c] = GeomOrb[i].C[c];
	   					temp[k].Gtf[Nc].l[c] = Stemp.lxyz[n].l[c];
	   				}
	 			}
				if(L==0) break;
	      		}
			if(L==0) break;
	      }
	}
	 for(i=0;i<NAOrb;i++) g_free(AOrb[i].Gtf);
	g_free(AOrb);
	NAOrb = NOrb;
	AOrb = temp;
	if(SAOrb) g_free(SAOrb);
	SAOrb = NULL;
	DefineAtomicNumOrb();
}


/**********************************************/
static gboolean ReadOneOrcaBasis(gint i,gint j,char *t,gint *nsym)
{
	gint k;
	gint n;
	gchar sym[10];
	gint l=1;
	gint idum;

	/*Debug("i = %d j= %d\n",i,j);*/
	/*Debug("t One %s\n",t);*/
        /*sscanf(t,"%s %d",sym,&Type[i].Ao[j].N);*/
	sscanf(t,"%s %d",sym,&n);
	/*	Debug("n = %d\n",n);*/
	Type[i].Ao[j].N=n;
	/*Debug("N = %d\n",Type[i].Ao[j].N);*/
	Type[i].Ao[j].Ex=g_malloc(Type[i].Ao[j].N*sizeof(gdouble));
	Type[i].Ao[j].Coef=g_malloc(Type[i].Ao[j].N*sizeof(gdouble));
	/*Debug("avant sym ==\n");*/
	if(strlen(sym)==2)
	{
	 	l=2;
     		Type[i].Ao=g_realloc(Type[i].Ao,(j+2)*sizeof(AO));
        	Type[i].Ao[j+1].N = Type[i].Ao[j].N;
		Type[i].Ao[j+1].Ex=g_malloc(Type[i].Ao[j].N*sizeof(gdouble));
		Type[i].Ao[j+1].Coef=g_malloc(Type[i].Ao[j].N*sizeof(gdouble));
	}
	*nsym = l;
	/*Debug("nsym = %d\n",l);*/

		
	for(k=0;k<Type[i].Ao[j].N;k++)
	{
    		{ char* e = fgets(t,BSIZE,forb);}
		for(n=0;n<(gint)strlen(t);n++)
			if(t[n]=='D') t[n] = 'e';
		/*Debug("t de One = %s\n",t);*/
		   
		if(l==1) sscanf(t,"%d %lf %lf ",&idum, &Type[i].Ao[j].Ex[k],&Type[i].Ao[j].Coef[k]);
		else
		{
			sscanf(t,"%d %lf %lf %lf ",&idum, &Type[i].Ao[j].Ex[k],&Type[i].Ao[j].Coef[k],&Type[i].Ao[j+1].Coef[k]);
			Type[i].Ao[j+1].Ex[k] = Type[i].Ao[j].Ex[k];
		}
	}
	/*Debug("end k\n");*/

    switch(sym[0])
    {
        case 's' : 
        case 'S' : Type[i].Ao[j].L=0;break;
        case 'p' :
        case 'P' : Type[i].Ao[j].L=1;break; 
        case 'd' : 
        case 'D' : Type[i].Ao[j].L=2;break;
        case 'f' : 
        case 'F' : Type[i].Ao[j].L=3;break;         
	case 'g' : 
        case 'G' : Type[i].Ao[j].L=4;break;
	case 'h' : 
        case 'H' : Type[i].Ao[j].L=5;break;
	case 'i' : 
        case 'I' : Type[i].Ao[j].L=6;break;
	case 'j' : 
        case 'J' : Type[i].Ao[j].L=7;break;
	case 'k' : 
        case 'K' : Type[i].Ao[j].L=8;break;
	case 'l' : 
        case 'L' : Type[i].Ao[j].L=9;break;
        default :	
			g_free(Type[i].Ao[j].Ex);
			g_free(Type[i].Ao[j].Coef);
         		return FALSE;
    }
    /*
	if(l == 2)
		Debug(" sym[1] =%c \n",sym[1]);
    */
    if(l == 2)
    switch(sym[1])
    {
        case 's' : 
        case 'S' : Type[i].Ao[j+1].L=0;break;
        case 'p' :
        case 'P' : Type[i].Ao[j+1].L=1;break; 
        case 'd' : 
        case 'D' : Type[i].Ao[j+1].L=2;break;
        case 'f' : 
        case 'F' : Type[i].Ao[j+1].L=3;break;         
	case 'g' : 
        case 'G' : Type[i].Ao[j+1].L=4;break;
	case 'h' : 
        case 'H' : Type[i].Ao[j+1].L=5;break;
	case 'i' : 
        case 'I' : Type[i].Ao[j+1].L=6;break;
	case 'j' : 
        case 'J' : Type[i].Ao[j+1].L=7;break;
	case 'k' : 
        case 'K' : Type[i].Ao[j+1].L=8;break;
	case 'l' : 
        case 'L' : Type[i].Ao[j+1].L=9;break;

        default :
			g_free(Type[i].Ao[j+1].Ex);
			g_free(Type[i].Ao[j+1].Coef);
         		return FALSE;
     }
	/*Debug("end readone basis \n");*/
       return TRUE;
}
/**********************************************/
/*
static void resortAtoms(gint* numAtoms)
{
	TypeGeomOrb* newGeom = NULL;
	gint i;
	if(!numAtoms)return;
	if(nCenters<1)return;
	for(i=0;i<nCenters;i++) if(numAtoms[i] == -1) return;
	newGeom = g_malloc(nCenters*sizeof(TypeGeomOrb));
	for(i=0;i<nCenters;i++) newGeom[i] = GeomOrb[numAtoms[i]];
	for(i=0;i<nCenters;i++) GeomOrb[i] = newGeom[i];
	g_free(newGeom);
}
*/
/**********************************************/
static gboolean DefineOrcaBasisType(gchar *fileName)
{
	gchar sym[50];
	gchar sdum[50];
	gchar t[BSIZE];
	gint i;
	gint j;
	gboolean ok;
	gint nsym;
	/* gint* numAtoms = NULL;*/
	gint nAtoms = 0;
	gchar* title = "BASIS SET IN INPUT FORMAT";
	gint nGroups = 0;

 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No file selected"),_("Error"),TRUE);
    		return FALSE;
 	}

 	forb = FOpen(fileName, "rb");
 	if(forb == NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
  		return FALSE;
 	}
	ok = FALSE;
	nGroups = 0;
	while(!feof(forb))
	{
		if(!fgets(t,BSIZE,forb))break;
		if(strstr(t,title)) 
		{

			/* Debug("t=%s\n",t);*/
			ok = TRUE;
			break;
		}
		if(strstr(t,"groups of distinct atoms"))
		{
			/* Debug("t=%s\n",t);*/
			if(strstr(t,"There are ") && strlen(t)>strlen("There are ")+10)
				sscanf(strstr(t,"There are ")+strlen("There are "),"%d",&nGroups);
			/* Debug("nGroups = %d\n",nGroups);*/
		}
		if(ok && nGroups>0) break;
	}
	/* printf("nGroups = %d\n",nGroups);*/
	if(nGroups<=0) ok = FALSE;
	if(!ok)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry\nI can not read basis from '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
  		return FALSE;
	}

	/* Debug("Ntype = %d\n",Ntype);*/
	/* numAtoms = g_malloc(nCenters*sizeof(gint));*/
	/* for(i=0;i<nCenters;i++) numAtoms[i] = -1;*/
	nAtoms = 0;
	Type = g_malloc(Ntype*sizeof(TYPE));
	for(i=0;i<Ntype;i++)
	{
		Type[i].Ao = NULL;
       		Type[i].Norb=0;
	}
	sprintf(t," ");
	while(!feof(forb) && nAtoms<nGroups)
	{
		if(!fgets(t,BSIZE,forb)) break;
     		if(strstr(t,"# Basis set for element :") && fgets(t,BSIZE,forb) && strstr(t,"NewGTO"))
		{
			/* Debug("%s\n",t);*/
			sscanf(t,"%s %s",sdum,sym);
			/* Debug("sym = %s\n",sym);*/
			i = -1;
			for(j=0;j<nCenters;j++) if(!strcmp(sym,GeomOrb[j].Symb)) {i = j;break;}
			/* if(i>-1 && i<nCenters) numAtoms[nAtoms] = i;*/
			nAtoms++;
			/* Debug("i1 = %d \n",i);*/
			if(i>-1)
			{

				i = GeomOrb[i].NumType;
				/* Debug("numType = %d\n",i);*/
     				Type[i].Symb=g_strdup(sym);
     				Type[i].N=GetNelectrons(sym);
     				j=-1;
				while(!feof(forb))
     				{
    					{ char* e = fgets(t,BSIZE,forb);}
						/* Debug("t = %s\n",t);*/
   					if(this_is_a_backspace(t) || strstr(t,"end")) break;
     					j++;
        				Type[i].Norb=j+1;
					/*
					Debug("debut Alloc %d %d \n",i,j);
					Debug("point %d \n",Type[i].Ao);
					*/
     					if(j == 0) Type[i].Ao=g_malloc(sizeof(AO));
     					else Type[i].Ao=g_realloc(Type[i].Ao,(j+1)*sizeof(AO));

					/* Debug("debut ReadOne i=%d j = %d \n",i,j);*/
					/* Debug("debut t = %s \n",t);*/
     					ok= ReadOneOrcaBasis(i,j,t,&nsym);
					/* Debug("nsym apres = %d\n",nsym);*/
        				if(nsym==2)
					{
						Type[i].Norb = j+2;
						j++;
					}
					if(!ok)
					{
						j  = j - nsym;
     						if(j==0) Type[i].Ao=g_malloc(sizeof(AO));
     						else Type[i].Ao=g_realloc(Type[i].Ao,(j+1)*sizeof(AO));
        					Type[i].Norb=j+1;
					}
     				}
			}
			else { break; }
		}
	}
	/* Debug("nGroups = %d nAtoms = %d\n",nGroups,nAtoms);*/
	for(i=0;i<Ntype;i++)
	if(Type[i].Ao == NULL)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not read '%s' file, problem with basis set\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
		/* if(numAtoms) g_free(numAtoms);*/
		return FALSE;
	}
	/*resortAtoms(numAtoms);*/
	/* if(numAtoms) g_free(numAtoms);*/
    	return TRUE;
}
#define NCOLS 6
/********************************************************************************/
static gint read_one_block_orbitals_in_orca_output_file(FILE* file, gint ncart, gdouble **CoefOrbitals, gdouble *EnerOrbitals, gchar **SymOrbitals, gdouble* occ,gint* nOcc)
{
 	gchar *t;
 	gchar *AtomCoord[NCOLS];
 	gint i;
	gint NumOrb[NCOLS];
	gdouble EnerOrb[NCOLS];
	gdouble OccOrb[NCOLS];
	gint n;
	/* gint k;*/
	gint nReadOrb = 0;
	gint nR = 0;
 	gchar sdum1[30];
 	gchar sdum2[30];

	*nOcc = 0;

 	t=g_malloc(BSIZE*sizeof(gchar));
 	for(i=0;i<NCOLS;i++) AtomCoord[i]=g_malloc(BSIZE*sizeof(gchar));

	nReadOrb = 0;
	nR = NCOLS;
	/* Debug("ncar = %d nOrb = %d\n",ncart,NOrb);*/
	for(n=0;n<ncart && nR==NCOLS;n++)
	{
    		{ char* e = fgets(t,BSIZE,file);}
		/* Debug("%s\n",t);*/
		nR = sscanf(t,"%d %d %d %d %d %d",&NumOrb[0],&NumOrb[1],&NumOrb[2],&NumOrb[3],&NumOrb[4],&NumOrb[5]);
		if(nR<0) nR = 0;
		/* Debug("nR = %d\n",nR);*/
		for(i=nR;i<NCOLS;i++) NumOrb[i]=NOrb-1;
		for(i=0;i<nR;i++) 
		if(NumOrb[i]<0 || NumOrb[i]>NOrb-1) 
		{
			nR=0;
			break;
		}
		if(nR<=0) break;

		for(i=0;i<nR;i++) SymOrbitals[NumOrb[i]] = g_strdup("UNK");

			
    		{ char* e = fgets(t,BSIZE,file);}
		/* Debug("%s\n",t); */
		sscanf(t,"%lf %lf %lf %lf %lf %lf",
			&EnerOrb[0], &EnerOrb[1], &EnerOrb[2], &EnerOrb[3], &EnerOrb[4], &EnerOrb[5]);
		for(i=0;i<nR;i++) EnerOrbitals[NumOrb[i]] = EnerOrb[i];

    		{ char* e = fgets(t,BSIZE,file);}
		/* Debug("%s\n",t); */
		sscanf(t,"%lf %lf %lf %lf %lf %lf",
			&OccOrb[0], &OccOrb[1], &OccOrb[2], &OccOrb[3], &OccOrb[4], &OccOrb[5]);
		for(i=0;i<nR;i++) occ[NumOrb[i]] = OccOrb[i];

		for(i=0;i<nR;i++)
        		if(OccOrb[i]>0) (*nOcc)++;

    		{ char* e = fgets(t,BSIZE,file);}
		/* Debug("%s\n",t);*/
		for(i=0;i<NOrb;i++)
		{
    			{ char* e = fgets(t,BSIZE,file);}
			/*Debug("%s\n",t);*/
			/* k = sscanf(t,"%s %s %lf %lf %lf %lf %lf %lf",sdum1,sdum2,*/
			sscanf(t,"%s %s %lf %lf %lf %lf %lf %lf",sdum1,sdum2,
					&CoefOrbitals[NumOrb[0]][i],
					&CoefOrbitals[NumOrb[1]][i],
					&CoefOrbitals[NumOrb[2]][i],
					&CoefOrbitals[NumOrb[3]][i],
					&CoefOrbitals[NumOrb[4]][i],
					&CoefOrbitals[NumOrb[5]][i]);
		}
		nReadOrb += nR;
		if(nR!=NCOLS)
		{
			for(i=nReadOrb;i<NOrb;i++)
			   		SymOrbitals[i] = g_strdup("DELETE");
		}
		
 	}
    	{ char* e = fgets(t,BSIZE,file);} /* backe space line */
 	g_free(t);
 	for(i=0;i<NCOLS;i++) g_free(AtomCoord[i]);
	return nReadOrb;
}
/********************************************************************************/
static gboolean read_last_orbitals_in_orca_output_file(gchar *fileName)
{
 	gchar *t;
 	gboolean OK;
 	FILE *file;
	gint ncart;
	gdouble **CoefOrbitals;
	gdouble *EnerOrbitals;
	gdouble *OccOrbitals;
	gchar **SymOrbitals;
	gint NOcc = 0;
	gint nReadOrb = 0;
	gchar* title ="MOLECULAR ORBITALS";
	long int pos = 0;
	gboolean UHF = FALSE;
	
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return FALSE;
 	}

 	t=g_malloc(BSIZE*sizeof(gchar));
 	file = FOpen(fileName, "rb");
 	if(file ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
  		return FALSE;
 	}
	OK = FALSE;
 	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file))break;
		if(strstr( t, "SPIN CONTAMINATION"))UHF = TRUE;
		if(strstr( t, " SPIN DOWN ORBITALS"))UHF = TRUE;
          	if(strstr( t, title))
		{
			if(!fgets(t,BSIZE,file))break;
			if(strstr( t, "----"))
			{
				OK = TRUE;
				pos = ftell(file);
			}
		}
	}
	if(!OK)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not read the orbitals from '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
		fclose(file);
  		return FALSE;
	}
	
	fseek(file, pos, SEEK_SET);
  
	CoefOrbitals = CreateTable2(NOrb);
	EnerOrbitals = g_malloc(NOrb*sizeof(gdouble));
	OccOrbitals = g_malloc(NOrb*sizeof(gdouble));
	SymOrbitals = g_malloc(NOrb*sizeof(gchar*));

  	ncart=NOrb/NCOLS;
	if(NOrb%NCOLS!=0) ncart++;

	/* Debug("Begin read_one_block_orbitals_in_orca_output_file \n");*/
	nReadOrb = read_one_block_orbitals_in_orca_output_file(file, ncart, CoefOrbitals, EnerOrbitals, SymOrbitals, OccOrbitals,&NOcc);
	/* Debug("nRead = %d\n",nReadOrb);*/
	if(nReadOrb>0 && UHF)
	{
		CoefAlphaOrbitals = CoefOrbitals;
		EnerAlphaOrbitals = EnerOrbitals;
		SymAlphaOrbitals = SymOrbitals;
		OccAlphaOrbitals = OccOrbitals;

		NAlphaOcc = NOcc;
		NAlphaOrb = nReadOrb;

		CoefOrbitals = CreateTable2(NOrb);
		EnerOrbitals = g_malloc(NOrb*sizeof(gdouble));
		OccOrbitals = g_malloc(NOrb*sizeof(gdouble));
		SymOrbitals = g_malloc(NOrb*sizeof(gchar*));
		/* Debug("Begin Beta read_one_block_orbitals_in_orca_output_file \n");*/
		nReadOrb =  read_one_block_orbitals_in_orca_output_file(file, ncart, CoefOrbitals, EnerOrbitals, SymOrbitals, OccOrbitals,&NOcc);
		/* Debug("nRead Beta = %d\n",nReadOrb);*/
		if(nReadOrb>0)
		{
			CoefBetaOrbitals = CoefOrbitals;
			EnerBetaOrbitals = EnerOrbitals;
			OccBetaOrbitals = OccOrbitals;
			SymBetaOrbitals = SymOrbitals;
			NBetaOcc = NOcc;
			NBetaOrb = nReadOrb;
		}
		else
		{
			FreeTable2(CoefOrbitals,NOrb);
			g_free(EnerOrbitals);
			g_free(OccOrbitals);
			g_free(SymOrbitals);

			FreeTable2(CoefAlphaOrbitals,NOrb);
			g_free(EnerAlphaOrbitals);
			g_free(OccAlphaOrbitals);
			g_free(SymAlphaOrbitals);
		}
	}
	else if(nReadOrb>0)
	{
		gint i;
		CoefAlphaOrbitals = CoefOrbitals;
		EnerAlphaOrbitals = EnerOrbitals;
		SymAlphaOrbitals = SymOrbitals;
		OccAlphaOrbitals = OccOrbitals;

		NAlphaOcc = NOcc;
		NAlphaOrb = nReadOrb;
		CoefBetaOrbitals = CoefOrbitals;
		EnerBetaOrbitals = EnerOrbitals;
		SymBetaOrbitals = SymOrbitals;
		OccBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		NBetaOcc = NOcc;
		NBetaOrb = nReadOrb;
		for(i=0;i<NOrb;i++)
		{
			OccBetaOrbitals[i] = 0;
			if(OccOrbitals[i]>1)
			{
				OccBetaOrbitals[i] = OccOrbitals[i]-1;
				OccOrbitals[i] = 1;
			}
		}
	}
	else
	{
		FreeTable2(CoefOrbitals,NOrb);
		g_free(EnerOrbitals);
		g_free(OccOrbitals);
		g_free(SymOrbitals);
	}
	
	if(nReadOrb>0) OK = TRUE;
	/* Debug("End of read Orbitals OK = %d\n",OK);*/
 	fclose(file);
 	g_free(t);
	return OK;
}
/********************************************************************************/
void read_orca_orbitals_from_output_file(gchar* fileName)
{
	gint typefile;
	gboolean Ok = FALSE;
 	gchar *t = NULL;

	typefile =get_type_file_orb(fileName);

	if(typefile==GABEDIT_TYPEFILE_UNKNOWN) return;
	if(typefile != GABEDIT_TYPEFILE_ORCA)
	{
		Message(_("Sorry, This file is not in Orca output file\n"),_("Error"),TRUE);
		return ;
	}
	free_data_all();
	t = get_name_file(fileName);
	set_status_label_info(_("File name"),t);
	g_free(t);
	set_status_label_info(_("File type"),"Orca");
	set_status_label_info(_("Mol. Orb."),"Reading");

	free_orbitals();	
 	if(!gl_read_orca_file_geomi(fileName,-1))
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}

 	InitializeAll();
 	if(!DefineOrcaBasisType(fileName))
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}
 	/* Debug("End define Basis Type NAOrb = %d \n",NAOrb);*/
 	/* PrintBasis();*/
 	/* Debug("End Print Basis\n");*/
	
	/* Debug("Begin DefineOrcaSphericalBasis\n");*/
 	DefineOrcaSphericalBasis();
 	
	/* Debug("End DefineOrcaSphericalBasis\n");*/
 	/* PrintAllBasis();*/
 	NormaliseAllBasis();
	/* 
	 Debug("Basis after normalisation\n"); 
 	PrintAllBasis();
	*/
 	/* Debug("Spherical basis(Norb=%d)\n",NAOrb);*/
 	DefineNOccs();

	Ok =  read_last_orbitals_in_orca_output_file(fileName);

	/* Debug("End read_last_orbitals_in_orca_output_file\n");*/
	if(Ok)
	{
		/*
		PrintAllOrb(CoefAlphaOrbitals);
		Debug("End PrintAllOrb Alpha\n");
		PrintAllOrb(CoefBetaOrbitals);
		Debug("End PrintAllOrb Beta\n");
		*/
		set_status_label_info(_("Mol. Orb."),_("Ok"));
		glarea_rafresh(GLArea); /* for geometry */
		NumSelOrb = NAlphaOcc-1;
		/*
		Debug("End NumSelOrb %d\n",NumSelOrb);
		Debug("End NAlphaOrb %d\n",NAlphaOrb);
		Debug("End NBetaOrb %d\n",NBetaOrb);
		*/
		create_list_orbitals();
	}
	else
	{
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
	}

}
#undef NCOLS
