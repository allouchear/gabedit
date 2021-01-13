/* AtomicOrbitals.c */
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
#include "StatusOrb.h"

/********************************************************************************/
void save_ao_orbitals_gabedit_format(FILE* file)
{
	gint i;
	gint j;
	gint k;
	fprintf(file,"[AO]\n");
	for(i=0;i<nCenters;i++)
	for(j=0;j<GeomOrb[i].NAlphaOrb;j++)
	{
		fprintf(file," Ene= %lf\n",GeomOrb[i].EnerAlphaOrbitals[j]);
		fprintf(file," Spin= Alpha\n");
		fprintf(file," Occup= %lf\n",GeomOrb[i].OccAlphaOrbitals[j]);
		fprintf(file," Atom= %s\n",GeomOrb[i].Symb);
		for(k=0;k<GeomOrb[i].NAOrb;k++)
			fprintf(file,"     %d    %lf\n",k+1, GeomOrb[i].CoefAlphaOrbitals[j][k]);
	}
	for(i=0;i<nCenters;i++)
	for(j=0;j<GeomOrb[i].NBetaOrb;j++)
	{
		fprintf(file," Ene= %lf\n",GeomOrb[i].EnerBetaOrbitals[j]);
		fprintf(file," Spin= Beta\n");
		fprintf(file," Occup= %lf\n",GeomOrb[i].OccBetaOrbitals[j]);
		fprintf(file," Atom= %s\n",GeomOrb[i].Symb);
		for(k=0;k<GeomOrb[i].NAOrb;k++)
			fprintf(file,"     %d    %lf\n",k+1, GeomOrb[i].CoefBetaOrbitals[j][k]);
	}
	fprintf(file,"\n");
}
/********************************************************************************/
void print_atomic_orbitals()
{
	gint k;
	gint j;

	for(j=0;j<nCenters;j++)
	{
		Debug("Center %d \n",j);

		Debug("NAlphaOrb = %d , NBetaOrb = %d \n",
			GeomOrb[j].NAlphaOrb,GeomOrb[j].NBetaOrb);

		Debug(" Alpha Occ = ");
		for(k = 0;k<GeomOrb[j].NAlphaOrb;k++)
			Debug(" %0.6f ",GeomOrb[j].OccAlphaOrbitals[k]);
		Debug("\n");
		Debug(" Beta  Occ = ");
		for(k = 0;k<GeomOrb[j].NBetaOrb;k++)
			Debug(" %0.6f ",GeomOrb[j].OccBetaOrbitals[k]);
		Debug("\n");
	}
}
/********************************************************************************/
void free_one_atomic_orbitals(gint AtNOrb, gint AtNAlphaOrb, gint AtNBetaOrb,
	gdouble* AtEnerAlphaOrbitals,gdouble* AtOccAlphaOrbitals,gchar** AtSymAlphaOrbitals,gdouble**AtCoefAlphaOrbitals,
	gdouble* AtEnerBetaOrbitals,gdouble* AtOccBetaOrbitals,gchar** AtSymBetaOrbitals,gdouble**AtCoefBetaOrbitals)
{
	gint i;
	if(AtEnerAlphaOrbitals == AtEnerBetaOrbitals)
	{
		if(AtEnerAlphaOrbitals)
			g_free(AtEnerAlphaOrbitals);
		AtEnerAlphaOrbitals = NULL;
		AtEnerBetaOrbitals = NULL;
	}
	else
	{
		if(AtEnerAlphaOrbitals)
			g_free(AtEnerAlphaOrbitals);
		AtEnerAlphaOrbitals = NULL;
		if(AtEnerBetaOrbitals)
			g_free(AtEnerBetaOrbitals);
		AtEnerBetaOrbitals = NULL;
	}
	if(AtOccAlphaOrbitals == AtOccBetaOrbitals)
	{
		if(AtOccAlphaOrbitals)
			g_free(AtOccAlphaOrbitals);
		AtOccAlphaOrbitals = NULL;
		AtOccBetaOrbitals = NULL;
	}
	else
	{
		if(AtOccAlphaOrbitals)
			g_free(AtOccAlphaOrbitals);
		AtOccAlphaOrbitals = NULL;
		if(AtOccBetaOrbitals)
			g_free(AtOccBetaOrbitals);
		AtOccBetaOrbitals = NULL;

	}
	if(AtSymAlphaOrbitals == AtSymBetaOrbitals)
	{
		if(AtSymAlphaOrbitals)
		{
			for(i=0;i<AtNAlphaOrb;i++)
				if(AtSymAlphaOrbitals[i])
					g_free(AtSymAlphaOrbitals[i]);
			g_free(AtSymAlphaOrbitals);
		}
		AtSymAlphaOrbitals = NULL;
		AtSymBetaOrbitals = NULL;
	}
	else
	{
		if(AtSymAlphaOrbitals)
		{
			for(i=0;i<AtNAlphaOrb;i++)
				if(AtSymAlphaOrbitals[i])
					g_free(AtSymAlphaOrbitals[i]);
			g_free(AtSymAlphaOrbitals);
		}
		AtSymAlphaOrbitals = NULL;
		if(AtSymBetaOrbitals)
		{
			for(i=0;i<AtNBetaOrb;i++)
				if(AtSymBetaOrbitals[i])
					g_free(AtSymBetaOrbitals[i]);
			g_free(AtSymBetaOrbitals);
		}
		AtSymBetaOrbitals = NULL;
	}

	if(AtCoefAlphaOrbitals == AtCoefBetaOrbitals)
	{
		if(AtCoefAlphaOrbitals)
		{
			for(i=0;i<AtNOrb;i++)
				if(AtCoefAlphaOrbitals[i])
					g_free(AtCoefAlphaOrbitals[i]);
			g_free(AtCoefAlphaOrbitals);
		}
		AtCoefAlphaOrbitals  = NULL;
		AtCoefBetaOrbitals  = NULL;
	}
	else
	{
		if(AtCoefAlphaOrbitals)
		{
			for(i=0;i<AtNOrb;i++)
				if(AtCoefAlphaOrbitals[i])
					g_free(AtCoefAlphaOrbitals[i]);
			g_free(AtCoefAlphaOrbitals);
		}
		AtCoefAlphaOrbitals  = NULL;
		if(AtCoefBetaOrbitals)
		{
			for(i=0;i<AtNOrb;i++)
				if(AtCoefBetaOrbitals[i])
					g_free(AtCoefBetaOrbitals[i]);
			g_free(AtCoefBetaOrbitals);
		}
		AtCoefBetaOrbitals = NULL;
	}

}
/************************************************************/
void init_atomic_orbitals()
{
	gint i;

	AOAvailable = FALSE;
	for(i=0;i<nCenters;i++)
	{
		GeomOrb[i].NAOrb = 0;
		GeomOrb[i].NAlphaOrb = 0;
		GeomOrb[i].NBetaOrb = 0;
		GeomOrb[i].CoefAlphaOrbitals = NULL;
		GeomOrb[i].OccAlphaOrbitals = NULL;
		GeomOrb[i].EnerAlphaOrbitals = NULL;
		GeomOrb[i].SymAlphaOrbitals = NULL;
		GeomOrb[i].CoefBetaOrbitals = NULL;
		GeomOrb[i].EnerBetaOrbitals = NULL;
		GeomOrb[i].OccBetaOrbitals = NULL;
		GeomOrb[i].SymBetaOrbitals = NULL;
	}

}

