
#ifndef __GABEDIT_ORCATYPES_H__
#define __GABEDIT_ORCATYPES_H__

typedef struct _OrcaColorBack
{
	GdkColor keyWord;
	GdkColor description;
}OrcaColorBack;

typedef struct _OrcaColorFore
{
	GdkColor keyWord;
	GdkColor description;
}OrcaColorFore;

typedef struct _OrcaAtom
{
	gdouble position[3];
	gchar* symbol;
}OrcaAtom;

typedef struct _OrcaMolecule
{
	gint numberOfAtoms;
	gint totalNumberOfElectrons;
	gint numberOfValenceElectrons;
	OrcaAtom* listOfAtoms;
	gchar* auxBasisName;
}OrcaMolecule;

typedef struct _OrcaBasis
{
	gchar* name;
	gchar* molecule;
	int numberOfBasisTypes;
	gchar** basisNames;
}OrcaBasis;

typedef struct _OrcaGuessWaveFunction
{
	gboolean fromHCore;
	gchar* method;
	gchar* functional;
	gchar* molecule;
	gchar* basis;
	gchar* memory;
	gint totalCharge;
	gint spinMultiplicity;
}OrcaGuessWaveFunction;

typedef enum
{
 LSDAC = 0, PBEC, PW91C, P86C, NewP86C, VWN1LC, VWN1LCRPA, VWN2LC, VWN3LC, VWN4LC, VWN5LC, PW92LC, PZ81LC, LYPC,
 HFX, Xalpha, SlaterX, Becke88X, PBEX, PW86X, PW91X, mPW91_B88X, mPW91_PW91X,mPW91_mPW91X, G96X
} OrcaFunctionalType;

typedef enum
{
 XALPHA = 0, HFS, HFB, HFG96, G96LYP, BLYP, SVWN1, SVWN1RPA, SVWN2, SVWN3, 
 SVWN4, SVWN5, SPZ81, SPW92, BPW91, BP86, B3LYP, B3PW91, B3P86, PBE, PW91,
 mPW_PW91_PW91, mPWPW91, mPW1PW91
} OrcaStdFunctionalType;

typedef struct _OrcaFunctional
{
	OrcaFunctionalType type;
	gchar* name;
	gchar* comment;
}OrcaFunctional;
typedef struct _OrcaStdFunctional
{
	OrcaStdFunctionalType type;
	gchar* name;
	gint n;
	OrcaFunctionalType* listOfTypes;
	gdouble* coefficients;
}OrcaStdFunctional;

#endif /* __GABEDIT_ORCATYPES_H__ */
