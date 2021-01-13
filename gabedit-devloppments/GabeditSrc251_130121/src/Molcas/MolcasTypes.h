
#ifndef __GABEDIT_MOLCASTYPES_H__
#define __GABEDIT_MOLCASTYPES_H__

typedef struct _MolcasAtom
{
	gdouble position[3];
	gint type;
}MolcasAtom;

typedef struct _MolcasMolecule
{
	gint numberOfAtoms;
	gint totalNumberOfElectrons;
	gint numberOfValenceElectrons;
	gint numberOfDifferentKindsOfAtoms;
	MolcasAtom* listOfAtoms;
	gchar** symbol;
	gchar* groupSymbol;
	gint* numberOfAtomsOfEachType;
	gint* numberOfAtomsOfEachTypeForAllAtoms;
	gint numberOfMolcasGenerators;
	gchar* molcasGenerators[3];
	gchar** basis;
}MolcasMolecule;

typedef struct _MolcasOptimization
{
	gint numberOfIterations;
}MolcasOptimization;

typedef struct _MolcasSolvation
{
	gchar method[100];
	gchar solvent[100];
}MolcasSolvation;

typedef struct _MolcasScf
{
	gchar title[1024];

	/* RHF, UHF, BLYP, B3LYP, B3LYP5, HFB, HFS, 
	   LDA, LDA5, LSDA, LSDA5, SVWN, SVWN5, TLYP, XPBE 
	*/
	gchar method[100];
	gchar type[100];

	/* You can use one and only one for  Charge, AUFBAU or OCCUPIED  Keyword*/
	gint typeOfOcupations; /* 0-> Default, 1-> charge KeyWord 2-> AUFBAU, 3-> Occupied Keyword */
	gint charge[2]; /* Charge Key Word  : charge and m parameter */
	gint aufBau[2]; /* AUFBAU  Key Word : number of electrons pair and m parameter */
	gint numberOfOccupedOrbitals[8]; /* OCCUPIED  (of alpha for UHF)°*/
	gint numberOfOccupedOrbitalsBeta[8]; /* beta number of Orbitals occuped for UHF */

	gint numberOfOrbitals;
	gint numberOfOrbitalsBeta;
	gdouble* occupationOrbitals;  /* Occupation (0 to 2 ) by orbitals(of alpha for UHF)°*/
	gdouble* occupationOrbitalsBeta; /* beta occupations for UHF */


	gint numberOfNDDOIterations;
	gint numberOfRHFIterations;
	gdouble convergenceThresholds[4]; /* EThr   DThr   FThr   DltNTh */
	gdouble diskSize[2];/* Semi-direct algorithm in Mb and size of the I/O buffer*/
	gboolean ivo; /* Improve the virtuals for MCSCF */

	/* for UHF program */
	gint zSpin; /* the default value is 0 (1) for a system with a even (odd) number of electrons */

	gboolean guessOrbitalsCore; /* CORE OR NDDO */
}MolcasScf;

typedef struct _MolcasColorBack
{
	GdkColor program;
	GdkColor shellCommand;
	GdkColor internCommand;
}MolcasColorBack;

typedef struct _MolcasColorFore
{
	GdkColor program;
	GdkColor shellCommand;
	GdkColor internCommand;
	GdkColor subProgram;
}MolcasColorFore;

typedef struct _MolcasSystemVariables
{
	gchar mem[100];
	gchar disk[100];
	gchar ramd[100];
	gchar trap[100];
	gchar workDir[100];
}MolcasSystemVariables;

typedef struct _MolcasGridIT
{
	gboolean ascii;
	gboolean all;
}MolcasGridIT;

#endif /* __GABEDIT_MOLCASTYPES_H__ */

