/* GeomConversion.c */
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
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Common/Help.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Geometry/GeomGlobal.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Files/ListeFiles.h"
#include "../Common/Windows.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Gaussian/Gaussian.h"
#include "../Molpro/Molpro.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/GeomZmatrix.h"

/*****************************************************************************/
typedef struct _GXYZ
{
 gchar *Symb;
 gchar *mmType;
 gchar *pdbType;
 gchar *Residue;
 gchar *Layer;
 gchar *Charge;
 gboolean Variable;
 Point P;
 gint* typeConnections;
}GXYZ;

static GXYZ* gxyz = NULL; /* gemetry in xyz mod with dummy atoms*/
static gint Nat = 0;
static gboolean *inStack = NULL;
static	gboolean done = FALSE;
static gint bonds = 0;
static	gint ringSize = 3;
/*****************************************************************************/
void free_gxyz()
{
	gint i;
	if(!gxyz) return;
	for(i=0;i<Nat;i++)
	{
		if(gxyz[i].Symb) g_free(gxyz[i].Symb);
		if(gxyz[i].mmType) g_free(gxyz[i].mmType);
		if(gxyz[i].pdbType) g_free(gxyz[i].pdbType);
		if(gxyz[i].Residue) g_free(gxyz[i].Residue);
		if(gxyz[i].Charge) g_free(gxyz[i].Charge);
		if(gxyz[i].Layer) g_free(gxyz[i].Layer);
		if(gxyz[i].typeConnections) g_free(gxyz[i].typeConnections);
	}
	g_free(gxyz);
	if(inStack) g_free(inStack);
	inStack = NULL;
	done = FALSE;
	bonds = 0;
	ringSize = 3;
	gxyz = NULL;
}
/*****************************************************************************/
static Point get_coord_xyz_float(gint i)
{
	Point A;
  	if(!test(GeomXYZ[i].X))
    		A.C[0] = get_value_variableXYZ(GeomXYZ[i].X);
  	else
    		A.C[0] = atof(GeomXYZ[i].X);
  	if(!test(GeomXYZ[i].Y))
    		A.C[1] = get_value_variableXYZ(GeomXYZ[i].Y);
  	else
    		A.C[1] = atof(GeomXYZ[i].Y);
  	if(!test(GeomXYZ[i].Z))
    		A.C[2] = get_value_variableXYZ(GeomXYZ[i].Z);
  	else
    		A.C[2] = atof(GeomXYZ[i].Z);
	return A;

}
/*****************************************************************************/
gboolean set_gxyz()
{
	gint i,j;
	Point A;
	if(!GeomXYZ)
		return FALSE;
	free_gxyz();
	Nat = NcentersXYZ;
	gxyz = g_malloc(Nat*sizeof(GXYZ));
	for(i=0;i<Nat;i++)
	{
		A = get_coord_xyz_float(i);
		gxyz[i].P = A;
		gxyz[i].Symb = g_strdup(GeomXYZ[i].Symb);
		gxyz[i].mmType = g_strdup(GeomXYZ[i].mmType);
		gxyz[i].pdbType = g_strdup(GeomXYZ[i].pdbType);
		gxyz[i].Residue = g_strdup(GeomXYZ[i].Residue);
		gxyz[i].Layer = g_strdup(GeomXYZ[i].Layer);
		gxyz[i].Charge = g_strdup(GeomXYZ[i].Charge);
		if(!test(GeomXYZ[i].X) || !test(GeomXYZ[i].Y) || !test(GeomXYZ[i].Z)) gxyz[i].Variable=TRUE;
		else gxyz[i].Variable=FALSE;
		gxyz[i].typeConnections = g_malloc(Nat*sizeof(gint));
		for(j=0;j<Nat;j++) gxyz[i].typeConnections[j] =0;
		if(GeomXYZ[i].typeConnections) for(j=0;j<Nat;j++) gxyz[i].typeConnections[j] =GeomXYZ[i].typeConnections[j];
	}
	return TRUE;

}
/*****************************************************************************/
static gdouble get_distance_xyz(gint i,gint j)
{
        gchar *serr;
        gdouble res;

        if(i>=Nat || j >=Nat || i<0 || j<0)
        {
           return -1000; 
        }

        
   	serr = get_distance_points(gxyz[i].P,gxyz[j].P,FALSE);
	res = atof(serr)/BOHR_TO_ANG;
	g_free(serr);
        return res;
}
/*****************************************************************************/
static gdouble get_angle_xyz(gint i,gint j,gint l)
{
        Point A;
        Point B;
        Point V1;
        Point V2;
        guint k;

        if(i>=Nat || j >=Nat || l >=Nat || i<0 || j<0 || l<0)
        {
           return -1000; 
        }
	A = gxyz[i].P;
	B = gxyz[j].P;
	for(k=0;k<3;k++)
		V1.C[k] = A.C[k] - B.C[k];
	A = gxyz[l].P;
	for(k=0;k<3;k++)
		V2.C[k] = A.C[k] - B.C[k];

        return atof(get_angle_vectors(V1,V2));
}
/*****************************************************************************/
static gdouble get_dihedral_xyz(gint i,gint j,gint l,gint m)
{
        Point A;
        Point B;
        Point V1;
        Point V2;
        Point W1;
        Point W2;
        guint k;
	gdouble angle;
	gdouble dihsgn;
	static gdouble precision = 1e-4;

	W1 = gxyz[i].P;
	W2 = gxyz[j].P;
	for(k=0;k<3;k++)
		V1.C[k] = W1.C[k] - W2.C[k];
	W1 = gxyz[l].P;
	for(k=0;k<3;k++)
		V2.C[k] = W1.C[k] - W2.C[k];

        A = get_produit_vectoriel(V1,V2);
	if(A.C[0]*A.C[0]+A.C[1]*A.C[1]+A.C[2]*A.C[2]<precision)
	{
           return -1000;
	}

	W1 = gxyz[m].P;
	W2 = gxyz[l].P;
	for(k=0;k<3;k++)
		V1.C[k] = W1.C[k] - W2.C[k];
	W1 = gxyz[j].P;
	for(k=0;k<3;k++)
		V2.C[k] = W1.C[k] - W2.C[k];

        B = get_produit_vectoriel(V2,V1);
	if(B.C[0]*B.C[0]+B.C[1]*B.C[1]+B.C[2]*B.C[2]<precision)
	{
           return -1001;
	}

        angle = atof(get_angle_vectors(A,B));
        W1 = get_produit_vectoriel(A,B);
        if (get_module(W1)<1e-5 )
              dihsgn = 1.0e0;
        else
	{
        dihsgn = get_scalaire(W1,V2);
        if (dihsgn>0)
            dihsgn = -1.0e0;
        else
           dihsgn = 1.0e0;
	}
          angle *=dihsgn;
	  return angle;
}
/*****************************************************************************/
Point get_X_position(gint i,gint j)
{
        Point V1;
        Point V2;
        Point W1;
        Point W2;
	gint ii;
	gint jj;
	gint k;
	gdouble n;

	W1 = gxyz[i].P;
	W2 = gxyz[j].P;
	for(k=0;k<3;k++)
		V1.C[k] = W1.C[k] - W2.C[k];

	/* Normalize Vector*/
	n = 0.0;
	for(ii=0;ii<3;ii++)
		n += V1.C[ii]*V1.C[ii];
	n = sqrt(n);
	for(ii=0;ii<3;ii++)
		V1.C[ii] /= n;
	jj = 0;
	for(ii=1;ii<3;ii++)
		if(fabs(V1.C[ii])>fabs(V1.C[jj]))
			jj = ii;

	V2.C[jj] = 0.0;
	V2.C[(jj+1)%3] = 1;
	V2.C[(jj+2)%3] = 1;

	/* Debug("j=%d\n",j);*/
	n = 0.0;
	for(ii=0;ii<3;ii++)
		n += 	V1.C[ii]*V2.C[ii];

	if(V1.C[jj] != 0)
		V2.C[jj] = -n/V1.C[jj];
	else
	{
		for(ii=0;ii<3;ii++)
			V2.C[ii] = 1;
		V2.C[(jj+1)%3] = 1.0;
	}
	/* Normalize V[0]*/
	n = 0.0;
	for(ii=0;ii<3;ii++)
		n += V2.C[ii]*V2.C[ii];
	n = sqrt(n);
	for(ii=0;ii<3;ii++)
		V2.C[ii] /= n;

	for(k=0;k<3;k++)
		V2.C[k] += W1.C[k];
	return V2;
}
/*****************************************************************************/
void sort_gxyz()
{
  gint i;
  gint j;
  gint k;
  GXYZ t;

  if (Nat <= 0)
    return ;
  for (i = 1; i <(gint)Nat-1; i++)
  {
	k = i;
  	for (j = i+1; j <Nat; j++)
  	if( get_distance_xyz(j,i-1)<get_distance_xyz(k,i-1))
		k = j; 
	if(k!=i)
	{
		t= gxyz[i];
		gxyz[i] = gxyz[k];
		gxyz[k] = t;
	}
  }
}
/*****************************************************************************/
void insert_dummy_atom_gxyz(gint after,Point P)
{
	gint i,j;
	Nat++;
	gxyz=g_realloc(gxyz,Nat*sizeof(GXYZ));
	for (j = 0; j<Nat-1;j++)
	{
		gxyz[j].typeConnections=g_realloc(gxyz[j].typeConnections,Nat*sizeof(gint));
		for (i = Nat-1; i >after;i--)
			gxyz[j].typeConnections[i]=gxyz[j].typeConnections[i-1];
		gxyz[j].typeConnections[after]=0;
	}
	for (i = Nat-1; i >after;i--)
  	{
		gxyz[i] = gxyz[i-1];
	}
	gxyz[after].Symb = g_strdup("X");
	gxyz[after].mmType = g_strdup("X");
	gxyz[after].pdbType = g_strdup("X");
	gxyz[after].Residue = g_strdup("DUM");
	gxyz[after].Layer = g_strdup(" ");
	gxyz[after].Charge = g_strdup("0.0");
	gxyz[after].Variable=FALSE;
	for (i = 0; i<3;i++)
		gxyz[after].P.C[i] = P.C[i];
	gxyz[after].typeConnections=g_malloc(Nat*sizeof(gint));
	for (i = 0; i <Nat;i++) gxyz[after].typeConnections[i]=0;
	/*
	for (i = 0; i<Nat;i++)
	       Debug("%s %f %f %f\n",gxyz[i].Symb,gxyz[i].P.C[0],gxyz[i].P.C[1],gxyz[i].P.C[2]);
	*/
}
/*****************************************************************************/
void insert_dummy_atoms_gxyz()
{
  gint i;
  gint k;
  gdouble dih;
  Point A;
  gboolean Ok = TRUE;

  if (Nat <= 3)
    return ;
  while(Ok)
  {
	  Ok = FALSE;
  for (i = 3; i <(gint)Nat; i++)
  {
    dih = get_dihedral_xyz(i-3,i-2,i-1,i);
    k = -1;
    if(dih==-1000)
	    k = i-2;
    else
    if(dih==-1001)
	    k = i-1;
    if(k!=-1)
    {
    	printf("Warning : insert dummy atom  : %d %d %d %d are linear\n",i-2,i-1,i,i+1);
	A = get_X_position(k,k+1);
	insert_dummy_atom_gxyz(k,A);
	Ok = TRUE;
	break;
    }
  }
  }
}
/*****************************************************************************/
void delete_dummy_atoms()
{
	gint ndummy = 0;
	gint i = 0;
	gint j;
	gint k;
	GeomXYZAtomDef t;
	for(i=NcentersXYZ-1;i>0;i--)
	{
		if(strstr(GeomXYZ[i].Symb,"X"))
			continue;
		k = -1;
		for(j=i-1;j>=0;j--)
			if(strstr(GeomXYZ[j].Symb,"X"))
			{
				k = j;
				break;
			}
		if(k!=-1)
		{
			t = GeomXYZ[i];
			GeomXYZ[i] = GeomXYZ[k];
			GeomXYZ[k] = t;
		}
		else
			break;
	}
	for(i=0;i<(gint)NcentersXYZ;i++)
	{
		if(strstr(GeomXYZ[i].Symb,"X"))
		{
			ndummy++;
			if(GeomXYZ[i].Symb)
				g_free(GeomXYZ[i].Symb);
			if(GeomXYZ[i].mmType)
				g_free(GeomXYZ[i].mmType);
			if(GeomXYZ[i].pdbType)
				g_free(GeomXYZ[i].pdbType);
			if(GeomXYZ[i].Layer)
				g_free(GeomXYZ[i].Layer);
			if(GeomXYZ[i].X)
				g_free(GeomXYZ[i].X);
			if(GeomXYZ[i].Y)
				g_free(GeomXYZ[i].Y);
			if(GeomXYZ[i].Z)
				g_free(GeomXYZ[i].Z);
		}
	}

	if(ndummy>0)
	{
		/*Debug("ndummys = %d\n",ndummy);*/
		NcentersXYZ -= ndummy;
		GeomXYZ = g_realloc(GeomXYZ,NcentersXYZ*sizeof(GeomXYZAtomDef));
	}
}
/*****************************************************************************/
gboolean zmat_to_xyz()
{
  gdouble cosph,sinph,costh,sinth,coskh,sinkh;
  gdouble cosa,sina,cosd,sind;
  gdouble dist,angle,dihed;
  gdouble xpd,ypd,zpd,xqd,yqd,zqd;
  gdouble xa,ya,za,xb,yb,zb;
  gdouble rbc,xyb,yza,temp;
  gdouble xpa,ypa,zqa;
  gdouble xd,yd,zd;
  gboolean flag;
  gint i, na, nb, nc;
  GeomXYZAtomDef* GeomXYZtemp = NULL;

  if (NcentersZmat <= 0)
    return( FALSE );

  GeomXYZtemp=g_malloc(NcentersZmat*sizeof(GeomXYZAtomDef));
  for (i = 0; i <(gint)NcentersZmat; i++)
  {
  GeomXYZtemp[i].Nentry = NUMBER_LIST_XYZ;
  GeomXYZtemp[i].Symb = NULL;
  GeomXYZtemp[i].mmType = NULL;
  GeomXYZtemp[i].pdbType = NULL;
  GeomXYZtemp[i].Residue = NULL;
  GeomXYZtemp[i].X = NULL; 
  GeomXYZtemp[i].Y = NULL; 
  GeomXYZtemp[i].Z = NULL; 
  GeomXYZtemp[i].Charge = NULL;
  GeomXYZtemp[i].Layer = NULL;
  GeomXYZtemp[i].typeConnections = NULL;
  }

  /* Atom #1 */
  GeomXYZtemp[0].Nentry = NUMBER_LIST_XYZ;
  GeomXYZtemp[0].Symb = g_strdup(Geom[0].Symb);
  GeomXYZtemp[0].mmType = g_strdup(Geom[0].mmType);
  GeomXYZtemp[0].pdbType = g_strdup(Geom[0].pdbType);
  GeomXYZtemp[0].Residue = g_strdup(Geom[0].Residue);
  GeomXYZtemp[0].X = g_strdup("0.0");
  GeomXYZtemp[0].Y =  g_strdup("0.0"); 
  GeomXYZtemp[0].Z =  g_strdup("0.0"); 
  GeomXYZtemp[0].Charge =  g_strdup("0.0"); 
  GeomXYZtemp[0].Layer = g_strdup(Geom[0].Layer);
  
  if(NcentersZmat==1)
  {
          FreeGeomXYZ(GeomXYZ, VariablesXYZ, NcentersXYZ, NVariablesXYZ);
	  NcentersXYZ = NcentersZmat;
	  GeomXYZ = GeomXYZtemp;
	  NVariablesXYZ =0;
	  VariablesXYZ = NULL;
	  reset_connections_XYZ();
	  return TRUE;
  }
  /* Atom #2 */
  GeomXYZtemp[1].Nentry = NUMBER_LIST_XYZ;
  GeomXYZtemp[1].Symb = g_strdup(Geom[1].Symb);
  GeomXYZtemp[1].mmType = g_strdup(Geom[1].mmType);
  GeomXYZtemp[1].pdbType = g_strdup(Geom[1].pdbType);
  GeomXYZtemp[1].Residue = g_strdup(Geom[1].Residue);
  GeomXYZtemp[1].Charge = g_strdup(Geom[1].Charge);
  if(!test(Geom[1].R))
    GeomXYZtemp[1].X = g_strdup_printf("%f",get_value_variableZmat(Geom[1].R));
  else
    GeomXYZtemp[1].X = g_strdup(Geom[1].R);

  GeomXYZtemp[1].Y = g_strdup("0.0"); 
  GeomXYZtemp[1].Z =g_strdup("0.0");  
  GeomXYZtemp[1].Layer = g_strdup(Geom[1].Layer);

  if(NcentersZmat==2)
  {
          FreeGeomXYZ(GeomXYZ, VariablesXYZ, NcentersXYZ, NVariablesXYZ);
	  NcentersXYZ = NcentersZmat;
	  GeomXYZ = GeomXYZtemp;
	  NVariablesXYZ =0;
	  VariablesXYZ = NULL;
  	  if(!test(Geom[1].R)) set_variable_one_atom_in_GeomXYZ(1);
	  reset_connections_XYZ();
	  return TRUE;
  }
  
  /* Atom #3 */
  GeomXYZtemp[2].Nentry = NUMBER_LIST_XYZ;
  GeomXYZtemp[2].Symb = g_strdup(Geom[2].Symb);
  GeomXYZtemp[2].mmType = g_strdup(Geom[2].mmType);
  GeomXYZtemp[2].pdbType = g_strdup(Geom[2].pdbType);
  GeomXYZtemp[2].Residue = g_strdup(Geom[2].Residue);
  GeomXYZtemp[2].Charge = g_strdup(Geom[2].Charge);
  if(!test(Geom[2].R))
    dist = get_value_variableZmat(Geom[2].R);
  else
    dist = atof(Geom[2].R);

  if(!test(Geom[2].Angle))
    angle = get_value_variableZmat(Geom[2].Angle);
  else
    angle = atof(Geom[2].Angle);

  GeomXYZtemp[1].Y = g_strdup("0.0"); 
  GeomXYZtemp[1].Z =g_strdup("0.0");  
  GeomXYZtemp[1].Layer = g_strdup(Geom[1].Layer);

  if(!test(Geom[2].R))
    dist = get_value_variableZmat(Geom[2].R);
  else
    dist = atof(Geom[2].R);

  if(!test(Geom[2].Angle))
    angle = get_value_variableZmat(Geom[2].Angle);
  else
    angle = atof(Geom[2].Angle);

    angle *=  DEG_TO_RAD;

  cosa = cos(angle);
  sina = sin(angle);
  if( atoi (Geom[2].NAngle) == 2 )
  	GeomXYZtemp[2].X =  g_strdup_printf("%f",atof(GeomXYZtemp[0].X) + cosa*dist);
  else 
  	GeomXYZtemp[2].X =  g_strdup_printf("%f",atof(GeomXYZtemp[1].X) - cosa*dist);
  
  GeomXYZtemp[2].Y =  g_strdup_printf("%f",sina*dist);
  GeomXYZtemp[2].Z = g_strdup("0.0");
  GeomXYZtemp[2].Layer = g_strdup(Geom[2].Layer);
  
  if(NcentersZmat==3)
  {
          FreeGeomXYZ(GeomXYZ, VariablesXYZ, NcentersXYZ, NVariablesXYZ);
	  NcentersXYZ = NcentersZmat;
	  GeomXYZ = GeomXYZtemp;
	  NVariablesXYZ =0;
	  VariablesXYZ = NULL;
  	  if(!test(Geom[1].R)) set_variable_one_atom_in_GeomXYZ(1);
  	  if(!test(Geom[2].R) || !test(Geom[2].Angle) ) set_variable_one_atom_in_GeomXYZ(2);
	  reset_connections_XYZ();
	  return TRUE;
  }
  for (i = 3; i <(gint)NcentersZmat; i++)
  {   
  if(!test(Geom[i].R))
    dist = get_value_variableZmat(Geom[i].R);
  else
    dist = atof(Geom[i].R);

  if(!test(Geom[i].Angle))
    angle = get_value_variableZmat(Geom[i].Angle);
  else
    angle = atof(Geom[i].Angle) ;

  if(!test(Geom[i].Dihedral))
    dihed = get_value_variableZmat(Geom[i].Dihedral);
  else
    dihed = atof(Geom[i].Dihedral) ;

    angle *= DEG_TO_RAD;
    dihed *= DEG_TO_RAD;

    na = atoi(Geom[i].NR)-1;
    nb = atoi(Geom[i].NAngle)-1;
    nc = atoi(Geom[i].NDihedral)-1;
    
    xb = atof(GeomXYZtemp[nb].X) - atof(GeomXYZtemp[na].X);
    yb = atof(GeomXYZtemp[nb].Y) - atof(GeomXYZtemp[na].Y);
    zb = atof(GeomXYZtemp[nb].Z) - atof(GeomXYZtemp[na].Z);
    
    rbc = xb*xb + yb*yb + zb*zb;
    if( rbc < 0.0001 )
    {
      FreeGeomXYZ(GeomXYZtemp, NULL, NcentersZmat, 0);
      return( FALSE );
    }
    rbc = 1.0/sqrt(rbc);
    
    cosa = cos(angle);
    sina = sin(angle);
    
    
    if( fabs(cosa) >= 0.999999 )
    { 
      /* Colinear */
      temp = dist*rbc*cosa;
      GeomXYZtemp[i].X  = g_strdup_printf("%f",atof(GeomXYZtemp[na].X) + temp*xb);
      GeomXYZtemp[i].Y  = g_strdup_printf("%f",atof(GeomXYZtemp[na].Y) + temp*yb);
      GeomXYZtemp[i].Z  = g_strdup_printf("%f",atof(GeomXYZtemp[na].Z) + temp*zb);
    } 
    else
    {
      xa = atof(GeomXYZtemp[nc].X) - atof(GeomXYZtemp[na].X);
      ya = atof(GeomXYZtemp[nc].Y) - atof(GeomXYZtemp[na].Y);
      za = atof(GeomXYZtemp[nc].Z) - atof(GeomXYZtemp[na].Z);
      
      sind = -sin(dihed);
      cosd = cos(dihed);
      
      xd = dist*cosa;
      yd = dist*sina*cosd;
      zd = dist*sina*sind;
      
      xyb = sqrt(xb*xb + yb*yb);
      if( xyb < 0.1 )
      {  
	/* Rotate about y-axis! */
	temp = za; za = -xa; xa = temp;
	temp = zb; zb = -xb; xb = temp;
	xyb = sqrt(xb*xb + yb*yb);
	flag = TRUE;
      }
      else 
	flag = FALSE;
      
      costh = xb/xyb;
      sinth = yb/xyb;
      xpa = costh*xa + sinth*ya;
      ypa = costh*ya - sinth*xa;
      
      sinph = zb*rbc;
      cosph = sqrt(1.0 - sinph*sinph);
      zqa = cosph*za  - sinph*xpa;
      
      yza = sqrt(ypa*ypa + zqa*zqa);
      
      if( yza > 1.0E-10 )
      {   
	coskh = ypa/yza;
	sinkh = zqa/yza;
	
	ypd = coskh*yd - sinkh*zd;
	zpd = coskh*zd + sinkh*yd;
      } 
      else
      { 
	/* coskh = 1.0; */
	/* sinkh = 0.0; */
	ypd = yd;
	zpd = zd;
      }
      
      xpd = cosph*xd  - sinph*zpd;
      zqd = cosph*zpd + sinph*xd;
      xqd = costh*xpd - sinth*ypd;
      yqd = costh*ypd + sinth*xpd;
      
      if( flag )
      { 
	/* Rotate about y-axis! */
	GeomXYZtemp[i].X = g_strdup_printf("%f",atof(GeomXYZtemp[na].X) - zqd);
	GeomXYZtemp[i].Y = g_strdup_printf("%f",atof(GeomXYZtemp[na].Y) + yqd);
	GeomXYZtemp[i].Z = g_strdup_printf("%f",atof(GeomXYZtemp[na].Z) + xqd);
      } 
      else
      {  
	GeomXYZtemp[i].X =g_strdup_printf("%f",atof(GeomXYZtemp[na].X) + xqd);
	GeomXYZtemp[i].Y =g_strdup_printf("%f",atof(GeomXYZtemp[na].Y) + yqd);
	GeomXYZtemp[i].Z =g_strdup_printf("%f",atof(GeomXYZtemp[na].Z) + zqd);
      }
    }
  }
  for(i=3;i<(gint)NcentersZmat;i++)
  {
  	GeomXYZtemp[i].Nentry = NUMBER_LIST_XYZ;
	GeomXYZtemp[i].Symb = g_strdup(Geom[i].Symb);
	GeomXYZtemp[i].mmType = g_strdup(Geom[i].mmType);
	GeomXYZtemp[i].pdbType = g_strdup(Geom[i].pdbType);
	GeomXYZtemp[i].Residue = g_strdup(Geom[i].Residue);
	GeomXYZtemp[i].Charge = g_strdup(Geom[i].Charge);
	GeomXYZtemp[i].Layer = g_strdup(Geom[i].Layer);
  }
  FreeGeomXYZ(GeomXYZ, VariablesXYZ, NcentersXYZ, NVariablesXYZ);
  NcentersXYZ = NcentersZmat;
  GeomXYZ = GeomXYZtemp;
  NVariablesXYZ =0;
  VariablesXYZ = NULL;
  if(!test(Geom[1].R)) set_variable_one_atom_in_GeomXYZ(1);
  if(!test(Geom[2].R) || !test(Geom[2].Angle) ) set_variable_one_atom_in_GeomXYZ(2);
  for(i=3;i<(gint)NcentersZmat;i++)
  {
  	if(!test(Geom[i].R) || !test(Geom[i].Angle) || !test(Geom[i].Dihedral) ) set_variable_one_atom_in_GeomXYZ(i);
  }
	  reset_connections_XYZ();
  return TRUE;
}
/*****************************************************************************/
static void rafresh_interface()
{
	if(GeomIsOpen)
	{
		if(MethodeGeom==GEOM_IS_XYZ)
		create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
		else
		create_geom_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
	}
	else
	{
		switch(iprogram)
		{
			case PROG_IS_MOLPRO : 
				insert_molpro(2);
			break;
			case PROG_IS_GAUSS : 
				insert_gaussian(2);
			break;
			default :
				edit_geometry();
		}
        }
	if(GeomDrawingArea != NULL)
		rafresh_drawing();
}
/*****************************************************************************/
void conversion_zmat_to_xyz()
{
	if(!zmat_to_xyz())
	{
		printf(_("I cannot convert the zmat to xyz\n"));
		return;
	}
	delete_dummy_atoms();
 	MethodeGeom = GEOM_IS_XYZ;
	rafresh_interface();
}
/*****************************************************************************/
gboolean xyz_to_zmat()
{
  gint i;
  GeomAtomDef* Geomtemp = NULL;

  set_gxyz();
  if (Nat <= 0)
    return FALSE ;
  insert_dummy_atoms_gxyz();

  Geomtemp=g_malloc(Nat*sizeof(GeomAtomDef));
  for (i = 0; i <(gint)Nat; i++)
  {
  Geomtemp[i].Nentry = NUMBER_LIST_ZMATRIX;
  Geomtemp[i].Symb = NULL;
  Geomtemp[i].mmType = NULL;
  Geomtemp[i].pdbType = NULL;
  Geomtemp[i].Residue = NULL;
  Geomtemp[i].R = NULL; 
  Geomtemp[i].NR = NULL; 
  Geomtemp[i].Angle = NULL; 
  Geomtemp[i].NAngle = NULL; 
  Geomtemp[i].Dihedral = NULL; 
  Geomtemp[i].NDihedral = NULL; 
  Geomtemp[i].Charge = NULL;
  Geomtemp[i].Layer = NULL;
  }

  /* Atom #1 */
  Geomtemp[0].Nentry = NUMBER_ENTRY_0;
  Geomtemp[0].Symb = g_strdup(gxyz[0].Symb);
  Geomtemp[0].mmType = g_strdup(gxyz[0].mmType);
  Geomtemp[0].pdbType = g_strdup(gxyz[0].pdbType);
  Geomtemp[0].Residue = g_strdup(gxyz[0].Residue);
  Geomtemp[0].Charge = g_strdup(gxyz[0].Charge);
  Geomtemp[0].Layer = g_strdup(gxyz[0].Layer);
  
  if(Nat==1)
  {
          freeGeom();
          freeVariables();
	  NcentersZmat = Nat;
          freeGeomXYZ();
          freeVariablesXYZ();
	  Geom = Geomtemp;
	  NVariables =0;
	  Variables = NULL;
	  if(gxyz[0].Variable) set_variable_one_atom_in_GeomZMatrix(0);
	  return TRUE;
  }
  /* Atom #2 */
  Geomtemp[1].Nentry =  NUMBER_ENTRY_R;
  Geomtemp[1].Symb = g_strdup(gxyz[1].Symb);
  Geomtemp[1].mmType = g_strdup(gxyz[1].mmType);
  Geomtemp[1].pdbType = g_strdup(gxyz[1].pdbType);
  Geomtemp[1].Residue = g_strdup(gxyz[1].Residue);
  Geomtemp[1].Charge = g_strdup(gxyz[1].Charge);
  Geomtemp[1].Layer = g_strdup(gxyz[1].Layer);
  Geomtemp[1].R = g_strdup_printf("%f",get_distance_xyz(0,1));
  Geomtemp[1].NR = g_strdup_printf("%d",1);

  if(Nat==2)
  {
          freeGeom();
          freeVariables();
	  NcentersZmat = Nat;
          freeGeomXYZ();
          freeVariablesXYZ();
	  Geom = Geomtemp;
	  NVariables =0;
	  Variables = NULL;
  	  for (i = 0; i <(gint)Nat; i++)
	  	if(gxyz[i].Variable) set_variable_one_atom_in_GeomZMatrix(i);
	  return TRUE;
  }
  
  /* Atom #3 */
  Geomtemp[2].Nentry =  NUMBER_ENTRY_ANGLE;
  Geomtemp[2].Symb = g_strdup(gxyz[2].Symb);
  Geomtemp[2].mmType = g_strdup(gxyz[2].mmType);
  Geomtemp[2].pdbType = g_strdup(gxyz[2].pdbType);
  Geomtemp[2].Residue = g_strdup(gxyz[2].Residue);
  Geomtemp[2].Charge = g_strdup(gxyz[2].Charge);
  Geomtemp[2].Layer = g_strdup(gxyz[2].Layer);
  Geomtemp[2].R = g_strdup_printf("%f",get_distance_xyz(1,2));
  Geomtemp[2].NR = g_strdup_printf("%d",2);
  Geomtemp[2].Angle =  g_strdup_printf("%f",get_angle_xyz(2,1,0));
  Geomtemp[2].NAngle = g_strdup_printf("%d",1);
  
  if(Nat==3)
  {
          freeGeom();
          freeVariables();
	  NcentersZmat = Nat;
          freeGeomXYZ();
          freeVariablesXYZ();
	  Geom = Geomtemp;
	  NVariables =0;
	  Variables = NULL;
  	  for (i = 0; i <(gint)Nat; i++)
	  	if(gxyz[i].Variable) set_variable_one_atom_in_GeomZMatrix(i);
	  return TRUE;
  }
  for (i = 3; i <(gint)Nat; i++)
  {   
  Geomtemp[i].Nentry =  NUMBER_ENTRY_DIHEDRAL;
  Geomtemp[i].Symb = g_strdup(gxyz[i].Symb);
  Geomtemp[i].mmType = g_strdup(gxyz[i].mmType);
  Geomtemp[i].pdbType = g_strdup(gxyz[i].pdbType);
  Geomtemp[i].Residue = g_strdup(gxyz[i].Residue);
  Geomtemp[i].Charge = g_strdup(gxyz[i].Charge);
  Geomtemp[i].Layer = g_strdup(gxyz[i].Layer);
  Geomtemp[i].R =  g_strdup_printf("%f",get_distance_xyz(i-1,i));
  Geomtemp[i].NR = g_strdup_printf("%d",i);
  Geomtemp[i].Angle =  g_strdup_printf("%f",get_angle_xyz(i,i-1,i-2));
  Geomtemp[i].NAngle = g_strdup_printf("%d",i-1);
  Geomtemp[i].Dihedral =  g_strdup_printf("%f",get_dihedral_xyz(i,i-1,i-2,i-3));
  Geomtemp[i].NDihedral = g_strdup_printf("%d",i-2);
  }
  freeGeom();
  freeVariables();
  NcentersZmat = Nat;
  freeGeomXYZ();
  freeVariablesXYZ();
  Geom = Geomtemp;
  NVariables =0;
  Variables = NULL;
  for (i = 0; i <(gint)Nat; i++)
	  if(gxyz[i].Variable) set_variable_one_atom_in_GeomZMatrix(i);

  return TRUE;
}
/*****************************************************************************/
void conversion_xyz_to_zmat()
{
	if(!xyz_to_zmat())
	{
		Message(_("Sorry\nConversion is not possible"),_("Warning"),TRUE);
		return;
	}
 	MethodeGeom = GEOM_IS_ZMAT;
	rafresh_interface();
}
/*****************************************************************************/
static gint get_parent_upper_level(gint numAtom, gint* levels)
{
	gint j;
  	for (j = 0; j <(gint)Nat; j++) 
		if(levels[j]==0 && gxyz[numAtom].typeConnections[j]>0) return j;
	return -1;
}
/*****************************************************************************/
static gint get_number_of_connections_to_upper_level(gint numAtom, gint* levels)
{
	gint j;
	gint n = 0;
  	for (j = 0; j <(gint)Nat; j++) 
		if(levels[j]==0 && gxyz[numAtom].typeConnections[j]>0) n++;
	return n;
}
/*****************************************************************************/
/*
static gboolean is_grand_parent_upper_level(gint numAtom, gint* levels)
{
	gint j;
  	for (j = 0; j <(gint)Nat; j++) 
		if(levels[j]==0 && gxyz[numAtom].typeConnections[j]>0) 
		{
			if(get_number_of_connections_to_upper_level(j, levels)>1) return TRUE;
		}
	return FALSE;
}
*/
/*****************************************************************************/
static gint add_level_non_connected_atom(gint* levels, GList** lists, gint nLevels)
{
	gint i;
	gint n = 0;
  	for (i = 0; i <(gint)Nat; i++) 
	{
		if(levels[i]>0) continue;
		n = get_number_of_connections_to_upper_level(i, levels);
		if(n==0)
		{
			levels[i] = nLevels+1;
			lists[nLevels] = g_list_append(lists[nLevels],GINT_TO_POINTER(-1)); /* the first is the grand parent */
			lists[nLevels] = g_list_append(lists[nLevels],GINT_TO_POINTER(-1)); /* the second is the parent */
			lists[nLevels] = g_list_append(lists[nLevels],GINT_TO_POINTER(i)); /* the first son */
			return 1;
		}
	}
	return 0;
}
/*****************************************************************************/
static gint add_levels_connected_to_one_atom(gint* levels, GList** lists, gint nLevels, gint* nNewLevels)
{
	gint i;
	gint n = 0;
	gint nAll = 0;
	gint nC = 0;
	gint nL = 0;
	gint* oneConnectedAtoms = NULL;
	gint* parents = NULL;
	if(Nat<1) return 0;
	oneConnectedAtoms = g_malloc(Nat*sizeof(gint));
	parents = g_malloc(Nat*sizeof(gint));
  	for (i = 0; i <(gint)Nat; i++) 
	{
		if(levels[i]>0) continue;
		n = get_number_of_connections_to_upper_level(i, levels);
		if(n==1) 
		{
			oneConnectedAtoms[nC] = i;
			parents[nC] = get_parent_upper_level(i, levels);
			nC++;
		}
	}
	if(nC<1) 
	{
		if(oneConnectedAtoms) g_free(oneConnectedAtoms);
		if(parents) g_free(parents);
		return 0;
	}
	/* sort by parents */
  	for (i = 0; i <nC-1; i++) 
	{
		gint k,j;
		k = i;
  		for (j = i+1; j <nC; j++) 
			if(parents[j]>parents[k]) k = j;
		if(k!=i)
		{
			gint t;
			t = oneConnectedAtoms[i];
			oneConnectedAtoms[i] = oneConnectedAtoms[k];
			oneConnectedAtoms[k] = t;
			t = parents[i];
			parents[i] = parents[k];
			parents[k] = t;
		}
	}
	nL = nLevels;
  	for (i = 0; i <nC; i++) 
	{
		gint j;
		if(i==0 || parents[i] != parents[i-1])
		{
			lists[nL] = g_list_append(lists[nL],GINT_TO_POINTER(-1)); /* the first is the grand parent */
			lists[nL] = g_list_append(lists[nL],GINT_TO_POINTER(parents[i])); /* the first is the parent */
			nL++;
		}
		j = oneConnectedAtoms[i];
		lists[nL-1] = g_list_append(lists[nL-1],GINT_TO_POINTER(j));
		levels[j] = nL;
	}
	nAll += nC;
	*nNewLevels = nL-nLevels;
	if(oneConnectedAtoms) g_free(oneConnectedAtoms);
	if(parents) g_free(parents);
	return nAll;
}
/************************************************************************/
static void initStack()
{
	gint i;
	if(Nat<1) return;
	if(inStack) g_free(inStack);
	inStack = g_malloc(Nat*sizeof(gboolean));
	for(i=0;i<Nat;i++) inStack[i] = FALSE;
}
/************************************************************************/
static void initRing(gint rS)
{
	done = FALSE;
	bonds = 0;
	ringSize = rS;
	initStack();
}
/************************************************************************/
static gboolean inRing(gint currentAtom, gint rootAtom, gint* levels)
{
	inStack[currentAtom] = TRUE;
	if (done) return TRUE;
	else if ( ( currentAtom == rootAtom ) && ( bonds == ringSize ) ) return TRUE;
	else if ( ( currentAtom == rootAtom ) && ( bonds > 2 ) && ( ringSize < 3 ) ) return TRUE;
	if ( bonds < ringSize )
	{
		gint i;
  		for (i = 0; i <(gint)Nat; i++) 
		{
			if(i==currentAtom) continue;
			if(levels[i]!=0)  continue;
			if(i==rootAtom && bonds<1)  continue;
			if(gxyz[currentAtom].typeConnections[i]<1)  continue;
			if ( ! ( inStack[i] ) )
			{
				bonds++;
				done = inRing(i, rootAtom,levels);
			}
			if (done) return TRUE;
		}
	}

	inStack[currentAtom] = FALSE;
	bonds--;
	return FALSE;
}
/************************************************************************/
static gint* getRingAtoms()
{
	gint i;
	gint n= 0;
 	gint* ringAtoms = NULL;
	gint k;
	gint j;
	if(inStack && ringSize>1)
	{
 		ringAtoms = g_malloc(ringSize*sizeof(gint));
		for(i=0;i<Nat;i++)
		{
			if(inStack[i])
			{
				ringAtoms[n] = i;
				n++;
				if(n>ringSize) break;
			}
		}
	}
	if(ringAtoms)
	{
		for(i=1;i<n;i++)
			if(ringAtoms[i]<ringAtoms[0])
			{
				gint t = ringAtoms[i];
				ringAtoms[i] = ringAtoms[0];
				ringAtoms[0] = t;
			}

		for(i=0;i<n-2;i++)
		{
			k = i+1;
			for(j=i+1;j<n;j++)
				if(gxyz[ringAtoms[i]].typeConnections[ringAtoms[j]]>0)
				{
					k = j;
					break;
				}
			if(k!=(i+1))
			{
				gint t = ringAtoms[i+1];
				ringAtoms[i+1] = ringAtoms[k];
				ringAtoms[k] = t;
			}
		}
	}
	return ringAtoms;
}
/********************************************************************************/
static gint* getListRingAtoms(gint i, gint j, gint* levels)
{

	gint* num  = NULL;
	gint n;
	gint nMax = 0;
	gint k;

	if(Nat <1) return NULL;
  	for (k = 0; k <(gint)Nat; k++) 
		if(levels[k]==0) nMax++;
	if(nMax <3) return NULL;

	for(n=2;n<nMax;n++)
	{
		initRing(n);
		if(inRing(i, j, levels) ) num = getRingAtoms();
		if(num) return num;
	}
	return num;
}
/*****************************************************************************/
static gint add_level_ring(gint* levels, GList** lists, gint nLevels)
{
	gint i;
	gint n = 0;
	gint nc = 0;
	gint nMax = 0;
	gint k;
	gint j;
	gint * numRing = NULL;

  	for (i = 0; i <(gint)Nat; i++) if(levels[i]==0) nMax++;
	for(n=2;n<nMax;n++)
  	for (i = 0; i <(gint)Nat; i++) 
	{
		if(levels[i]!=0) continue;
		nc = get_number_of_connections_to_upper_level(i, levels);
		if(nc==n)
		{
			gint parent = -1;
			j = -1;
  			for (k = 0; k <(gint)Nat; k++) 
				if(k !=i && levels[k]==0 && gxyz[i].typeConnections[k]>0) {j = k; break;}
			if(j<0) continue;
			
			gxyz[i].typeConnections[j]= 0;
			gxyz[j].typeConnections[i]= 0;
			numRing = getListRingAtoms( i,  j, levels);
			gxyz[i].typeConnections[j]= 1;
			gxyz[j].typeConnections[i]= 1;
			if(!numRing) continue;

			parent = -1;
  			for (k = 0; k <=ringSize; k++) 
			{
  				for (j = 0; j <Nat; j++) 
					if(levels[j]==0 && gxyz[numRing[k]].typeConnections[j]>0)
					{
						gint kp;
  						for (kp = 0; kp <=ringSize; kp++) 
							if(j==numRing[kp]) break;
						if(j==numRing[kp]) continue;
						parent = j;
						break;
					}
				if(parent>-1) 
				{
					/* the first son of ring is connected to parent */
					gint t = numRing[0];
					numRing[0] = numRing[k];
					numRing[k] = t;
					break;
				}
			}
  			for (k = 0; k <=ringSize; k++) 
				levels[numRing[k]] = nLevels+1;
			lists[nLevels] = g_list_append(lists[nLevels],GINT_TO_POINTER(-1)); /* the first is the grand parent */
			lists[nLevels] = g_list_append(lists[nLevels],GINT_TO_POINTER(parent)); /* the second is the parent */
  			for (k = 0; k <=ringSize; k++) 
				lists[nLevels] = g_list_append(lists[nLevels],GINT_TO_POINTER(numRing[k]));
			return ringSize+1;
		}
	}
	return 0;
}
/*****************************************************************************/
static GList* get_parent(GList* gp)
{
	if(gp) return gp->next;
	else return NULL;
}
/*****************************************************************************/
static GList* get_first_son(GList* gp)
{
	GList* p = get_parent(gp);
	if(p) return p->next;
	else return NULL;
}
/*****************************************************************************/
static gint build_parents(gint* levels, GList** lists, gint nLevels)
{
	gint i,j,k;
	GList* l = NULL;
	GList* p = NULL;
	gdouble d = -1;
	gint im = -1;
  	for (i = 0; i <nLevels; i++) 
	{
		p = get_parent(lists[i]);
		if(!p || GPOINTER_TO_INT(p->data)>-1) continue;

		l = p->next;
		if(!l) continue;
		j =GPOINTER_TO_INT(l->data);
		if(j<0 || j>Nat-1) continue;
		d = -1;
  		for (k = 0; k <Nat; k++) 
			if(k!=j && (levels[j]<levels[k]) && (d<0 || get_distance_xyz(k,j)<d)) 
			{
				d = get_distance_xyz(k,j);
				im = k;
			}
		p->data = GINT_TO_POINTER(im);
	}
	return 0;
}
/*****************************************************************************/
static gint build_grand_parents(gint* levels, GList** lists, gint nLevels)
{
	gint i,j;
	GList* p = NULL;
	GList* gp = NULL;
	gint lj = -1;
  	for (i = 0; i <nLevels; i++) 
	{
		gp = lists[i];
		if(!gp || GPOINTER_TO_INT(gp->data)>-1) continue;

		p = gp->next;
		if(!p || GPOINTER_TO_INT(p->data)<0) continue;
		j =GPOINTER_TO_INT(p->data);
		lj = levels[j];

		p = get_parent(lists[lj]);
		if(!p) continue;
		lists[i]->data = p->data;
	}
	return 0;
}
/*****************************************************************************/
static gint build_levels(gint* levels, GList** lists)
{
	gint nLevels = 0;
	gint i;
	gint n = 0;
	gint nAll = 0;

  	for (i = 0; i <(gint)Nat; i++) 
	{
		n = add_level_non_connected_atom(levels, lists, nLevels);
	}
  	for (i = 0; i <(gint)Nat; i++) 
	{
		gint nL;
		n = add_level_non_connected_atom(levels, lists, nLevels);
		nAll += n;
		if(n>0)
		{ 
			nLevels++; 
			if(nAll>=Nat) break;
			continue;
		}
		n = add_levels_connected_to_one_atom(levels, lists, nLevels, &nL);
		nAll += n;
		if(n>0)
		{ 
			nLevels+=nL; 
			if(nAll>=Nat) break;
			continue;
		}
		n = add_level_ring(levels, lists, nLevels);
		nAll += n;
		if(n==0)
		{
			printf(_("Sorry I cannot compute the levels of groupes\n"));
			return 0;
		}
		nLevels++; 
		if(nAll>=Nat) break;
	}
	/*
  	for (i = 0; i <nLevels; i++) 
	{
		GList* l;
		printf("ilevel = %d\n",i);
		for(l=lists[i];l!=NULL;l=l->next)
			printf("%d ",GPOINTER_TO_INT(l->data));
		printf("\n\n");
	}
	*/
	build_parents(levels, lists, nLevels);
	build_grand_parents(levels, lists, nLevels);
	/*
	printf("After build parents and grand parents\n");
  	for (i = 0; i <nLevels; i++) 
	{
		GList* l;
		printf("ilevel = %d\n",i);
		for(l=lists[i];l!=NULL;l=l->next)
			printf("%d ",GPOINTER_TO_INT(l->data));
		printf("\n\n");
	}
	*/
	return nLevels;
}
/*****************************************************************************/
static gint build_list_of_atoms_by_level(gint* levels, GList** lists, gint nLevels, gint* atomNumbers)
{
	gint i;
	GList* l = NULL;
	GList* son = NULL;
	gint nA = 0;
  	for (i = 0; i <Nat; i++)  atomNumbers[i] = 0;
  	for (i = nLevels-1; i>=0; i--) 
	{
		son = get_first_son(lists[i]);
		for(l=son;l!=NULL;l=l->next)
		{
			atomNumbers[nA]=GPOINTER_TO_INT(l->data);
			nA++;
		}
	}
	return nA;
}
/*****************************************************************************/
gint get_iop_nr(gint nL, gint I, gint* atomNumbersZ, GList** lists)
{
	gint Io = -1;
	gdouble d=-1;
	static gdouble precision = 1e-1;
	GList* l;
	for(l=get_first_son(lists[nL]);l!=NULL;l=l->next)
	{
		gint J = GPOINTER_TO_INT(l->data);
		if(J==I) continue;
		if(atomNumbersZ[J]>=atomNumbersZ[I]) continue;
		d =  get_distance_xyz(I,J);
		if(fabs(d)>precision)
		{
			Io = J;
			if(gxyz[I].typeConnections[J]>0) break;
		}
	}
	return Io;
}
/*****************************************************************************/
gint get_nr(gint i, gint* atomNumbers, gint* atomNumbersZ, gint* levels, GList** lists)
{
	gint I = atomNumbers[i];
	GList* p = get_parent(lists[levels[I]-1]);
	gint Ip = -1;

	if(p) Ip = GPOINTER_TO_INT(p->data);
	if(Ip<0) Ip = atomNumbers[i-1];
	if(atomNumbersZ[Ip]>=atomNumbersZ[I])
	{
		gint nL;
		for(nL=Nat-1;nL>levels[I]-1 ;nL--)
		{	
			Ip = get_iop_nr( nL, I,  atomNumbersZ, lists);
			if(Ip>-1) break;
		}
	}
	return Ip;
}
/*****************************************************************************/
gint get_iop_nangle(gint nL, gint I, gint IR, gint* atomNumbersZ, GList** lists)
{
	gint Io = -1;
	gdouble angle=180;
	static gdouble precision = 1e-1;
	GList* l;
	for(l=get_first_son(lists[nL]);l!=NULL;l=l->next)
	{
		gint J = GPOINTER_TO_INT(l->data);
		if(J==I) continue;
		if(J==IR) continue;
		if(atomNumbersZ[J]>=atomNumbersZ[I]) continue;
		angle =  get_angle_xyz(I,IR,J);
		if(fabs(angle-180)>precision &&fabs(angle)>precision)
		{
			Io = J;
			if(gxyz[IR].typeConnections[J]>0) break;
		}
	}
	return Io;
}
/*****************************************************************************/
gint get_nangle(gint i, gint* atomNumbers, gint* atomNumbersZ, gint* levels, GList** lists, gint IR)
{
	gint I = atomNumbers[i];
	gint Igp = -1;
	gint nL=levels[I]-1;

	Igp = get_iop_nangle( nL, I,  IR, atomNumbersZ, lists);

	nL=levels[IR]-1;
	if(Igp<0) Igp = get_iop_nangle( nL, I,  IR, atomNumbersZ, lists);
	if(Igp<0)
	for(nL=Nat-1;nL>levels[I]-1 ;nL--)
	{
		Igp = get_iop_nangle( nL, I,  IR, atomNumbersZ, lists);
		if(Igp>-1) break;
	}
	if(Igp<0)
	{
		GList* gp = lists[levels[I]-1];
		if(gp) Igp = GPOINTER_TO_INT(gp->data);
		if(Igp<0) Igp = atomNumbers[i-2];
		if(IR == Igp) Igp = (IR+1)%i;
		if(I == Igp) Igp = (I+1)%i;
	}
	return Igp;
}
/*****************************************************************************/
gint get_iop_ndihedral(gint nL, gint I, gint IR, gint IAngle, gint* atomNumbersZ, GList** lists)
{
	gint Io = -1;
	gdouble angle=180;
	static gdouble precision = 1e-1;
	GList* l;
	for(l=get_first_son(lists[nL]);l!=NULL;l=l->next)
	{
		gint J = GPOINTER_TO_INT(l->data);
		if(J==I) continue;
		if(J==IR) continue;
		if(J==IAngle) continue;
		if(atomNumbersZ[J]>=atomNumbersZ[I]) continue;
		angle =  get_angle_xyz(IR,IAngle,J);
		if(fabs(angle-180)>precision &&fabs(angle)>precision)
		{
			Io = J;
			if(gxyz[IAngle].typeConnections[J]>0) break;
		}
	}
	return Io;
}
/*****************************************************************************/
gint get_ndihedral(gint i, gint* atomNumbers, gint* atomNumbersZ, gint* levels, GList** lists, gint IR, gint IAngle)
{
	gint Id = -1;
	gint I = atomNumbers[i];
	gint nL=levels[IR]-1;
	Id = get_iop_ndihedral( nL, I, IR,  IAngle, atomNumbersZ, lists);
	if(Id<0)
	for(nL=Nat-1;nL>=0;nL--)
	{
		Id = get_iop_ndihedral( nL, I, IR,  IAngle, atomNumbersZ, lists);
		if(Id>-1) break;
	}
	if(Id<0)
	{
		if(IR!=atomNumbers[i-1]) Id = atomNumbers[i-1];
		else Id = atomNumbers[i-3];
	}
	return Id;
}
/*****************************************************************************/
gboolean xyz_to_zmat_using_connections()
{
	gint i;
	GeomAtomDef* Geomtemp = NULL;
	gint* levels = NULL;
	gint* atomNumbers = NULL;
	gint* atomNumbersZ = NULL;
	GList **lists=NULL;
	gint nLevels = 0;
	gint Ip,IpZ;
	gint Igp,IgpZ;
	gint Id,IdZ;
	gint nA;
	gint I;


	for(i=0;i<NcentersXYZ;i++)
	if(GeomXYZ[i].typeConnections==NULL)
	{
		reset_connections_XYZ();
		break;
	}

	if(NcentersXYZ<4) return xyz_to_zmat();

	set_gxyz();
	if (Nat <= 0) return FALSE ;
  	levels = g_malloc(Nat*sizeof(gint));
  	atomNumbers = g_malloc(Nat*sizeof(gint));
  	atomNumbersZ = g_malloc(Nat*sizeof(gint));
  	lists = g_malloc(Nat*sizeof(GList*));
  	for (i = 0; i <(gint)Nat; i++) levels[i] = 0;
  	for (i = 0; i <(gint)Nat; i++) lists[i] = NULL;
	nLevels = build_levels(levels, lists);
	/*
	printf("End build levels\n");
  	for (i = 0; i <(gint)Nat; i++) printf("%d\n",levels[i]);
	*/
	nA = build_list_of_atoms_by_level(levels, lists, nLevels, atomNumbers);
	/*
	printf("End list by level nA = %d\n",nA);
  	for (i = 0; i <(gint)Nat; i++) printf("%d numLevel = %d\n",atomNumbers[i],levels[atomNumbers[i]]);
	*/
	if(nA != Nat) return FALSE;
  	for (i = 0; i <(gint)Nat; i++) atomNumbersZ[atomNumbers[i]] = i;

	Geomtemp=g_malloc(Nat*sizeof(GeomAtomDef));
	for (i = 0; i <(gint)Nat; i++)
	{
		Geomtemp[i].Nentry = NUMBER_LIST_ZMATRIX;
		Geomtemp[i].Symb = NULL;
		Geomtemp[i].mmType = NULL;
		Geomtemp[i].pdbType = NULL;
		Geomtemp[i].Residue = NULL;
		Geomtemp[i].R = NULL; 
		Geomtemp[i].NR = NULL; 
		Geomtemp[i].Angle = NULL; 
		Geomtemp[i].NAngle = NULL; 
		Geomtemp[i].Dihedral = NULL; 
		Geomtemp[i].NDihedral = NULL; 
		Geomtemp[i].Charge = NULL;
		Geomtemp[i].Layer = NULL;
	}


	/* Atom #1 */
	I = atomNumbers[0];
	Geomtemp[0].Nentry = NUMBER_ENTRY_0;
	Geomtemp[0].Symb = g_strdup(gxyz[I].Symb);
	Geomtemp[0].mmType = g_strdup(gxyz[I].mmType);
	Geomtemp[0].pdbType = g_strdup(gxyz[I].pdbType);
	Geomtemp[0].Residue = g_strdup(gxyz[I].Residue);
	Geomtemp[0].Charge = g_strdup(gxyz[I].Charge);
	Geomtemp[0].Layer = g_strdup(gxyz[I].Layer);
  
	/* Atom #2 */
	I = atomNumbers[1];
	Geomtemp[1].Nentry =  NUMBER_ENTRY_R;
	Geomtemp[1].Symb = g_strdup(gxyz[I].Symb);
	Geomtemp[1].mmType = g_strdup(gxyz[I].mmType);
	Geomtemp[1].pdbType = g_strdup(gxyz[I].pdbType);
	Geomtemp[1].Residue = g_strdup(gxyz[I].Residue);
	Geomtemp[1].Charge = g_strdup(gxyz[I].Charge);
	Geomtemp[1].Layer = g_strdup(gxyz[I].Layer);
	Geomtemp[1].R = g_strdup_printf("%f",get_distance_xyz(atomNumbers[0],I));
	Geomtemp[1].NR = g_strdup_printf("%d",1);

	/* Atom #3 */
	I = atomNumbers[2];
	Ip = get_nr(2, atomNumbers, atomNumbersZ, levels, lists);
	IpZ = atomNumbersZ[Ip];

	Igp = get_nangle(2, atomNumbers, atomNumbersZ, levels, lists,Ip);
	IgpZ = atomNumbersZ[Igp];

	if(IpZ>Nat) printf("Erreur Ipz>Nat");
	if(IgpZ>Nat) printf("Erreur IgpZ>Nat");

	Geomtemp[2].Nentry =  NUMBER_ENTRY_ANGLE;
	Geomtemp[2].Symb = g_strdup(gxyz[I].Symb);
	Geomtemp[2].mmType = g_strdup(gxyz[I].mmType);
	Geomtemp[2].pdbType = g_strdup(gxyz[I].pdbType);
	Geomtemp[2].Residue = g_strdup(gxyz[I].Residue);
	Geomtemp[2].Charge = g_strdup(gxyz[I].Charge);
	Geomtemp[2].Layer = g_strdup(gxyz[I].Layer);
	Geomtemp[2].R = g_strdup_printf("%f",get_distance_xyz(I,Ip));
	Geomtemp[2].NR = g_strdup_printf("%d",IpZ+1);
	Geomtemp[2].Angle =  g_strdup_printf("%f",get_angle_xyz(I,Ip,Igp));
	Geomtemp[2].NAngle = g_strdup_printf("%d",IgpZ+1);
  
	for (i = 3; i <(gint)Nat; i++)
	{   
		I = atomNumbers[i];
		Ip = get_nr(i, atomNumbers, atomNumbersZ, levels, lists);
		IpZ = atomNumbersZ[Ip];

		Igp = get_nangle(i, atomNumbers, atomNumbersZ, levels, lists,Ip);
		IgpZ = atomNumbersZ[Igp];

		Id = get_ndihedral( i, atomNumbers, atomNumbersZ, levels, lists, Ip, Igp);
		IdZ = atomNumbersZ[Id];

		if(IdZ>Nat) printf("Erreur Idz>Nat");
		if(IpZ>Nat) printf("Erreur Ipz>Nat");
		if(IgpZ>Nat) printf("Erreur IgpZ>Nat");

		Geomtemp[i].Nentry =  NUMBER_ENTRY_DIHEDRAL;
		Geomtemp[i].Symb = g_strdup(gxyz[I].Symb);
		Geomtemp[i].mmType = g_strdup(gxyz[I].mmType);
		Geomtemp[i].pdbType = g_strdup(gxyz[I].pdbType);
		Geomtemp[i].Residue = g_strdup(gxyz[I].Residue);
		Geomtemp[i].Charge = g_strdup(gxyz[I].Charge);
		Geomtemp[i].Layer = g_strdup(gxyz[I].Layer);
		Geomtemp[i].R =  g_strdup_printf("%f",get_distance_xyz(Ip,I));
		Geomtemp[i].NR = g_strdup_printf("%d",IpZ+1);
		Geomtemp[i].Angle =  g_strdup_printf("%f",get_angle_xyz(I,Ip,Igp));
		Geomtemp[i].NAngle = g_strdup_printf("%d",IgpZ+1);
		Geomtemp[i].Dihedral =  g_strdup_printf("%f",get_dihedral_xyz(I,Ip,Igp,Id));
		Geomtemp[i].NDihedral = g_strdup_printf("%d",IdZ+1);
	}
	freeGeom();
	freeVariables();
	NcentersZmat = Nat;
	freeGeomXYZ();
	freeVariablesXYZ();
	Geom = Geomtemp;
	NVariables =0;
	Variables = NULL;
	for (i = 0; i <(gint)Nat; i++)
	  if(gxyz[atomNumbers[i]].Variable) set_variable_one_atom_in_GeomZMatrix(i);

	if(levels) g_free(levels);
	if(atomNumbers) g_free(atomNumbers);
	if(atomNumbersZ) g_free(atomNumbersZ);
	if(lists) g_free(lists);

	return TRUE;
}
/*****************************************************************************/
void conversion_xyz_to_zmat_using_connections()
{
	if(!xyz_to_zmat_using_connections())
	{
		Message(_("Sorry\nConversion is not possible"),_("Warning"),TRUE);
		return;
	}
 	MethodeGeom = GEOM_IS_ZMAT;
	rafresh_interface();
}
