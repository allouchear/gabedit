/* Basis.c */
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
#include "UtilsOrb.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Utils/Zlm.h"
#include "../Utils/GTF.h"

/********************************************************************************/
void save_basis_gabedit_format(FILE* file)
{
	gint i;
	gint j;
	gint k;
	gint c;
	fprintf(file,"[Basis]\n");
	if(!Type) return;
	for(c = 0;c<nCenters; c++)
	{
		i = GeomOrb[c].NumType;
		fprintf(file,"   %d 0\n", c+1);
 		for(j=0;j<Type[i].Norb;j++)
		{
			fprintf(file," %c %d 1.0\n",GetSymmetry(Type[i].Ao[j].L),Type[i].Ao[j].N);
			for(k=0;k<Type[i].Ao[j].N;k++)
				fprintf(file," %14.6f %14.6f\n",Type[i].Ao[j].Ex[k],Type[i].Ao[j].Coef[k]);
		}
		fprintf(file,"\n");
	}
	fprintf(file,"\n");
}
/**********************************************/
gint ReadCommandLines()
{
  gint taille=BSIZE;
  char *t = g_malloc(taille);

  if(forb == NULL)
	return 0;
  while(!feof(forb))
  {
    { char* e = fgets(t,taille,forb);}
   if(!strcmp(t," ") ||  !strcmp(t,"\n") )
        return 1; 
   if( t[0] != '#' )
   {
     Debug("\n\nERROR : the firsts lines is not a commands lines\n\n");
     return 0;
   }
  }
 return 0;
}
/**********************************************/
gint OpenDataFile(char * NameFile)
{
 forb = FOpen(NameFile, "rb");
 if(forb == NULL)
 {
   Debug("\n\nERROR Can not open %s data file\n\n",NameFile);
   return 0;
 }
 return 1;
}
/**********************************************/
void PrintBasis()
{
 gint i;
 gint j,k;
 Debug("\n\n");
 printLineChar('*',72);
 for(i=0;i<Ntype;i++)
 {
  Debug("Basis for center %s \n",Type[i].Symb);
  Debug("====================\n");
  Debug("Norb\t\t= %d\n",Type[i].Norb);
  Debug("\n");
 Debug("- --------- --------- \n");
 Debug("l     Ex       Coef   \n");
 Debug("- --------- --------- \n");
 for(j=0;j<Type[i].Norb;j++)
 {	k=0;
	Debug("%c %14.6f %14.6f\n",GetSymmetry(Type[i].Ao[j].L),Type[i].Ao[j].Ex[k],Type[i].Ao[j].Coef[k]);
	for(k=1;k<Type[i].Ao[j].N;k++)
		Debug("  %14.6f %14.6f\n",Type[i].Ao[j].Ex[k],Type[i].Ao[j].Coef[k]);
    
 }
 Debug("\n");
 /*
 if(Type[i].nps>0)
 {
	Debug("Pseudo potential for center %s\n",Type[i].Symb);
  	Debug("==============================\n\n");
 	Debug("-\t-  --------- ---------\n");
 	Debug("l\tn    Eps       Tau\n");
 	Debug("-\t-  --------- ---------\n");
 }
 else
	Debug("No Pseudo potential for %s\n",Type[i].Symb);
 for(j=0;j<Type[i].nps;j++)
 {
    Debug("%d\t%d %9.6f %9.6f ",Type[i].ps[j].l,Type[i].ps[j].n,Type[i].ps[j].eps,Type[i].ps[j].tau);
    Debug("\n");

 }
 */
 printLineChar('*',72);
 Debug("\n");
 }
}

