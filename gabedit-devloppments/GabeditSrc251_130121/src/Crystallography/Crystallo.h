
#ifndef __GABEDIT_CRYSTALLO_H__
#define __GABEDIT_CRYSTALLO_H__

#include "../Common/Global.h"
#include "../Utils/Constants.h"

typedef struct _CrystalloAtom
{ 
	gchar symbol[20]; 
	gdouble C[3];
	gchar mmType[20]; 
	gchar pdbType[20]; 
	gchar residueName[10]; 
	gint residueNumber;
	gdouble charge;
}CrystalloAtom;

typedef struct _CrystalloSymOp
{ 
	gchar* S[3]; /* M[0]="ax+by+cz+k" string */
	gdouble W[3][3];
	gdouble w[3]; /* R = W r + w */
}CrystalloSymOp;

/* a,b,c and positions are in agnstrom */
typedef struct _Crystal
{ 
	GList* atoms;
	GList* operators;
	gdouble alpha;
	gdouble beta;
	gdouble gamma;
	gdouble a;
	gdouble b;
	gdouble c;
}Crystal;

typedef enum
{
  GABEDIT_CRYSTALLO_REDUCTION_NIGGLI,
  GABEDIT_CRYSTALLO_REDUCTION_DELAUNAY,
  GABEDIT_CRYSTALLO_REDUCTION_PRIMITIVE
} GabeditCrystalloReductionType;

#include "../Crystallography/GabeditKPoints.h"


gboolean crystalloRotate(GList* atoms, gdouble T[][3], gboolean invers);
void crystalloFreeSymOp(gpointer data);
void crystalloFreeAtom(gpointer data);
void initCrystal(Crystal* crystal);
void freeCrystal(Crystal* crystal);
void crystalloInitAtom(CrystalloAtom* a, gchar* symbol);
gdouble crystalloGetDistance2(CrystalloAtom* a1, CrystalloAtom* a2);
gboolean crystalloSmallDistance(CrystalloAtom* a1, CrystalloAtom* a2);
void crystalloPrintAtoms(GList* atoms);
void crystalloPrintSymOp(GList* operators);
void crystalloBuildWwFromStr(CrystalloSymOp* crystalloSymOp);
gboolean crystalloSetAtomsInBox(GList* atoms);
gboolean crystalloSetCartnAtomsInBox(GList* atoms);
gint crystalloNumberOfTv(GList* atoms);
gboolean crystalloAddTvectorsToGeom(Crystal* crystal);
void crystalloPrintNumberOfAtoms(GList* atoms);
gboolean crystalloRemoveAtomsWithSmallDistance(GList** patoms);
gboolean crystalloApplySymOperators(GList** patoms, GList* operators);
gboolean crystalloCartnToFractWw(GList* atoms, gdouble W[][3], gdouble w[]);
gboolean crystalloCartnToFract(GList* atoms);
gboolean crystalloFractToCartn(GList* atoms);
gint crystalloCenter(GList* atoms);
void crystalloRemoveTv(GList** patoms);
gboolean buildSuperCellSimple(GList** patoms, gint nReplicas1, gint nReplicas2, gint nReplicas3);
gint crystalloBuildTablesSymbolsXYZ(GList* atoms, gchar** atomSymbols[], gdouble* positions[]);
gint crystalloNumberOfAtoms(GList* atoms);
gint crystalloGetTv(GList* atoms, gdouble Tv[][3]);
void crystalloCreateCellNano(GList** patoms, gdouble radius);
void crystalloCutPlane(GList** patoms, gdouble direction[], gdouble layer);
void createWulffCluster(GList** patoms, gint nSurfaces, gdouble** surfaces, gdouble* layers);
gdouble crystalloGetVolume(GList* atoms);
void createSlab(GList** patoms, gdouble surface[], gdouble layers[], gdouble emptySpaceSize, gboolean orientXZ);
gboolean crystalloReduceCell(GList** patoms, gdouble symprec, GabeditCrystalloReductionType type);
gint crystalloGetSpaceSymmetryGroup(GList* atoms, char groupName[], gdouble symprec);
gboolean crystalloStandardizeCell(GList** patoms,  gint to_primitive, gint no_idealize, gdouble symprec);
gchar* crystalloGetSymmetryInfo(Crystal* crystal, gdouble symprec);
gboolean crystalloBuildSymOperators(Crystal* crystal, int W[][3][3], double w[][3], int nOp);
gchar* crystalloGetCIFOperators(GList* operators);
gchar* crystalloGetCIF(Crystal* crystal, gdouble symprec, gboolean withSymmetryOperators);
gboolean crystalloComputeLengthsAndAngles(Crystal* crystal);
gboolean generateExtendedPearsonSymbol(char extendedPearsonSymbol[], Crystal* crystal, gdouble symprec);
gchar* crystalloGetVASPAtomsPositions(GList* atoms);
gboolean crystalloAllRecObtuse(GList* atoms, gdouble symprec);
gboolean crystalloPrimitiveCellHinuma(GList** patoms,  gchar* pearsonSymbol);
gboolean crystalloPrimitiveCell(GList** patoms,  gdouble symprec);
gboolean crystalloTranslate(GList* atoms, gdouble T[], gboolean cartn);
gboolean buildSuperCell(GList** patoms, gint P[][3], gdouble p[]);


#endif /* __GABEDIT_CRYSTALLO_H__ */
