
#ifndef __GABEDIT_DEMONBASIS_H__
#define __GABEDIT_DEMONBASIS_H__

void initDeMonBasis();
void freeDeMonBasis();
void addDeMonTypeBasisToTable(GtkWidget *table, gint i, GtkWidget* comboBasis);
GtkWidget* addDeMonBasisToTable(GtkWidget *table, gint i);
void addDeMonAuxBasisToTable(GtkWidget *table, gint i, GtkWidget* comboMethod, GtkWidget* comboExcited);
void putDeMonBasisInfoInTextEditor();
void putDeMonAuxBasisInTextEditor();

#endif /* __GABEDIT_DEMONBASIS_H__ */

