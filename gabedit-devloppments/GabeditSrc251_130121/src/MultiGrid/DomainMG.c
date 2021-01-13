/* DomainMG.c */
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
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <gtk/gtk.h>
#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
#include "../Utils/Constants.h"
#include "DomainMG.h"

/*********************************************************/
void printDomain(DomainMG*g)
{
	printf("xSize   = %d ySize      = %d zSize      = %d\n",g->xSize, g->ySize, g->zSize);
	printf("x0      = %f y0         = %f z0         = %f\n",g->x0, g->y0, g->z0);
	printf("xLength = %f yLength    = %f zLength    = %f\n",g->xLength,g->yLength ,g->zLength );
	printf("xh      = %f yh         = %f zh         = %f\n",g->xh,g->yh ,g->zh );
	printf("size      = %d\n",g->size);
	printf("laplacianOrder      = %d\n",g->laplacianOrder);
}
/*********************************************************/
gboolean ifEqualDomainMG(DomainMG*g, DomainMG* right)
{
	if(g->xSize != right->xSize) return FALSE;
	if(g->ySize != right->ySize) return FALSE;
	if(g->zSize != right->zSize) return FALSE;

	if(g->x0 != right->x0) return FALSE;
	if(g->y0 != right->y0) return FALSE;
	if(g->z0 != right->z0) return FALSE;

	if(g->xLength != right->xLength) return FALSE;
	if(g->yLength != right->yLength) return FALSE;
	if(g->zLength != right->zLength) return FALSE;

	if(g->xh != right->xh) return FALSE;
	if(g->yh != right->yh) return FALSE;
	if(g->zh != right->zh) return FALSE;

	if(g->size != right->size) return FALSE;

	if(g->laplacianOrder != right->laplacianOrder) return FALSE;

	return TRUE;
}
/*********************************************************/
gint setMaxLevelDomainMG1(DomainMG*d , gint size)
{
	size++;
	gint level = 1;
	if(size>1)
	while(size % 2 == 0)
	{
		size = size/2;
		level++;
	}
	return level;
}
/*********************************************************/
void setMaxLevelDomainMG(DomainMG*d)
{
	gint level;
	d->maxLevel = setMaxLevelDomainMG1(d,d->xSize);
	level = setMaxLevelDomainMG1(d,d->ySize);
	if(level<d->maxLevel) d->maxLevel = level;
	level = setMaxLevelDomainMG1(d,d->zSize);
	if(level<d->maxLevel) d->maxLevel = level;
}
/*********************************************************/
void setParametersDomainMG(DomainMG*d)
{
	d->nBoundary = d->laplacianOrder/2;

	d->xh = d->xLength/(d->xSize+1);
	d->yh = d->yLength/(d->ySize+1);
	d->zh = d->zLength/(d->zSize+1);

	d->cellVolume = d->xh*d->yh*d->zh;

	d->size = (d->xSize+2*d->nBoundary)*(d->ySize+2*d->nBoundary)*(d->zSize+2*d->nBoundary);


	d->incx = (d->ySize +2*d->nBoundary)*(d->zSize +2*d->nBoundary);
	d->incy = (d->zSize +2*d->nBoundary);
	d->incz = 1;
	d->nShift = d->nBoundary-1;

	d->iXBeginBoundaryLeft = -d->nBoundary + 1;
	d->iXEndBoundaryLeft = 0;
	d->iXBeginBoundaryRight = d->xSize + 1;
	d->iXEndBoundaryRight = d->xSize + d->nBoundary;

	d->iYBeginBoundaryLeft = -d->nBoundary + 1;
	d->iYEndBoundaryLeft = 0;
	d->iYBeginBoundaryRight = d->ySize + 1;
	d->iYEndBoundaryRight = d->ySize + d->nBoundary;

	d->iZBeginBoundaryLeft = -d->nBoundary + 1;
	d->iZEndBoundaryLeft = 0;
	d->iZBeginBoundaryRight = d->zSize + 1;
	d->iZEndBoundaryRight = d->zSize + d->nBoundary;

	d->iXBeginInterior = d->iXEndBoundaryLeft + 1;
	d->iXEndInterior = d->iXBeginBoundaryRight - 1;

	d->iYBeginInterior = d->iYEndBoundaryLeft + 1;
	d->iYEndInterior = d->iYBeginBoundaryRight - 1;

	d->iZBeginInterior = d->iZEndBoundaryLeft + 1;
	d->iZEndInterior = d->iZBeginBoundaryRight - 1;
	setMaxLevelDomainMG(d);

	/*
	cout<<"IX = "<<
		d->iXBeginBoundaryLeft<<" "<<d->iXEndBoundaryLeft<<" "
		<<d->iXBeginInterior<<" "<<d->iXEndInterior<<" "<<
		d->iXBeginBoundaryRight<<" "<<d->iXEndBoundaryRight<<endl;
		*/
}
/*********************************************************/
void setCoefsLaplacianDomainMG(DomainMG*d)
{
	gdouble* coefs =  g_malloc((d->nBoundary+1)*sizeof(gdouble));
	gint i;
	switch(d->nBoundary)
	{
		case 1:{
				gdouble c[] = {-2.0, 1.0};
				for( i=0;i<=d->nBoundary;i++)
					coefs[i] = c[i];
				break;
			}
		case 2:{
				gdouble denom = 12.0;
				gdouble c[] = {-30.0, 16.0, -1.0};
				for( i=0;i<=d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 3:{
				gdouble denom = 180.0;
				gdouble c[] = {-490.0, 270.0,-27.0, 2.0};
				for( i=0;i<=d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 4:{
				gdouble denom = 5040.0;
				gdouble c[] = {-14350.0, 8064.0, -1008.0, 128.0, -9.0};
				for( i=0;i<=d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 5:{
				gdouble denom = 25200.0;
				gdouble c[] = {-73766.0, 42000.0, -6000.0, 1000.0, -125.0, 8.0};
				for( i=0;i<=d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 6:{
				gdouble denom = 831600.0;
			 	gdouble c[] = {-2480478.0,1425600.0,-222750.0,44000.0,-7425.0,864.0,-50.0};
				for( i=0;i<=d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 7:{
				gdouble denom = 75675600.0;
				gdouble c[] = {-228812298.0,132432300.0,-22072050.0,4904900.0,-1003275.0, 160524.0,-17150.0,900.0};
				for( i=0;i<=d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 8:{
				gdouble denom = 302702400.0;
				gdouble c[] = {-924708642.0,538137600.0,-94174080.0,22830080.0,-5350800.0,1053696.0,-156800.0,15360.0,-735.0};
				for( i=0;i<=d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}



		case 9:{
				gdouble denom =  15437822400.0;
				gdouble c[] = {-47541321542.0,+27788080320.0, -5052378240.0,+1309875840.0,-340063920.0,+77728896.0,-14394240.0,+1982880.0,-178605.0,+7840.0};
				for( i=0;i<=d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 10:{
				gdouble denom = 293318625600.0;
				gdouble c[] = {-909151481810.0,+533306592000.0, -99994986000.0,+27349056000.0,-7691922000.0,+1969132032.0,-427329000.0,+73872000.0,-9426375.0,+784000.0,-31752.0};
				for( i=0;i<=d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 11:{
				gdouble denom =  3226504881600.0;
				gdouble c[] = {-10053996959110.0,+5915258949600.0,-1137549798000.0,+325014228000.0,-97504268400.0,+27301195152.0,-6691469400.0,+1365606000.0,-220114125.0,+26087600.0,-2012472.0,+75600.0};
				for( i=0;i<=d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 12:{
				gdouble denom = 74209612276800.0;
				gdouble c[] = {-232272619118930.0,+137002361126400.0,-26911178078400.0,+7973682393600.0,-2522922944850.0,+759845028096.0,-205205061600.0,+47609337600.0,-9112724775.0,+1371462400.0,-151484256.0,+10886400.0,-381150.0};
				for( i=0;i<=d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
	}

	gdouble x2h = 1.0 / (d->xh * d->xh);
	gdouble y2h = 1.0 / (d->yh * d->yh);
	gdouble z2h = 1.0 / (d->zh * d->zh);

	d->cc = x2h + y2h + z2h;
	d->cc *= coefs[0];

	for(i=1;i<=d->nBoundary;i++)
	{
		d->fLaplacinaX[i] =  x2h * coefs[i];
		d->fLaplacinaY[i] =  y2h * coefs[i];
		d->fLaplacinaZ[i] =  z2h * coefs[i];
	}


	d->diag = 1.0/d->cc;

	g_free(coefs);
}
/*********************************************************/
void setCoefsGradientDomainMG(DomainMG*d)
{
	gdouble* coefs =  g_malloc(d->nBoundary*sizeof(gdouble));
	gint i;
	switch(d->nBoundary)
	{
		case 1:{
				gdouble denom = 2.0;
				gdouble c[] = {-1.0};
				for(i=0;i<d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 2:{
				gdouble denom =12.0;
				gdouble c[] = { 1.0, -8.0};
				for( i=0;i<d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 3:{
				gdouble denom =60.0;
				gdouble c[] = { -1.0, +9.0, -45.0};
				for( i=0;i<d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 4:{
				gdouble denom =840.0;
				gdouble c[] = { 3.0, -32.0, +168.0, -672.0};
				for( i=0;i<d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 5:{
				gdouble denom =2520.0 ;
				gdouble c[] = { -2.0, +25.0, -150.0,+600.0, -2100.0};
				for( i=0;i<d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 6:{
				gdouble denom =27720.0 ;
				gdouble c[] = { 5.0, -72.0, +495.0, -2200.0, +7425.0, -23760.0};
				for( i=0;i<d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 7:{
				gdouble denom =360360.0;
				gdouble c[] = { -15.0, +245.0, -1911.0, +9555.0, -35035.0, +105105.0, -315315.0};
				for( i=0;i<d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
		case 8:{
				gdouble denom =720720.0;
				gdouble c[] = { 7.0, -128.0, +1120.0, -6272.0, +25480.0, -81536.0, +224224.0, -640640.0};
				for( i=0;i<d->nBoundary;i++)
					coefs[i] = c[i]/denom;
				break;
			}
	}

	gdouble xxh = 1.0 / (d->xh);
	gdouble yyh = 1.0 / (d->yh);
	gdouble zzh = 1.0 / (d->zh);

	for(i=0;i<d->nBoundary;i++)
	{
		d->fGradientX[i] =  xxh * coefs[i];
		d->fGradientY[i] =  yyh * coefs[i];
		d->fGradientZ[i] =  zzh * coefs[i];
	}

	g_free(coefs);
}
/*********************************************************/
DomainMG getDomainMG(int sx,int sy,int sz,gdouble x00,gdouble y00, gdouble z00, gdouble xL, gdouble yL, gdouble zL, LaplacianOrderMG order)
{
	DomainMG dd;
	DomainMG* d=&dd;
	d->xSize = sx;
	d->ySize = sy;
	d->zSize = sz;
	if(d->xSize%2 == 0) d->xSize--;
	if(d->ySize%2 == 0) d->ySize--;
	if(d->zSize%2 == 0) d->zSize--;

	d->x0 = x00; d->y0 = y00; d->z0 = z00;

	d->xLength = xL; d->yLength = yL; d->zLength = zL;

	d->laplacianOrder = order;


	setParametersDomainMG(d);
	/* printDomain(d);*/
	setCoefsLaplacianDomainMG(d);
	setCoefsGradientDomainMG(d);
	return dd;
}
/*********************************************************/
DomainMG getNullDomainMG()
{
	DomainMG dd;
	DomainMG* d=&dd;
	d->xSize = 0; d->ySize = 0; d->zSize = 0;
	d->size  = 0;

	d->x0 = 0; d->y0 = 0; d->z0 = 0;

	d->xLength = 0; d->yLength = 0; d->zLength = 0;

	d->laplacianOrder = GABEDIT_LAPLACIAN_2;

	setParametersDomainMG(d);
	setCoefsLaplacianDomainMG(d);
	setCoefsGradientDomainMG(d);
	return dd;
}
/*********************************************************/
void destroyDomainMG(DomainMG*d)
{
	d->xSize = 0; d->ySize = 0; d->zSize = 0;
	d->size  = 0;

	d->x0 = 0; d->y0 = 0; d->z0 = 0;

	d->xLength = 0; d->yLength = 0; d->zLength = 0;

	d->laplacianOrder = GABEDIT_LAPLACIAN_2;

	setParametersDomainMG(d);
	setCoefsLaplacianDomainMG(d);
	setCoefsGradientDomainMG(d);
}
/*********************************************************/
void setLaplacianOrderDomainMG(DomainMG*d, const LaplacianOrderMG order)
{
	d->laplacianOrder = order;

	setParametersDomainMG(d);
	setCoefsLaplacianDomainMG(d);
	setCoefsGradientDomainMG(d);
}
/*********************************************************/
void levelUpDomainMG(DomainMG*d)
{
	d->xSize = (d->xSize+1)*2-1;
	d->ySize = (d->ySize+1)*2-1;
	d->zSize = (d->zSize+1)*2-1;

	setParametersDomainMG(d);
	setCoefsLaplacianDomainMG(d);
	setCoefsGradientDomainMG(d);
}
/*********************************************************/
void levelDownDomainMG(DomainMG*d)
{
	d->xSize = (d->xSize+1)/2-1;
	d->ySize = (d->ySize+1)/2-1;
	d->zSize = (d->zSize+1)/2-1;

	setParametersDomainMG(d);
	setCoefsLaplacianDomainMG(d);
	setCoefsGradientDomainMG(d);
}
/*********************************************************/
