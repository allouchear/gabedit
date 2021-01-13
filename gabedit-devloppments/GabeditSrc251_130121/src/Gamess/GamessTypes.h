
#ifndef __GABEDIT_GAMESSTYPES_H__
#define __GABEDIT_GAMESSTYPES_H__

typedef struct _GamessColorBack
{
	GdkColor keyWord;
	GdkColor description;
}GamessColorBack;

typedef struct _GamessColorFore
{
	GdkColor keyWord;
	GdkColor description;
}GamessColorFore;

typedef struct _GamessAtom
{
	gdouble position[3];
	gchar* symbol;
}GamessAtom;

typedef struct _GamessMolecule
{
	gint numberOfAtoms;
	gint totalNumberOfElectrons;
	gint numberOfValenceElectrons;
	GamessAtom* listOfAtoms;
	gchar* groupSymmetry;
}GamessMolecule;

typedef struct _GamessBasis
{
	gchar* name;
	gchar* molecule;
	int numberOfBasisTypes;
	gchar** basisNames;
}GamessBasis;

typedef enum
{
 LSDAC = 0, PBEC, PW91C, P86C, NewP86C, VWN1LC, VWN1LCRPA, VWN2LC, VWN3LC, VWN4LC, VWN5LC, PW92LC, PZ81LC, LYPC,
 HFX, Xalpha, SlaterX, Becke88X, PBEX, PW86X, PW91X, mPW91_B88X, mPW91_PW91X,mPW91_mPW91X, G96X
} GamessFunctionalType;

typedef enum
{
 XALPHA = 0, HFS, HFB, HFG96, G96LYP, BLYP, SVWN1, SVWN1RPA, SVWN2, SVWN3, 
 SVWN4, SVWN5, SPZ81, SPW92, BPW91, BP86, B3LYP, B3PW91, B3P86, PBE, PW91,
 mPW_PW91_PW91, mPWPW91, mPW1PW91
} GamessStdFunctionalType;

typedef struct _GamessFunctional
{
	GamessFunctionalType type;
	gchar* name;
	gchar* comment;
}GamessFunctional;
typedef struct _GamessStdFunctional
{
	GamessStdFunctionalType type;
	gchar* name;
	gint n;
	GamessFunctionalType* listOfTypes;
	gdouble* coefficients;
}GamessStdFunctional;

#endif /* __GABEDIT_GAMESSTYPES_H__ */
