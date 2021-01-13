
#ifndef __GABEDIT_MPQCTYPES_H__
#define __GABEDIT_MPQCTYPES_H__

typedef struct _MPQCColorBack
{
	GdkColor keyWord;
	GdkColor description;
}MPQCColorBack;

typedef struct _MPQCColorFore
{
	GdkColor keyWord;
	GdkColor description;
}MPQCColorFore;

typedef struct _MPQCAtom
{
	gdouble position[3];
	gchar* symbol;
}MPQCAtom;

typedef struct _MPQCMolecule
{
	gint numberOfAtoms;
	gint totalNumberOfElectrons;
	gint numberOfValenceElectrons;
	MPQCAtom* listOfAtoms;
	gchar* groupSymmetry;
}MPQCMolecule;

typedef struct _MPQCBasis
{
	gchar* name;
	gchar* molecule;
	int numberOfBasisTypes;
	gchar** basisNames;
}MPQCBasis;

typedef struct _MPQCGuessWaveFunction
{
	gboolean fromHCore;
	gchar* method;
	gchar* functional;
	gchar* molecule;
	gchar* basis;
	gchar* memory;
	gint totalCharge;
	gint spinMultiplicity;
}MPQCGuessWaveFunction;

typedef struct _MPQCOptimization
{
	gchar* method;
	gboolean transitionState;
	gboolean modeFollowing;
	gint maxIterations;
	gchar* updateMethod;
	gchar* function;
	gchar* energy;
	gboolean cartesian;
}MPQCOptimization;

typedef struct _MPQCMole
{
	gchar* method;
	gchar* functional;
	gchar* molecule;
	gchar* basis;
	gchar* auxBasisName;
	gchar* memory;
	gint totalCharge;
	gint spinMultiplicity;
	gboolean printNPA; /* natural population analysis will be printed. */
	gboolean printNAO; /* natural atomic orbitals will be printed. */
	gchar* guessWaveFunction;
}MPQCMole;
typedef struct _MPQCMpqc
{
	gboolean do_gradient;
	gboolean optimize;
	gboolean restart;
	gboolean checkpoint;
	gboolean savestate;
	gchar* mole;
	gchar* opt;
	gboolean frequencies;
}MPQCMpqc;

typedef enum
{
 LSDAC = 0, PBEC, PW91C, P86C, NewP86C, VWN1LC, VWN1LCRPA, VWN2LC, VWN3LC, VWN4LC, VWN5LC, PW92LC, PZ81LC, LYPC,
 HFX, Xalpha, SlaterX, Becke88X, PBEX, PW86X, PW91X, mPW91_B88X, mPW91_PW91X,mPW91_mPW91X, G96X
} MPQCFunctionalType;

typedef enum
{
 XALPHA = 0, HFS, HFB, HFG96, G96LYP, BLYP, SVWN1, SVWN1RPA, SVWN2, SVWN3, 
 SVWN4, SVWN5, SPZ81, SPW92, BPW91, BP86, B3LYP, B3PW91, B3P86, PBE, PW91,
 mPW_PW91_PW91, mPWPW91, mPW1PW91
} MPQCStdFunctionalType;

typedef struct _MPQCFunctional
{
	MPQCFunctionalType type;
	gchar* name;
	gchar* comment;
}MPQCFunctional;
typedef struct _MPQCStdFunctional
{
	MPQCStdFunctionalType type;
	gchar* name;
	gint n;
	MPQCFunctionalType* listOfTypes;
	gdouble* coefficients;
}MPQCStdFunctional;

#endif /* __GABEDIT_MPQCTYPES_H__ */
