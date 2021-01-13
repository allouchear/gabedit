
#ifndef __GABEDIT_QCHEMMOLECULE_H__
#define __GABEDIT_QCHEMMOLECULE_H__

gint getQChemMultiplicity();
void initQChemMoleculeButtons();
void initQChemMolecule();
void freeQChemMolecule();
void putQChemMoleculeInfoInTextEditor();
gboolean setQChemMolecule();
void createQChemChargeMultiplicityFrame(GtkWidget *box);

#endif /* __GABEDIT_QCHEMMOLECULE_H__ */

