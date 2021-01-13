
#ifndef __GABEDIT_FIREFLYMOLECULE_H__
#define __GABEDIT_FIREFLYMOLECULE_H__

gint getFireFlyMultiplicity();
void initFireFlyMoleculeButtons();
void initFireFlyMolecule();
void freeFireFlyMolecule();
void putFireFlyGeometryInfoInTextEditor();
gboolean setFireFlyMolecule();
void createFireFlySymmetryFrame(GtkWidget *win, GtkWidget *box);
void createFireFlyChargeMultiplicityFrame(GtkWidget *box);
void putFireFlyChargeAndSpinInfoInTextEditor();

#endif /* __GABEDIT_FIREFLYMOLECULE_H__ */

