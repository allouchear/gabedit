/* InterfaceGene.h */
/**********************************************************************************************************
Copyright (c) 2002-2013 Abdul-Rahman Allouche. All rights reserved

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the Gabedit), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions
  of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
************************************************************************************************************/

#ifndef __GABEDIT_MINTERFACEGENE_H__
#define __GABEDIT_MINTERFACEGENE_H__

typedef struct _GeneS
{
GtkWidget *title; 
GtkWidget *memory; 

GtkWidget *gth_zero; 
GtkWidget *gth_oneint; 
GtkWidget *gth_twoint; 
GtkWidget *gth_prefac; 
GtkWidget *gth_locali; 
GtkWidget *gth_eorder; 
GtkWidget *gth_energy; 
GtkWidget *gth_gradiant; 
GtkWidget *gth_step; 
GtkWidget *gth_orbital; 
GtkWidget *gth_civec; 
GtkWidget *gth_coeff; 
GtkWidget *gth_printci; 
GtkWidget *gth_punchci; 

GtkWidget *basis; 
GtkWidget *distance; 
GtkWidget *angles; 
GtkWidget *orbital; 
GtkWidget *civector; 
GtkWidget *pairs; 
GtkWidget *cs; 
GtkWidget *cp; 
GtkWidget *ref; 
GtkWidget *pspace; 
GtkWidget *micro; 
GtkWidget *cpu; 
GtkWidget *io; 
GtkWidget *variable; 

GtkWidget *RadCenter;
GtkWidget *entryC;
GtkWidget *entryX;
GtkWidget *entryY;
GtkWidget *entryZ;

GtkWidget *Overlap;
GtkWidget *Kinetic;
GtkWidget *Potential;
GtkWidget *Delta;
GtkWidget *Delta4;
GtkWidget *Darwin;
GtkWidget *MassVelocity;
GtkWidget *CowanGriffin;
GtkWidget *ProductsMO;
GtkWidget *Dipole;
GtkWidget *Second;
GtkWidget *Quadrupole;
GtkWidget *Electric;
GtkWidget *Field;
GtkWidget *AngularMO;
GtkWidget *Velocity;
GtkWidget *SpinOrbit;
GtkWidget *ECP;
}GeneS;
void AjoutePageGene(GtkWidget *,GeneS *);

#endif /* __GABEDIT_MINTERFACEGENE_H__ */

