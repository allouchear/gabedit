/* OrbitalsMolpro.c */
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
#include "../Utils/Zlm.h"
#include "../Utils/GTF.h"
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
/********************************************************************************/
typedef struct _TypeFileListOrb
{
 long int geompos;
 long int baspos;
 gint norbtype;
 long int* orbpos;
 gchar** prognames;
 gchar** orbtypes;
}TypeFileListOrb;
/********************************************************************************/
typedef struct _OneBasis
{
 gint numbasis;
 gint numsym;

 gchar *sym;

 gint ncenters;
 gint* numcenters;
 gint* signe;

 gchar *type;

 gint nexps;
 gdouble* exps;
 gdouble* coefs;

 gint* numstandards;
}OneBasis;
/********************************************************************************/
typedef struct _TypeBasisBySym
{
 gint Nbasis;
 OneBasis* Basis;
}TypeBasisBySym;
/********************************************************************************/
typedef struct _OneOrbital
{
 gint numorb;
 gint numsym;
 gdouble energy;
 gdouble occ;

 gint ncoefs;
 gdouble* coefs;
}OneOrbital;
/********************************************************************************/
typedef struct _TypeOrbitalsBySym
{
 gint NumSym;
 gint NOrbs;
 OneOrbital* Orbs;
}TypeOrbitalsBySym;
/********************************************************************************/
void PrintMolproOneBasis(OneBasis bas)
{
	gint i=0;
	Debug(" %d %d %s \n",bas.numbasis,bas.numsym,bas.sym);
	Debug(" ncenters = %d : ",bas.ncenters);
	for(i=0;i<bas.ncenters;i++)
	{
		Debug(" (%d)%d ",bas.signe[i],bas.numcenters[i]);
	}
	Debug("\n Orbitals type : %s\n ",bas.type);
	Debug("Nexp = : %d\n ",bas.nexps);
	for(i=0;i<bas.nexps;i++)
	{
		Debug(" %lf %lf \n",bas.exps[i],bas.coefs[i]);
	}
}
/*****************************************************************************************************/
static gint get_first_integer(gchar* str)
{
	gchar t[BSIZE];
	gchar* tmp= str;
	gint i;
	gint n=-1;
	for(i=0;i<(gint)strlen(str);i++)
	{
		if(!testi(*tmp))
		{
			n=i-1;
			break;
		}
		tmp++;
	}
	if(n<0)
		return 0;
	for(i=0;i<=n;i++)
	{
		t[i] = str[i];
	}
	t[n+1] ='\0';
	/* Debug("tfirst=%s\n",t); */
	return atoi(t);

}
/********************************************************************************/
void delete_identique_atomic_standard_orb(TypeBasisBySym BasisBySym[MAXSYM],gint maxsym)
{
 gint i;
 gint j;
 CGTF* tmp = NULL;
 gint naorbs = 1;
 gboolean* todelete;

 if(NAOrb<1)
	 return;

 tmp = g_malloc(NAOrb*sizeof(CGTF));
 todelete = g_malloc(NAOrb*sizeof(gboolean));
 tmp[0] = AOrb[0];
 todelete[0] = FALSE;
 for(i=1;i<NAOrb;i++)
 {
 	todelete[i] = TRUE;
 	for(j=0;j<i;j++)
		if(CGTFEqCGTF(&tmp[j],&AOrb[i]))
			break;
	if(j!=i)
		continue;
 	todelete[i] = FALSE;
	tmp[naorbs]=AOrb[i];
	naorbs++;
 }
  for(i=0;i<NAOrb;i++)
	if(todelete[i])
		g_free(AOrb[i].Gtf);
  if(AOrb)
	  g_free(AOrb);
 NAOrb = naorbs;
 AOrb = tmp;
 if(SAOrb) g_free(SAOrb);
 SAOrb = NULL;

}
/********************************************************************************/
void sort_atomic_standard_orb(TypeBasisBySym BasisBySym[MAXSYM],gint maxsym)
{
 gint i;
 gint j;
 gint k;
 CGTF temp;

 for(i=0;i<NAOrb-1;i++)
 {
	k = i;
 	for(j=i+1;j<NAOrb;j++)
	{
		if( AOrb[j].NumCenter< AOrb[k].NumCenter)
			k=j;
		else
		if( AOrb[j].NumCenter== AOrb[k].NumCenter)
		{
			if(AOrb[j].L<AOrb[k].L)
				k = j;
			else
				if(AOrb[j].L==AOrb[k].L)
				{
					if(AOrb[j].L!=1)
					{
				      		if(abs(AOrb[j].M)<abs(AOrb[k].M) )
							k = j;
				      		else
				      			if(abs(AOrb[j].M)==abs(AOrb[k].M) &&AOrb[j].M>AOrb[k].M )
								k = j;
					}
					else /* orbital p : px, py and pz */
					{
				      		if(abs(AOrb[j].M)>abs(AOrb[k].M) )
							k = j;
				      		else
				      			if(abs(AOrb[j].M)==abs(AOrb[k].M) &&AOrb[j].M>AOrb[k].M )
								k = j;
					}
				}
		}
	}
	if( k!= i)
	{
		temp =AOrb[k];
		AOrb[k] = AOrb[i];
		AOrb[i] = temp;
	}
 }
}
/*****************************************************************************************************/
void define_standard_spherical_basis_from_molpro_basis(TypeBasisBySym BasisBySym[MAXSYM],gint maxsym)
{
 gint i;
 gint j;
 gint k;
 gint ic;
 gint c;
 gint L,M;
 CGTF *temp;
 Zlm Stemp;
 gint N,Nc,n;
 gchar* t;
 gint naorbs;


 naorbs = 0;
 for(i=0;i<maxsym;i++)
	 for(j=0;j<BasisBySym[i].Nbasis;j++)
	 		naorbs +=BasisBySym[i].Basis[j].ncenters;

 /* Debug("naorbs = %d\n",naorbs); */
 temp  = g_malloc(naorbs*sizeof(CGTF));

 k=-1;
 for(i=0;i<maxsym;i++)
	 for(j=0;j<BasisBySym[i].Nbasis;j++)
	{
	 	L =abs(get_first_integer(BasisBySym[i].Basis[j].type))-1;
		t = BasisBySym[i].Basis[j].type;
		/* Debug("t = %s\n",t);*/
		if(*t=='-')
			t = t+3;
		else
			t = t+2;
		/* Debug("t = %s\n",t);*/
		switch(L)
		{
			case 0 : M = 0;break;
			case 1 : if(strstr(t,"x"))
					 M = +1;
				 else
				 if(strstr(t,"y"))
					 M = -1;
				 else
					 M = 0;
				 break;
			default : M = atoi(t);
				 if(strstr(t,"-"))
					 M = -M;
				 break;

		}
		/* Debug("L = %d M = %d \n",L,M);*/
	 	Stemp =  getZlm(L,M);
	 	BasisBySym[i].Basis[j].numstandards = g_malloc(BasisBySym[i].Basis[j].ncenters*sizeof(gint));
		for(c=0;c<BasisBySym[i].Basis[j].ncenters;c++)
		{
	 		k++;

	 		BasisBySym[i].Basis[j].numstandards[c] = k;

	 		temp[k].numberOfFunctions=Stemp.numberOfCoefficients*BasisBySym[i].Basis[j].nexps;
		    temp[k].NumCenter=BasisBySym[i].Basis[j].numcenters[c]-1;
		    /* Debug("numcenter = %d\n",temp[k].NumCenter);*/
		    temp[k].L = L;
		    temp[k].M = M;
	 		temp[k].Gtf =g_malloc(temp[k].numberOfFunctions*sizeof(GTF));
          		Nc=-1;
	 		for(N=0;N<BasisBySym[i].Basis[j].nexps;N++)
	 			 for(n=0;n<Stemp.numberOfCoefficients;n++)
	 			{
	 			   	Nc++;
	   				temp[k].Gtf[Nc].Ex   = BasisBySym[i].Basis[j].exps[N];
	   				temp[k].Gtf[Nc].Coef = BasisBySym[i].Basis[j].coefs[N]*Stemp.lxyz[n].Coef;
	   				for(ic=0;ic<3;ic++)
	   				{
	   					temp[k].Gtf[Nc].C[ic] = GeomOrb[temp[k].NumCenter].C[ic];
	   					temp[k].Gtf[Nc].l[ic] = Stemp.lxyz[n].l[ic];
	   				}
	 			}
		}
	}
	/* Debug("Begin free AOrb\n");*/
 	if(AOrb)
	 for(i=0;i<NAOrb;i++)
		g_free(AOrb[i].Gtf);
	g_free(AOrb);
	NAOrb  = naorbs;
	AOrb = temp;
 	if(SAOrb) g_free(SAOrb);
 	SAOrb = NULL;
	/* Debug("Begin DefineAtomicNumOrb\n");*/
	DefineAtomicNumOrb();
	/* Debug("End standard\n"); */
}

