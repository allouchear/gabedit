MolproLib.o: MolproLib.c ../../Config.h ../Common/GabeditType.h \
 ../Utils/Constants.h ../Utils/Utils.h ../Molpro/MolproLib.h
MInterfaceBasis.o: MInterfaceBasis.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/UtilsInterface.h \
 ../Utils/AtomsProp.h ../Utils/Constants.h ../Gaussian/GaussGlobal.h \
 ../Geometry/GeomGlobal.h ../Geometry/../Common/GabeditType.h \
 ../Utils/Utils.h ../Molpro/MInterfaceGeom.h ../Molpro/MInterfaceBasis.h \
 ../Molpro/MolproLib.h ../Molpro/MolproBasisLibrary.h
MInterfaceComm.o: MInterfaceComm.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/UtilsInterface.h \
 ../Utils/GabeditTextEdit.h ../Geometry/GeomGlobal.h \
 ../Geometry/../Common/GabeditType.h MInterfaceComm.h
MInterfaceGene.o: MInterfaceGene.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h MInterfaceGene.h \
 ../Utils/UtilsInterface.h
MInterfaceGeom.o: MInterfaceGeom.c ../../Config.h MInterfaceGeom.h \
 ../Common/Global.h ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Geometry/GeomGlobal.h \
 ../Geometry/../Common/GabeditType.h ../Geometry/GeomXYZ.h \
 ../Geometry/GeomZmatrix.h ../Gaussian/GaussGlobal.h \
 ../Utils/UtilsInterface.h ../Geometry/InterfaceGeom.h
Molpro.o: Molpro.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/UtilsInterface.h \
 ../Utils/Constants.h MInterfaceGeom.h MInterfaceGene.h MInterfaceBasis.h \
 MInterfaceComm.h ../Geometry/GeomGlobal.h \
 ../Geometry/../Common/GabeditType.h ../Geometry/InterfaceGeom.h \
 ../Utils/Utils.h ../Utils/AtomsProp.h ../Utils/GabeditTextEdit.h \
 ../../pixmaps/Next.xpm
MolproBasisLibrary.o: MolproBasisLibrary.c ../../Config.h \
 ../Utils/Constants.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Molpro/MolproLib.h ../Utils/Utils.h \
 ../Utils/UtilsInterface.h ../Utils/AtomsProp.h \
 ../Molpro/MolproBasisLibrary.h ../Common/StockIcons.h
