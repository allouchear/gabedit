/* ResultsAnalise.c */
/**********************************************************************************************************
Copyright (c) 2002-2013 Abdul-Rahman Allouche. All rights reserved

Permission is hereby granted, free of gcharge, to any person obtaining a copy of this software and associated
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
#include <math.h>
#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "../Geometry/ResultsAnalise.h"
#include "../Geometry/EnergiesCurves.h"
#include "../Common/Run.h"
#include "../Display/ViewOrb.h"

/*********************************************************************/
DataGeomConv free_geom_conv(DataGeomConv GeomConv)
{
 gint i,j;

 for(i = 0;i<GeomConv.Ntype;i++)
		if(GeomConv.TypeData[i])
			g_free(GeomConv.TypeData[i]);

 if(GeomConv.GeomFile)
		g_free(GeomConv.GeomFile);
 GeomConv.GeomFile = NULL;
 if(GeomConv.TypeData)
		g_free(GeomConv.TypeData);
 GeomConv.TypeData = NULL;

 if(GeomConv.TypeCalcul)
		g_free(GeomConv.TypeCalcul);
 GeomConv.TypeCalcul = NULL;

  if( GeomConv.Npoint<1)
  {
	if(GeomConv.Data)
		g_free(GeomConv.Data);
        GeomConv.Data = NULL;
	if(GeomConv.NumGeom)
		g_free(GeomConv.NumGeom);
	GeomConv.NumGeom = NULL;
	return GeomConv;
  }
 for(j = 0;j<GeomConv.Ntype;j++)
 {
  	if(GeomConv.Data[j])
  	for(i = 0;i<GeomConv.Npoint;i++)
		if(GeomConv.Data[j][i])
			g_free(GeomConv.Data[j][i]);

  	if(GeomConv.Data[j])
		g_free(GeomConv.Data[j]);

  	GeomConv.Data[j] = NULL;
 }  
 GeomConv.Npoint = 0;
 GeomConv.Ntype  = 0;

 return GeomConv;
}
/*********************************************************************/
DataGeomConv init_geom_dalton_conv(gchar *namefile)
{
	DataGeomConv GeomConv;
	gint i;
	GeomConv.Npoint = 0;
	GeomConv.Ntype  = 2;
	GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
	GeomConv.fileType = GABEDIT_TYPEFILE_DALTON;
	GeomConv.TypeData[0] = g_strdup(" Energy ");
	GeomConv.TypeData[1] = g_strdup(" Norm Step ");
	GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
	for(i = 0;i<GeomConv.Ntype;i++) GeomConv.Data[i] = NULL;
	GeomConv.TypeCalcul = NULL;
	GeomConv.NumGeom = NULL;
	GeomConv.GeomFile = g_strdup(namefile);

	return GeomConv;
}
/*********************************************************************/
DataGeomConv init_geom_mopac_conv(gchar *namefile)
{
	DataGeomConv GeomConv;
	gint i;
	GeomConv.Npoint = 0;
	GeomConv.Ntype  = 2;
	GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
	GeomConv.fileType = GABEDIT_TYPEFILE_MOPAC;
	GeomConv.TypeData[0] = g_strdup(_(" Energy(KCal/Mol) "));
	GeomConv.TypeData[1] = g_strdup(_(" Gradient "));
	GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
	for(i = 0;i<GeomConv.Ntype;i++) GeomConv.Data[i] = NULL;
	GeomConv.TypeCalcul = NULL;
	GeomConv.NumGeom = NULL;
	GeomConv.GeomFile = g_strdup(namefile);

	return GeomConv;
}
/*********************************************************************/
gint find_energy_mopac_aux(gchar* NomFichier)
{
	guint  i=0;
	guint  j=0;
	guint  k=0;
	gchar *temp =  g_malloc(50*sizeof(gchar));	
	gchar *tmp =  g_malloc(50*sizeof(gchar));	
	guint taille=BSIZE;
	gchar *t;
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gboolean OK;

        
	t=g_malloc(taille);
 	fd = FOpen(NomFichier, "rb"); 
        if(!fd)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),NomFichier);
		Message(t,_("Error"),TRUE);
		if(t) g_free(t);
		return -1;
	}
        
	OK = TRUE;
	while(!feof(fd))
	{
    		 { gchar* e = fgets(t,taille,fd);}
                 if(strstr(t,"Geometry optimization") && strstr(t,"#") && Ncalculs <1)
		 {
         		Ncalculs = 1;
                        GeomConv =  g_malloc(sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_mopac_conv(NomFichier);
		 }
                 if(strstr(t,"Geometry optimization") && strstr(t,"#") )
		 {
			OK = TRUE;
			while(!feof(fd) && OK )
			{
		 		if(!fgets(t, BSIZE,fd)) { OK = FALSE; break; }
				if(strstr(t,"####################################"))
				{
					OK = TRUE;
					break;
				}
		 	}
		 }
		 if(!OK) break;
		 if(Ncalculs>0)
		 {

			OK = TRUE;
			while(!feof(fd) && OK )
			{
		 		if(!fgets(t, BSIZE,fd)) { OK = FALSE; break; }
				if(strstr(t,"HEAT_OF_FORM_UPDATED"))
				{
		 			gchar* t1 = strstr(t,"=");
					gint c;
					for(c=0;c<strlen(t);c++) if(t[c]=='D'||t[c]=='d')t[c]='e';
					if(t1) sscanf(t1+1,"%s",tmp); /* energy */
					else { OK = FALSE; break; }
					OK = TRUE;
					break;
				}
		 	}
		 	if(!OK) break;
			OK = TRUE;
			while(!feof(fd) && OK )
			{
		 		if(!fgets(t, BSIZE,fd)) { OK = FALSE; break; }
				if(strstr(t,"GRADIENT_UPDATED"))
				{
		 			gchar* t1 = strstr(t,"=");
					if(t1) sscanf(t1+1,"%s",temp); /* gradient */
					else { OK = FALSE; break; }
					OK = TRUE;
					break;
				}
		 	}
		 	if(!OK) break;
		
		  	GeomConv[Ncalculs-1].Npoint++;
		  	if(GeomConv[Ncalculs-1].Npoint == 1 )
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++) GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  	}
		  	else
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  	}
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
                 	{
		 		GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
                 		GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1][0] = '\0';
                 	}
		 	GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
		 	GeomConv[Ncalculs-1].TypeCalcul = g_strdup(" ");
                 	sprintf(GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1], "%s",tmp);
                 	sprintf(GeomConv[Ncalculs-1].Data[1][GeomConv[Ncalculs-1].Npoint-1], "%s",temp);
		}
	}
	fclose(fd);
   
	if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
	{
		GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
		Ncalculs--;
		if(Ncalculs>0) GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
		else
		{
			g_free(GeomConv);
			GeomConv =  NULL;
		}
	}
	k = 0;
	for(i=0;(gint)i<Ncalculs;i++)
   	for(j=0;(gint)j<GeomConv[i].Npoint;j++)
	{
	   k++;
	   GeomConv[i].NumGeom[j] = k;
	}
	create_energies_curves(GeomConv,Ncalculs);
	g_free(t);
	g_free(temp);
	g_free(tmp);
	if( Ncalculs<=0) return 1;
	if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 1) return 1;
	return 0;
}
/*********************************************************************/
DataGeomConv init_geom_mopac_irc_conv(gchar *namefile)
{
	DataGeomConv GeomConv;
	gint i;
	GeomConv.Npoint = 0;
	GeomConv.Ntype  = 3;
	GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
	GeomConv.fileType = GABEDIT_TYPEFILE_MOPAC_IRC;
	GeomConv.TypeData[0] = g_strdup(" Energy(KCal/Mol) ");
	GeomConv.TypeData[1] = g_strdup(" Potential");
	GeomConv.TypeData[2] = g_strdup(" Energy lost");
	GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
	for(i = 0;i<GeomConv.Ntype;i++) GeomConv.Data[i] = NULL;
	GeomConv.TypeCalcul = NULL;
	GeomConv.NumGeom = NULL;
	GeomConv.GeomFile = g_strdup(namefile);

	return GeomConv;
}
/*********************************************************************/
void find_energy_mopac_irc_output(gchar* NomFichier)
{
	guint  i=0;
	guint  j=0;
	guint  k=0;
	gchar dum[100];
	gchar *elost =  g_malloc(50*sizeof(gchar));
	gchar *potential =  g_malloc(50*sizeof(gchar));
	gchar *e =  g_malloc(50*sizeof(gchar));
	guint taille=BSIZE;
	gchar *t;
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gboolean OK;

        
	t=g_malloc(taille*sizeof(gchar));
 	fd = FOpen(NomFichier, "rb"); 
        if(!fd)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),NomFichier);
		Message(t,_("Error"),TRUE);
		if(t)
			g_free(t);
		return;
	}
        
	OK = TRUE;
	while(!feof(fd))
	{
		 if(!fgets(t,taille,fd))break;
                 if(strstr(t,"INTRINSIC REACTION COORDINATE") && Ncalculs <1)
		 {
         		Ncalculs = 1;
                        GeomConv =  g_malloc(sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_mopac_irc_conv(NomFichier);
		 	GeomConv[Ncalculs-1].TypeCalcul = g_strdup(" ");
			break;
		 }
	}
	if(Ncalculs>0)
	{
		OK = TRUE;
		while(!feof(fd) && OK )
		{
	 		if(!fgets(t, BSIZE,fd)) { OK = FALSE; break; }
			if(
				strstr(t,"POTENTIAL") && 
				strstr(t,"LOST") &&
				strstr(t,"TOTAL")
			) 
			{
		   		sprintf(elost," ");
		   		sprintf(potential," ");
		   		sprintf(e," ");
	 			if(!fgets(t, BSIZE,fd)) { OK = FALSE; break; }
				gint c;
				for(c=0;c<strlen(t);c++) if(t[c]=='D'||t[c]=='d')t[c]='e';
				if(4!=sscanf(t,"%s %s %s %s",dum,potential,elost,e))
				{ OK = FALSE; break; }
			}
			else continue;
		  	GeomConv[Ncalculs-1].Npoint++;
		  	if(GeomConv[Ncalculs-1].Npoint == 1 )
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++) 
					GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  	}
		  	else
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  	}
		 	GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1] = g_strdup(e);
		 	GeomConv[Ncalculs-1].Data[1][GeomConv[Ncalculs-1].Npoint-1] = g_strdup(elost);
		 	GeomConv[Ncalculs-1].Data[2][GeomConv[Ncalculs-1].Npoint-1] = g_strdup(potential);
		}
	}
	fclose(fd);
   
	if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
	{
		GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
		Ncalculs--;
		if(Ncalculs>0) GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
		else
		{
			g_free(GeomConv);
			GeomConv =  NULL;
		}
	}
	k = 0;
	for(i=0;(gint)i<Ncalculs;i++)
   	for(j=0;(gint)j<GeomConv[i].Npoint;j++)
	{
	   k++;
	   GeomConv[i].NumGeom[j] = k;
	}
	create_energies_curves(GeomConv,Ncalculs);
	g_free(t);
	g_free(elost);
	g_free(potential);
	g_free(e);
}
/*********************************************************************/
DataGeomConv init_geom_mopac_scan_conv(gchar *namefile)
{
	DataGeomConv GeomConv;
	gint i;
	GeomConv.Npoint = 0;
	GeomConv.Ntype  = 3;
	GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
	GeomConv.fileType = GABEDIT_TYPEFILE_MOPAC_SCAN;
	GeomConv.TypeData[0] = g_strdup(" Energy(KCal/Mol) ");
	GeomConv.TypeData[1] = g_strdup(" Variable 1");
	GeomConv.TypeData[2] = g_strdup(" Variable 2");
	GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
	for(i = 0;i<GeomConv.Ntype;i++) GeomConv.Data[i] = NULL;
	GeomConv.TypeCalcul = NULL;
	GeomConv.NumGeom = NULL;
	GeomConv.GeomFile = g_strdup(namefile);

	return GeomConv;
}
/*********************************************************************/
void find_energy_mopac_scan_output(gchar* NomFichier)
{
	guint  i=0;
	guint  j=0;
	guint  k=0;
	gchar *var1 =  g_malloc(50*sizeof(gchar));
	gchar *var2 =  g_malloc(50*sizeof(gchar));
	gchar *e =  g_malloc(50*sizeof(gchar));
	guint taille=BSIZE;
	gchar *t;
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gboolean OK;

        
	t=g_malloc(taille*sizeof(gchar));
 	fd = FOpen(NomFichier, "rb"); 
        if(!fd)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),NomFichier);
		Message(t,_("Error"),TRUE);
		if(t)
			g_free(t);
		return;
	}
        
	OK = TRUE;
	while(!feof(fd))
	{
		 if(!fgets(t,taille,fd))break;
                 if(strstr(t,"POINT") && (strstr(t,"NUMBER OF POINTS") || strstr(t,"NUMBER OF ROWS")) && Ncalculs <1)
		 {
         		Ncalculs = 1;
                        GeomConv =  g_malloc(sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_mopac_scan_conv(NomFichier);
		 	GeomConv[Ncalculs-1].TypeCalcul = g_strdup(" ");
			break;
		 }
	}
	if(Ncalculs>0)
	{
		OK = TRUE;
		while(!feof(fd) && OK )
		{
	 		if(!fgets(t, BSIZE,fd)) { OK = FALSE; break; }
			if(strstr(t,"VARIABLE")&& strstr(t,"FUNCTION"))
			{
		   		sprintf(var1," ");
		   		sprintf(var2," ");
		   		sprintf(e," ");
	 			if(!fgets(t, BSIZE,fd)) { OK = FALSE; break; }
	 			gchar* t1 = strstr(t,":");
				gint c;
				for(c=0;c<strlen(t);c++) if(t[c]=='D'||t[c]=='d')t[c]='e';
				if(t1) 
				{
					if(3!=sscanf(t1+1,"%s%s%s",var1,var2,e))
					{
					   sscanf(t1+1,"%s%s",var1,e);
					   sprintf(var2," ");
					}
				}
				else { OK = FALSE; break; }
			}
			else continue;
		  	GeomConv[Ncalculs-1].Npoint++;
		  	if(GeomConv[Ncalculs-1].Npoint == 1 )
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++) 
					GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  	}
		  	else
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  	}
		 	GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1] = g_strdup(e);
		 	GeomConv[Ncalculs-1].Data[1][GeomConv[Ncalculs-1].Npoint-1] = g_strdup(var1);
		 	GeomConv[Ncalculs-1].Data[2][GeomConv[Ncalculs-1].Npoint-1] = g_strdup(var2);
		}
	}
	fclose(fd);
   
	if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
	{
		GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
		Ncalculs--;
		if(Ncalculs>0) GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
		else
		{
			g_free(GeomConv);
			GeomConv =  NULL;
		}
	}
	k = 0;
	for(i=0;(gint)i<Ncalculs;i++)
   	for(j=0;(gint)j<GeomConv[i].Npoint;j++)
	{
	   k++;
	   GeomConv[i].NumGeom[j] = k;
	}
	create_energies_curves(GeomConv,Ncalculs);
	g_free(t);
	g_free(var1);
	g_free(var2);
	g_free(e);
}
/*********************************************************************/
DataGeomConv init_geom_mpqc_conv(gchar *namefile)
{
	DataGeomConv GeomConv;
	gint i;
	GeomConv.Npoint = 0;
	GeomConv.Ntype  = 3;
	GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
	GeomConv.fileType = GABEDIT_TYPEFILE_MPQC;
	GeomConv.TypeData[0] = g_strdup(_(" Energy "));
	GeomConv.TypeData[1] = g_strdup(_(" Max Gradient "));
	GeomConv.TypeData[2] = g_strdup(_(" Max Displacement "));
	GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
	for(i = 0;i<GeomConv.Ntype;i++) GeomConv.Data[i] = NULL;
	GeomConv.TypeCalcul = NULL;
	GeomConv.NumGeom = NULL;
	GeomConv.GeomFile = g_strdup(namefile);

	return GeomConv;
}
/*********************************************************************/
void find_energy_mpqc_output(gchar* NomFichier)
{
	gchar *pdest;
	guint  i=0;
	guint  j=0;
	guint  k=0;
	gchar *temp =  g_malloc(50*sizeof(gchar));	
	guint taille=BSIZE;
	gchar *t;
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gboolean newGeom = FALSE;
	gboolean mp2 = FALSE;

        
        Ncalculs++;
	GeomConv =  g_malloc(sizeof(DataGeomConv) );
  	GeomConv[Ncalculs-1] = init_geom_mpqc_conv(NomFichier);

	t=g_malloc(taille);
 	fd = FOpen(NomFichier, "rb"); 
        if(!fd)
	{
		g_free(t);
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),NomFichier);
		Message(t,_("Error"),TRUE);
		if(t) g_free(t);
		return;
	}
        
	 while(!feof(fd))
	{
		pdest = NULL;
    		 { gchar* e = fgets(t,taille,fd);}
		 /*
                 if( strlen(t)>2 && strstr(t,"changing atomic coordinates:") )
		 {
         		Ncalculs++;
			GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_mpqc_conv(NomFichier);
			GeomConv[Ncalculs-1].TypeCalcul = g_malloc(100*sizeof(gchar));
                 	sscanf(t,"%s",GeomConv[Ncalculs-1].TypeCalcul);
		 }
		 */
                 if(strstr(t,"changing atomic coordinates:"))
		 {
			GeomConv[Ncalculs-1].TypeCalcul = g_malloc(100*sizeof(gchar));
                 	sscanf(t,"%s",GeomConv[Ncalculs-1].TypeCalcul);
			newGeom = TRUE;
			if(strstr(t,"MBPT2")) mp2 = TRUE;
		 }

          	if(newGeom && mp2)
		{
			pdest = NULL;
    		 	pdest = strstr( t,"MP2");
    		 	if(pdest)
			{
				if(strstr(t,"correlation")) pdest = NULL;
				else pdest = strstr( t,"energy");
   				if( pdest != NULL ) pdest = strstr( t,":");
			}
		}
		if(newGeom && !mp2)
		{
			pdest = NULL;
			pdest = strstr( t,"total scf energy");
   			if( pdest != NULL ) pdest = strstr( t,"=");
		}

   		if( pdest != NULL )
		{
			pdest++;
			GeomConv[Ncalculs-1].Npoint++;
			if(GeomConv[Ncalculs-1].Npoint == 1 )
			{
				GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
					GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
			}
		  	else
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
					GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  	}
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
            		{
		 		GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
                 		GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1][0] = '\0';
                 	}
		 	GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
                 	sscanf(pdest,"%s", GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1]);
		}
			
                 pdest = NULL;
    		 pdest = strstr( t,"Max Gradient");
                 if( pdest != NULL && GeomConv[Ncalculs-1].Npoint>0)
		 {
    		 	pdest = strstr( t,":");
			if(pdest)
			{
				pdest++;
                 		sscanf(pdest,"%s",GeomConv[Ncalculs-1].Data[1][GeomConv[Ncalculs-1].Npoint-1]);
			}
		 }
    		 pdest = strstr( t,"Max Displacement");
                 if( pdest != NULL && GeomConv[Ncalculs-1].Npoint>0)
		 {
    		 	pdest = strstr( t,":");
			if(pdest)
			{
				pdest++;
                 		sscanf(pdest,"%s",GeomConv[Ncalculs-1].Data[2][GeomConv[Ncalculs-1].Npoint-1]);
			}
		 }
	}

	fclose(fd);
   
	if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
	{
		GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
		Ncalculs--;
		if(Ncalculs>0)
			GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
		else
		{
			g_free(GeomConv);
			GeomConv =  NULL;
		}
	}

	/*
    printf("Npoint  = %d\n ",GeomConv[Ncalculs-1].Npoint);
    printf("TypeCalcul  = %s\n ",GeomConv[Ncalculs-1].TypeCalcul);
   for(i=0;i<GeomConv[Ncalculs-1].Npoint;i++)
   {
   	for(j=0;j<GeomConv[Ncalculs-1].Ntype;j++)
          	printf("%s ",GeomConv[Ncalculs-1].Data[j][i]);
       	printf("\n ");
	
   }
   */

	k = 0;
	for(i=0;(gint)i<Ncalculs;i++)
	{
		for(j=0;(gint)j<GeomConv[i].Npoint;j++)
		{
			k++;
			GeomConv[i].NumGeom[j] = k;
		}
		k++;
	}
	create_energies_curves(GeomConv,Ncalculs);
	g_free(t);
	g_free(temp);
}
/*********************************************************************/
DataGeomConv init_geom_xyz_conv(gchar *namefile)
{
	DataGeomConv GeomConv;
	GeomConv.Npoint = 0;
	GeomConv.Ntype  = 1;
	GeomConv.fileType = GABEDIT_TYPEFILE_XYZ;
	GeomConv.TypeCalcul = g_strdup(_("Geometries for an xyz file"));
	GeomConv.NumGeom = NULL;
	GeomConv.GeomFile = g_strdup(namefile);
	GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
	GeomConv.TypeData[0] = g_strdup(_(" Geometry number "));
	GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
	GeomConv.Data[0] = NULL;
	return GeomConv;
}
/*********************************************************************/
static void find_energy_xyz(gchar* fileName)
{
	gint  i=0;
	gint  j=0;
	guint taille=BSIZE;
	gchar t[BSIZE];
	FILE *file;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gint ne;
	gint nAtoms;

	GeomConv = NULL;
        
 	file = FOpen(fileName, "rb"); 
        if(!file)
	{
		sprintf(t,_(" Error : I can not open file %s\n"),fileName);
		Message(t,_("Error"),TRUE);
		return;
	}
        
	 while(!feof(file))
	{
		if(!fgets(t,taille,file))break;
		ne = sscanf(t,"%d",&nAtoms);
		if(ne==1 && nAtoms>0)
		 {
         		if(Ncalculs==0)
			{
				Ncalculs = 1;
				GeomConv =  g_malloc(sizeof(DataGeomConv) );
  				GeomConv[0] = init_geom_xyz_conv(fileName);
			}
		 	if(!fgets(t,taille,file)) break; /* title */
			GeomConv[0].Npoint++;
			for(i=0;i<nAtoms;i++)
				if(!fgets(t,taille,file))break;
		 }
		else
			break;
	}

	fclose(file);
   
	if(GeomConv)
	{
		if(GeomConv[0].Npoint == 0)
		{
			GeomConv[0] =  free_geom_conv(GeomConv[0]);
			g_free(GeomConv);
			GeomConv =  NULL;
			Ncalculs = 0;
		}
		else
		{
			GeomConv[0].NumGeom = g_malloc(GeomConv[0].Npoint*sizeof(gint));	
			GeomConv[0].Data[0] = g_malloc(GeomConv[0].Npoint*sizeof(gchar*));	
			for(j=0;(gint)j<GeomConv[0].Npoint;j++)
			{
				GeomConv[0].NumGeom[j] = j+1;
		 		GeomConv[0].Data[0][j] = g_strdup_printf("%d",j+1);
			}
		}
	}
	create_energies_curves(GeomConv,Ncalculs);
}
/*********************************************************************/
DataGeomConv init_geom_gauss_conv(gchar *namefile)
{
  DataGeomConv GeomConv;
  gint i;
  GeomConv.Npoint = 0;
  GeomConv.Ntype  = 5;
  GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
  GeomConv.fileType = GABEDIT_TYPEFILE_GAUSSIAN;
 GeomConv.TypeData[0] = g_strdup(_(" Energy "));
 GeomConv.TypeData[1] = g_strdup(_(" Force Max "));
 GeomConv.TypeData[2] = g_strdup(_(" Force RMS "));
 GeomConv.TypeData[3] = g_strdup(_(" Dep. Max "));
 GeomConv.TypeData[4] = g_strdup(_(" Dep. RMS "));
 GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
 for(i = 0;i<GeomConv.Ntype;i++)
 	GeomConv.Data[i] = NULL;
 GeomConv.TypeCalcul = NULL;
 GeomConv.NumGeom = NULL;
 GeomConv.GeomFile = g_strdup(namefile);

  return GeomConv;
}
/*********************************************************************/
DataGeomConv init_geom_molpro_conv(gchar* namefile)
{
  DataGeomConv GeomConv;
 gint i;
  GeomConv.Npoint = 0;
  GeomConv.Ntype  = 2;
  GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
  GeomConv.fileType = GABEDIT_TYPEFILE_MOLPRO;
 GeomConv.TypeData[0] = g_strdup(_(" Energy "));
 GeomConv.TypeData[1] = g_strdup(_(" Convergence "));
 GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
 for(i = 0;i<GeomConv.Ntype;i++)
 	GeomConv.Data[i] = NULL;
 GeomConv.TypeCalcul = NULL;
 GeomConv.NumGeom = NULL;
 GeomConv.GeomFile = g_strdup(namefile);
 return GeomConv;
}
/*********************************************************************/
void find_energy_dalton_output(gchar* NomFichier)
{
	guint  i=0;
	guint  j=0;
	guint  k=0;
	gchar *temp =  g_malloc(50*sizeof(gchar));	
	gchar *tmp =  g_malloc(50*sizeof(gchar));	
	guint taille=BSIZE;
	gchar *t;
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gboolean OK;

        
	t=g_malloc(taille);
 	fd = FOpen(NomFichier, "rb"); 
        if(!fd)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),NomFichier);
		Message(t,_("Error"),TRUE);
		if(t)
			g_free(t);
		return;
	}
        
	while(!feof(fd))
	{
    		 { gchar* e = fgets(t,taille,fd);}
                 if(strstr(t,"Optimization Control Center") && Ncalculs <1)
		 {
         		Ncalculs = 1;
                        GeomConv =  g_malloc(sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_dalton_conv(NomFichier);
		 }
                 if(strstr(t,"Optimization Control Center") )
		 {
			OK = TRUE;
			while(!feof(fd) && OK )
			{
		 		if(!fgets(t, BSIZE,fd)) { OK = FALSE; break; }
				if(strstr(t,"Next geometry") || strstr(t,"Final geometry"))
				{
		 			if(!fgets(t, BSIZE,fd)) { OK = FALSE; break; }
		 			if(!fgets(t, BSIZE,fd)) { OK = FALSE; break; }
					OK = TRUE;
					break;
				}
		 	}
		 	if(!OK) break;

			OK = TRUE;
			while(!feof(fd) && OK )
			{
		 		if(!fgets(t, BSIZE,fd)) { OK = FALSE; break; }
				if(strstr(t,"Energy at this geometry is"))
				{
		 			gchar* t1 = strstr(t,":");
					if(t1) sscanf(t1+1,"%s",tmp); /* energy */
					else { OK = FALSE; break; }
					OK = TRUE;
					break;
				}
		 	}
		 	if(!OK) break;
			OK = TRUE;
			while(!feof(fd) && OK )
			{
		 		if(!fgets(t, BSIZE,fd)) { OK = FALSE; break; }
				if(strstr(t,"Norm of step"))
				{
		 			gchar* t1 = strstr(t,":");
					if(t1) sscanf(t1+1,"%s",temp); /* rmsStep */
					else { OK = FALSE; break; }
					OK = TRUE;
					break;
				}
		 	}
		 	if(!OK) break;
		
		  	GeomConv[Ncalculs-1].Npoint++;
		  	if(GeomConv[Ncalculs-1].Npoint == 1 )
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++) GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  	}
		  	else
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  	}
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
                 	{
		 		GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
                 		GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1][0] = '\0';
                 	}
		 	GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
		 	GeomConv[Ncalculs-1].TypeCalcul = g_strdup(" ");
                 	sprintf(GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1], "%s", tmp);
                 	sprintf(GeomConv[Ncalculs-1].Data[1][GeomConv[Ncalculs-1].Npoint-1], "%s", temp);
		}
	}
	fclose(fd);
   
	if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
	{
		GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
		Ncalculs--;
		if(Ncalculs>0) GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
		else
		{
			g_free(GeomConv);
			GeomConv =  NULL;
		}
	}
	k = 0;
	for(i=0;(gint)i<Ncalculs;i++)
   	for(j=0;(gint)j<GeomConv[i].Npoint;j++)
	{
	   k++;
	   GeomConv[i].NumGeom[j] = k;
	}
	create_energies_curves(GeomConv,Ncalculs);
	g_free(t);
	g_free(temp);
	g_free(tmp);
}
/*********************************************************************/
DataGeomConv init_geom_gamess_conv(gchar *namefile)
{
	DataGeomConv GeomConv;
	gint i;
	GeomConv.Npoint = 0;
	GeomConv.Ntype  = 3;
	GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
	GeomConv.fileType = GABEDIT_TYPEFILE_GAMESS;
	GeomConv.TypeData[0] = g_strdup(_(" Energy "));
	GeomConv.TypeData[1] = g_strdup(_(" MAX Gradient "));
	GeomConv.TypeData[2] = g_strdup(_(" RMS Gradient "));
	GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
	for(i = 0;i<GeomConv.Ntype;i++) GeomConv.Data[i] = NULL;
	GeomConv.TypeCalcul = NULL;
	GeomConv.NumGeom = NULL;
	GeomConv.GeomFile = g_strdup(namefile);

	return GeomConv;
}
/*********************************************************************/
void find_energy_gamess_output(gchar* NomFichier)
{
	guint  i=0;
	guint  j=0;
	guint  k=0;
	gchar *maxgrad =  g_malloc(50*sizeof(gchar));	
	gchar *rmsgrad =  g_malloc(50*sizeof(gchar));	
	gchar *tmp =  g_malloc(50*sizeof(gchar));	
	guint taille=BSIZE;
	gchar *t;
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gboolean OK;

        
	t=g_malloc(taille);
 	fd = FOpen(NomFichier, "rb"); 
        if(!fd)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),NomFichier);
		Message(t,_("Error"),TRUE);
		if(t)
			g_free(t);
		return;
	}
        
	while(!feof(fd))
	{
    		 { gchar* e = fgets(t,taille,fd);}
                 if(strstr(t,"NSERCH=   0") && Ncalculs <1)
		 {
         		Ncalculs = 1;
                        GeomConv =  g_malloc(sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_gamess_conv(NomFichier);
		 }
                 if(strstr(t,"COORDINATES OF ALL ATOMS ARE (ANGS)") )
		 {
			OK = TRUE;
			while(!feof(fd) && OK )
			{
		 		if(!fgets(t, BSIZE,fd)) { OK = FALSE; break; }
				if(strstr(t,"NSERCH") && strstr(t,"ENERGY="))
				{
		 			gchar* t1 = strstr(t,"ENERGY=");
					if(t1) sscanf(t1+7,"%s",tmp); /* energy */
					else { OK = FALSE; break; }
					OK = TRUE;
					break;
				}
		 	}
		 	if(!OK) break;
			OK = TRUE;
			while(!feof(fd) && OK )
			{
		 		if(!fgets(t, BSIZE,fd)) { OK = FALSE; break; }
				if(strstr(t,"MAXIMUM GRADIENT =")&& strstr(t,"RMS GRADIENT ="))
				{
		 			gchar* t1 = strstr(t,"MAXIMUM GRADIENT =");
					if(t1) sscanf(t1+19,"%s",maxgrad); /* maxGrad */
					else { OK = FALSE; break; }
		 			t1 = strstr(t,"RMS GRADIENT =");
					if(t1) sscanf(t1+15,"%s",rmsgrad); /* rmsGrad */
					else { OK = FALSE; break; }
					OK = TRUE;
					break;
				}
		 	}
		 	if(!OK) break;
		
		  	GeomConv[Ncalculs-1].Npoint++;
		  	if(GeomConv[Ncalculs-1].Npoint == 1 )
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++) GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  	}
		  	else
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  	}
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
                 	{
		 		GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
                 		GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1][0] = '\0';
                 	}
		 	GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
		 	GeomConv[Ncalculs-1].TypeCalcul = g_strdup(" ");
                 	sprintf(GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1], "%s", tmp);
                 	sprintf(GeomConv[Ncalculs-1].Data[1][GeomConv[Ncalculs-1].Npoint-1], "%s", maxgrad);
                 	sprintf(GeomConv[Ncalculs-1].Data[2][GeomConv[Ncalculs-1].Npoint-1], "%s", rmsgrad);
		}
	}
	fclose(fd);
   
	if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
	{
		GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
		Ncalculs--;
		if(Ncalculs>0) GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
		else
		{
			g_free(GeomConv);
			GeomConv =  NULL;
		}
	}
	k = 0;
	for(i=0;(gint)i<Ncalculs;i++)
   	for(j=0;(gint)j<GeomConv[i].Npoint;j++)
	{
	   k++;
	   GeomConv[i].NumGeom[j] = k+1;
	}
	create_energies_curves(GeomConv,Ncalculs);
	g_free(t);
	g_free(tmp);
	g_free(maxgrad);
	g_free(rmsgrad);
}
/*********************************************************************/
void find_energy_gamess_output_heat(gchar* NomFichier)
{
	guint  i=0;
	guint  j=0;
	guint  k=0;
	gchar *maxgrad =  g_malloc(50*sizeof(gchar));	
	gchar *rmsgrad =  g_malloc(50*sizeof(gchar));	
	gchar *tmp =  g_malloc(50*sizeof(gchar));	
	guint taille=BSIZE;
	gchar *t;
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gboolean OK;
	gchar* myStr = "HEAT OF FORMATION IS";
	gint lmyStr = strlen(myStr);

        
	t=g_malloc(taille);
 	fd = FOpen(NomFichier, "rb"); 
        if(!fd)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),NomFichier);
		Message(t,_("Error"),TRUE);
		if(t)
			g_free(t);
		return;
	}
        
	while(!feof(fd))
	{
    		 { gchar* e = fgets(t,taille,fd);}
                 if(strstr(t,"NSERCH=   0") && Ncalculs <1)
		 {
         		Ncalculs = 1;
                        GeomConv =  g_malloc(sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_gamess_conv(NomFichier);
		 }
                 if(strstr(t,"COORDINATES OF ALL ATOMS ARE (ANGS)") )
		 {
			OK = TRUE;
			while(!feof(fd) && OK )
			{
		 		if(!fgets(t, BSIZE,fd)) { OK = FALSE; break; }
				if(strstr(t,myStr))
				{
		 			gchar* t1 = strstr(t,myStr);
					if(t1) sscanf(t1+lmyStr,"%s",tmp); /* energy */
					else { OK = FALSE; break; }
					OK = TRUE;
					break;
				}
		 	}
		 	if(!OK) break;
			OK = TRUE;
			while(!feof(fd) && OK )
			{
		 		if(!fgets(t, BSIZE,fd)) { OK = FALSE; break; }
				if(strstr(t,"MAXIMUM GRADIENT =")&& strstr(t,"RMS GRADIENT ="))
				{
		 			gchar* t1 = strstr(t,"MAXIMUM GRADIENT =");
					if(t1) sscanf(t1+19,"%s",maxgrad); /* maxGrad */
					else { OK = FALSE; break; }
		 			t1 = strstr(t,"RMS GRADIENT =");
					if(t1) sscanf(t1+15,"%s",rmsgrad); /* rmsGrad */
					else { OK = FALSE; break; }
					OK = TRUE;
					break;
				}
		 	}
		 	if(!OK) break;
		
		  	GeomConv[Ncalculs-1].Npoint++;
		  	if(GeomConv[Ncalculs-1].Npoint == 1 )
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++) GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  	}
		  	else
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  	}
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
                 	{
		 		GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
                 		GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1][0] = '\0';
                 	}
		 	GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
		 	GeomConv[Ncalculs-1].TypeCalcul = g_strdup(" ");
                 	sprintf(GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1], "%s", tmp);
                 	sprintf(GeomConv[Ncalculs-1].Data[1][GeomConv[Ncalculs-1].Npoint-1], "%s", maxgrad);
                 	sprintf(GeomConv[Ncalculs-1].Data[2][GeomConv[Ncalculs-1].Npoint-1], "%s", rmsgrad);
		}
	}
	fclose(fd);
   
	if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
	{
		GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
		Ncalculs--;
		if(Ncalculs>0) GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
		else
		{
			g_free(GeomConv);
			GeomConv =  NULL;
		}
	}
	k = 0;
	for(i=0;(gint)i<Ncalculs;i++)
   	for(j=0;(gint)j<GeomConv[i].Npoint;j++)
	{
	   k++;
	   GeomConv[i].NumGeom[j] = k+1;
	}
	create_energies_curves(GeomConv,Ncalculs);
	g_free(t);
	g_free(tmp);
	g_free(maxgrad);
	g_free(rmsgrad);
}
/*********************************************************************/
DataGeomConv init_geom_gamess_irc(gchar *namefile)
{
	DataGeomConv GeomConv;
	gint i;
	GeomConv.Npoint = 0;
	GeomConv.Ntype  = 1;
	GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
	GeomConv.fileType = GABEDIT_TYPEFILE_GAMESSIRC;
	GeomConv.TypeData[0] = g_strdup(_(" Energy "));
	GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
	for(i = 0;i<GeomConv.Ntype;i++) GeomConv.Data[i] = NULL;
	GeomConv.TypeCalcul = NULL;
	GeomConv.NumGeom = NULL;
	GeomConv.GeomFile = g_strdup(namefile);

	return GeomConv;
}
/*********************************************************************/
void find_energy_gamess_irc(gchar* NomFichier)
{
	guint  i=0;
	guint  j=0;
	guint  k=0;
	gchar *tmp =  g_malloc(50*sizeof(gchar));	
	guint taille=BSIZE;
	gchar *t;
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;

        
	t=g_malloc(taille*sizeof(gchar));
 	fd = FOpen(NomFichier, "rb"); 
        if(!fd)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),NomFichier);
		Message(t,_("Error"),TRUE);
		if(t)
			g_free(t);
		return;
	}
        
	while(!feof(fd))
	{
		 if(!fgets(t,taille,fd))break;
                 if(strstr(t,"POINT=") && Ncalculs <1)
		 {
         		Ncalculs = 1;
                        GeomConv =  g_malloc(sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_gamess_irc(NomFichier);
		 }
                 if(strstr(t,"POINT=") && strstr(t,"E=") )
		 {
		 	gchar* t1 = strstr(t,"E=");
			sscanf(t1+2,"%s",tmp); /* energy */
		}
		else continue;
		
	  	GeomConv[Ncalculs-1].Npoint++;
	  	if(GeomConv[Ncalculs-1].Npoint == 1 )
	  	{
			GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++) GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
	  	}
	  	else
	  	{
			GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
			GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
	  	}
		for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
               	{
	 		GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
               		GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1][0] = '\0';
               	}
	 	GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
	 	GeomConv[Ncalculs-1].TypeCalcul = g_strdup(" ");
               	sprintf(GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1], "%s", tmp);
	}
	fclose(fd);
   
	if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
	{
		GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
		Ncalculs--;
		if(Ncalculs>0) GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
		else
		{
			g_free(GeomConv);
			GeomConv =  NULL;
		}
	}
	k = 0;
	for(i=0;(gint)i<Ncalculs;i++)
   	for(j=0;(gint)j<GeomConv[i].Npoint;j++)
	{
	   k++;
	   GeomConv[i].NumGeom[j] = k;
	}
	create_energies_curves(GeomConv,Ncalculs);
	g_free(t);
	g_free(tmp);
}
/*********************************************************************/
void find_energy_gauss_standard_output(gchar* NomFichier)
{
	gchar* pdest = NULL;
	guint  i=0;
	guint  j=0;
	guint  k=0;
	gchar *temp =  g_malloc(50*sizeof(gchar));	
	guint taille=BSIZE;
	gchar *t;
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;

        
        Ncalculs++;
/*
	if(GeomConv) 
		*GeomConv =  free_geom_conv(*GeomConv);
        else
*/
		GeomConv =  g_malloc(sizeof(DataGeomConv) );
		
  	GeomConv[Ncalculs-1] = init_geom_gauss_conv(NomFichier);

	t=g_malloc(taille);
 	fd = FOpen(NomFichier, "rb"); 
        if(!fd)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),NomFichier);
		Message(t,_("Error"),TRUE);
		if(t)
			g_free(t);
		return;
	}
        
	 while(!feof(fd))
	{
    		 { gchar* e = fgets(t,taille,fd);}
                 if( strlen(t)>2 && strstr(t,"Normal termination of Gaussian") )
		 {
         		Ncalculs++;
			GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_gauss_conv(NomFichier);
		 }
		 uppercase(t);
                 if( strlen(t)>2 && t[1] == '#' && !GeomConv[Ncalculs-1].TypeCalcul)
		 {
			GeomConv[Ncalculs-1].TypeCalcul = g_malloc(100*sizeof(gchar));
                 	sscanf(t,"%s %s", temp, GeomConv[Ncalculs-1].TypeCalcul);
		 }
    		 pdest = strstr( t,"SCF DONE");
   		if( pdest != NULL )
		{
    		 pdest = strstr( t,"=");
		}
          	if(!pdest)
		{
    		 pdest = strstr( t,"ENERGY=");
          	 if(pdest && t[1] == 'E' )
    		 	pdest = strstr( t,"=");
                 else
    		 	pdest = NULL;
			
		}

   		if( pdest != NULL )
		{
		  pdest++;
		  GeomConv[Ncalculs-1].Npoint++;
		  if(GeomConv[Ncalculs-1].Npoint == 1 )
		  {
			GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  }
		  else
		  {
			GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  }
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
            {
		 	GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
                 	GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1][0] = '\0';
                 }
		 GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
                 sscanf(pdest,"%s", GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1]);
		}
			
                 pdest = NULL;
    		 pdest = strstr( t,"CONVERGED?");
                 if( pdest != NULL && GeomConv[Ncalculs-1].Npoint>0)
		 {
			for(i=1;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
			{
    		 		{ gchar* e = fgets(t,taille,fd);}
                 		sscanf(t,"%s %s %s", temp,temp,GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1]);
			}
		 }
	}

    fclose(fd);
   
    if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
    {
	GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
	Ncalculs--;
	if(Ncalculs>0)
		GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
	else
	{
		g_free(GeomConv);
		GeomConv =  NULL;
	}
    }