/********************************************************************************/
OneBasis get_one_basis_from_strbasis(gchar **strbasis,gint begin,gint end,gint norb,gint numorb)
{
	OneBasis bas;
	gchar t[BSIZE];
	gchar sym[BSIZE];
	gchar tc[BSIZE];
	gchar ttype[BSIZE];
	gint i;
	gchar* temp;
	gchar** allreals;
	/*gint j;*/

	sscanf(strbasis[begin+numorb-1],"%s %s",t,sym);
	bas.numbasis = atoi(t);
	bas.numsym = (gint) ( (atof(t)-atoi(t))*10.0+0.5);
	bas.sym = g_strdup(sym);
	bas.ncenters = 0;
 	bas.numcenters = g_malloc(sizeof(gint));
 	bas.signe = g_malloc(sizeof(gint));
 	bas.numstandards = NULL;
	bas.type = NULL;
	for(i=begin;i<=end;i++)
	{
		if((i-begin+1)<=norb)
		{
			if(sscanf(strbasis[i],"%s %s %s %s",t,sym,tc,ttype)!=4)
				break;
			if(atoi(tc)<=0)
				break;

			if(this_is_a_real(ttype))
				break;

			bas.ncenters++;
 			bas.numcenters = g_realloc(bas.numcenters ,bas.ncenters*sizeof(gint));
 			bas.signe = g_realloc(bas.signe, bas.ncenters*sizeof(gint));
 			bas.numcenters[bas.ncenters-1] = atoi(tc);
			bas.signe[bas.ncenters-1] = 1;
			if(ttype[0] =='-')
				bas.signe[bas.ncenters-1] = -1;


		}
		else
		{
			if(sscanf(strbasis[i],"%s %s",tc,ttype)!=2)
				break;
			if(atoi(tc)<=0)
				break;
			if(this_is_a_real(ttype))
				break;
			bas.ncenters++;
 			bas.numcenters = g_realloc(bas.numcenters ,bas.ncenters*sizeof(gint));
 			bas.signe = g_realloc(bas.signe, bas.ncenters*sizeof(gint));
 			bas.numcenters[bas.ncenters-1] = atoi(tc);
			bas.signe[bas.ncenters-1] = 1;
			if(ttype[0] =='-')
				bas.signe[bas.ncenters-1] = -1;
		}
		if(i==begin)
			bas.type = g_strdup(ttype);
	}
	if(bas.ncenters==0)
	{
 		g_free(bas.numcenters);
 		bas.numcenters = NULL;
 		g_free(bas.signe);
 		bas.signe = NULL;
	}
 	bas.exps = g_malloc(sizeof(gdouble));
 	bas.coefs = g_malloc(sizeof(gdouble));
	bas.nexps = 0;
	for(i=begin;i<=end;i++)
	{
		if((i-begin+1)<=norb)
		{
			if(sscanf(strbasis[i],"%s %s %s %s",t,sym,tc,ttype)!=4)
				break;
			if(atoi(tc) != 0 && !this_is_a_real(ttype))
				temp = strstr(strbasis[i],ttype);
			else
				temp = strstr(strbasis[i],sym);

			if(temp)
			{
					temp = strstr(temp," ");
					temp = g_strdup(temp); 
					 g_strstrip( temp ) ;
			}
			/* a remplacer par une autre fonction. */
 			/*allreals =g_strsplit (temp," ",norb+1);*/
 			allreals =gab_split (temp);
			/*
				Debug("temp : %s\n",temp); 
				Debug("allreal : ");
				for(j=0;j<=norb;j++)
					Debug("%s ",allreals [j]); 
				Debug("\n");
			*/
			bas.exps = g_realloc(bas.exps,(bas.nexps+1)*sizeof(gdouble));
			bas.coefs = g_realloc(bas.coefs,(bas.nexps+1)*sizeof(gdouble));
			bas.exps[bas.nexps] = atof(allreals [0]);
			bas.coefs[bas.nexps] = atof(allreals [numorb]);
			g_strfreev(allreals);
			bas.nexps++;
			if(temp)
				g_free(temp);
		}
		else
		{
			if(sscanf(strbasis[i],"%s %s",tc,ttype)!=2)
			{
				break;
			}
			if(!this_is_a_real(ttype))
			{
				temp = strstr(strbasis[i],ttype);
				if(temp)
					temp = strstr(temp," ");
			}
			else
			{
				temp = strbasis[i];
			}

			if(temp && sscanf(temp,"%s",tc)==1 && this_is_a_real(tc))
			{
				temp = g_strdup(temp);
			 	g_strstrip( temp ) ;
 			/*	allreals =g_strsplit (temp," ",norb+1);*/
			/*	Debug("temp : %s\n",temp); */
 				allreals =gab_split (temp);
				bas.exps = g_realloc(bas.exps,(bas.nexps+1)*sizeof(gdouble));
				bas.coefs = g_realloc(bas.coefs,(bas.nexps+1)*sizeof(gdouble));
			/*
				Debug("temp : %s\n",temp); 
				Debug("allreal : ");
				for(j=0;j<=norb;j++)
					Debug("%s ",allreals [j]); 
				Debug("\n");
			*/
				bas.exps[bas.nexps] = atof(allreals [0]);
				bas.coefs[bas.nexps] = atof(allreals [numorb]);
				g_strfreev(allreals);
				bas.nexps++;
				if(temp)
					g_free(temp);
			}
		}
	}
	if(bas.nexps==0)
	{
		g_free(bas.exps);
		g_free(bas.coefs);
 		bas.exps = NULL;
 		bas.coefs = NULL;
	}

	return bas;

}
/********************************************************************************/
OneBasis* get_basis_from_strbasis(gchar **strbasis,gint nrows,gint *nbas)
{
	
	OneBasis* Basis=g_malloc(sizeof(OneBasis));
	gint begin = 0;
	gint end = -1;
	gchar t[BSIZE];
	gchar tsym[BSIZE];
	gint i;
	gint norb;
	gint k;
	gint j;
	gint nbasis = 0;
	/* norb = Nb orb by rows */
	/* end = Nb of rows for new basis.*/
	for(k=0;k<nrows;k+=end-begin+1)
	{
		/* Debug("k=%d\n",k); */
	begin=end+1;
	norb = numb_of_string_by_row(strbasis[begin])-5;
	i=1;
	if(begin>=nrows-1)
		end = begin+1;
	else
	{
		end = begin+1;
		while(end<nrows)
		{
			sscanf(strbasis[end],"%s %s",t,tsym);
			if(atof(t)>atoi(t) && !this_is_a_real(tsym) )
			{
				if(i<norb)
				{
					i++;
					end++;
				}
				else
				{
					/* Debug("End of search\n");*/
					break;
				}
			
			}
			else
			{
				end++;
			}
		}
	}
	end--;
	
	/* Debug("Begin = %d End = %d Norb = %d\n",begin,end,norb); */
	for(j=begin;j<=end;j++)
	{
		/* Debug("\t %s",strbasis[j]); */

	}

	nbasis+=norb;
	Basis=g_realloc(Basis,nbasis*sizeof(OneBasis));
	{
		gint k = nbasis-norb-1;
		for(j=1;j<=norb;j++)
		{
			Basis[k+j] =  get_one_basis_from_strbasis(strbasis,begin,end,norb,j);
			/* PrintMolproOneBasis(Basis[k+j]);*/
		}
	}
	}
	if(nbasis==0)
		g_free(Basis);
	*nbas = nbasis;
	return Basis;	

}
/********************************************************************************/
OneBasis* read_basis_from_a_molpro_output_file(gchar *FileName,gint* nbasis,long int pos)
{
 	gchar **strbasis;
 	gchar *t;
 	FILE *fd;
 	gint taille=BSIZE;
 	gint nrows=0;
 	gint i=0;
	OneBasis* Basis;
	long int n = -1;
	gboolean OK = FALSE;
	
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return NULL;
 	}

 	fd = FOpen(FileName, "rb");
 	if(fd ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
  		return NULL;
 	}

 	t=g_malloc(taille);
 	while(!feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
		n++;
		if(n==pos)
		{
			OK = TRUE;
			break;
		}
        }
	if(!OK)
	{
		g_free(t);
  		Message(_("Sorry I can read basis in this file\n"),_("Error"),TRUE);
  		return NULL;
	}
    	{ char* e = fgets(t,taille,fd);}
    	{ char* e = fgets(t,taille,fd);}
    	{ char* e = fgets(t,taille,fd);}

	strbasis=g_malloc(sizeof(gchar*));
 	while(!feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
		if(this_is_a_backspace(t))
			break;
		nrows++;
		strbasis = g_realloc(strbasis,nrows*sizeof(gchar*));
		strbasis[nrows-1] = g_strdup(t);
	}
	if(nrows == 0)
	{
		g_free(t);
		g_free(strbasis);
  		Message(_("Sorry I can read basis in this file\n"),_("Error"),TRUE);
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
	Basis = get_basis_from_strbasis(strbasis,nrows,nbasis);
	for(i=0;i<nrows;i++)
		g_free(strbasis[i]);
	g_free(strbasis);
 	fclose(fd);
 	g_free(t);
	if(Type) g_free(Type);
	Type = NULL;
	return Basis;
}
/********************************************************************************/
OneOrbital get_one_orbital_from_strorbs(gchar **strorbs,gint begin,gint end,gint type)
{
	OneOrbital orb;
	gint i;
	gint j;
	gint n;
	gint k;
	gchar** allreals;
	gint b = 0;
	gint verybegin = begin;

	/*gint j;*/

	switch(type)
	{
		case 0: b= 4; break;
		case 1: b = 3; break;
		case 2: b = 2; break;
	}

	for(j=begin;j<=end;j++)
	{
		verybegin = j;
		if(!this_is_a_backspace(strorbs[j]))
			break;
	}
	orb.ncoefs = numb_of_string_by_row(strorbs[verybegin])-b;
	/*
	Debug("begin %d verybegin %d end %d\n",begin,verybegin,end);
	Debug("begin str = %s\n",strorbs[begin]);
	Debug("Verybegin str = %s\n",strorbs[verybegin]);
	Debug("end str = %s\n",strorbs[end]);
	*/
	if(orb.ncoefs<1)
	{
		orb.coefs = NULL;
		orb.energy = 0;
		orb.occ = -1;
		orb.numsym = -1;
		orb.numorb = -1;

		return orb;
	}
 	allreals =gab_split (strorbs[verybegin]);
	orb.numorb = atoi(allreals[0]); 
	orb.numsym = (gint)( (atof(allreals[0]) - atoi(allreals[0]))*10 +0.5); 
	if(strstr(allreals[1],"+") || strstr(allreals[1],"-") )
		orb.occ = 1.0;
	else
		orb.occ = atof(allreals[1]); 
	if(type!=2)
		orb.energy = atof(allreals[2]); 
	else
		orb.energy = 0.0;

	orb.coefs = g_malloc(orb.ncoefs *sizeof(gdouble));
	for(i=0;i<orb.ncoefs;i++)
		orb.coefs[i] = atof(allreals[b+i]);

	g_strfreev(allreals);
	for(j=verybegin+1;j<=end;j++)
	{
 		allreals =gab_split (strorbs[j]);
		n = numb_of_string_by_row(strorbs[j]);
		k =  orb.ncoefs ;
		orb.ncoefs += n;
		orb.coefs = g_realloc( orb.coefs ,orb.ncoefs *sizeof(gdouble));
		for(i=0;i<n;i++)
			orb.coefs[k+i] = atof(allreals[i]);
		g_strfreev(allreals);
	}

	return orb;

}
/********************************************************************************/
void PrintMolproBasis(TypeBasisBySym BasisBySym[MAXSYM],gint maxsym)
{
	gint i;
	gint j;
	for(i=0;i<maxsym;i++)
	{
		Debug("Symmetry number %d\n",i+1);
		Debug("Number of Orbitals %d\n",BasisBySym[i].Nbasis);
		for(j=0;j<BasisBySym[i].Nbasis;j++)
			PrintMolproOneBasis(BasisBySym[i].Basis[j]);
		Debug("\n\n\n");
	}

}
/********************************************************************************/
gint sort_basis_by_sym(OneBasis* Basis,gint nbasis,TypeBasisBySym BasisBySym[MAXSYM])
{
	gint i;
	gint j;
	gint maxsym = 0;
	for(i=0;i<MAXSYM;i++)
	{
		BasisBySym[i].Nbasis = 0;
		BasisBySym[i].Basis = NULL;
	}
	for(j=0;j<nbasis;j++)
	{
		if(Basis[j].numsym>maxsym)
			maxsym = Basis[j].numsym;
		i = Basis[j].numsym-1;
		BasisBySym[i].Nbasis++;
		if(!BasisBySym[i].Basis)
			BasisBySym[i].Basis = g_malloc(sizeof(OneBasis));
		else
			BasisBySym[i].Basis = g_realloc(BasisBySym[i].Basis,BasisBySym[i].Nbasis*sizeof(OneBasis));
		BasisBySym[i].Basis[BasisBySym[i].Nbasis-1] = Basis[j];
	}
	return maxsym;


}
/********************************************************************************/
void PrintMolproOneOrb(OneOrbital orb)
{
	gint i=0;
	Debug(" %d %d %lf %lf \n",orb.numorb,orb.numsym,orb.occ,orb.energy);
	Debug("Ncoefs = : %d\n ",orb.ncoefs);
	for(i=0;i<orb.ncoefs;i++)
	{
		Debug(" %lf ",orb.coefs[i]);
	}
	Debug("\n\n");
}
/********************************************************************************/
void PrintMolproOrbitals(TypeOrbitalsBySym OrbitalsBySym[MAXSYM],gint maxsym)
{
	gint i;
	gint j;
	for(i=0;i<maxsym;i++)
	{
		Debug("Symmetry number %d\n",i+1);
		Debug("Number of Orbitals %d\n",OrbitalsBySym[i].NOrbs);
		for(j=0;j<OrbitalsBySym[i].NOrbs;j++)
			PrintMolproOneOrb(OrbitalsBySym[i].Orbs[j]);
		Debug("\n\n\n");
	}

}
/********************************************************************************/
OneOrbital* get_orbitals_from_strorbs(gchar **strorbs,gint nrows,gint *norbitals,gint type)
{
	
	OneOrbital* orbs=g_malloc(sizeof(OneOrbital));
	gint begin = 0;
	gint end = -2;
	gint i;
	gint k;
	gint norbs = 0;
	OneOrbital orb;
	for(k=0;k<nrows;k+=end-begin+2)
	{
		/* Debug("k=%d\n",k); */
		begin=end+2;
		i=1;
		if(begin>=nrows-1)
			end = begin+1;
		else
		{
			end = begin+1;
			while(end<nrows)
			{
				if(this_is_a_backspace(strorbs[end]))
					break;
				else
					end++;
			}
		}
		end--;
		/* 
		 * Debug("\nOne orbital Begin = %d Begin = %d \n",begin,end);
		for(i=begin;i<=end;i++)
		{
			Debug("%s",strorbs[i]);
		}
		*/
		orb =  get_one_orbital_from_strorbs(strorbs,begin,end,type);
		if(orb.ncoefs>0)
		{
			norbs+=1;
			orbs=g_realloc(orbs,norbs*sizeof(OneOrbital));
			orbs[norbs-1] =  orb;
		}
	}
	if(norbs==0)
		g_free(orbs);
	*norbitals = norbs;
	/*
	for(i=0;i<norbs;i++)
		PrintMolproOneOrb(orbs[i]);
		*/
	return orbs;	

}
/********************************************************************************/
long int read_orbitals_from_a_molpro_output_file(gchar *FileName,TypeOrbitalsBySym OrbitalsBySym[MAXSYM],gint* maxsym,long int pos,gint type)
{
 	gchar **strorb;
 	gchar *t;
 	FILE *fd;
 	gint taille=BSIZE;
 	gint nrows=0;
 	gint i=0;
	gboolean endreading;
	gdouble val;
	gint norbitals;
	OneOrbital *orbs;
	long int n = -1;
	gboolean OK = FALSE;
	
	/* Debug("pos = %ld\n",pos);*/
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return n;
 	}

	set_status_label_info(_("Mol. Orb."),_("Reading"));

 	fd = FOpen(FileName, "rb");
 	if(fd ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
  		return n;
 	}

 	t=g_malloc(taille);
 	while(!feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
		n++;
		if(n==pos)
		{
			OK = TRUE;
			break;
		}
        }
	/* Debug("real pos = %ld\n",n); */
	if(!OK)
	{
		g_free(t);
  		Message(_("Sorry I can read basis in this file\n"),_("Error"),TRUE);
  		return n;
	}
	*maxsym = 0;
	for(i=0;i<MAXSYM;i++)
	{
		OrbitalsBySym[i].NumSym= 0;
		OrbitalsBySym[i].NOrbs= 0;
		OrbitalsBySym[i].Orbs= NULL;
	}

 	while(!feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
		n++;
		if(strstr(t,"Coefficients"))
                {
			break;
		}
															                        }  
    	{ char* e = fgets(t,taille,fd);}
	n++;

	strorb=g_malloc(sizeof(gchar*));
	endreading = FALSE;
 	while(!feof(fd))
	{
		nrows=0;
		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
			n++;
			if(this_is_a_backspace(t))
				break;
			if(
				strstr(t,"********") || 
				strstr(t,"Natural orbital dump") || 
			  	strstr(t,"Natural orbitals saved") || 
				strstr(t,"ELECTRON ORBITALS FOR NEGATIVE SPIN") ||
			  	strstr(t,"EXPECTATION VALUE OF S") || 
			  	strstr(t,"EXTRA SYMMETRY") ||
			  	strstr(t,"RECORD") 
			)
			{
				/* Debug("t = \n",t); */
				endreading = TRUE;
				break;
			}
		}
		if(endreading)
			break;
		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
			n++;
			if(
				strstr(t,"********") || 
				strstr(t,"Natural orbital dump") || 
			  	strstr(t,"Natural orbitals saved") || 
				strstr(t,"ELECTRON ORBITALS FOR NEGATIVE SPIN") ||
			  	strstr(t,"EXPECTATION VALUE OF S") || 
			  	strstr(t,"EXTRA SYMMETRY") ||
			  	strstr(t,"RECORD") 
			)
			{
				/* Debug("t = \n",t); */
				endreading = TRUE;
				break;
			}
			val = 0;
			sscanf(t,"%lf",&val);
			if(fabs(val-(gint)val)<1e-6&& fabs(val)>1e-6)
				break;
			nrows++;
			strorb = g_realloc(strorb,nrows*sizeof(gchar*));
			strorb[nrows-1] = g_strdup(t);

		}
		orbs = get_orbitals_from_strorbs(strorb,nrows,&norbitals,type);
		if(norbitals>0)
		{
			OrbitalsBySym[orbs[0].numsym-1].NumSym= orbs[0].numsym;
			OrbitalsBySym[orbs[0].numsym-1].NOrbs= norbitals;
			OrbitalsBySym[orbs[0].numsym-1].Orbs= orbs;
			if(orbs[0].numsym>*maxsym)
				*maxsym = orbs[0].numsym;
		}
		/* Debug("Orbitals for one sym\n"); 
		for(i=0;i<nrows;i++)
		{
			Debug("%s",strorb[i]);
		}
		*/
		for(i=0;i<nrows;i++)
			if(strorb[i])
				g_free(strorb[i]);
		if(endreading)
			break;

	}
	/*
	Debug("End of read \n");
	Debug("t = \n",t);
	*/
 	fclose(fd);
 	g_free(t);
	return n-1;
}
/********************************************************************************/
void define_standard_orbitlas_from_molpro_orbitals(TypeBasisBySym BasisBySym[MAXSYM],TypeOrbitalsBySym OrbitalsBySym[MAXSYM],gint maxsym,gint itype)
{	
	gint i;
	gint j;
	gint k;
	gint kp;
	gint n;
	gint ic;
	gdouble c;
	gint numsym;

	gdouble **CoefOrbitals;
	gdouble *EnerOrbitals;
	gdouble *OccOrbitals;
	gchar **SymOrbitals;
	gint Nocc = 0;
	gint Norb = 0;


	CoefOrbitals = CreateTable2(NAOrb);
	EnerOrbitals = g_malloc(NAOrb*sizeof(gdouble));
	OccOrbitals = g_malloc(NAOrb*sizeof(gdouble));
	SymOrbitals = g_malloc(NAOrb*sizeof(gchar*));

	for(i=0;i<NAOrb;i++)
	{
		EnerOrbitals[i] = 0.0;
		OccOrbitals [i] = 0.0;
		SymOrbitals[i] = NULL;
		for(j=0;j<NAOrb;j++)
			CoefOrbitals[i][j] = 0.0; 
	}


	Norb = 0;
	Nocc = 0;
	k = 0;
	for(i=0;i<maxsym;i++)
	{
		Norb += OrbitalsBySym[i].NOrbs;
		numsym = OrbitalsBySym[i].NumSym-1;
		for(j=0;j<OrbitalsBySym[i].NOrbs;j++)
		{
			
			EnerOrbitals[k] = OrbitalsBySym[i].Orbs[j].energy;
			OccOrbitals [k] = OrbitalsBySym[i].Orbs[j].occ;
			if(OccOrbitals [k]>0)
				Nocc = k+1;
			/*
			printf("Nocc=%d\n",Nocc);
			Debug("NumSym = %d NumOrbs = %d\n",i,j);
			Debug("NBasis= %d NumOrbs = %d\n",BasisBySym[numsym].Nbasis,OrbitalsBySym[i].Orbs[j].ncoefs);
			*/
			SymOrbitals[k] = g_strdup(BasisBySym[numsym].Basis[0].sym);
			
			for(n=0;n<OrbitalsBySym[i].Orbs[j].ncoefs;n++)
			{
				c = OrbitalsBySym[i].Orbs[j].coefs[n];
				for(ic=0;ic<BasisBySym[numsym].Basis[n].ncenters;ic++)
				{
					/*Debug("\t NumCoef = %d NumCenter  = %d\n",n,ic);*/
					kp = BasisBySym[numsym].Basis[n].numstandards[ic];
					/* Debug("\t k = %d kp = %d\n",k,kp);*/
					CoefOrbitals[k][kp] = c*BasisBySym[numsym].Basis[n].signe[ic]; 

				}
			}
			k++;
		}
	}

	/* 
	 * itype = 0 => Alpha end Beta 
	 * itype = 1 => Alpha 
	 * itype = 2 =>  Beta 
	*/
	switch(itype)
	{
		case 0 :
		CoefAlphaOrbitals = CoefOrbitals ;
		EnerAlphaOrbitals = EnerOrbitals;
		OccAlphaOrbitals = g_malloc(NAOrb*sizeof(gdouble));
		for(i=0;i<NAOrb;i++)
			OccAlphaOrbitals[i] = 0.0;
		for(i=0;i<Norb;i++)
		{
			if((OccOrbitals[i]-1)>1e-6)
				OccAlphaOrbitals[i] = 1.0;
			else
				OccAlphaOrbitals[i] = OccOrbitals[i];
		}
		SymAlphaOrbitals =  SymOrbitals ;
		NAlphaOrb = Norb;
		NAlphaOcc = Nocc;
		CoefBetaOrbitals = CoefOrbitals ;
		EnerBetaOrbitals = EnerOrbitals;
		OccBetaOrbitals = g_malloc(NAOrb*sizeof(gdouble));
		for(i=0;i<NAOrb;i++)
			OccBetaOrbitals[i] = 0.0;
		for(i=0;i<Norb;i++)
		{
			if((OccOrbitals[i]-1)>1e-6)
				OccBetaOrbitals[i] = OccOrbitals[i]-1;
			else
				OccBetaOrbitals[i] = 0.0;
		}
		SymBetaOrbitals =  SymOrbitals ;
		NBetaOrb = Norb;
		NBetaOcc = Nocc;
			break;
		case 1 :
		CoefAlphaOrbitals = CoefOrbitals ;
		EnerAlphaOrbitals = EnerOrbitals;
		OccAlphaOrbitals = g_malloc(NAOrb*sizeof(gdouble));
		for(i=0;i<NAOrb;i++)
			OccAlphaOrbitals[i] = 0.0;
		for(i=0;i<Norb;i++)
				OccAlphaOrbitals[i] = OccOrbitals[i];

		SymAlphaOrbitals =  SymOrbitals ;
		NAlphaOrb = Norb;
		NAlphaOcc = Nocc;
			break;
		case 2 :
		CoefBetaOrbitals = CoefOrbitals ;
		EnerBetaOrbitals = EnerOrbitals;
		OccBetaOrbitals = g_malloc(NAOrb*sizeof(gdouble));
		for(i=0;i<NAOrb;i++)
			OccBetaOrbitals[i] = 0.0;
		for(i=0;i<Norb;i++)
				OccBetaOrbitals[i] =  OccOrbitals[i]; 
		SymBetaOrbitals =  SymOrbitals ;
		NBetaOrb = Norb;
		NBetaOcc = Nocc;
			break;
	}

/*
	for(i=0;i<Norb;i++)
	{
		switch(itype)
		{
			case 0 : Debug("Orb n %d Occ = %lf OccA = %lf OccB = %lf \n",i+1,OccOrbitals[i],OccAlphaOrbitals[i],OccBetaOrbitals[i]);
				 break;
			case 1 : Debug("Orb n %d Occ = %lf OccA = %lf\n",i+1,OccOrbitals[i],OccAlphaOrbitals[i]);
				 break;
			case 2 : Debug("Orb n %d Occ = %lf OccB = %lf \n",i+1,OccOrbitals[i],OccBetaOrbitals[i]);
				 break;
		}
	}

	Debug("NAlphaOrb = %d NAlphaOcc = %d\n",NAlphaOrb,NAlphaOcc);
	Debug("NBetaOrb = %d NBetaOcc = %d\n",NBetaOrb,NBetaOcc);
	*/

	g_free(OccOrbitals);


}
/********************************************************************************/
TypeFileListOrb* scan_orbs_in_molpro_output_file(gchar* FileName,gint *norbtype)
{
	gint n=0;
	long int pos = -1;
	TypeFileListOrb* listorb = NULL;
	TypeFileListOrb onelistorb;
	gchar t[BSIZE];
	FILE* fd;
	gint taille = BSIZE;

	*norbtype=0;

 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return NULL;
 	}

 	fd = FOpen(FileName, "rb");
 	if(fd ==NULL)
 	{
		gchar* t = g_strdup_printf(_("Sorry I can not open \"%s\" file\n"),FileName);
  		Message(t,_("Error"),TRUE);
		g_free(t);
  		return NULL;
 	}

	pos++;
    	{ char* e = fgets(t,taille,fd);}
 	while(!feof(fd))
	{

          	if(strstr(t, "ATOMIC COORDINATES" ))
		{
			/*
	  		fgets(t,taille,fd);
	  		fgets(t,taille,fd);
			if(strstr(t,"Q_EFF"))continue;
			*/
			/* One geometry, Scan basis */
			onelistorb.prognames = NULL;
			onelistorb.orbtypes = NULL;
			onelistorb.orbpos = NULL;
			onelistorb.baspos = 0;
			onelistorb.norbtype = 0;
			onelistorb.geompos = pos;
			pos++;
    			{ char* e = fgets(t,taille,fd);}
			/* Debug("New Geometry\n");*/
 			while(!feof(fd) && !strstr(t, "ATOMIC COORDINATES" ))
			{
				if(strstr(t, "BASIS DATA" ))
				{
					/* One basi, Scan all programs*/
					onelistorb.baspos = pos;
					onelistorb.norbtype = 0;
					/* Debug("\tBasis OK\n");*/
					pos++;
    					{ char* e = fgets(t,taille,fd);}
 					while(!feof(fd) && !strstr(t, "ATOMIC COORDINATES"))
					{
						if(strstr(t, "PROGRAM *" ))
						{
							/* One program, Scan orbitals */
							gchar tmp[BSIZE];
							gchar dump[BSIZE];
							sscanf(t,"%s %s %s",dump,dump,tmp);
							/* Debug("\t\tProgram : %s\n",tmp);*/
							pos++;
    							{ char* e = fgets(t,taille,fd);}
 							while(!feof(fd) && !strstr(t,"PROGRAM *") && !strstr(t, "ATOMIC COORDINATES"))
							{
								if(strstr(t, "ELECTRON ORBITALS" ) && !strstr(t, "NEGATIVE"))
								{
									onelistorb.norbtype++;
									if(!onelistorb.prognames)
										onelistorb.prognames = g_malloc(sizeof(gchar*));
									else
										onelistorb.prognames = 
										g_realloc(onelistorb.prognames,onelistorb.norbtype*sizeof(gchar*));

							if(onelistorb.norbtype>=2 && strstr(onelistorb.prognames[onelistorb.norbtype-2],tmp))
							onelistorb.prognames[onelistorb.norbtype-1] = g_strdup_printf("%s_%d",tmp,onelistorb.norbtype);
							else
							onelistorb.prognames[onelistorb.norbtype-1] = g_strdup(tmp);

									if(!onelistorb.orbtypes)
										onelistorb.orbtypes = g_malloc(sizeof(gchar*));
									else
										onelistorb.orbtypes = 
										g_realloc(onelistorb.orbtypes,onelistorb.norbtype*sizeof(gchar*));

									onelistorb.orbtypes[onelistorb.norbtype-1] = g_strdup("ELECTRON ORBITALS");

									if(!onelistorb.orbpos)
										onelistorb.orbpos = g_malloc(sizeof(long int));
									else
										onelistorb.orbpos = 
										g_realloc(onelistorb.orbpos,onelistorb.norbtype*sizeof(long int));

									onelistorb.orbpos[onelistorb.norbtype-1] = pos;

									/*Debug("\t\t\tELECTRON ORBITALS\n");*/
									break;
								}
								if(strstr(t, "NATURAL ORBITALS" ))
								{
									onelistorb.norbtype++;
									if(!onelistorb.prognames)
										onelistorb.prognames = g_malloc(sizeof(gchar*));
									else
										onelistorb.prognames = 
										g_realloc(onelistorb.prognames,onelistorb.norbtype*sizeof(gchar*));

							if(onelistorb.norbtype>=2 && strstr(onelistorb.prognames[onelistorb.norbtype-2],tmp))
							onelistorb.prognames[onelistorb.norbtype-1] = g_strdup_printf("%s_%d",tmp,onelistorb.norbtype);
							else
							onelistorb.prognames[onelistorb.norbtype-1] = g_strdup(tmp);

									if(!onelistorb.orbtypes)
										onelistorb.orbtypes = g_malloc(sizeof(gchar*));
									else
										onelistorb.orbtypes = 
										g_realloc(onelistorb.orbtypes,onelistorb.norbtype*sizeof(gchar*));

									onelistorb.orbtypes[onelistorb.norbtype-1] = g_strdup("NATURAL ORBITALS");

									if(!onelistorb.orbpos)
										onelistorb.orbpos = g_malloc(sizeof(long int));
									else
										onelistorb.orbpos = 
										g_realloc(onelistorb.orbpos,onelistorb.norbtype*sizeof(long int));

									onelistorb.orbpos[onelistorb.norbtype-1] = pos;
									/*Debug("\t\t\tNATURAL ORBITALS\n");*/
									break;
								}
								if(strstr(t, "Natural orbitals" ))
								{
									onelistorb.norbtype++;
									if(!onelistorb.prognames)
										onelistorb.prognames = g_malloc(sizeof(gchar*));
									else
										onelistorb.prognames = 
										g_realloc(onelistorb.prognames,onelistorb.norbtype*sizeof(gchar*));

							if(onelistorb.norbtype>=2 && strstr(onelistorb.prognames[onelistorb.norbtype-2],tmp))
							onelistorb.prognames[onelistorb.norbtype-1] = g_strdup_printf("%s_%d",tmp,onelistorb.norbtype);
							else
							onelistorb.prognames[onelistorb.norbtype-1] = g_strdup(tmp);

									if(!onelistorb.orbtypes)
										onelistorb.orbtypes = g_malloc(sizeof(gchar*));
									else
										onelistorb.orbtypes = 
										g_realloc(onelistorb.orbtypes,onelistorb.norbtype*sizeof(gchar*));

									onelistorb.orbtypes[onelistorb.norbtype-1] = g_strdup("Natural orbitals");

									if(!onelistorb.orbpos)
										onelistorb.orbpos = g_malloc(sizeof(long int));
									else
										onelistorb.orbpos = 
										g_realloc(onelistorb.orbpos,onelistorb.norbtype*sizeof(long int));

									onelistorb.orbpos[onelistorb.norbtype-1] = pos;
									/*Debug("\t\t\tNatural orbitals\n");*/
									break;
								}
								pos++;
    								{ char* e = fgets(t,taille,fd);}
							}/* End while */
						}/* End if */
						if(!strstr(t,"PROGRAM *") && !strstr(t, "ATOMIC COORDINATES"))
						{
							pos++;
    							{ char* e = fgets(t,taille,fd);}
						}
					}/* End while */
					break;
				}/* End if*/
				if(!strstr(t, "ATOMIC COORDINATES"))
				{
					pos++;
    					{ char* e = fgets(t,taille,fd);}
				}
			}/* End while */
			
			/* Debug */
			/*
			{
				gint i;
				Debug("Geometry pos = %ld\n",onelistorb.geompos);
				Debug("Basis pos = %ld\n",onelistorb.baspos);
				Debug("Norb = %ld\n",onelistorb.norbtype);
				for(i=0;i<onelistorb.norbtype;i++)
				{
					Debug("\tProgram = %s\n",onelistorb.prognames[i]);
					Debug("\tOrbitals type = %s\n",onelistorb.orbtypes[i]);
				}
			}
			*/
			if(onelistorb.norbtype>0)
			{
				n++;
				if(!listorb)
					listorb = g_malloc(sizeof(TypeFileListOrb));
				else
					listorb = g_realloc(listorb,n*sizeof(TypeFileListOrb));
				listorb[n-1] = onelistorb;
			}
		}/* End if */
		if(!strstr(t, "ATOMIC COORDINATES"))
		{
			pos++;
    			{ char* e = fgets(t,taille,fd);}
		}
        }/* End while */
	fclose(fd);

	/* Debug*/
	/*
	{
		gint j;
		for(j=0;j<n;j++)
		{
		gint i;
		Debug("Geometry pos = %ld\n",listorb[j].geompos);
		Debug("Basis pos = %ld\n",listorb[j].baspos);
		Debug("Norb = %ld\n",listorb[j].norbtype);
		for(i=0;i<listorb[j].norbtype;i++)
		{
			Debug("\tProgram = %s\n",listorb[j].prognames[i]);
			Debug("\tOrbitals type = %s\n",listorb[j].orbtypes[i]);
		}
		}
	}
	*/
	*norbtype=n;
	return listorb;
}
/********************************************************************************/
void changed_geom(GtkWidget *combo,gpointer data)
{
	GtkWidget **entry = (GtkWidget **)data;
	gint i;
        GList *glist = NULL;
	gint norbtype;
	gint* norb;
	TypeFileListOrb* listorb=NULL;
	G_CONST_RETURN gchar* t;
	gint numorb;
	GtkWidget* comboprog = NULL;

        if(!this_is_an_object((GtkObject*)combo)) return;
        if(!this_is_an_object((GtkObject*)entry[0])) return;
        if(!this_is_an_object((GtkObject*)entry[1])) return;

	norb = (gint*)g_object_get_data(G_OBJECT(entry[0]), "Norb");
	norbtype = *norb;
	listorb = g_object_get_data(G_OBJECT (entry[0]), "ListOrb");

	t = gtk_entry_get_text(GTK_ENTRY(entry[0]));

	numorb = atoi(t)-1;
	if(numorb>norbtype-1 || numorb<0)
		return;

	for(i=0;i<listorb[numorb].norbtype;i++)
  			glist = g_list_append(glist,listorb[numorb].prognames[i]);

	if(glist)
	{
		comboprog = g_object_get_data (G_OBJECT (entry[1]), "Combo");
        	gtk_combo_box_entry_set_popdown_strings(comboprog, glist) ;
	}


	g_list_free(glist);
}
/********************************************************************************/
void changed_program(GtkWidget *combo,gpointer data)
{
	GtkWidget **entry = (GtkWidget **)data;
	gint i;
	gint norbtype;
	gint* norb;
	TypeFileListOrb* listorb=NULL;
	G_CONST_RETURN gchar* t;
	gint numorb;
	gint numprog;
	GtkWidget* label = NULL;

        if(!this_is_an_object((GtkObject*)combo)) return;
        if(!this_is_an_object((GtkObject*)entry[0])) return;
        if(!this_is_an_object((GtkObject*)entry[1])) return;

	norb = (gint*)g_object_get_data(G_OBJECT (entry[0]), "Norb");
	norbtype = *norb;
	listorb = g_object_get_data(G_OBJECT (entry[0]), "ListOrb");

	t = gtk_entry_get_text(GTK_ENTRY(entry[0]));

	numorb = atoi(t)-1;
	if(numorb>norbtype-1 || numorb<0)
		return;

	t = gtk_entry_get_text(GTK_ENTRY(entry[1]));
	numprog=-1;
	for(i=0;i<listorb[numorb].norbtype;i++)
	{
  		if(strstr(listorb[numorb].prognames[i],t))
		{
			numprog=i;
			break;
		}
	}

	if(numprog>-1)
	{
		label = g_object_get_data (G_OBJECT (entry[1]), "Label");
        	if(this_is_an_object((GtkObject*)label))
			gtk_label_set_text(GTK_LABEL(label),listorb[numorb].orbtypes[numprog]);
	}


	
}
/********************************************************************************/
void apply_orbs(GtkWidget *button,gpointer data)
{
	GtkWidget **entry = (GtkWidget **)data;
	GtkWidget *Win = g_object_get_data (G_OBJECT (button), "Window");
	TypeFileListOrb* listorb=NULL;
	gint nbasis;
	OneBasis* Basis;
	gint maxsym;
	TypeBasisBySym BasisBySym[MAXSYM];
	TypeOrbitalsBySym OrbitalsBySym[MAXSYM];
	G_CONST_RETURN gchar* t;
	gchar* FileName;
	gint i;
	gint* norb;
	gint norbtype;
	gint numorb;
	gint numprog;
	gint type;
	long int newpos = -1;

	free_data_all();
        if(!this_is_an_object((GtkObject*)entry[0]))
		return;
        if(!this_is_an_object((GtkObject*)entry[1]))
		return;

	norb = (gint*)g_object_get_data(G_OBJECT (entry[0]), "Norb");
	norbtype = *norb;
	listorb = g_object_get_data(G_OBJECT (entry[0]), "ListOrb");

	t = gtk_entry_get_text(GTK_ENTRY(entry[0]));

	numorb = atoi(t)-1;
	if(numorb>norbtype-1 || numorb<0)
	{
		Message(_("Error detected in gabedit\ni Please contat the author for this bug"),_("Error"),TRUE);
  		delete_child(Win);
		return;
	}

	t = gtk_entry_get_text(GTK_ENTRY(entry[1]));
	numprog=-1;
	for(i=0;i<listorb[numorb].norbtype;i++)
	{
  		if(strstr(listorb[numorb].prognames[i],t))
		{
			numprog=i;
			break;
		}
	}
	if(numprog<0)
	{
  		delete_child(Win);
		return;
	}
	/*
	Debug("Geometry number %d\n",numorb);
	Debug("Program number %d\n",numprog);
	*/
	FileName = (gchar*)g_object_get_data(G_OBJECT (entry[0]), "FileName");
	/* Debug("FileName = %s\n",FileName);*/
 	if(gl_read_molpro_file_geom_pos(FileName,listorb[numorb].geompos))
	{
		if(this_is_a_new_geometry()) free_objects_all();
		glarea_rafresh(GLArea);
		Basis = read_basis_from_a_molpro_output_file(FileName,&nbasis,listorb[numorb].baspos);
		if(Basis)
		{
			maxsym = sort_basis_by_sym(Basis,nbasis,BasisBySym);
			/* Debug */
			/* PrintMolproBasis(BasisBySym,maxsym);*/

			define_standard_spherical_basis_from_molpro_basis(BasisBySym,maxsym);
			NormaliseAllBasis();
			/* PrintAllBasis();*/
			type = 0;
			if(strstr(listorb[numorb].orbtypes[numprog],"NATURAL ORBITALS")) type = 1;
			else if(strstr(listorb[numorb].orbtypes[numprog],"Natural orbitals")) type = 2;
					
			newpos = read_orbitals_from_a_molpro_output_file(FileName,OrbitalsBySym,&maxsym,listorb[numorb].orbpos[numprog],type);
			/* PrintMolproOrbitals(OrbitalsBySym,maxsym);*/
			if(strstr(listorb[numorb].prognames[numprog],"UHF") || strstr(listorb[numorb].prognames[numprog],"UKS"))
			{
				/*
				Debug("End alpha reading\n");
				Debug("New pod = %ld\n",newpos);
				*/
				define_standard_orbitlas_from_molpro_orbitals(BasisBySym,OrbitalsBySym,maxsym,1);
				read_orbitals_from_a_molpro_output_file(FileName,OrbitalsBySym,&maxsym,newpos,type);
				define_standard_orbitlas_from_molpro_orbitals(BasisBySym,OrbitalsBySym,maxsym,2);

			}
			else
			{
				define_standard_orbitlas_from_molpro_orbitals(BasisBySym,OrbitalsBySym,maxsym,0);
			}
			NOrb = NAOrb;

			if(NAlphaOrb>0 || NBetaOrb>0)
			{
				set_status_label_info(_("Mol. Orb."),"Ok");
				if(NAlphaOrb>0)
				{
					TypeSelOrb = 1;
					NumSelOrb = NAlphaOcc-1;
				}
				else
				{
					TypeSelOrb = 2;
					NumSelOrb = NBetaOcc-1;
				}
				create_list_orbitals();
			}
			else
			{
				set_status_label_info(_("File name"),_("Nothing"));
				set_status_label_info(_("File type"),_("Nothing"));
				set_status_label_info(_("Mol. Orb."),_("Nothing"));
			}
		}
	}

  	delete_child(Win);


}
/********************************************************************************/
GtkWidget *create_orbitals_list_frame( GtkWidget *vboxall,GtkWidget **entry,TypeFileListOrb* listorb,gint norbtype,gchar* FileName)
{
  GtkWidget *frame;
  GtkWidget *combo;
  GtkWidget *vboxframe;
  GtkWidget *label;
#define NL 3
  gchar      *LabelLeft[NL];
  gchar      **tlistgeom = NULL;
  gushort i;
  GtkWidget *Table;
  gint* norb = g_malloc(sizeof(gint*));
  *norb = norbtype;

  LabelLeft[0] = g_strdup("Geometry number");
  LabelLeft[1] = g_strdup("Program");
  LabelLeft[2] = g_strdup("Type");

  if(norbtype>0) tlistgeom = g_malloc(norbtype*sizeof(gchar*));

  for(i=0;i<norbtype;i++)
  {
	  tlistgeom[i] = g_strdup_printf("%d",i+1);
  }

  frame = gtk_frame_new (_("Selection of type of orbitals"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  Table = gtk_table_new(3,3,FALSE);
  gtk_container_add(GTK_CONTAINER(vboxframe),Table);

	add_label_table(Table,LabelLeft[0],0,0);
	add_label_table(Table,":",0,1);
	combo = create_combo_box_entry(tlistgeom,norbtype,TRUE,-1,-1);
	gtk_table_attach(GTK_TABLE(Table),combo,2,4,0,1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);
	gtk_widget_show (combo);
	entry[0] = GTK_BIN(combo)->child;
	gtk_widget_set_sensitive(entry[0],FALSE); 
        g_object_set_data(G_OBJECT (entry[0]), "FileName",FileName);
        g_object_set_data(G_OBJECT (entry[0]), "Combo",combo);
	g_object_set_data(G_OBJECT (entry[0]), "Norb", norb);
	g_object_set_data(G_OBJECT (entry[0]), "ListOrb", listorb);
        g_signal_connect(G_OBJECT(GTK_COMBO_BOX(combo)), "changed",G_CALLBACK(changed_geom),entry);

	add_label_table(Table,LabelLeft[1],1,0);
	add_label_table(Table,":",1,1);
	combo = create_combo_box_entry(listorb[0].prognames,listorb[0].norbtype,TRUE,-1,-1);
	add_widget_table(Table,combo,1,2);
	entry[1] = GTK_BIN(combo)->child;
	gtk_widget_set_sensitive(entry[1],FALSE); 
        g_object_set_data(G_OBJECT (entry[1]), "Combo",combo);
	g_object_set_data(G_OBJECT (entry[1]), "Norb", norb);
	g_object_set_data(G_OBJECT (entry[1]), "ListOrb", listorb);

	add_label_table(Table,LabelLeft[2],2,0);
	add_label_table(Table,":",2,1);
	label = add_label_table(Table,listorb[0].orbtypes[0],2,2);
	g_object_set_data(G_OBJECT (entry[1]), "Label", label);
        g_signal_connect(G_OBJECT(GTK_COMBO_BOX(combo)), "changed",G_CALLBACK(changed_program),entry);

	gtk_widget_show_all(frame);
  
  	for (i=0;i<NL;i++)
        	g_free(LabelLeft[i]);
  	for (i=0;i<norbtype;i++)
        	g_free(tlistgeom[i]);

  	return frame;
}
/********************************************************************************/
void create_window_list_orbitals(GtkWidget *w,TypeFileListOrb* listorb,gint norbtype,gchar* FileName)
{
  GtkWidget *fp;
  GtkWidget *sep;
  GtkWidget *frame;
  GtkWidget *vboxall;
  GtkWidget *vboxwin;
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget **entry;
  gchar *title = g_strdup_printf(_("Orbitals available "));

  entry=g_malloc(2*sizeof(GtkWidget *));

  /* Principal Window */
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(fp),title);
  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(PrincipalWindow));
  gtk_window_set_modal (GTK_WINDOW (fp), TRUE);

  gtk_widget_realize(fp);

  add_glarea_child(fp," Orb. List ");


  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);
  vboxall = create_vbox(fp);
  vboxwin = vboxall;

  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_add(GTK_CONTAINER(vboxall),frame);
  gtk_widget_show (frame);

  vboxall = create_vbox(frame);

  frame = create_orbitals_list_frame(vboxall,entry,listorb,norbtype,FileName);

  sep = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(vboxwin), sep, FALSE, FALSE, 2);
  gtk_widget_show(sep);
  /* boutons box */
  hbox = create_hbox(vboxwin);
  gtk_widget_realize(fp);

  button = create_button(fp,_("Cancel"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 5);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(fp));
  gtk_widget_show (button);

  button = create_button(fp,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 5);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);
  g_object_set_data(G_OBJECT (button), "Window", fp);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(apply_orbs),(gpointer)entry);
  

  /* Show all */
  gtk_widget_show_all(fp);
}
/********************************************************************************/
void read_molpro_orbitals(gchar* FileName)
{
	gint norbtype;
	TypeFileListOrb* listorb = NULL;
	gchar* filename = g_strdup(FileName);

	set_status_label_info(_("File name"),FileName);
	set_status_label_info(_("File type"),"Molpro");
	set_status_label_info(_("Mol. Orb."),_("Nothing"));

 	InitializeAll();

	listorb = scan_orbs_in_molpro_output_file(FileName,&norbtype);
	if(listorb)
		create_window_list_orbitals(NULL,listorb,norbtype,filename);
	else
	{
		gchar* t = g_strdup_printf(_("Sorry,\nI can not read atomic basis and/or molecular orbitals from\n \"%s\"\n file\n"
					   "\nFor display density and/or molecular orbitals read from molpro output file,\n"
					   "the following keywords are required:\n"
					   "Gprint, basis;\nGprint, orbital;\n"
					   "After a hf or multi calculation :  Printorb, 1000;\n"
					   "After a ci calculation :  NATORB, 2001.2 , PRINT=1000;\n"
					   "\n"),
						filename);
		Message(t,_("Error"),TRUE);
		g_free(t);
	}

} 
