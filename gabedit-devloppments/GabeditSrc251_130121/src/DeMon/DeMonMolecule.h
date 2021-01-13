
#ifndef __GABEDIT_DEMONMOLECULE_H__
#define __GABEDIT_DEMONMOLECULE_H__

gint getDeMonMultiplicity();
void initDeMonMoleculeButtons();
void initDeMonMolecule();
void freeDeMonMolecule();
void putDeMonMoleculeInfoInTextEditor();
gboolean setDeMonMolecule();
void createDeMonChargeMultiplicityFrame(GtkWidget *box);

#endif /* __GABEDIT_DEMONMOLECULE_H__ */

