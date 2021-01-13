
#ifndef __GABEDIT_MOLCASBASISLIBRARY_H__
#define __GABEDIT_MOLCASBASISLIBRARY_H__

typedef struct _MolcasBasis  MolcasBasis;
typedef struct _MolcasAtom  MolcasAtom;
typedef struct _MolcasOneBasis  MolcasOneBasis;

/************************************/
struct _MolcasOneBasis
{
	gchar* basisName;
	gchar* author;
	gchar* primitive;
	gchar* contraction;
	gchar* ecpType;
};
/************************************/
struct _MolcasAtom
{
	gchar* symbol;
	gint numberOfBasis;
	MolcasOneBasis* basis;
};
/************************************/
struct _MolcasBasis
{
	gint numberOfAtoms;
	MolcasAtom* atoms;
};
void get_molcas_basis_list(gchar* Symb,gchar* outfile,gchar* errfile);
void setMolcasBasisDlg();
void loadMolcasBasis();
void saveMolcasBasis();

#endif /* __GABEDIT_MOLCASBASISLIBRARY_H__ */

