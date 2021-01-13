
#ifndef __GABEDIT_PSICODETYPES_H__
#define __GABEDIT_PSICODETYPES_H__

typedef struct _PsicodeColorBack
{
	GdkColor keyWord;
	GdkColor description;
}PsicodeColorBack;

typedef struct _PsicodeColorFore
{
	GdkColor keyWord;
	GdkColor description;
}PsicodeColorFore;

typedef struct _PsicodeAtom
{
	gdouble position[3];
	gchar* symbol;
}PsicodeAtom;

typedef struct _PsicodeMolecule
{
	gint numberOfAtoms;
	gint totalNumberOfElectrons;
	gint numberOfValenceElectrons;
	PsicodeAtom* listOfAtoms;
	gchar* auxBasisName;
}PsicodeMolecule;

typedef struct _PsicodeBasis
{
	gchar* name;
	gchar* molecule;
	int numberOfBasisTypes;
	gchar** basisNames;
}PsicodeBasis;

typedef struct _PsicodeGuessWaveFunction
{
	gboolean fromHCore;
	gchar* method;
	gchar* functional;
	gchar* molecule;
	gchar* basis;
	gchar* memory;
	gint totalCharge;
	gint spinMultiplicity;
}PsicodeGuessWaveFunction;

typedef enum
{
 LSDAC = 0, PBEC, PW91C, P86C, NewP86C, VWN1LC, VWN1LCRPA, VWN2LC, VWN3LC, VWN4LC, VWN5LC, PW92LC, PZ81LC, LYPC,
 HFX, Xalpha, SlaterX, Becke88X, PBEX, PW86X, PW91X, mPW91_B88X, mPW91_PW91X,mPW91_mPW91X, G96X
} PsicodeFunctionalType;

typedef enum
{
 XALPHA = 0, HFS, HFB, HFG96, G96LYP, BLYP, SVWN1, SVWN1RPA, SVWN2, SVWN3, 
 SVWN4, SVWN5, SPZ81, SPW92, BPW91, BP86, B3LYP, B3PW91, B3P86, PBE, PW91,
 mPW_PW91_PW91, mPWPW91, mPW1PW91
} PsicodeStdFunctionalType;

typedef struct _PsicodeFunctional
{
	PsicodeFunctionalType type;
	gchar* name;
	gchar* comment;
}PsicodeFunctional;
typedef struct _PsicodeStdFunctional
{
	PsicodeStdFunctionalType type;
	gchar* name;
	gint n;
	PsicodeFunctionalType* listOfTypes;
	gdouble* coefficients;
}PsicodeStdFunctional;

#endif /* __GABEDIT_PSICODETYPES_H__ */
