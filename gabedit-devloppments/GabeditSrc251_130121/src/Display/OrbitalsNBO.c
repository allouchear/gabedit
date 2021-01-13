/* OrbitalsNBO.c */
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
#include "../Utils/Zlm.h"
#include "../Utils/Constants.h"
#include "../Geometry/GeomGlobal.h"
#include "GeomDraw.h"
#include "GLArea.h"
#include "UtilsOrb.h"
#include "Basis.h"
#include "GeomOrbXYZ.h"
#include "AtomicOrbitals.h"
#include "StatusOrb.h"
#include "Basis.h"
#include "Orbitals.h"
#include "GeomOrbXYZ.h"
#include "BondsOrb.h"
#include "LabelsGL.h"

/********************************************************************************/
typedef enum
{
  GABEDIT_ORBLOCALTYPE_BOYS=0,
  GABEDIT_ORBLOCALTYPE_EDMISTON,
  GABEDIT_ORBLOCALTYPE_PIPEK,
  GABEDIT_ORBLOCALTYPE_UNKNOWN
} GabEditOrbLocalType;

typedef enum
{
  GABEDIT_ORBTYPE_ALPHA = 0,
  GABEDIT_ORBTYPE_BETA,
  GABEDIT_ORBTYPE_MOLECULAR,
  GABEDIT_ORBTYPE_MCSCF,
  GABEDIT_ORBTYPE_EIGENVECTORS,
  GABEDIT_ORBTYPE_BOYS_ALPHA,
  GABEDIT_ORBTYPE_BOYS_BETA,
  GABEDIT_ORBTYPE_BOYS,
  GABEDIT_ORBTYPE_EDMISTON_ALPHA,
  GABEDIT_ORBTYPE_EDMISTON_BETA,
  GABEDIT_ORBTYPE_EDMISTON,
  GABEDIT_ORBTYPE_PIPEK_ALPHA,
  GABEDIT_ORBTYPE_PIPEK_BETA,
  GABEDIT_ORBTYPE_PIPEK,
} GabEditOrbType;
/********************************************************************************/
static gboolean sphericalBasis = TRUE;
/********************************************************************************/
static void get_charges_from_nbo_output_file(FILE* file,gint nAtoms)
{
/* charges not available in a nbo file. I set it to 0.0 */
	gint i;
	for(i=0;i<nAtoms;i++)
		GeomOrb[i].partialCharge = 0.0;
}
/********************************************************************************/
static gboolean goToLine(FILE* file,char* nextString)
{
	static char t[BSIZE];
 	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file))break;
		if (strstr(t,nextString)) return TRUE;
	}
	return FALSE;
}
/********************************************************************************/
static void setTitle(FILE* file)
{
	gchar t[BSIZE];
	
	if(!file)
	{
	   	set_label_title("",0,0);
		return;
	}
	rewind(file);
	if(fgets(t,BSIZE,file))
	if(fgets(t,BSIZE,file))
	   	set_label_title(t,0,0);
}
/********************************************************************************/
static gboolean read_geomorb_nbo_file_geom(gchar *fileName)
{
 	gchar *tmp = NULL;
 	FILE *file;
 	gint i;
	gint k;
 	gint j=0;
	gint uni=1;
	gint z = 0;
	static gchar t[BSIZE];
	gint nAtoms, nShell, nExp;
  
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No file selected"),_("Error"),TRUE);
    		return FALSE;
 	}

 	file = FOpen(fileName, "rb");
 	if(file == NULL)
 	{
  		Message(_("Sorry\nI cannot open this file"),_("Error"),TRUE);
  		return FALSE;
 	}
	if(!goToLine(file,"--------")) return FALSE;
	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%d %d %d",&nAtoms,&nShell,&nExp);
	if(!goToLine(file,"--------")) return FALSE;
	if(nAtoms<1) return FALSE;
  	init_dipole();
    	GeomOrb=g_malloc(nAtoms*sizeof(TypeGeomOrb));
	uni = 1;

	tmp = get_name_file(fileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"NBO");
	set_status_label_info(_("Geometry"),_("Reading"));

	j = 0;
    	for(k=0;k<nAtoms;k++) 
	{
		if(!fgets(t,BSIZE,file)) break;
		sscanf(t,"%d %lf %lf %lf",&z, &GeomOrb[j].C[0], &GeomOrb[j].C[1], &GeomOrb[j].C[2]);
		if(z<=0) GeomOrb[j].Symb=g_strdup("X");
		else GeomOrb[j].Symb=get_symbol_using_z(z);
		if(uni==1) for(i=0;i<3;i++) GeomOrb[j].C[i] *= ANG_TO_BOHR;
		GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
		GeomOrb[j].variable = TRUE;
		j++;
		
	}
	nCenters = 0;
	if(k==nAtoms) nCenters = nAtoms;
	if(nCenters !=0) get_charges_from_nbo_output_file(file,nCenters);
 	fclose(file);
 	if(nCenters == 0 )
	{
		g_free(GeomOrb);
		sprintf(t,_("Sorry, I can not read this format from '%s' file\n"),fileName);
  		Message(t,_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		RebuildGeomD = TRUE;
		return FALSE;
	}
 	else
	{
  		/* DefineType();*/
 		gint i;
		if(Type)
		for(i=0;i<Ntype;i++)
		{
			if(Type[i].Ao != NULL)
			{
				for(j=0;j<Type[i].Norb;j++)
				{
					if(Type[i].Ao[j].Ex != NULL) g_free(Type[i].Ao[j].Ex);
					if(Type[i].Ao[j].Coef != NULL) g_free(Type[i].Ao[j].Coef);
				}
				g_free(Type[i].Ao);
			}
		}
		if(Type) g_free(Type);
 		Ntype =nCenters;
		Type = g_malloc(Ntype*sizeof(TYPE));
		for(i=0;i<nCenters;i++) 
		{
			GeomOrb[i].NumType = i;
     			Type[i].Symb=g_strdup(GeomOrb[i].Symb);
     			Type[i].N=GetNelectrons(GeomOrb[i].Symb);
			Type[i].Norb = 0;
			Type[i].Ao = NULL;
		}
		buildBondsOrb();
		RebuildGeomD = TRUE;
		return TRUE;
	}
	return TRUE;
}
/**********************************************/
static void DefineNBOSphericalBasis()
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

