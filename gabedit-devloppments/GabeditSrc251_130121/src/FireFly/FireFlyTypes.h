
#ifndef __GABEDIT_GAMESSTYPES_H__
#define __GABEDIT_GAMESSTYPES_H__

typedef struct _FireFlyColorBack
{
	GdkColor keyWord;
	GdkColor description;
}FireFlyColorBack;

typedef struct _FireFlyColorFore
{
	GdkColor keyWord;
	GdkColor description;
}FireFlyColorFore;

typedef struct _FireFlyAtom
{
	gdouble position[3];
	gchar* symbol;
}FireFlyAtom;

typedef struct _FireFlyMolecule
{
	gint numberOfAtoms;
	gint totalNumberOfElectrons;
	gint numberOfValenceElectrons;
	FireFlyAtom* listOfAtoms;
	gchar* groupSymmetry;
}FireFlyMolecule;

typedef struct _FireFlyBasis
{
	gchar* name;
	gchar* molecule;
	int numberOfBasisTypes;
	gchar** basisNames;
}FireFlyBasis;

typedef enum
{
 LSDAC = 0, PBEC, PW91C, P86C, NewP86C, VWN1LC, VWN1LCRPA, VWN2LC, VWN3LC, VWN4LC, VWN5LC, PW92LC, PZ81LC, LYPC,
 HFX, Xalpha, SlaterX, Becke88X, PBEX, PW86X, PW91X, mPW91_B88X, mPW91_PW91X,mPW91_mPW91X, G96X
} FireFlyFunctionalType;

typedef enum
{
 XALPHA = 0, HFS, HFB, HFG96, G96LYP, BLYP, SVWN1, SVWN1RPA, SVWN2, SVWN3, 
 SVWN4, SVWN5, SPZ81, SPW92, BPW91, BP86, B3LYP, B3PW91, B3P86, PBE, PW91,
 mPW_PW91_PW91, mPWPW91, mPW1PW91
} FireFlyStdFunctionalType;

typedef struct _FireFlyFunctional
{
	FireFlyFunctionalType type;
	gchar* name;
	gchar* comment;
}FireFlyFunctional;
typedef struct _FireFlyStdFunctional
{
	FireFlyStdFunctionalType type;
	gchar* name;
	gint n;
	FireFlyFunctionalType* listOfTypes;
	gdouble* coefficients;
}FireFlyStdFunctional;

#endif /* __GABEDIT_GAMESSTYPES_H__ */
