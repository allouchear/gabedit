
#ifndef __GABEDIT_PSICODEMOLECULE_H__
#define __GABEDIT_PSICODEMOLECULE_H__

gint getPsicodeMultiplicity();
void initPsicodeMoleculeButtons();
void initPsicodeMolecule();
void freePsicodeMolecule();
void putPsicodeMoleculeInfoInTextEditor();
gboolean setPsicodeMolecule();
void createPsicodeChargeMultiplicityFrame(GtkWidget *box);

#endif /* __GABEDIT_PSICODEMOLECULE_H__ */

