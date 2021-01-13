/* GeomOrbXYZ.c */
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
#include "../Utils/AtomsProp.h"
#include "../Utils/Vector3d.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Transformation.h"
#include "../Utils/Constants.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/OpenBabel.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsGL.h"
#include "GLArea.h"
#include "StatusOrb.h"
#include "AtomicOrbitals.h"
#include "BondsOrb.h"

#ifdef G_OS_WIN32
#include <fcntl.h>
#include <io.h>
#else 
#include <unistd.h>
#endif

#define MAXNAME 6
#define MAXATOMTYPE 4
#define MAXRESIDUENAME 4
#define MAXSYMBOL 2

typedef struct _OldGeometry
{
	gint numberOfAtoms;
	gchar** symbols;
	gdouble* X;
	gdouble* Y;
	gdouble* Z;
}OldGeometry;
static OldGeometry oldGeometry = {0,NULL,NULL,NULL,NULL};

/***************************************************************************************/
static void free_old_geometry()
{
	gint i;
	if(oldGeometry.numberOfAtoms==0)return;
	if(oldGeometry.X) g_free(oldGeometry.X);
	if(oldGeometry.Y) g_free(oldGeometry.Y);
	if(oldGeometry.Z) g_free(oldGeometry.Z);
	for(i=0;i<oldGeometry.numberOfAtoms;i++)
		if(oldGeometry.symbols[i])g_free(oldGeometry.symbols[i]);
	if(oldGeometry.symbols) g_free(oldGeometry.symbols);
	oldGeometry.numberOfAtoms = 0;
	oldGeometry.X = NULL;
	oldGeometry.Y = NULL;
	oldGeometry.Z = NULL;
	oldGeometry.symbols = NULL;
}
/***************************************************************************************/
void reset_old_geometry()
{
	gint i;
	free_old_geometry();
	if(nCenters<1) return;
	oldGeometry.numberOfAtoms = nCenters;
	oldGeometry.X = g_malloc(oldGeometry.numberOfAtoms*sizeof(gdouble));
	oldGeometry.Y = g_malloc(oldGeometry.numberOfAtoms*sizeof(gdouble));
	oldGeometry.Z = g_malloc(oldGeometry.numberOfAtoms*sizeof(gdouble));
	oldGeometry.symbols = g_malloc(oldGeometry.numberOfAtoms*sizeof(gchar*));
	for(i=0;i<oldGeometry.numberOfAtoms;i++)
	{
		oldGeometry.X[i] = GeomOrb[i].C[0];
		oldGeometry.Y[i] = GeomOrb[i].C[1];
		oldGeometry.Z[i] = GeomOrb[i].C[2];
		oldGeometry.symbols[i] = g_strdup(GeomOrb[i].Symb);
	}

}
/***************************************************************************************/
gboolean this_is_a_new_geometry()
{
	gint i;
	if(oldGeometry.numberOfAtoms<1) return TRUE;
	if(oldGeometry.numberOfAtoms != nCenters) return TRUE;
	for(i=0;i<oldGeometry.numberOfAtoms;i++)
		if(strcmp(GeomOrb[i].Symb,oldGeometry.symbols[i]) != 0) return TRUE;

	for(i=0;i<oldGeometry.numberOfAtoms;i++)
		if(fabs(GeomOrb[i].C[0]-oldGeometry.X[i])>1e-6) return TRUE;
	for(i=0;i<oldGeometry.numberOfAtoms;i++)
		if(fabs(GeomOrb[i].C[1]-oldGeometry.Y[i])>1e-6) return TRUE;
	for(i=0;i<oldGeometry.numberOfAtoms;i++)
		if(fabs(GeomOrb[i].C[2]-oldGeometry.Z[i])>1e-6) return TRUE;
	
	return FALSE;
}
/***************************************************************************************/
void free_geometry()
{
	gint i;
	set_status_label_info(_("File name"),_("Nothing"));
	set_status_label_info(_("File type"),_("Nothing"));
	set_status_label_info(_("Geometry"),_("Nothing"));
	init_dipole();
	reset_old_geometry();
	if(!GeomOrb)
	{
		nCenters = 0;
		return;
	}
	free_atomic_orbitals();
	for(i=0;i<nCenters;i++)
		if(GeomOrb[i].Symb)
			g_free(GeomOrb[i].Symb);
	g_free(GeomOrb);
	GeomOrb = NULL;
	nCenters = 0;
}
/*****************************************/
gdouble get_cube_length()
{
 gint i,j;
 gdouble min;
 gdouble max;
 
 min = GeomOrb[0].C[0];
 max = GeomOrb[0].C[0];
 for(i=0;i<nCenters;i++)
 {
   for(j=0;j<3;j++)
   {
	if(min>GeomOrb[i].C[j])
		min = GeomOrb[i].C[j];
	if(max<GeomOrb[i].C[j])
		max = GeomOrb[i].C[j];
   }
 }
 return (max -min );
}
/*****************************************/
void reset_grid_limits()
  {
 	gint i;
	gdouble t = get_cube_length();
	if(fabs(t)<1e-10)
		t = 5.0;
  	for(i=0;i<3;i++)
   		limits.MinMax[0][i] = -t;
  	for(i=0;i<3;i++)
   		limits.MinMax[1][i] = t;
  }
/*****************************************/

void create_xyz_gl_spheres_list()
{
	V4d Specular = {1.0f,1.0f,1.0f,1.0f};
	V4d Diffuse  = {0.0f,0.0f,0.0f,1.0f};
	V4d Ambiant  = {0.0f,0.0f,0.0f,1.0f};
	int i;
	int k;
	for(i=0;i<nCenters;i++)
	{
		Diffuse[0] = GeomOrb[i].Prop.color.red/(gdouble)65535;
		Diffuse[1] = GeomOrb[i].Prop.color.green/(gdouble)65535;
		Diffuse[2] = GeomOrb[i].Prop.color.blue/(gdouble)65535;
		for(k=0;k<3;k++)
			Ambiant[k] = Diffuse[k]*0.8;

		GeomOrb[i].Sphere = Sphere_Get_List(
			i+1,GeomOrb[i].Prop.radii,
			Specular,Diffuse,Ambiant);
	}
}

