#ifndef __GABEDIT_POISSONMG_H__
#define __GABEDIT_POISSONMG_H__
#include "GridMG.h"
typedef GridMG*   (*GetSource)(DomainMG);
typedef void   (*SetBoundary)(GridMG*, GridMG*);
typedef struct _PoissonMG PoissonMG;
struct _PoissonMG
{
		gdouble diag;
		GridMG* potential;
		GridMG* source;
		SetBoundary setBoundary;
		Condition condition;
};
PoissonMG* getNullPoissonMG();
PoissonMG* getPoissonMGUsingDomain(DomainMG);
PoissonMG* getPoissonMG2(GridMG*, GridMG*, Condition, SetBoundary);
PoissonMG* getPoissonMG(GridMG*, GridMG*);
PoissonMG* getCopyPoissonMG(PoissonMG* ps);
void destroyPoissonMG(PoissonMG*);
void setOperationPoissonMG(PoissonMG*,OperationTypeMG);
DomainMG getDomainPoissonMG(PoissonMG*);
gdouble getDiagPoissonMG(PoissonMG*);

void prolongationPoissonMG(PoissonMG* ps);
void ginterpolationTriLinearPoissonMG(PoissonMG* ps);
void ginterpolationCubicPoissonMG(PoissonMG* ps);
void restrictionPoissonMG(PoissonMG* ps);
void restrictionInjectionPoissonMG(PoissonMG* ps);

GridMG* residualPoissonMG(PoissonMG* ps);
gdouble residualNormPoissonMG(PoissonMG* ps);
void tradesBoundaryPoissonMG(PoissonMG* ps);
void smootherPoissonMG(PoissonMG* ps,gint max);
void printFilePoissonMG(PoissonMG* ps);
void printFileNamePoissonMG(PoissonMG* ps, gchar* fileName);
void printPoissonMGAll(PoissonMG* ps);
void printMinPoissonMG(PoissonMG* ps);
void printMaxPoissonMG(PoissonMG* ps);
void printPoissonMG(PoissonMG* ps, gint i, gint j, gint k);
GridMG* getPotentialPoissonMG(PoissonMG* ps);

GridMG* getSourcePoissonMG(PoissonMG* ps);
void solveCGPoissonMG(PoissonMG* ps, gint max, gdouble acc);
gdouble solveMGPoissonMG(PoissonMG* ps, gint levelMax);
void solveMGPoissonMG2(PoissonMG* ps, gint levelMax,gdouble acc, gint verbose);
void solveMGPoissonMG3(PoissonMG* ps, int levelMax, int nIter, gdouble acc, int verbose);
void solveSmootherPoissonMG2(PoissonMG* ps, gint max, gint nf);
void solveSmootherPoissonMG(PoissonMG* ps, gint imax, gdouble eps);
#endif /* __GABEDIT_POISSONMG_H__ */