/*
    printf("Npoint  = %d\n ",GeomConv[Ncalculs-1].Npoint);
    printf("TypeCalcul  = %s\n ",GeomConv[Ncalculs-1].TypeCalcul);
   for(i=0;i<GeomConv[Ncalculs-1].Npoint;i++)
   {
   	for(j=0;j<GeomConv[Ncalculs-1].Ntype;j++)
          	printf("%s ",GeomConv[Ncalculs-1].Data[j][i]);
       	printf("\n ");
	
   }
*/
   k = 0;
   for(i=0;(gint)i<Ncalculs;i++)
   {
   	for(j=0;(gint)j<GeomConv[i].Npoint;j++)
	{
	   k++;
	   GeomConv[i].NumGeom[j] = k;
	}
        k++;
  }
  create_energies_curves(GeomConv,Ncalculs);
  g_free(t);
  g_free(temp);
}
/*********************************************************************/
DataGeomConv init_geom_qchem_conv(gchar* namefile)
{
  DataGeomConv GeomConv;
 gint i;
  GeomConv.Npoint = 0;
  GeomConv.Ntype  = 3;
  GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
  GeomConv.fileType = GABEDIT_TYPEFILE_QCHEM;

 GeomConv.TypeData[0] = g_strdup(_(" Energy "));
 GeomConv.TypeData[1] = g_strdup(_(" Gradient Max "));
 GeomConv.TypeData[2] = g_strdup(_(" Dep. Max "));

 GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
 for(i = 0;i<GeomConv.Ntype;i++)
 	GeomConv.Data[i] = NULL;
 GeomConv.TypeCalcul = NULL;
 GeomConv.NumGeom = NULL;
 GeomConv.GeomFile = g_strdup(namefile);
 return GeomConv;
}
/*********************************************************************/
DataGeomConv init_geom_orca_conv(gchar* namefile)
{
  DataGeomConv GeomConv;
 gint i;
  GeomConv.Npoint = 0;
  GeomConv.Ntype  = 3;
  GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
  GeomConv.fileType = GABEDIT_TYPEFILE_ORCA;

 GeomConv.TypeData[0] = g_strdup(_(" Energy "));
 GeomConv.TypeData[1] = g_strdup(_(" RMS Gradient"));
 GeomConv.TypeData[2] = g_strdup(_(" MAX Gradient "));

 GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
 for(i = 0;i<GeomConv.Ntype;i++)
 	GeomConv.Data[i] = NULL;
 GeomConv.TypeCalcul = NULL;
 GeomConv.NumGeom = NULL;
 GeomConv.GeomFile = g_strdup(namefile);
 return GeomConv;
}
/*********************************************************************/
void find_energy_orca_output(gchar* NomFichier)
{
	gchar *pdest;
	guint  i=0;
	guint  j=0;
	guint  k=0;
	gchar *temp =  g_malloc(50*sizeof(gchar));	
	guint taille=BSIZE;
	gchar *t;
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gboolean Ok = FALSE;

        
	t=g_malloc(taille*sizeof(gchar));
 	fd = FOpen(NomFichier, "rb"); 
        if(!fd)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),NomFichier);
		Message(t,_("Error"),TRUE);
		if(t) g_free(t);
		return;
	}
        
	 while(!feof(fd))
	{
    		 { gchar* e = fgets(t,taille,fd);}
                 if(strstr(t,"ORCA OPTIMIZATION COORDINATE SETUP") )
		 {
         		Ncalculs++;
                        if(Ncalculs == 1)
				GeomConv =  g_malloc(sizeof(DataGeomConv) );
			else
				GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_orca_conv(NomFichier);
		 }
    		 pdest = strstr( t,"GEOMETRY OPTIMIZATION CYCLE");

   		if( pdest != NULL  && Ncalculs>0)
		{
		  GeomConv[Ncalculs-1].Npoint++;
		  if(GeomConv[Ncalculs-1].Npoint == 1 )
		  {
			GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  }
		  else
		  {
			GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));
		  }
		  for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
                  {
		 	GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
                 	GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1][0] = '\0';
                  }

		  Ok = TRUE;
                 while(!feof(fd) && Ok )
		 {
		 	if(!fgets(t,taille,fd))
			{
				Ok = FALSE;
				break;
			}
			else
			{
				if(strstr(t,"FINAL SINGLE POINT ENERGY"))
				{
					Ok = TRUE;
					break;
				}

			}
		 }
		 if(!Ok) break;
                 sscanf(t,"%s %s %s %s %s",temp,temp,temp,temp,GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1]);
		 uppercase(t);
		 GeomConv[Ncalculs-1].TypeCalcul = g_strdup(" ");
                 do
                 {
		 	if(!fgets(t,taille,fd))break;
                 }while(strstr(t,"RMS gradient")==NULL && !feof(fd) ) ;
                 if(strstr(t,"RMS gradient")!=NULL)
		 {
                 	sscanf(t,"%s %s %s %s", temp,temp,temp,GeomConv[Ncalculs-1].Data[1][GeomConv[Ncalculs-1].Npoint-1]);
		 	if(!fgets(t,taille,fd))break;
                 	sscanf(t,"%s %s %s %s", temp,temp,temp,GeomConv[Ncalculs-1].Data[2][GeomConv[Ncalculs-1].Npoint-1]);
		 }
		}
	}

    fclose(fd);
   
    if(!Ok && GeomConv && GeomConv[Ncalculs-1].Npoint>0) GeomConv[Ncalculs-1].Npoint--;
    if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
    {
	GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
	Ncalculs--;
	if(Ncalculs>0)
		GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
	else
	{
		g_free(GeomConv);
		GeomConv =  NULL;
	}
    }
   k = 0;
   for(i=0;(gint)i<Ncalculs;i++)
   	for(j=0;(gint)j<GeomConv[i].Npoint;j++)
	{
	   k++;
	   GeomConv[i].NumGeom[j] = k;
	}
  create_energies_curves(GeomConv,Ncalculs);
  g_free(t);
  g_free(temp);
}
/*********************************************************************/
DataGeomConv init_geom_vasp_conv_outcar(gchar* namefile)
{
  DataGeomConv GeomConv;
 gint i;
  GeomConv.Npoint = 0;
  GeomConv.Ntype  = 4;
  GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
  GeomConv.fileType = GABEDIT_TYPEFILE_VASPOUTCAR;

 GeomConv.TypeData[0] = g_strdup(_(" Energy "));
 GeomConv.TypeData[1] = g_strdup(_(" dX "));
 GeomConv.TypeData[2] = g_strdup(_(" dY "));
 GeomConv.TypeData[3] = g_strdup(_(" dZ "));

 GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
 for(i = 0;i<GeomConv.Ntype;i++)
 	GeomConv.Data[i] = NULL;
 GeomConv.TypeCalcul = NULL;
 GeomConv.NumGeom = NULL;
 GeomConv.GeomFile = g_strdup(namefile);
 return GeomConv;
}
/*********************************************************************/
void find_energy_vasp_outcar(gchar* NomFichier)
{
	gchar *pdest;
	guint  i=0;
	guint  j=0;
	guint  k=0;
	gchar *temp =  g_malloc(50*sizeof(gchar));	
	gchar *temp1 =  g_malloc(50*sizeof(gchar));	
	gchar *temp2 =  g_malloc(50*sizeof(gchar));	
	guint taille=BSIZE;
	gchar* tag = "energy  without entropy=";
	gchar *t;
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gboolean Ok = FALSE;

        
	t=g_malloc(taille*sizeof(gchar));
 	fd = FOpen(NomFichier, "rb"); 
        if(!fd)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),NomFichier);
		Message(t,_("Error"),TRUE);
		if(t) g_free(t);
		return;
	}
	 while(!feof(fd))
	{
    		 { gchar* e = fgets(t,taille,fd);}
                 if(strstr(t,tag) )
		 {
         		Ncalculs++;
                        if(Ncalculs == 1) GeomConv =  g_malloc(sizeof(DataGeomConv) );
			else GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_vasp_conv_outcar(NomFichier);
			break;
		 }
	}
	rewind(fd);
        
	 while(!feof(fd))
	{
    		 { gchar* e = fgets(t,taille,fd);}
    		 pdest = strstr( t,tag);
   		if( pdest != NULL  && Ncalculs>0)
		{
		  GeomConv[Ncalculs-1].Npoint++;
		  if(GeomConv[Ncalculs-1].Npoint == 1 )
		  {
			GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++) GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  }
		  else
		  {
			GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++) GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));
		  }
		  for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
                  {
		 	GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
                 	GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1][0] = '\0';
                  }
		  pdest = strstr(t,tag)+strlen(tag);
		  sscanf(pdest,"%s",t);
		  GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1] = g_strdup_printf("%s",t);

		  Ok = TRUE;
                 while(!feof(fd) && Ok )
		 {
		 	if(!fgets(t,taille,fd))
			{
				Ok = FALSE;
				break;
			}
			else
			{
				if(strstr(t,"  total drift:"))
				{
					Ok = TRUE;
					break;
				}

			}
		 }
		 if(!Ok) break;
                 sscanf(t,"%s %s %s %s %s",temp,temp,temp,temp1,temp2);
		  GeomConv[Ncalculs-1].Data[1][GeomConv[Ncalculs-1].Npoint-1] = g_strdup(temp);
		  GeomConv[Ncalculs-1].Data[2][GeomConv[Ncalculs-1].Npoint-1] = g_strdup(temp1);
		  GeomConv[Ncalculs-1].Data[3][GeomConv[Ncalculs-1].Npoint-1] = g_strdup(temp2);
		}
	}

    fclose(fd);
   
    if(!Ok && GeomConv && GeomConv[Ncalculs-1].Npoint>0) GeomConv[Ncalculs-1].Npoint--;
    if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
    {
	GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
	Ncalculs--;
	if(Ncalculs>0)
		GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
	else
	{
		g_free(GeomConv);
		GeomConv =  NULL;
	}
    }
   k = 0;
   for(i=0;(gint)i<Ncalculs;i++)
   	for(j=0;(gint)j<GeomConv[i].Npoint;j++)
	{
	   k++;
	   GeomConv[i].NumGeom[j] = k;
	}
  create_energies_curves(GeomConv,Ncalculs);
  g_free(t);
  g_free(temp);
}
/*********************************************************************/
DataGeomConv init_geom_vasp_xml_conv(gchar* namefile)
{
  DataGeomConv GeomConv;
 gint i;
  GeomConv.Npoint = 0;
  GeomConv.Ntype  = 4;
  GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
  GeomConv.fileType = GABEDIT_TYPEFILE_VASPXML;

 GeomConv.TypeData[0] = g_strdup(_(" Energy "));
 GeomConv.TypeData[1] = g_strdup(_(" dX "));
 GeomConv.TypeData[2] = g_strdup(_(" dY "));
 GeomConv.TypeData[3] = g_strdup(_(" dZ "));

 GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
 for(i = 0;i<GeomConv.Ntype;i++) GeomConv.Data[i] = NULL;
 GeomConv.TypeCalcul = NULL;
 GeomConv.NumGeom = NULL;
 GeomConv.GeomFile = g_strdup(namefile);
 return GeomConv;
}
/*********************************************************************/
void find_energy_vasp_xml(gchar* NomFichier)
{
	gchar *pdest;
	guint  i=0;
	guint  j=0;
	guint  k=0;
	gchar* tag = "<structure>";
	gchar *t;
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gboolean Ok = FALSE;
	gint nf = 0;
	gdouble x,y,z;
	gdouble X,Y,Z;
	gdouble energy;
	gchar* pos;

        
	t=g_malloc(BSIZE*sizeof(gchar));
 	fd = FOpen(NomFichier, "rb"); 
        if(!fd)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),NomFichier);
		Message(t,_("Error"),TRUE);
		if(t) g_free(t);
		return;
	}
	 while(!feof(fd))
	{
    		 { gchar* e = fgets(t,BSIZE,fd);}
                 if(strstr(t,tag) )
		 {
         		Ncalculs++;
                        if(Ncalculs == 1) GeomConv =  g_malloc(sizeof(DataGeomConv) );
			else GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_vasp_xml_conv(NomFichier);
			break;
		 }
	}
	rewind(fd);
        
	 while(!feof(fd))
	{
    		{ gchar* e = fgets(t,BSIZE,fd);}
    		pdest = strstr( t,tag);
   		if( pdest != NULL  && Ncalculs>0)
		{
			Ok = TRUE;
		  	GeomConv[Ncalculs-1].Npoint++;
		  	if(GeomConv[Ncalculs-1].Npoint == 1 )
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++) GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  	}
		  	else
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++) GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));
		  	}
		  	for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
                  	{
		 		GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
                 		GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1][0] = '\0';
                  	}
                 	while(!feof(fd))
		 	{
		 		if(!fgets(t,BSIZE,fd)) { Ok = FALSE; break; }
                 		if(strstr(t,"varray name") && strstr(t,"forces")) break;
			}
			if(!Ok) break;
			nf = 0;
			X = Y = Z = 0;
                 	while(!feof(fd))
		 	{
		 		if(!fgets(t,BSIZE,fd)) { Ok = FALSE; break; }
                 		if(strstr(t,"varray")) break;
				pos = strstr(t,">");
				if(!pos) break;
				if(3!=sscanf(pos+1,"%lf %lf %lf",&x,&y,&z)) { Ok = FALSE; break;}
				X += x*x; Y += y*y; Z += z*z;
				nf++;
			}
			if(!Ok) break;
			if(nf<1) { Ok = FALSE; break;}
			X = sqrt(X/nf);
			Y = sqrt(Y/nf);
			Z = sqrt(Z/nf);

                 	while(!feof(fd))
		 	{
		 		if(!fgets(t,BSIZE,fd)) { Ok = FALSE; break; }
                 		if(strstr(t,"/energy")) { Ok = FALSE; break;}
                 		if(strstr(t,"e_fr_energy")) break;
			}
			if(!Ok) break;
                	if(!strstr(t,"e_fr_energy")) { Ok = FALSE; break;}
			pos = strstr(t,">");
			if(!pos) { Ok = FALSE; break;}
			if(1!=sscanf(pos+1,"%lf",&energy)) { Ok = FALSE; break;}
			/* printf("energy = %f X = %f Y = %f Z = %f\n",energy,X,Y,Z);*/
			GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1] = g_strdup_printf("%0.8f",energy);
			GeomConv[Ncalculs-1].Data[1][GeomConv[Ncalculs-1].Npoint-1] = g_strdup_printf("%0.6f",X);
			GeomConv[Ncalculs-1].Data[2][GeomConv[Ncalculs-1].Npoint-1] = g_strdup_printf("%0.6f",Y);
			GeomConv[Ncalculs-1].Data[3][GeomConv[Ncalculs-1].Npoint-1] = g_strdup_printf("%0.6f",Z);
		}
	}
	fclose(fd);
   
	if(!Ok && GeomConv && GeomConv[Ncalculs-1].Npoint>0) GeomConv[Ncalculs-1].Npoint--;
	if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
	{
		GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
		Ncalculs--;
		if(Ncalculs>0) GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
		else
		{
			g_free(GeomConv);
			GeomConv =  NULL;
		}
	}
	k = 0;
	for(i=0;(gint)i<Ncalculs;i++)
   	for(j=0;(gint)j<GeomConv[i].Npoint;j++)
	{
		k++;
		GeomConv[i].NumGeom[j] = k;
	}
	create_energies_curves(GeomConv, Ncalculs);
	g_free(t);
}
/*************************************************************************************/
static DataGeomConv init_geom_molden_gabedit_conv(gchar *fileName, GabEditTypeFile type)
{
	DataGeomConv GeomConv;
	gint i;
	GeomConv.Npoint = 0;
	GeomConv.Ntype  = 5;
	GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
  	GeomConv.fileType = type;
	GeomConv.TypeData[0] = g_strdup(_(" Energy "));
	GeomConv.TypeData[1] = g_strdup(_(" Force Max "));
	GeomConv.TypeData[2] = g_strdup(_(" Force RMS "));
	GeomConv.TypeData[3] = g_strdup(_(" Dep. Max "));
	GeomConv.TypeData[4] = g_strdup(_(" Dep. RMS "));
	GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
	for(i = 0;i<GeomConv.Ntype;i++)
		GeomConv.Data[i] = NULL;
	GeomConv.TypeCalcul = NULL;
	GeomConv.NumGeom = NULL;
	GeomConv.GeomFile = g_strdup(fileName);

	return GeomConv;
}
/*********************************************************************/
void find_energy_molpro_log(gchar* NomFichier)
{
	gchar *pdest;
	guint  i=0;
	guint  j=0;
	guint  k=0;
	gchar *temp =  g_malloc(50*sizeof(gchar));	
	guint taille=BSIZE;
	gchar *t;
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gboolean Ok;

        
	t=g_malloc(taille);
 	fd = FOpen(NomFichier, "rb"); 
        if(!fd)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),NomFichier);
		Message(t,_("Error"),TRUE);
		if(t)
			g_free(t);
		return;
	}
        
	 while(!feof(fd))
	{
    		 { gchar* e = fgets(t,taille,fd);}
                 if( strlen(t)>2 && strstr(t,"GEOMETRY OPTIMIZATION STEP  1") )
		 {
         		Ncalculs++;
                        if(Ncalculs == 1)
				GeomConv =  g_malloc(sizeof(DataGeomConv) );
			else
				GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_molpro_conv(NomFichier);
		 }
                 if( strlen(t)>2 && strstr(t,"Optimization point  1") && Ncalculs<1)
		 {
         		Ncalculs++;
                        if(Ncalculs == 1)
				GeomConv =  g_malloc(sizeof(DataGeomConv) );
			else
				GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_molpro_conv(NomFichier);
		 }

    		 pdest = strstr( t,"Optimization point");

   		if( pdest != NULL  && Ncalculs>0)
		{
		  GeomConv[Ncalculs-1].Npoint++;
		  if(GeomConv[Ncalculs-1].Npoint == 1 )
		  {
			GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  }
		  else
		  {
			GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  }
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
                 {
		 	GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
                 	GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1][0] = '\0';
                 }

		 Ok = TRUE;
                 while(!feof(fd) && Ok )
		 {
		 	if(!fgets(t,taille,fd))
			{
				Ok = FALSE;
				break;
			}
			else
			{
				if(strstr(t,"(") && strstr(t,")"))
				{
					Ok = TRUE;
					break;
				}

			}
		 }
		 if(!Ok) break;
                 sscanf(t,"%s %s %s %s %s",temp,temp,temp,temp,GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1]);
		 uppercase(t);
		 GeomConv[Ncalculs-1].TypeCalcul = g_malloc(100*sizeof(gchar));
    		 pdest = strstr( t,"(");
                 pdest++;
                 sscanf(pdest,"%s)",GeomConv[Ncalculs-1].TypeCalcul);
                 for(i=0;i<strlen(GeomConv[Ncalculs-1].TypeCalcul);i++)
		 {
			if(GeomConv[Ncalculs-1].TypeCalcul[i] == ')')
				GeomConv[Ncalculs-1].TypeCalcul[i] = ' ';
		 }

                 do
                 {
    		 { gchar* e = fgets(t,taille,fd);}
                 }while(strstr(t,"Convergence:")==NULL && !feof(fd) ) ;
                 if(strstr(t,"Convergence:")!=NULL)
                 	sscanf(t,"%s %s %s %s %s ", temp,temp,temp,temp,GeomConv[Ncalculs-1].Data[1][GeomConv[Ncalculs-1].Npoint-1]);
		}
			
	}

    fclose(fd);
   
    if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
    {
	GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
	Ncalculs--;
	if(Ncalculs>0)
		GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
	else
	{
		g_free(GeomConv);
		GeomConv =  NULL;
	}
    }
   k = 0;
   for(i=0;(gint)i<Ncalculs;i++)
   	for(j=0;(gint)j<GeomConv[i].Npoint;j++)
	{
	   k++;
	   GeomConv[i].NumGeom[j] = k;
	}
  create_energies_curves(GeomConv,Ncalculs);
  g_free(t);
  g_free(temp);
}
/*********************************************************************/
void find_energy_qchem_log(gchar* NomFichier)
{
	gchar *pdest;
	guint  i=0;
	guint  j=0;
	guint  k=0;
	gchar *temp =  g_malloc(50*sizeof(gchar));	
	guint taille=BSIZE;
	gchar *t;
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gboolean Ok;

        
	t=g_malloc(taille);
 	fd = FOpen(NomFichier, "rb"); 
        if(!fd)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),NomFichier);
		Message(t,_("Error"),TRUE);
		if(t)
			g_free(t);
		return;
	}
        
	 while(!feof(fd))
	{
    		 { gchar* e = fgets(t,taille,fd);}
                 if( strlen(t)>2 && strstr(t,"Optimization Cycle:   1") )
		 {
         		Ncalculs++;
                        if(Ncalculs == 1)
				GeomConv =  g_malloc(sizeof(DataGeomConv) );
			else
				GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_qchem_conv(NomFichier);
		 }
    		 pdest = strstr( t,"Optimization Cycle:");

   		if( pdest != NULL  && Ncalculs>0)
		{
		  GeomConv[Ncalculs-1].Npoint++;
		  if(GeomConv[Ncalculs-1].Npoint == 1 )
		  {
			GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  }
		  else
		  {
			GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  }
		  for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
                  {
		 	GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
                 	GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1][0] = '\0';
                  }

		  Ok = TRUE;
                 while(!feof(fd) && Ok )
		 {
		 	if(!fgets(t,taille,fd))
			{
				Ok = FALSE;
				break;
			}
			else
			{
				if(strstr(t,"Energy is"))
				{
					Ok = TRUE;
					break;
				}

			}
		 }
		 if(!Ok) break;
                 sscanf(t,"%s %s %s",temp,temp,GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1]);
		 uppercase(t);
		 GeomConv[Ncalculs-1].TypeCalcul = g_strdup(" ");
                 do
                 {
		 	if(!fgets(t,taille,fd))break;
                 }while(strstr(t,"Cnvgd?")==NULL && !feof(fd) ) ;
                 if(strstr(t,"Cnvgd?")!=NULL)
		 {
		 	if(!fgets(t,taille,fd))break;
                 	sscanf(t,"%s %s", temp,GeomConv[Ncalculs-1].Data[1][GeomConv[Ncalculs-1].Npoint-1]);
		 	if(!fgets(t,taille,fd))break;
                 	sscanf(t,"%s %s", temp,GeomConv[Ncalculs-1].Data[2][GeomConv[Ncalculs-1].Npoint-1]);
		 }
		}
			
	}

    fclose(fd);
   
    if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
    {
	GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
	Ncalculs--;
	if(Ncalculs>0)
		GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
	else
	{
		g_free(GeomConv);
		GeomConv =  NULL;
	}
    }
   k = 0;
   for(i=0;(gint)i<Ncalculs;i++)
   	for(j=0;(gint)j<GeomConv[i].Npoint;j++)
	{
	   k++;
	   GeomConv[i].NumGeom[j] = k;
	}
  create_energies_curves(GeomConv,Ncalculs);
  g_free(t);
  g_free(temp);
}
/*********************************************************************/
DataGeomConv init_geom_nwchem_conv(gchar* namefile)
{
  DataGeomConv GeomConv;
 gint i;
  GeomConv.Npoint = 0;
  GeomConv.Ntype  = 3;
  GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
  GeomConv.fileType = GABEDIT_TYPEFILE_NWCHEM;

 GeomConv.TypeData[0] = g_strdup(_(" Energy "));
 GeomConv.TypeData[1] = g_strdup(_(" Gradient Max "));
 GeomConv.TypeData[2] = g_strdup(_(" Dep. Max "));

 GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
 for(i = 0;i<GeomConv.Ntype;i++)
 	GeomConv.Data[i] = NULL;
 GeomConv.TypeCalcul = NULL;
 GeomConv.NumGeom = NULL;
 GeomConv.GeomFile = g_strdup(namefile);
 return GeomConv;
}
/*********************************************************************/
void find_energy_nwchem_log(gchar* NomFichier)
{
	gchar *pdest;
	guint  i=0;
	guint  j=0;
	guint  k=0;
	gchar *temp =  g_malloc(50*sizeof(gchar));	
	guint taille=BSIZE;
	gchar *t;
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gboolean Ok = FALSE;

        
	t=g_malloc(taille);
 	fd = FOpen(NomFichier, "rb"); 
        if(!fd)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),NomFichier);
		Message(t,_("Error"),TRUE);
		if(t)
			g_free(t);
		return;
	}
        
	 while(!feof(fd))
	{
    		 { gchar* e = fgets(t,taille,fd);}
                 if(strstr(t,"Step   0") )
		 {
         		Ncalculs++;
                        if(Ncalculs == 1)
				GeomConv =  g_malloc(sizeof(DataGeomConv) );
			else
				GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_nwchem_conv(NomFichier);
		 }
		pdest = NULL;
		if(strstr( t,"Step ") && !strstr(t,"Energy")) pdest = strstr( t,"Step ");

   		if( pdest != NULL  && Ncalculs>0)
		{
		  GeomConv[Ncalculs-1].Npoint++;
		  if(GeomConv[Ncalculs-1].Npoint == 1 )
		  {
			GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  }
		  else
		  {
			GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  }
		  for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
                  {
		 	GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
                 	GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1][0] = '\0';
                  }

		  Ok = TRUE;
                 while(!feof(fd) && Ok )
		 {
		 	if(!fgets(t,taille,fd))
			{
				Ok = FALSE;
				break;
			}
			else
			{
				if(strstr(t,"Step       Energy      Delta E   Gmax     Grms     Xrms     Xmax"))
				{
					Ok = TRUE;
					break;
				}

			}
		 }
		 if(!Ok) break;
		 if(!fgets(t,taille,fd)) break;
		 if(!fgets(t,taille,fd)) break;
                 sscanf(t,"%s %s %s %s %s %s %s %s",temp, temp,GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1],temp,GeomConv[Ncalculs-1].Data[1][GeomConv[Ncalculs-1].Npoint-1],temp,temp,GeomConv[Ncalculs-1].Data[2][GeomConv[Ncalculs-1].Npoint-1]);
		 GeomConv[Ncalculs-1].TypeCalcul = g_strdup(" ");
		}
			
	}
    fclose(fd);
   
    if(!Ok && Ncalculs>0 && GeomConv[Ncalculs-1].Npoint>0) GeomConv[Ncalculs-1].Npoint--;
    if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
    {
	GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
	Ncalculs--;
	if(Ncalculs>0)
		GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
	else
	{
		g_free(GeomConv);
		GeomConv =  NULL;
	}
    }
   k = 0;
   for(i=0;(gint)i<Ncalculs;i++)
   	for(j=0;(gint)j<GeomConv[i].Npoint;j++)
	{
	   k++;
	   GeomConv[i].NumGeom[j] = k;
	}
  create_energies_curves(GeomConv,Ncalculs);
  g_free(t);
  g_free(temp);
}
/*********************************************************************/
DataGeomConv init_geom_psicode_conv(gchar* namefile)
{
  DataGeomConv GeomConv;
 gint i;
  GeomConv.Npoint = 0;
  GeomConv.Ntype  = 5;
  GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
  GeomConv.fileType = GABEDIT_TYPEFILE_PSICODE;

 GeomConv.TypeData[0] = g_strdup(_(" Energy "));
 GeomConv.TypeData[1] = g_strdup(_(" MAX Force "));
 GeomConv.TypeData[2] = g_strdup(_(" RMS Force "));
 GeomConv.TypeData[3] = g_strdup(_(" MAX Dsip. "));
 GeomConv.TypeData[4] = g_strdup(_(" RMS Disp. "));

 GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
 for(i = 0;i<GeomConv.Ntype;i++)
 	GeomConv.Data[i] = NULL;
 GeomConv.TypeCalcul = NULL;
 GeomConv.NumGeom = NULL;
 GeomConv.GeomFile = g_strdup(namefile);
 return GeomConv;
}
/*********************************************************************/
void find_energy_psicode_log(gchar* NomFichier)
{
	gchar *pdest;
	guint  i=0;
	guint  c=0;
	guint  j=0;
	guint  k=0;
	gchar *temp =  g_malloc(50*sizeof(gchar));	
	guint taille=BSIZE;
	gchar *t;
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gboolean Ok = FALSE;

        
	t=g_malloc(taille);
 	fd = FOpen(NomFichier, "rb"); 
        if(!fd)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),NomFichier);
		Message(t,_("Error"),TRUE);
		if(t)
			g_free(t);
		return;
	}
        
	 while(!feof(fd))
	{
    		 { gchar* e = fgets(t,taille,fd);}
                 if(strstr(t,"OPTKING")  && Ncalculs == 0)
		 {
         		Ncalculs++;
                        if(Ncalculs == 1)
				GeomConv =  g_malloc(sizeof(DataGeomConv) );
			else
				GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_psicode_conv(NomFichier);
		 }
		pdest = NULL;
		if(strstr( t,"Step ") 
			&& strstr(t,"Energy")
			&& strstr(t,"MAX Force")
			&& strstr(t,"RMS Force")
			&& strstr(t,"MAX Disp")
			&& strstr(t,"RMS Disp")
			) 
		 {
		
			pdest = strstr( t,"Step ");
		 }

   		if( pdest != NULL  && Ncalculs>0)
		{
		  GeomConv[Ncalculs-1].Npoint++;
		  if(GeomConv[Ncalculs-1].Npoint == 1 )
		  {
			GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  }
		  else
		  {
			GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  }
		  for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
                  {
		 	GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
                 	GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1][0] = '\0';
                  }

		  Ok = TRUE;
                 while(!feof(fd) && Ok )
		 {
		 	if(!fgets(t,taille,fd))
			{
				Ok = FALSE;
				break;
			}
			else
			{
				if(strstr(t,"Convergence Criteria"))
				{
					Ok = TRUE;
					break;
				}

			}
		 }
		 if(!Ok) break;
		 if(!fgets(t,taille,fd)) break;/* ---- */
		 if(!fgets(t,taille,fd)) break;
		for(c=0;c<strlen(t);c++) if(t[c]=='*'||t[c]=='o')t[c]=' ';
                 sscanf(t,"%s %s %s %s %s %s %s",
		temp,
		GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1],
		temp,
		GeomConv[Ncalculs-1].Data[1][GeomConv[Ncalculs-1].Npoint-1],
		GeomConv[Ncalculs-1].Data[2][GeomConv[Ncalculs-1].Npoint-1],
		GeomConv[Ncalculs-1].Data[3][GeomConv[Ncalculs-1].Npoint-1],
		GeomConv[Ncalculs-1].Data[4][GeomConv[Ncalculs-1].Npoint-1]
);
		 GeomConv[Ncalculs-1].TypeCalcul = g_strdup(" ");
		}
			
	}
    fclose(fd);
   
    if(!Ok && Ncalculs>0 && GeomConv[Ncalculs-1].Npoint>0) GeomConv[Ncalculs-1].Npoint--;
    if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
    {
	GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
	Ncalculs--;
	if(Ncalculs>0)
		GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
	else
	{
		g_free(GeomConv);
		GeomConv =  NULL;
	}
    }
   k = 0;
   for(i=0;(gint)i<Ncalculs;i++)
   	for(j=0;(gint)j<GeomConv[i].Npoint;j++)
	{
	   k++;
	   GeomConv[i].NumGeom[j] = k;
	}
  create_energies_curves(GeomConv,Ncalculs);
  g_free(t);
  g_free(temp);
}
/*********************************************************************/
DataGeomConv init_geom_gauss_scanOpt_conv(gchar *namefile)
{
	DataGeomConv GeomConv;
	gint i;
	GeomConv.Npoint = 0;
	GeomConv.Ntype  = 5;
	GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
	GeomConv.fileType = GABEDIT_TYPEFILE_GAUSSIAN;
	GeomConv.TypeData[0] = g_strdup(_(" Energy "));
	GeomConv.TypeData[1] = g_strdup(_(" Force Max "));
	GeomConv.TypeData[2] = g_strdup(_(" Force RMS "));
	GeomConv.TypeData[3] = g_strdup(_(" Dep. Max "));
	GeomConv.TypeData[4] = g_strdup(_(" Dep. RMS "));
	GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
	for(i = 0;i<GeomConv.Ntype;i++) GeomConv.Data[i] = NULL;
	GeomConv.TypeCalcul = NULL;
	GeomConv.NumGeom = NULL;
	GeomConv.GeomFile = g_strdup(namefile);
	return GeomConv;
}
/*********************************************************************/
void find_energy_gauss_scanOpt_output(gchar* NomFichier)
{
	gchar* pdest = NULL;
	guint  i=0;
	guint  j=0;
	guint  k=0;
	gchar *temp =  g_malloc(100*sizeof(gchar));	
	gchar *t;
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gint numGeom = 0;
	gchar** props = NULL;
	gint nTypes = 0;

        
        Ncalculs++;
	GeomConv =  g_malloc(sizeof(DataGeomConv) );
  	GeomConv[Ncalculs-1] = init_geom_gauss_scanOpt_conv(NomFichier);
	props = g_malloc(GeomConv[Ncalculs-1].Ntype*sizeof(gchar*));
	nTypes = GeomConv[Ncalculs-1].Ntype;
	for(i=0;(gint)i<nTypes;i++) props[i] =  g_malloc(100*sizeof(gchar));

	t=g_malloc(BSIZE*sizeof(gchar));
 	fd = FOpen(NomFichier, "rb"); 
        if(!fd)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),NomFichier);
		Message(t,_("Error"),TRUE);
		if(t) g_free(t);
		return;
	}
        
	 while(!feof(fd))
	{
    		 { gchar* e = fgets(t,BSIZE,fd);}
                 if( strlen(t)>2 && strstr(t,"Normal termination of Gaussian") )
		 {
         		Ncalculs++;
			GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_gauss_scanOpt_conv(NomFichier);
			continue;
		 }
		 uppercase(t);
                 if( strlen(t)>2 && t[1] == '#' && !GeomConv[Ncalculs-1].TypeCalcul)
		 {
			GeomConv[Ncalculs-1].TypeCalcul = g_malloc(100*sizeof(gchar));
                 	sscanf(t,"%s %s", temp, GeomConv[Ncalculs-1].TypeCalcul);
		 }
    		 pdest = strstr( t,"SCF DONE");
   		if( pdest != NULL ) pdest = strstr( t,"=");
          	if(!pdest)
		{
    		 	pdest = strstr( t,"ENERGY=");
          	 	if(pdest && t[1] == 'E' ) pdest = strstr( t,"=");
                 	else pdest = NULL;
			
		}
   		if( pdest != NULL )
		{
			numGeom++;
                 	sscanf(pdest+1,"%s", props[0]);
			/*
			fprintf(stderr,"%s\n",t);
			fprintf(stderr,"%s\n",props[0]);
			*/
		}
                pdest = NULL;
    		pdest = strstr( t,"CONVERGED?");
                if( pdest != NULL)
		{
			for(i=1;(gint)i<nTypes;i++)
			{
    		 		{ gchar* e = fgets(t,BSIZE,fd);}
                 		sscanf(t,"%s %s %s", temp,temp,props[i]);
			}
		}
                pdest = NULL;
    		pdest = strstr( t," STATIONARY POINT FOUND");
   		if( pdest != NULL )
		{
			pdest++;
		  	GeomConv[Ncalculs-1].Npoint++;
		  	if(GeomConv[Ncalculs-1].Npoint == 1 )
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
					GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  	}
		  	else
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
					GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));
		  	}
		  	for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
                  	{
		 		GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(100*sizeof(gchar)); 
                 		GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1][0] = '\0';
                 	}
			GeomConv[Ncalculs-1].NumGeom[GeomConv[Ncalculs-1].Npoint-1] = numGeom;
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
                 		sprintf( GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1], "%s", props[i]);
		}
	}
	fclose(fd);
	if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
	{
		GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
		Ncalculs--;
		if(Ncalculs>0) GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
		else
		{
			g_free(GeomConv);
			GeomConv =  NULL;
		}
	}
	/*
	{
		gint j;
		for( j=0;j<Ncalculs;j++)
		{
			gint np=GeomConv[j].Npoint;
			fprintf(stderr,"DEBUG Npoint = %d\n",np);
			gint i;
			for(i=0;i<np;i++)
				fprintf(stderr,"numGeom = %d\n",GeomConv[j].NumGeom[i]);
		}
	}
	*/
	create_energies_curves(GeomConv,Ncalculs);
	g_free(t);
	g_free(temp);
	for(i=0;(gint)i<nTypes;i++) if(props[i]) g_free(props[i]);
}
/*********************************************************************/
DataGeomConv init_geom_gauss_ircOpt_conv(gchar *namefile)
{
	/* fprintf(stderr,"DEBUG init_geom_gauss_ircOpt_conv\n");*/
	DataGeomConv GeomConv;
	gint i;
	GeomConv.Npoint = 0;
	GeomConv.Ntype  = 3;
	GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
	GeomConv.fileType = GABEDIT_TYPEFILE_GAUSSIAN;
	GeomConv.TypeData[0] = g_strdup(_(" Energy "));
	GeomConv.TypeData[1] = g_strdup(_(" Force Max "));
	GeomConv.TypeData[2] = g_strdup(_(" Force RMS "));
	GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
	for(i = 0;i<GeomConv.Ntype;i++) GeomConv.Data[i] = NULL;
	GeomConv.TypeCalcul = NULL;
	GeomConv.NumGeom = NULL;
	GeomConv.GeomFile = g_strdup(namefile);
	return GeomConv;
}
/*********************************************************************/
void find_energy_gauss_ircOpt_output(gchar* NomFichier)
{
	gchar* pdest = NULL;
	guint  i=0;
	guint  j=0;
	guint  k=0;
	gchar *temp =  g_malloc(100*sizeof(gchar));	
	gchar *t;
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gint numGeom = 0;
	gchar** props = NULL;
	gint nTypes = 0;
	gint oneDir = 0;

        
        Ncalculs++;
	GeomConv =  g_malloc(sizeof(DataGeomConv) );
  	GeomConv[Ncalculs-1] = init_geom_gauss_ircOpt_conv(NomFichier);
	props = g_malloc(GeomConv[Ncalculs-1].Ntype*sizeof(gchar*));
	nTypes = GeomConv[Ncalculs-1].Ntype;
	for(i=0;(gint)i<nTypes;i++) props[i] =  g_malloc(100*sizeof(gchar));

	t=g_malloc(BSIZE*sizeof(gchar));
 	fd = FOpen(NomFichier, "rb"); 
        if(!fd)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),NomFichier);
		Message(t,_("Error"),TRUE);
		if(t)
			g_free(t);
		return;
	}
        
	 while(!feof(fd))
	{
    		 { gchar* e = fgets(t,BSIZE,fd);}
                 if( strlen(t)>2 && strstr(t,"Normal termination of Gaussian") )
		 {
         		Ncalculs++;
			GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_gauss_conv(NomFichier);
		 }
		if(strstr(t,"path following direction ")) oneDir = 1;
		 uppercase(t);
                 if( strlen(t)>2 && t[1] == '#' && !GeomConv[Ncalculs-1].TypeCalcul)
		 {
			GeomConv[Ncalculs-1].TypeCalcul = g_malloc(100*sizeof(gchar));
                 	sscanf(t,"%s %s", temp, GeomConv[Ncalculs-1].TypeCalcul);
		 }
    		 pdest = strstr( t,"SCF DONE");
   		if( pdest != NULL ) pdest = strstr( t,"=");
          	if(!pdest)
		{
    		 	pdest = strstr( t,"ENERGY=");
          	 	if(pdest && t[1] == 'E' ) pdest = strstr( t,"=");
                 	else pdest = NULL;
			
		}
   		if( pdest != NULL )
		{
			numGeom++;
                 	sscanf(pdest+1,"%s", props[0]);
			/*
			fprintf(stderr,"%s\n",t);
			fprintf(stderr,"%s\n",props[0]);
			*/
		}
                pdest = NULL;
    		if(strstr( t,"MAX ") && strstr( t,"RMS "))
		{
    			pdest = strstr( t,"MAX ") + strlen("MAX ");
                 	sscanf(pdest,"%s", props[1]);
    			pdest = strstr( t,"RMS ") + strlen("RMS ");
                 	sscanf(pdest,"%s", props[2]);
		}
                pdest = NULL;
    		pdest = strstr( t," CONVERGENCE MET");
   		if( pdest != NULL )
		{
			pdest++;
		  	GeomConv[Ncalculs-1].Npoint++;
		  	if(GeomConv[Ncalculs-1].Npoint == 1 )
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
					GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
		  	}
		  	else
		  	{
				GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
					GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));
		  	}
		  	for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
                  	{
		 		GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(100*sizeof(gchar)); 
                 		GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1][0] = '\0';
                 	}
			GeomConv[Ncalculs-1].NumGeom[GeomConv[Ncalculs-1].Npoint-1] = numGeom;
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
                 		sprintf( GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1], "%s", props[i]);
		}
	}
	fclose(fd);
	if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
	{
		GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
		Ncalculs--;
		if(Ncalculs>0) GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
		else
		{
			g_free(GeomConv);
			GeomConv =  NULL;
		}
	}
	if(oneDir==0)
	{
		/* sort */
		gint j;
		/* fprintf(stderr,"DEBUG sorting IRC\n");*/
		for( j=0;j<Ncalculs;j++)
		{
			gint n=GeomConv[j].Npoint/2;
			/*
			fprintf(stderr,"DEBUG n = %d\n",n);
			fprintf(stderr,"DEBUG Npoint = %d\n",GeomConv[j].Npoint);
			*/
			gint i;
			gint k;
			for(i=0;(gint)i<GeomConv[j].Ntype;i++)
			for(k=0;k<n;k++)
			{
				gchar* data = GeomConv[j].Data[i][k];
				GeomConv[j].Data[i][k] = GeomConv[j].Data[i][n+k];
				GeomConv[j].Data[i][n+k] = data;
				gint numGeom = GeomConv[j].NumGeom[k];
				GeomConv[j].NumGeom[k] = GeomConv[j].NumGeom[n+k];
				GeomConv[j].NumGeom[n+k] = numGeom;
			}
			for(i=0;(gint)i<GeomConv[j].Ntype;i++)
			for(k=0;k<n/2;k++)
			{
				gchar* data = GeomConv[j].Data[i][k];
				GeomConv[j].Data[i][k] = GeomConv[j].Data[i][n-k-1];
				GeomConv[j].Data[i][n-k-1] = data;
				gint numGeom = GeomConv[j].NumGeom[k];
				GeomConv[j].NumGeom[k] = GeomConv[j].NumGeom[n-k-1];
				GeomConv[j].NumGeom[n-k-1] = numGeom;
			}
		}
	}
	create_energies_curves(GeomConv,Ncalculs);
	g_free(t);
	g_free(temp);
	for(i=0;(gint)i<nTypes;i++) if(props[i]) g_free(props[i]);
}
/*********************************************************************/
void find_energy_molden_gabedit(gchar* FileName, GabEditTypeFile type)
{
	guint  i=0;
	guint  j=0;
	guint  k=0;
	guint taille=BSIZE;
	gchar temp[BSIZE];
	gchar t[BSIZE];
	FILE *fd;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gint nn = 0;
        
 	fd = FOpen(FileName, "rb"); 
        if(!fd)
	{
		sprintf(t,_(" Error : I can not open file %s\n"),FileName);
		Message(t,_("Error"),TRUE);
		return;
	}
        
	{ gchar* e = fgets(t,taille,fd);}
	 while(!feof(fd))
	{
		 uppercase(t);
		/*fprintf(stderr,"Ncalculs=%d\n",Ncalculs);*/
		/*fprintf(stderr,"%s\n",t);*/
                 if(strstr(t,"[GEOCONV]") )
		 {
		 	/* printf("OK GEOCONV %s\n",t);*/
         		Ncalculs++;
			if(Ncalculs==1) GeomConv =  g_malloc(sizeof(DataGeomConv) );
			else GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_molden_gabedit_conv(FileName, type);
			GeomConv[Ncalculs-1].TypeCalcul = g_malloc(100*sizeof(gchar));
			GeomConv[Ncalculs-1].fileType = type;
                 	nn = sscanf(t,"%s %s", temp, GeomConv[Ncalculs-1].TypeCalcul);
			if(nn<2) sprintf(GeomConv[Ncalculs-1].TypeCalcul," ");
		 	if(!fgets(t,taille,fd)) break;
		 	uppercase(t);
		 }

		 /* printf("%s\n",t);*/

   		if(Ncalculs>0 &&  strstr( t,"ENERGY"))
		{
	 		while(!feof(fd))
			{
		 		if(!fgets(t,taille,fd)) break;

				str_delete_n(t);
				delete_last_spaces(t);
				delete_first_spaces(t);
		 		if(!isFloat(t))
				{
					/* printf("%s is not a real \n",t);*/
					break;
				}
				GeomConv[Ncalculs-1].Npoint++;
				if(GeomConv[Ncalculs-1].Npoint == 1 )
				{
					GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
					for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
						GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
				}
				else
				{
					GeomConv[Ncalculs-1].NumGeom =  g_realloc(GeomConv[Ncalculs-1].NumGeom,GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
					for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
						GeomConv[Ncalculs-1].Data[i] =  g_realloc(GeomConv[Ncalculs-1].Data[i],GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
				}
				for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
				{
					GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
					GeomConv[Ncalculs-1].Data[i][GeomConv[Ncalculs-1].Npoint-1][0] = '\0';
				}
				GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1] = g_malloc(50*sizeof(gchar)); 
				sscanf(t,"%s", GeomConv[Ncalculs-1].Data[0][GeomConv[Ncalculs-1].Npoint-1]);
			}
			if(GeomConv[Ncalculs-1].Npoint<1) continue;
		}
		else if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint>0 && strstr( t,"MAX-FORCE"))
		{
			i = 0;
	 		while(!feof(fd))
			{
				if(!fgets(t,taille,fd)) break;
				if(i==GeomConv[Ncalculs-1].Npoint) break;
				str_delete_n(t);
				delete_last_spaces(t);
				delete_first_spaces(t);
				if(!isFloat(t)) break;
				sscanf(t,"%s", GeomConv[Ncalculs-1].Data[1][i]);
				i++;
			}
			GeomConv[Ncalculs-1].Npoint = i;
		}
		else if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint>0 &&  strstr( t,"RMS-FORCE"))
		{
			i = 0;
	 		while(!feof(fd))
			{
		 		if(!fgets(t,taille,fd)) break;
				if(i==GeomConv[Ncalculs-1].Npoint) break;
				str_delete_n(t);
				delete_last_spaces(t);
				delete_first_spaces(t);
		 		if(!isFloat(t)) break;
				sscanf(t,"%s", GeomConv[Ncalculs-1].Data[2][i]);
				i++;
			}
			GeomConv[Ncalculs-1].Npoint = i;
		}
		else if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint>0 && strstr( t,"MAX-STEP") )
		{
			i = 0;
	 		while(!feof(fd))
			{
				if(!fgets(t,taille,fd)) break;
				if(i==GeomConv[Ncalculs-1].Npoint) break;
				str_delete_n(t);
				delete_last_spaces(t);
				delete_first_spaces(t);
				if(!isFloat(t)) break;
				sscanf(t,"%s", GeomConv[Ncalculs-1].Data[3][i]);
				i++;
			}
			GeomConv[Ncalculs-1].Npoint = i;
		}
		else if(  Ncalculs>0 && GeomConv[Ncalculs-1].Npoint>0 && strstr( t,"RMS-STEP"))
		{
			i = 0;
	 		while(!feof(fd))
			{
				if(!fgets(t,taille,fd)) break;
				if(i==GeomConv[Ncalculs-1].Npoint) break;
				str_delete_n(t);
				delete_last_spaces(t);
				delete_first_spaces(t);
				if(!isFloat(t)) break;
				sscanf(t,"%s", GeomConv[Ncalculs-1].Data[4][i]);
				i++;
			}
			GeomConv[Ncalculs-1].Npoint = i;
		}
		else if(!fgets(t,taille,fd)) break;
	}

	fclose(fd);
   
	if( Ncalculs>0 && GeomConv[Ncalculs-1].Npoint == 0)
	{
		GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
		Ncalculs--;
		if(Ncalculs>0)
			GeomConv =  g_realloc(GeomConv, Ncalculs*sizeof(DataGeomConv) );
		else
		{
			g_free(GeomConv);
			GeomConv =  NULL;
		}
	}
	/*
	printf("Ncalculs  = %d\n ",Ncalculs);
	if(Ncalculs>0)
	{
		printf("Npoint  = %d\n ",GeomConv[Ncalculs-1].Npoint);
		printf("TypeCalcul  = %s\n ",GeomConv[Ncalculs-1].TypeCalcul);
		for(i=0;i<GeomConv[Ncalculs-1].Npoint;i++)
		{
			for(j=0;j<GeomConv[Ncalculs-1].Ntype;j++)
          			printf("%s ",GeomConv[Ncalculs-1].Data[j][i]);
       			printf("\n ");
		}
	}
	*/

	k = 0;
	for(i=0;(gint)i<Ncalculs;i++)
	{
		for(j=0;(gint)j<GeomConv[i].Npoint;j++)
		{
			k++;
			GeomConv[i].NumGeom[j] = k;
		}
	}
	create_energies_curves(GeomConv,Ncalculs);
}
/*************************************************************************************/
static DataGeomConv init_geom_gabedit_geoms(gchar *fileName, gint nLabels, gchar** labels)
{
	DataGeomConv GeomConv;
	gint i;
	GeomConv.Npoint = 0;
	GeomConv.Ntype  = nLabels;
	if(nLabels>0) GeomConv.TypeData = g_malloc(GeomConv.Ntype*sizeof(gchar*) );
	else GeomConv.TypeData = NULL;
  	GeomConv.fileType = GABEDIT_TYPEFILE_GABEDIT;
	for(i=0;i<nLabels;i++) GeomConv.TypeData[i] = g_strdup(labels[i]);
	if(nLabels>0) GeomConv.Data = g_malloc(GeomConv.Ntype*sizeof(gchar**) );
	else GeomConv.Data = NULL;
	for(i = 0;i<GeomConv.Ntype;i++) GeomConv.Data[i] = NULL;
	GeomConv.TypeCalcul = NULL;
	GeomConv.NumGeom = NULL;
	GeomConv.GeomFile = g_strdup(fileName);

	return GeomConv;
}
/*********************************************************************/
static gint find_energy_gabedit_geoms(gchar* FileName)
{
	guint  i=0;
	guint  j=0;
	guint  k=0;
	gchar t[BSIZE];
	gchar dum1[BSIZE];
	gchar dum2[BSIZE];
	FILE *file;
        gint Ncalculs = 0;
  	static DataGeomConv* GeomConv =NULL;
	gchar** labels = NULL;
	gint nGeometries = 0;
	gint nAtoms = 0;
	gint nLabels = 0;
	gboolean OK = FALSE;
	gchar* pdest = NULL;
        
 	file = FOpen(FileName, "rb"); 
        if(!file)
	{
		sprintf(t,_(" Error : I can not open file %s\n"),FileName);
		Message(t,_("Error"),TRUE);
		return -1;
	}
	OK = FALSE;
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file))break;
		pdest = strstr( t, "[GEOMS]");
 		if (pdest)
		{
			if(!fgets(t,BSIZE,file))break;
			sscanf(t,"%d %d",&nGeometries, &nLabels);
			if(nGeometries<1)break;
			if(nLabels<1)break;
			labels = g_malloc(nLabels*sizeof(gchar*));
			for(i=0;i<nLabels;i++) labels[i] = NULL;
			for(i=0;i<nLabels;i++)
			{
				labels[i] = g_malloc(BSIZE*sizeof(gchar));
				if(!fgets(t,BSIZE,file))break;
				if(sscanf(t,"%s%s",dum1,dum2)!=2)break;
				sprintf(labels[i],"%s(%s)",dum1,dum2);
			}
			if(i!=nLabels)
			{
				for(i=0;i<nLabels;i++) if(labels[i]) g_free(labels[i]);
				if(labels) g_free(labels);
				break;
			}
			sscanf(labels[0],"%s",t);
			uppercase(t);
			if(!strstr(t,"ENERG"))
			{
				for(i=0;i<nLabels;i++) if(labels[i]) g_free(labels[i]);
				if(labels) g_free(labels);
				break;
			}
			
			Ncalculs=1;
			GeomConv =  g_malloc(sizeof(DataGeomConv) );
  			GeomConv[Ncalculs-1] = init_geom_gabedit_geoms(FileName, nLabels,labels);
			for(i=0;i<nLabels;i++) if(labels[i]) g_free(labels[i]);
			if(labels) g_free(labels);
			GeomConv[Ncalculs-1].TypeCalcul =  g_strdup(" ");
			GeomConv[Ncalculs-1].fileType = GABEDIT_TYPEFILE_GABEDIT;
			GeomConv[Ncalculs-1].Npoint = nGeometries;
			GeomConv[Ncalculs-1].NumGeom =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gint));	
			for(i=0;(gint)i<GeomConv[Ncalculs-1].Ntype;i++)
			{
				GeomConv[Ncalculs-1].Data[i] =  g_malloc(GeomConv[Ncalculs-1].Npoint*sizeof(gchar*));	
				for(j=0;(gint)j<GeomConv[Ncalculs-1].Npoint;j++)
					GeomConv[Ncalculs-1].Data[i][j] = g_malloc(50*sizeof(gchar)); 
			}
			OK = TRUE;
			break;
		}
	}
	for(j=0;j<nGeometries;j++)
	{
		for(i=0;i<nLabels;i++) 
		{
			if(!fgets(t,BSIZE,file))break;
			sscanf(t,"%s",GeomConv[Ncalculs-1].Data[i][j]);
		}
		if(i!=nLabels) 
		{
			OK = FALSE;
			break;
		}
		if(!fgets(t,BSIZE,file))
		{
			OK = FALSE;
			break;
		}
		sscanf(t,"%d",&nAtoms);

		for(i=0; i<nAtoms; i++)
			if(!fgets(t,BSIZE,file))break;
		if(i!=nAtoms) 
		{
			OK = FALSE;
			break;
		}
	}

	fclose(file);
	if(!OK) 
	{
		if(Ncalculs>0)
		GeomConv[Ncalculs-1] =  free_geom_conv(GeomConv[Ncalculs-1]);
		return 3;
	}
        
	/*
	printf("Ncalculs  = %d\n ",Ncalculs);
	if(Ncalculs>0)
	{
		printf("Npoint  = %d\n ",GeomConv[Ncalculs-1].Npoint);
		printf("TypeCalcul  = %s\n ",GeomConv[Ncalculs-1].TypeCalcul);
		for(i=0;i<GeomConv[Ncalculs-1].Npoint;i++)
		{
			for(j=0;j<GeomConv[Ncalculs-1].Ntype;j++)
          			printf("%s ",GeomConv[Ncalculs-1].Data[j][i]);
       			printf("\n ");
		}
	}
	*/

	k = 0;
	for(i=0;(gint)i<Ncalculs;i++)
	{
		for(j=0;(gint)j<GeomConv[i].Npoint;j++)
		{
			k++;
			GeomConv[i].NumGeom[j] = k;
		}
	}
	create_energies_curves(GeomConv,Ncalculs);
	return 0;
}
/*********************************************************************/
void find_energy_molden(gchar* fileName)
{
	find_energy_molden_gabedit(fileName,GABEDIT_TYPEFILE_MOLDEN);
}
/*********************************************************************/
void find_energy_gabedit(gchar* fileName)
{
	gint res = find_energy_gabedit_geoms(fileName);
	if(res == 0) return;
	if(res == -1) return;
	find_energy_molden_gabedit(fileName,GABEDIT_TYPEFILE_GABEDIT);
}
/*********************************************************************/
void find_energy_gauss_output(gchar* fileName)
{
	GabEditTypeFile fileType = get_type_file(fileName);
	/* fprintf(stderr,"DEBUG : ktype = %d\n", fileType);*/
	if(fileType == GABEDIT_TYPEFILE_GAUSSIAN_SCANOPT) find_energy_gauss_scanOpt_output(fileName);
	else if(fileType == GABEDIT_TYPEFILE_GAUSSIAN_IRCOPT) find_energy_gauss_ircOpt_output(fileName);
	else find_energy_gauss_standard_output(fileName);
}
/*********************************************************************/
void find_energy_all(GtkWidget *wid,gpointer data)
{
 	gchar* fileName = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.logfile);
	if( iprogram == PROG_IS_GAMESS) find_energy_gamess_output(fileName);
	if( iprogram == PROG_IS_FIREFLY) find_energy_gamess_output(fileName);
	if( iprogram == PROG_IS_MPQC) find_energy_mpqc_output(fileName);
	if( iprogram == PROG_IS_MOLPRO) find_energy_molpro_log(fileName);
	if( iprogram == PROG_IS_ORCA) find_energy_orca_output(fileName);
	if( iprogram == PROG_IS_QCHEM) find_energy_qchem_log(fileName);
	if( iprogram == PROG_IS_NWCHEM) find_energy_nwchem_log(fileName);
	if( iprogram == PROG_IS_PSICODE) find_energy_psicode_log(fileName);
	if( iprogram == PROG_IS_DEMON) 
	{
 		gchar* fileName = g_strdup_printf("%s%s%s.molden",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.projectname);
		find_energy_molden(fileName);
		g_free(fileName);
	}
	if( iprogram == PROG_IS_MOPAC) 
	{
		GabEditTypeFile fileType = GABEDIT_TYPEFILE_UNKNOWN;
		gint ng = 0;
		gchar* ext=strstr(fileName,".aux");
		ng = find_energy_mopac_aux(fileName);
		if(ng==1 && ext && strlen(ext)>=4)
		{
			ext[1]='o';
			ext[2]='u';
			ext[3]='t';
			fileType = get_type_file(fileName);
			if(fileType == GABEDIT_TYPEFILE_MOPAC_SCAN) find_energy_mopac_scan_output(fileName);
			if(fileType == GABEDIT_TYPEFILE_MOPAC_IRC) find_energy_mopac_irc_output(fileName);
		}
	}
	if( iprogram == PROG_IS_MOLCAS)
	{
 		gchar* fileName = g_strdup_printf("%s%s%s.geo.molden",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.projectname);
		find_energy_molden(fileName);
		g_free(fileName);
	}
	if( iprogram == PROG_IS_GAUSS) 
	{
		find_energy_gauss_output(fileName);
	}
	g_free(fileName);
}
/************************************************************************************/
void read_geometries_conv_dalton(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
 	find_energy_dalton_output(fileName);
}
/************************************************************************************/
void read_geometries_conv_mopac(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
 	find_energy_mopac_aux(fileName);
}
/************************************************************************************/
void read_geometries_conv_mopac_irc(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
 	find_energy_mopac_irc_output(fileName);
}
/************************************************************************************/
void read_geometries_conv_mopac_scan(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
 	find_energy_mopac_scan_output(fileName);
}
/************************************************************************************/
void read_geometries_conv_gamess(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
 	find_energy_gamess_output(fileName);
}
/************************************************************************************/
void read_geometries_irc_gamess(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
 	find_energy_gamess_irc(fileName);
}
/************************************************************************************/
void read_geometries_conv_gaussian(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
 	find_energy_gauss_output(fileName);
}
/************************************************************************************/
void read_geometries_conv_molpro(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
 	find_energy_molpro_log(fileName);
}
/************************************************************************************/
void read_geometries_conv_orca(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
 	find_energy_orca_output(fileName);
}
/************************************************************************************/
void read_geometries_conv_vasp_outcar(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
 	find_energy_vasp_outcar(fileName);
}
/************************************************************************************/
void read_geometries_conv_vasp_xml(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
 	find_energy_vasp_xml(fileName);
}
/************************************************************************************/
void read_geometries_conv_nwchem(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
 	find_energy_nwchem_log(fileName);
}
/************************************************************************************/
void read_geometries_conv_psicode(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
 	find_energy_psicode_log(fileName);
}
/************************************************************************************/
void read_geometries_conv_qchem(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
 	find_energy_qchem_log(fileName);
}
/*************************************************************************************/
void read_geometries_conv_gabedit(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *FileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	find_energy_gabedit(FileName);
}
/************************************************************************************/
void read_geometries_conv_molden(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *FileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	find_energy_molden(FileName);
}
/************************************************************************************/
void read_geometries_conv_mpqc(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	find_energy_mpqc_output(fileName);
}
/************************************************************************************/
void read_geometries_conv_xyz(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	find_energy_xyz(fileName);
}
/********************************************************************************/
/********************************************************************************/
static void sensitive_password()
{
	GtkWidget* Table;
	Table = g_object_get_data(G_OBJECT (ResultRemoteFrame), "PasswordTable");
	if(fileopen.netWorkProtocol==GABEDIT_NETWORK_FTP_RSH)
		gtk_widget_show(Table);
	else
	{
#ifdef G_OS_WIN32
		gtk_widget_show(Table);
#else
		gtk_widget_hide(Table);
#endif
	}
}
/********************************************************************************/
static void sensitive_buttons_false()
{
	sensitive_password();
  	gtk_widget_set_sensitive(ResultRemoteFrame, FALSE);
}
/********************************************************************************/
static void sensitive_buttons_true()
{
	sensitive_password();
  	gtk_widget_set_sensitive(ResultRemoteFrame, TRUE);
	if(
		fileopen.remotehost && !this_is_a_backspace(fileopen.remotehost) &&
		fileopen.remoteuser && !this_is_a_backspace(fileopen.remoteuser) 
	)
	{
		if(iprogram == PROG_IS_GAUSS)
		{
			GtkWidget* button = g_object_get_data(G_OBJECT(ResultRemoteFrame), "OutButton");
  			gtk_widget_set_sensitive(button, FALSE);
			button = g_object_get_data(G_OBJECT(ResultRemoteFrame),"AuxButton");
  			gtk_widget_set_sensitive(button, FALSE);
		}
		else
		{
			GtkWidget* button = g_object_get_data(G_OBJECT(ResultRemoteFrame),"OutButton");
  			gtk_widget_set_sensitive(button, TRUE);
			button = g_object_get_data(G_OBJECT(ResultRemoteFrame),"AuxButton");
  			gtk_widget_set_sensitive(button, TRUE);
		}
	}
	else
  		gtk_widget_set_sensitive(ResultRemoteFrame, FALSE);

	
}
/*********************************************************************/
void set_sensitive_remote_frame(gboolean sensitive)
{
	if(sensitive)
		sensitive_buttons_true();
	else
		sensitive_buttons_false();
}
/*********************************************************************/
void create_bar_result(GtkWidget* Vbox)
{
    GtkWidget *Button;
    GtkWidget *vboxframe;
    GtkWidget *frame;
    GtkWidget *handlebox;
    GtkWidget *vbox;
    GtkWidget *Table;
    /* GtkWidget *Label;*/
    GtkWidget *hseparator;
    static gint log = LOGFILE;
    static gint out = OUTFILE;
    static gint mol = MOLDENFILE;
    static gint all = ALLFILES;
  
/* here  init_geom_gauss_conv();
*/

  handlebox =gtk_handle_box_new ();
  g_object_ref (handlebox);
  gtk_container_add( GTK_CONTAINER (Vbox), handlebox);
  gtk_handle_box_set_shadow_type (GTK_HANDLE_BOX(handlebox),GTK_SHADOW_IN);
/*
  GTK_SHADOW_NONE,
  GTK_SHADOW_IN,
  GTK_SHADOW_OUT,
  GTK_SHADOW_ETCHED_IN,
  GTK_SHADOW_ETCHED_OUT 
*/
  gtk_handle_box_set_handle_position  (GTK_HANDLE_BOX(handlebox),GTK_POS_TOP);   
  gtk_widget_show (handlebox);

  vbox = gtk_vbox_new (FALSE, 2);
  gtk_container_add( GTK_CONTAINER(handlebox), vbox);
  gtk_widget_show (vbox);

  frame = gtk_frame_new ("Local");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX(vbox), frame, FALSE, TRUE, 2); 
  gtk_widget_show (frame);


  vboxframe = create_vbox(frame);
  ResultLocalFrame = frame;



  /* The Update Button */
  Button = create_button(Fenetre,_("Update"));
  gtk_box_pack_start (GTK_BOX(vboxframe ), Button, FALSE, TRUE, 2);
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Button);
  gtk_widget_show(Button);
  g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)view_result,NULL);

  Button = create_button(Fenetre,_("Go to end"));
  g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)goto_end_result,NULL);
  gtk_box_pack_start (GTK_BOX(vboxframe ), Button, FALSE, TRUE, 2);
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_show(Button);

  Button = create_button(Fenetre,_("Update/end"));
  g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)view_result_end,NULL);
  gtk_box_pack_start (GTK_BOX(vboxframe ), Button, FALSE, TRUE, 2);
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_show(Button);

  Button = create_button(Fenetre,_("Geom. Conv."));
  g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)find_energy_all,NULL);
  gtk_box_pack_start (GTK_BOX(vboxframe ), Button, FALSE, TRUE, 2);
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_show(Button);

  Button = create_button(Fenetre,_("Dens. Orb."));
  g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)draw_density_orbitals_gamess_or_gauss_or_molcas_or_molpro,NULL);
  gtk_box_pack_start (GTK_BOX(vboxframe ), Button, FALSE, TRUE, 2);
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_show(Button);

  frame = gtk_frame_new (_("Remote"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX(vbox), frame, FALSE, TRUE, 2); 
  gtk_widget_show (frame);
  vboxframe = create_vbox(frame);

  Table = gtk_table_new(3,1,FALSE);
  gtk_container_add(GTK_CONTAINER(vboxframe),Table);
  gtk_widget_show_all(vboxframe);
  /* Label = add_label_table(Table,_("Password : "),0,0);*/
  add_label_table(Table,_("Password : "),0,0);
  ResultEntryPass = gtk_entry_new(); 
  gtk_entry_set_max_length  (GTK_ENTRY(ResultEntryPass),15);
  gtk_widget_set_size_request (ResultEntryPass,(gint)(ScreenHeight*0.05),-1);
  gtk_entry_set_visibility(GTK_ENTRY (ResultEntryPass),FALSE);

  gtk_table_attach(GTK_TABLE(Table),ResultEntryPass,0,1,1,2,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);
  hseparator = gtk_hseparator_new ();
  gtk_table_attach(GTK_TABLE(Table),hseparator,0,1,2,3,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);


  ResultRemoteFrame = frame;
  gtk_widget_set_sensitive(ResultRemoteFrame, FALSE);
  g_object_set_data(G_OBJECT (ResultRemoteFrame), "PasswordTable",Table);

  Button = create_button(Fenetre,_("Get All files"));
  g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)get_file_frome_remote_host,&all);
  gtk_box_pack_start (GTK_BOX(vboxframe ), Button, FALSE, TRUE, 2);
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_show(Button);
  g_object_set_data(G_OBJECT (ResultRemoteFrame), "AllButton",Button);
  g_signal_connect_swapped(G_OBJECT (ResultEntryPass), "activate",
			(GCallback) gtk_button_clicked,
			GTK_OBJECT (Button));

  Button = create_button(Fenetre,_("Get log file"));
  g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)get_file_frome_remote_host,&log);
  gtk_box_pack_start (GTK_BOX(vboxframe ), Button, FALSE, TRUE, 2);
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_show(Button);
  g_object_set_data(G_OBJECT (ResultRemoteFrame), "LogButton",Button);

  Button = create_button(Fenetre,_("Get out file"));
  g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)get_file_frome_remote_host,&out);

  gtk_box_pack_start (GTK_BOX(vboxframe ), Button, FALSE, TRUE, 2);
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_show(Button);
  g_object_set_data(G_OBJECT (ResultRemoteFrame), "OutButton",Button);

  Button = create_button(Fenetre,_("Get aux. files"));
  g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)get_file_frome_remote_host,&mol);

  gtk_box_pack_start (GTK_BOX(vboxframe ), Button, FALSE, TRUE, 2);
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_show(Button);
  g_object_set_data(G_OBJECT (ResultRemoteFrame), "AuxButton",Button);
}

