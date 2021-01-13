
#ifndef __GABEDIT_ORCAMOLECULE_H__
#define __GABEDIT_ORCAMOLECULE_H__

gint getOrcaMultiplicity();
void initOrcaMoleculeButtons();
void initOrcaMolecule();
void freeOrcaMolecule();
void putOrcaMoleculeInfoInTextEditor();
gboolean setOrcaMolecule();
void createOrcaChargeMultiplicityFrame(GtkWidget *box);

#endif /* __GABEDIT_ORCAMOLECULE_H__ */

