#ifndef __GABEDIT_MATHFUNCS_H__
#define __GABEDIT_MATHFUNCS_H__
#include "Zlm.h"
gdouble factorial(gint);
gdouble doubleFactorial(gint);
gdouble binomial(gint, gint);
gdouble binomial2(gint, gint);
gdouble fprod(gint, gint);
Zlm fZlm(gint, gint);
gdouble modifiedSphericalBessel(gint l, gdouble z);
void modifiedSphericalBessel0(gint l, gdouble z[], gdouble b[], gint n);
void destroyFactorial();
void destroyDoubleFactorial();
void destroyBinomial();
void destroyFprod();
void destroyZlmTable();

gint m1p(gint i);
gdouble dpn(gdouble e,gint n);
gdouble H(gint m[3],gdouble **PQn,gdouble *Gk);
#endif /* __GABEDIT_MATHFUNCS_H__ */
