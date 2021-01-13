
#ifndef __GABEDIT_UTILSCIF_H__
#define __GABEDIT_UTILSCIF_H__

#include "../Crystallography/Crystallo.h"

gchar* read_geometry_from_cif_fileOld(gchar* fileName, gint* pnAtoms, gchar** atomSymbols[], gdouble* positions[], gboolean applySymOp);
gchar* read_geometry_from_cif_file(Crystal* crystal, gchar* fileName, gboolean applySymOp);

#endif /* __GABEDIT_UTILSCIF_H__ */
