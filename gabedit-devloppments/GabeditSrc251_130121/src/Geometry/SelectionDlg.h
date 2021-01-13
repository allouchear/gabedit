
#ifndef __GABEDIT_SELECTION_DLG_H__
#define __GABEDIT_SELECTION_DLG_H__

void selectionDlg();
void SelectFixedVariableAtoms(gboolean variable);
void SelectFirstResidue();
void SelectLastResidue();
void selectResidueByNameDlg();
void selectResidueByNumberDlg();
void selectAtomsByMMTypeDlg();
void selectAtomsByPDBTypeDlg();
void selectAtomsBySymbolDlg();
void selectAtomsByChargeValues(gboolean positive);
void SelectAllAtoms();
void InvertSelectionOfAtoms();
void unSelectAllAtoms();
void SelectLayerAtoms(GabEditLayerType layer);
void selectAtomsBySphereDlg();
gchar** getListResidueNumbers(gint* nlist, gint**nums);
gchar** getListResidues(gint* nlist);
gchar** getListGeomMMTypes(gint* nlist);
gchar** getListPDBTypesFromGeom(gint* nlist);
gchar** getListSymbols(gint* nlist);

#endif /* __GABEDIT_SELECTION_DLG_H__ */

