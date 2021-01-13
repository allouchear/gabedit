
#ifndef __GABEDIT_QCHEMBASIS_H__
#define __GABEDIT_QCHEMBASIS_H__

void initQChemBasis();
void freeQChemBasis();
void addQChemBasisToTable(GtkWidget *table, gint i);
void addQChemBasisGuessToTable(GtkWidget *table, gint i, GtkWidget* comboGuess);
void addQChemAuxBasisToTable(GtkWidget *table, gint i, GtkWidget* comboCorrelation);
void putQChemBasisInfoInTextEditor();
void putQChemAuxBasisInTextEditor();
void putQChemBasis2InTextEditor();

#endif /* __GABEDIT_QCHEMBASIS_H__ */