/**********************************************/
gboolean ReadOneBasis(gint i,gint j,char *t,gint *nsym)
{
	gint k;
	gint n;
	gint taille = BSIZE;
	gchar *sym = g_malloc(10*sizeof(gchar));
	gint l=1;

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
    		{ char* e = fgets(t,taille,forb);}
		for(n=0;n<(gint)strlen(t);n++)
			if(t[n]=='D') t[n] = 'e';
		/*Debug("t de One = %s\n",t);*/
		   
		if(l==1) sscanf(t,"%lf %lf ",&Type[i].Ao[j].Ex[k],&Type[i].Ao[j].Coef[k]);
		else
		{
			sscanf(t,"%lf %lf %lf ",&Type[i].Ao[j].Ex[k],&Type[i].Ao[j].Coef[k],&Type[i].Ao[j+1].Coef[k]);
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
        default :	g_free(sym);
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

        default :	g_free(sym);
			g_free(Type[i].Ao[j+1].Ex);
			g_free(Type[i].Ao[j+1].Coef);
         		return FALSE;
     }
	/*Debug("end readone basis \n");*/
       g_free(sym);
       return TRUE;
}
/**********************************************/
gboolean DefineBasisType(gchar *fileName)
{
	gchar *sym;
	gchar *t;
	gchar *pdest;
	gint taille=BSIZE;
	gint i;
	gint j;
	gboolean ok;
	gint nsym;
	long int geompos =  0;

	/* Debug("debut de DefineBasisType\n");*/
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message("Sorry\n No file selected","Error",TRUE);
    		return FALSE;
 	}

 	t=g_malloc(taille*sizeof(gchar));
 	forb = FOpen(fileName, "rb");
 	if(forb == NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,"Sorry, I can not open '%s' file\n",fileName);
  		Message(buffer,"Error",TRUE);
  		return FALSE;
 	}
	ok = FALSE;
	while(!feof(forb))
	{
    		{ char* e = fgets(t,taille,forb);}
		pdest = strstr(t,"asis set"); /* Basis for g98 and basis for g03 */
		if(pdest != NULL)
		{
			ok = TRUE;
			geompos =  ftell(forb);
			/* break; NO for get the last basis */
		}
	}
	if(!ok)
	{
		gchar* t = g_strdup_printf("Sorry,\nI can not read atomic basis and/or molecular orbitals from\n \"%s\"\n file\n"
					   "\nFor display density and/or molecular orbitals from gaussian output file,\n"
					   "the following keywords are required:\n"
					   "#P  Gfinput  IOP(6/7=3)  Pop=full\n"
					   "\n",
					   fileName);
		Message(t,"Error",TRUE);
		g_free(t);
  		return FALSE;
	}
	fseek(forb, geompos, SEEK_SET);

	sym=g_malloc(10*sizeof(gchar));

	t=(gchar*)g_malloc(taille*sizeof(gchar));
 
	if(forb !=NULL)
	{
		/* Debug("Ntype = %d\n",Ntype);*/
		Type = g_malloc(Ntype*sizeof(TYPE));
		for(i=0;i<Ntype;i++)
			Type[i].Ao = NULL;
		while(!feof(forb))
		{
    			{ char* e = fgets(t,taille,forb);}
     			if(!strcmp(t,"\n") || !strcmp(t," ") )
     			{
				g_free(sym);
				g_free(t);
				for(i=0;i<Ntype;i++)
				if(Type[i].Ao == NULL)
				{
					gchar buffer[BSIZE];
					sprintf(buffer,"Sorry, I can not read '%s' file, problem with basis set\n",fileName);
  					Message(buffer,"Error",TRUE);
					return FALSE;
				}
       				return TRUE;
			}

			if(  strcmp(t,"\n")!= 0  && strcmp(t," ") && t[1] !='*')
			{
				sscanf(t,"%s",sym);
				sym[0]=toupper(sym[0]);
				if(strlen(sym)>1)
     				sym[1]=tolower(sym[1]);
				/*i=GetNumType(sym);*/
				i=atoi(sym)-1;
				if(i>-1)
				{
					sym = g_strdup( GeomOrb[i].Symb);
					i = GeomOrb[i].NumType;
     					Type[i].Symb=g_strdup(sym);
     					Type[i].N=GetNelectrons(sym);
     					j=-1;
					while(!feof(forb))
     					{
    						{ char* e = fgets(t,taille,forb);}
     						if(!strcmp(t,"\n") || t[1]=='*') break;
     						j++;
        					Type[i].Norb=j+1;
     						if(j == 0) Type[i].Ao=g_malloc(sizeof(AO));
     						else Type[i].Ao=g_realloc(Type[i].Ao,(j+1)*sizeof(AO));

     						ok= ReadOneBasis(i,j,t,&nsym);
        					if(nsym==2)
						{
							Type[i].Norb=j+2;
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
				else
				{
     					if(!strcmp(t,"\n") || !strcmp(t," ") )
     					{
     						g_free(sym);
       						g_free(t);
						for(i=0;i<Ntype;i++)
						if(Type[i].Ao == NULL)
						{
							gchar buffer[BSIZE];
							sprintf(buffer,"Sorry, I can not read '%s' file, problem with basis set\n",fileName);
  							Message(buffer,"Error",TRUE);
							return FALSE;
						}
       						return TRUE;
     					}
        				break;
				}
			}
		}
	}
	for(i=0;i<Ntype;i++)
	if(Type[i].Ao == NULL)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,"Sorry, I can not read '%s' file, problem with basis set\n",fileName);
  		Message(buffer,"Error",TRUE);
		return FALSE;
	}
	return TRUE;
}
/**********************************************/
static void resortAtoms(gint* numAtoms)
{
	TypeGeomOrb* newGeom = NULL;
	gint i;
	/* printf("begin resortAtoms\n  ");*/
	if(!numAtoms)return;
	if(nCenters<1)return;
	/*
	printf("Sorting  ");
	for(i=0;i<nCenters;i++) printf("%d ",numAtoms[i]);
	printf("\n");
	*/
	for(i=0;i<nCenters;i++) if(numAtoms[i] == -1) return;
	newGeom = g_malloc(nCenters*sizeof(TypeGeomOrb));
	for(i=0;i<nCenters;i++) newGeom[i] = GeomOrb[numAtoms[i]];
	for(i=0;i<nCenters;i++) GeomOrb[i] = newGeom[i];
	g_free(newGeom);
}
/**********************************************/
static gint getNumberOfBasisCenters(gchar *fileName, gchar* title)
{
	gchar t[BSIZE];
	gint i;
	gboolean ok;
	gint nAtoms = 0;

 	if ((!fileName) || (strcmp(fileName,"") == 0)) return nAtoms;

 	forb = FOpen(fileName, "rb");
 	if(forb == NULL) return nAtoms;
	ok = FALSE;
	while(!feof(forb))
	{
    		{ char* e = fgets(t,BSIZE,forb);}
		if(strstr(t,title) != NULL)
		{
			ok = TRUE;
			break;
		}
	}
	if(!ok) 
	{
		fclose(forb);
		return nAtoms;
	}

	nAtoms = 0;
	while(!feof(forb))
	{
    		{ char* e = fgets(t,BSIZE,forb);}
		if(this_is_a_backspace(t) || strstr(t,"[")) break;

		/* printf("tt = %s\n",t);*/
		i=atoi(t);
		if(i>0)
		{
			nAtoms++;
			while(!feof(forb))
     			{
    				{ char* e = fgets(t,BSIZE,forb);}
     				if(this_is_a_backspace(t) || strstr(t,"[")) break;
     			}
		}
		else
		{
     			if(this_is_a_backspace(t) || strstr(t,"[")) break;
		}
		if(strstr(t,"[")) break;
	}
	fclose(forb);
    	return nAtoms;
}
/**********************************************/
gboolean DefineGabeditMoldenBasisType(gchar *fileName,gchar* title)
{
	gchar *sym;
	gchar *t;
	gchar *pdest;
	gint taille=BSIZE;
	gint i;
	gint j;
	gboolean ok;
	gint nsym;
	gint* numAtoms = NULL;
	gint nAtoms = 0;
	gint nC = getNumberOfBasisCenters(fileName, title);

 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message("Sorry\n No file selected","Error",TRUE);
    		return FALSE;
 	}

 	t=g_malloc(taille*sizeof(gchar));
 	forb = FOpen(fileName, "rb");
 	if(forb == NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,"Sorry, I can not open '%s' file\n",fileName);
  		Message(buffer,"Error",TRUE);
  		return FALSE;
 	}
	ok = FALSE;
	while(!feof(forb))
	{
    		{ char* e = fgets(t,taille,forb);}
		pdest = strstr(t,title);
		if(pdest != NULL)
		{
			ok = TRUE;
			break;
		}
	}
	if(!ok)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,"Sorry\nI can not read basis from '%s' file\n",fileName);
  		Message(buffer,"Error",TRUE);
  		return FALSE;
	}

	sym=g_malloc(10*sizeof(char));
	/* printf("nC = %d\n",nC);*/
	/* basis available for all centers */
	if(nC==nCenters)
	{
		Ntype = nCenters;
 		for(i=0;i<nCenters;i++) GeomOrb[i].NumType= i;
	}

	if(forb !=NULL)
	{
		/* Debug("Ntype = %d\n",Ntype);*/
		numAtoms = g_malloc(nCenters*sizeof(gint));
		for(i=0;i<nCenters;i++) numAtoms[i] = -1;
		nAtoms = 0;
		Type = g_malloc(Ntype*sizeof(TYPE));
		for(i=0;i<Ntype;i++)
		{
			Type[i].Ao = NULL;
        		Type[i].Norb=0;
		}
		sprintf(t," ");
		while(!feof(forb))
		{
			if(strstr(t,"[")) break;
    			{ char* e = fgets(t,taille,forb);}
			/* Debug("tav = %s\n",t);*/
			if(this_is_a_backspace(t) || strstr(t,"["))
     			{
				g_free(sym);
				g_free(t);
				for(i=0;i<Ntype;i++)
				if(Type[i].Ao == NULL)
				{
					gchar buffer[BSIZE];
					sprintf(buffer,"Sorry, I can not read '%s' file, problem with basis set (1)\n",fileName);
  					Message(buffer,"Error",TRUE);
					printf("AO pour i = %d\n", i);
					if(numAtoms) g_free(numAtoms);
					return FALSE;
				}
				resortAtoms(numAtoms);
				if(numAtoms) g_free(numAtoms);
       				return TRUE;
			}

     			if(!this_is_a_backspace(t))
			{
				/*Debug("tap = %s\n",t);*/
				i=atoi(t)-1;
				if(i>-1 && i<nCenters) numAtoms[nAtoms] = i;
				nAtoms++;
				/*Debug("i1 = %d \n",i);*/
				if(i>-1)
				{
					sym = g_strdup( GeomOrb[i].Symb);
					i = GeomOrb[i].NumType;
					/* printf("numType = %d\n",i);*/
     					Type[i].Symb=g_strdup(sym);
     					Type[i].N=GetNelectrons(sym);
     					j=-1;
					while(!feof(forb))
     					{
    						{ char* e = fgets(t,taille,forb);}
						/* Debug("t = %s\n",t);*/
     						if(this_is_a_backspace(t) || strstr(t,"["))
				 		{
							/*Debug("This is a backspace\n");*/
        						break;
						}
						/*
						else
							Debug("This is not a backspace\n");
						*/
     						j++;
        					Type[i].Norb=j+1;
						/*
						Debug("debut Alloc %d %d \n",i,j);
						Debug("point %d \n",Type[i].Ao);
						*/
     						if(j == 0) Type[i].Ao=g_malloc(sizeof(AO));
     						else Type[i].Ao=g_realloc(Type[i].Ao,(j+1)*sizeof(AO));

						/*
						Debug("debut ReadOne i=%d j = %d \n",i,j);
						Debug("debut t = %s \n",t);
						*/
     						ok= ReadOneBasis(i,j,t,&nsym);
						/*Debug("nsym apres = %d\n",nsym);*/
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
				else
				{
    					/*Debug("else = %s\n",t);*/
     					if(this_is_a_backspace(t) || strstr(t,"["))
     					{
     						g_free(sym);
       						g_free(t);
						for(i=0;i<Ntype;i++)
						if(Type[i].Ao == NULL)
						{
							gchar buffer[BSIZE];
							sprintf(buffer,"Sorry, I can not read '%s' file, problem with basis set (2)\n",fileName);
  							Message(buffer,"Error",TRUE);
							if(numAtoms) g_free(numAtoms);
							return FALSE;
						}
						resortAtoms(numAtoms);
						if(numAtoms) g_free(numAtoms);
       						return TRUE;
     					}
        				break;
				}
			}
		}
	}
	for(i=0;i<Ntype;i++)
	if(Type[i].Ao == NULL)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,"Sorry, I can not read '%s' file, problem with basis set (3)\n",fileName);
  		Message(buffer,"Error",TRUE);
		if(numAtoms) g_free(numAtoms);
		return FALSE;
	}
	resortAtoms(numAtoms);
	if(numAtoms) g_free(numAtoms);
    	return TRUE;
}
/**********************************************/
gboolean DefineMoldenBasisType(gchar *fileName)
{
	/* Debug("begin of DefineMoldenBasisType\n");*/
	return DefineGabeditMoldenBasisType(fileName,"[GTO]");
}
/**********************************************/
gboolean DefineGabeditBasisType(gchar *fileName)
{
/*	Debug("begin of DefineGabeditBasisType\n");*/
	return DefineGabeditMoldenBasisType(fileName,"[Basis]");
}
/**********************************************/
void PrintAllBasis()
{
 gint k,j,n;
 gint l;
 char *XYZ[]={"x","y","z"};


 for(k=0;k<NAOrb;k++)
 {

	 for(n=0;n<AOrb[k].numberOfFunctions;n++)
	 {
		
		 l=0;
		 for(j=0;j<3;j++)
		   l += AOrb[k].Gtf[n].l[j];

		 Debug("%c",GetSymmetry(l));
		 for(j=0;j<3;j++)
		 {
			 switch(AOrb[k].Gtf[n].l[j])
			 {
			 case 0:break;
			 case 1:Debug("%s",XYZ[j]);break;
			 default :Debug("%s%d",XYZ[j],AOrb[k].Gtf[n].l[j]);
			 }
		 }
			Debug("\t%9.6f %9.6f \tC=%9.6f %9.6f %9.6f",AOrb[k].Gtf[n].Ex,AOrb[k].Gtf[n].Coef, AOrb[k].Gtf[n].C[0],AOrb[k].Gtf[n].C[1],AOrb[k].Gtf[n].C[2]);
			Debug("\n");
	 }
	 Debug("\n");
 }
}
/**********************************************/
void NormaliseAllBasis()
{
 gint k,n;

 for(k=0;k<NAOrb;k++)
	 for(n=0;n<AOrb[k].numberOfFunctions;n++)
		 normaliseRadialGTF(&AOrb[k].Gtf[n]);

 for(k=0;k<NAOrb;k++)
		 normaliseCGTF(&AOrb[k]);
}
/**********************************************/
void NormaliseAllNoRadBasis()
{
 gint k;

 for(k=0;k<NAOrb;k++)
		 normaliseCGTF(&AOrb[k]);
}
/**********************************************/
/**********************************************/
void DefineAtomicNumOrb()
{
	gint i;
	gint j;
	for(i=0;i<nCenters;i++)
	{
	/*	Debug("i= %d \n",i);*/
		GeomOrb[i].NAOrb = 0;
		GeomOrb[i].NAlphaOrb = 0;
		GeomOrb[i].NBetaOrb = 0;
		GeomOrb[i].NumOrb = NULL;
		GeomOrb[i].CoefAlphaOrbitals = NULL;
		GeomOrb[i].OccAlphaOrbitals = NULL;
		GeomOrb[i].EnerAlphaOrbitals = NULL;
		GeomOrb[i].SymAlphaOrbitals = NULL;
		GeomOrb[i].CoefBetaOrbitals = NULL;
		GeomOrb[i].EnerBetaOrbitals = NULL;
		GeomOrb[i].OccBetaOrbitals = NULL;
		GeomOrb[i].SymBetaOrbitals = NULL;
	}
	/* Debug("End Geom init \n");*/

	for(j=0;j<NAOrb;j++)
	{
		if(AOrb) i = AOrb[j].NumCenter;
		else if(SAOrb) i = SAOrb[j].NumCenter;
		GeomOrb[i].NAOrb++;
		if(!GeomOrb[i].NumOrb)
			GeomOrb[i].NumOrb = g_malloc(GeomOrb[i].NAOrb*sizeof(gint));
		else
			GeomOrb[i].NumOrb = g_realloc(GeomOrb[i].NumOrb,GeomOrb[i].NAOrb*sizeof(gint));
		GeomOrb[i].NumOrb[GeomOrb[i].NAOrb-1] = j;
	}
}
/**********************************************/
void DefineCartBasis()
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
		 l[0][m] = 1;l[1][m] = 2;l[2][m] = 0; /* XYY */
		 m++;
		 l[0][m] = 2;l[1][m] = 1;l[2][m] = 0; /* XXY */
		 m++;
		 l[0][m] = 2;l[1][m] = 0;l[2][m] = 1; /* XXZ */
		 m++;
		 l[0][m] = 1;l[1][m] = 0;l[2][m] = 2; /* XZZ */
		 m++;
		 l[0][m] = 0;l[1][m] = 1;l[2][m] = 2; /* YZZ */
		 m++;
		 l[0][m] = 0;l[1][m] = 2;l[2][m] = 1; /* YYZ */
		 m++;
		 l[0][m] = 1;l[1][m] = 1;l[2][m] = 1; /* XYZ */
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
/**********************************************/
void DefineSphericalBasis()
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
		/*Debug("L=%d \n",L);*/

		/*Debug("L =%d \n",L);*/
		if(L==1)
		{
                	klbeg = L;
                	klend = 0;
                	klinc = -1;
		}
		else
		{
                	klbeg = 0;
                	klend = L;
                	klinc = +1;
		}
		for(kl = klbeg;(klbeg == 0 && kl<=klend) || (klbeg == L && kl>=klend);kl +=klinc)
		{
		if(kl!=0)
		    inc = 2*kl;	
		else
		    inc = 1;
		for(M=kl;M>=-kl;M -=inc)
    		{
			/*Debug("L =%d kl=%d M=%d \n",L,kl,M);*/
	 		k++;
	 	   	Stemp =  getZlm(L,M);

	 		temp[k].numberOfFunctions=Stemp.numberOfCoefficients*Type[GeomOrb[i].NumType].Ao[j].N;
		    temp[k].NumCenter=i;
			/*Debug("M=%d N=%d\n",M,temp[k].N);*/
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
		if(L==0)
		  break;
	      }
		if(L==0)
		  break;
	      }
	}
	 for(i=0;i<NAOrb;i++)
		g_free(AOrb[i].Gtf);
