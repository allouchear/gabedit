
#ifndef __GABEDIT_MPQCMOLECULE_H__
#define __GABEDIT_MPQCMOLECULE_H__

void initMoleculeButtons();
void initMPQCMolecule();
void freeMPQCMolecule();
void putMPQCGeometryInfoInTextEditor();
gboolean setMPQCMolecule();
void createMPQCSymmetryFrame(GtkWidget *win, GtkWidget *box);

#endif /* __GABEDIT_MPQCMOLECULE_H__ */