/********************************************************************************/
static void DefineNBOCartBasis()
{
 gint i,j,k,n;
 gint l1,l2,l3;
 gint L;
 gint *l[3]={NULL,NULL,NULL};
 gint m;

 NAOrb = 0;
 for(i=0;i<nCenters;i++)
 {
	 for(j=0;j<Type[GeomOrb[i].NumType].Norb;j++)
	 {
		L=Type[GeomOrb[i].NumType].Ao[j].L;
		NAOrb += (L+1)*(L+2)/2;
	 }
 }

 AOrb = g_malloc(NAOrb*sizeof(CGTF));
 if(SAOrb) g_free(SAOrb);
 SAOrb = NULL;
 
 k=-1;
 for(i=0;i<nCenters;i++)
	 for(j=0;j<Type[GeomOrb[i].NumType].Norb;j++)
 {
	L = Type[GeomOrb[i].NumType].Ao[j].L;

	for(m=0;m<3;m++)
	{
		if(l[m])
		   g_free(l[m]);
		l[m] = g_malloc((L+1)*(L+2)/2*sizeof(gint));
	}
	switch(L)
	{
	  case 1 :
		 m=0;
		 l[0][m] = 1;l[1][m] = 0;l[2][m] = 0; /* X */
		 m++;
		 l[0][m] = 0;l[1][m] = 1;l[2][m] = 0; /* Y */
		 m++;
		 l[0][m] = 0;l[1][m] = 0;l[2][m] = 1; /* Z */
	  	 break;
	  case 2 :
		 m=0;
		 l[0][m] = 2;l[1][m] = 0;l[2][m] = 0; /* XX */
		 m++;
		 l[0][m] = 0;l[1][m] = 2;l[2][m] = 0; /* YY */
		 m++;
		 l[0][m] = 0;l[1][m] = 0;l[2][m] = 2; /* ZZ */
		 m++;
		 l[0][m] = 1;l[1][m] = 1;l[2][m] = 0; /* XY */
		 m++;
		 l[0][m] = 1;l[1][m] = 0;l[2][m] = 1; /* XZ */
		 m++;
		 l[0][m] = 0;l[1][m] = 1;l[2][m] = 1; /* YZ */
		 break;
	  case 3 :
		 m=0;
		 l[0][m] = 3;l[1][m] = 0;l[2][m] = 0; /* XXX */
		 m++;
		 l[0][m] = 0;l[1][m] = 3;l[2][m] = 0; /* YYY */
		 m++;
		 l[0][m] = 0;l[1][m] = 0;l[2][m] = 3; /* ZZZ */
		 m++;
		 l[0][m] = 2;l[1][m] = 1;l[2][m] = 0; /* XXY */
		 m++;
		 l[0][m] = 2;l[1][m] = 0;l[2][m] = 1; /* XXZ */
		 m++;
		 l[0][m] = 1;l[1][m] = 2;l[2][m] = 0; /* YYX */
		 m++;
		 l[0][m] = 0;l[1][m] = 2;l[2][m] = 1; /* YYZ */
		 m++;
		 l[0][m] = 1;l[1][m] = 0;l[2][m] = 2; /* ZZX */
		 m++;
		 l[0][m] = 0;l[1][m] = 1;l[2][m] = 2; /* ZZY */
		 m++;
		 l[0][m] = 1;l[1][m] = 1;l[2][m] = 1; /* XYZ */
		 break;
	  case 4 :
		 m=0; l[0][m] = 4;l[1][m] = 0;l[2][m] = 0; /* XXXX */
		 m++; l[0][m] = 0;l[1][m] = 4;l[2][m] = 0; /* YYYY */
		 m++; l[0][m] = 0;l[1][m] = 0;l[2][m] = 4; /* ZZZZ */
		 m++; l[0][m] = 3;l[1][m] = 1;l[2][m] = 0; /* XXXY */
		 m++; l[0][m] = 3;l[1][m] = 0;l[2][m] = 1; /* XXXZ */
		 m++; l[0][m] = 1;l[1][m] = 3;l[2][m] = 0; /* YYYX */
		 m++; l[0][m] = 0;l[1][m] = 3;l[2][m] = 1; /* YYYZ */
		 m++; l[0][m] = 1;l[1][m] = 0;l[2][m] = 3; /* ZZZX */
		 m++; l[0][m] = 0;l[1][m] = 1;l[2][m] = 3; /* ZZZY */
		 m++; l[0][m] = 2;l[1][m] = 2;l[2][m] = 0; /* XXYY */
		 m++; l[0][m] = 2;l[1][m] = 0;l[2][m] = 2; /* XXZZ */
		 m++; l[0][m] = 0;l[1][m] = 2;l[2][m] = 2; /* YYZZ */
		 m++; l[0][m] = 2;l[1][m] = 1;l[2][m] = 1; /* XXYZ */
		 m++; l[0][m] = 1;l[1][m] = 2;l[2][m] = 1; /* YYXZ */
		 m++; l[0][m] = 1;l[1][m] = 1;l[2][m] = 2; /* ZZXY */
		 break;
	  default :
		m=0;
		for(l3=Type[GeomOrb[i].NumType].Ao[j].L;l3>=0;l3--)
			for(l2=Type[GeomOrb[i].NumType].Ao[j].L-l3;l2>=0;l2--)
		{
	 		l1 = Type[GeomOrb[i].NumType].Ao[j].L-l2-l3;
			l[0][m] = l1;
			l[1][m] = l2;
			l[2][m] = l3;
			m++;
		}
	}
		for(m=0;m<(L+1)*(L+2)/2;m++)
 		{
			l1 = l[0][m];
			l2 = l[1][m];
	 		l3 = l[2][m];
	 		k++;
	 		AOrb[k].numberOfFunctions=Type[GeomOrb[i].NumType].Ao[j].N;
			AOrb[k].NumCenter = i;
	 		AOrb[k].Gtf =g_malloc(AOrb[k].numberOfFunctions*sizeof(GTF));
	 		for(n=0;n<AOrb[k].numberOfFunctions;n++)
	 		{
	   			AOrb[k].Gtf[n].Ex   = Type[GeomOrb[i].NumType].Ao[j].Ex[n];
	   			AOrb[k].Gtf[n].Coef = Type[GeomOrb[i].NumType].Ao[j].Coef[n];
	   			AOrb[k].Gtf[n].C[0] = GeomOrb[i].C[0];
	   			AOrb[k].Gtf[n].C[1] = GeomOrb[i].C[1];
	   			AOrb[k].Gtf[n].C[2] = GeomOrb[i].C[2];
	   			AOrb[k].Gtf[n].l[0] = l1;
	   			AOrb[k].Gtf[n].l[1] = l2;
	   			AOrb[k].Gtf[n].l[2] = l3;
	 		}
	 
 		}
}

NOrb = NAOrb;
DefineAtomicNumOrb();
/* DefineNorb();*/
}
/********************************************************************************/
static gboolean read_basis_from_a_nbo_output_file(gchar *fileName)
{
 	static gchar t[BSIZE];
 	FILE *file;
	gint nAtoms = 0, nShell = 0, nExp = 0;
	gdouble* expo = NULL;
	gdouble** coefs = NULL;
	gint* numCenters = NULL;
	gint* nOrbs = NULL;
	gint** numTypes = NULL;
	gint* iPointers = NULL;
	gint* nGauss = NULL; 
	gint i;
	gint j;
	gint k;
	gint is;
	gint ie;
	gint jj;
	gint lmax = 0;

	
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return FALSE;
 	}

 	file = FOpen(fileName, "rb");
 	if(file == NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
  		return FALSE;
 	}
	if(!goToLine(file,"--------")) return FALSE;
	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%d %d %d",&nAtoms,&nShell,&nExp);
	if(nAtoms!=nCenters || nAtoms <= 0 || nShell <= 0 || nExp <= 0) return FALSE;
	if(!goToLine(file,"--------")) return FALSE;
	if(!goToLine(file,"--------")) return FALSE;

	numCenters = g_malloc(nShell*sizeof(gint));
	nOrbs = g_malloc(nShell*sizeof(gint));
	numTypes = g_malloc(nShell*sizeof(gint*));
	for(j=0;j<nShell;j++) numTypes[j] = g_malloc(nShell*sizeof(gint));
	iPointers = g_malloc(nShell*sizeof(gint));
	nGauss = g_malloc(nShell*sizeof(gint));

	lmax = 0;
	for(is = 0; is<nShell; is++)
	{
		if(1!=fscanf(file,"%d",&numCenters[is])) break;
		numCenters[is]--;
		if(1!=fscanf(file,"%d",&nOrbs[is])) break;
		if(1!=fscanf(file,"%d",&iPointers[is])) break;
		if(1!=fscanf(file,"%d",&nGauss[is])) break;
		for(k=0;k<nOrbs[is];k++)
		{
			if(1!=fscanf(file,"%d",&numTypes[is][k]))break;
        		gint l = numTypes[is][k]/100;
			if(lmax<l) lmax = l;
		}
		if(k!=nOrbs[is]) break;
	}
	if(is!=nShell) 
	{
		if(numCenters) g_free(numCenters);
		if(nOrbs) g_free(nOrbs);
		if(numTypes) for(j=0;j<nShell;j++) if(numTypes[j]) g_free(numTypes[j]);
		if(numTypes) g_free(numTypes);
		if(iPointers) g_free(iPointers);
		if(nGauss) g_free(nGauss);
		return FALSE;
	}
	if(!goToLine(file,"--------"))
	{
		if(numCenters) g_free(numCenters);
		if(nOrbs) g_free(nOrbs);
		if(numTypes) for(j=0;j<nShell;j++) if(numTypes[j]) g_free(numTypes[j]);
		if(numTypes) g_free(numTypes);
		if(iPointers) g_free(iPointers);
		if(nGauss) g_free(nGauss);
		return FALSE;
	}

    	expo = g_malloc(nExp*sizeof(gdouble));
	coefs = g_malloc((lmax+1)*sizeof(gdouble*));
	for(i=0;i<=lmax;i++) 
	{
		coefs[i] = g_malloc(nExp*sizeof(gdouble));
		for(j=0;j<nExp;j++)  coefs[i][j] = 0.0;
	}
	for(ie = 0; ie<nExp; ie++)
	{
		{int ii = fscanf(file,"%lf",&expo[ie]);}
	}
	if(!fgets(t,BSIZE,file))
	{
		if(numCenters) g_free(numCenters);
		if(nOrbs) g_free(nOrbs);
		if(numTypes) for(j=0;j<nShell;j++) if(numTypes[j]) g_free(numTypes[j]);
		if(numTypes) g_free(numTypes);
		if(iPointers) g_free(iPointers);
		if(nGauss) g_free(nGauss);
		if(expo) g_free(expo);
		if(coefs)
		{
			for(i=0;i<=lmax;i++) if(coefs[i]) g_free(coefs[i]);
			g_free(coefs);
		}
		return FALSE;
	}
	
	for(i=0;i<=lmax;i++) 
	{
		for(ie = 0; ie<nExp; ie++)
			if(1!=fscanf(file,"%lf",&coefs[i][ie])) break;
		if(!fgets(t,BSIZE,file)) break; /* f orb is not always available */
	}
	for(i=0;i<nCenters;i++) 
	{
		Type[i].Norb = 0;
		Type[i].Ao = NULL;
	}
	for(is = 0; is<nShell; is++)
	{
		i = numCenters[is];
		Type[i].Norb++;

		for(k=1;k<nOrbs[is];k++) 
        		if(numTypes[is][k]/100 != numTypes[is][k-1]/100) Type[i].Norb++;
	}

	for(i=0;i<nCenters;i++) 
	{
		Type[i].Ao=g_malloc(Type[i].Norb*sizeof(AO));
		for(j=0;j< Type[i].Norb;j++)
		{
			Type[i].Ao[j].Ex = NULL;
			Type[i].Ao[j].Coef = NULL;
		}
		j = 0;
		for(is = 0; is<nShell; is++)
		{
			gint ncont = nGauss[is];
			if(i != numCenters[is]) continue;
			for(k=0;k<nOrbs[is];k++) 
			{
        			gint l = numTypes[is][k]/100;
        			if(k>0 && numTypes[is][k]/100 == numTypes[is][k-1]/100) continue;
        			Type[i].Ao[j].L = l;
       				Type[i].Ao[j].N = ncont;
				Type[i].Ao[j].Ex=g_malloc(Type[i].Ao[j].N*sizeof(gdouble));
				Type[i].Ao[j].Coef=g_malloc(Type[i].Ao[j].N*sizeof(gdouble));
				for(jj=0;jj<Type[i].Ao[j].N;jj++)
				{
					gint jjP = iPointers[is] - 1 + jj;
					Type[i].Ao[j].Ex[jj] = expo[jjP];
					Type[i].Ao[j].Coef[jj] = coefs[l][jjP];
				}
				j++;
			}
		}

	}
