
#ifndef __GABEDIT_MOLPROBASISLIBRARY_H__
#define __GABEDIT_MOLPROBASISLIBRARY_H__

typedef struct _MolproBasis  MolproBasis;
typedef struct _MolproAtom  MolproAtom;
typedef struct _MolproOneBasis  MolproOneBasis;

/************************************/
struct _MolproOneBasis
{
	gchar* name;
	gboolean pseudo;
	gboolean orbitals[21];
};
/************************************/
struct _MolproAtom
{
	gchar* symbol;
	gint numberOfBasis;
	MolproOneBasis* basis;
};
/************************************/
struct _MolproBasis
{
	gint numberOfAtoms;
	MolproAtom* atoms;
};
void get_molpro_basis_list(gchar* Symb,gchar* orb,gchar* outfile,gchar* errfile);
void setMolproBasisDlg();
void loadMolproBasis();
void saveMolproBasis();

#endif /* __GABEDIT_MOLPROBASISLIBRARY_H__ */

