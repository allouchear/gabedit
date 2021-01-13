
#ifndef __GABEDIT_QCHEMTYPES_H__
#define __GABEDIT_QCHEMTYPES_H__

typedef struct _QChemColorBack
{
	GdkColor keyWord;
	GdkColor description;
}QChemColorBack;

typedef struct _QChemColorFore
{
	GdkColor keyWord;
	GdkColor description;
}QChemColorFore;

typedef struct _QChemAtom
{
	gdouble position[3];
	gchar* symbol;
}QChemAtom;

typedef struct _QChemMolecule
{
	gint numberOfAtoms;
	gint totalNumberOfElectrons;
	gint numberOfValenceElectrons;
	QChemAtom* listOfAtoms;
	gchar* auxBasisName;
}QChemMolecule;

typedef struct _QChemBasis
{
	gchar* name;
	gchar* molecule;
	int numberOfBasisTypes;
	gchar** basisNames;
}QChemBasis;

typedef struct _QChemGuessWaveFunction
{
	gboolean fromHCore;
	gchar* method;
	gchar* functional;
	gchar* molecule;
	gchar* basis;
	gchar* memory;
	gint totalCharge;
	gint spinMultiplicity;
}QChemGuessWaveFunction;

typedef enum
{
 LSDAC = 0, PBEC, PW91C, P86C, NewP86C, VWN1LC, VWN1LCRPA, VWN2LC, VWN3LC, VWN4LC, VWN5LC, PW92LC, PZ81LC, LYPC,
 HFX, Xalpha, SlaterX, Becke88X, PBEX, PW86X, PW91X, mPW91_B88X, mPW91_PW91X,mPW91_mPW91X, G96X
} QChemFunctionalType;

typedef enum
{
 XALPHA = 0, HFS, HFB, HFG96, G96LYP, BLYP, SVWN1, SVWN1RPA, SVWN2, SVWN3, 
 SVWN4, SVWN5, SPZ81, SPW92, BPW91, BP86, B3LYP, B3PW91, B3P86, PBE, PW91,
 mPW_PW91_PW91, mPWPW91, mPW1PW91
} QChemStdFunctionalType;

typedef struct _QChemFunctional
{
	QChemFunctionalType type;
	gchar* name;
	gchar* comment;
}QChemFunctional;
typedef struct _QChemStdFunctional
{
	QChemStdFunctionalType type;
	gchar* name;
	gint n;
	QChemFunctionalType* listOfTypes;
	gdouble* coefficients;
}QChemStdFunctional;

#endif /* __GABEDIT_QCHEMTYPES_H__ */