/*****************************************/
gint GetNumType(char *Symb)
{
 gint i;
 for(i=0;i<nCenters;i++)
 {
   	if(!strcmp(Symb,GeomOrb[i].Symb)) 
           return GeomOrb[i].NumType;
 }
 return -1;
}
/*****************************************/
void DefineType()
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
/*****************************************/
void PrintGeomOrb()
{
 gint i,j;
 Debug("Geometry(in au)\n");
 Debug("===============\n\n");
 for(i=0;i<nCenters;i++)
 {
   Debug("%3s ",GeomOrb[i].Symb);
   for(j=0;j<3;j++)
   	Debug("%9.6f ",GeomOrb[i].C[j]);
   Debug("\n");
 }
}
/******************************************************************************************************************/
gboolean gl_read_molden_gabedit_geom_conv_file(gchar *fileName, gint geometryNumber, GabEditTypeFile type)
{
	gchar* t;
	gboolean OK;
	gchar* AtomCoord[5];
	FILE *file;
	gint i;
	gint j;
	gint l;
	gint numgeom;
	gchar *pdest;
	gint nn;
	gchar* tmp;


	free_data_all();
	tmp = get_name_file(fileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	if(type == GABEDIT_TYPEFILE_GABEDIT) set_status_label_info(_("File type"),"Gabedit");
	if(type == GABEDIT_TYPEFILE_MOLDEN) set_status_label_info(_("File type"),"Molden");

	file = FOpen(fileName, "rb");

	if(file ==NULL)
	{
		t = g_strdup_printf(_("Sorry\nI can not open %s  file "),fileName);
		Message(t,_("Error"),TRUE);
		g_free(t);
		set_status_label_info(_("Geometry"),_("Nothing"));
		return FALSE;
	}
	t = g_malloc(BSIZE*sizeof(gchar));
	for(i=0;i<5;i++) AtomCoord[i] = g_malloc(BSIZE*sizeof(gchar));

	numgeom =0;
	OK=TRUE;
	while(!feof(file))
	{
		if(!fgets(t, BSIZE,file))break;
		pdest = strstr( t, "[GEOMETRIES]");
		if(pdest && strstr(t,"ZMAT"))
		{
			if(type == GABEDIT_TYPEFILE_MOLDEN)
				sprintf(t,_("Sorry\nMolden file with ZMAT coordinate is not supported by Gabedit"));
			if(type == GABEDIT_TYPEFILE_GABEDIT)
				sprintf(t,_("Sorry\nGabedit file with ZMAT coordinate is not supported by Gabedit"));

			Message(t,_("Error"),TRUE);
			g_free(t);
			set_status_label_info(_("Geometry"),_("Nothing"));
			return FALSE;
		}	
 		if (pdest)
		{
			while(!feof(file))
			{
				if(!fgets(t, BSIZE,file))break;

				str_delete_n(t);
				delete_last_spaces(t);
				delete_first_spaces(t);
				if(!isInteger(t))break;
               			numgeom++;
				if(numgeom == geometryNumber)
				{
					nn = atoi(t);
					if(nn<1)break;
    					GeomOrb=g_malloc(nn*sizeof(TypeGeomOrb));
					if(!fgets(t, BSIZE,file))break; /* title */
					for(j=0; j<nn; j++)
					{
						if(!fgets(t, BSIZE,file))break;
    						sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
						AtomCoord[0][0]=toupper(AtomCoord[0][0]);
						l=strlen(AtomCoord[0]);
						if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
    						GeomOrb[j].Symb=g_strdup(AtomCoord[0]);
    						for(i=0;i<3;i++) GeomOrb[j].C[i]=atof(AtomCoord[i+1])*ANG_TO_BOHR;
						GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
						GeomOrb[j].partialCharge = 0.0;
						GeomOrb[j].variable = TRUE;
						GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
					}
 					nCenters = nn;
					OK = TRUE;
					break;
				}
				else
				{
					nn = atoi(t);
					if(!fgets(t, BSIZE,file)) break;
					for(i=0; i<nn; i++)
						if(!fgets(t, BSIZE,file))break;
					if(i!=nn) { OK = FALSE; break;}
				}
				if(!OK) break;
			}
		}
		if(!OK) break;
	}

	fclose(file);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(nCenters == 0 ) { if(GeomOrb) g_free(GeomOrb); }
 	else DefineType();

	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),"Ok");
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
	return TRUE;
}
/*********************************************************************/
gboolean gl_read_xyz_file_geomi(gchar *fileName,gint geometryNumber)
{
	gchar* t;
	gboolean OK;
	gchar* AtomCoord[5];
	FILE *file;
	gint i;
	gint j;
	gint l;
	gint numgeom;
	gint nn;
	gchar* tmp;


	free_data_all();
	tmp = get_name_file(fileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"XYZ");

	file = FOpen(fileName, "rb");

	if(file ==NULL)
	{
		t = g_strdup_printf(_("Sorry\nI can not open %s  file "),fileName);
		Message(t,_("Error"),TRUE);
		g_free(t);
		set_status_label_info(_("Geometry"),_("Nothing"));
		return FALSE;
	}
	t = g_malloc(BSIZE*sizeof(gchar));
	for(i=0;i<5;i++) AtomCoord[i] = g_malloc(BSIZE*sizeof(gchar));

	numgeom =0;
	OK=TRUE;
	while(!feof(file))
	{
		if(!fgets(t, BSIZE,file))break;

		str_delete_n(t);
		delete_last_spaces(t);
		delete_first_spaces(t);
		if(!isInteger(t))break;
		numgeom++;
		if(numgeom == geometryNumber)
		{
			nn = atoi(t);
			if(nn<1)break;
    			GeomOrb=g_malloc(nn*sizeof(TypeGeomOrb));
			if(!fgets(t, BSIZE,file))break; /* title */
			for(j=0; j<nn; j++)
			{
				if(!fgets(t, BSIZE,file))break;
    				sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
				AtomCoord[0][0]=toupper(AtomCoord[0][0]);
				l=strlen(AtomCoord[0]);
				if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
    				GeomOrb[j].Symb=g_strdup(AtomCoord[0]);
    				for(i=0;i<3;i++) GeomOrb[j].C[i]=atof(AtomCoord[i+1])*ANG_TO_BOHR;
				GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
				GeomOrb[j].partialCharge = 0.0;
				GeomOrb[j].variable = TRUE;
				GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
			}
 			nCenters = nn;
			OK = TRUE;
			break;
		}
		else
		{
			nn = atoi(t);
			if(!fgets(t, BSIZE,file)) break;
			for(i=0; i<nn; i++)
				if(!fgets(t, BSIZE,file))break;
			if(i!=nn) { OK = FALSE; break;}
		}
		if(!OK) break;
	}

	fclose(file);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(nCenters == 0 ) { if(GeomOrb) g_free(GeomOrb); }
 	else DefineType();

	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),"Ok");
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
	return TRUE;
}
/********************************************************************************/
gboolean gl_read_xyz_file(gchar* FileName)
{
 gchar *t;
 gchar *tmp=NULL;
 gboolean OK;
 gchar *AtomCoord[5];
 FILE *fd;
 guint taille=BSIZE;
 guint i;
 gint j;
 gint l;
 gdouble scal;
 gint n;

 for(i=0;i<5;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(char));

 
 if ((!FileName) || (strcmp(FileName,"") == 0))
 {
	Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    	return FALSE ;
 }


 t=g_malloc(taille);
 fd = FOpen(FileName, "rb");
 OK=TRUE;
 if(fd!=NULL)
 {
  { char* e = fgets(t,taille,fd);}
  n = atoi(t);
  if(n < 1)
  {
	OK=FALSE;
	Message(_("Sorry\n this is not a XYZ file"),_("Error"),TRUE);
	goto end;
  }
  free_data_all();
  /* nCenters = n;*/
  if(OK) GeomOrb=g_malloc(n*sizeof(TypeGeomOrb));
  tmp = get_name_file(FileName);
  set_status_label_info(_("File name"),tmp);
  g_free(tmp);
  set_status_label_info(_("File type"),"XYZ");
  set_status_label_info(_("Geometry"),_("Reading"));

  init_dipole();
  progress_orb(0,GABEDIT_PROGORB_READGEOM,TRUE);
  scal = (gdouble)1.01/n;
  { char* e = fgets(t,taille,fd);}
  j=-1;
  while(!feof(fd) && OK && (j<(gint)n))
  {
	  
	j++;
	if(j>=(gint)n) break;
	progress_orb(scal,GABEDIT_PROGORB_READGEOM,FALSE);
	{ char* e = fgets(t,taille,fd);}
	sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
	AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	l=strlen(AtomCoord[0]);
	if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
	GeomOrb[j].Symb=g_strdup(AtomCoord[0]);

	GeomOrb[j].C[0] = atof(ang_to_bohr(AtomCoord[1]));
	GeomOrb[j].C[1] = atof(ang_to_bohr(AtomCoord[2]));
	GeomOrb[j].C[2] = atof(ang_to_bohr(AtomCoord[3]));
	GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
	GeomOrb[j].partialCharge = 0.0;
	GeomOrb[j].variable = TRUE;
	GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
  }
  fclose(fd);

  nCenters = n;
  if(nCenters>10000)
  {
  	TypeGeom = GABEDIT_TYPEGEOM_WIREFRAME;
  	RebuildGeomD = TRUE;
  }
  else
  {
  }
  
 }
end:
 g_free(t);
 for(i=0;i<5;i++)
	g_free(AtomCoord[i]);
 if(OK)
 {
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	buildBondsOrb();
	glarea_rafresh(GLArea);
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
 }
 return OK;
}
/********************************************************************************/
void gl_read_xyz_file_sel(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();
 
	add_objects_for_new_grid();
 	gl_read_xyz_file(FileName);
}
/********************************************************************************/
static gchar* get_symb_atom(gchar* symb)
{
	gchar* S;
	gchar Forbidden[]={'0','1','2','3','4','5','6','7','8','9'};

	if(strlen(symb)==1)
		S = g_strdup(symb);
	else
	{
		gint Ok = 1;
		gint i;
		for(i=0;i<10;i++)
			if(symb[1]== Forbidden[i])
			{
				Ok =0;
				break;
			}
		if(Ok==1)
		{
			S = g_strdup_printf("%c%c",toupper(symb[0]),symb[1]);
		}
		else
			S = g_strdup_printf("%c",toupper(symb[0]));
	}
	if(!test_atom_define(S))
		S = g_strdup_printf("%c",toupper(symb[0]));


	return S;
}
/*************************************************************************************/
static gboolean read_atom_pdb_file(gchar* line,gchar* listFields[])
{
	gint i;
	gint k = 0;
	if(strlen(line)<54)
		return FALSE;

	/* 0 -> Atom Type */
	k = 0;
	for(i=0;i<MAXATOMTYPE;i++)
		listFields[k][i] = line[13+i-1];
	listFields[k][MAXATOMTYPE] = '\0';
	if(isdigit(listFields[k][0]))
	{
		gchar c0 = listFields[k][0];
		for(i=0;i<MAXATOMTYPE-1;i++)
			listFields[k][i] = listFields[k][i+1];
		listFields[k][MAXATOMTYPE-1] = c0;
	}

	/* 1-> Residue Name */
	k = 1;
	for(i=0;i<MAXRESIDUENAME;i++)
		listFields[k][i] = line[17+i-1];
	listFields[k][MAXRESIDUENAME] = '\0';

	/* 2-> Residue Number */
	k = 2;
	for(i=0;i<4;i++)
		listFields[k][i] = line[23+i-1];
	listFields[k][4] = '\0';
	/* 3-> x */
	k = 3;
	for(i=0;i<8;i++)
		listFields[k][i] = line[31+i-1];
	listFields[k][8] = '\0';

	/* 4-> y */
	k = 4;
	for(i=0;i<8;i++)
		listFields[k][i] = line[39+i-1];
	listFields[k][8] = '\0';

	/* 5-> z */
	k = 5;
	for(i=0;i<8;i++)
		listFields[k][i] = line[47+i-1];
	listFields[k][8] = '\0';

	/* 6-> Symbol */
	k = 6;
	if(strlen(line)>=78)
	{
		for(i=0;i<2;i++)
		{
			listFields[k][i] = line[76+i];
		}
		listFields[k][2] = '\0';
		if(listFields[k][1]==' ')
			listFields[k][1] = '\0';
		if(listFields[k][0]==' ')
			listFields[k][0] = '\0';
	}
	else
		listFields[k][0] = '\0';
	/* 7-> Charge */
	k = 7;
	if(strlen(line)>=80)
	{
		for(i=0;i<(gint)strlen(line)-79+1;i++)
			listFields[k][i] = line[79+i-1];

		listFields[k][strlen(line)-79+1] = '\0';

		if(listFields[k][strlen(line)-79]=='\n')
			listFields[k][strlen(line)-79]='\0';

	}
	else
		listFields[k][0] = '\0';

	for(i=0;i<8;i++)
	{
		delete_last_spaces(listFields[i]);
		delete_first_spaces(listFields[i]);
	}
	return TRUE;

}
/********************************************************************************/
gboolean gl_read_pdb_file(gchar* FileName)
{
	gchar *t;
	gchar *tmp=NULL;
	gboolean OK;
	FILE *fd;
	guint taille=BSIZE;
	guint i;
	gint j;
	gchar *listFields[8];

	if ((!FileName) || (strcmp(FileName,"") == 0))
	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return FALSE ;
	}


	fd = FOpen(FileName, "rb");
	if(fd==NULL)
	{
		gchar* tmp = g_strdup_printf(_("Sorry\n I can not open %s file"), FileName);
		Message(tmp,_("Error"),TRUE);
		g_free(tmp);
    		return FALSE ;
	}
	for(i=0;i<8;i++) listFields[i]=g_malloc(taille*sizeof(gchar));
	t=g_malloc(taille*sizeof(gchar));
	OK=TRUE;
	free_data_all();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"pdb");
	set_status_label_info(_("Geometry"),_("Reading"));
  	init_dipole();
	j=0;
	GeomOrb= NULL;
	while(!feof(fd))
	{
    		if(!fgets(t,taille,fd)) break;
    		sscanf(t,"%s",listFields[0]);
		if(strcmp(listFields[0],"HETATM")!=0 && strcmp(listFields[0],"ATOM")!=0) continue;
		if(!strcmp(t,"END")) break;
		if(!read_atom_pdb_file(t,listFields)) continue;
		/* 0 -> Atom Type  1-> Residue Name  2-> Residue Number 
		 * 3-> x  4-> y  5-> z  6-> Symbol 7-> Charge */

		GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));
		if(strlen(listFields[6])<=0) 
		{
			/* printf("PPPP listFields[6] = %s\n",listFields[6]);*/
			GeomOrb[j].Symb= get_symb_atom(listFields[0]);
		}
		else GeomOrb[j].Symb= g_strdup(listFields[6]);

    		GeomOrb[j].C[0] = atof(ang_to_bohr(listFields[3]));
    		GeomOrb[j].C[1] = atof(ang_to_bohr(listFields[4]));
    		GeomOrb[j].C[2] = atof(ang_to_bohr(listFields[5]));
    		GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb[j].partialCharge = 0.0;
		GeomOrb[j].variable = TRUE;
		GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
		j++;
	}
	if(j!=0)
	{
		nCenters = j;
		readBondsPDB(fd);
	}
	fclose(fd);

	OK = FALSE;
	if(j!=0) OK = TRUE;

	if(OK)
	{
		nCenters = j;
		if(nCenters>1000)
		{
  			TypeGeom = GABEDIT_TYPEGEOM_WIREFRAME;
  			RebuildGeomD = TRUE;
		}
	}
	else
	{
		nCenters = 0;
		set_status_label_info(_("Geometry"),_("Nothing"));
	}
	g_free(t);
	for(i=0;i<8;i++) g_free(listFields[i]);
	if(OK)
	{
		RebuildGeomD = TRUE;
		if(this_is_a_new_geometry()) free_objects_all();
		/* buildBondsOrb();*/
		glarea_rafresh(GLArea);
		reset_grid_limits();
		init_atomic_orbitals();
		set_status_label_info(_("Geometry"),_("Ok"));
	}
	return OK;
}
/********************************************************************************/
void gl_read_pdb_file_sel(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();
 
	add_objects_for_new_grid();
 	gl_read_pdb_file(FileName);
}
/*************************************************************************************/
static void read_hin_numbers_of_atoms(FILE* file, int* natoms, int* nresidues)
{
	guint taille = BSIZE;
	gchar t[BSIZE];
	gchar dump[BSIZE];

	*natoms = 0;
	*nresidues = 0;


	fseek(file, 0L, SEEK_SET);
	while(!feof(file))
	{
    		if(!fgets(t,taille,file)) break;
    		sscanf(t,"%s",dump);
		g_strdup(dump);
		if(!strcmp(dump,"ATOM")) (*natoms)++;
		if(!strcmp(dump,"RES")) (*nresidues)++;
	}
}
/*************************************************************************************/
static gboolean read_atom_hin_file(FILE* file,gchar* listFields[])
{
	guint taille = BSIZE;
	gchar t[BSIZE];
	gchar dump[BSIZE];
	gint i;

    	if(!fgets(t,taille,file)) return FALSE;
    	sscanf(t,"%s",dump);
	g_strdup(dump);
	if(strcmp(dump,"ATOM")!=0)
	{
		if(strcmp(dump,"RES")==0)
		{
    			sscanf(t,"%s %s %s",dump,dump,listFields[1]);
			sprintf(listFields[0],"Unknown");
		}
		else
			return FALSE;
	}
	else
	{
		/* 0 -> Atom Type PDB Style*/
		/* 1 -> Atom Symbol*/
		/* 2 -> Atom Type Amber*/
		/* 3 -> Atom Charge*/
		/* 4 -> x*/
		/* 5 -> y*/
		/* 6 -> z*/
    		sscanf(t,"%s %s %s %s %s %s %s %s %s %s",dump,dump,listFields[0],listFields[1],listFields[2],dump,listFields[3],listFields[4],listFields[5],listFields[6]);
	}
	for(i=0;i<6;i++)
	{
		delete_last_spaces(listFields[i]);
		delete_first_spaces(listFields[i]);
	}
	return TRUE;

}
/********************************************************************************/
gboolean gl_read_hin_file(gchar* FileName)
{
	gchar *t;
	gchar *tmp=NULL;
	gboolean OK;
	FILE *fd;
	guint taille=BSIZE;
	guint i;
	gint j;
	gchar *listFields[8];
	int natoms = 0;
	int nresidues = 0;

	if ((!FileName) || (strcmp(FileName,"") == 0))
	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return FALSE ;
	}

	fd = FOpen(FileName, "rb");

	if(fd==NULL)
	{
		gchar* tmp = g_strdup_printf(_("Sorry\n I can not open %s file"), FileName);
		Message(tmp,_("Error"),TRUE);
		g_free(tmp);
    		return FALSE ;
	}
	read_hin_numbers_of_atoms(fd,&natoms, &nresidues);
	if(natoms<1)
	{
		gchar* tmp = g_strdup_printf(_("Sorry\n I can not read grometry from %s file"), FileName);
		Message(tmp,_("Error"),TRUE);
		g_free(tmp);
    		return FALSE ;
	}

	OK=TRUE;
	for(i=0;i<8;i++) listFields[i]=g_malloc(taille*sizeof(gchar));
	t=g_malloc(taille*sizeof(gchar));

	free_data_all();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Hyperchem");
	set_status_label_info(_("Geometry"),_("Reading"));
  	init_dipole();
	j=0;

	GeomOrb=g_malloc(natoms*sizeof(TypeGeomOrb));
	fseek(fd, 0L, SEEK_SET);
	j = 0;
	i = -1; /* number residu */
	while(!feof(fd))
	{
		if(!read_atom_hin_file(fd,listFields))continue;
		if(strcmp(listFields[0],"Unknown")==0)
		{
			i++;
			continue;
		}
		/* 0 -> Atom Type PDB Style*/
		/* 1 -> Atom Symbol*/
		/* 2 -> Atom Type Amber*/
		/* 3 -> Atom Charge*/
		/* 4 -> x*/
		/* 5 -> y*/
		/* 6 -> z*/
		/* printf("%s %s %s %s %s %s %s \n",listFields[0],listFields[1],listFields[2],listFields[3],listFields[4],listFields[5],listFields[6]);*/
		GeomOrb[j].Symb= g_strdup(listFields[1]);

    		GeomOrb[j].C[0] = atof(ang_to_bohr(listFields[4]));
    		GeomOrb[j].C[1] = atof(ang_to_bohr(listFields[5]));
    		GeomOrb[j].C[2] = atof(ang_to_bohr(listFields[6]));
    		GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb[j].partialCharge = atof(listFields[3]);
		GeomOrb[j].variable = TRUE;
		GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
		j++;
		if(j>=natoms)break;
	}
	OK = TRUE;
	if(natoms!=j) OK = FALSE;

	if(OK)
	{
		nCenters = natoms;
		if(nCenters>10000)
		{
  			TypeGeom = GABEDIT_TYPEGEOM_WIREFRAME;
  			RebuildGeomD = TRUE;
		}
		readBondsHIN(fd);
	}
	else
	{
		nCenters = 0;
		set_status_label_info(_("Geometry"),_("Nothing"));
	}
	fclose(fd);
	g_free(t);
	for(i=0;i<8;i++) g_free(listFields[i]);
	if(OK)
	{
		RebuildGeomD = TRUE;
		if(this_is_a_new_geometry()) free_objects_all();
		/* buildBondsOrb();*/
		glarea_rafresh(GLArea);
		reset_grid_limits();
		init_atomic_orbitals();
		set_status_label_info(_("Geometry"),_("Ok"));
	}
	return OK;
}
/********************************************************************************/
void gl_read_hin_file_sel(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();
 
	add_objects_for_new_grid();
 	gl_read_hin_file(FileName);
}
/*****************************************************************************************/
static void conversion_to_hin_and_read(GtkWidget *wid,gpointer data)
{
	gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	gchar *outputFileName =  g_strdup_printf("%s%stmp%sgeom.hin",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	G_CONST_RETURN gchar* inputFileName;
	gchar inputFileType[100];
	gchar outputFileType[] ="hin" ;
	GtkWidget* entryFileName = NULL;
	GtkWidget* entryFileType = NULL;
	G_CONST_RETURN gchar* t0;
	gchar* t;
	gchar* strout;
	gchar* strerr;
	G_CONST_RETURN gchar* babelCommand =get_open_babel_command();

	GtkWidget *Win = (GtkWidget *)data;
        if(!this_is_an_object((GtkObject*)Win)) return;

	entryFileName = g_object_get_data (G_OBJECT (Win), "EntryFileName");
	entryFileType = g_object_get_data (G_OBJECT (Win), "EntryCombo");
	
        if(!this_is_an_object((GtkObject*)entryFileName)) return;
        if(!this_is_an_object((GtkObject*)entryFileType)) return;

	inputFileName = gtk_entry_get_text(GTK_ENTRY(entryFileName)); 

	t0 = gtk_entry_get_text(GTK_ENTRY(entryFileType)); 
	sscanf(t0,"%s",inputFileType);
	if(strstr(t0,"Automatic")) t = g_strdup_printf("%s %s %s", babelCommand, inputFileName, outputFileName);
	else t = g_strdup_printf("%s -i%s %s -o%s %s", babelCommand, inputFileType, inputFileName, outputFileType, outputFileName);

	unlink(fout);
	unlink(ferr);
	run_local_command(fout,ferr,t,FALSE);
	strout = cat_file(fout,FALSE);
	strerr = cat_file(ferr,FALSE);
  	if(strout)
	{
 		Message(strout, _("Info"), TRUE);
		g_free(strout);
	}
  	if(strerr)
	{
 		Message(strerr, _("Error"), TRUE);
		g_free(strerr);
	}
	else
	{
		gl_read_hin_file(outputFileName);
	}
	unlink(fout);
	unlink(ferr);
	unlink(outputFileName);
	g_free(fout);
	g_free(ferr);
	g_free(t);
	g_free(outputFileName);
}
/********************************************************************************/
void gl_create_babel_read_dialogue()
{
	GtkWidget* okButton = create_babel_read_save_dialogue(TRUE);
	GtkWidget* win = g_object_get_data (G_OBJECT (okButton), "Window");

	g_signal_connect(G_OBJECT(okButton), "clicked", G_CALLBACK(conversion_to_hin_and_read),win);
	g_signal_connect_swapped(G_OBJECT(okButton), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(win));
}
/********************************************************************************/
gboolean gl_read_molden_or_gabedit_file_geom(gchar *FileName,gint type)
{
 	gchar *t;
	gchar *tmp;
 	gchar *AtomCoord[5];
 	FILE *fd;
 	guint taille=BSIZE;
 	guint idummy;
 	gint iz;
 	guint i;
 	gint j=0;
 	gint l;
	gboolean Angs = FALSE;
	gchar title[BSIZE];
	gchar titleCap[BSIZE];
	gchar tCap[BSIZE];
	gchar message[BSIZE];

	if(type == 0)
	{
		sprintf(title,"[Molden Format]");
		sprintf(message,"Sorry\nThis file is not a molden type file");
	}
	else
	{
		sprintf(title,"[Gabedit Format]");
		sprintf(message,"Sorry\nThis file is not a gabedit type file");
	}

	sprintf(titleCap,"%s",title);
	uppercase(titleCap);

 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));
  
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
 		for(i=0;i<5;i++) g_free(AtomCoord[i]);
    		return FALSE;
 	}

 	t=g_malloc(taille*sizeof(gchar));
 	fd = FOpen(FileName, "rb");
 	if(fd ==NULL)
 	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
 		g_free(t);
 		for(i=0;i<5;i++) g_free(AtomCoord[i]);
  		return FALSE;
 	}
	if(type==0)
	{
		gboolean ok = FALSE;
		while(!feof(fd))
		{
    			if(!fgets(t,taille,fd)) break;
			uppercase(t);
			if(strstr( t, "[MOLDEN FORMAT]" ))
			{ 
				ok = TRUE;
				break;
			}
		}
		if(ok) sprintf(tCap,"%s",t);
		else sprintf(tCap,"ERROR");
	}
	else{
    		if(!feof(fd)) { char* e = fgets(t,taille,fd);}
		sprintf(tCap,"%s",t);
	}
	uppercase(tCap);

	if(!strstr(tCap,titleCap))
	{
  		Message(message,_("Error"),TRUE);
 		g_free(t);
 		for(i=0;i<5;i++) g_free(AtomCoord[i]);
  		return FALSE;
	}

	free_data_all();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	if(type == 0)
		set_status_label_info(_("File type"),"Molden");
	else
		set_status_label_info(_("File type"),"Gabedit");	
	set_status_label_info(_("Geometry"),_("Reading"));
	j = 0;
 	while(!feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
		uppercase(t);
		if( !strstr( t, "[ATOMS" ) )
			continue;
		if( strstr( t, "ANGS" ))
			Angs = TRUE;
		else
			Angs = FALSE;

		j = 0;
		if(GeomOrb)
			g_free(GeomOrb);
		GeomOrb = NULL;
		while( !feof(fd) )
		{
			gint n = 0;
    			{ char* e = fgets(t,taille,fd);}
			n = sscanf(t,"%s %d %d %s %s %s",
				AtomCoord[0],&idummy,&iz,
				AtomCoord[1],AtomCoord[2],AtomCoord[3]);
			if(n<6) break;
    			if(GeomOrb==NULL) GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    			else GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]);
          		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);

    			GeomOrb[j].Symb=g_strdup(AtomCoord[0]);
			if(!test_atom_define(GeomOrb[j].Symb))
			{
				GeomOrb[j].Symb = get_symbol_using_z(iz);
			}

    			for(i=0;i<3;i++) GeomOrb[j].C[i]=atof((AtomCoord[i+1]));
			GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
			GeomOrb[j].partialCharge = 0.0;
			GeomOrb[j].variable = TRUE;
			GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
			j++;
  		}
		break;
	}

 	nCenters = j;
 	fclose(fd);
 	g_free(t);
 	for(i=0;i<5;i++)
		g_free(AtomCoord[i]);
	if(Angs)
	for(j=0;j<nCenters;j++)
		for(i=0;i<3;i++)
			GeomOrb[j].C[i] *=ANG_TO_BOHR;

 	if(nCenters == 0 )
	{
		if(GeomOrb) g_free(GeomOrb);
		set_status_label_info(_("Geometry"),_("Nothing"));
		return FALSE;
	}
 	else
	{
  		DefineType();
  		/* PrintGeomOrb();*/
	}
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	return TRUE;
}
/********************************************************************************/
gboolean gl_read_molden_file_geom(gchar *FileName)
{
	if( gl_read_molden_or_gabedit_file_geom(FileName,0))
	{
		RebuildGeomD = TRUE;
		if(this_is_a_new_geometry()) free_objects_all();
		glarea_rafresh(GLArea);
		return TRUE;
	}
	return FALSE;
}
/********************************************************************************/
gboolean  gl_read_gabedit_file_geom(gchar *FileName)
{
	if(gl_read_molden_or_gabedit_file_geom(FileName,1))
	{
		RebuildGeomD = TRUE;
		if(this_is_a_new_geometry()) free_objects_all();
		glarea_rafresh(GLArea);
		return TRUE;
	}
	return FALSE;
}
/********************************************************************************/
gboolean gl_read_molpro_file_geom_pos(gchar *FileName,long int pos)
{
 	gchar t[BSIZE];
 	gchar *tmp = NULL;
 	gboolean OK;
 	gchar *AtomCoord[5];
	gchar AtomCharge[100];
 	FILE *fd;
 	guint taille=BSIZE;
 	guint idummy;
 	guint i;
 	gint j=0;
 	gint l;
 	long int n=-1;
	gint kk;
	gchar dum[100];

 	for(i=0;i<5;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(char));
  
 	fd = FOpen(FileName, "rb");
 	if(fd ==NULL)
 	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
 		for(i=0;i<5;i++)
			g_free(AtomCoord[i]);
  		return FALSE;
 	}

	free_data_all();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Molpro");
	set_status_label_info(_("Geometry"),_("Reading"));
 	OK=FALSE;
	n=-1;
 	while(!feof(fd))
	{
		n++;
    		{ char* e = fgets(t,taille,fd);}
		if (n==pos)
		{
    			{ char* e = fgets(t,taille,fd);}
    			{ char* e = fgets(t,taille,fd);}
    			{ char* e = fgets(t,taille,fd);}
                	OK = TRUE;
	  		break;
	  	}
        }
 	if(!OK)
	{
  		Message(_("Sorry\nI can not read geometry in this file"),_("Error"),TRUE);
 		fclose(fd);
 		for(i=0;i<5;i++)
			g_free(AtomCoord[i]);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Geometry"),_("Nothing"));
		return FALSE;
    	}

	j=-1;
  	while(!feof(fd) )
  	{
    		{ char* e = fgets(t,taille,fd);}
		if ( !strcmp(t,"\n"))
				break;
    		j++;
    		if(GeomOrb==NULL)
			GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    		else
    			GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));

		/*
		kk = sscanf(t,"%d %s %s %s %s %s %s",&idummy, AtomCoord[0],AtomCoord[1],AtomCoord[1], AtomCoord[2],AtomCoord[3], dum);
		if(kk==7) sscanf(t,"%d %s %s %s %s %s %s",&idummy, AtomCoord[0],AtomCoord[1],dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);
		*/
		kk = sscanf(t,"%d %s %s %s %s %s %s",&idummy, AtomCoord[0],AtomCharge,AtomCoord[1], AtomCoord[2],AtomCoord[3], dum);
		if(kk==7) sscanf(t,"%d %s %s %s %s %s %s",&idummy, AtomCoord[0],AtomCharge,dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);

		for(i=0;i<(gint)strlen(AtomCoord[0]);i++) if(isdigit(AtomCoord[0][i])) AtomCoord[0][i] = ' ';
		delete_all_spaces(AtomCoord[0]);

		AtomCoord[0][0]=toupper(AtomCoord[0][0]);
 		l=strlen(AtomCoord[0]);
          	if (l==2)
 			AtomCoord[0][1]=tolower(AtomCoord[0][1]);

		if (atoi(AtomCharge) == 0)
			sprintf(AtomCoord[0],"X");

    		GeomOrb[j].Symb=g_strdup(AtomCoord[0]);
    		for(i=0;i<3;i++)
			GeomOrb[j].C[i]=atof((AtomCoord[i+1]));

		GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb[j].partialCharge = 0.0;
		GeomOrb[j].variable = TRUE;
		GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
  	}

 	nCenters = j+1;
 	fclose(fd);
 	for(i=0;i<5;i++)
		g_free(AtomCoord[i]);
 	if(nCenters == 0 )
  		g_free(GeomOrb);
 	else
	{
  		DefineType();
  		/* PrintGeomOrb();*/
	}
	RebuildGeomD = TRUE;
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	return TRUE;
}
/********************************************************************************/
gboolean gl_read_dalton_file_geomi(gchar *FileName,gint num)
{
 	gchar *t;
 	gchar *tmp = NULL;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	FILE *fd;
 	guint taille=BSIZE;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numgeom;
	gchar dum[100];
	gint kk;
	long int geompos = 0;

 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));
  
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No file selected"),_("Error"),TRUE);
 		for(i=0;i<5;i++) g_free(AtomCoord[i]);
    		return FALSE ;
 	}

 	t=g_malloc(taille);
 	fd = FOpen(FileName, "rb");
 	if(fd ==NULL)
 	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
 		g_free(t);
 		for(i=0;i<5;i++) g_free(AtomCoord[i]);
  		return FALSE;
 	}

	free_data_all();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Dalton");
 	numgeom =1;
 	do 
 	{
		set_status_label_info(_("Geometry"),_("Reading"));
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
			if ( strstr(t,"geometry (au)"))
			{
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}
 				numgeom++;
                		OK = TRUE;
	  			break;
	  		}
        	}
 		if(!OK && (numgeom == 1) )
		{
  			Message(_("Sorry\nI can not read geometry from this file"),_("Error"),TRUE);
 			fclose(fd);
 			g_free(t);
 			for(i=0;i<5;i++) g_free(AtomCoord[i]);
			set_status_label_info(_("File name"),_("Nothing"));
			set_status_label_info(_("File type"),_("Nothing"));
			set_status_label_info(_("Geometry"),_("Nothing"));
			return FALSE;
    		}
 		if(!OK)break;

  		j=-1;
  		while(!feof(fd) )
  		{
    			{ char* e = fgets(t,taille,fd);}
			if ( !strcmp(t,"\n"))
			{
				geompos = ftell(fd);
 				get_dipole_from_dalton_output_file(fd);
				fseek(fd, geompos, SEEK_SET);
				break;
			}
    			j++;
    			if(GeomOrb==NULL) GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    			else GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));

			kk = sscanf(t,"%s %s %s %s %s",AtomCoord[0],AtomCoord[1], AtomCoord[2],AtomCoord[3], dum);
			if(kk==5) sscanf(t,"%s %s %s %s %s",AtomCoord[0],dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);

			for(i=0;i<(gint)strlen(AtomCoord[0]);i++) if(isdigit(AtomCoord[0][i])) AtomCoord[0][i] = ' ';
			delete_all_spaces(AtomCoord[0]);
			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]);
          		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			/*
			printf("j = %d symb = %s %s %s %s \n",j, AtomCoord[0], AtomCoord[1], AtomCoord[2],AtomCoord[3]);
			*/

    			GeomOrb[j].Symb=g_strdup(AtomCoord[0]);
    			for(i=0;i<3;i++) GeomOrb[j].C[i]=atof((AtomCoord[i+1]));

			GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
			GeomOrb[j].partialCharge = 0.0;
			GeomOrb[j].variable = TRUE;
			GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
  		}
		if(num >0 && (gint)numgeom-1 == num) break;
			
 	}while(!feof(fd));

 	nCenters = j+1;
 	fclose(fd);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(nCenters == 0 ) g_free(GeomOrb);
 	else
	{
  		DefineType();
  		/* PrintGeomOrb();*/
	}
	RebuildGeomD = TRUE;
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	return TRUE;
}
/********************************************************************************/
void gl_get_charges_from_gaussian_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;
	gint ngrad=0;


  	while(!feof(fd) )
	{
    		pdest = NULL;
    		{ char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "Total atomic charges");
		if(!pdest) /* Gaussian 03 */
    			pdest = strstr( t, "atomic charges");
		 if(!pdest) /* Gaussian 09D */
                        pdest = strstr( t, "Mulliken charges and spin densities:");
		 if(!pdest) /* Gaussian 09D */
                        pdest = strstr( t, "Mulliken charges:");


		if(pdest)
		{
    			if(!feof(fd)) { char* e = fgets(t,taille,fd);}
			else
				break;

			for(i=0;i<N;i++)
			{
    				if(!feof(fd)) { char* e = fgets(t,taille,fd);}
				else
					break;
				if(sscanf(t,"%s %s %s",dump,dump,d)==3)
				{
					GeomOrb[i].partialCharge = atof(d);
				}
			}
			break;
		}
		else
		{
          		pdest = strstr( t, "GradGradGrad" );
			if(pdest)
			{
				ngrad++;
			}
			if(ngrad>2)
				break;
		}

	}
}
/********************************************************************************/
void gl_get_natural_charges_from_gaussian_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;
	gint ngrad =0;



  	while(!feof(fd) )
	{
    		pdest = NULL;
    		{ char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "Summary of Natural Population Analysis:");
		if(!pdest) /* Gaussian 03 */
    			pdest = strstr( t, "Summary of Natural Population Analysis:");

		if(pdest)
		{
    			if(!feof(fd)) { char* e = fgets(t,taille,fd);}
			else break;
    			if(!feof(fd)) { char* e = fgets(t,taille,fd);}
			else break;
			if(!strstr(t,"Natural Population"))break;
    			if(!feof(fd)) { char* e = fgets(t,taille,fd);}
			else break;
			if(!strstr(t,"Natural"))break;
    			if(!feof(fd)) { char* e = fgets(t,taille,fd);}
			else break;
			if(!strstr(t,"Charge"))break;
    			if(!feof(fd)) { char* e = fgets(t,taille,fd);}
			else break;
			if(!strstr(t,"-------------"))break;

			for(i=0;i<N;i++)
			{
    				if(!feof(fd)) { char* e = fgets(t,taille,fd);}
				else break;
				if(sscanf(t,"%s %s %s",dump,dump,d)==3)
				{
					GeomOrb[i].partialCharge = atof(d);
				}
			}
			break;
		}
		else
		{
          		pdest = strstr( t, "GradGradGrad" );
			if(pdest)
			{
				ngrad++;
			}
			if(ngrad>2)
				break;
		}

	}
}
/********************************************************************************/
void gl_get_esp_charges_from_gaussian_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;
	gint ngrad=0;


  	while(!feof(fd) )
	{
    		pdest = NULL;
    		{ char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "Charges from ESP fit");
		if(!pdest) /* Gaussian 03 */
    			pdest = strstr( t, "harges from ESP");

		if(pdest)
		{
    			if(!feof(fd)) { char* e = fgets(t,taille,fd);}
			else break;
    			if(!feof(fd)) { char* e = fgets(t,taille,fd);}
			else break;

			for(i=0;i<N;i++)
			{
    				if(!feof(fd)) { char* e = fgets(t,taille,fd);}
				else break;
				if(sscanf(t,"%s %s %s",dump,dump,d)==3)
				{
					GeomOrb[i].partialCharge = atof(d);
				}
			}
			break;
		}
		else
		{
          		pdest = strstr( t, "GradGradGrad" );
			if(pdest)
			{
				ngrad++;
			}
			if(ngrad>2)
				break;
		}

	}
}
/********************************************************************************/
void gl_get_charges_from_gamess_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;


  	while(!feof(fd) )
	{
    		pdest = NULL;
    		if(!fgets(t,taille,fd)) break;
    		pdest = strstr( t, "TOTAL MULLIKEN AND LOWDIN ATOMIC POPULATIONS");

		if(pdest)
		{
    			if(!fgets(t,taille,fd)) break;
			for(i=0;i<N;i++)
			{
    				if(!fgets(t,taille,fd)) break;
				if(sscanf(t,"%s %s %s %s %s %s",dump, dump ,dump, dump, dump, d)==6)
				{
					GeomOrb[i].partialCharge = atof(d);
				}
				else break;
			}
			break;
		}
	}
}
/********************************************************************************/
gboolean gl_read_gamess_file_geomi(gchar *FileName,gint num)
{
 	gchar *t;
 	gchar *tmp = NULL;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	FILE *fd;
 	guint taille=BSIZE;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numgeom;
	gchar dum[100];
	gint uni=1;
	long geompos = 0;

 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));
  
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No file selected"),_("Error"),TRUE);
 		for(i=0;i<5;i++) g_free(AtomCoord[i]);
    		return FALSE ;
 	}

 	t=g_malloc(taille);

