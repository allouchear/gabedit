#ifndef __GABEDIT_GABEDITSPG_H__
#define __GABEDIT_GABEDITSPG_H__

#include "../../spglib/spglib.h"
gboolean crystalloGetNiggli(GList* atoms, gdouble newTv[][3], gdouble symprec);
gboolean crystalloGetDelaunay(GList* atoms, gdouble newTv[][3], gdouble symprec);
/* gboolean crystalloGetPrimitive(GList* atoms, gdouble newTv[][3], gdouble symprec);*/
gint crystalloGetGroupName(GList* atoms, char groupName[], gdouble symprec);
GList* crystalloStandardizeCellSPG(GList* atoms, gint to_primitive, gint no_idealize, gdouble symprec);
GList* crystalloPrimitiveSPG(GList* atoms, gdouble symprec);
SpglibDataset* crystalloGetDataSet(GList* atoms, gdouble symprec);
SpglibDataset* standardizeFromDataSetSPG(Crystal* crystal, gdouble symprec);

#endif
