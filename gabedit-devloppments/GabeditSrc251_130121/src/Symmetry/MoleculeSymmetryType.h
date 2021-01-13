
#ifndef __GABEDIT_MOLECULESYMMETRYTYPE_H__
#define __GABEDIT_MOLECULESYMMETRYTYPE_H__

#define SYM_H  0x01
#define SYM_V  0x02
#define SYM_D  0x04
#define SYM_I  0x08
#define SYM_S  0x10
#define SYM_DD 0x20
#define SYM_T  0x40
#define SYM_O  0x80
#define SYM_IC 0x100
#define SYM_R  0x200
#define SYM_U  0x400

typedef struct _MolSymAtom
{
	gdouble position[3];
	gint type;
	gdouble eps; /* precision for atom position */
}MolSymAtom;

typedef struct _MolSymMolecule
{
	gint numberOfAtoms;
	gint numberOfDifferentKindsOfAtoms;
	MolSymAtom* listOfAtoms;
	gchar** symbol;
	gint* numberOfAtomsOfEachType;
	gdouble* masse;
}MolSymMolecule;

typedef enum _RotationReflectionOperator
{
	XY_PLANE = 0,
	XZ_PLANE,
	YZ_PLANE,
	POINT_INV,
	ROT2X,
	ROT2Y,
	ROT2Z
} RotationReflectionOperator;

#endif /* __GABEDIT_MOLECULESYMMETRYTYPE_H__ */