#ifdef G_OS_WIN32 
 	fd = FOpen(FileName, "rb");
#else
	fd = FOpen(FileName, "rb");
#endif

 	if(fd ==NULL)
 	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
 		g_free(t);
 		for(i=0;i<5;i++) g_free(AtomCoord[i]);
  		return FALSE;
 	}

  	init_dipole();
	free_data_all();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Gamess");
 	numgeom =1;
 	do 
 	{
		set_status_label_info(_("Geometry"),_("Reading"));
 		OK=FALSE;
 		while(!feof(fd))
		{
			if(!fgets(t,taille,fd))break;
			if ( strstr(t,"COORDINATES (BOHR)"))
			{
    				{ char* e = fgets(t,taille,fd);}
 				numgeom++;
                		OK = TRUE;
				uni = 0;
	  			break;
	  		}
			if ( strstr(t,"COORDINATES OF ALL ATOMS ARE (ANGS)"))
			{
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}
 				numgeom++;
				uni=1;
				if((gint)numgeom-1 == num ) { OK = TRUE; break; }
				if(num<0 ) { OK = TRUE; break; }
	  		}
        	}
 		if(!OK && (numgeom == 1) )
		{
  			Message(_("Sorry\nI can not read geometry from this file"),_("Error"),TRUE);
 			fclose(fd);
 			g_free(t);
 			for(i=0;i<5;i++) g_free(AtomCoord[i]);
			set_status_label_info(_("File name"),_("Nothing"));
			set_status_label_info(_("File type"),_("Nothing"));
			set_status_label_info(_("Geometry"),_("Nothing"));
			return FALSE;
    		}
 		if(!OK)break;

  		j=-1;
  		while(!feof(fd) )
  		{
    			{ char* e = fgets(t,taille,fd);}
			if ( !strcmp(t,"\n")) break;
			if ( !strcmp(t,"\r\n")) break;
    			j++;
    			if(GeomOrb==NULL) GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    			else GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));

			sscanf(t,"%s %s %s %s %s",AtomCoord[0],dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);

			for(i=0;i<(gint)strlen(AtomCoord[0]);i++) if(isdigit(AtomCoord[0][i])) AtomCoord[0][i] = ' ';
			delete_all_spaces(AtomCoord[0]);
			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]);
          		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			/*
			printf("j = %d symb = %s %s %s %s \n",j, AtomCoord[0], AtomCoord[1], AtomCoord[2],AtomCoord[3]);
			*/

    			/* GeomOrb[j].Symb=g_strdup(AtomCoord[0]);*/
			GeomOrb[j].Symb=get_symbol_using_z(atoi(dum));
    			for(i=0;i<3;i++) 
			{
				GeomOrb[j].C[i]=atof((AtomCoord[i+1]));
				if(uni==1)GeomOrb[j].C[i] *= ANG_TO_BOHR;
			}

			GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
			GeomOrb[j].partialCharge = 0.0;
			GeomOrb[j].variable = TRUE;
			GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
  		}
		if(num >0 && (gint)numgeom-1 == num) break;
		if(num<0) geompos = ftell(fd);
 	}while(!feof(fd));

 	nCenters = j+1;
	printf("nCenters=%d\n",nCenters);

	if ( nCenters >0)
	{
		long int gp = 0;
		if(num<0) fseek(fd, geompos, SEEK_SET);
		fseek(fd, geompos, SEEK_SET);
		gp = ftell(fd);
		get_dipole_from_gamess_output_file(fd);
		fseek(fd, gp, SEEK_SET);
		gl_get_charges_from_gamess_output_file(fd,nCenters);
		/* PrintGeomOrb();*/
	}

 	fclose(fd);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(nCenters == 0 ) g_free(GeomOrb);
 	else
	{
  		DefineType();
  		/* PrintGeomOrb();*/
	}
	RebuildGeomD = TRUE;
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	return TRUE;
}
/********************************************************************************/
void gl_get_charges_from_mpqc_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
	gint i;
	gboolean OK = FALSE;


	fseek(fd, 0L, SEEK_SET);
	OK = FALSE;
  	while(!feof(fd) )
	{
    		if(!fgets(t,taille,fd))break;
    		if(strstr(t, "Natural Population Analysis"))
		{
			OK = TRUE;
			break;
		}
	}
	if(!OK) return;

	OK = FALSE;
  	while(!feof(fd) )
	{
    		if(!fgets(t,taille,fd))break;
    		if(strstr(t, "atom") && strstr(t, "charge"))
		{
			OK = TRUE;
			break;
		}
	}
	if(!OK) return;
	for(i=0;i<N && !feof(fd) ;i++)
	{
		if(!fgets(t,taille,fd)) break;
		if(sscanf(t,"%s %s %s",dump,dump,d)==3)
		{
			GeomOrb[i].partialCharge = atof(d);
		}
	}
	if(i==N) return;
	for(i=0;i<N;i++)
	{
		GeomOrb[i].partialCharge = 0.0;
	}

	return;
}
/********************************************************************************/
gboolean gl_read_mpqc_file_geomi(gchar *fileName,gint numGeometry)
{
 	gchar *t;
 	gchar *tmp = NULL;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	FILE *fd;
 	guint taille=BSIZE;
 	guint idummy;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numGeom;
	gdouble tmpReal;

 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));
  
 	fd = FOpen(fileName, "rb");

	free_data_all();
	tmp = get_name_file(fileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"MPQC");

	t=g_malloc(taille);

	numGeom = 0;
	do 
	{
		gboolean unitOfOutAng = FALSE;
		OK=FALSE;
		set_status_label_info(_("Geometry"),_("Reading"));
		while(!feof(fd))
		{
			if(!fgets(t,taille,fd)) break;
			if (strstr(t,"<Molecule>"))
			{
				gboolean OkUnit = FALSE;
				do{
		 			if(!fgets(t,taille,fd))break;
					if(strstr(t,"unit"))
					{
						OkUnit = TRUE;
						if(strstr(t,"angstrom"))unitOfOutAng=TRUE;
						break;
					};
					
				}while(!feof(fd));
				if(!OkUnit) break;
				numGeom++;
	       			if((gint)numGeom == numGeometry )
				{
					OK = TRUE;
		 			break;
				}
	       			if(numGeometry<0 )
				{
					OK = TRUE;
		 			break;
				}
			}
	       }
		if(!OK && (numGeom == 0) )
		{
	 		g_free(t);
	 		t = g_strdup_printf(_("Sorry\nI can read Geometry from %s  file "),fileName);
	 		Message(t,_("Error"),TRUE);
	 		g_free(t);
			for(i=0;i<5;i++) g_free(AtomCoord[i]);
	 		return FALSE;
		}
		if(!OK) break;
		OK = FALSE;
		while(!feof(fd) )
		{
	   		if(!fgets(t,taille,fd))break;
			if ( !(strstr(t,"atoms") && strstr(t,"geometry"))) continue;
			OK = TRUE;
			break;
		}
		if(!OK)
		{
	 		g_free(t);
	 		t = g_strdup_printf(_("Sorry\nI can read Geometry from %s  file "),fileName);
	 		Message(t,_("Error"),TRUE);
	 		g_free(t);
			for(i=0;i<5;i++) g_free(AtomCoord[i]);
	 		return FALSE;
		}

		j=-1;
		while(!feof(fd) )
		{
	   		if(!fgets(t,taille,fd))break;
	   		if(strstr(t,"}"))break;
			j++;

    			if(GeomOrb==NULL) GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    			else GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));

			for(i=0;i<strlen(t);i++) if(t[i]=='[' || t[i] ==']') t[i]=' ';
			sscanf(t,"%d %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
			for(i=1;i<=3;i++)
			{
				tmpReal = atof(AtomCoord[i]);
				sprintf(AtomCoord[i],"%lf",tmpReal);
			}

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);

			l=strlen(AtomCoord[0]);

			if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);

    			GeomOrb[j].Symb=g_strdup(AtomCoord[0]);
    			for(i=0;i<3;i++)
				if(unitOfOutAng)
    					GeomOrb[j].C[i]=atof(ang_to_bohr(AtomCoord[i+1]));
				else
    					GeomOrb[j].C[i]=atof(AtomCoord[i+1]);

			GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
			GeomOrb[j].partialCharge = 0.0;
			GeomOrb[j].variable = TRUE;
			GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
		}

		if(numGeometry<0) continue;
		if(OK) break;
	}while(!feof(fd));

 	nCenters = j+1;
	if(numGeometry<0) gl_get_charges_from_mpqc_output_file(fd,nCenters);
 	fclose(fd);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(nCenters == 0 ) g_free(GeomOrb);
 	else DefineType();

	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	RebuildGeomD = TRUE;
	return TRUE;
}
/********************************************************************************/
gboolean gl_read_molcas_file_geomi(gchar *FileName,gint num)
{
 	gchar *t;
 	gchar *tmp = NULL;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	FILE *file;
 	guint taille=BSIZE;
 	guint idummy;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numgeom;

  
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No file selected"),_("Error"),TRUE);
    		return FALSE ;
 	}


 	t=g_malloc(taille);
 	file = FOpen(FileName, "rb");
 	if(file ==NULL)
 	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
 		g_free(t);
  		return FALSE;
 	}

 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));

	free_data_all();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Molcas");
 	numgeom =1;
 	do 
 	{
		set_status_label_info(_("Geometry"),_("Reading"));
 		OK=FALSE;
 		while(!feof(file))
		{
    			{ char* e = fgets(t,BSIZE,file);}
			if (strstr(t," Cartesian coordinates:"))
			{
    				{ char* e = fgets(t,taille,file);}
    				{ char* e = fgets(t,taille,file);}
    				{ char* e = fgets(t,taille,file);}
				numgeom++;
                		OK = TRUE;
	  			break;
	 		}
        	}
 		if(!OK && (numgeom == 1) )
		{
  			Message(_("Sorry\nI can not read geometry in this file"),_("Error"),TRUE);
 			fclose(file);
 			g_free(t);
 			for(i=0;i<5;i++) g_free(AtomCoord[i]);
			set_status_label_info(_("File name"),_("Nothing"));
			set_status_label_info(_("File type"),_("Nothing"));
			set_status_label_info(_("Geometry"),_("Nothing"));
			return FALSE;
    		}
 		if(!OK)break;

  		j=-1;
  		while(!feof(file) )
  		{
    			{ char* e = fgets(t,taille,file);}
			if ( !strcmp(t,"\n"))
			{
				/*
				long int geompos = 0;
				geompos = ftell(fd);
 				get_dipole_from_molcas_output_file(fd);
				fseek(fd, geompos, SEEK_SET);
				*/
				break;
			}
    			j++;
    			if(GeomOrb==NULL) GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    			else GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));

			sscanf(t,"%d %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1], AtomCoord[2],AtomCoord[3]);

			for(i=0;i<(gint)strlen(AtomCoord[0]);i++) if(isdigit(AtomCoord[0][i])) AtomCoord[0][i] = ' ';
			delete_all_spaces(AtomCoord[0]);

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]); if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);

    			GeomOrb[j].Symb=g_strdup(AtomCoord[0]);
    			for(i=0;i<3;i++) GeomOrb[j].C[i]=atof((AtomCoord[i+1]));
			GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
			GeomOrb[j].partialCharge = 0.0;
			GeomOrb[j].variable = TRUE;
			GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
  		}
		if(num >0 && (gint)numgeom-1 == num) break;
			
 	}while(!feof(file));

 	nCenters = j+1;
 	fclose(file);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(nCenters == 0 ) g_free(GeomOrb);
 	else
	{
  		DefineType();
  		/* PrintGeomOrb();*/
	}
	RebuildGeomD = TRUE;
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	return TRUE;
}
/********************************************************************************/
gboolean gl_read_molpro_file_geomi(gchar *FileName,gint num)
{
 	gchar *t;
 	gchar *tmp = NULL;
 	gboolean OK;
 	gchar *AtomCoord[5];
	gchar AtomCharge[100];
 	FILE *fd;
 	guint taille=BSIZE;
 	guint idummy;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numgeom;
	gchar dum[100];
	gint kk;

 	for(i=0;i<5;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(char));
  
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No file selected"),_("Error"),TRUE);
 		for(i=0;i<5;i++)
			g_free(AtomCoord[i]);
    		return FALSE ;
 	}

 	t=g_malloc(taille);
 	fd = FOpen(FileName, "rb");
 	if(fd ==NULL)
 	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
 		g_free(t);
 		for(i=0;i<5;i++)
			g_free(AtomCoord[i]);
  		return FALSE;
 	}

	free_data_all();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Molpro");
 	numgeom =1;
 	do 
 	{
		set_status_label_info(_("Geometry"),_("Reading"));
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
			if ( !strcmp(t," ATOMIC COORDINATES\n"))
			{
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}
 				numgeom++;
                		OK = TRUE;
	  			break;
	  		}
        	}
 		if(!OK && (numgeom == 1) )
		{
  			Message(_("Sorry\nI can not read geometry in this file"),_("Error"),TRUE);
 			fclose(fd);
 			g_free(t);
 			for(i=0;i<5;i++)
				g_free(AtomCoord[i]);
			set_status_label_info(_("File name"),_("Nothing"));
			set_status_label_info(_("File type"),_("Nothing"));
			set_status_label_info(_("Geometry"),_("Nothing"));
			return FALSE;
    	}
 		if(!OK)break;

  		j=-1;
  		while(!feof(fd) )
  		{
    			{ char* e = fgets(t,taille,fd);}
			if ( !strcmp(t,"\n"))
			{
				long int geompos = 0;
				geompos = ftell(fd);
 				get_dipole_from_molpro_output_file(fd);
				fseek(fd, geompos, SEEK_SET);
				break;
			}
    			j++;
    			if(GeomOrb==NULL) GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    			else GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));

			/*
			kk = sscanf(t,"%d %s %s %s %s %s %s",&idummy, AtomCoord[0],AtomCoord[1],AtomCoord[1], AtomCoord[2],AtomCoord[3], dum);
			if(kk==7) sscanf(t,"%d %s %s %s %s %s %s",&idummy, AtomCoord[0],AtomCoord[1],dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);
			*/
			kk = sscanf(t,"%d %s %s %s %s %s %s",&idummy, AtomCoord[0],AtomCharge,AtomCoord[1], AtomCoord[2],AtomCoord[3], dum);
			if(kk==7) sscanf(t,"%d %s %s %s %s %s %s",&idummy, AtomCoord[0],AtomCharge,dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);

			for(i=0;i<(gint)strlen(AtomCoord[0]);i++) if(isdigit(AtomCoord[0][i])) AtomCoord[0][i] = ' ';
			delete_all_spaces(AtomCoord[0]);
			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]);
          		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			if (atoi(AtomCharge) == 0)
				sprintf(AtomCoord[0],"X");

    			GeomOrb[j].Symb=g_strdup(AtomCoord[0]);
    			for(i=0;i<3;i++) GeomOrb[j].C[i]=atof((AtomCoord[i+1]));

			GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
			GeomOrb[j].partialCharge = 0.0;
			GeomOrb[j].variable = TRUE;
			GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
  		}
		if(num >0 && (gint)numgeom-1 == num) break;
			
 	}while(!feof(fd));

 	nCenters = j+1;
 	fclose(fd);
 	g_free(t);
 	for(i=0;i<5;i++)
		g_free(AtomCoord[i]);
 	if(nCenters == 0 )
  		g_free(GeomOrb);
 	else
	{
  		DefineType();
  		/* PrintGeomOrb();*/
	}
	RebuildGeomD = TRUE;
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	return TRUE;
}

