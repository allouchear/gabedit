#ifndef __GABEDIT_GABEDITKPOINTS_H__
#define __GABEDIT_GABEDITKPOINTS_H__

#include "../../spglib/spglib.h"

typedef struct _GabeditKPoint
{ 
	gchar name[20];
	gdouble C[3];
}GabeditKPoint;

typedef struct _GabeditKPointsPath
{ 
	gchar pearsonSymbol[20];
	GList* points;
	GList* path;
	gdouble Q[3][3];
	gdouble Qm1[3][3];
	gchar* strPathPrim;
	gchar* strPathConv;
	gchar* strPointsPrim;
	gchar* strPointsConv;
	gchar* strAtomsPrim;
	gchar* strAtomsConv;
	gchar* spaceGroupName;
	gint spaceGroupNumber;

}GabeditKPointsPath;
gchar* getVASPKPointsPath(Crystal* crystal, gdouble symprec);
GabeditKPointsPath getKpointsPath(Crystal* crystal, gdouble symprec);
gchar* getStrKPointsPath(GabeditKPointsPath* kpointsPath);

#endif
