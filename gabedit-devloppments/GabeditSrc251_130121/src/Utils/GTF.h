#ifndef __GABEDIT_GTF_H__
#define __GABEDIT_GTF_H__

#include "TTables.h"

gdouble normeGTF(GTF* p);
void normaliseRadialGTF(GTF*p);
void normaliseGTF(GTF*p);
gdouble overlapGTF(GTF* p,GTF* q);
gdouble overlap3GTF(GTF* p,GTF* q, GTF* r);
gdouble overlap4GTF(GTF* p,GTF* q, GTF* r);
gdouble GTFxyzGTF(GTF* p,GTF* q, gint ix, gint iy, gint iz);
gdouble kineticGTF(GTF* left, GTF* right);
gdouble ionicPotentialGTF(GTF* left, GTF* right,gdouble* C,gdouble Z);
gdouble ERIGTF(GTF* p,GTF* q, GTF* r, GTF* s);
gboolean CGTFEqCGTF(CGTF* t1,CGTF* t2);
gdouble ERITABLES(gint i,gint j,gint k,gint l,gint ni,gint nj,gint nk,gint nl,TTABLES** Ttables);
gdouble ERICTABLES(gint i,gint j,gint k,gint l, TTABLES** Ttables);
gdouble ERICGTF(CGTF* p, CGTF* q, CGTF* r, CGTF*s);
void normaliseCGTF(CGTF* left);
gdouble overlapCGTF(CGTF* left, CGTF* right);
gdouble overlap3CGTF(CGTF* left, CGTF* midle, CGTF* right);
gdouble overlap4CGTF(CGTF* A,CGTF* B, CGTF* C, CGTF* D);
gdouble CGTFxyzCGTF(CGTF* left, CGTF* right, gint ix, gint iy, gint iz);
gdouble kineticCGTF(CGTF* left, CGTF* right);
gdouble ionicPotentialCGTF(CGTF* left, CGTF* right, gdouble* C, gdouble Z);
gdouble CGTFstarCGTF(CGTF* left, CGTF* right);
#endif /* __GABEDIT_GTF_H__ */
