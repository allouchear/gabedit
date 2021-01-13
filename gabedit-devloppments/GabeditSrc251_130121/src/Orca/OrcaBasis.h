
#ifndef __GABEDIT_ORCABASIS_H__
#define __GABEDIT_ORCABASIS_H__

void initOrcaBasis();
void freeOrcaBasis();
void addOrcaTypeBasisToTable(GtkWidget *table, gint i, GtkWidget* comboBasis);
GtkWidget* addOrcaBasisToTable(GtkWidget *table, gint i);
void addOrcaAuxBasisToTable(GtkWidget *table, gint i, GtkWidget* comboMethod, GtkWidget* comboExcited);
void putOrcaBasisInfoInTextEditor();
void putOrcaAuxBasisInTextEditor();

#endif /* __GABEDIT_ORCABASIS_H__ */

