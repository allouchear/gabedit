
#ifndef __GABEDIT_DOSSPECTRUM_H__
#define __GABEDIT_DOSSPECTRUM_H__

void createDOSSpectrum(GtkWidget *parentWindow, GabEditTypeFile typeOfFile);
void createDOSSpectrumFromEnergiesTable(GtkWidget *parentWindow, gdouble* auEnergies, gint nEnergies);

#endif /* __GABEDIT_DOSSPECTRUM_H__ */
