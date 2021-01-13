
#ifndef __GABEDIT_GAMESSMOLECULE_H__
#define __GABEDIT_GAMESSMOLECULE_H__

gint getGamessMultiplicity();
void initGamessMoleculeButtons();
void initGamessMolecule();
void freeGamessMolecule();
void putGamessGeometryInfoInTextEditor();
gboolean setGamessMolecule();
void createGamessSymmetryFrame(GtkWidget *win, GtkWidget *box);
void createGamessChargeMultiplicityFrame(GtkWidget *box);
void putGamessChargeAndSpinInfoInTextEditor();

#endif /* __GABEDIT_GAMESSMOLECULE_H__ */

