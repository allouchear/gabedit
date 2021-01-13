
#ifndef __GABEDIT_MPQCBASIS_H__
#define __GABEDIT_MPQCBASIS_H__

void initMPQCBasis();
void freeMPQCBasis();
void addMPQCBasisToTable(GtkWidget *table, gint i);
void addMPQCBasisGuessToTable(GtkWidget *table, gint i);
void addMPQCAuxBasisToTable(GtkWidget *table, gint i, GtkWidget* comboMethod);
void putMPQCBasisInfoInTextEditor();

#endif /* __GABEDIT_MPQCBASIS_H__ */

