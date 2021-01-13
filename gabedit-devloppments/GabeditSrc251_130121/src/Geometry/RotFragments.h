
#ifndef __GABEDIT_ROTFRAGMENTS_H__
#define __GABEDIT_ROTFRAGMENTS_H__

gdouble AngleAboutXAxis(gdouble* coord);
gdouble AngleAboutYAxis(gdouble* coord);
gdouble AngleAboutZAxis(gdouble* coord);
gdouble AngleToAtoms(gdouble*C3,gdouble* C1,gdouble* C2);
gdouble TorsionToAtoms(gdouble*C4,gdouble*C1,gdouble* C2,gdouble* C3);
void SetBondDistance(GeomDef* geom,gint n1,gint n2,gdouble bondLength,gint list[],gint nlist);
void SetBondDistanceX(GeomDef* geom,gint n1,gint n2,gdouble bondLength,gint list[],gint nlist);
void SetTorsion(gint Natoms,GeomDef *geometry, gint a1num, gint a2num, gint a3num, gint a4num, gdouble torsion,gint atomList[], gint numberOfElements );
void SetAngle(gint Natoms,GeomDef *geometry, gint a1num, gint a2num, gint a3num, gdouble angle,gint atomList[], gint numberOfElements );
void SetOposedBondeds(gint Natoms,GeomDef *geometry, gint ns, gint toBond,gint atomList[], gint numberOfElements );

#endif /* __GABEDIT_ROTFRAGMENTS_H__ */

