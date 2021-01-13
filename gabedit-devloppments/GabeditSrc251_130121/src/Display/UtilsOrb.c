/* UtilsOrb.c */
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
#include "../Display/GlobalOrb.h"
#include "../Utils/Vector3d.h"
#include "../Display/GLArea.h"
#include "../Display/Orbitals.h"
#include "../Display/OrbitalsMolpro.h"
#include "../Display/OrbitalsGamess.h"
#include "../Display/OrbitalsQChem.h"
#include "../Display/GeomOrbXYZ.h"
#include "../Display/BondsOrb.h"
#include "../Display/UtilsOrb.h"
#include "../Display/TriangleDraw.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Common/Windows.h"
#include "../Display/Vibration.h"
#include "../Display/ContoursPov.h"
#include "../Display/PlanesMappedPov.h"
#include "../Display/GridCube.h"
#include "../Display/GridCP.h"
#include "../Display/ColorMap.h"
#include "../Display/LabelsGL.h"

/**********************************************/
static gint getOptimalN(gint nG)
{
	/* 2^n = 16, 32, 64, 128, 256, 512, 1024 */
	/* 3*2^n = 24, 48, 96, 192, 384, 768 */
	static gint def[] = {17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769,1025};
	static gint nd = sizeof(def)/sizeof(gint);
	gint i;
	if(nG<17) nG = 17;
	for(i=0;i<nd-1;i++)
	{
		if(nG>def[i] && nG<def[i+1]) 
		{
			nG = def[i];
			break;
		}
	}
	if(nG%2==0) nG++;
	if(nG>65) nG = 65;
	return nG;
}
/**********************************************/
void InitializeAll()
{
 	CoefAlphaOrbitals = NULL;
 	EnerAlphaOrbitals = NULL;
	TypeSelOrb = 1; 
	NumSelOrb = -1;
	free_objects_all();
}
/**********************************************/
gdouble** CreateTable2(gint N)
{
	gdouble** T;
	gint i;
	gint j;

	T = g_malloc(N*sizeof(gdouble*)) ;
	for(i=0;i<N;i++)
	{
		T[i] = g_malloc(N*sizeof(gdouble));
		for(j=0;j<N;j++) T[i][j] = 0.0;
	}

	return T;
}
/**********************************************/
gdouble** FreeTable2(gdouble **T,gint N)
{
	gint i;
	for(i=0;i<N;i++) g_free(T[i]);
	g_free(T);
	return NULL;
}
/**********************************************/
gint GetTotalNelectrons()
{
  gint i;
  gint N=0;
 for(i=0;i<nCenters;i++)
 {
         if(Type[GeomOrb[i].NumType].N>0)
              N +=Type[GeomOrb[i].NumType].N;
 }
 return N;
}
/**********************************************/
gdouble GetSumAbsCharges()
{
	gdouble s = 0;
	gint i;
	for(i=0;i<nCenters;i++)
		s += fabs(GeomOrb[i].partialCharge);
	return s;
}
/**********************************************/
void DefineNOccs()
{
  NTotOcc = GetTotalNelectrons();
  NAlphaOcc =NTotOcc/2+1;
  NBetaOcc =NTotOcc/2;
  NTotOcc /=2;
  TypeSelOrb = 1;
  NumSelOrb = NAlphaOcc-1;
}
/**********************************************/
char GetSymmetry(gint l)
{
        switch(l)
        {
        case 0 : return 'S';
        case 1 : return 'P';
        case 2 : return 'D';
        case 3 : return 'F';
        case 4 : return 'G';
        case 5 : return 'H';
        case 6 : return 'I';
        }
       return 'x';
}
/**********************************************/
gint GetNelectrons(char *symb)
{
  gint i;
  gint  Z[NATOMS]={
	1,2,
	3,4,5,6,7,8,9,10,
	11,12,13,14,15,16,17,18,
	19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,
        37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,
        55,56,57,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,
        87,88,89,
        58,59,60,61,62,63,64,65,66,67,68,69,70,71, 
        90,91,92,93,94,95,96,97,98,99,100,101,102,103,
        0
	};
  char *Symb[NATOMS]={
	"H","He",
	"Li","Be","B","C","N","O","F","Ne",
	"Na","Mg","Al","Si","P","S","Cl","Ar",
	"K","Ca","Sc","Ti","V","Cr","Mn","Fe",
	"Co","Ni","Cu","Zn","Ga","Ge","As","Se",
	"Br","Kr","Rb","Sr","Y","Zr","Nb","Mo",
	"Tc","Ru","Rh","Pd","Ag","Cd","In",
	"Sn","Sb","Te","I","Xe","Cs","Ba","La",
	"Hf","Ta","W","Re","Os","Ir","Pt","Au","Hg","Tl",
	"Pb","Bi","Po","At","Rn","Fr","Ra","Ac",
	"Ce","Pr","Nd","Pm","Sm","Eu","Gd","Tb",
	"Dy","Ho","Er","Tm","Yb","Lu",
	"Th","Pa","U","Np","Pu","Am","Cm","Bk","Cf",
	"Es","Fm","Md","No","Lr",
        "X"
	};

   for(i=0;i<NATOMS;i++)
    if( !strcmp(symb,Symb[i]) )
      return Z[i];

   return -1;
}
/**********************************************/
void printLineChar(char c,gint n)
{
  gint i;
  for(i=0;i<n;i++)
  	printf("%c",c);
  printf("\n");
}
/**********************************************/
gint get_type_file_orb(gchar *fileName)
{
 	gchar *t;
 	FILE *fd;
 	guint taille=BSIZE;
	gint ktype = GABEDIT_TYPEFILE_UNKNOWN;

 	t=g_malloc(taille);
 	fd = FOpen(fileName, "rb");

 	if(fd ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
 		g_free(t);
  		return ktype;
 	}
	if(0<get_one_int_from_fchk_gaussian_file(fd,"Number of atoms "))
	{
		ktype = GABEDIT_TYPEFILE_GAUSSIAN_FCHK;
 		g_free(t);
		fclose(fd);
		printf("GABEDIT_TYPEFILE_GAUSSIAN_FCHK\n");
		return ktype;
	}
	rewind(fd);
    	{ char* e = fgets(t,taille,fd);}
	uppercase(t);
        if(strstr(t, "ENTERING" ))
		ktype = GABEDIT_TYPEFILE_GAUSSIAN;
	else
        if(strstr( t, "[MOLDEN FORMAT]" ))
			ktype = GABEDIT_TYPEFILE_MOLDEN;
	else 
	if(mystrcasestr( t, "<Title>"))
                        ktype = GABEDIT_TYPEFILE_WFX;
	else
	if(strstr( t, "[GABEDIT FORMAT]" ))
		ktype = GABEDIT_TYPEFILE_GABEDIT;
	else
	if(strstr( t, "GAMESS" ))
		ktype = GABEDIT_TYPEFILE_GAMESS;
	else
	if(atoi(t)>0 && !strstr(t,"**********"))
		ktype = GABEDIT_TYPEFILE_XYZ;
	if( ktype == GABEDIT_TYPEFILE_UNKNOWN)
	{
		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
			if(strstr(t,"PROGRAM SYSTEM MOLPRO"))
			{
				ktype = GABEDIT_TYPEFILE_MOLPRO;
				break;
			}
			if(mystrcasestr( t, "<Title>"))
			{
                        	ktype = GABEDIT_TYPEFILE_WFX;
				break;
			}
			if(strstr(t,"GAMESS VERSION") || strstr(t,"PC GAMESS"))
			{
				ktype = GABEDIT_TYPEFILE_GAMESS;
				break;
			}
			if(strstr(t,"Welcome to Q-Chem"))
			{
				ktype = GABEDIT_TYPEFILE_QCHEM;
				break;
			}
			if(strstr(t,"Northwest Computational Chemistry Package"))
			{
				ktype = GABEDIT_TYPEFILE_NWCHEM;
				break;
			}
			uppercase(t);
        		if(strstr(t, "ENTERING GAUSSIAN" ))
			{
				ktype = GABEDIT_TYPEFILE_GAUSSIAN;
				break;
			}
			if(strstr( t, "[MOLDEN FORMAT]" ))
			{ 
				ktype = GABEDIT_TYPEFILE_MOLDEN;
				break;
			}
		}
	}
	rewind(fd);
	if( ktype == GABEDIT_TYPEFILE_UNKNOWN)
	{
		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
			if(strstr(t,"* O   R   C   A *"))
			{
				ktype = GABEDIT_TYPEFILE_ORCA;
				break;
			}
		}
	}
	rewind(fd);
	if( ktype == GABEDIT_TYPEFILE_UNKNOWN)
	{
		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
			if(strstr(t,"GAMESS"))
			{
    				{ char* e = fgets(t,taille,fd);}
				if(strstr(t,"FROM IOWA STATE UNIVERSITY"))
				ktype = GABEDIT_TYPEFILE_GAMESS;
				break;
			}
		}
	}
	rewind(fd);
	if( ktype == GABEDIT_TYPEFILE_UNKNOWN)
	{
    		{ char* e = fgets(t,taille,fd);}
		if(strstr(t,"START OF MOPAC FILE"))
			ktype = GABEDIT_TYPEFILE_MOPAC_AUX;
	}
	rewind(fd);
	if( ktype == GABEDIT_TYPEFILE_UNKNOWN)
	{
    		{ char* e = fgets(t,taille,fd);}
		if(strstr(t,"BEGIN IRC"))
			ktype = GABEDIT_TYPEFILE_GAMESSIRC;
	}
	rewind(fd);
	if( ktype == GABEDIT_TYPEFILE_UNKNOWN)
	{
		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
			if(strstr(t,"in the AO basis:"))
			{
    				{ char* e = fgets(t,taille,fd);}
				if(strstr(t,"------------------"))
				ktype = GABEDIT_TYPEFILE_NBO;
				break;
			}
		}
	}
 	fclose(fd);
 	g_free(t);
 	if(ktype==GABEDIT_TYPEFILE_UNKNOWN)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry,  I can not determine the type of '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
 	}
	return ktype;
        
}
/**********************************************************/
/* return 
   -1 : undefined
    0 : cartezian
    1 : Spherical
*/
gint get_type_basis_in_gamess_file(gchar *fileName)
{
 	gchar *t;
 	FILE *fd;
 	guint taille=BSIZE;
	gint ktype = -1;

 	t=g_malloc(taille);
 	fd = FOpen(fileName, "rb");

 	if(fd ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);

 		g_free(t);
  		return ktype;
 	}
	ktype = 0;
	while(!feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
        	if(strstr( t, "ISPHER="))
		{
			gchar t1[50];
			gchar t2[50];
			sscanf(t,"%s %s",t1,t2);
			if(strstr(t2,"-")) ktype = 1;
			else if(atoi(t2)==0) ktype = -1;
			break;
		}
	}
 	fclose(fd);
 	g_free(t);
	return ktype;
        
}
/**********************************************************/
/* return 
   -1 : undefined
    0 : cartezian
    1 : Spherical
*/
gint get_type_basis_in_gaussian_file(gchar *fileName)
{
 	gchar *t;
 	FILE *fd;
 	guint taille=BSIZE;
	gint ktype = -1;

 	t=g_malloc(taille);
 	fd = FOpen(fileName, "rb");

 	if(fd ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);

 		g_free(t);
  		return ktype;
 	}
	ktype = 0;
	while(!feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
        	if(strstr( t, "(5D, 7F)"))
		{
			ktype = 1;
			break;
		}
        	if(strstr( t, "(6D, 7F)"))
		{
			ktype = 0;
			break;
		}
        	if(strstr( t, "(6D, 10F)"))
		{
			ktype = 0;
			break;
		}
	}

 	fclose(fd);
 	g_free(t);
	return ktype;
        
}
/**********************************************************/
/* return 
   -1 : undefined
    0 : cartezian
    1 : Spherical
*/
gint get_type_basis_in_nwchem_file(gchar *fileName)
{
 	gchar *t;
 	FILE *fd;
 	guint taille=BSIZE;
	gint ktype = -1;

 	t=g_malloc(taille);
 	fd = FOpen(fileName, "rb");

 	if(fd ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);

 		g_free(t);
  		return ktype;
 	}
	ktype = 0;
	while(!feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
        	if(strstr( t, "ao basis") && strstr( t, "spherical")) {ktype=1;break;}
        	if(strstr( t, "ao basis") && strstr( t, "cart")) {ktype=0;break;}
	}
 	fclose(fd);
 	g_free(t);
	return ktype;
        
}
/**********************************************************/
/* return 
   -1 : undefined
    0 : cartezian
    1 : Spherical
*/
gint get_type_basis_in_qchem_file(gchar *fileName)
{
 	gchar *t;
 	FILE *fd;
 	guint taille=BSIZE;
	gint ktype = -1;
	gint ks = 0;
	gint kc = 0;

 	t=g_malloc(taille);
 	fd = FOpen(fileName, "rb");

 	if(fd ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);

 		g_free(t);
  		return ktype;
 	}
	ktype = 0;
	while(!feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
        	if(strstr( t, "  d 1 ")) ks++;
        	if(strstr( t, "  f 1 ")) ks++;
        	if(strstr( t, "  g 1 ")) ks++;
        	if(strstr( t, "  h 1 ")) ks++;
        	if(strstr( t, "  i 1 ")) ks++;
        	if(strstr( t, "  j 1 ")) ks++;
        	if(strstr( t, " dxx ")) kc++;
        	if(strstr( t, " fxyz ")) kc++;
        	if(strstr( t, " gxxxx ")) kc++;
        	if(strstr( t, " hxxxxx ")) kc++;
        	if(strstr( t, " ixxxxx ")) kc++;
        	if(strstr( t, " jxxxxx ")) kc++;
	}
	/* printf("ks = %d kc = %d\n",ks,kc);*/
	if(ks>0 && kc ==0) ktype = 1;
	else if(ks==0 && kc >0) ktype = 0;
	else if(ks==0 && kc==0) ktype = 0;
	else ktype= -1;

 	fclose(fd);
 	g_free(t);
	return ktype;
        
}
/**********************************************************/
/* return 
   -1 : undefined
    0 : cartezian
    1 : Spherical
*/
gint get_type_basis_in_gabedit_file(gchar *fileName)
{
 	gchar *t;
 	FILE *fd;
 	guint taille=BSIZE;
	gint ktype = -1;

 	t=g_malloc(taille);
 	fd = FOpen(fileName, "rb");

 	if(fd ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
 		g_free(t);
  		return ktype;
 	}
	ktype = -1;
    	{ char* e = fgets(t,taille,fd);}
	uppercase(t);
	if(strstr( t, "[GABEDIT FORMAT]"))
	{
		if(strstr( t, "SPHE"))
			ktype = 1;
		else
			ktype = 0;
	}
	
 	fclose(fd);
 	g_free(t);
	return ktype;
        
}
/**********************************************************/
/* return 
   -1 : undefined
    0 : cartezian
    1 : Spherical
*/
gint get_type_basis_in_molden_file(gchar *fileName)
{
 	gchar *t;
 	FILE *fd;
 	guint taille=BSIZE;
	gint ktype = -1;

 	t=g_malloc(taille);
 	fd = FOpen(fileName, "rb");

 	if(fd ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
 		g_free(t);
  		return ktype;
 	}
	ktype = -1;
    	{ char* e = fgets(t,taille,fd);}
	uppercase(t);
	//if(strstr( t, "[MOLDEN FORMAT]"))
	{
		ktype = 0;
		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
			uppercase(t);
			if(strstr( t, "[5D"))
			{
				ktype = 1;
				break;
			}
		}
	}
	
 	fclose(fd);
 	g_free(t);
	return ktype;        
}
/**********************************************************/
void PrintAllOrb(gdouble** M)
{
 gint i,j;


 gint Nblock;
 gint Nrest;
 gint Nb;
 gint Nbmax=5;
 gint Nbj,Nej;
 gint l;
 char *XYZ[]={"x","y","z"};

 Nblock = NAOrb/Nbmax;
 Nrest = NAOrb%Nbmax;
 if(Nrest !=0) Nblock++;
 

 for(Nb=0;Nb<Nblock;Nb++)
 { 
 printf("\n");
 printf("\t");
 Nbj=Nbmax*Nb;
 Nej=Nbmax*Nb+Nbmax;
 if(Nb==Nblock-1 && Nrest !=0)
 	Nej=Nbmax*Nb+Nrest;
 for( j=Nbj;j<Nej;j++)
 {
     if(j<NTotOcc)
	printf("%9s ","Occuped");
     else
	printf("%9s ","Virtual");
 }
 printf("\n");
 printf("\t");
 for( j=Nbj;j<Nej;j++)
	printf("%9s ","-------");
 printf("\n");
 i=-1;

 for( i=0;i<NAOrb;i++)
 {
		 l=0;
		 for(j=0;j<3;j++)
		   l += AOrb[i].Gtf[0].l[j];

		 printf("%c",GetSymmetry(l));
		 for(j=0;j<3;j++)
		 {
			 switch(AOrb[i].Gtf[0].l[j])
			 {
			 case 0:break;
			 case 1:printf("%s",XYZ[j]);break;
			 default :printf("%s%d",XYZ[j],AOrb[i].Gtf[0].l[j]);
			 }
		 }
		printf("\t");

 for( j=Nbj;j<Nej;j++)
     printf("%9.6f ",M[j][i]);
   printf("\n");
 }

 }/*End Nb loop */
   printf("\n");
}
/*************************************************************************************/
static gboolean testascii(char c)
{
	switch ( c )
	{
	case	'0':
	case	'1':
	case	'2':
	case	'3':
	case	'4':
	case	'5':
	case	'6':
	case	'7':
	case	'8':
	case	'9':
	case	'.':
	case	'e':
	case	'E':
	case	'+':
	case	'-':return TRUE;
	}
	return FALSE;
}
/*************************************************************************************/
gboolean testi(char c)
{
	switch ( c )
	{
	case	'0':
	case	'1':
	case	'2':
	case	'3':
	case	'4':
	case	'5':
	case	'6':
	case	'7':
	case	'8':
	case	'9': return TRUE;
	}
	return FALSE;
}
/*************************************************************************************/
gboolean this_is_a_real(gchar *t)
{
	guint i;
	for(i=0;i<strlen(t);i++)
		if(!testascii(t[i]) ) return FALSE;
	if(t[0] =='e' || t[0] =='E' ) return FALSE;
	return TRUE;

}
/*************************************************************************************/
gboolean this_is_a_integer(gchar *t)
{
	guint i;
	if(!testi(t[0])&& t[0] != '-' ) return FALSE;
	for(i=1;i<strlen(t);i++)
		if(!testi(t[i]) ) return FALSE;
	return TRUE;

}
/*************************************************************************************/
void free_data_all()
{
        free_grid_all();
        free_iso_all();
        free_orbitals();
        free_geometry();
	set_label_title("",0,0);
}  
/********************************************************************************/
static void change_entry_value(GtkWidget *Entry, gpointer data)
{
	GtkWidget **entriestmp = NULL;
	gushort ii;
	gushort jj;
	gushort i;
	gushort j;
	gint k;
	gchar* temp;
	GtkWidget *entries[3][6];
	gchar tnG[100]="49";
	gint nG = (gint)fabs(limits.MinMax[1][1]-limits.MinMax[0][1])*6;

	nG = getOptimalN(nG);
	sprintf(tnG,"%d",nG);
	if(GTK_IS_WIDGET(Entry))
	{
		entriestmp = (GtkWidget **)g_object_get_data (G_OBJECT (Entry), "Entries");
	}
	else return;

	if(entriestmp==NULL) return;

	for(i=0;i<3;i++)
	for(j=0;j<6;j++)
		entries[i][j] = entriestmp[i*6+j];

	for(i=0;i<3;i++)
	for(j=0;j<6;j++)
		if(!GTK_IS_WIDGET(entries[i][j]))return;
	ii = 10;
	jj = 10;
	for(i=0;i<3;i++)
	for(j=0;j<6;j++)
	{
		if(Entry == entries[i][j])
		{
			ii = i;
			jj = j;
			break;
		}
	}
	if(ii==10 || jj == 10) return;
	/* reset second and third direction */
	if(ii==0 && jj <= 2)
	{
		gdouble C1[3];
		gdouble C2[3];
		gdouble C3[3];
		gdouble p = 0;

		for(i=0;i<3;i++) C1[i] = atof(gtk_entry_get_text(GTK_ENTRY(entries[0][i])));
		for(i=0;i<3;i++) C2[i] = 0.0;
		for(i=0;i<3;i++)
			if(fabs(C1[i])<1e-6)
			{
				C2[i] = 1.0;
				break;
			}
		if(i==3) for(i=0;i<3;i++) C2[i] = 1.0;

		k = 0;
		for(i=0;i<3;i++)
		{
			if(fabs(C1[i])>1e-6)
			{
				k = i;
				break;
			}
		}
		if(C1[k]!=0)
		{
			for(i=0;i<3;i++) if(k!=i) p += C1[i]*C2[i];
			C2[k] = -p/C1[k];
		}
		for(i=0;i<3;i++)
			C3[i] = C1[(i+1)%3]*C2[(i+2)%3] - C2[(i+1)%3]*C1[(i+2)%3];


		for(i=0;i<3;i++)
		{
			temp = g_strdup_printf("%lf",C2[i]);
		       	gtk_entry_set_text(GTK_ENTRY(entries[1][i]),temp);
			g_free(temp);
			temp = g_strdup_printf("%lf",C3[i]);
		       	gtk_entry_set_text(GTK_ENTRY(entries[2][i]),temp);
			g_free(temp);
		}
		return ;
	}
	/* reset z for second direction, reset third direction */
	if(ii==1 && jj <= 1)
	{
		gdouble C1[3];
		gdouble C2[3];
		gdouble C3[3];
		gdouble p = 0;

		for(i=0;i<3;i++) C1[i] = atof(gtk_entry_get_text(GTK_ENTRY(entries[0][i])));
		for(i=0;i<3;i++) C2[i] = atof(gtk_entry_get_text(GTK_ENTRY(entries[1][i])));

		k = 2;
		if(C1[k]!=0)
		{
			for(i=0;i<3;i++) if(k!=i) p += C1[i]*C2[i];
			C2[k] = -p/C1[k];
		}
		for(i=0;i<3;i++) C3[i] = C1[(i+1)%3]*C2[(i+2)%3] - C2[(i+1)%3]*C1[(i+2)%3];

		temp = g_strdup_printf("%lf",C2[k]);
		gtk_entry_set_text(GTK_ENTRY(entries[1][k]),temp);
		g_free(temp);

		for(i=0;i<3;i++)
		{
			temp = g_strdup_printf("%lf",C3[i]);
		       	gtk_entry_set_text(GTK_ENTRY(entries[2][i]),temp);
			g_free(temp);
		}
		return;
	}
	if(jj==3)
	{
		gdouble min = 0;
		gdouble max = 0;

		min = atof(gtk_entry_get_text(GTK_ENTRY(entries[ii][jj])));
		max = -min;

		nG = (gint)fabs(max-min)*6;
		nG = getOptimalN(nG);
		sprintf(tnG,"%d",nG);

		temp = g_strdup_printf("%lf",max);
		gtk_entry_set_text(GTK_ENTRY(entries[ii][jj+1]),temp);
		g_free(temp);
		gtk_entry_set_text(GTK_ENTRY(entries[ii][jj+2]),tnG);
		if(ii<2)
		{
			temp = g_strdup_printf("%lf",min);
			gtk_entry_set_text(GTK_ENTRY(entries[ii+1][jj]),temp);
			g_free(temp);
		}

		return;
	}
	if(jj==4&& ii<2)
	{
		gdouble max = 0;

		max = atof(gtk_entry_get_text(GTK_ENTRY(entries[ii][jj])));
		temp = g_strdup_printf("%lf",max);
		gtk_entry_set_text(GTK_ENTRY(entries[ii+1][jj]),temp);
		g_free(temp);
		return;
	}
	if(jj==5 && ii<2)
	{
		gint nPoints = 10;

		nPoints = atoi(gtk_entry_get_text(GTK_ENTRY(entries[ii][jj])));
		temp = g_strdup_printf("%d", nPoints);
		gtk_entry_set_text(GTK_ENTRY(entries[ii+1][jj]),temp);
		g_free(temp);
		return;
	}
}
/********************************************************************************/
GtkWidget *create_grid_frame( GtkWidget *vboxall,gchar* title)
{
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gushort i;
	gushort j;
	GtkWidget *Table;
	gchar* temp = NULL;
	static GtkWidget* entries[3][6];
	gint nG = (gint)fabs(limits.MinMax[1][1]-limits.MinMax[0][1])*6;
	gchar tnG[100]="49";

	nG = getOptimalN(nG);
	sprintf(tnG,"%d",nG);

	frame = gtk_frame_new (title);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	Table = gtk_table_new(4,7,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),Table);

	/* first line , titles */
	i = 0; j = 0;
	j++;
	add_label_at_table(Table, _("Vx"),(gushort)i,(gushort)j,GTK_JUSTIFY_CENTER);
	j++;
	add_label_at_table(Table, _("Vy"),(gushort)i, (gushort)j,GTK_JUSTIFY_CENTER);
	j++;
	add_label_at_table(Table, _("Vz"),(gushort)i, (gushort)j,GTK_JUSTIFY_CENTER);
	j++;
	add_label_at_table(Table, _("Minimum"),(gushort)i, (gushort)j,GTK_JUSTIFY_CENTER);
	j++;
	add_label_at_table(Table, _("Maximum"),(gushort)i, (gushort)j,GTK_JUSTIFY_CENTER);
	j++;
	add_label_at_table(Table, _("Number of points"),(gushort)i, (gushort)j,GTK_JUSTIFY_CENTER);

	/* First direction */
	j = 0;
	i++;
	add_label_at_table(Table, _("First direction "),(gushort)i,(gushort)j,GTK_JUSTIFY_LEFT);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),50,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),"1.0");

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),50,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),"0.0");

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),50,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),"0.0");

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),100,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	temp = g_strdup_printf("%lf",limits.MinMax[0][0]);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),temp);
	if(temp) g_free(temp);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),100,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	temp = g_strdup_printf("%lf",limits.MinMax[1][0]);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),temp);
	if(temp) g_free(temp);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),100,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),tnG);

	/* Second direction */
	j = 0;
	i++;
	add_label_at_table(Table, _("Second direction "),(gushort)i,(gushort)j,GTK_JUSTIFY_LEFT);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),50,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),"0.0");

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),50,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),"1.0");

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),50,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),"0.0");
	gtk_widget_set_sensitive(entries[i-1][j-1], FALSE);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),100,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	temp = g_strdup_printf("%lf",limits.MinMax[0][1]);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),temp);
	if(temp) g_free(temp);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),100,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	temp = g_strdup_printf("%lf",limits.MinMax[1][1]);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),temp);
	if(temp) g_free(temp);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),100,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),tnG);

	/* Third direction */
	j = 0;
	i++;
	add_label_at_table(Table, _("Third direction "),(gushort)i,(gushort)j,GTK_JUSTIFY_LEFT);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),50,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),"0.0");
	gtk_widget_set_sensitive(entries[i-1][j-1], FALSE);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),50,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),"0.0");
	gtk_widget_set_sensitive(entries[i-1][j-1], FALSE);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),50,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),"1.0");
	gtk_widget_set_sensitive(entries[i-1][j-1], FALSE);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),100,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	temp = g_strdup_printf("%lf",limits.MinMax[0][2]);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),temp);
	if(temp) g_free(temp);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),100,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	temp = g_strdup_printf("%lf",limits.MinMax[1][2]);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),temp);
	if(temp) g_free(temp);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),100,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),tnG);



	for(i=0;i<3;i++)
	for(j=0;j<6;j++)
	{
		g_object_set_data (G_OBJECT (entries[i][j]), "Entries", &entries);
	}

	for(i=0;i<3;i++)
	for(j=0;j<6;j++)
  		g_signal_connect(G_OBJECT(entries[i][j]), "changed",G_CALLBACK(change_entry_value),entries[i][j]);

	gtk_widget_show_all(frame);
	g_object_set_data (G_OBJECT (frame), "Entries",&entries);

  	return frame;
}
/********************************************************************************/
void create_grid(gchar* title)
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *vboxall;
	GtkWidget *vboxwin;
	GtkWidget *button;
	GtkWidget** entries;

	if(!GeomOrb)
	{
		Message(_("Sorry, Please load a file beforee\n"),_("Error"),TRUE);
		return;
	}
	if(!CoefAlphaOrbitals && TypeGrid != GABEDIT_TYPEGRID_MEP_CHARGES)
	{
		Message(_("Sorry, Please load the MO beforee\n"),_("Error"),TRUE);
		return;
	}
	if(TypeGrid == GABEDIT_TYPEGRID_MEP_CHARGES)
	{
		gdouble s= GetSumAbsCharges();
		if(s<1e-6) Message(_("Sorry, All partial charges are null\n"),_("Error"),TRUE);
		return;
	}

	if(!AOAvailable &&(TypeGrid == GABEDIT_TYPEGRID_DDENSITY || TypeGrid == GABEDIT_TYPEGRID_ADENSITY))
	{
		Message(_("Sorry, No atomic orbitals available.\nPlease use a gabedit file for load : \n"
		  "Geometry, Molecular and Atomic Orbitals\n"),_("Error"),TRUE);
		return;
	}
	
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),title);
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Grid ");

	vboxall = create_vbox(Win);
	vboxwin = vboxall;
	frame = create_grid_frame(vboxall,_("Box & Grid"));
	entries = (GtkWidget**) g_object_get_data (G_OBJECT (frame), "Entries");
	g_object_set_data (G_OBJECT (Win), "Entries",entries);
   

	hbox = create_hbox_false(vboxwin);
	gtk_widget_realize(Win);

	button = create_button(Win,_("Cancel"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, G_OBJECT(Win));
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,G_OBJECT(Win));
	gtk_widget_show (button);

	button = create_button(Win,_("OK"));
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)applygrid,G_OBJECT(Win));
  

	gtk_widget_show_all (Win);
}
/********************************************************************************/
void applyelfdens(GtkWidget *Win,gpointer data)
{
	GtkWidget** entriestmp = NULL;
	G_CONST_RETURN gchar* temp;
	gchar* dump;
	gint i;
	gint j;
	GridLimits limitstmp;
	gint NumPointstmp[3];
	GtkWidget *entries[3][6];
	gdouble V[3][3];
	Grid* gridDens = NULL;
	gboolean ongrid = TRUE;

	if(GTK_IS_WIDGET(Win))
	{
		entriestmp = (GtkWidget **)g_object_get_data(G_OBJECT (Win), "Entries");
		ongrid = GPOINTER_TO_INT(g_object_get_data(G_OBJECT (Win), "OnGrid"));
	}
	else return;

	if(entriestmp==NULL) return;

	for(i=0;i<3;i++)
	for(j=0;j<6;j++)
		entries[i][j] = entriestmp[i*6+j];
	
	for(i=0;i<3;i++)
	{
		for(j=3;j<5;j++)
		{
        		temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][j])); 
			dump = NULL;
			if(temp && strlen(temp)>0)
			{
				dump = g_strdup(temp);
				delete_first_spaces(dump);
				delete_last_spaces(dump);
			}

			if(dump && strlen(dump)>0 && this_is_a_real(dump))
			{
				limitstmp.MinMax[j-3][i] = atof(dump);
			}
			else
			{
				GtkWidget* message = Message(_("Error : one entry is not a float "),_("Error"),TRUE);
  				gtk_window_set_modal (GTK_WINDOW (message), TRUE);
				return;
			}
			if(dump) g_free(dump);
		}
        	temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][5])); 
		NumPointstmp[i] = atoi(temp);
		if(NumPointstmp[i] <=2)
		{
			GtkWidget* message = Message(_("Error : The number of points should be > 2. "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		
	}

	for(i=0;i<3;i++)
	{
		if( limitstmp.MinMax[0][i]> limitstmp.MinMax[1][i])
		{
			GtkWidget* message = Message(_("Error :  The minimal value should be smaller than the maximal value "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
	}
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
		{
			V[i][j] = 0;
        		temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][j])); 
			dump = NULL;
			if(temp && strlen(temp)>0)
			{
				dump = g_strdup(temp);
				delete_first_spaces(dump);
				delete_last_spaces(dump);
			}

			if(dump && strlen(dump)>0 && this_is_a_real(dump))
			{
				V[i][j] = atof(dump);
			}
			else
			{
				GtkWidget* message = Message(_("Error : one entry is not a float "),_("Error"),TRUE);
  				gtk_window_set_modal (GTK_WINDOW (message), TRUE);
				return;
			}
			if(dump) g_free(dump);
		}
	}
        
	for(i=0;i<3;i++)
	{
		gdouble norm = 0.0;
		for(j=0;j<3;j++)
			norm += V[i][j]*V[i][j];
		if(fabs(norm)<1e-8)
		{
			GtkWidget* message = Message(_("Error : the norm is equal to 0 "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		for(j=0;j<3;j++)
			V[i][j] /= sqrt(norm);
	}
	for(j=0;j<3;j++) originOfCube[j] = 0;
	for(j=0;j<3;j++) firstDirection[j] = V[0][j];
	for(j=0;j<3;j++) secondDirection[j] = V[1][j];
	for(j=0;j<3;j++) thirdDirection[j] = V[2][j];

	for(i=0;i<3;i++)
	{
		NumPoints[i] =NumPointstmp[i] ; 
		for(j=0;j<2;j++)
			limits.MinMax[j][i] =limitstmp.MinMax[j][i]; 
	}


	delete_child(Win);
	free_grid_all();
	grid = define_grid_ELFSAVIN(NumPoints,limits);
	if(grid)
	{
		add_surface();
		free_iso_all();
		limits.MinMax[0][3] = grid->limits.MinMax[0][3];
		limits.MinMax[1][3] = grid->limits.MinMax[1][3];
	}
	gridDens = define_grid_electronic_density(NumPoints,limits);
	computeELFAttractors(grid, gridDens, ongrid);
	Define_Iso(0.9);
	glarea_rafresh(GLArea);
}
/********************************************************************************/
void create_grid_ELF_Dens_analyze(gboolean ongrid)
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *vboxall;
	GtkWidget *vboxwin;
	GtkWidget *button;
	GtkWidget** entries;

	if(!GeomOrb)
	{
		Message(_("Sorry, Please load a file beforee\n"),_("Error"),TRUE);
		return;
	}
	if(!CoefAlphaOrbitals && TypeGrid != GABEDIT_TYPEGRID_MEP_CHARGES)
	{
		Message(_("Sorry, Please load the MO beforee\n"),_("Error"),TRUE);
		return;
	}
	if(TypeGrid == GABEDIT_TYPEGRID_MEP_CHARGES)
	{
		gdouble s= GetSumAbsCharges();
		if(s<1e-6) Message(_("Sorry, All partial charges are null\n"),_("Error"),TRUE);
		return;
	}

	if(!AOAvailable &&(TypeGrid == GABEDIT_TYPEGRID_DDENSITY || TypeGrid == GABEDIT_TYPEGRID_ADENSITY))
	{
		Message(_("Sorry, No atomic orbitals available.\nPlease use a gabedit file for load : \n"
		  "Geometry, Molecular and Atomic Orbitals\n"),_("Error"),TRUE);
		return;
	}
	
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),_("ELF Attractors"));
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Grid ");

	vboxall = create_vbox(Win);
	vboxwin = vboxall;
	frame = create_grid_frame(vboxall,_("Box & Grid"));
	entries = (GtkWidget**) g_object_get_data (G_OBJECT (frame), "Entries");
	g_object_set_data (G_OBJECT (Win), "Entries",entries);
	g_object_set_data (G_OBJECT (Win), "OnGrid",GINT_TO_POINTER(ongrid));
   

	hbox = create_hbox_false(vboxwin);
	gtk_widget_realize(Win);

	button = create_button(Win,_("Cancel"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, G_OBJECT(Win));
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,G_OBJECT(Win));
	gtk_widget_show (button);

	button = create_button(Win,_("OK"));
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)applyelfdens,G_OBJECT(Win));
  

	gtk_widget_show_all (Win);
}
/********************************************************************************/
void read_any_file(gchar* FileName)
{
	gint filetype = get_type_file_orb(FileName);
	switch(filetype)
	{
		case GABEDIT_TYPEFILE_GAMESS : read_gamess_orbitals(FileName);break;
		case GABEDIT_TYPEFILE_GAUSSIAN : read_gauss_orbitals(FileName);break;
		case GABEDIT_TYPEFILE_MOLPRO : read_molpro_orbitals(FileName);break;
		case GABEDIT_TYPEFILE_QCHEM : read_qchem_orbitals(FileName);break;
		case GABEDIT_TYPEFILE_MOLDEN : read_molden_orbitals(FileName);break;
		case GABEDIT_TYPEFILE_GABEDIT : read_gabedit_orbitals(FileName);break;
		case GABEDIT_TYPEFILE_XYZ : gl_read_xyz_file(FileName);break;
		case GABEDIT_TYPEFILE_GAUSSIANINPUT : 
		case GABEDIT_TYPEFILE_MOLCASINPUT :  
		case GABEDIT_TYPEFILE_MOLPROINPUT :  
		case GABEDIT_TYPEFILE_UNKNOWN : break;
	}
}
/********************************************************************************/
gint get_number_of_point(GtkWidget* Entry)
{
	gchar* t = NULL;
	gint N = -1;
        G_CONST_RETURN gchar* temp = gtk_entry_get_text(GTK_ENTRY(Entry)); 
	if(temp)
	{
		t = g_strdup(temp);
		delete_first_spaces(t);
		delete_last_spaces(t);
	}
	if(t && !this_is_a_integer(t))
	{
		GtkWidget* win = Message(_("Error : The number of points should be integer. "),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (win), TRUE);
		g_free(t);
		return -1;
	}
	if(t) g_free(t);
	N = atoi(temp);
	if(N<=0)
	{
		GtkWidget* win = Message(_("Error : The number of points should be positive. "),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (win), TRUE);
		return -1;
	}
	return N;
}
/********************************************************************************/
gboolean get_a_float(GtkWidget* Entry,gdouble* value, gchar* errorMessage)
{
	G_CONST_RETURN gchar* temp;
	gchar* t = NULL;
        temp	= gtk_entry_get_text(GTK_ENTRY(Entry)); 
	if(temp&& strlen(temp)>0)
	{
		t = g_strdup(temp);
		delete_first_spaces(t);
		delete_last_spaces(t);
	}
	else
	{
		GtkWidget* win = Message(errorMessage,_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (win), TRUE);
		return FALSE;
	}
	if(t && !this_is_a_real(t))
	{
		GtkWidget* win = Message(errorMessage,_("Error"),TRUE);
		g_free(t);
  		gtk_window_set_modal (GTK_WINDOW (win), TRUE);
		return FALSE;
	}
	if(t) g_free(t);
	*value = atof(temp);
	return TRUE;
}
/********************************************************************************/
void initialise_global_orbitals_variables()
{
	gint i;
	PopupMenuIsOpen = FALSE;
	BondsOrb = NULL;
	GeomOrb = NULL;
	GLArea = NULL;
	nCenters =0;
 	ShowDipoleOrb = TRUE;
 	ShowHBondOrb = FALSE;
 	ShowHAtomOrb = TRUE;
	ShowMultiBondsOrb = TRUE;
 	ShowVibration = FALSE;
	TypeGrid = GABEDIT_TYPEGRID_ORBITAL;
	TypeGeom = GABEDIT_TYPEGEOM_STICK;
	TypeBlend = GABEDIT_BLEND_NO;
	TypePosWireFrame = GABEDIT_POS_WIREFRAME_NO;
	TypeNegWireFrame = GABEDIT_NEG_WIREFRAME_NO;
	SurfShow = GABEDIT_SURFSHOW_POSNEG;
	TypeTexture = GABEDIT_TYPETEXTURE_NONE;
	Title = NULL;
	ScreenWidthD = gdk_screen_width();
	ScreenHeightD = gdk_screen_height();
	for(i=0;i<3;i++)
		limits.MinMax[0][i] = -5;
	for(i=0;i<3;i++)
		limits.MinMax[1][i] = 5;
	NumPoints[0] = 49;
	NumPoints[1] = 49;
	NumPoints[2] = 49;
	CoefAlphaOrbitals = NULL;
	EnerAlphaOrbitals = NULL;
	OccAlphaOrbitals = NULL;
	SymAlphaOrbitals = NULL;
	CoefBetaOrbitals = NULL;
	EnerBetaOrbitals = NULL;
	OccBetaOrbitals = NULL;
	SymBetaOrbitals = NULL;
	isopositive = NULL;
	isonegative = NULL;
	isonull = NULL;
	grid = NULL;
	gridPlaneForContours = NULL;
	gridPlaneForPlanesMapped = NULL;

	reDrawContoursPlane = FALSE;
	reDrawPlaneMappedPlane = FALSE;

	AOAvailable = FALSE;
	RebuildSurf = TRUE;
	RebuildGeomD = TRUE;
	newContours = TRUE;
	newSurface = FALSE;
	DeleteContours = FALSE;
	CancelCalcul = FALSE;
	init_vibration();
	createBMPFiles = FALSE;
	numBMPFile = 0;
	createPPMFiles = FALSE;
	numPPMFile = 0;
	createPOVFiles = FALSE;
	numPOVFile = 0;
	AOrb = NULL;
	SAOrb = NULL;
	SOverlaps = NULL;
	solventRadius = 1.4;
	alphaFED = 3.0; /* eV^-1 */
}
/********************************************************************************/
void close_window_orb(GtkWidget*win, gpointer data)
{
	if(!GTK_IS_WIDGET(PrincipalWindow)) return;

	CancelCalcul = TRUE;
	while( gtk_events_pending() ) gtk_main_iteration();

	free_data_all();
	free_objects_all();

  	destroy_children(PrincipalWindow);

	GLArea = NULL;
	PrincipalWindow = NULL;
}
/********************************************************************************/
void add_glarea_child(GtkWidget* winchild,gchar* title)
{
  if(GLArea)
  {
  	add_child(PrincipalWindow,winchild,gtk_widget_destroy,title);
  	g_signal_connect(G_OBJECT(winchild),"delete_event",(GCallback)delete_child,NULL);
  }
  else
  {
  	add_button_windows(title,winchild);
  	g_signal_connect(G_OBJECT(winchild), "delete_event",(GCallback)destroy_button_windows,NULL);
  	g_signal_connect(G_OBJECT(winchild), "delete_event",G_CALLBACK(gtk_widget_destroy),NULL);
  }
}
/*************************************************************************************/
void create_color_surfaces_file()
{
	gchar *colorsurface;
	FILE *fd;
	gdouble v[4];
	gint i;
	gint j;
	gint n=3;

	colorsurface = g_strdup_printf("%s/colorssurface",gabedit_directory());

	fd = FOpen(colorsurface, "w");
	if(fd)
	{
		fprintf(fd,"%d\n",n);
		for(i=0;i<n;i++)
		{
			get_color_surface(i,v) ;
			for(j=0;j<4;j++)
				fprintf(fd,"%lf ",v[j]);
			fprintf(fd,"\n");
		}
		fclose(fd);
	}

	g_free(colorsurface);
}
/*************************************************************************************/
void read_color_surfaces_file()
{
	gchar *colorsurface; 
	FILE *fd;
	gint i;
	gint n = 0;
	gchar t[BSIZE];
	gint len = BSIZE;
	gdouble v[4];

	colorsurface = g_strdup_printf("%s/colorssurface",gabedit_directory());

	fd = fopen(colorsurface, "rb");
	if(fd)
	{
    		{ char* e = fgets(t,BSIZE,fd);}
		n = atoi(t);
		for(i=0;i<n;i++)
		{
    			{ char* e = fgets(t,len,fd);}
			sscanf(t,"%lf %lf %lf %lf",&v[0],&v[1],&v[2],&v[3]);
			set_color_surface(i,v);
		}
 		fclose(fd);
	}
	g_free(colorsurface);
}
/*************************************************************************************/
static void applygridsas(GtkWidget *Win,gpointer data)
{
	GtkWidget** entriestmp = NULL;
	G_CONST_RETURN gchar* temp;
	gchar* dump;
	gint i;
	gint j;
	GridLimits limitstmp;
	gint NumPointstmp[3];
	GtkWidget *entries[4][6];
	gdouble V[3][3];


	if(GTK_IS_WIDGET(Win))
	{
		entriestmp = (GtkWidget **)g_object_get_data(G_OBJECT (Win), "Entries");
	}
	else return;

	if(entriestmp==NULL) return;

	for(i=0;i<3;i++)
	for(j=0;j<6;j++)
		entries[i][j] = entriestmp[i*6+j];
	entries[3][0] = entriestmp[18];
	
	for(i=0;i<3;i++)
	{
		for(j=3;j<5;j++)
		{
        		temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][j])); 
			dump = NULL;
			if(temp && strlen(temp)>0)
			{
				dump = g_strdup(temp);
				delete_first_spaces(dump);
				delete_last_spaces(dump);
			}

			if(dump && strlen(dump)>0 && this_is_a_real(dump))
			{
				limitstmp.MinMax[j-3][i] = atof(dump);
			}
			else
			{
				GtkWidget* message = Message(_("Error : one entry is not a float "),_("Error"),TRUE);
  				gtk_window_set_modal (GTK_WINDOW (message), TRUE);
				return;
			}
			if(dump) g_free(dump);
		}
        	temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][5])); 
		NumPointstmp[i] = atoi(temp);
		if(NumPointstmp[i] <=2)
		{
			GtkWidget* message = Message(_("Error : The number of points should be > 2. "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		
	}
	{
        	temp	= gtk_entry_get_text(GTK_ENTRY(entries[3][0])); 
		dump = NULL;
		if(temp && strlen(temp)>0)
		{
			dump = g_strdup(temp);
			delete_first_spaces(dump);
			delete_last_spaces(dump);
		}

		if(dump && strlen(dump)>0 && this_is_a_real(dump))
		{
			solventRadius = atof(dump);
		}
		else
		{
			GtkWidget* message = Message(_("Error : The solvent radius should be a float "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		if(dump) g_free(dump);
	}

	for(i=0;i<3;i++)
	{
		if( limitstmp.MinMax[0][i]> limitstmp.MinMax[1][i])
		{
			GtkWidget* message = Message(_("Error :  The minimal value should be smaller than the maximal value "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
	}
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
		{
			V[i][j] = 0;
        		temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][j])); 
			dump = NULL;
			if(temp && strlen(temp)>0)
			{
				dump = g_strdup(temp);
				delete_first_spaces(dump);
				delete_last_spaces(dump);
			}

			if(dump && strlen(dump)>0 && this_is_a_real(dump))
			{
				V[i][j] = atof(dump);
			}
			else
			{
				GtkWidget* message = Message(_("Error : one entry is not a float "),_("Error"),TRUE);
  				gtk_window_set_modal (GTK_WINDOW (message), TRUE);
				return;
			}
			if(dump) g_free(dump);
		}
	}
        
	for(i=0;i<3;i++)
	{
		gdouble norm = 0.0;
		for(j=0;j<3;j++)
			norm += V[i][j]*V[i][j];
		if(fabs(norm)<1e-8)
		{
			GtkWidget* message = Message(_("Error : the norm is equal to 0 "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		for(j=0;j<3;j++)
			V[i][j] /= sqrt(norm);
	}
	for(j=0;j<3;j++) originOfCube[j] = 0;
	for(j=0;j<3;j++) firstDirection[j] = V[0][j];
	for(j=0;j<3;j++) secondDirection[j] = V[1][j];
	for(j=0;j<3;j++) thirdDirection[j] = V[2][j];

	for(i=0;i<3;i++)
	{
		NumPoints[i] =NumPointstmp[i] ; 
		for(j=0;j<2;j++)
			limits.MinMax[j][i] =limitstmp.MinMax[j][i]; 
	}


	delete_child(Win);
	if(TypeGrid == GABEDIT_TYPEGRID_SAS)
	{
		TypeGrid = GABEDIT_TYPEGRID_SAS;
		Define_Grid();
		Define_Iso(0.0);
	}
	if(TypeGrid == GABEDIT_TYPEGRID_SASMAP)
	{
		TypeGrid = GABEDIT_TYPEGRID_SAS;
		Define_Grid();
		Define_Iso(0.0);
		CancelCalcul = FALSE;
		mapping_with_mep_from_charges();
	}
	
	glarea_rafresh(GLArea);
}
/*************************************************************************************/
static GtkWidget *create_grid_sas_frame( GtkWidget *vboxall,gchar* title)
{
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gushort i;
	gushort j;
	GtkWidget *Table;
	gchar* temp = NULL;
	static GtkWidget* entries[4][6];
	gint nG = (gint)fabs(limits.MinMax[1][1]-limits.MinMax[0][1])*6;
	gchar tnG[100]="49";
	gchar sr[100]="1.4";

	nG = getOptimalN(nG);
	sprintf(tnG,"%d",nG);

	sprintf(sr,"%lf",solventRadius);

	frame = gtk_frame_new (title);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	Table = gtk_table_new(4,7,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),Table);

	/* first line , titles */
	i = 0; j = 0;
	j++;
	add_label_at_table(Table, _("Vx"),(gushort)i,(gushort)j,GTK_JUSTIFY_CENTER);
	j++;
	add_label_at_table(Table, _("Vy"),(gushort)i, (gushort)j,GTK_JUSTIFY_CENTER);
	j++;
	add_label_at_table(Table, _("Vz"),(gushort)i, (gushort)j,GTK_JUSTIFY_CENTER);
	j++;
	add_label_at_table(Table, _("Minimum"),(gushort)i, (gushort)j,GTK_JUSTIFY_CENTER);
	j++;
	add_label_at_table(Table, _("Maximum"),(gushort)i, (gushort)j,GTK_JUSTIFY_CENTER);
	j++;
	add_label_at_table(Table, _("Number of points"),(gushort)i, (gushort)j,GTK_JUSTIFY_CENTER);

	/* First direction */
	j = 0;
	i++;
	add_label_at_table(Table, _("First direction "),(gushort)i,(gushort)j,GTK_JUSTIFY_LEFT);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),50,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),"1.0");

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),50,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),"0.0");

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),50,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),"0.0");

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),100,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	temp = g_strdup_printf("%lf",limits.MinMax[0][0]);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),temp);
	if(temp) g_free(temp);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),100,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	temp = g_strdup_printf("%lf",limits.MinMax[1][0]);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),temp);
	if(temp) g_free(temp);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),100,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),tnG);

	/* Second direction */
	j = 0;
	i++;
	add_label_at_table(Table, _("Second direction "),(gushort)i,(gushort)j,GTK_JUSTIFY_LEFT);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),50,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),"0.0");

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),50,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),"1.0");

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),50,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),"0.0");
	gtk_widget_set_sensitive(entries[i-1][j-1], FALSE);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),100,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	temp = g_strdup_printf("%lf",limits.MinMax[0][1]);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),temp);
	if(temp) g_free(temp);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),100,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	temp = g_strdup_printf("%lf",limits.MinMax[1][1]);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),temp);
	if(temp) g_free(temp);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),100,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),tnG);

	/* Third direction */
	j = 0;
	i++;
	add_label_at_table(Table, _("Third direction "),(gushort)i,(gushort)j,GTK_JUSTIFY_LEFT);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),50,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),"0.0");
	gtk_widget_set_sensitive(entries[i-1][j-1], FALSE);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),50,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),"0.0");
	gtk_widget_set_sensitive(entries[i-1][j-1], FALSE);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),50,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),"1.0");
	gtk_widget_set_sensitive(entries[i-1][j-1], FALSE);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),100,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	temp = g_strdup_printf("%lf",limits.MinMax[0][2]);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),temp);
	if(temp) g_free(temp);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),100,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	temp = g_strdup_printf("%lf",limits.MinMax[1][2]);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),temp);
	if(temp) g_free(temp);

	j++;
	entries[i-1][j-1] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][j-1]),100,-1);
	add_widget_table(Table,entries[i-1][j-1],(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][j-1]),tnG);

	/* Slovent radius */
	j = 0;
	i++;
	{
		GtkWidget *Label;
		GtkWidget *hbox = gtk_hbox_new(0,FALSE);
	
		Label = gtk_label_new ("Solvent radius (Angstrum) ");
   		gtk_label_set_justify(GTK_LABEL(Label),GTK_JUSTIFY_LEFT);
		gtk_box_pack_start (GTK_BOX (hbox), Label, FALSE, FALSE, 0);
		gtk_table_attach(GTK_TABLE(Table),hbox,j,j+2,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  3,3);
	}

	j+=2;
	entries[i-1][0] = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entries[i-1][0]),50,-1);
	gtk_table_attach(GTK_TABLE(Table),GTK_WIDGET(entries[i-1][0]),j,j+3,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  3,3);
	gtk_entry_set_text(GTK_ENTRY(entries[i-1][0]),sr);


	for(i=0;i<3;i++)
	for(j=0;j<6;j++)
	{
		g_object_set_data (G_OBJECT (entries[i][j]), "Entries", &entries);
	}
	g_object_set_data (G_OBJECT (entries[3][0]), "Entries", &entries);

	for(i=0;i<3;i++)
	for(j=0;j<6;j++)
  		g_signal_connect(G_OBJECT(entries[i][j]), "changed",G_CALLBACK(change_entry_value),entries[i][j]);

	gtk_widget_show_all(frame);
	g_object_set_data (G_OBJECT (frame), "Entries",&entries);

  	return frame;
}
/********************************************************************************/
void create_grid_sas(gchar* title)
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *vboxall;
	GtkWidget *vboxwin;
	GtkWidget *button;
	GtkWidget** entries;

	if(!GeomOrb)
	{
		Message(_("Sorry, Please load a file beforee\n"),_("Error"),TRUE);
		return;
	}
	
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),title);
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Grid ");

	vboxall = create_vbox(Win);
	vboxwin = vboxall;
	frame = create_grid_sas_frame(vboxall,_("Box & Grid & Solvent radius"));
	entries = (GtkWidget**) g_object_get_data (G_OBJECT (frame), "Entries");
	g_object_set_data (G_OBJECT (Win), "Entries",entries);
   

	hbox = create_hbox_false(vboxwin);
	gtk_widget_realize(Win);

	button = create_button(Win,_("Cancel"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, G_OBJECT(Win));
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,G_OBJECT(Win));
	gtk_widget_show (button);

	button = create_button(Win,_("OK"));
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)applygridsas,G_OBJECT(Win));
  

	gtk_widget_show_all (Win);
}
/********************************************************************************/
static void apply_set_scale_ball_stick(GtkWidget *Win,gpointer data)
{
	GtkWidget* entryBall = NULL;
	GtkWidget* entryStick = NULL;
	gdouble sBall = 1;
	gdouble sStick = 1;

	if(!GTK_IS_WIDGET(Win)) return;

	entryBall = g_object_get_data (G_OBJECT (Win), "EntryBall");
	entryStick = g_object_get_data (G_OBJECT (Win), "EntryStick");

	if(entryBall) sBall = atof(gtk_entry_get_text(GTK_ENTRY(entryBall)));
	if(entryStick) sStick = atof(gtk_entry_get_text(GTK_ENTRY(entryStick)));
	RebuildGeomD = TRUE;
	setScaleBall(sBall);
	setScaleStick(sStick);
	glarea_rafresh(GLArea);
}
/********************************************************************************/
static void apply_set_scale_ball_stick_close(GtkWidget *Win,gpointer data)
{
	apply_set_scale_ball_stick(Win,data);
	delete_child(Win);
}
/********************************************************************************/
static GtkWidget *add_entry_scale(GtkWidget *table, gchar* strLabel, gint il, gdouble val)
{
	gushort i;
	gushort j;
	GtkWidget *entry;
	GtkWidget *label;

/*----------------------------------------------------------------------------------*/
	i = il;
	j = 0;
	label = gtk_label_new(strLabel);
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = il;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = il;
	j = 2;
	entry =  gtk_entry_new();
	{
		gchar* v = g_strdup_printf("%lf",val);
		if(v)gtk_entry_set_text(GTK_ENTRY(entry),v);
		else gtk_entry_set_text(GTK_ENTRY(entry),"1.0");
		if(v) g_free(v);
	}

	gtk_table_attach(GTK_TABLE(table),entry,
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);

  	return entry;
}
/********************************************************************************/
void set_scale_ball_stick_dlg()
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *hbox;
	GtkWidget *table;
	GtkWidget *vboxall;
	GtkWidget *button;
	GtkWidget *entryBall;
	GtkWidget *entryStick;

	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),_("Scale Ball&Stick"));
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"ScaleBallStick");

	vboxall = create_vbox(Win);
	frame = gtk_frame_new (NULL);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	table = gtk_table_new(2,3,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);

	entryBall = add_entry_scale(table, _("Scale Ball"), 0,getScaleBall());
	g_object_set_data (G_OBJECT (Win), "EntryBall",entryBall);
	entryStick = add_entry_scale(table, _("Scale Stick"), 1,getScaleStick());
	g_object_set_data (G_OBJECT (Win), "EntryStick",entryStick);

	hbox = create_hbox_false(vboxall);
	gtk_widget_realize(Win);

	button = create_button(Win,_("OK"));
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_set_scale_ball_stick_close,G_OBJECT(Win));

	button = create_button(Win,_("Apply"));
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_set_scale_ball_stick,G_OBJECT(Win));

	button = create_button(Win,_("Cancel"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, G_OBJECT(Win));
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,G_OBJECT(Win));
	gtk_widget_show (button);

	gtk_widget_show_all (Win);
}
/*********************************************************************************************************/
static void resetGridColorMap()
{
	GtkWidget* handleBoxColorMapGrid = g_object_get_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapGrid");

	if(handleBoxColorMapGrid)
	{
		GtkWidget* entryLeft  = g_object_get_data(G_OBJECT(handleBoxColorMapGrid), "EntryLeft");
		GtkWidget* entryRight = g_object_get_data(G_OBJECT(handleBoxColorMapGrid), "EntryRight");
		GtkWidget* darea      = g_object_get_data(G_OBJECT(handleBoxColorMapGrid), "DrawingArea");
		ColorMap* colorMap = g_object_get_data(G_OBJECT( handleBoxColorMapGrid),"ColorMap");
		if(colorMap && entryLeft && entryRight && darea && colorMap->numberOfColors>0)
		{
			gdouble minValue = colorMap->colorValue[0].value;
			gdouble maxValue = colorMap->colorValue[colorMap->numberOfColors-1].value;
			ColorMap* newColorMap = new_colorMap_min_max(minValue, maxValue);
			colormap_free(colorMap);
			g_free(colorMap);
			colorMap = newColorMap;
			g_object_set_data(G_OBJECT(handleBoxColorMapGrid),"ColorMap", colorMap);
			g_object_set_data(G_OBJECT(entryLeft),"ColorMap", colorMap);
			g_object_set_data(G_OBJECT(entryRight),"ColorMap", colorMap);
			g_object_set_data(G_OBJECT(darea),"ColorMap", colorMap);
			color_map_refresh(handleBoxColorMapGrid);
			if(GTK_WIDGET_VISIBLE(handleBoxColorMapGrid))
			{
				gtk_widget_hide(handleBoxColorMapGrid);
				gtk_widget_show(handleBoxColorMapGrid);
			}
		}
	}
}
/*********************************************************************************************************/
static void resetPlanesMappedColorMap()
{
	GtkWidget* handleBoxColorMapPlanesMapped = g_object_get_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapPlanesMapped");
	if(handleBoxColorMapPlanesMapped)
	{
		GtkWidget* entryLeft  = g_object_get_data(G_OBJECT(handleBoxColorMapPlanesMapped), "EntryLeft");
		GtkWidget* entryRight = g_object_get_data(G_OBJECT(handleBoxColorMapPlanesMapped), "EntryRight");
		GtkWidget* darea      = g_object_get_data(G_OBJECT(handleBoxColorMapPlanesMapped), "DrawingArea");
		ColorMap* colorMap = g_object_get_data(G_OBJECT( handleBoxColorMapPlanesMapped),"ColorMap");
		if(colorMap && entryLeft && entryRight && darea && colorMap->numberOfColors>0)
		{
			gdouble minValue = colorMap->colorValue[0].value;
			gdouble maxValue = colorMap->colorValue[colorMap->numberOfColors-1].value;
			ColorMap* newColorMap = new_colorMap_min_max(minValue, maxValue);
			colormap_free(colorMap);
			g_free(colorMap);
			colorMap = newColorMap;
			g_object_set_data(G_OBJECT(handleBoxColorMapPlanesMapped),"ColorMap", colorMap);
			g_object_set_data(G_OBJECT(entryLeft),"ColorMap", colorMap);
			g_object_set_data(G_OBJECT(entryRight),"ColorMap", colorMap);
			g_object_set_data(G_OBJECT(darea),"ColorMap", colorMap);
			color_map_refresh(handleBoxColorMapPlanesMapped);
			if(GTK_WIDGET_VISIBLE(handleBoxColorMapPlanesMapped))
			{
				gtk_widget_hide(handleBoxColorMapPlanesMapped);
				gtk_widget_show(handleBoxColorMapPlanesMapped);
			}

		}
	}
}
/*********************************************************************************************************/
static void resetContoursColorMap()
{
	GtkWidget* handleBoxColorMapContours = g_object_get_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapContours");

	if(handleBoxColorMapContours)
	{
		GtkWidget* entryLeft  = g_object_get_data(G_OBJECT(handleBoxColorMapContours), "EntryLeft");
		GtkWidget* entryRight = g_object_get_data(G_OBJECT(handleBoxColorMapContours), "EntryRight");
		GtkWidget* darea      = g_object_get_data(G_OBJECT(handleBoxColorMapContours), "DrawingArea");
		ColorMap* colorMap = g_object_get_data(G_OBJECT( handleBoxColorMapContours),"ColorMap");
		if(colorMap && entryLeft && entryRight && darea && colorMap->numberOfColors>0)
		{
			gdouble minValue = colorMap->colorValue[0].value;
			gdouble maxValue = colorMap->colorValue[colorMap->numberOfColors-1].value;
			ColorMap* newColorMap = new_colorMap_min_max(minValue, maxValue);
			colormap_free(colorMap);
			g_free(colorMap);
			colorMap = newColorMap;
			g_object_set_data(G_OBJECT(handleBoxColorMapContours),"ColorMap", colorMap);
			g_object_set_data(G_OBJECT(entryLeft),"ColorMap", colorMap);
			g_object_set_data(G_OBJECT(entryRight),"ColorMap", colorMap);
			g_object_set_data(G_OBJECT(darea),"ColorMap", colorMap);
			color_map_refresh(handleBoxColorMapContours);
			if(GTK_WIDGET_VISIBLE(handleBoxColorMapContours))
			{
				gtk_widget_hide(handleBoxColorMapContours);
				gtk_widget_show(handleBoxColorMapContours);
			}
		}
	}
}
/*********************************************************************************************************/
void resetAllColorMapOrb()
{
	resetGridColorMap();
	resetContoursColorMap();
	resetPlanesMappedColorMap();
	resetBeginNegative();
	glarea_rafresh(GLArea);
}
/*********************************************************************************************************************/
static void set_alphaFED(GtkWidget *button,gpointer data)
{
	GtkWidget* entry = (GtkWidget*)data;
	G_CONST_RETURN gchar* temp;
	gchar* dump = NULL;
	GtkWidget* Win = g_object_get_data (G_OBJECT (button), "Win");

	if(!GTK_IS_WIDGET(data)) return;

       	temp	= gtk_entry_get_text(GTK_ENTRY(entry)); 
	if(temp && strlen(temp)>0)
	{
		dump = g_strdup(temp);
		delete_first_spaces(dump);
		delete_last_spaces(dump);
	}

	if(dump && strlen(dump)>0 && this_is_a_real(dump) && atof(dump)>=0 && atof(dump)<=100)
	{
		alphaFED = atof(dump);
		if(dump) g_free(dump);
		gtk_widget_destroy(Win);
	}
	else
	{
		GtkWidget* message = Message(_("Error : alpha should be a real between 0 and 100 "),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		if(dump) g_free(dump);
		gtk_window_set_transient_for(GTK_WINDOW(message),GTK_WINDOW(Win));
		return;
	}
}
/*********************************************************************/
GtkWidget* set_alphaFED_dialog ()
{
	GtkWidget *fp;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget *vboxframe;
	GtkWidget *hbox;
	GtkWidget *button;
	GtkWidget *label;
	GtkWidget* entry;
	GtkWidget *hseparator;
	gchar* tlabel="Alpha(eV^-1) : ";
	gchar* val = NULL;
	gchar* info = 
		"f (x,y,z) = (2 - n)/2{\n" 
	      	"      [sum_j(1 to N) O_j  Phi_j (x,y,z)^2 e^(-alpha(e_HOMO -e_j ))]/\n"
	        "     [sum_j(1 to N) O_j   e^(-alpha(e_HOMO -e_j ))]\n"
      		"}\n"
      		"+ n/2 {\n" 
	        "      [sum_j(1 to N) (2-O_j)  Phi_j (x,y,z)^2 e^(+alpha(e_LUMO -e_j ))]/\n"
	        "      [sum_j(1 to N) (2-O_j)   e^(+alpha(e_LUMO -e_j ))]\n"
      		"}\n\n"
		"n  = 0 for an electrophilic reaction,\n"
     		"     1 for a radical reaction, and\n"
     		"     2 for a nucleophilic reaction.\n"
		"N is the number of orbitals.\n" 
		"O_j is the number of electrons in orbital j.\n" 
		"Phi_j(x,y,z) is the value of the orbital j at point (x,y,z).\n"
		"e_j is the energy of orbital j.\n";

	fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_modal(GTK_WINDOW(fp),TRUE);
	gtk_window_set_title(GTK_WINDOW(fp),_("Set alpha for FED calculation"));
	gtk_container_set_border_width (GTK_CONTAINER (fp), 5);

	gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
	gtk_window_set_modal (GTK_WINDOW (fp), TRUE);

	g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)gtk_widget_destroy,NULL);

	vboxall = create_vbox(fp);
	frame = gtk_frame_new (NULL);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);

	hbox = create_hbox(vboxframe);
	label = gtk_label_new (info);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, FALSE, 0);

	hseparator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vboxframe), hseparator, TRUE, FALSE, 0);

	hbox = create_hbox(vboxframe);
	label = gtk_label_new (tlabel);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, FALSE, 0);

	entry = gtk_entry_new ();
	gtk_widget_show (entry);
	gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, TRUE, 0);
	val = g_strdup_printf("%f",alphaFED);
       	gtk_entry_set_text(GTK_ENTRY(entry),val);
	if(val) g_free(val);

	hbox = create_hbox(vboxall);

	button = create_button(PrincipalWindow,_("OK"));
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(set_alphaFED),(gpointer)entry);
	g_object_set_data (G_OBJECT (button), "Win", fp);
	gtk_widget_show (button);

	button = create_button(PrincipalWindow,_("Cancel"));
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(fp));

	gtk_widget_show (button);
   
	gtk_widget_show_all(fp);
	return fp;
}
