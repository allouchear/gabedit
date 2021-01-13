
#ifndef __GABEDIT_MPQCBASISLIBRARY_H__
#define __GABEDIT_MPQCBASISLIBRARY_H__

typedef struct _MPQCBasis  MPQCBasis;
typedef struct _MPQCAtom  MPQCAtom;
typedef struct _MPQCOneBasis  MPQCOneBasis;

/************************************/
struct _MPQCOneBasis
{
	gchar* basisName;
};
/************************************/
struct _MPQCAtom
{
	gchar* symbol;
	gint numberOfBasis;
	MPQCOneBasis* basis;
};
/************************************/
struct _MPQCBasis
{
	gint numberOfAtoms;
	MPQCAtom* atoms;
};
void get_molcas_basis_list(gchar* Symb,gchar* outfile,gchar* errfile);
void setMPQCBasisDlg();
void loadMPQCBasis();
void saveMPQCBasis();

#endif /* __GABEDIT_MPQCBASISLIBRARY_H__ */

