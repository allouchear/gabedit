#ifndef __GABEDIT_QCHEMCONTROL_H__
#define __GABEDIT_QCHEMCONTROL_H__

gboolean qchemSemiEmperical();
void putQChemRemInfoInTextEditor();
void createQChemRemFrame(GtkWidget *win, GtkWidget *box);
void setQChemSCFMethod(gboolean okRHF);

#endif /* __GABEDIT_QCHEMCONTROL_H__ */
