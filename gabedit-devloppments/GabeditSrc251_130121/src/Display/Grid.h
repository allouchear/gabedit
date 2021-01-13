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

#ifndef __GABEDIT_GRID_H__
#define __GABEDIT_GRID_H__
#include "../MultiGrid/PoissonMG.h"

typedef struct _Point5
{
	gdouble C[5];
}Point5;
typedef struct _GridLimits
{
	gdouble MinMax[2][4];
}GridLimits;

typedef struct _Grid
{
	gint N[3];
	Point5 ***point;
	GridLimits limits;
	gboolean mapped;
}Grid;

extern GridLimits limits;
extern gint NumPoints[3];
extern gdouble firstDirection[3];
extern gdouble secondDirection[3];
extern gdouble thirdDirection[3];
extern gdouble originOfCube[3];

gdouble get_value_orbital(gdouble x,gdouble y,gdouble z,gint k);
gdouble get_value_electronic_density_on_atom(gdouble x,gdouble y,gdouble z,gint n);
gdouble get_value_electronic_density_atomic(gdouble x,gdouble y,gdouble z,gint dump);
gdouble get_value_electronic_density(gdouble x,gdouble y,gdouble z,gint dump);
gdouble get_value_electronic_density_bonds(gdouble x,gdouble y,gdouble z,gint dump);
gdouble get_value_spin_density(gdouble x,gdouble y,gdouble z,gint dump);
gboolean test_grid_all_positive(Grid* grid);
Grid* grid_point_alloc(gint N[],GridLimits limits);
Grid* define_grid(gint N[],GridLimits limits);
Grid* free_grid(Grid* grid);

Grid* get_grid_laplacian(Grid* grid, gint nBoundary);
Grid* get_grid_norm_gradient(Grid* grid, gint nBoundary);
Grid* get_grid_sign_lambda2_density(Grid* grid, gint nBoundary);
Grid* compute_mep_grid_using_multipol_from_density_grid(Grid* grid, gint lmax);
gdouble** compute_multipol_from_grid(Grid* grid, gint lmax, gdouble xOff, gdouble yOff, gdouble zOff);
Grid* compute_mep_grid_using_multipol_from_orbitals(gint N[],GridLimits limits, gint lmax);
Grid* compute_mep_grid_using_partial_charges_cube_grid(Grid* grid);
Grid* compute_mep_grid_using_partial_charges(gint N[],GridLimits limits);
Grid* solve_poisson_equation_from_density_grid(Grid* grid, PoissonSolverMethod psMethod);
Grid* solve_poisson_equation_from_orbitals(gint N[],GridLimits limits, PoissonSolverMethod psMethod);
gboolean compute_coulomb_integrale(Grid* grid);
gboolean compute_coulomb_integrale_iijj(gint N[],GridLimits limits, gint typeOrbi, gint i, gint typeOrbj, gint j,
		gdouble* pInteg, gdouble* pNormi, gdouble* pNormj, gdouble* pOverlap);
gboolean compute_coulomb_integrale_iijj_poisson(gint N[],GridLimits limits, gint typeOrbi, gint i, gint typeOrbj, gint j,
		gdouble* pInteg, gdouble* pNorm, gdouble* pNormj, gdouble* pOverlap);
Grid* define_grid_electronic_density(gint N[],GridLimits limits);
Grid* define_grid_ELFBECKE(gint N[],GridLimits limits);
Grid* define_grid_ELFSAVIN(gint N[],GridLimits limits);
Grid* define_grid_FED(gint N[],GridLimits limits, gint n);
Grid* compute_fed_grid_using_cube_grid(Grid* grid, gint n);
gboolean compute_transition_matrix_numeric(gint N[],GridLimits limits, gint typeOrbi, gint i, gint typeOrbj, gint j,
gdouble* pInteg, gdouble* pNormi, gdouble* pNormj, gdouble* pOverlap);
gboolean compute_spatial_overlapiijj_numeric(gint N[],GridLimits limits, gint typeOrbi, gint i, gint typeOrbj, gint j,
		gdouble* pInteg, gdouble* pNormi, gdouble* pNormj, gdouble* pOverlap);
gboolean compute_spatial_overlapij_numeric(gint N[],GridLimits limits, gint typeOrbi, gint i, gint typeOrbj, gint j,
		gdouble* pInteg, gdouble* pNormi, gdouble* pNormj, gdouble* pOverlap);
gboolean compute_isovalue_percent_from_grid(Grid* grid, gboolean square, gdouble percent, gdouble precision, gdouble* pIsovalue);
Grid* copyGrid(Grid* grid);
Grid* compute_mep_grid_exact(gint N[],GridLimits limits);
Grid* compute_mep_grid_using_orbitals(gint N[],GridLimits limits);
void reset_boundary(Grid* grid, gint nBoundary);
gdouble getLambda2(Grid* grid, gint i, gint j, gint k, gdouble* fcx, gdouble* fcy, gdouble* fcz, gdouble* lfcx, gdouble* lfcy, gdouble* lfcz, gint nBoundary);
gboolean get_charge_transfer_centers(Grid* grid, gdouble* CN, gdouble* CP, gdouble *qn, gdouble* qp, gdouble* H);
gboolean compute_integrale_from_grid_all_space(Grid* grid, gdouble* pInteg);
gdouble get_value_CBTF(gdouble x,gdouble y,gdouble z,gint i);

#endif /* __GABEDIT_GRID_H__ */

