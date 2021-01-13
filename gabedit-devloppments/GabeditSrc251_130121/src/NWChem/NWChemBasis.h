
#ifndef __GABEDIT_NWCHEMBASIS_H__
#define __GABEDIT_NWCHEMBASIS_H__

void initNWChemBasis();
void freeNWChemBasis();
void addNWChemTypeBasisToTable(GtkWidget *table, gint i, GtkWidget* comboBasis);
GtkWidget* addNWChemBasisToTable(GtkWidget *table, gint i);
void addNWChemAuxBasisToTable(GtkWidget *table, gint i, GtkWidget* comboMethod, GtkWidget* comboExcited);
void putNWChemBasisInfoInTextEditor();
void putNWChemAuxBasisInTextEditor();

#endif /* __GABEDIT_NWCHEMBASIS_H__ */