/********************************************************************************/
void free_atomic_orbitals()
{
	gint AtNOrb;
	gint AtNAlphaOrb;
	gint AtNBetaOrb;
	gdouble* AtEnerAlphaOrbitals;
	gdouble* AtOccAlphaOrbitals;
	gchar** AtSymAlphaOrbitals;
	gdouble**AtCoefAlphaOrbitals;
	gdouble* AtEnerBetaOrbitals;
	gdouble* AtOccBetaOrbitals;
	gchar** AtSymBetaOrbitals;
	gdouble**AtCoefBetaOrbitals;
	gint i;

	for(i=0;i<nCenters;i++)
	{
		AtNOrb = GeomOrb[i].NAOrb;
		AtNAlphaOrb = GeomOrb[i].NAlphaOrb;
		AtNBetaOrb = GeomOrb[i].NBetaOrb;
		AtEnerAlphaOrbitals = GeomOrb[i].EnerAlphaOrbitals;
		AtOccAlphaOrbitals  = GeomOrb[i].OccAlphaOrbitals;
		AtSymAlphaOrbitals  = GeomOrb[i].SymAlphaOrbitals;
		AtCoefAlphaOrbitals = GeomOrb[i].CoefAlphaOrbitals;
		AtEnerBetaOrbitals = GeomOrb[i].EnerBetaOrbitals;
		AtOccBetaOrbitals  = GeomOrb[i].OccBetaOrbitals;
		AtSymBetaOrbitals  = GeomOrb[i].SymBetaOrbitals;
		AtCoefBetaOrbitals = GeomOrb[i].CoefBetaOrbitals;

		free_one_atomic_orbitals(AtNOrb,AtNAlphaOrb,AtNBetaOrb,
		AtEnerAlphaOrbitals,AtOccAlphaOrbitals,AtSymAlphaOrbitals,AtCoefAlphaOrbitals,
		AtEnerBetaOrbitals,AtOccBetaOrbitals,AtSymBetaOrbitals,AtCoefBetaOrbitals);
	}

	init_atomic_orbitals();
	set_status_label_info(_("At. Orb."),_("Nothing"));

}
/********************************************************************************/
gboolean read_atomic_orbitals_in_gabedit(gchar *fileName,gint itype)
{
 	gchar *t;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	FILE *fd;
 	gint taille=BSIZE;
 	gint i;
 	gint j=0;
 	gint numorb;
 	gchar *pdest;
	gint n = 0;
	gint k;
	gdouble **CoefOrbitals;
	gdouble *EnerOrbitals;
	gdouble *OccOrbitals;
	gchar **SymOrbitals;
	gchar **AtSymbOrbitals;
	gint NOcc = 0;
	gint idump;

	
	/*printf("debut de orbital\n");*/
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    	return FALSE;
 	}

 	t=g_malloc(taille);
 	fd = FOpen(fileName, "rb");
 	if(fd ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
		g_free(t);
  		return FALSE;
 	}

 	for(i=0;i<5;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(char));
  
	CoefOrbitals = CreateTable2(NOrb);
	EnerOrbitals = g_malloc(NOrb*sizeof(gdouble));
	OccOrbitals = g_malloc(NOrb*sizeof(gdouble));
	SymOrbitals = g_malloc(NOrb*sizeof(gchar*));
	AtSymbOrbitals = g_malloc(NOrb*sizeof(gchar*));

 	numorb =1;
 	do 
 	{
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
          	pdest = strstr( t, "[AO]" );
	 		if ( pdest != NULL )
	  		{
                numorb++;
                OK = TRUE;
	  			break;
	  		}
        }
 		if(!OK && (numorb == 1) )
		{
			FreeTable2(CoefOrbitals,NOrb);
			g_free(EnerOrbitals);
			g_free(SymOrbitals);
			g_free(AtSymbOrbitals);
			return FALSE;
    	}
 		if(!OK)
		{
			goto end;
    	}

		n = -1;
    		{ char* e = fgets(t,taille,fd);}
		while(!feof(fd))
		{
			gdouble e =0.0;
			gdouble o = 0.0;
			gchar* begin = NULL;
			gchar* spin = NULL;
			gchar sym[BSIZE];
			gchar symb[BSIZE];
			gboolean begincoef = FALSE;
			
			sym[0] = '\0';
			symb[0] = '\0';
			while(!feof(fd) && !begincoef)
			{
				/* Debug("t= %s atoi = %d\n",t,atoi(t)); */
				if( this_is_a_backspace(t))
				{
					begincoef = FALSE;
					break;
				}
				if(atoi(t) != 0)
				{
					begincoef = TRUE;
					break;
				}
				begin = strstr(t,"=")+1;
				if(strstr(t,"Ene")!= 0)
					e = atof(begin);
			
				if( strstr(t,"Occ") != 0)
					o = atof(begin);

				if( strstr(t,"Spin") != 0)
					spin = g_strdup(begin);

				if( strstr(t,"Sym") != 0)
					sscanf(begin,"%s",sym);
				if( strstr(t,"Atom") != 0)
					sscanf(begin,"%s",symb);
    				{ char* e = fgets(t,taille,fd);}
				
			}
			if(!begincoef || feof(fd))
				break;
			if(!spin)
			{
    				{ char* e = fgets(t,taille,fd);}
				continue;
			}
			if(strstr(spin,"Alpha") && itype == 2)
			{
    				{ char* e = fgets(t,taille,fd);}
				continue;
			}
			if(strstr(spin,"Beta") && itype == 1)
			{
    				{ char* e = fgets(t,taille,fd);}
				continue;
			}

			n++;
			EnerOrbitals[n] = e;
			if(sym[0] != '\0')
				SymOrbitals[n] = g_strdup(sym);
			else
				SymOrbitals[n] = g_strdup("Unknown");

			if(symb[0] != '\0')
				AtSymbOrbitals[n] = g_strdup(symb);
			else
				AtSymbOrbitals[n] = g_strdup("Unknown");

			OccOrbitals[n] = o;

			if(o>0)
				NOcc++;

			i = 0;
			/* Debug("atof t  = %d\n",atoi(t));*/ 
			while(!feof(fd) && atoi(t)!=0)
			{
				sscanf(t,"%d %lf",&idump,&CoefOrbitals[n][i]);
    				{ char* e = fgets(t,taille,fd);}
				i++;
			}
			/* Debug("i  = %d\n",i);*/
			if(n == NOrb-1)
				OK = FALSE;
		}
		
 	}while(!feof(fd));

