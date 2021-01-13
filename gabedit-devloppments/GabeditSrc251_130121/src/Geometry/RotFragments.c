/* RotFragments.c */
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
#include <math.h>
#include "../Common/Global.h"
#include "../Utils/Matrix3D.h"
#include "../Utils/Constants.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Utils/AtomsProp.h"


/*****************************************************************/
static void GetATomCoords(gint n,GeomDef *geometry,gdouble C[])
{
		C[0] = geometry[n].X;
		C[1] = geometry[n].Y;
		C[2] = geometry[n].Z;
}
/*****************************************************************/
gdouble AngleToAtoms(gdouble*C3,gdouble* C1,gdouble* C2)
{
	gdouble x12, x32, y12, y32, z12, z32, l12, l32, dp;
	
        x12 = C1[ 0 ] - C2[ 0 ];
       	y12 = C1[ 1 ] - C2[ 1 ];
       	z12 = C1[ 2 ] - C2[ 2 ];
       	x32 = C3[ 0 ] - C2[ 0 ];
       	y32 = C3[ 1 ] - C2[ 1 ];
       	z32 = C3[ 2 ] - C2[ 2 ];

       	l12 = sqrt( x12 * x12 + y12 * y12 + z12 * z12 );
       	l32 = sqrt( x32 * x32 + y32 * y32 + z32 * z32 );
        if( l12 == 0.0 )
	{
               	return 0.0;
        }
        if( l32 == 0.0 )
	{
               	return 0.0;
       	}
        dp = ( x12 * x32 + y12 * y32 + z12 * z32 ) / (l12 * l32 );
	if ( dp < -1.0 )
		dp = -1.0;
	else if ( dp > 1.0 )
		dp = 1.0;
    return RAD_TO_DEG * acos(dp);
}
/*****************************************************************/
gdouble TorsionToAtoms(gdouble*C4,gdouble*C1,gdouble* C2,gdouble* C3)
{
	    gdouble   xij, yij, zij;
       	gdouble   xkj, ykj, zkj;
	    gdouble   xkl, ykl, zkl;
      	gdouble   dx, dy, dz;
        gdouble   gx, gy, gz;
        gdouble   bi, bk;
        gdouble   ct, d, ap, app, bibk;

        xij = C1[ 0 ] - C2[ 0 ];
        yij = C1[ 1 ] - C2[ 1 ];
        zij = C1[ 2 ] - C2[ 2 ];
        xkj = C3[ 0 ] - C2[ 0 ];
        ykj = C3[ 1 ] - C2[ 1 ];
        zkj = C3[ 2 ] - C2[ 2 ];
        xkl = C3[ 0 ] - C4[ 0 ];
        ykl = C3[ 1 ] - C4[ 1 ];
        zkl = C3[ 2 ] - C4[ 2 ];

        dx = yij * zkj - zij * ykj;
        dy = zij * xkj - xij * zkj;
        dz = xij * ykj - yij * xkj;
        gx = zkj * ykl - ykj * zkl;
        gy = xkj * zkl - zkj * xkl;
        gz = ykj * xkl - xkj * ykl;

        bi = dx * dx + dy * dy + dz * dz;
        bk = gx * gx + gy * gy + gz * gz;
        ct = dx * gx + dy * gy + dz * gz;
		bibk = bi * bk;
		
		if ( bibk < 1.0e-6 )	
			return 0;
        
		ct = ct / sqrt( bibk );
        
		if( ct < -1.0 )
                ct = -1.0;
        else if( ct > 1.0 )
                ct = 1.0;

        ap = acos( ct );
        
		d  = xkj*(dz*gy-dy*gz) + ykj*(dx*gz-dz*gx) + zkj*(dy*gx-dx*gy);
        
		if( d < 0.0 )
                ap = -ap;
        
		ap = PI - ap;
       	app = 180.0 * ap / PI;
       	if( app > 180.0 )
               	app = app - 360.0;
        return( app );
}
/*****************************************************************/
gdouble AngleAboutXAxis(gdouble* coord)
{
		gdouble distance = sqrt( coord[ 0 ] * coord[ 0 ] +
			coord[ 1 ] * coord[ 1 ] + coord[ 2 ] * coord[ 2 ] );
		gdouble yComponent = coord[ 1 ] / distance;
		gdouble zComponent = coord[ 2 ] / distance;
		gdouble theta = 0.0;
		if ( ( zComponent == 0 ) && ( yComponent > 0 ) )
			theta = 90.0;
		else if ( ( zComponent == 0 ) && ( yComponent < 0 ) )
			theta = 270.0;
		else if ( ( yComponent == 0 ) && ( zComponent > 0 ) )
			theta = 180.0;
		else if ( ( yComponent == 0 ) && ( zComponent <= 0 ) )
			theta = 0.0;
		else{ 
			theta = atan( fabs( yComponent / zComponent ) ) * RAD_TO_DEG;
			if ( ( yComponent > 0 ) && ( zComponent > 0 ) )
				theta = 90.0 + atan( fabs( zComponent / 
					yComponent ) ) * RAD_TO_DEG;
			else if ( ( yComponent < 0 ) && ( zComponent > 0 ) )
				theta += 180.0;
			else if ( ( yComponent < 0 ) && ( zComponent < 0 ) )
				theta = 270.0 + atan( fabs( zComponent / 
					yComponent ) ) * RAD_TO_DEG;
		}

	return( theta );
}
/*****************************************************************/
gdouble AngleAboutYAxis(gdouble* coord)
{
		gdouble distance = sqrt( coord[ 0 ] * coord[ 0 ] +
			coord[ 1 ] * coord[ 1 ] + coord[ 2 ] * coord[ 2 ] );
		gdouble xComponent = coord[ 0 ] / distance;
		gdouble zComponent = coord[ 2 ] / distance;
		gdouble theta = 0.0;
		if ( ( zComponent == 0 ) && ( xComponent > 0 ) )
			theta = 180.0;
		else if ( ( zComponent == 0 ) && ( xComponent <= 0 ) )
			theta = 0.0;
		else if ( ( xComponent == 0 ) && ( zComponent > 0 ) )
			theta = 90.0;
		else if ( ( xComponent == 0 ) && ( zComponent < 0 ) )
			theta = 270.0;
		else{ 
			theta = atan( fabs( zComponent / xComponent ) ) * RAD_TO_DEG;
			if ( ( zComponent > 0 ) && ( xComponent > 0 ) )
				theta = 90.0 + atan( fabs( xComponent / 
					zComponent ) ) * RAD_TO_DEG;
			else if ( ( zComponent < 0 ) && ( xComponent > 0 ) )
				theta += 180.0;
			else if ( ( zComponent < 0 ) && ( xComponent < 0 ) )
				theta = 270.0 + atan( fabs( xComponent / 
					zComponent ) ) * RAD_TO_DEG;
		}
	return( theta );
}
/*****************************************************************/
gdouble AngleAboutZAxis(gdouble* coord)
{
		double distance = sqrt( coord[ 0 ] * coord[ 0 ] +
			coord[ 1 ] * coord[ 1 ] + coord[ 2 ] * coord[ 2 ] );
		double xComponent = coord[ 0 ] / distance;
		double yComponent = coord[ 1 ] / distance;
		double theta = 0.0;
		if ( ( xComponent == 0 ) && ( yComponent > 0 ) )
			theta = 90.0;
		else if ( ( xComponent == 0 ) && ( yComponent < 0 ) )
			theta = 270.0;
		else if ( ( yComponent == 0 ) && ( xComponent >= 0 ) )
			theta = 0.0;
		else if ( ( yComponent == 0 ) && ( xComponent < 0 ) )
			theta = 180.0;
		else{ 
			theta = atan( fabs( yComponent / xComponent ) ) * RAD_TO_DEG;
			if ( ( yComponent > 0 ) && ( xComponent < 0 ) )
				theta = 90.0 + atan( fabs( xComponent / 
					yComponent ) ) * RAD_TO_DEG;
			else if ( ( yComponent < 0 ) && ( xComponent < 0 ) )
				theta += 180.0;
			else if ( ( yComponent < 0 ) && ( xComponent > 0 ) )
				theta = 270.0 + atan( fabs( xComponent / 
					yComponent ) ) * RAD_TO_DEG;
		}

	return( theta );
}

