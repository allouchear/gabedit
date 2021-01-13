#ifndef __GABEDIT_IRSPECTRUM_H__
#define __GABEDIT_IRSPECTRUM_H__

#include "../Display/Vibration.h"

void createIRSpectrum(GtkWidget *parentWindow, GabEditTypeFile typeOfFile);
void createIRSpectrumFromVibration(GtkWidget *parentWindow, Vibration ibration);
GtkWidget* createIRSpectrumWin(gint numberOfFrequencies, gdouble* frequencies, gdouble* intensities);

#endif /* __GABEDIT_IRSPECTRUM_H__ */
