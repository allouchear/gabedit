#ifndef __GABEDIT_FIREFLYCONTROL_H__
#define __GABEDIT_FIREFLYCONTROL_H__

gboolean fireflySemiEmperical();
void putFireFlyControlInfoInTextEditor();
void createFireFlyControlFrame(GtkWidget *win, GtkWidget *box);
void setFireFlySCFMethod(gboolean okRHF);
void setFireFlyTD(gboolean ok);

#endif /* __GABEDIT_FIREFLYCONTROL_H__ */