g_free(AOrb);
NAOrb = NOrb;
AOrb = temp;
 if(SAOrb) g_free(SAOrb);
 SAOrb = NULL;
DefineAtomicNumOrb();
/* DefineNorb();*/
}
/***************************************************************/
static void getlTable(gint L, gint* nCoefs, gdouble** coefs, gint** l[3])
{
	gint m;
	gint l1,l2,l3;
	gint n;
	/* Spehrical D,F, ...*/
	if(L<-1)
	{
		gint klbeg=0, klend=-L, klinc=1;
		gint inc;
		gint kl;
		gint M;
		gint c;
 		Zlm Stemp;
		gint m = -1;
		L = abs(L);
		if(L==1)
		{
                	klbeg = L;
                	klend = 0;
                	klinc = -1;
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
				if(L==1) m = M+abs(L);
				else m++;
	 	   		Stemp =  getZlm(L,M);
				nCoefs[m] = Stemp.numberOfCoefficients;
	 			for(n=0;n<Stemp.numberOfCoefficients;n++)
	 			{
	   				coefs[m][n] = Stemp.lxyz[n].Coef;
	   				for(c=0;c<3;c++) l[c][m][n] = Stemp.lxyz[n].l[c];
	 			}
				if(L==0) break;
	      		}
			if(L==0) break;
	      }
	      return;
	}
	/* Cartesian S,P,D,F,..*/
	L = abs(L); /* for P */
	for(m=0;m<(L+1)*(L+2)/2;m++) 
	{
		nCoefs[m] = 1;
		coefs[m][0] = 1.0;
	}
	switch(L)
	{
	  case 0 :
		 m=0;
		 l[0][m][0] = 0;l[1][m][0] = 0;l[2][m][0] = 0;
	  	 break;
	  case 1 :
		 m=0;
		 l[0][m][0] = 1;l[1][m][0] = 0;l[2][m][0] = 0; /* X */
		 m++;
		 l[0][m][0] = 0;l[1][m][0] = 1;l[2][m][0] = 0; /* Y */
		 m++;
		 l[0][m][0] = 0;l[1][m][0] = 0;l[2][m][0] = 1; /* Z */
	  	 break;
	  case 2 :
		 m=0;
		 l[0][m][0] = 2;l[1][m][0] = 0;l[2][m][0] = 0; /* XX */
		 m++;
		 l[0][m][0] = 0;l[1][m][0] = 2;l[2][m][0] = 0; /* YY */
		 m++;
		 l[0][m][0] = 0;l[1][m][0] = 0;l[2][m][0] = 2; /* ZZ */
		 m++;
		 l[0][m][0] = 1;l[1][m][0] = 1;l[2][m][0] = 0; /* XY */
		 m++;
		 l[0][m][0] = 1;l[1][m][0] = 0;l[2][m][0] = 1; /* XZ */
		 m++;
		 l[0][m][0] = 0;l[1][m][0] = 1;l[2][m][0] = 1; /* YZ */
		 break;
	  case 3 :
		 m=0;
		 l[0][m][0] = 3;l[1][m][0] = 0;l[2][m][0] = 0; /* XXX */
		 m++;
		 l[0][m][0] = 0;l[1][m][0] = 3;l[2][m][0] = 0; /* YYY */
		 m++;
		 l[0][m][0] = 0;l[1][m][0] = 0;l[2][m][0] = 3; /* ZZZ */
		 m++;
		 l[0][m][0] = 1;l[1][m][0] = 2;l[2][m][0] = 0; /* XYY */
		 m++;
		 l[0][m][0] = 2;l[1][m][0] = 1;l[2][m][0] = 0; /* XXY */
		 m++;
		 l[0][m][0] = 2;l[1][m][0] = 0;l[2][m][0] = 1; /* XXZ */
		 m++;
		 l[0][m][0] = 1;l[1][m][0] = 0;l[2][m][0] = 2; /* XZZ */
		 m++;
		 l[0][m][0] = 0;l[1][m][0] = 1;l[2][m][0] = 2; /* YZZ */
		 m++;
		 l[0][m][0] = 0;l[1][m][0] = 2;l[2][m][0] = 1; /* YYZ */
		 m++;
		 l[0][m][0] = 1;l[1][m][0] = 1;l[2][m][0] = 1; /* XYZ */
		 break;
	  default :
		m=0;
		for(l3=abs(L);l3>=0;l3--)
		for(l2=abs(L)-l3;l2>=0;l2--)
		{
	 		l1 = abs(L)-l2-l3;
			l[0][m][0] = l1;
			l[1][m][0] = l2;
			l[2][m][0] = l3;
			m++;
		}
	}
}
/********************************************************************************/
gboolean readBasisFromGaussianFChk(gchar *fileName)
{
 	FILE *file;
	gint n;
	gint nS;
	gint c;
	gint nShells = 0;
	gint nPrimitives = 0;
	gint lMax = 0;
	gint contMax = 0;
	gint* shellTypes = NULL;
	gint* nPrimitivesByShell = NULL;
	gint* numAtoms = NULL;
	gdouble* primitiveExponents = NULL;
	gdouble* contractionsCoefs = NULL;
	gdouble* contractionsCoefsSP = NULL;
	gdouble* coordinatesForShells = NULL;
	gint** l[3] = {NULL,NULL,NULL};
	gdouble** coefs = NULL;
	gint* nCoefs = NULL;
	CGTF *temp = NULL;
	gint kOrb, kPrimitive;
	gint m;
	gint nBasis;
	gboolean sp = FALSE;
	gint llMax = 0;

	file = FOpen(fileName, "rb");
	if(file ==NULL)
	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  		return FALSE;
	}

	nBasis = get_one_int_from_fchk_gaussian_file(file,"Number of basis functions  ");
	if(nBasis<1)
	{
  		Message(_("Sorry\nI can not read the number of basis functions"),_("Error"),TRUE);
		fclose(file);
  		return FALSE;
	}

	nShells = get_one_int_from_fchk_gaussian_file(file,"Number of contracted shells ");
	if(nShells<1)
	{
  		Message(_("Sorry\nI can not the number of contracted shells"),_("Error"),TRUE);
		fclose(file);
  		return FALSE;
	}
	nPrimitives = get_one_int_from_fchk_gaussian_file(file,"Number of primitive shells ");
	if(nPrimitives<1)
	{
  		Message(_("Sorry\nI can not the number of primitive shells"),_("Error"),TRUE);
		fclose(file);
  		return FALSE;
	}
	rewind(file);
	lMax = get_one_int_from_fchk_gaussian_file(file,"Highest angular momentum ");
	if(lMax<0)
	{
  		Message(_("Sorry\nI can not the value of the highest angular momentum"),_("Error"),TRUE);
		fclose(file);
  		return FALSE;
	}
	rewind(file);
	contMax = get_one_int_from_fchk_gaussian_file(file,"Largest degree of contraction ");
	if(contMax<1)
	{
  		Message(_("Sorry\nI can not the value of the largest degree of contraction"),_("Error"),TRUE);
		fclose(file);
  		return FALSE;
	}
	rewind(file);
	shellTypes = get_array_int_from_fchk_gaussian_file(file, "Shell types ", &n);
	if(!shellTypes || n!=nShells)
	{
  		Message(_("Sorry\nI can not read the shell types"),_("Error"),TRUE);
		if(shellTypes) g_free(shellTypes);
		fclose(file);
  		return FALSE;
	}
	for(nS = 0;nS<nShells;nS++) if( shellTypes[nS]==-1) { sp = TRUE; break;}

	NOrb = 0;
	for(nS=0;nS<nShells;nS++) 
	{
		if(shellTypes[nS]<-1) NOrb += 2*abs(shellTypes[nS])+1; /* Sperical D, F, G, ...*/
		else if(shellTypes[nS]==-1) NOrb +=  4; /* This a SP.*/
		else NOrb +=  (shellTypes[nS]+1)*(shellTypes[nS]+2)/2; /* Cartezian S,P,D,F,G,..*/
	}
	if(NOrb != nBasis)
	{
  		Message(_("Sorry\nThe number of basis function in fch file is not equal to that computed by Gabedit!"),_("Error"),TRUE);
		if(shellTypes) g_free(shellTypes);
		fclose(file);
		return FALSE;
	}
	nPrimitivesByShell = get_array_int_from_fchk_gaussian_file(file, "Number of primitives per shell ", &n);
	if(!nPrimitivesByShell || n!=nShells)
	{
  		Message(_("Sorry\nI can not read the number of primitives per shell"),_("Error"),TRUE);
		if(nPrimitivesByShell) g_free(nPrimitivesByShell);
		fclose(file);
  		return FALSE;
	}
	numAtoms = get_array_int_from_fchk_gaussian_file(file, "Shell to atom map ", &n);
	if(!numAtoms || n!=nShells)
	{
  		Message(_("Sorry\nI can not read the atoms number for shell"),_("Error"),TRUE);
		if(numAtoms) g_free(numAtoms);
		fclose(file);
  		return FALSE;
	}
	primitiveExponents = get_array_real_from_fchk_gaussian_file(file, "Primitive exponents ", &n);
	if(!primitiveExponents || n != nPrimitives)
	{
  		Message(_("Sorry\nI can not read the primitive exponents "),_("Error"),TRUE);
		if(primitiveExponents) g_free(primitiveExponents);
		fclose(file);
  		return FALSE;
	}
	contractionsCoefs = get_array_real_from_fchk_gaussian_file(file, "Contraction coefficients ", &n);
	if(!contractionsCoefs || n != nPrimitives)
	{
  		Message(_("Sorry\nI can not read the contraction coefficients "),_("Error"),TRUE);
		if(contractionsCoefs) g_free(contractionsCoefs);
		fclose(file);
  		return FALSE;
	}
	if(sp)
	{
		contractionsCoefsSP = get_array_real_from_fchk_gaussian_file(file, "P(S=P) Contraction coefficients ", &n);
		if(!contractionsCoefsSP || n != nPrimitives)
		{
  			Message(_("Sorry\nI can not read the P(S=P) contraction coefficients "),_("Error"),TRUE);
			if(contractionsCoefsSP) g_free(contractionsCoefsSP);
			fclose(file);
  			return FALSE;
		}
	}
	coordinatesForShells = get_array_real_from_fchk_gaussian_file(file, "Coordinates of each shell ", &n);
	if(!contractionsCoefs || n != nShells*3)
	{
  		Message(_("Sorry\nI can not read the coordinates of each shell "),_("Error"),TRUE);
		if(coordinatesForShells) g_free(coordinatesForShells);
		fclose(file);
  		return FALSE;
	}
	fclose(file);
	/* printf("close file\n");*/

	llMax = (lMax+1)*(lMax+2)/2;
	nCoefs = g_malloc(llMax*sizeof(gint));
	coefs = g_malloc(llMax*sizeof(gdouble*));
	for(m=0;m<llMax;m++) coefs[m] = g_malloc(llMax*sizeof(gdouble));

	for(c=0;c<3;c++) 
	{
		l[c] = g_malloc(llMax*sizeof(gint*));
		for(m=0;m<llMax;m++) l[c][m] = g_malloc(llMax*sizeof(gint));
	}

	temp  = g_malloc(NOrb*sizeof(CGTF));
	kOrb = 0;
	kPrimitive = 0;
	for(nS = 0;nS<nShells; nS++)
	{
		gint nM = 0;
		/* printf("begin primitive nS = %d\n",nS);*/
		if(shellTypes[nS]<-1) nM = 2*abs(shellTypes[nS])+1; /* Sperical D, F, G, ...*/
		else if(shellTypes[nS]==-1) nM = 1; /* This a SP. Make S before */
		else nM = (shellTypes[nS]+1)*(shellTypes[nS]+2)/2;

		/* printf("nM = %d\n",nM);*/
		if(shellTypes[nS]==-1) getlTable(0, nCoefs, coefs, l); /* This a SP. Make S before */
		else getlTable(shellTypes[nS], nCoefs, coefs, l); 
		/* printf("end getlTable\n");*/
		for(m=0;m<nM;m++)
		{
			gint ip,j,n;
	 		temp[kOrb].numberOfFunctions=nCoefs[m]*nPrimitivesByShell[nS];
			temp[kOrb].NumCenter=numAtoms[nS]-1;
			/* printf("m = %d nCoef = %d nPrim = %d\n",m,nCoefs[m],nPrimitivesByShell[nS]);*/
			/*Debug("M=%d N=%d\n",M,temp[k].N);*/
	 		temp[kOrb].Gtf =g_malloc(temp[kOrb].numberOfFunctions*sizeof(GTF));
          		j = -1;
	 		for(ip=0;ip<nPrimitivesByShell[nS];ip++)
 			for(n=0;n<nCoefs[m];n++)
	 		{
	 		   	j++;
	   			temp[kOrb].Gtf[j].Ex   = primitiveExponents[kPrimitive+ip];
	   			temp[kOrb].Gtf[j].Coef = contractionsCoefs[kPrimitive+ip]*coefs[m][n];
	   			for(c=0;c<3;c++)
	   			{
	   				temp[kOrb].Gtf[j].C[c] = coordinatesForShells[c+nS*3];
	   				temp[kOrb].Gtf[j].l[c] = l[c][m][n];
	   			}
	 		}
			kOrb++;
		}
		if(shellTypes[nS]==-1) /* This a SP. Now make P*/
		{
			getlTable(-1, nCoefs, coefs, l);
			nM = 3;
			for(m=0;m<nM;m++)
			{
				gint ip,j,n;
				/* printf("P : m = %d nCoef = %d nPrim = %d\n",m,nCoefs[m],nPrimitivesByShell[nS]);*/
	 			temp[kOrb].numberOfFunctions=nCoefs[m]*nPrimitivesByShell[nS];
				temp[kOrb].NumCenter=numAtoms[nS]-1;
	 			temp[kOrb].Gtf =g_malloc(temp[kOrb].numberOfFunctions*sizeof(GTF));
          			j = -1;
	 			for(ip=0;ip<nPrimitivesByShell[nS];ip++)
 				for(n=0;n<nCoefs[m];n++)
	 			{
	 		   		j++;
	   				temp[kOrb].Gtf[j].Ex   = primitiveExponents[kPrimitive+ip];
	   				temp[kOrb].Gtf[j].Coef = contractionsCoefsSP[kPrimitive+ip]*coefs[m][n];
	   				for(c=0;c<3;c++)
	   				{
	   					temp[kOrb].Gtf[j].C[c] = coordinatesForShells[c+nS*3];
	   					temp[kOrb].Gtf[j].l[c] = l[c][m][n];
	   				}
	 			}
				kOrb++;
			}
		}
		/* printf("end primitive nS = %d\n",nS);*/
		kPrimitive += nPrimitivesByShell[nS];
	}
	if(nCoefs) g_free(nCoefs);
	if(coefs)
	{
		for(m=0;m<llMax;m++) if(coefs[m]) g_free(coefs[m]);
		g_free(coefs);
	}
	for(c=0;c<3;c++) 
	if(l[c]) 
	{
		for(m=0;m<llMax;m++) if(l[c][m]) g_free(l[c][m]);
		g_free(l[c]);
	}
	if(shellTypes) g_free(shellTypes);
	if(nPrimitivesByShell) g_free(nPrimitivesByShell);
	if(numAtoms) g_free(numAtoms);
	if(primitiveExponents) g_free(primitiveExponents);
	if(contractionsCoefs) g_free(contractionsCoefs);
	if(coordinatesForShells) g_free(coordinatesForShells);
	for(kOrb=0;kOrb<NAOrb;kOrb++) g_free(AOrb[kOrb].Gtf);
	if(AOrb) g_free(AOrb);
	NAOrb = NOrb;
	AOrb = temp;
	if(SAOrb) g_free(SAOrb);
	SAOrb = NULL;
	DefineAtomicNumOrb();
	/* printf("End all read Basis\n");*/
	return TRUE;
}
