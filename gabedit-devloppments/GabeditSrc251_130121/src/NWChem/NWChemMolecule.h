
#ifndef __GABEDIT_NWCHEMMOLECULE_H__
#define __GABEDIT_NWCHEMMOLECULE_H__

gint getNWChemMultiplicity();
void initNWChemMoleculeButtons();
void initNWChemMolecule();
void freeNWChemMolecule();
void putNWChemMoleculeInfoInTextEditor();
gboolean setNWChemMolecule();
void createNWChemChargeMultiplicityFrame(GtkWidget *box);

#endif /* __GABEDIT_NWCHEMMOLECULE_H__ */

