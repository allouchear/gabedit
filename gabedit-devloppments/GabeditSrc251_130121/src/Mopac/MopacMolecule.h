
#ifndef __GABEDIT_MOPACMOLECULE_H__
#define __GABEDIT_MOPACMOLECULE_H__

void putMopacReactionPathInTextEditor();
gint getMopacMultiplicity();
void initMopacMoleculeButtons();
void initMopacMolecule();
void freeMopacMolecule();
void putMopacMoleculeInfoInTextEditor();
void putMopacChargeMultiplicityInTextEditor();
gboolean setMopacMolecule();
void createMopacChargeMultiplicityFrame(GtkWidget *box);
void createReactionPathFrame(GtkWidget *box);

#endif /* __GABEDIT_MOPACMOLECULE_H__ */

