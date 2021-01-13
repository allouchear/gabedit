
#ifndef __GABEDIT_NWCHEMTYPES_H__
#define __GABEDIT_NWCHEMTYPES_H__

typedef struct _NWChemColorBack
{
	GdkColor keyWord;
	GdkColor description;
}NWChemColorBack;

typedef struct _NWChemColorFore
{
	GdkColor keyWord;
	GdkColor description;
}NWChemColorFore;

typedef struct _NWChemAtom
{
	gdouble position[3];
	gchar* symbol;
}NWChemAtom;

typedef struct _NWChemMolecule
{
	gint numberOfAtoms;
	gint totalNumberOfElectrons;
	gint numberOfValenceElectrons;
	NWChemAtom* listOfAtoms;
	gchar* auxBasisName;
}NWChemMolecule;

typedef struct _NWChemBasis
{
	gchar* name;
	gchar* molecule;
	int numberOfBasisTypes;
	gchar** basisNames;
}NWChemBasis;

typedef struct _NWChemGuessWaveFunction
{
	gboolean fromHCore;
	gchar* method;
	gchar* functional;
	gchar* molecule;
	gchar* basis;
	gchar* memory;
	gint totalCharge;
	gint spinMultiplicity;
}NWChemGuessWaveFunction;

typedef enum
{
 LSDAC = 0, PBEC, PW91C, P86C, NewP86C, VWN1LC, VWN1LCRPA, VWN2LC, VWN3LC, VWN4LC, VWN5LC, PW92LC, PZ81LC, LYPC,
 HFX, Xalpha, SlaterX, Becke88X, PBEX, PW86X, PW91X, mPW91_B88X, mPW91_PW91X,mPW91_mPW91X, G96X
} NWChemFunctionalType;

typedef enum
{
 XALPHA = 0, HFS, HFB, HFG96, G96LYP, BLYP, SVWN1, SVWN1RPA, SVWN2, SVWN3, 
 SVWN4, SVWN5, SPZ81, SPW92, BPW91, BP86, B3LYP, B3PW91, B3P86, PBE, PW91,
 mPW_PW91_PW91, mPWPW91, mPW1PW91
} NWChemStdFunctionalType;

typedef struct _NWChemFunctional
{
	NWChemFunctionalType type;
	gchar* name;
	gchar* comment;
}NWChemFunctional;
typedef struct _NWChemStdFunctional
{
	NWChemStdFunctionalType type;
	gchar* name;
	gint n;
	NWChemFunctionalType* listOfTypes;
	gdouble* coefficients;
}NWChemStdFunctional;

#endif /* __GABEDIT_NWCHEMTYPES_H__ */