/* free tables */
	if(numCenters) g_free(numCenters);
	if(nOrbs) g_free(nOrbs);
	if(expo) g_free(expo);
	if(coefs)
	{
		for(i=0;i<=lmax;i++) if(coefs[i]) g_free(coefs[i]);
		g_free(coefs);
	}
	
	if(numTypes)
	for(j=0;j<nShell;j++) if(numTypes[j]) g_free(numTypes[j]);
	if(numTypes) g_free(numTypes);
	if(iPointers) g_free(iPointers);
	if(nGauss) g_free(nGauss);

 	fclose(file);
	return TRUE;
}
/********************************************************************************/
/* typeobr = ALPHA or BETA */
static gboolean read_orbitals_in_nbo_file_alpha_or_beta(gchar *fileName,gint nAlpha, gint nBeta, gchar* typeOrb)
{
 	FILE *file;
 	gint i;
	gint k;
	gint j;
	gdouble **CoefOrbitals;
	gdouble *EnerOrbitals;
	gchar **SymOrbitals;
	gboolean om = FALSE;
	gdouble dum;
	
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return FALSE;
 	}

 	file = FOpen(fileName, "rb");
 	if(file ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
  		return FALSE;
 	}
	{
		gchar t[BSIZE];
    		{ char* e = fgets(t,BSIZE,file);}
    		{ char* e = fgets(t,BSIZE,file);}
		if(strstr(t," MOs ")) om = TRUE;
	}

	if(!goToLine(file,"--------")) return FALSE;
	if(!goToLine(file,typeOrb)) return FALSE;

  
	CoefOrbitals = CreateTable2(NOrb);
	EnerOrbitals = g_malloc(NOrb*sizeof(gdouble));
	SymOrbitals = g_malloc(NOrb*sizeof(gchar*));

	for(j=0;j<NOrb;j++)
	{
		EnerOrbitals[j]=0.0;
		for(i=0;i<NOrb;i++)
		{
			if(1!=fscanf(file,"%lf",&dum))break;
			if(om) CoefOrbitals[i][j] = dum;
			else CoefOrbitals[j][i] = dum;
		}
		if(i!=NOrb)
		{
			for(k=j;k<NOrb;k++) SymOrbitals[k] = g_strdup("DELETED");
			break;
		}
		SymOrbitals[j] = g_strdup("UNK");
	}
	/*
	printf("norb read=%d\n",j);
	*/
	if(strstr(typeOrb,"ALPHA"))
	{
	CoefAlphaOrbitals = CoefOrbitals;
	EnerAlphaOrbitals = EnerOrbitals;
	SymAlphaOrbitals = SymOrbitals;
	OccAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
	for(i=0;i<nAlpha;i++) OccAlphaOrbitals[i] = 1.0;
	for(i=nAlpha;i<NOrb;i++) OccAlphaOrbitals[i] = 0.0;
	}
	else{
	CoefBetaOrbitals = CoefOrbitals;
	EnerBetaOrbitals = EnerOrbitals;
	OccBetaOrbitals = OccAlphaOrbitals;
	SymBetaOrbitals = SymOrbitals;
	NAlphaOcc = nAlpha;
	NBetaOcc = nBeta;
	NAlphaOrb = NOrb;
	NBetaOrb = NOrb;
	}
	setTitle(file);
 	fclose(file);

	return TRUE;
}
/********************************************************************************/
static gboolean read_orbitals_in_nbo_file_all(gchar *fileName,gint nAlpha, gint nBeta)
{
 	FILE *file;
 	gint i;
	gint k;
	gint j;
	gdouble **CoefOrbitals;
	gdouble *EnerOrbitals;
	gchar **SymOrbitals;
	gboolean om = FALSE;
	gdouble dum;
	
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return FALSE;
 	}

 	file = FOpen(fileName, "rb");
 	if(file ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
  		return FALSE;
 	}
	{
		gchar t[BSIZE];
    		{ char* e = fgets(t,BSIZE,file);}
    		{ char* e = fgets(t,BSIZE,file);}
		if(strstr(t," MOs ")) om = TRUE;
	}
	if(!goToLine(file,"--------")) return FALSE;

  
	CoefOrbitals = CreateTable2(NOrb);
	EnerOrbitals = g_malloc(NOrb*sizeof(gdouble));
	SymOrbitals = g_malloc(NOrb*sizeof(gchar*));

	for(j=0;j<NOrb;j++)
	{
		EnerOrbitals[j]=0.0;
		for(i=0;i<NOrb;i++)
		{
			if(1!=fscanf(file,"%lf",&dum))break;
			if(om) CoefOrbitals[i][j] = dum;
			else CoefOrbitals[j][i] = dum;
		}
		if(i!=NOrb)
		{
			for(k=j;k<NOrb;k++) SymOrbitals[k] = g_strdup("DELETED");
			break;
		}
		SymOrbitals[j] = g_strdup("UNK");
	}
	/*
	printf("norb read=%d\n",j);
	*/
	CoefAlphaOrbitals = CoefOrbitals;
	EnerAlphaOrbitals = EnerOrbitals;
	SymAlphaOrbitals = SymOrbitals;
	OccAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
	nAlpha = nBeta;
	for(i=0;i<nAlpha;i++) OccAlphaOrbitals[i] = 1.0;
	for(i=nAlpha;i<NOrb;i++) OccAlphaOrbitals[i] = 0.0;

	CoefBetaOrbitals = CoefOrbitals;
	EnerBetaOrbitals = EnerOrbitals;
	OccBetaOrbitals = OccAlphaOrbitals;
	SymBetaOrbitals = SymOrbitals;
	NAlphaOcc = nAlpha;
	NBetaOcc = nBeta;
	NAlphaOrb = NOrb;
	NBetaOrb = NOrb;
	setTitle(file);
 	fclose(file);

	return TRUE;
}
/********************************************************************************/
static gboolean read_orbitals_in_nbo_file(gchar *fileName,gint nAlpha, gint nBeta)
{
	if(read_orbitals_in_nbo_file_alpha_or_beta(fileName, nAlpha,  nBeta, "ALPHA"))
	{
		return read_orbitals_in_nbo_file_alpha_or_beta(fileName, nAlpha,  nBeta, "BETA");
	}
	else
		return read_orbitals_in_nbo_file_all(fileName, nAlpha,  nBeta);

	return TRUE;
}
/********************************************************************************/
void read_nbo_orbitals(gchar* fileName)
{
	gint typefile;
	/* gint typebasis=1;*/ /* NBO print OM in cartezian presentation even ISPHER=0 or 1 or -1 */
	gchar *t = NULL;
	gint i;
	gint nAlpha;
	gint nBeta;
	gint typebasis = 0;
	gboolean Ok;
	gchar* fileName31 = NULL;


	typefile =get_type_file_orb(fileName);
	if(typefile==GABEDIT_TYPEFILE_UNKNOWN) return;


	if(typefile != GABEDIT_TYPEFILE_NBO)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not read this format from '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
		return ;
	}
	fileName31 = g_strdup_printf("%s.31",get_suffix_name_file(fileName));

	free_data_all();
	t = get_name_file(fileName31);
	set_status_label_info(_("File name"),t);
	g_free(t);
	set_status_label_info(_("File type"),"NBO");
	set_status_label_info(_("Mol. Orb."),_("Reading"));
	
	free_orbitals();	

	/* typebasis =get_type_basis_in_nbo_file(fileName);
	if(typebasis == -1)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,
				"Sorry, Gabedit does not support spherical basis with contaminant cartezian function\n\n"
				"Use ISPHER=-1 or ISPHER=1 in CONTROL block"
		       );
  		Message(buffer,_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}
	*/

 	if(!read_geomorb_nbo_file_geom(fileName31))
	{
		free_geometry();
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}
	set_status_label_info(_("Geometry"),_("Ok"));
	glarea_rafresh(GLArea); /* for geometry*/
	init_atomic_orbitals();
	if(!read_basis_from_a_nbo_output_file(fileName31))
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I cannot read basis from '%s' file\n"),fileName31);
  		Message(buffer,_("Error"),TRUE);
		if(GeomOrb)
		{
			init_atomic_orbitals();
			for(i=0;i<nCenters;i++) GeomOrb[i].Prop = prop_atom_get("H");
			free_geometry();
		}
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}

	g_free(fileName31);

	t = get_name_file(fileName);
	set_status_label_info(_("File name"),t);
	g_free(t);
	set_status_label_info(_("Mol. Orb."),_("Reading"));
 	InitializeAll();
	buildBondsOrb();
	RebuildGeomD = TRUE;
	reset_grid_limits();
	init_atomic_orbitals();


	if(typebasis == 0)
	{
 		DefineNBOSphericalBasis();
		sphericalBasis = TRUE;
	}
	else
	{
 		DefineNBOCartBasis();
		sphericalBasis = FALSE;
	}
 	
	/* 
	printf("Not normalized basis\n");
 	PrintAllBasis();
	*/
 	/* NormaliseAllBasis();*/
	NormaliseAllNoRadBasis();
	/*
	printf("Normalized basis\n");
 	PrintAllBasis();
	*/
 	DefineNOccs();

	nAlpha = NOrb;
	nBeta = NOrb;
	nAlpha = NAlphaOcc;
	nBeta = NBetaOcc;

	/*
	printf("Number of ALPHA occ = %d\n",nAlpha);
	printf("Number of BETA  occ = %d\n",nBeta);
	printf("NOrb = %d\n",NOrb);
	*/
	Ok = read_orbitals_in_nbo_file(fileName,nAlpha,nBeta);

	if(Ok)
	{
		/*PrintAllOrb(CoefAlphaOrbitals);*/
		set_status_label_info(_("Mol. Orb."),_("Ok"));
		glarea_rafresh(GLArea); /* for geometry*/
		NumSelOrb = NAlphaOcc-1;
		create_list_orbitals();
	}
	else
	{
		free_orbitals();	
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
	}

} 
