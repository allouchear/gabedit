/* CalculTypesAmber.c */
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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "../Common/Global.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/Fragments.h"
#include "../Utils/Transformation.h"
#include "../Utils/Constants.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Vector3d.h"
#include "../Utils/HydrogenBond.h"
#include "../Utils/AtomsProp.h"
#include "../MolecularMechanics/CalculTypesAmber.h"
/**********************************************************************************************************/
typedef struct _Molecule Molecule;
struct _Molecule
{
	gint numberOfAtoms;
	GeomDef* geom;
	GList* bondsList;
	gint* numberOfDoubleBonds;
	gint* numberOfTripleBonds;
	gint **connected;
};
static gboolean *inStack = NULL;
static gboolean doneRing = FALSE;
static gint nBondsRing = 0;
/************************************************************************/
static gboolean bonded(GeomDef* geom, gint i,gint j)
{
	gdouble distance;
	V3d dif;
	
	dif[0] = geom[i].X - geom[j].X;
	dif[1] = geom[i].Y - geom[j].Y;
	dif[2] = geom[i].Z - geom[j].Z;

	distance = v3d_length(dif);

	if(distance<(geom[i].Prop.covalentRadii + geom[j].Prop.covalentRadii)) return TRUE;
	else return FALSE;
}
/************************************************************************/
static gboolean hbonded(GeomDef* geom, gint nAtoms, gint i,gint j)
{
	gdouble minDistanceH;
	gdouble maxDistanceH;
	gdouble minDistanceH2;
	gdouble maxDistanceH2;
	gdouble minAngleH;
	gdouble maxAngleH;
	gdouble distance2;
	gdouble angle;
	gchar* strAngle;
	Point A;
	Point B;
	gdouble dx, dy, dz;

	gint k;
	gint kH;
	gint kO;

	if(!strcmp(geom[i].Prop.symbol,"H"))
	{
		kH = i;
		kO = j;
		if(!atomCanDoHydrogenBond(geom[j].Prop.symbol)) return FALSE;
	}
	else
	{
		if(!strcmp(geom[j].Prop.symbol,"H"))
		{
			kH = j;
			kO = i;
			if(!atomCanDoHydrogenBond(geom[i].Prop.symbol)) return FALSE;
		}
		else return FALSE;
	}
	minDistanceH = getMinDistanceHBonds();
	minDistanceH2 = minDistanceH*minDistanceH*ANG_TO_BOHR*ANG_TO_BOHR;

	maxDistanceH = getMaxDistanceHBonds();
	maxDistanceH2 = maxDistanceH*maxDistanceH*ANG_TO_BOHR*ANG_TO_BOHR;

	minAngleH = getMinAngleHBonds();
	maxAngleH = getMaxAngleHBonds();

	dx = geom[i].X - geom[j].X;
	dy = geom[i].Y - geom[j].Y;
	dz = geom[i].Z - geom[j].Z;
	distance2 = (dx*dx+dy*dy+dz*dz);
	if(distance2<minDistanceH2 || distance2>maxDistanceH2) return FALSE;

	for(k=0;k<nAtoms;k++)
	{
		if(k==kH) continue;
		if(k==kO) continue;
		/* angle kO, kH, connection to kH */
		if(!bonded(geom, kH,k)) continue;
		A.C[0]=geom[kO].X-geom[kH].X;
		A.C[1]=geom[kO].Y-geom[kH].Y;
		A.C[2]=geom[kO].Z-geom[kH].Z;

		B.C[0]=geom[k].X-geom[kH].X;
		B.C[1]=geom[k].Y-geom[kH].Y;
		B.C[2]=geom[k].Z-geom[kH].Z;
        
        	strAngle = get_angle_vectors(A,B);
		angle = atof(strAngle);
		if(strAngle) g_free(strAngle);
		if(angle>=minAngleH &&angle<=maxAngleH) return TRUE;
	}
	return FALSE;
}
/************************************************************************/
static void freeBonds(GList* bondsList)
{
	GList* list;
	if(!bondsList) return;
	for(list=bondsList;list!=NULL;list=list->next)
	{
		if(list->data) g_free(list->data);
	}
	g_list_free(bondsList);
	bondsList = NULL;
}
/************************************************************************/
static void setMultipleBonds(Molecule* m, gint* nBonds)
{
	GList* list;
	GList* bondsList = m->bondsList;
	GeomDef* geom = m->geom;

	if(!bondsList) return;
	for(list=bondsList;list!=NULL;list=list->next)
	{
		BondType* data=(BondType*)list->data;
		gint i = data->n1;
		gint j = data->n2;
		if(data->bondType == GABEDIT_BONDTYPE_HYDROGEN) continue;
		if(
		 nBonds[i] < geom[i].Prop.maximumBondValence -1 &&
		 nBonds[j] < geom[j].Prop.maximumBondValence -1 
		)
		{
			data->bondType = GABEDIT_BONDTYPE_TRIPLE;
			nBonds[i] += 2;
			nBonds[j] += 2;
		}
		else
		if(
		 nBonds[i] < geom[i].Prop.maximumBondValence &&
		 nBonds[j] < geom[j].Prop.maximumBondValence 
		)
		{
			data->bondType = GABEDIT_BONDTYPE_DOUBLE;
			nBonds[i] += 1;
			nBonds[j] += 1;
		}
	}
}
/************************************************************************/
static gboolean buildBonds(Molecule* m, gboolean buildHbonds)
{
	gint i;
	gint j;
	GeomDef* geom = m->geom;
	gint nAtoms = m->numberOfAtoms;
	gint* nBonds = NULL;
	m->bondsList = NULL;
	if(nAtoms<1) return FALSE;
	nBonds =g_malloc(nAtoms*sizeof(gint));
	for(i = 0;i<nAtoms;i++) nBonds[i] = 0;
	for(i = 0;i<nAtoms;i++)
	{
		for(j=i+1;j<nAtoms;j++)
			if(bonded(geom, i,j))
			{
				BondType* A=g_malloc(sizeof(BondType));
				A->n1 = i;
				A->n2 = j;
				nBonds[i]++;
				nBonds[j]++;
				A->bondType = GABEDIT_BONDTYPE_SINGLE;
				m->bondsList = g_list_append(m->bondsList,A);
			}
		        else
			if(buildHbonds && hbonded(geom, nAtoms, i,j))
			{
				BondType* A=g_malloc(sizeof(BondType));
				A->n1 = i;
				A->n2 = j;
				A->bondType = GABEDIT_BONDTYPE_HYDROGEN;
				m->bondsList = g_list_append(m->bondsList,A);
			}
	  }
	setMultipleBonds(m, nBonds);
	g_free(nBonds);
	nBonds = NULL;
	return TRUE;
}
/************************************************************************/
static void freeStack()
{
	if(inStack) g_free(inStack);
	inStack = NULL;
}
/************************************************************************/
static void initStack(gint nAtoms)
{
	gint i;
	if(inStack) freeStack();
	inStack = g_malloc(nAtoms*sizeof(gboolean));
	for(i=0;i<nAtoms;i++) inStack[i] = FALSE;
}
/************************************************************************/
static void freeConnections(Molecule* m)
{
	gint i;
	if(!m->connected) return;
	for(i=0;i< m->numberOfAtoms;i++)
	{
		if(m->connected[i]) g_free(m->connected[i]);
	}
	g_free(m->connected);
	m->connected = NULL;
}
/************************************************************************/
/*
static void printConnections(Molecule* m)
{
	gint nAtoms = m->numberOfAtoms;
	gint i = 0;
	gint j = 0;
	gint** connected = m->connected;
	for(i=0;i<nAtoms;i++)
	{
		printf("Nc = %d : ", i+1);
		for(j=0;j<connected[i][0];j++)
			printf(" %d ",connected[i][j+1]+1);
		printf("\n");
	}
}
*/
/************************************************************************/
static void buildConnections(Molecule* m)
{
	GList* list;
	gint nAtoms = m->numberOfAtoms;
	GList* bondsList = m->bondsList;
	gint i = 0;
	gint k = 0;
	gint** connected = NULL;
	if(!bondsList) return;
	connected = g_malloc(nAtoms*sizeof(gint*));
	for(i=0;i<nAtoms;i++)
	{
		connected[i] = g_malloc((nAtoms+1)*sizeof(gint));
		connected[i][0] = 0;
	}
	for(list=bondsList;list!=NULL;list=list->next)
	{
		BondType* data=(BondType*)list->data;
		gint i = data->n1;
		gint j = data->n2;

		connected[i][0]++;
		connected[j][0]++;

		k = connected[i][0];
		connected[i][k]=j;

		k = connected[j][0];
		connected[j][k]=i;

	}
	m->connected = connected;
}
/************************************************************************/
static gboolean inRingRecursive(Molecule* m, gint currentAtom, gint rootAtom, gint ringSize, gboolean begin)
{
	gint i;
	gint** connected = m->connected;

	if(!begin) inStack[currentAtom] = TRUE;

	if (doneRing) return TRUE;
	else if ( ( currentAtom == rootAtom ) && ( nBondsRing == ringSize ) ) return TRUE;
	else if ( ( currentAtom == rootAtom ) && ( nBondsRing > 2 ) && ( ringSize < 3 ) ) return TRUE;
	if ( nBondsRing < ringSize )
	{
		gint numberOfConnections = connected[ currentAtom ][ 0 ];
		for (i = 1; i <= numberOfConnections; i++ )
		{
			gint newAtom = connected[currentAtom][i];
			if ( ! ( inStack[newAtom] ) )
			{
				nBondsRing++;
				doneRing = inRingRecursive( m, newAtom, rootAtom, ringSize, FALSE);
			}
			if (doneRing) return TRUE;
		}
	}
	inStack[currentAtom] = FALSE;
	nBondsRing--;
	return FALSE;
}
/************************************************************************/
static gboolean atomInRing(Molecule* m, gint numAtom, gint ringSize)
{
	doneRing = FALSE;
	nBondsRing = 0;
	initStack(m->numberOfAtoms);
	if(!m->connected) return FALSE;
	return inRingRecursive( m, numAtom,  numAtom, ringSize, TRUE);
}
/************************************************************************/
static gboolean allCarbon(Molecule* m)
{
	GeomDef* geom = m->geom;
	gint i;
	for(i=0;i<m->numberOfAtoms;i++)
		if(inStack[i] && strcmp(geom[i].Prop.symbol,"C" )) return FALSE;
	return TRUE;
}
/************************************************************************/
static gchar* subString(gchar* str, gint begin, gint end)
{
	gchar* res = NULL;
	gint i;
	gint l = strlen(str);
	gint l2 = 0;
	if(l<begin) return NULL;
	if(l<end) end = l;
	if(end<0) end = l;
	l2 = end - begin + 1;
	res= g_malloc((l2+1)*sizeof(gchar));
	for(i=0;i<l2;i++) res[i] = str[i+begin];
	res[l2] = '\0';
	return res;
}
/************************************************************************/
static gchar getCharFromString(gchar* str, gint index)
{
	gint l = 0;
	if(!str) return '\0';
	l = strlen(str);
	if(l<=index) return '\0';
	return str[index];
}
/************************************************************************/
static gboolean bondedType(Molecule* m, gint atom1, gint atom2, GabEditBondType type)
{
	GList* list;
	GList* bondsList= m->bondsList;
	if(!bondsList) return FALSE;
	for(list=bondsList;list!=NULL;list=list->next)
	{
		BondType* data=(BondType*)list->data;
		gint i = data->n1;
		gint j = data->n2;
		if( (i== atom1 && j == atom2 ) || (j== atom1 && i == atom2 ) )
		{
			if(data->bondType == type ) return TRUE;
		}
	}
	return FALSE;
}
/************************************************************************/
static gint numberOfConnectedTypes( Molecule* m, gint atomA, gchar* name )
{
	GeomDef* geom = m->geom;
	gint numberOfConnections = m->connected[ atomA ][ 0 ];
	gint numberMatching = 0;
	gint i;
	if(!name) return 0;
	if (!strcmp(name,"*")) return numberOfConnections;
	for ( i = 1; i <= numberOfConnections; i++ )
	{
		gint atomB = m->connected[ atomA ][ i ];
		if ( !strcmp(geom[atomB].Prop.symbol,name ))
			numberMatching++;
	}
	return( numberMatching );
}
/************************************************************************/
gint sp(Molecule*m, gint atomNumber)
{
	gint i;
	gint spHybridation = 0;
	gint totalNumberOfBonds = 0;


	if(!m->connected) return 0;
	totalNumberOfBonds = m->connected[ atomNumber ][ 0 ];
	if ( m->numberOfTripleBonds[atomNumber] >= 1 ) return 1;
	else if ( m->numberOfDoubleBonds[atomNumber] >= 2 ) return 1;
	else if ( m->numberOfDoubleBonds[atomNumber] == 1 ) spHybridation += 2;
	for (  i = 1; i < totalNumberOfBonds; i++ )
	{
		gint bondedToAtomNumber = m->connected[ atomNumber ][ i ];
		if (m->numberOfTripleBonds[bondedToAtomNumber] >= 1 ) return 1;
		else if ( bondedType( m, bondedToAtomNumber, atomNumber, GABEDIT_BONDTYPE_DOUBLE )) spHybridation += 2;
	}
	if ( ( spHybridation == 2 ) && ( totalNumberOfBonds == 3 ) ) return 2;
	else if ( ( spHybridation == 4 ) && ( totalNumberOfBonds == 2 ) ) return 1;
	return ( totalNumberOfBonds - 1 );
}
/************************************************************************/
static gboolean isConnectedTo( Molecule* m, gint atomA, gchar* expression, gboolean initialize )
{
	static gboolean* inStack = NULL;
	gint i;
	gint numberOfConnections;
	gchar* rootAtom;
	gchar* restOfExpression;
	gchar firstChar;
	gchar lastChar;
	gint openClose = 0;
	gint begin = 0;
	gint multiplicity = 1;
	gint bondOrder = 0;
	gchar* index;
	GList* stack = NULL;
	GeomDef* geom = m->geom;

	if ( initialize )
	{
		inStack = g_realloc(inStack, m->numberOfAtoms*sizeof(gboolean));	
		for( i = 0; i < m->numberOfAtoms; i++ )
				inStack[ i ] = FALSE;
	}
	inStack[ atomA ] = TRUE;
	if ( expression == NULL ) return TRUE;
	if(!m->connected) return FALSE;
	numberOfConnections = m->connected[ atomA ][ 0 ];
	/* printf("numberOfConnections = %d\n",numberOfConnections);*/

	rootAtom = g_strdup(expression);
	restOfExpression = NULL;
	index = strstr(expression, "(" );
	if ( index != expression )
	{  /* find the new root atom*/
		if ( index != NULL) 
		{
			gint n0 = index - expression;
			gint n1 = strlen(expression)-1;
			rootAtom = subString(expression, 0, n0-1); /* "N", e.g. */
			restOfExpression = subString(expression, n0, n1); /*  "(C)(C)", e.g. */
		}
		else
			restOfExpression = NULL;
		/*
		if(!strcmp(geom[atomA].Prop.symbol,"N"))
		{
			if(expression) printf("expression = %s\n", expression);
			printf("rootAtom = %s\n", rootAtom);
			if(restOfExpression) printf("restOfExpression = %s\n", restOfExpression);
			else printf("restOfExpression = NULL\n");
		}
		*/

		firstChar = getCharFromString(rootAtom, 0);
		if(rootAtom) lastChar = getCharFromString(rootAtom, strlen(rootAtom) - 1);
		else lastChar = '\0';
		if ( firstChar != '\0' )
		{
			gchar* rA = rootAtom;
			if ( firstChar == '-' )
			{
				bondOrder = 1;
				rootAtom = subString(rootAtom, 1, -1);
				if(rA) g_free(rA);
			}
			else if ( firstChar == '=' )
			{
				bondOrder = 2;
				rootAtom = subString(rootAtom, 1, -1);
				if(rA) g_free(rA);
			}
			else if ( firstChar == '~')
			{
				bondOrder = 2;
				rootAtom = subString(rootAtom, 1, -1);
				if(rA) g_free(rA);
			}
			else if ( firstChar == '#' )
			{
				bondOrder = 3;
				rootAtom = subString(rootAtom, 1, -1);
				if(rA) g_free(rA);
			}
		}
		if ( lastChar  != '\0')
		{
			gchar* rA = rootAtom;
			if ( lastChar == '2' )
			{
				multiplicity = 2;
				rootAtom = subString(rootAtom, 0, strlen(rootAtom) - 2);
				if(rA) g_free(rA);
			}
			else if ( lastChar == '3' )
			{
				multiplicity = 3;
				rootAtom = subString(rootAtom, 0, strlen(rootAtom) - 2);
				if(rA) g_free(rA);
			}
			else if ( lastChar == '4'  )
			{
				multiplicity = 4;
				rootAtom = subString(rootAtom, 0, strlen(rootAtom) - 2);
				if(rA) g_free(rA);
			}
		}
		if ( multiplicity > numberOfConnectedTypes( m, atomA, rootAtom ) ) return FALSE;
		for (i = 1; i <= numberOfConnections; i++ )
		{
			gint atomB = m->connected[ atomA ][ i ];
			if ( inStack[ atomB ] ) continue;
			if ( !strcmp(geom[atomB ].Prop.symbol,rootAtom) || ( !strcmp(rootAtom,"*" ) ) )
			{
				if ( isConnectedTo( m, atomB, restOfExpression, FALSE ) )
				{
					if ( bondOrder == 0 ) return TRUE;
					else if ( bondOrder == 3 )
					{
						if ( ( bondedType( m, atomA, atomB, GABEDIT_BONDTYPE_TRIPLE ) ) || 
						      ( sp(m,  atomA ) == 1 ) || ( sp(m, atomB ) == 1 ) )
								return TRUE;
					}
					else if ( bondOrder == 2 )
					{
						if ( ( bondedType( m, atomA, atomB, GABEDIT_BONDTYPE_DOUBLE ) ) || 
						     ( sp(m, atomA ) == 2 ) || ( sp(m, atomB ) == 2 ) )
								return TRUE;
					}
					else if ( bondOrder == 1 )
					{
						if ( sp(m, atomA ) + sp(m, atomB ) >= 4 ) return TRUE;
					}
				}
			}
		}
		return FALSE;
	} 
	/* below, push (C)(C) onto stack */
	restOfExpression = g_strdup(expression);
	stack = NULL;
	openClose = 0;
	for (i = 0; i < strlen(restOfExpression); i++ )
	{
		if ( restOfExpression[i] == '(' )
		{
			if ( openClose++ == 0 ) begin = i + 1;
		}
		else if ( restOfExpression[i] == ')' )
		{
			if ( --openClose == 0 ) stack = g_list_append(stack,subString(expression, begin, i-1) );
		}
	}
	while ( stack != NULL )
	{
		gchar* newExpression = NULL;
		GList* last =  g_list_last(stack);
		newExpression = (gchar*)(last->data);
		stack = g_list_remove(stack, newExpression);
		if ( ! ( isConnectedTo( m, atomA, newExpression, FALSE ) ) ) return FALSE;
	}
	return TRUE;
}
/**********************************************************************************************************/
static gchar* getAmberTypeOfAtom(Molecule* m, gint atomNumber)
{
	GeomDef* geom = m->geom;
	/* printf("Atom number = %d symbol = %s\n",atomNumber, geom[atomNumber].Prop.symbol);*/
	if ( !strcmp(geom[atomNumber].Prop.symbol,"H" ))
	{
		if (  isConnectedTo( m, atomNumber, "N(*4)", TRUE ) ) return "H";
		else if (  isConnectedTo( m, atomNumber, "N(H)(C(N(H2)))", TRUE ) ) return "H";
		else if ( (  isConnectedTo( m, atomNumber, "N(C(N2))", TRUE ) ) &&
			 isConnectedTo( m, atomNumber, "N(C(N(H2))(N(H2)))", TRUE ) ) return "H";
		else if (  isConnectedTo( m, atomNumber, "C", TRUE ) ) 
		{
			gint numberOfConnections = m->connected[ atomNumber ][ 0 ];
			gint i;
			for (i = 1; i <= numberOfConnections; i++ )
			{
				gint newAtom = m->connected[atomNumber][i];
				if(!strcmp(geom[newAtom].Prop.symbol,"C" ))
				{
					/* printf("atom = %d sym = %s\n", newAtom,geom[newAtom].Prop.symbol);*/
					if(atomInRing( m, newAtom, 6 ) && allCarbon(m)) return "HA";
					else if (  isConnectedTo( m, newAtom, "N2", TRUE ) ) return "H5";
					else if (  isConnectedTo( m, newAtom, "N(C)", TRUE ) ) return "H4";
					else if (  isConnectedTo( m, newAtom, "N(H3)", TRUE ) ) return "HP";
					else if (  isConnectedTo( m, newAtom, "N(*)", TRUE ) ) return "H1";
					else if (  isConnectedTo( m, newAtom, "S", TRUE ) ) return "H1";
					else if (  isConnectedTo( m, newAtom, "O", TRUE ) ) return "H1";
				}
			}
			return "HC";
		}
		else if (  isConnectedTo( m, atomNumber, "O(H)", TRUE ) ) return "HW";
		else if (  isConnectedTo( m, atomNumber, "O", TRUE ) ) return "HO";
		else if (  isConnectedTo( m, atomNumber, "S", TRUE ) ) return "HS";
		/*
		else if (  isConnectedTo( m, atomNumber, "N(H)(C(~*))", TRUE ) ) return "H2";
		else if (  isConnectedTo( m, atomNumber, "N(H)(C(=N)(-C))", TRUE ) ) return "H2";
		else if (  isConnectedTo( m, atomNumber, "N(H)(C(=N)(-N))", TRUE ) ) return "H2";
		else if (  isConnectedTo( m, atomNumber, "N(H2)", TRUE ) ) return "H2";
		*/
		else return "H";
	}
	else if ( !strcmp(geom[atomNumber].Prop.symbol,"C" ))
	{
		/* printf("sp = %d\n",sp(m, atomNumber ));*/
		if ( sp(m, atomNumber ) == 2 )
		{
			if ( ( atomInRing( m, atomNumber, 5 ) ) && ( atomInRing( m, atomNumber, 6 ) ) )
			{
				if (  isConnectedTo( m, atomNumber, "C(N)", TRUE ) ) return "CB";
				else if (  isConnectedTo( m, atomNumber, "N(Fe)", TRUE ) ) return "CC";
				else if (  isConnectedTo( m, atomNumber, "N", TRUE ) ) return "CN";
				else return "CT";
			}
			else if ( atomInRing( m, atomNumber, 5 ) )
			{
				if (  isConnectedTo( m, atomNumber, "(C)(C)(C(N(Fe)))", TRUE ) ) return "CB";
				else if ( (  isConnectedTo( m, atomNumber, "(H)(N2)", TRUE ) )
					&& (  isConnectedTo( m, atomNumber, "(N)(H)(N(C(N(C(N(C))))))", TRUE ) ) )
							return "CK";
				else if (  isConnectedTo( m, atomNumber,"(N2)(H)", TRUE ) ) return "CR";
				else if (  isConnectedTo( m, atomNumber,"(C(H2))(N)", TRUE ) ) return "CC";
				else if (  isConnectedTo( m, atomNumber,"C(H2)", TRUE ) ) return "C*";
				else if (  isConnectedTo( m, atomNumber,"N(~*)(*)", TRUE ) ) return "CW";
				else if (  isConnectedTo( m, atomNumber,"N", TRUE ) ) return "CV";
				else return "CT";
			}
			else if (atomInRing( m, atomNumber, 6 ) )
			{
				if (  isConnectedTo( m, atomNumber, "O(H)", TRUE ) ) return "C";
				else if (  isConnectedTo( m, atomNumber, "=O", TRUE ) ) return "C";
				else if (  isConnectedTo( m, atomNumber, "(N2)(H)", TRUE ) ) return "CQ";
				else if (  isConnectedTo( m, atomNumber, "N(H2)", TRUE ) ) return "CA";
				else if (  isConnectedTo( m, atomNumber, "C(N(C(=O)))", TRUE ) ) return "CM";
				else if (  isConnectedTo( m, atomNumber, "N(C(=O))", TRUE ) ) return "CM";
				else return "CA";
			}
			else if (  isConnectedTo( m, atomNumber, "(~O2)", TRUE ) ) return "C";
			else if (  isConnectedTo( m, atomNumber, "=O", TRUE ) ) return "C";
			else if (  isConnectedTo( m, atomNumber, "(=C)(-S)", TRUE ) ) return "CY";
			/*else if (  isConnectedTo( m, atomNumber, "(=C)", TRUE ) ) return "CX";*/
			else if (  isConnectedTo( m, atomNumber, "(N3)", TRUE ) ) return "CA";
			else return "CT";
		}
		else if ( sp(m, atomNumber ) == 3 )
		{
			if (  isConnectedTo( m, atomNumber, "(N3)", TRUE ) ) return "CA";
			else if (  isConnectedTo( m, atomNumber, "=O", TRUE ) ) return "C";
			else if (  isConnectedTo( m, atomNumber, "(=C)(-S)", TRUE ) ) return "CY";
			/* else if (  isConnectedTo( m, atomNumber, "(=C)", TRUE ) ) return "CT";*/
			else return "CT";
		}
		else { 
			if (  isConnectedTo( m, atomNumber, "(N3)", TRUE ) ) return "CA";
			else if (  isConnectedTo( m, atomNumber, "=O", TRUE ) ) return "C";
			else if (  isConnectedTo( m, atomNumber, "(=C)(-S)", TRUE ) ) return "CY";
			/* else if (  isConnectedTo( m, atomNumber, "(=C)", TRUE ) ) return "CX";*/
			else if (  isConnectedTo( m, atomNumber, "#C", TRUE ) ) return "CZ";
			else return "CT";
		}
	}
	else if ( !strcmp(geom[atomNumber].Prop.symbol,"N" ) )
	{
		if ( sp(m, atomNumber ) < 3 )
		{
			if ( atomInRing( m, atomNumber, 5 ) )
			{
				if ( (  isConnectedTo( m, atomNumber, "(C3)", TRUE ) ) && 
				     (  isConnectedTo( m, atomNumber, "(C)(C(=*))(C(=*))", TRUE ) ) ) return "N*";
				else if (  isConnectedTo( m, atomNumber, "(C3)", TRUE ) ) return "N*";
				else if (  isConnectedTo( m, atomNumber, "Fe", TRUE ) ) return "NA";
				else if (  isConnectedTo( m, atomNumber, "H", TRUE ) ) return "NA";
				else return "NB";
			}
			else if (atomInRing( m, atomNumber, 6 ) )
			{
				if (  isConnectedTo( m, atomNumber, "H", TRUE ) ) return "NA";
				else if ( (  isConnectedTo( m, atomNumber, "(H)(C2)", TRUE ) ) 
					&& (  isConnectedTo( m, atomNumber, "(H)(C(=O))(C(=O))", TRUE ) ) ) return "NA";
				else if (  isConnectedTo( m, atomNumber, "(H)(C(=O))(C(=N))", TRUE ) ) return "NA";
				else if (  isConnectedTo( m, atomNumber, "(C3)", TRUE ) ) return "N*";
				else return "NC";
			}
			else if (  isConnectedTo( m, atomNumber, "-C(=O)", TRUE ) ) 
			{
				return "N";
			}
			else if (  isConnectedTo( m, atomNumber, "-C(-C(=O))", TRUE ) )
			{
				return "N";
			}
			else if (  isConnectedTo( m, atomNumber, "C(N2)", TRUE ) ) return "N2";
			else if (  isConnectedTo( m, atomNumber, "(H2)(C~*)", TRUE ) ) return "N2";
			else if (  isConnectedTo( m, atomNumber, "(H2)(C=*)", TRUE ) ) return "N2";
			else return "NT";
		}
		else if ( sp(m, atomNumber ) == 3 ) return "N3";
		else return "NT";
	}
	else if ( !strcmp(geom[atomNumber].Prop.symbol,"O" ))
	{
		if (  isConnectedTo( m, atomNumber, "~C(~O2)", TRUE ) ) 
		{
			if (  isConnectedTo( m, atomNumber, "H", TRUE ) ) return "OH";
			else return "O2";
		}
		else if (  isConnectedTo( m, atomNumber, "H2", TRUE ) ) return "OW";
		else if (  isConnectedTo( m, atomNumber, "H", TRUE ) ) return "OH";
		else if (  isConnectedTo( m, atomNumber, "=C", TRUE ) ) return "O";
		else if (  isConnectedTo( m, atomNumber, "(H)(C(=O))", TRUE ) ) return "OH";
		else if (  isConnectedTo( m, atomNumber, "-C2", TRUE ) ) return "OS";
		else if (  isConnectedTo( m, atomNumber, "(C(=O))(C)", TRUE ) ) return "OS";
		else if (  isConnectedTo( m, atomNumber, "(C)(P)", TRUE ) ) return "OS";
		else if (  isConnectedTo( m, atomNumber, "P", TRUE ) ) return "O2";
		else return "OS";
	}
	else if ( !strcmp(geom[atomNumber].Prop.symbol,"F" ) ) return "F";
	else if ( !strcmp(geom[atomNumber].Prop.symbol,"Na" ) ) return "IP";
	else if ( !strcmp(geom[atomNumber].Prop.symbol,"Mg" )) return "MG";
	else if ( !strcmp(geom[atomNumber].Prop.symbol,"P" ) ) return "P";
	else if ( !strcmp(geom[atomNumber].Prop.symbol,"S" ) )
	{
		if (  isConnectedTo( m, atomNumber, "H", TRUE ) ) return "SH";
		else return "S";
	}
	else if ( !strcmp(geom[atomNumber].Prop.symbol,"Cl" ))
	{
		if ( !m->connected ) return "CL";
		if ( m->connected[ atomNumber ][ 0 ] > 0 ) return "CL";
		else return "IM";
	}
	else if ( !strcmp(geom[atomNumber].Prop.symbol,"Ca" ) ) return "C0";
	else if ( !strcmp(geom[atomNumber].Prop.symbol,"Fe" ) ) return "FE";
	else if ( !strcmp(geom[atomNumber].Prop.symbol,"Cu" ) ) return "CU";
	else if ( !strcmp(geom[atomNumber].Prop.symbol,"Br" ) ) return "BR";
	else if ( !strcmp(geom[atomNumber].Prop.symbol,"I" ) ) return "I";

	return geom[atomNumber].Prop.symbol;
}
/************************************************************************/
static gint* getNumberOfBonds(Molecule* m, GabEditBondType type)
{
	gint i;
	gint j;
	GList* bondsList = m->bondsList;
	gint nAtoms = m->numberOfAtoms;
	gint* numberOfBonds = NULL;
	GList* list = NULL;
	if(nAtoms<1) return NULL;
	numberOfBonds =g_malloc(nAtoms*sizeof(gint));
	for(i = 0;i<nAtoms;i++) numberOfBonds[i] = 0;
	for(list=bondsList;list!=NULL;list=list->next)
	{
		BondType* data=(BondType*)list->data;
		i = data->n1;
		j = data->n2;
		if(data->bondType == type) 
		{
			numberOfBonds[i]++;
			numberOfBonds[j]++;
		}
	}
	return numberOfBonds;
}
/**********************************************************************************************************/
static Molecule getMyMolecule(GeomDef* geom, gint nAtoms)
{
	Molecule m;
	m.numberOfAtoms = nAtoms;
	m.geom = geom;

	m.bondsList = NULL;
	m.connected = NULL;
	m.numberOfDoubleBonds = NULL;
	m.numberOfTripleBonds = NULL;
	buildBonds(&m, FALSE);
	m.numberOfDoubleBonds = getNumberOfBonds(&m, GABEDIT_BONDTYPE_DOUBLE);
	m.numberOfTripleBonds = getNumberOfBonds(&m, GABEDIT_BONDTYPE_TRIPLE);
	buildConnections(&m);
	/* printConnections(&m);*/
	return m;
}
/**********************************************************************************************************/
static void freeMolecule(Molecule* m)
{
	g_free(m->numberOfDoubleBonds);
	g_free(m->numberOfTripleBonds);
	freeConnections(m);
	freeBonds(m->bondsList);
}
/**********************************************************************************************************/
void calculTypesAmber(GeomDef* geom, gint nAtoms)
{
	gint i;
	Molecule m;
	if(nAtoms<1) return;
	m = getMyMolecule(geom, nAtoms);

	for(i=0;i<nAtoms;i++)
	{
 
		/*
		if( atomInRing( &m, i, 5 )) printf("atom number %d is in pentagon\n",i+1);
		if( atomInRing( &m, i, 6 )) printf("atom number %d is in hexagon\n",i+1);
		if (  isConnectedTo( &m, i, "C", TRUE ) ) printf("atom number %d  have %s type \n",i+1,"HC");
		*/

		/* printf("i = %d\n",i);*/
		if(geom[i].mmType) g_free(geom[i].mmType);
		geom[i].mmType = g_strdup(getAmberTypeOfAtom(&m, i));
	}
	freeMolecule(&m);
}
/**********************************************************************************************************/
static GeomDef* getMyGeomFromFrag(Fragment* F)
{
	GeomDef* geom = NULL;
	gint i;
	if(F->NAtoms<=0) return NULL;

	geom  = g_malloc(F->NAtoms*sizeof(GeomDef)); 

	for(i=0;i<F->NAtoms;i++)
		geom[i].Prop = prop_atom_get(F->Atoms[i].Symb);

	for(i=0;i<F->NAtoms;i++)
	{
		geom[i].X = F->Atoms[i].Coord[0];
		geom[i].Y = F->Atoms[i].Coord[1];
		geom[i].Z = F->Atoms[i].Coord[2];
		geom[i].Prop = prop_atom_get(F->Atoms[i].Symb);
		geom[i].mmType =g_strdup(F->Atoms[i].mmType);
		geom[i].pdbType =g_strdup(F->Atoms[i].pdbType);
		geom[i].Layer = HIGH_LAYER;
		geom[i].Variable = FALSE;
		geom[i].Residue =g_strdup(F->Atoms[i].Residue);
		geom[i].ResidueNumber= 0;
		geom[i].show= TRUE;
		geom[i].Variable= TRUE;
		geom[i].Charge = F->Atoms[i].Charge;
		geom[i].N = i+1;
		geom[i].typeConnections = NULL;
	}
	return geom;
}
/**********************************************************************************************************/
void calculTypesAmberForAFragment(Fragment* F)
{
	gint i;
	Molecule m;
	GeomDef* geom = NULL;
	if(F->NAtoms<1) return;
	geom = getMyGeomFromFrag(F);
	if(!geom) return;
	m = getMyMolecule(geom, F->NAtoms);
	for(i=0;i<F->NAtoms;i++)
	{
 
		/*
		*/
		if(F->Atoms[i].mmType) g_free(F->Atoms[i].mmType);
		F->Atoms[i].mmType = g_strdup(getAmberTypeOfAtom(&m, i));
	}
	Free_One_Geom(geom,F->NAtoms);
	freeMolecule(&m);
}