/********************************************************************************/
gboolean gl_read_gaussn_file_geomi_str(gchar *FileName,gint num,gchar* str)
{
 	gchar *t;
 	gchar *tmp = NULL;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	FILE *fd;
 	guint taille=BSIZE;
 	guint idummy;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numgeom;
 	gchar *pdest;
 	guint itype=0;
	long int geompos = 0;

 	for(i=0;i<5;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(char));
  
 	fd = FOpen(FileName, "rb");

	free_data_all();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Gaussian");

	t=g_malloc(taille);

	numgeom =1;
 	do 
 	{
		set_status_label_info(_("Geometry"),_("Reading"));
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
	 		if (strstr( t,str))
	  		{
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}

				if(strstr( t, "Type" ))
					itype=1;
				else
					itype=0;
    				{ char* e = fgets(t,taille,fd);}
                		numgeom++;
				OK = TRUE;
				break;
	  		}
		}
 		if(!OK && (numgeom == 1) )
		{
 			fclose(fd);
 			g_free(t);
 			for(i=0;i<5;i++)
				g_free(AtomCoord[i]);
			set_status_label_info(_("File name"),_("Nothing"));
			set_status_label_info(_("File type"),_("Nothing"));
			set_status_label_info(_("Geometry"),_("Nothing"));
			return FALSE;
		}
 		if(!OK)break;

  		j=-1;
  		while(!feof(fd) )
  		{
    			{ char* e = fgets(t,taille,fd);}
    			if (strstr( t, "----------------------------------" ) )
    			{
				geompos = ftell(fd);
 				get_dipole_from_gaussian_output_file(fd);
				fseek(fd, geompos, SEEK_SET);
				gl_get_charges_from_gaussian_output_file(fd,j+1);
				gl_get_natural_charges_from_gaussian_output_file(fd,j+1);
				fseek(fd, geompos, SEEK_SET);
				gl_get_esp_charges_from_gaussian_output_file(fd,j+1);
				fseek(fd, geompos, SEEK_SET);
      				break;
    			}
    			j++;
    			if(GeomOrb==NULL)
				GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    			else
    				GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));

    			if(itype==0)
    				sscanf(t,"%d %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
    			else
    				sscanf(t,"%d %s %d %s %s %s",&idummy,AtomCoord[0],&idummy,AtomCoord[1],AtomCoord[2],AtomCoord[3]);

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]);
          		if (l==2)
	 			AtomCoord[0][1]=tolower(AtomCoord[0][1]);

    			GeomOrb[j].Symb=g_strdup(symb_atom_get((guint)atoi(AtomCoord[0])));
    			for(i=0;i<3;i++)
    				GeomOrb[j].C[i]=atof(ang_to_bohr(AtomCoord[i+1]));

			GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
			GeomOrb[j].partialCharge = 0.0;
			GeomOrb[j].variable = TRUE;
			GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
			/* Debug("%s %lf %lf %lf \n",GeomOrb[j].Symb,GeomOrb[j].C[0],GeomOrb[j].C[1],GeomOrb[j].C[2]);*/
  		}
		if(num >0 && (gint)numgeom-1 == num)
			break;
			
 	}while(!feof(fd));

 	nCenters = j+1;
 	fclose(fd);
 	g_free(t);
 	for(i=0;i<5;i++)
		g_free(AtomCoord[i]);
 	if(nCenters == 0 )
  		g_free(GeomOrb);
 	else
	{
  		DefineType();
  		/* PrintGeomOrb();*/
	}
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
	return TRUE;
}
/********************************************************************************/
gboolean gl_read_gaussn_file_geomi(gchar *FileName,gint num)
{



	FILE* fd;
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No file selected"),_("Error"),TRUE);
    		return FALSE;
 	}
 	fd = FOpen(FileName, "rb");
 	if(fd ==NULL)
 	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  		return FALSE;
 	}
	fclose(fd);


	if(gl_read_gaussn_file_geomi_str(FileName,num,"Standard orientation:"))
		return TRUE;
	if(gl_read_gaussn_file_geomi_str(FileName,num,"Input orientation:"))
		return TRUE;
	/* for calculation with nosym option */
	if(!gl_read_gaussn_file_geomi_str(FileName,num,"Z-Matrix orientation:"))
	{
  		Message(_("Sorry\nI can not read geometry in this file"),_("Error"),TRUE);
		return FALSE;
	}
	return TRUE;

}
/********************************************************************************/
gboolean gl_read_fchk_gaussn_file_geom(gchar *fileName)
{
 	FILE *file;
	gint i,j;
	gint n;
	gchar* tmp = NULL;
	gdouble* coords = NULL;
	gdouble* charges = NULL;
	gdouble* dipole = NULL;
	gint* z = NULL;
	gdouble* zn = NULL;

	file = FOpen(fileName, "rb");
	if(file ==NULL)
	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  		return FALSE;
	}

	j = get_one_int_from_fchk_gaussian_file(file,"Number of atoms ");
	if(j<1)
	{
  		Message(_("Sorry\nI can not the number of atoms from this file"),_("Error"),TRUE);
  		return FALSE;
	}
	z = get_array_int_from_fchk_gaussian_file(file, "Atomic numbers ", &n);
	if(n!=j)
	{
  		Message(_("Sorry\nI can not read the atomic numbers from this file"),_("Error"),TRUE);
  		return FALSE;
	}
	coords = get_array_real_from_fchk_gaussian_file(file, "Current cartesian coordinates  ", &n);
	if(n!=3*j)
	{
  		Message(_("Sorry\nI can not read the current cartesian coordinates from this file"),_("Error"),TRUE);
  		return FALSE;
	}
	free_data_all();
	tmp = get_name_file(fileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Gaussian fchk");
	set_status_label_info(_("Geometry"),_("Reading"));
	rewind(file);
	nCenters = j;

    	GeomOrb=g_malloc(nCenters*sizeof(TypeGeomOrb));
	for(j=0;j<nCenters;j++)
	{
    		GeomOrb[j].Symb=g_strdup(symb_atom_get(z[j]));
		for(i=0;i<3;i++) GeomOrb[j].C[i]=coords[j*3+i];
		GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb[j].partialCharge = 0.0;
		GeomOrb[j].variable = TRUE;
		GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
	}
	if(z) g_free(z);
	if(coords) g_free(coords);
	z = NULL;
	coords = NULL;
	rewind(file);
	charges = get_array_real_from_fchk_gaussian_file(file, "NPA Charges ", &n);
	if(n==nCenters && charges)
	{
		for(j=0;j<nCenters;j++)
    			GeomOrb[j].partialCharge = charges[j];

	}
	else
	{
		rewind(file);
		charges = get_array_real_from_fchk_gaussian_file(file, "ESP Charges  ", &n);
		if(n==nCenters && charges) 
		{
			for(j=0;j<nCenters;j++)
    				GeomOrb[j].partialCharge = charges[j];
		}
		else
		{
			rewind(file);
			charges = get_array_real_from_fchk_gaussian_file(file, "Mulliken Charges  ", &n);
			if(n==nCenters && charges) 
			{
				for(j=0;j<nCenters;j++)
    					GeomOrb[j].partialCharge = charges[j];
			}
			if(charges) g_free(charges);
		}
	}
	rewind(file);
	zn = get_array_real_from_fchk_gaussian_file(file, "Nuclear charges ", &n);
	if(zn && n== j)
	{
		for(j=0;j<nCenters;j++)
    			GeomOrb[j].nuclearCharge = zn[j];
	}
	if(zn)g_free(zn);
	if(n!=j)
	{
  		Message(_("Sorry\nI can not read the atomic numbers from this file"),_("Error"),TRUE);
  		return FALSE;
	}
	dipole = get_array_real_from_fchk_gaussian_file(file, "Dipole Moment  ", &n);
	init_dipole();
	if(n==3)
	{
		for(i=0;i<3;i++) Dipole.value[i] = dipole[i] / AUTODEB;
		Dipole.def = TRUE;

	}
	if(dipole) g_free(dipole);
 	fclose(file);
 	if(nCenters == 0 ) g_free(GeomOrb);
 	else DefineType();
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
	return TRUE;
}
/********************************************************************************/
void gl_get_esp_charges_from_mopac_output_file(FILE* fd)
{
 	guint taille=BSIZE;
  	gchar *t = g_malloc(BSIZE*sizeof(gchar));
  	gchar* pdest;

	init_dipole();

  	while(!feof(fd) )
	{
    		pdest = NULL;
		init_dipole();
		if(!fgets(t,taille,fd)) break;
    		pdest = strstr( t, "ELECTROSTATIC POTENTIAL CHARGES");

		if(pdest)
		{
			gint j;
			gchar dum1[100];
			gchar dum2[100];
			gchar dum3[100];

			if(!fgets(t,taille,fd)) break;
			if(!fgets(t,taille,fd)) break;
			for(j=0;j<nCenters;j++)
			{
    				if(!fgets(t,taille,fd)) break;
				sscanf(t,"%s %s %s",dum1,dum2,dum3);
				GeomOrb[j].partialCharge=atof(dum3);
			}
			break;
		}
	}
	g_free(t);
}
/********************************************************************************/
void gl_get_charges_from_mopac_output_file(FILE* fd)
{
 	guint taille=BSIZE;
  	gchar *t = g_malloc(BSIZE*sizeof(gchar));
  	gchar* pdest;

	init_dipole();

  	while(!feof(fd) )
	{
    		pdest = NULL;
		init_dipole();
    		{ char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "TYPE          CHARGE");

		if(pdest)
		{
			gint j;
			gchar dum1[100];
			gchar dum2[100];
			gchar dum3[100];

			for(j=0;j<nCenters;j++)
			{
    				if(!fgets(t,taille,fd)) break;
				sscanf(t,"%s %s %s",dum1,dum2,dum3);
				GeomOrb[j].partialCharge=atof(dum3);
			}
			break;
		}
	}
	g_free(t);
}
/********************************************************************************/
gboolean gl_read_mopac_output_file_geomi(gchar *fileName, gint numgeometry)
{
	gchar *t;
	gchar *tmp;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	guint idummy;
	guint i;
	gint j=0;
	gint l;
	gint numgeom;
	gchar *pdest;
	long int geomposok = 0;
	gchar tmp1[100];
	gchar tmp2[100];
	/* gboolean OK = FALSE;*/

	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(gchar));
	 
	t=g_malloc(taille*sizeof(gchar));
	fd = FOpen(fileName, "rb");
	if(fd ==NULL)
	{
	 	g_free(t);
	 	t = g_strdup_printf(_("Sorry\nI can not open %s  file "),fileName);
	 	Message(t,_("Error"),TRUE);
	 	g_free(t);
	 	return FALSE;
	}
	free_data_all();
	tmp = get_name_file(fileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Mopac");
	set_status_label_info(_("Geometry"),_("Reading"));
	numgeom =0;
	/* OK=FALSE;*/
	 while(!feof(fd))
	 {
		if(!fgets(t,taille,fd))break;
		pdest = strstr( t, "ATOM   CHEMICAL          X               Y               Z");
		if(pdest) 
		{
			if(!fgets(t,taille,fd)) {pdest=0;break;}
			if(!fgets(t,taille,fd)) {pdest=0;break;}
		}
		if ( pdest )
		{
			numgeom++;
			geomposok = ftell(fd);
			if(numgeom == numgeometry )
			{
				/* OK = TRUE;*/
				break;
			}
			/*
			if(numgeometry<0)
			{
				OK = TRUE;
			}
			*/
		}
	 }
	 if(numgeom == 0)
	 {
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not read geometry in %s file "),fileName);
		Message(t,_("Error"),TRUE);
		g_free(t);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Geometry"),_("Nothing"));
		return FALSE;
	  }
	init_dipole();
	for(i=0;i<3;i++) Dipole.value[i] = 0;
	j=-1;
	fseek(fd, geomposok, SEEK_SET);
	while(!feof(fd) )
	{
		gint ii;
		if(!fgets(t,taille,fd))break;
		if(0==sscanf(t,"%d",&ii))break;
		if(this_is_a_backspace(t))
		{
			if(numgeometry<0)
			{
				nCenters = j+1;
				fseek(fd, 0, SEEK_SET);
				gl_get_charges_from_mopac_output_file(fd);
				fseek(fd, 0, SEEK_SET);
				gl_get_esp_charges_from_mopac_output_file(fd);
				fseek(fd, 0, SEEK_SET);
 				get_dipole_from_mopac_output_file(fd);
			}
			break;
		}
		j++;
    		if(GeomOrb==NULL) GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    		else GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));
		for(ii=0;ii<strlen(t);ii++) if(t[ii]=='*') t[ii]=' ';
		sscanf(t,"%d %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
		AtomCoord[0][0]=toupper(AtomCoord[0][0]);
		l=strlen(AtomCoord[0]); 
		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
		if(l==1)sprintf(t,"%c",AtomCoord[0][0]);
		else sprintf(t,"%c%c",AtomCoord[0][0],AtomCoord[0][1]);


		GeomOrb[j].Symb=g_strdup(AtomCoord[0]);
    		for(i=0;i<3;i++) 
		{
			GeomOrb[j].C[i]=atof((AtomCoord[i+1]))*ANG_TO_BOHR;
		}

		GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb[j].partialCharge = 0.0;
		GeomOrb[j].variable = TRUE;
		GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
	  }
	fclose(fd);
	nCenters = j+1;
	if(nCenters >0)
  		DefineType();

	RebuildGeomD = TRUE;
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	g_free(t);
	for(i=0;i<5;i++) g_free(AtomCoord[i]);
	return TRUE;
}
/********************************************************************************/
gboolean gl_read_mopac_aux_file_geomi(gchar *fileName, gint numgeometry)
{
	gchar *t;
	gchar *tmp;
	gboolean OK;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	guint i;
	gint j=0;
	gint l;
	gint numgeom;
	gchar *pdest;
	long int geomposok = 0;
	gchar** elements = NULL;
	gint nElements = 0;
	gchar** nuclearCharges = NULL;
	gint nNuclearCharges = 0;
	gchar** partialCharges = NULL;
	gint nPartialCharges = 0;

	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(gchar));
	 
	t=g_malloc(taille*sizeof(gchar));
	fd = FOpen(fileName, "rb");
	if(fd ==NULL)
	{
	 	g_free(t);
	 	t = g_strdup_printf(_("Sorry\nI can not open %s file "),fileName);
	 	Message(t,_("Error"),TRUE);
	 	g_free(t);
	 	return FALSE;
	}
	OK=FALSE;
	elements = get_one_block_from_aux_mopac_file(fd, "ATOM_EL[",  &nElements);
	if(elements) OK = TRUE;
	if(!OK) 
	{
	 	g_free(t);
	 	t = g_strdup_printf(_("Sorry\nI can not read the atom symbols in %s file "),fileName);
	 	Message(t,_("Error"),TRUE);
	 	g_free(t);
		fclose(fd);
	 	return FALSE;
	 }
 	geomposok = ftell(fd);
	nuclearCharges = get_one_block_from_aux_mopac_file(fd, "ATOM_CORE[",  &nNuclearCharges);
	if(!nuclearCharges) fseek(fd, geomposok, SEEK_SET);

	numgeom =0;
	free_data_all();
	tmp = get_name_file(fileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Mopac");
	set_status_label_info(_("Geometry"),_("Reading"));
	 while(!feof(fd))
	 {
		if(!fgets(t,taille,fd))break;
		if(numgeometry<0)
			pdest = strstr( t, "ATOM_X_OPT:ANGSTROMS");
		else
			pdest = strstr( t, "ATOM_X_UPDATED:ANGSTROMS");
		if ( pdest )
		{
			numgeom++;
			geomposok = ftell(fd);
			if(numgeom == numgeometry )
			{
				OK = TRUE;
				break;
			}
			if(numgeometry<0)
			{
				OK = TRUE;
			}
		}
	 }
	 if(numgeom == 0)
	 {
		free_one_string_table(elements, nElements);
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not read geometry in %s file "),fileName);
		Message(t,_("Error"),TRUE);
		g_free(t);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Geometry"),_("Nothing"));
		fclose(fd);
		return FALSE;
	  }

	init_dipole();
	for(i=0;i<3;i++) Dipole.value[i] = 0;
	j=-1;
	fseek(fd, geomposok, SEEK_SET);
	while(!feof(fd) )
	{
		if(!fgets(t,taille,fd))break;
		if(strstr( t, "[")
		  || strstr(t,"HEAT_OF_FORM_UPDATED")
		  || strstr( t, "####################################")
		  ||this_is_a_backspace(t))
		{
			break;
		}
		if(j+1>nElements)break;
		j++;
    		if(GeomOrb==NULL) GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    		else GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));

		sscanf(t,"%s %s %s",AtomCoord[1],AtomCoord[2],AtomCoord[3]);
		if(j<nElements) sprintf(AtomCoord[0],"%s",elements[j]);
		else sprintf(AtomCoord[0],"X");
		AtomCoord[0][0]=toupper(AtomCoord[0][0]);
		l=strlen(AtomCoord[0]); 
		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
		if(l==1)sprintf(t,"%c",AtomCoord[0][0]);
		else sprintf(t,"%c%c",AtomCoord[0][0],AtomCoord[0][1]);

		GeomOrb[j].Symb=g_strdup(AtomCoord[0]);
    		for(i=0;i<3;i++) 
		{
			GeomOrb[j].C[i]=atof((AtomCoord[i+1]))*ANG_TO_BOHR;
		}

		GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb[j].partialCharge = 0.0;
		if(nuclearCharges && nNuclearCharges>j) GeomOrb[j].nuclearCharge = atof(nuclearCharges[j]);
		else GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
	  }
	nCenters = j+1;
	if(nCenters >0)
  		DefineType();

	if(numgeometry<0)
	{
		fseek(fd, geomposok, SEEK_SET);
		partialCharges = get_one_block_from_aux_mopac_file(fd, "ATOM_CHARGES[",  &nPartialCharges);
		if(partialCharges)
		{
    			for(j=0;j<nCenters;j++) 
				if(j<nPartialCharges) GeomOrb[j].partialCharge=atof(partialCharges[j]);
			free_one_string_table(partialCharges, nPartialCharges);
		}
	}
	fclose(fd);

	g_free(t);
	free_one_string_table(elements, nElements);
	free_one_string_table(nuclearCharges, nNuclearCharges);
	for(i=0;i<5;i++) g_free(AtomCoord[i]);
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
	return TRUE;
}
/********************************************************************************/
gboolean gl_read_wfx_file_geom(gchar *fileName)
{
	gint k;
	gint j;
	gchar t[BSIZE];
	gchar** symbols = NULL;
	gdouble* coordinates = NULL;
	gint nSymbols,nCoordinates;
	FILE* file = FOpen(fileName, "rb");
	gdouble* nuclearCharges = NULL;
	gint nNuclearCharges;
	gchar* tmp = NULL;

	if(file ==NULL)
	{
		sprintf(t,"Sorry\nI can not open %s  file ",fileName);
		Message(t,_("Error"),TRUE);
		return FALSE;
	}
	symbols = get_one_block_from_wfx_file(file,"Nuclear Name",&nSymbols);
	//fprintf(stderr,"nSymb = %d\n",nSymbols);
	if(!symbols || nSymbols<1)
	{
		sprintf(t,"Sorry\nI cannot read symbol from %s  file ",fileName);
		Message(t,_("Error"),TRUE);
		fclose(file);
		if(symbols) free_one_string_table(symbols, nSymbols);
		return FALSE;
	}
	rewind(file);
	coordinates = get_one_block_real_from_wfx_file(file,"Nuclear Cartesian Coordinates",&nCoordinates);
	//fprintf(stderr,"nC = %d\n",nCoordinates);
	if(!coordinates || nCoordinates<3)
	{
		sprintf(t,"Sorry\nI cannot read coordinates from %s  file ",fileName);
		Message(t,_("Error"),TRUE);
		fclose(file);
		if(symbols) free_one_string_table(symbols, nSymbols);
		if(coordinates) g_free(coordinates);
		return FALSE;
	}
	if(nCoordinates!=3*nSymbols)
	{
		sprintf(t,"Sorry\nI cannot read geometry from %s  file",fileName);
		Message(t,_("Error"),TRUE);
		fclose(file);
		if(symbols) free_one_string_table(symbols, nSymbols);
		if(coordinates) g_free(coordinates);
		return FALSE;
	}

	rewind(file);
	nuclearCharges = get_one_block_real_from_wfx_file(file,"Nuclear Charges",&nNuclearCharges);
	fclose(file);

	free_data_all();
	tmp = get_name_file(fileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"WFX");
	set_status_label_info(_("Geometry"),_("Reading"));

	if(GeomOrb==NULL) GeomOrb=g_malloc(nSymbols*sizeof(TypeGeomOrb));
        else GeomOrb=g_realloc(GeomOrb,nSymbols*sizeof(TypeGeomOrb));

	for(j=0;j<nSymbols;j++)
	{
		gint i,l;

		for(k=0;k<(gint)strlen(symbols[j]);k++) if(isdigit(symbols[j][k])) symbols[j][k] = ' ';
		delete_all_spaces(symbols[j]);

		symbols[j][0]=toupper(symbols[j][0]);
		l=strlen(symbols[j]); 
		if (l==2) symbols[j][1]=tolower(symbols[j][1]);
		if(l==1)sprintf(t,"%c",symbols[j][0]);
		else sprintf(t,"%c%c",symbols[j][0],symbols[j][1]);
		//fprintf(stderr,"t=%s\n", t);

		GeomOrb[j].Symb=g_strdup(t);
    		for(i=0;i<3;i++) GeomOrb[j].C[i]=coordinates[3*j+i];
		//fprintf(stderr,"XYZ0 = %f %f %f\n", GeomOrb[j].C[0], GeomOrb[j].C[1], GeomOrb[j].C[2]);
		GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb[j].partialCharge = 0.0;
		GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
		if(nuclearCharges && nNuclearCharges>j) GeomOrb[j].nuclearCharge = nuclearCharges[j];
		//fprintf(stderr,"XYZ = %f %f %f\n", GeomOrb[j].C[0], GeomOrb[j].C[1], GeomOrb[j].C[2]);
	}
	nCenters = nSymbols;
	if(nCenters >0) DefineType();
	if(symbols) free_one_string_table(symbols, nSymbols);
	//fprintf(stderr,"End free symbols");
	if(coordinates) g_free(coordinates);
	//fprintf(stderr,"End free coordinates");
	if(nuclearCharges) g_free(nuclearCharges);
	//fprintf(stderr,"End free nuclearCharges");
	init_dipole();
	Dipole.value[0] = Dipole.value[1] = Dipole.value[2] = 0;

	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
	return TRUE;
}
/********************************************************************************/
void gl_get_charges_from_orca_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;


	for(i=0;i<N;i++)
		GeomOrb[i].partialCharge = 0.0;

  	while(!feof(fd) )
	{
    		pdest = NULL;
    		{ char* e = fgets(t,taille,fd);}
		if(strstr(t,"GEOMETRY OPTIMIZATION CYCLE")) break;
    		pdest = strstr( t, "MULLIKEN ATOMIC CHARGES");

		if(pdest)
		{
			gboolean OK = FALSE;
  			while(!feof(fd) )
			{
    				if(!fgets(t,taille,fd)) break;
				if(strstr(t,"----------------"))
				{
					OK = TRUE;
					break;
				}
			}
			if(!OK) break;

			for(i=0;i<N;i++)
			{
    				if(!feof(fd)) { char* e = fgets(t,taille,fd);}
				else break;
				if(sscanf(t,"%s %s %s %s",dump,dump,dump,d)==4)
				{
					GeomOrb[i].partialCharge = atof(d);
				}
			}
			break;
		}
	}
}
/********************************************************************************/
gboolean gl_read_orca_file_geomi(gchar *FileName,gint num)
{
 	gchar *t;
 	gchar *tmp = NULL;
 	/* gboolean OK;*/
 	gchar *AtomCoord[5];
 	FILE *fd;
 	guint taille=BSIZE;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numgeom;
 	gchar *pdest;
	long int geomposok = 0;
	long int geompos = 0;
	gchar dum[BSIZE];
	gdouble mass;
	gdouble nucCharge;

 	for(i=0;i<5;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(char));
  
 	fd = FOpen(FileName, "rb");

	free_data_all();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Orca");

	t=g_malloc(taille);

	numgeom =0;
	set_status_label_info(_("Geometry"),_("Reading"));
	/* OK=FALSE;*/

	while(!feof(fd))
	{
		if(!fgets(t,taille,fd))break;
		pdest = strstr( t, "CARTESIAN COORDINATES (A.U.)");
		if(pdest) 
		{
			if(!fgets(t,taille,fd))break;
			if(!fgets(t,taille,fd))break;
		}
		if (strstr( t, "FRAG") && strstr( t, "MASS") )
		{
			numgeom++;
			geomposok = ftell(fd);
			if(numgeom == num )
			{
				/* OK = TRUE;*/
				break;
			}
			/*
			if(num<0)
			{
				OK = TRUE;
			}
			*/
		}
	}
	if(numgeom == 0)
	{
 		fclose(fd);
 		g_free(t);
 		for(i=0;i<5;i++) g_free(AtomCoord[i]);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Geometry"),_("Nothing"));
		return FALSE;
	}
	j=-1;
	fseek(fd, geomposok, SEEK_SET);
 	while(!feof(fd) )
  	{
    		{ char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "----------------------------------" );
		if(!pdest) pdest = strstr( t, "due to ECP");
    		if (pdest || this_is_a_backspace(t))
    		{
			geompos = ftell(fd);
 			get_dipole_from_orca_output_file(fd);
			fseek(fd, geompos, SEEK_SET);
			gl_get_charges_from_orca_output_file(fd,j+1);
			fseek(fd, geompos, SEEK_SET);
      			break;
    		}
    		j++;
    		if(GeomOrb==NULL) GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    		else GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));

		//printf("t=%s\n",t);
   		//sscanf(t,"%s %s %lf %s %lf %s %s %s",dum,AtomCoord[0],&nucCharge,dum,&mass,AtomCoord[1],AtomCoord[2],AtomCoord[3]);
   		sscanf(t,"%s %s %s %s %lf %s %s %s",dum,AtomCoord[0],AtomCoord[4],dum,&mass,AtomCoord[1],AtomCoord[2],AtomCoord[3]);
		nucCharge = atof(AtomCoord[4]);

		AtomCoord[0][0]=toupper(AtomCoord[0][0]);
 		l=strlen(AtomCoord[0]);
       		if (l==2) 
		{
			AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			if(isdigit(AtomCoord[0][1]))l=1;
		}
		if(l==1)sprintf(t,"%c",AtomCoord[0][0]);
	         else sprintf(t,"%c%c",AtomCoord[0][0],AtomCoord[0][1]);
		/* if(!strcmp(t,"-")) sprintf(t,"X");*/
		if(!strcmp(t,"-")) { j--; continue;}

		GeomOrb[j].Symb=g_strdup(t);
   		for(i=0;i<3;i++) GeomOrb[j].C[i]=atof(AtomCoord[i+1]);

		GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb[j].Prop.masse = mass;
		GeomOrb[j].partialCharge = 0.0;
		GeomOrb[j].variable = TRUE;
		GeomOrb[j].nuclearCharge = nucCharge;
  	}

 	nCenters = j+1;
 	fclose(fd);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(nCenters == 0 ) g_free(GeomOrb);
 	else
	{
  		DefineType();
	}
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
	return TRUE;
}
/********************************************************************************/
gboolean gl_read_orca_file_hessian(gchar *FileName)
{
 	gchar *t;
 	gchar *tmp = NULL;
 	/* gboolean OK;*/
 	gchar *AtomCoord[5];
 	FILE *fd;
 	guint taille=BSIZE;
 	guint i;
 	gint j=0;
 	gint l;
 	/* guint numgeom;*/
 	gchar *pdest;
	long int geomposok = 0;
	long int geompos = 0;
	gchar dum[BSIZE];
	gdouble mass;
	gdouble nucCharge;
	gint nAtoms = 0;

 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(gchar));
  
 	fd = FOpen(FileName, "rb");

	free_data_all();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Orca");

	t=g_malloc(taille*sizeof(gchar));

	/* numgeom =0;*/
	set_status_label_info(_("Geometry"),_("Reading"));
	/* OK=FALSE;*/

	while(!feof(fd))
	{
		if(!fgets(t,taille,fd))break;
		if(strstr( t, "$atoms"))
		{
			if(!fgets(t,taille,fd))break;
			sscanf(t,"%d",&nAtoms);
			break;
		}
	}
	if(nAtoms == 0)
	{
 		fclose(fd);
 		g_free(t);
 		for(i=0;i<5;i++) g_free(AtomCoord[i]);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Geometry"),_("Nothing"));
		return FALSE;
	}
	/* printf("nAtoms = %d\n",nAtoms);*/
    	GeomOrb=g_malloc(nAtoms*sizeof(TypeGeomOrb));
 	for(j=0;j<nAtoms;j++)
  	{
    		{ char* e = fgets(t,taille,fd);}
   		sscanf(t,"%s %lf %s %s %s",AtomCoord[0],&mass,AtomCoord[1],AtomCoord[2],AtomCoord[3]);
		/* printf("t=%s\n",t);*/
		AtomCoord[0][0]=toupper(AtomCoord[0][0]);
 		l=strlen(AtomCoord[0]);
       		if (l==2) 
		{
			AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			if(isdigit(AtomCoord[0][1]))l=1;
		}
		if(l==1)sprintf(t,"%c",AtomCoord[0][0]);
	        else sprintf(t,"%c%c",AtomCoord[0][0],AtomCoord[0][1]);

		GeomOrb[j].Symb=g_strdup(t);
   		for(i=0;i<3;i++) GeomOrb[j].C[i]=atof(AtomCoord[i+1]);

		GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb[j].Prop.masse = mass;
		GeomOrb[j].partialCharge = 0.0;
		GeomOrb[j].variable = TRUE;
		GeomOrb[j].nuclearCharge = 1.0;
  	}
	fseek(fd, 0, SEEK_SET);
 	get_dipole_from_orca_output_file(fd);
	fseek(fd, geompos, SEEK_SET);
	gl_get_charges_from_orca_output_file(fd,nAtoms);
	fseek(fd, geompos, SEEK_SET);
 	fclose(fd);
 	g_free(t);
	nCenters = nAtoms;
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(nCenters == 0 ) g_free(GeomOrb);
 	else
	{
  		DefineType();
	}
	buildBondsOrb();
	reset_grid_limits();
	//printf("end reset_grid_limits\n");
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	//printf("end free_objects_all\n");
	glarea_rafresh(GLArea);
	return TRUE;
}
/********************************************************************************/
void gl_get_charges_from_qchem_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;
	gint ngrad=0;


  	while(!feof(fd) )
	{
    		pdest = NULL;
    		{ char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "Mulliken Net Atomic Charges");

		if(pdest)
		{
			gboolean OK = FALSE;
  			while(!feof(fd) )
			{
    				if(!fgets(t,taille,fd)) break;
				if(strstr(t,"----------------"))
				{
					OK = TRUE;
					break;
				}
			}
			if(!OK) break;

			for(i=0;i<N;i++)
			{
    				if(!feof(fd)) { char* e = fgets(t,taille,fd);}
				else break;
				if(sscanf(t,"%s %s %s",dump,dump,d)==3)
				{
					GeomOrb[i].partialCharge = atof(d);
				}
			}
			break;
		}
		else
		{
          		pdest = strstr( t, "GradGradGrad" );
			if(pdest)
			{
				ngrad++;
			}
			if(ngrad>2)
				break;
		}

	}
}
/********************************************************************************/
gboolean gl_read_qchem_file_geomi(gchar *FileName,gint num)
{
 	gchar *t;
 	gchar *tmp = NULL;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	FILE *fd;
 	guint taille=BSIZE;
 	guint idummy;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numgeom;
 	gchar *pdest;
	long geompos = 0;

 	for(i=0;i<5;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(char));
  
 	fd = FOpen(FileName, "rb");

	free_data_all();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Q-Chem");

	t=g_malloc(taille);

	numgeom =1;
 	do 
 	{
		set_status_label_info(_("Geometry"),_("Reading"));
 		OK=FALSE;
 		while(!feof(fd))
		{
	  		if(!fgets(t,taille,fd))break;
			pdest = NULL;
	 		if (strstr( t,"Atom") && strstr( t,"X") && strstr( t,"Y") && strstr( t,"Z") ) 
			{
	  			if(!fgets(t,taille,fd))break;
				pdest = strstr( t,"----------------------------------");
			}
	 		if (pdest)
	  		{
                		numgeom++;
				OK = TRUE;
				break;
	  		}
		}
 		if(!OK && (numgeom == 1) )
		{
 			fclose(fd);
 			g_free(t);
 			for(i=0;i<5;i++) g_free(AtomCoord[i]);
			set_status_label_info(_("File name"),_("Nothing"));
			set_status_label_info(_("File type"),_("Nothing"));
			set_status_label_info(_("Geometry"),_("Nothing"));
			return FALSE;
		}
 		if(!OK)break;

  		j=-1;
  		while(!feof(fd) )
  		{
    			{ char* e = fgets(t,taille,fd);}
    			pdest = strstr( t, "----------------------------------" );
    			if (pdest)
    			{
				geompos = ftell(fd);
 				get_dipole_from_qchem_output_file(fd);
				fseek(fd, geompos, SEEK_SET);
				gl_get_charges_from_qchem_output_file(fd,j+1);
				fseek(fd, geompos, SEEK_SET);
      				break;
    			}
    			j++;
    			if(GeomOrb==NULL) GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    			else GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));

    			sscanf(t,"%d %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]);
          		if (l==2) 
			{
				AtomCoord[0][1]=tolower(AtomCoord[0][1]);
				if(isdigit(AtomCoord[0][1]))l=1;
			}
			if(l==1)sprintf(t,"%c",AtomCoord[0][0]);
		         else sprintf(t,"%c%c",AtomCoord[0][0],AtomCoord[0][1]);

    			GeomOrb[j].Symb=g_strdup(t);
    			for(i=0;i<3;i++) GeomOrb[j].C[i]=atof(ang_to_bohr(AtomCoord[i+1]));

			GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
			GeomOrb[j].partialCharge = 0.0;
			GeomOrb[j].variable = TRUE;
			GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
  		}
		if(num >0 && (gint)numgeom-1 == num) break;
 	}while(!feof(fd));

 	nCenters = j+1;
 	fclose(fd);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(nCenters == 0 ) g_free(GeomOrb);
 	else
	{
  		DefineType();
	}
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
	return TRUE;
}
/********************************************************************************/
void gl_get_charges_from_nwchem_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar d3[BSIZE];
  	gchar d4[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;


	for(i=0;i<N;i++)
		GeomOrb[i].partialCharge = 0.0;

  	while(!feof(fd) )
	{
    		pdest = NULL;
    		{ char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "Atom       Charge   Shell Charges");

		if(pdest)
		{
			gboolean OK = FALSE;
  			while(!feof(fd) )
			{
    				if(!fgets(t,taille,fd)) break;
				if(strstr(t,"----------------"))
				{
					OK = TRUE;
					break;
				}
			}
			if(!OK) break;

			for(i=0;i<N;i++)
			{
    				if(!feof(fd)) { char* e = fgets(t,taille,fd);}
				else break;
				if(sscanf(t,"%s %s %s %s",d,d,d3,d4)==4)
				{
					GeomOrb[i].partialCharge = atof(d3)-atof(d4);
				}
			}
			break;
		}
	}
}
/********************************************************************************/
gulong gl_read_nwchem_file_geomi(gchar *FileName,gint num)
{
 	gchar *t;
 	gchar *tmp = NULL;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	FILE *fd;
 	guint taille=BSIZE;
 	guint idummy;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numgeom;
 	gchar *pdest;
	long geompos = 0;
	gboolean ang = FALSE;
	gulong line = 0;
	gulong lineg = 0;

 	for(i=0;i<5;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(char));
  
 	fd = FOpen(FileName, "rb");

	free_data_all();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"NWChem");

	t=g_malloc(taille);

	numgeom =1;
 	do 
 	{
		set_status_label_info(_("Geometry"),_("Reading"));
 		OK=FALSE;
 		while(!feof(fd))
		{
			line++;
	  		if(!fgets(t,taille,fd))break;
			if(strstr(t,"Output coordinates in angstroms")) ang = TRUE;
			if(strstr(t,"Output coordinates in a.u.")) ang = FALSE;
			pdest = strstr( t, "Charge          X              Y              Z");
			if(pdest) 
			{
				line++;
				if(!fgets(t,taille,fd))break;
				pdest = strstr( t, "--------------");
			}
	 		if (pdest)
	  		{
				lineg = line;
                		numgeom++;
				OK = TRUE;
				break;
	  		}
		}
 		if(!OK && (numgeom == 1) )
		{
 			fclose(fd);
 			g_free(t);
 			for(i=0;i<5;i++) g_free(AtomCoord[i]);
			set_status_label_info(_("File name"),_("Nothing"));
			set_status_label_info(_("File type"),_("Nothing"));
			set_status_label_info(_("Geometry"),_("Nothing"));
			return 0;
		}
 		if(!OK)break;

  		j=-1;
  		while(!feof(fd) )
  		{
			line++;
    			{ char* e = fgets(t,taille,fd);}
			if(this_is_a_backspace(t))
    			{
				geompos = ftell(fd);
 				get_dipole_from_nwchem_output_file(fd);
				fseek(fd, geompos, SEEK_SET);
				gl_get_charges_from_nwchem_output_file(fd,j+1);
				fseek(fd, geompos, SEEK_SET);
      				break;
    			}
    			j++;
    			if(GeomOrb==NULL) GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    			else GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));

			sscanf(t,"%d %s %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[4],AtomCoord[1],AtomCoord[2],AtomCoord[3]);

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]);
          		if (l==2) 
			{
				AtomCoord[0][1]=tolower(AtomCoord[0][1]);
				if(isdigit(AtomCoord[0][1]))l=1;
			}
			if(l==1)sprintf(t,"%c",AtomCoord[0][0]);
		         else sprintf(t,"%c%c",AtomCoord[0][0],AtomCoord[0][1]);

			/* HERE 
    			GeomOrb[j].Symb=get_symbol_using_z((int)(atof(AtomCoord[4])+0.5));
			*/
    			GeomOrb[j].Symb=g_strdup(t);
			if(ang)
    			for(i=0;i<3;i++) GeomOrb[j].C[i]=atof(ang_to_bohr(AtomCoord[i+1]));
			else
    			for(i=0;i<3;i++) GeomOrb[j].C[i]=atof(AtomCoord[i+1]);

			GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
			GeomOrb[j].partialCharge = 0.0;
			GeomOrb[j].variable = TRUE;
			GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
  		}
		if(num >0 && (gint)numgeom-1 == num) break;
 	}while(!feof(fd));

 	nCenters = j+1;
 	fclose(fd);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(nCenters == 0 ) g_free(GeomOrb);
 	else
	{
  		DefineType();
	}
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
	return lineg;
}
/********************************************************************************/
void gl_get_charges_from_psicode_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar d3[BSIZE];
  	gchar d4[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;


	for(i=0;i<N;i++)
		GeomOrb[i].partialCharge = 0.0;

  	while(!feof(fd) )
	{
    		pdest = NULL;
    		{ char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "Atom       Charge   Shell Charges");

		if(pdest)
		{
			gboolean OK = FALSE;
  			while(!feof(fd) )
			{
    				if(!fgets(t,taille,fd)) break;
				if(strstr(t,"----------------"))
				{
					OK = TRUE;
					break;
				}
			}
			if(!OK) break;

			for(i=0;i<N;i++)
			{
    				if(!feof(fd)) { char* e = fgets(t,taille,fd);}
				else break;
				if(sscanf(t,"%s %s %s %s",d,d,d3,d4)==4)
				{
					GeomOrb[i].partialCharge = atof(d3)-atof(d4);
				}
			}
			break;
		}
	}
}
/********************************************************************************/
gulong gl_read_psicode_file_geomi(gchar *FileName,gint num)
{
 	gchar *t;
 	gchar *tmp = NULL;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	FILE *fd;
 	guint taille=BSIZE;
 	guint idummy;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numgeom;
 	gchar *pdest;
	long geompos = 0;
	gboolean ang = TRUE;
	gulong line = 0;
	gulong lineg = 0;

 	for(i=0;i<5;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(char));
  
 	fd = FOpen(FileName, "rb");

	free_data_all();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Psicode");

	t=g_malloc(taille*sizeof(gchar));

	numgeom =1;
 	do 
 	{
		set_status_label_info(_("Geometry"),_("Reading"));
 		OK=FALSE;
 		while(!feof(fd))
		{
			line++;
	  		if(!fgets(t,taille,fd))break;
			pdest = strstr( t, "Geometry (in Angstrom),");
			if(pdest) 
			{
			if(!fgets(t,taille,fd))break;
			if(!fgets(t,taille,fd))break;
			pdest = strstr( t, "Center              X                  Y                   Z");
			}
			if(pdest) 
			{
				line++;
				if(!fgets(t,taille,fd))break;
				pdest = strstr( t, "--------------");
			}
	 		if (pdest)
	  		{
				lineg = line;
                		numgeom++;
				OK = TRUE;
				break;
	  		}
		}
 		if(!OK && (numgeom == 1) )
		{
 			fclose(fd);
 			g_free(t);
 			for(i=0;i<5;i++) g_free(AtomCoord[i]);
			set_status_label_info(_("File name"),_("Nothing"));
			set_status_label_info(_("File type"),_("Nothing"));
			set_status_label_info(_("Geometry"),_("Nothing"));
			return 0;
		}
 		if(!OK)break;

  		j=-1;
  		while(!feof(fd) )
  		{
			line++;
    			{ char* e = fgets(t,taille,fd);}
			if(this_is_a_backspace(t))
    			{
				geompos = ftell(fd);
 				get_dipole_from_psicode_output_file(fd);
				fseek(fd, geompos, SEEK_SET);
				gl_get_charges_from_psicode_output_file(fd,j+1);
				fseek(fd, geompos, SEEK_SET);
      				break;
    			}
    			j++;
    			if(GeomOrb==NULL) GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    			else GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));

			sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]);
          		if (l==2) 
			{
				AtomCoord[0][1]=tolower(AtomCoord[0][1]);
				if(isdigit(AtomCoord[0][1]))l=1;
			}
			if(l==1)sprintf(t,"%c",AtomCoord[0][0]);
		         else sprintf(t,"%c%c",AtomCoord[0][0],AtomCoord[0][1]);

    			GeomOrb[j].Symb=g_strdup(t);
			if(ang)
    			for(i=0;i<3;i++) GeomOrb[j].C[i]=atof(ang_to_bohr(AtomCoord[i+1]));
			else
    			for(i=0;i<3;i++) GeomOrb[j].C[i]=atof(AtomCoord[i+1]);

			GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
			GeomOrb[j].partialCharge = 0.0;
			GeomOrb[j].variable = TRUE;
			GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
  		}
		if(num >0 && (gint)numgeom-1 == num) break;
 	}while(!feof(fd));

 	nCenters = j+1;
 	fclose(fd);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(nCenters == 0 ) g_free(GeomOrb);
 	else
	{
  		DefineType();
	}
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
	return lineg;
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
static gboolean gl_read_geom_nbo_file(gchar *fileName)
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
	free_data_all();
  	init_dipole();
    	GeomOrb=g_malloc(nAtoms*sizeof(TypeGeomOrb));
	uni = 1;

	tmp = get_name_file(fileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"NBO");
	set_status_label_info(_("Geometry"),_("Reading"));

	gint ix = 0;
	j = 0;
    	for(k=0;k<nAtoms;k++) 
	{
		if(!fgets(t,BSIZE,file)) break;
		sscanf(t,"%d",&z);
		if(z<=0)
		{
			ix++;
			continue;
		}
		sscanf(t,"%d %lf %lf %lf",&z, &GeomOrb[j].C[0], &GeomOrb[j].C[1], &GeomOrb[j].C[2]);
    		GeomOrb[j].Symb=get_symbol_using_z(z);
		if(uni==1) for(i=0;i<3;i++) GeomOrb[j].C[i] *= ANG_TO_BOHR;
		GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb[j].partialCharge = 0;
		GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
		GeomOrb[j].variable = TRUE;
		j++;
		
	}
	nAtoms -= ix;
	nCenters = 0;
	if(k==nAtoms+ix) nCenters = nAtoms;
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
 		Ntype =nCenters;
		for(i=0;i<nCenters;i++) 
		{
			GeomOrb[i].NumType = i;
			/*
     			Type[i].Symb=g_strdup(GeomOrb[i].Symb);
     			Type[i].N=GetNelectrons(GeomOrb[i].Symb);
			*/
		}
		buildBondsOrb();
		RebuildGeomD = TRUE;
		reset_grid_limits();
		init_atomic_orbitals();
		return TRUE;
	}
	return TRUE;
}
/********************************************************************************/
static void gl_get_charges_from_aimall_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;


  	while(!feof(fd) )
	{
    		pdest = NULL;
    		if(!fgets(t,taille,fd)) break;
    		pdest = strstr( t, "Atom A          q(A)");
		if(pdest)
		{
    			if(!fgets(t,taille,fd)) break;
			for(i=0;i<N;i++)
			{
    				if(!fgets(t,taille,fd)) break;
				if(sscanf(t,"%s %s",dump, d)==2)
				{
					GeomOrb[i].partialCharge = atof(d);
				}
				else break;
			}
			break;
		}
	}
}
/********************************************************************************/
static gboolean gl_read_aimall_file(gchar *fileName)
{
	gchar *t;
	/* gboolean OK;*/
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	guint i;
	gint j=0;
	gint l;
	gchar dum[100];
	gint uni=0;

	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));
  
	t=g_malloc(taille);
	fd = FOpen(fileName, "rb");
	if(fd ==NULL)
	{
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not open %s  file "),fileName);
		Message(t,_("Error"),TRUE);
		g_free(t);
		return FALSE;
	}
	/* OK=FALSE;*/
	while(!feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
		if ( strstr(t,"Atom      Charge                X                  Y                  Z"))
		{
    			{ char* e = fgets(t,taille,fd);}
			/* OK = TRUE;*/
			break; 
  		}
	}

	j=-1;
	while(!feof(fd) )
	{
    		{ char* e = fgets(t,taille,fd);}
		if(this_is_a_backspace(t))
		{
			gl_get_charges_from_aimall_file(fd,j+1);
			break;
		}
		j++;

   		if(GeomOrb==NULL) GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    		else GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));

		sscanf(t,"%s %s %s %s %s",AtomCoord[0],dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);
		{
			gint k;
			for(k=0;k<(gint)strlen(AtomCoord[0]);k++) if(isdigit(AtomCoord[0][k])) AtomCoord[0][k] = ' ';
			delete_all_spaces(AtomCoord[0]);
		}
		AtomCoord[0][0]=toupper(AtomCoord[0][0]);
		l=strlen(AtomCoord[0]);
		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
		GeomOrb[j].Symb=get_symbol_using_z(atoi(dum));
    		for(i=0;i<3;i++) 
		{
			GeomOrb[j].C[i]=atof((AtomCoord[i+1]));
			if(uni==1)GeomOrb[j].C[i] *= ANG_TO_BOHR;
		}

		GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb[j].partialCharge = 0.0;
		GeomOrb[j].variable = TRUE;
		GeomOrb[j].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[j].Symb);
	}


 	nCenters = j+1;

	if ( nCenters >0)
	{
		gl_get_charges_from_aimall_file(fd,nCenters);
		/* PrintGeomOrb();*/
	}

 	fclose(fd);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(nCenters == 0 ) g_free(GeomOrb);
 	else
	{
  		DefineType();
  		/* PrintGeomOrb();*/
	}
	RebuildGeomD = TRUE;
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	return TRUE;
}
/********************************************************************************/
void gl_read_aimall_file_sel(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();
 
	add_objects_for_new_grid();
 	gl_read_aimall_file(FileName);
}
/********************************************************/
void gl_read_first_dalton_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
 	gl_read_dalton_file_geomi(FileName,1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
} 
/********************************************************/
void gl_read_last_dalton_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();
	add_objects_for_new_grid();
 	gl_read_dalton_file_geomi(FileName,-1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
}
/********************************************************/
void gl_read_first_gamess_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
 	gl_read_gamess_file_geomi(FileName,1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
} 
/********************************************************/
void gl_read_last_gamess_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();
	add_objects_for_new_grid();
 	gl_read_gamess_file_geomi(FileName,-1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
}
/********************************************************/
void gl_read_first_gauss_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
 	gl_read_gaussn_file_geomi(FileName,1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
} 
/********************************************************/
void gl_read_last_gauss_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();
	add_objects_for_new_grid();
 	gl_read_gaussn_file_geomi(FileName,-1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
}
/********************************************************/
gboolean gl_read_fchk_gauss_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();
	add_objects_for_new_grid();
 	if(gl_read_fchk_gaussn_file_geom(FileName))
	{
		RebuildGeomD = TRUE;
		if(this_is_a_new_geometry()) free_objects_all();
		glarea_rafresh(GLArea);
		return TRUE;
	}
	glarea_rafresh(GLArea);
	return FALSE;
}
/********************************************************/
void gl_read_first_molcas_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();
	add_objects_for_new_grid();
 	gl_read_molcas_file_geomi(FileName,1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
} 
/********************************************************/
void gl_read_last_molcas_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();
	add_objects_for_new_grid();
 	gl_read_molcas_file_geomi(FileName,-1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
}  
/********************************************************/
void gl_read_first_molpro_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();
	add_objects_for_new_grid();
 	gl_read_molpro_file_geomi(FileName,1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
} 
/********************************************************/
void gl_read_last_molpro_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();
	add_objects_for_new_grid();
 	gl_read_molpro_file_geomi(FileName,-1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
}  
/********************************************************/
void gl_read_first_mpqc_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *fileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();
	add_objects_for_new_grid();
 	gl_read_mpqc_file_geomi(fileName,1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
} 
/********************************************************/
void gl_read_last_mpqc_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *fileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();
	add_objects_for_new_grid();
 	gl_read_mpqc_file_geomi(fileName,-1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
}
/********************************************************/
void gl_read_first_mopac_output_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
 	gl_read_mopac_output_file_geomi(FileName,1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
} 
/********************************************************/
void gl_read_last_mopac_output_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();
	add_objects_for_new_grid();
 	gl_read_mopac_output_file_geomi(FileName,-1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
}
/********************************************************/
void gl_read_first_mopac_aux_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
 	gl_read_mopac_aux_file_geomi(FileName,1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
} 
/********************************************************/
void gl_read_last_mopac_aux_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();
	add_objects_for_new_grid();
 	gl_read_mopac_aux_file_geomi(FileName,-1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
}
/********************************************************/
void gl_read_wfx_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
 	gl_read_wfx_file_geom(FileName);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
} 
/********************************************************/
void gl_read_first_orca_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
 	gl_read_orca_file_geomi(FileName,1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
} 
/********************************************************/
void gl_read_last_orca_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();
	add_objects_for_new_grid();
 	gl_read_orca_file_geomi(FileName,-1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
}
/********************************************************/
void gl_read_hessian_orca_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();
	add_objects_for_new_grid();
 	gl_read_orca_file_hessian(FileName);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
}
/********************************************************/
void gl_read_first_qchem_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
 	gl_read_qchem_file_geomi(FileName,1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
} 
/********************************************************/
void gl_read_last_qchem_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();
	add_objects_for_new_grid();
 	gl_read_qchem_file_geomi(FileName,-1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
}
/********************************************************/
void gl_read_first_nwchem_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
 	gl_read_nwchem_file_geomi(FileName,1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
} 
/********************************************************/
void gl_read_nbo_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
 	gl_read_geom_nbo_file(FileName);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
} 
/********************************************************/
void gl_read_last_nwchem_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();
	add_objects_for_new_grid();
 	gl_read_nwchem_file_geomi(FileName,-1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
}
/********************************************************/
void gl_read_last_psicode_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();
	add_objects_for_new_grid();
 	gl_read_psicode_file_geomi(FileName,-1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
}
/********************************************************/
void gl_read_first_psicode_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	add_objects_for_new_grid();
 	gl_read_psicode_file_geomi(FileName,1);
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
} 
/*******************************************************/
void gl_read_molden_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();
	add_objects_for_new_grid();
 	gl_read_molden_file_geom(FileName);
} 
/*******************************************************/
void gl_read_gabedit_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar *FileName;
	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() )
		gtk_main_iteration();
	add_objects_for_new_grid();
 	gl_read_gabedit_file_geom(FileName);
} 
/*******************************************************/
gboolean gl_read_geom_any_file(gchar* fileName)
{
	GabEditTypeFile fileType = get_type_file(fileName);
	gboolean Ok = FALSE;
	if(fileType == GABEDIT_TYPEFILE_XYZ) Ok = gl_read_xyz_file(fileName);
	else if(fileType == GABEDIT_TYPEFILE_PDB) Ok =  gl_read_pdb_file(fileName);
	else if(fileType == GABEDIT_TYPEFILE_HIN) Ok = gl_read_hin_file(fileName);
	else if(fileType == GABEDIT_TYPEFILE_AIMALL) Ok = gl_read_aimall_file(fileName);
	else if(fileType == GABEDIT_TYPEFILE_DALTON) Ok = gl_read_dalton_file_geomi(fileName,-1);
	else if(fileType == GABEDIT_TYPEFILE_GAMESS) Ok = gl_read_gamess_file_geomi(fileName,-1);
	else if(fileType == GABEDIT_TYPEFILE_FIREFLY) Ok = gl_read_gamess_file_geomi(fileName,-1);
	else if(fileType == GABEDIT_TYPEFILE_GAUSSIAN) Ok = gl_read_gaussn_file_geomi(fileName,-1);
	else if(fileType == GABEDIT_TYPEFILE_MOLCAS) Ok = gl_read_molcas_file_geomi(fileName,-1);
	else if(fileType == GABEDIT_TYPEFILE_MOLPRO) Ok = gl_read_molpro_file_geomi(fileName,-1);
	else if(fileType == GABEDIT_TYPEFILE_MOPAC) Ok = gl_read_mopac_output_file_geomi(fileName,-1);
	else if(fileType == GABEDIT_TYPEFILE_MOPAC_AUX) Ok = gl_read_mopac_aux_file_geomi(fileName,-1);
	else if(fileType == GABEDIT_TYPEFILE_MPQC) Ok = gl_read_mpqc_file_geomi(fileName,-1);
	else if(fileType == GABEDIT_TYPEFILE_ORCA) Ok = gl_read_orca_file_geomi(fileName,-1);
	else if(fileType == GABEDIT_TYPEFILE_QCHEM) Ok = gl_read_qchem_file_geomi(fileName,-1);
	else if(fileType == GABEDIT_TYPEFILE_NWCHEM) Ok = (0!=gl_read_nwchem_file_geomi(fileName,-1));
	else if(fileType == GABEDIT_TYPEFILE_GABEDIT) Ok = gl_read_gabedit_file_geom(fileName);
	else if(fileType == GABEDIT_TYPEFILE_MOLDEN) Ok = gl_read_molden_file_geom(fileName);
	else if(fileType == GABEDIT_TYPEFILE_WFX) Ok = gl_read_wfx_file_geom(fileName);
	else if(fileType == GABEDIT_TYPEFILE_GAUSSIAN_FCHK) Ok = gl_read_fchk_gaussn_file_geom(fileName);
	else if(
		fileType ==  GABEDIT_TYPEFILE_ORCAINPUT||
		fileType ==  GABEDIT_TYPEFILE_FIREFLYINPUT||
		fileType ==  GABEDIT_TYPEFILE_GAMESSINPUT||
		fileType ==  GABEDIT_TYPEFILE_QCHEMINPUT||
		fileType ==  GABEDIT_TYPEFILE_NWCHEMINPUT||
		fileType ==  GABEDIT_TYPEFILE_MOPACINPUT||
		fileType ==  GABEDIT_TYPEFILE_MPQCINPUT||
		fileType ==  GABEDIT_TYPEFILE_GAUSSIANINPUT||
		fileType ==  GABEDIT_TYPEFILE_MOLCASINPUT||
		fileType ==  GABEDIT_TYPEFILE_MOLPROINPUT
	)
	{
		Message(
			_(
			"Sorry, I cannot read this file here\n"
			"You can read it from the 'Draw Geometry' window, save it in hin and read it from this window 'Display window'\n"
			)
			,_("Error"),TRUE);
		return FALSE;
	}
	else if(fileType == GABEDIT_TYPEFILE_UNKNOWN) 
	{
		Message(
			_(
			"Sorry, I cannot find the type of your file\n"
			" Try to read it using openbabel\n"
			)
			,_("Error"),TRUE);
		return FALSE;
	}
	RebuildGeomD = TRUE;
	if(this_is_a_new_geometry()) free_objects_all();
	glarea_rafresh(GLArea);
	return Ok;
}
/*************************************************************************/
void gl_read_geom_any_file_sel(GabeditFileChooser *selecFile, gint response_id)
{
	gchar *fileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(selecFile);
	gtk_widget_hide(GTK_WIDGET(selecFile));
	while( gtk_events_pending() ) gtk_main_iteration();
 
	add_objects_for_new_grid();
 	gl_read_geom_any_file(fileName);
}
