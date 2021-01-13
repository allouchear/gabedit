#ifndef __GABEDIT_GAMESSCONTROL_H__
#define __GABEDIT_GAMESSCONTROL_H__

gboolean gamessSemiEmperical();
void putGamessControlInfoInTextEditor();
void createGamessControlFrame(GtkWidget *win, GtkWidget *box);
void setGamessSCFMethod(gboolean okRHF);

#endif /* __GABEDIT_GAMESSCONTROL_H__ */
