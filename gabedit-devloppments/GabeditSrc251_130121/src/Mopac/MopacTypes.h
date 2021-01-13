
#ifndef __GABEDIT_MOPACTYPES_H__
#define __GABEDIT_MOPACTYPES_H__

typedef struct _MopacColorBack
{
	GdkColor keyWord;
	GdkColor description;
}MopacColorBack;

typedef struct _MopacColorFore
{
	GdkColor keyWord;
	GdkColor description;
}MopacColorFore;

typedef struct _MopacAtom
{
	gdouble position[3];
	gchar* symbol;
}MopacAtom;

typedef struct _MopacMolecule
{
	gint numberOfAtoms;
	gint totalNumberOfElectrons;
	gint numberOfValenceElectrons;
	MopacAtom* listOfAtoms;
	gchar* auxBasisName;
}MopacMolecule;

typedef enum
{
 LSDAC = 0, PBEC, PW91C, P86C, NewP86C, VWN1LC, VWN1LCRPA, VWN2LC, VWN3LC, VWN4LC, VWN5LC, PW92LC, PZ81LC, LYPC,
 HFX, Xalpha, SlaterX, Becke88X, PBEX, PW86X, PW91X, mPW91_B88X, mPW91_PW91X,mPW91_mPW91X, G96X
} MopacFunctionalType;

typedef enum
{
 XALPHA = 0, HFS, HFB, HFG96, G96LYP, BLYP, SVWN1, SVWN1RPA, SVWN2, SVWN3, 
 SVWN4, SVWN5, SPZ81, SPW92, BPW91, BP86, B3LYP, B3PW91, B3P86, PBE, PW91,
 mPW_PW91_PW91, mPWPW91, mPW1PW91
} MopacStdFunctionalType;

typedef struct _MopacFunctional
{
	MopacFunctionalType type;
	gchar* name;
	gchar* comment;
}MopacFunctional;
typedef struct _MopacStdFunctional
{
	MopacStdFunctionalType type;
	gchar* name;
	gint n;
	MopacFunctionalType* listOfTypes;
	gdouble* coefficients;
}MopacStdFunctional;

#endif /* __GABEDIT_MOPACTYPES_H__ */
