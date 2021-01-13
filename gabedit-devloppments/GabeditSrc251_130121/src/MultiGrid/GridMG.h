#ifndef __GABEDIT_GRIDMG_H__
#define __GABEDIT_GRIDMG_H__
#include "DomainMG.h"
typedef struct _GridMG GridMG;
struct _GridMG
{
	DomainMG domain;
	OperationTypeMG operationType;
	gdouble* values;
};
/* public methods for GridMG */
GridMG* getNewGridMG();
GridMG* getNewGridMGUsingDomain(DomainMG* domain);
GridMG* getNewGridMGFromOldGrid(GridMG* src);
void initGridMG(GridMG* g, gdouble);
void equalGridMG(GridMG* g, GridMG* src);
void copyGridMG(GridMG* g, GridMG* src);
void plusEqualGridMG(GridMG* g, GridMG* src);
void moinsEqualGridMG(GridMG* g, GridMG* src);
void multEqualGridMG(GridMG* g, GridMG* src);
void multEqualRealGridMG(GridMG* g, gdouble a);
void divEqualBoundaryGridMG(GridMG* g, gdouble a);
gdouble dotGridMG(GridMG* g, GridMG* src); 
gdouble normGridMG(GridMG* g);
gdouble normDiffGridMG(GridMG* g, GridMG* src);
gdouble sommeGridMG(GridMG* g);
gdouble normalizeGridMG(GridMG* g);
void tradesBoundaryGridMG(GridMG* g, const Condition condition);
void destroyGridMG(GridMG* g);

gdouble* getArrayGridMG(GridMG* g);
gdouble getDiagGridMG(GridMG* g);
gdouble laplacianGridMG(GridMG* g, GridMG* src);
gdouble plusLaplacianGridMG(GridMG* g, GridMG* src);
gdouble moinsLaplacianGridMG(GridMG* g, GridMG* src);

void averageGridMG(GridMG* g);
void resetLaplacianOrderGridMG(GridMG* g, LaplacianOrderMG order);
void levelUpGridMG(GridMG* g);

GridMG*  getInterpolationTriLinearGridMG(GridMG* src);
void  interpolationTriLinearGridMG(GridMG* g);
GridMG*  getInterpolationCubicGridMG(GridMG* src);
void  interpolationCubicGridMG(GridMG* src);
GridMG*  getProlongationGridMG(GridMG* src);
void  prolongationGridMG(GridMG* g);

void levelDownGridMG(GridMG* g);
GridMG* getRestrictionGridMG(GridMG* src);
void restrictionGridMG(GridMG* g);
GridMG* grtRestrictionInjectionGridMG(GridMG* g, GridMG* src);
void restrictionInjectionGridMG(GridMG* g);

void addGaussian(GridMG* g, gdouble Z, gdouble x1, gdouble y1, gdouble z1, gdouble sigma);


DomainMG getDomainGridMG(GridMG* g);
void setOperationGridMG(GridMG* g, const OperationTypeMG);
OperationTypeMG getOperationGridMG(GridMG* g);
gdouble getValGridMG(GridMG* g, int ix, int iy, int iz);
void setValGridMG(GridMG* g, int ix, int iy, int iz, gdouble v);
void  addValGridMG(GridMG* g, gint ix, gint iy, gint iz, gdouble v);
void  multValGridMG(GridMG* g, gint ix, gint iy, gint iz, gdouble v);
void printGridMG(GridMG* g, const gint ix, const gint iy, const gint iz);
void printMaxGridMG(GridMG* g);
void printFileNameGridMG(GridMG* g, char* fileName);
void printFileGridMG(GridMG* g);
void printMinGridMG(GridMG* g);
void printGridFileGridMG(GridMG*g,FILE*file);
#endif /* __GABEDIT_GRIDMG_H__ */
