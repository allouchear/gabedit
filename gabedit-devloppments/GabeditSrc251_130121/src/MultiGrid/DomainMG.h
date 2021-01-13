/*
 * xSize = total number of grids(includ boundary)
 * 0..nb-1 and xSize-(nb-1)..xsize = boundary
 * nb to xSize-nb   = interior
 */
#ifndef __GABEDIT_DOMAINMG_H__
#define __GABEDIT_DOMAINMG_H__
#include "TypesMG.h"
typedef struct _DomainMG DomainMG;
struct _DomainMG
{
	       	gint xSize;
	       	gint ySize;
	       	gint zSize;
	       	gint size;
		gdouble xLength;
		gdouble yLength;
		gdouble zLength;
		gdouble diag;
		gdouble cc;
		gdouble fLaplacinaX[MAXBOUNDARY+1];
		gdouble fLaplacinaY[MAXBOUNDARY+1];
		gdouble fLaplacinaZ[MAXBOUNDARY+1];

		gdouble fGradientX[MAXBOUNDARY];
		gdouble fGradientY[MAXBOUNDARY];
		gdouble fGradientZ[MAXBOUNDARY];
		gdouble x0;
		gdouble y0;
		gdouble z0;
		gdouble xh;
		gdouble yh;
		gdouble zh;
		gdouble cellVolume;
		gint maxLevel;

		LaplacianOrderMG laplacianOrder;
		gint nBoundary;
		glong incx;
		gint incy;
		gint incz;
		gint nShift;

		gint iXBeginBoundaryLeft;
		gint iXEndBoundaryLeft;
		gint iXBeginBoundaryRight;
		gint iXEndBoundaryRight;

		gint iYBeginBoundaryLeft;
		gint iYEndBoundaryLeft;
		gint iYBeginBoundaryRight;
		gint iYEndBoundaryRight;

		gint iZBeginBoundaryLeft;
		gint iZEndBoundaryLeft;
		gint iZBeginBoundaryRight;
		gint iZEndBoundaryRight;

		gint iXBeginInterior;
		gint iXEndInterior;

		gint iYBeginInterior;
		gint iYEndInterior;

		gint iZBeginInterior;
		gint iZEndInterior;

};
/* Method of DomainMG struct */
void printDomain(DomainMG*g);
gboolean ifEqualDomainMG(DomainMG*g, DomainMG* right);
gint getMaxLevelDomainMG1(DomainMG*, gint size);
void setMaxLevelDomainMG(DomainMG*);
void setParametersDomainMG(DomainMG*);
void setCoefsLaplacianDomainMG(DomainMG*);
void setCoefsGradientDomainMG(DomainMG*);
void destroyDomainMG(DomainMG*);
void setLaplacianOrderDomainMG(DomainMG*, const LaplacianOrderMG);
void levelUpDomainMG(DomainMG*);
void levelDownDomainMG(DomainMG*);
DomainMG getDomainMG(gint, gint, gint, gdouble, gdouble, gdouble, gdouble, gdouble, gdouble, LaplacianOrderMG);
DomainMG getNullDomainMG();
#endif /* __GABEDIT_DOMAINMG_H__ */
