
#ifndef __GABEDIT_PSICODEBASIS_H__
#define __GABEDIT_PSICODEBASIS_H__

void initPsicodeBasis();
void freePsicodeBasis();
void addPsicodeTypeBasisToTable(GtkWidget *table, gint i, GtkWidget* comboBasis);
GtkWidget* addPsicodeBasisToTable(GtkWidget *table, gint i);
void addPsicodeAuxBasisToTable(GtkWidget *table, gint i, GtkWidget* comboMethod, GtkWidget* comboExcited);
void putPsicodeBasisInfoInTextEditor();
void putPsicodeAuxBasisInTextEditor();

#endif /* __GABEDIT_PSICODEBASIS_H__ */