/********************************************************************************/
void SetBondDistance(GeomDef* geom,gint n1,gint n2,gdouble bondLength,gint list[],gint nlist)
{
	gdouble a1[]={geom[n1].X,geom[n1].Y,geom[n1].Z};
	gdouble a2[]={geom[n2].X,geom[n2].Y,geom[n2].Z};
	gdouble coord[3];
	gint i;
	gint atomNumber;
	gdouble len = 0;

	bondLength *= ANG_TO_BOHR;

	for(i=0;i<3;i++)
	 	coord[i] = a1[i] - a2[i];
	for(i=0;i<3;i++)
		len += coord[i]*coord[i];

	if(len>1e-3)
	{
		len = 1-bondLength/sqrt(len);
		for(i=0;i<3;i++)
	 		coord[i] *=  len;
	}
	else
		coord[0] -= bondLength;

	for ( i = 0; i < nlist; i++ )
	{
		atomNumber = list[ i ];	
		if ( ( atomNumber == n1 ) || ( atomNumber == n2 ) )
			continue;
		geom[atomNumber].X += coord[0];
		geom[atomNumber].Y += coord[1];
		geom[atomNumber].Z += coord[2];
	} 
	atomNumber = n2;	
	geom[atomNumber].X += coord[0];
	geom[atomNumber].Y += coord[1];
	geom[atomNumber].Z += coord[2];
}
/********************************************************************************/
void SetBondDistanceX(GeomDef* geom,gint n1,gint n2,gdouble bondLength,gint list[],gint nlist)
{
	gdouble a1[]={geom[n1].X,geom[n1].Y,geom[n1].Z};
	gdouble a2[]={geom[n2].X,geom[n2].Y,geom[n2].Z};
	gdouble coord[3];
	gint i;
	gint atomNumber;

	bondLength *= ANG_TO_BOHR;

	for(i=0;i<3;i++)
	 	coord[i] = a1[i] - a2[i];

	coord[0] -= bondLength;

	for ( i = 0; i < nlist; i++ )
	{
		atomNumber = list[ i ];	
		if ( ( atomNumber == n1 ) || ( atomNumber == n2 ) )
			continue;
		geom[atomNumber].X += coord[0];
		geom[atomNumber].Y += coord[1];
		geom[atomNumber].Z += coord[2];
	} 
	atomNumber = n2;	
	geom[atomNumber].X += coord[0];
	geom[atomNumber].Y += coord[1];
	geom[atomNumber].Z += coord[2];
}
/*****************************************************************/
void SetAngle(gint Natoms,GeomDef *geometry, gint a1num, gint a2num, gint a3num, gdouble angle,gint atomList[], gint numberOfElements )
{
		gdouble a1[3];
		gdouble a2[3];
		gdouble a3[3];
		gint i;
		gint j;
		gdouble** pseudoAtom =g_malloc(sizeof(gdouble*));
		gdouble originalAngle = 0;
		gdouble** tmp = NULL;
		gdouble** M=g_malloc(3*sizeof(gdouble*));/* Matrix 3D */
	
		gint numberOfCoordinates = 0;
		gint atomListLength = 0;
		gint atomNumber;
        gdouble a1YAngle = 0;
        gdouble a1ZAngle = 0;
		gdouble a3XAngle = 0;


		GetATomCoords(a1num,geometry,a1);
		GetATomCoords(a2num,geometry,a2);
		GetATomCoords(a3num,geometry,a3);

		
		originalAngle = AngleToAtoms(a3,a1,a2);

			
		pseudoAtom[0] = g_malloc(3*sizeof(gdouble));
		
		for(j=0;j<3;j++)
			pseudoAtom[0][j] = a1[j] - a2[j];

		if ( numberOfElements == 0 ){
			atomListLength = 0;
			numberOfCoordinates = 1;
		}
		else{
			atomListLength = numberOfElements;
			numberOfCoordinates = atomListLength + 1;
		}
		
	
		for(j=0;j<3;j++)
			M[j] = g_malloc(4*sizeof(gdouble));

		tmp = g_malloc(numberOfCoordinates*sizeof(gdouble*));
		for(i=0;i<numberOfCoordinates;i++)
			tmp[i] = g_malloc(3*sizeof(gdouble));


		/* store atom 3 as first entry in array*/
		for(j=0;j<3;j++)
			tmp[0][j] = a3[j] - a2[j];

		/* translate all other atoms by -a2*/
		for (i = 0; i < atomListLength; i++ )
		{
			atomNumber = atomList[ i ];	

			for(j=0;j<3;j++)
				tmp[i+1][j] = 0;

			if ( ( atomNumber == a1num ) || ( atomNumber == a2num ) || 
				 ( atomNumber == a3num ) )
				continue;

			
			tmp[i+1][0] = geometry[atomNumber].X- a2[ 0 ];
			tmp[i+1][1] = geometry[atomNumber].Y- a2[ 1 ];
			tmp[i+1][2] = geometry[atomNumber].Z- a2[ 2 ];
		} 


		/* rotate a1 to 1,0,0*/
		UnitMat3D(M);
        	a1YAngle = AngleAboutYAxis(pseudoAtom[0]);
		RotMat3D(M, -a1YAngle ,1);
		TransformMat3D(M,pseudoAtom,1);	               	

        	a1ZAngle = AngleAboutZAxis(pseudoAtom[0]);
		UnitMat3D(M);
		RotMat3D(M, 180-a1ZAngle ,2);
		TransformMat3D(M,pseudoAtom,1);



		UnitMat3D(M);
		RotMat3D(M, -a1YAngle ,1);
		TransformMat3D(M,tmp,numberOfCoordinates);

		UnitMat3D(M);
		RotMat3D(M, 180-a1ZAngle ,2);
		TransformMat3D(M,tmp,numberOfCoordinates);	               	

		a3XAngle = AngleAboutXAxis(tmp[0]);

		UnitMat3D(M);
		RotMat3D(M, a3XAngle,0);
		TransformMat3D(M,tmp,numberOfCoordinates);	               	

		UnitMat3D(M);
		RotMat3D(M,  originalAngle - angle,1);
		TransformMat3D(M,tmp,numberOfCoordinates);	               	

		UnitMat3D(M);
		RotMat3D(M,  a1ZAngle - 180.0,2);
		TransformMat3D(M,tmp,numberOfCoordinates);	               	

		UnitMat3D(M);
		RotMat3D(M, a1YAngle ,1);
		TransformMat3D(M,tmp,numberOfCoordinates);	               	
		
		UnitMat3D(M);
		TranslatMat3D(M,a2[0],a2[1],a2[2]);
		TransformMat3D(M,tmp,numberOfCoordinates);

		
		for (i = 0; i < atomListLength; i++ )
		{
			atomNumber = atomList[ i ];	
			if ( ( atomNumber == a1num ) || ( atomNumber == a2num ) || 
				( atomNumber == a3num ) )
				continue;

			geometry[atomNumber].X = tmp[i+1][0];
			geometry[atomNumber].Y = tmp[i+1][1];
			geometry[atomNumber].Z = tmp[i+1][2];
		
		}

		geometry[a3num].X = tmp[0][0];
		geometry[a3num].Y = tmp[0][1];
		geometry[a3num].Z = tmp[0][2];
		/* Free M */
		for(i=0;i<3;i++)
			g_free(M[i]);
		g_free(M);

		/* Free tmp */
		for(i=0;i<numberOfCoordinates;i++)
			g_free(tmp[i]);
		g_free(tmp);
		/* Free pseudo atom */
		g_free(pseudoAtom[0]);
		g_free(pseudoAtom);

}
/*****************************************************************/
void SetTorsion(gint Natoms,GeomDef *geometry, gint a1num, gint a2num, gint a3num, gint a4num, gdouble torsion,gint atomList[], gint numberOfElements )
{
		gdouble a1[3];
		gdouble a2[3];
		gdouble a3[3];
		gdouble a4[3];
		gint i;
		gint j;
		gdouble** pseudoAtom2 =g_malloc(sizeof(gdouble*));
		gdouble originalTorsion = 0;
		gdouble** tmp = NULL;
		gdouble** M=g_malloc(3*sizeof(gdouble*));/* Matrix 3D */
	
		gint numberOfCoordinates = 0;
		gint atomListLength = 0;
		gint atomNumber;
        gdouble a2YAngle = 0;
        gdouble a2ZAngle = 0;


		GetATomCoords(a1num,geometry,a1);
		GetATomCoords(a2num,geometry,a2);
		GetATomCoords(a3num,geometry,a3);
		GetATomCoords(a4num,geometry,a4);
		originalTorsion = TorsionToAtoms(a4, a1, a2, a3 );
		pseudoAtom2[0] = g_malloc(3*sizeof(gdouble));
		
		for(i=0;i<3;i++)
			pseudoAtom2[0][i] = a2[i] - a3[i];

		if ( numberOfElements == 0 ){
			atomListLength = 0;
			numberOfCoordinates = 1;
		}
		else{
			atomListLength = numberOfElements;
			numberOfCoordinates = atomListLength + 1;
		}
		
		tmp = g_malloc(numberOfCoordinates*sizeof(gdouble*));
	
		for(i=0;i<3;i++)
			M[i] = g_malloc(4*sizeof(gdouble));
	
		for(i=0;i<numberOfCoordinates;i++)
			tmp[i] = g_malloc(3*sizeof(gdouble));

		/* array of atom numbers to be rotated*/

		/* store atom 4 as first entry in array*/
		tmp[0][0] = a4[ 0 ] - a3[ 0 ];
		tmp[0][1] = a4[ 1 ] - a3[ 1 ];
		tmp[0][2] = a4[ 2 ] - a3[ 2 ];
		/* translate all other atoms by -a3*/
		for (i = 0; i < atomListLength; i++ )
		{
			atomNumber = atomList[ i ];
			for(j=0;j<3;j++)
				tmp[i+1][j] = 0;
			
			if ( ( atomNumber == a1num ) || ( atomNumber == a2num ) || 
				 ( atomNumber == a3num ) || ( atomNumber == a4num ) )
				continue;
			tmp[i+1][0] = geometry[atomNumber].X- a3[ 0 ];
			tmp[i+1][1] = geometry[atomNumber].Y- a3[ 1 ];
			tmp[i+1][2] = geometry[atomNumber].Z- a3[ 2 ];
		} /* get angle info from atom2*/

		UnitMat3D(M);
		a2YAngle = AngleAboutYAxis(pseudoAtom2[0]);
		RotMat3D(M, -a2YAngle ,1);
		TransformMat3D(M,pseudoAtom2,1);	               	

		UnitMat3D(M);
		a2ZAngle = AngleAboutZAxis(pseudoAtom2[0]);

		UnitMat3D(M);
		RotMat3D(M, -a2YAngle ,1);
		TransformMat3D(M,tmp,numberOfCoordinates);	               	

		UnitMat3D(M);
		RotMat3D(M, 180-a2ZAngle ,2);
		TransformMat3D(M,tmp,numberOfCoordinates);	               	

		UnitMat3D(M);
		RotMat3D(M,  originalTorsion - torsion  ,0);
		TransformMat3D(M,tmp,numberOfCoordinates);	               	

		UnitMat3D(M);
		RotMat3D(M,  a2ZAngle - 180.0,2);
		TransformMat3D(M,tmp,numberOfCoordinates);	               	

		UnitMat3D(M);
		RotMat3D(M, a2YAngle ,1);
		TransformMat3D(M,tmp,numberOfCoordinates);	               	
		
		UnitMat3D(M);
		TranslatMat3D(M,a3[0],a3[1],a3[2]);
		TransformMat3D(M,tmp,numberOfCoordinates);
		
		for (i = 0; i < atomListLength; i++ )
		{
			atomNumber = atomList[ i ];	
			if ( ( atomNumber == a1num ) || ( atomNumber == a2num ) || 
				( atomNumber == a3num ) || ( atomNumber == a4num ) )
				continue;

			geometry[atomNumber].X = tmp[i+1][0];
			geometry[atomNumber].Y = tmp[i+1][1];
			geometry[atomNumber].Z = tmp[i+1][2];
		
		}
		geometry[a4num].X = tmp[0][0];
		geometry[a4num].Y = tmp[0][1];
		geometry[a4num].Z = tmp[0][2];
		/* Free M */
		for(i=0;i<3;i++)
			g_free(M[i]);
		g_free(M);

		/* Free tmp */
		for(i=0;i<numberOfCoordinates;i++)
			g_free(tmp[i]);
		g_free(tmp);
		/* Free pseudo atom */
		g_free(pseudoAtom2[0]);
		g_free(pseudoAtom2);

}
/*****************************************************************/
void SetOposedBondeds(gint Natoms,GeomDef *geometry, gint ns, gint toBond,gint atomList[], gint numberOfElements )
{
		gint i;
		gint j;
		gint k;
		GeomDef *tmpGeom = NULL;
		gdouble V[3];
		gdouble norm;

        	if(!geometry[ns].typeConnections) return;
		if(Natoms<2) return;

		V[0] = V[1] = V[2] = 0;
		for(j=0;j<Natoms;j++)
		{
			gboolean ok = FALSE;
			for(k=0;k<numberOfElements;k++)
			{
				if(atomList[k]==j) 
				{
					ok = TRUE;
					break;
				}
			}
			if(ok) continue;

        		if(geometry[ns].typeConnections[geometry[j].N-1]>0)
			{
				V[0]+=geometry[ns].X-geometry[j].X;
				V[1]+=geometry[ns].Y-geometry[j].Y;
				V[2]+=geometry[ns].Z-geometry[j].Z;
			}
		}
		norm =  V[0]* V[0]+ V[1]* V[1]+V[2]* V[2];
		if(norm<1e-8) return;
		for(k=0;k<3;k++) V[k]/= sqrt(norm);
		tmpGeom =g_malloc((Natoms+1)*sizeof(GeomDef));
		for(i=0;i<Natoms;i++)
		{
			tmpGeom[i].X = geometry[i].X;
			tmpGeom[i].Y = geometry[i].Y;
			tmpGeom[i].Z = geometry[i].Z; 
			tmpGeom[i].N = geometry[i].N; 
        		tmpGeom[i].typeConnections = NULL;
			tmpGeom[i].Prop = prop_atom_get(geometry[i].Prop.symbol);
			tmpGeom[i].mmType = g_strdup(geometry[i].mmType);
			tmpGeom[i].pdbType = g_strdup(geometry[i].pdbType);
			tmpGeom[i].Layer = geometry[i].Layer;
			tmpGeom[i].Variable = geometry[i].Variable;
			tmpGeom[i].Residue = g_strdup(geometry[i].Residue);
			tmpGeom[i].ResidueNumber = geometry[i].ResidueNumber;
			tmpGeom[i].show = geometry[i].show;
			tmpGeom[i].Charge = geometry[i].Charge;
		}	 
		/* pseudo atom */
		i = Natoms;
		tmpGeom[i].X = V[0]+geometry[ns].X;
		tmpGeom[i].Y = V[1]+geometry[ns].Y;
		tmpGeom[i].Z = V[2]+geometry[ns].Z;
		tmpGeom[i].N = Natoms;
        	tmpGeom[i].typeConnections = NULL;
		tmpGeom[i].Prop = prop_atom_get("H");
		tmpGeom[i].mmType = g_strdup("H");
		tmpGeom[i].pdbType = g_strdup("H");
		tmpGeom[i].Layer = geometry[i-1].Layer;
		tmpGeom[i].Variable = geometry[i-1].Variable;
		tmpGeom[i].Residue = g_strdup("H");
		tmpGeom[i].ResidueNumber = geometry[i-1].ResidueNumber;
		tmpGeom[i].show = geometry[i-1].show;
		SetAngle(Natoms+1,tmpGeom,ns,Natoms, toBond,180.0,atomList, numberOfElements);
		for(i=0;i<Natoms;i++)
		{
			geometry[i].X = tmpGeom[i].X;
			geometry[i].Y = tmpGeom[i].Y;
			geometry[i].Z = tmpGeom[i].Z;
		}

		Free_One_Geom(tmpGeom,Natoms+1);
}
