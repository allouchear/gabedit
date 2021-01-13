
#ifndef __GABEDIT_DEMONTYPES_H__
#define __GABEDIT_DEMONTYPES_H__

typedef struct _DeMonColorBack
{
	GdkColor keyWord;
	GdkColor description;
}DeMonColorBack;

typedef struct _DeMonColorFore
{
	GdkColor keyWord;
	GdkColor description;
}DeMonColorFore;

typedef struct _DeMonAtom
{
	gdouble position[3];
	gchar* symbol;
}DeMonAtom;

typedef struct _DeMonMolecule
{
	gint numberOfAtoms;
	gint totalNumberOfElectrons;
	gint numberOfValenceElectrons;
	DeMonAtom* listOfAtoms;
	gchar* auxBasisName;
}DeMonMolecule;

typedef struct _DeMonBasis
{
	gchar* name;
	gchar* molecule;
	int numberOfBasisTypes;
	gchar** basisNames;
}DeMonBasis;

typedef struct _DeMonGuessWaveFunction
{
	gboolean fromHCore;
	gchar* method;
	gchar* functional;
	gchar* molecule;
	gchar* basis;
	gchar* memory;
	gint totalCharge;
	gint spinMultiplicity;
}DeMonGuessWaveFunction;

typedef enum
{
 LSDAC = 0, PBEC, PW91C, P86C, NewP86C, VWN1LC, VWN1LCRPA, VWN2LC, VWN3LC, VWN4LC, VWN5LC, PW92LC, PZ81LC, LYPC,
 HFX, Xalpha, SlaterX, Becke88X, PBEX, PW86X, PW91X, mPW91_B88X, mPW91_PW91X,mPW91_mPW91X, G96X
} DeMonFunctionalType;

typedef enum
{
 XALPHA = 0, HFS, HFB, HFG96, G96LYP, BLYP, SVWN1, SVWN1RPA, SVWN2, SVWN3, 
 SVWN4, SVWN5, SPZ81, SPW92, BPW91, BP86, B3LYP, B3PW91, B3P86, PBE, PW91,
 mPW_PW91_PW91, mPWPW91, mPW1PW91
} DeMonStdFunctionalType;

typedef struct _DeMonFunctional
{
	DeMonFunctionalType type;
	gchar* name;
	gchar* comment;
}DeMonFunctional;
typedef struct _DeMonStdFunctional
{
	DeMonStdFunctionalType type;
	gchar* name;
	gint n;
	DeMonFunctionalType* listOfTypes;
	gdouble* coefficients;
}DeMonStdFunctional;

#endif /* __GABEDIT_DEMONTYPES_H__ */