end:
	/*Debug("End of read n = %d\n",n);*/
 	fclose(fd);
 	g_free(t);
	for(i=0;i<5;i++)
		g_free(AtomCoord[i]);

	switch(itype)
	{
		case 1 : 
			for(j=0;j<nCenters;j++)
			{
				/* Debug("Center n %d NAOrb = %d \n",j,GeomOrb[j].NAOrb);*/
				GeomOrb[j].NAlphaOrb = 0;
				GeomOrb[j].EnerAlphaOrbitals = g_malloc(GeomOrb[j].NAOrb*sizeof(gdouble));
				GeomOrb[j].OccAlphaOrbitals  = g_malloc(GeomOrb[j].NAOrb*sizeof(gdouble));
				GeomOrb[j].SymAlphaOrbitals  = g_malloc(GeomOrb[j].NAOrb*sizeof(gchar*));
				GeomOrb[j].CoefAlphaOrbitals = g_malloc(GeomOrb[j].NAOrb*sizeof(gdouble*));
				for(k=0;k<GeomOrb[j].NAOrb;k++)
					GeomOrb[j].CoefAlphaOrbitals[k] = g_malloc(GeomOrb[j].NAOrb*sizeof(gdouble));
				/*Debug("End Allocation\n");*/
				for(i=0;i<=n;i++)
				{
					if(strstr(GeomOrb[j].Symb,AtSymbOrbitals[i])==0)
						continue;
					/* Debug("Center n %d NA = %d i = %d \n",j,GeomOrb[j].NAlphaOrb,i);*/
					GeomOrb[j].EnerAlphaOrbitals[GeomOrb[j].NAlphaOrb] = EnerOrbitals[i];
					GeomOrb[j].OccAlphaOrbitals[GeomOrb[j].NAlphaOrb]  = OccOrbitals[i];
					GeomOrb[j].SymAlphaOrbitals[GeomOrb[j].NAlphaOrb]  = g_strdup(SymOrbitals[i]);
					for(k=0;k<GeomOrb[j].NAOrb;k++)
						GeomOrb[j].CoefAlphaOrbitals[GeomOrb[j].NAlphaOrb][k] = CoefOrbitals[i][k];
					GeomOrb[j].NAlphaOrb++;
				}
			}
			break;
		case 2 : 
			for(j=0;j<nCenters;j++)
			{
				GeomOrb[j].NBetaOrb = 0;
				GeomOrb[j].EnerBetaOrbitals = g_malloc(GeomOrb[j].NAOrb*sizeof(gdouble));
				GeomOrb[j].OccBetaOrbitals  = g_malloc(GeomOrb[j].NAOrb*sizeof(gdouble));
				GeomOrb[j].SymBetaOrbitals  = g_malloc(GeomOrb[j].NAOrb*sizeof(gchar*));
				GeomOrb[j].CoefBetaOrbitals = g_malloc(GeomOrb[j].NAOrb*sizeof(gdouble*));
				for(k=0;k<GeomOrb[j].NAOrb;k++)
					GeomOrb[j].CoefBetaOrbitals[k] = g_malloc(GeomOrb[j].NAOrb*sizeof(gdouble));
				
				for(i=0;i<=n;i++)
				{
					if(strstr(GeomOrb[j].Symb,AtSymbOrbitals[i])==0)
						continue;
					GeomOrb[j].EnerBetaOrbitals[GeomOrb[j].NBetaOrb] = EnerOrbitals[i];
					GeomOrb[j].OccBetaOrbitals[GeomOrb[j].NBetaOrb]  = OccOrbitals[i];
					GeomOrb[j].SymBetaOrbitals[GeomOrb[j].NBetaOrb]  = g_strdup(SymOrbitals[i]);
					for(k=0;k<GeomOrb[j].NAOrb;k++)
						GeomOrb[j].CoefBetaOrbitals[GeomOrb[j].NBetaOrb][k] = CoefOrbitals[i][k];
					GeomOrb[j].NBetaOrb++;
				}
			}
			break;
		case 3 : 
			for(j=0;j<nCenters;j++)
			{
				GeomOrb[j].NAlphaOrb = 0;
				GeomOrb[j].EnerAlphaOrbitals = g_malloc(GeomOrb[j].NAOrb*sizeof(gdouble));
				GeomOrb[j].OccAlphaOrbitals  = g_malloc(GeomOrb[j].NAOrb*sizeof(gdouble));
				GeomOrb[j].SymAlphaOrbitals  = g_malloc(GeomOrb[j].NAOrb*sizeof(gchar*));
				GeomOrb[j].CoefAlphaOrbitals = g_malloc(GeomOrb[j].NAOrb*sizeof(gdouble*));
				for(k=0;k<GeomOrb[j].NAOrb;k++)
					GeomOrb[j].CoefAlphaOrbitals[k] = g_malloc(GeomOrb[j].NAOrb*sizeof(gdouble));
				
				for(i=0;i<=n;i++)
				{
					if(strstr(GeomOrb[j].Symb,AtSymbOrbitals[i])==0)
						continue;
					GeomOrb[j].EnerAlphaOrbitals[GeomOrb[j].NAlphaOrb] = EnerOrbitals[i];
					GeomOrb[j].OccAlphaOrbitals[GeomOrb[j].NAlphaOrb]  = OccOrbitals[i];
					GeomOrb[j].SymAlphaOrbitals[GeomOrb[j].NAlphaOrb]  = g_strdup(SymOrbitals[i]);
					for(k=0;k<GeomOrb[j].NAOrb;k++)
						GeomOrb[j].CoefAlphaOrbitals[GeomOrb[j].NAlphaOrb][k] = CoefOrbitals[i][k];
					GeomOrb[j].NAlphaOrb++;
				}
			}
			for(j=0;j<nCenters;j++)
			{
				GeomOrb[j].NBetaOrb = 0;
				GeomOrb[j].EnerBetaOrbitals = g_malloc(GeomOrb[j].NAOrb*sizeof(gdouble));
				GeomOrb[j].OccBetaOrbitals  = g_malloc(GeomOrb[j].NAOrb*sizeof(gdouble));
				GeomOrb[j].SymBetaOrbitals  = g_malloc(GeomOrb[j].NAOrb*sizeof(gchar*));
				GeomOrb[j].CoefBetaOrbitals = g_malloc(GeomOrb[j].NAOrb*sizeof(gdouble*));
				for(k=0;k<GeomOrb[j].NAOrb;k++)
					GeomOrb[j].CoefBetaOrbitals[k] = g_malloc(GeomOrb[j].NAOrb*sizeof(gdouble));
				
				for(i=0;i<=n;i++)
				{
					if(strstr(GeomOrb[j].Symb,AtSymbOrbitals[i])==0)
						continue;
					GeomOrb[j].EnerBetaOrbitals[GeomOrb[j].NBetaOrb] = EnerOrbitals[i];
					GeomOrb[j].OccBetaOrbitals[GeomOrb[j].NBetaOrb]  = OccOrbitals[i];
					GeomOrb[j].SymBetaOrbitals[GeomOrb[j].NBetaOrb]  = g_strdup(SymOrbitals[i]);
					for(k=0;k<GeomOrb[j].NAOrb;k++)
						GeomOrb[j].CoefBetaOrbitals[GeomOrb[j].NBetaOrb][k] = CoefOrbitals[i][k];
					GeomOrb[j].NBetaOrb++;
				}
			}
			break;
		}
		if(n<0)
			return FALSE;
		else
			return TRUE;
}
/********************************************************************************/
void read_gabedit_atomic_orbitals(gchar *FileName)
{
	gboolean OkAlpha;
	gboolean OkBeta;
	gint i;
	gint j;

	/* Debug("Begin of free atomic orbitals\n");*/
	free_atomic_orbitals();	
 	/*Debug("End of free atomic orbitals\n");*/

	set_status_label_info(_("At. Orb."),_("Reading"));
	OkBeta = read_atomic_orbitals_in_gabedit(FileName,2);/* if beta orbital*/
	/* Debug("Ok atomic orbital = %d\n",Ok);*/
	OkAlpha = read_atomic_orbitals_in_gabedit(FileName,1);
	/* Debug("End of reding Alpha atomic orbitals\n");*/
	if(!OkBeta)
	{
		for(j=0;j<nCenters;j++)
		{
			GeomOrb[j].CoefBetaOrbitals = GeomOrb[j].CoefAlphaOrbitals;
			GeomOrb[j].EnerBetaOrbitals = GeomOrb[j].EnerAlphaOrbitals;		
			GeomOrb[j].SymBetaOrbitals = GeomOrb[j].SymAlphaOrbitals;
			GeomOrb[j].NBetaOrb = GeomOrb[j].NAlphaOrb;

			GeomOrb[j].OccBetaOrbitals = g_malloc(GeomOrb[j].NAOrb*sizeof(gdouble));
			for(i=0;i<GeomOrb[j].NBetaOrb;i++)
			{
				if(GeomOrb[j].OccAlphaOrbitals[i]>1.0)
				{
					GeomOrb[j].OccBetaOrbitals[i] = GeomOrb[j].OccAlphaOrbitals[i]/2;
					GeomOrb[j].OccAlphaOrbitals[i] = GeomOrb[j].OccBetaOrbitals[i];
				}
				else
					GeomOrb[j].OccBetaOrbitals[i] = 0.0;
			}
		}
	}
	/* print_atomic_orbitals();*/
	if(OkAlpha || OkBeta)
	{
		AOAvailable = TRUE;
		set_status_label_info(_("At. Orb."),_("Ok"));
		glarea_rafresh(GLArea); /* for geometry*/
	}
	else
	{
		AOAvailable = FALSE;
		set_status_label_info(_("At. Orb."),_("Nothing"));
	}

}
