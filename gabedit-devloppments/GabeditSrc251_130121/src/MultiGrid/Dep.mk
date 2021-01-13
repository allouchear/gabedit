DomainMG.o: DomainMG.c ../../Config.h ../Utils/Vector3d.h \
 ../Utils/Transformation.h ../Utils/Constants.h DomainMG.h TypesMG.h
GridMG.o: GridMG.c ../../Config.h ../Utils/Vector3d.h \
 ../Utils/Transformation.h ../Utils/Constants.h GridMG.h DomainMG.h \
 TypesMG.h
PoissonMG.o: PoissonMG.c ../../Config.h ../Utils/Vector3d.h \
 ../Utils/Transformation.h ../Utils/Constants.h ../Utils/Zlm.h \
 ../Utils/../Common/GabeditType.h ../Utils/MathFunctions.h ../Utils/Zlm.h \
 PoissonMG.h GridMG.h DomainMG.h TypesMG.h ../Common/GabeditType.h \
 ../Display/GlobalOrb.h ../Display/../Files/GabeditFileChooser.h \
 ../Display/../../gl2ps/gl2ps.h ../Display/Grid.h \
 ../Display/../MultiGrid/PoissonMG.h ../Display/IsoSurface.h \
 ../Display/../Common/GabeditType.h ../Display/StatusOrb.h
