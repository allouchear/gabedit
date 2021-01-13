Atom.o: Atom.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/Constants.h \
 ../Utils/AtomsProp.h ../Geometry/Fragments.h ../Geometry/DrawGeom.h \
 ../Geometry/DrawGeomCairo.h ../Geometry/Fragments.h Atom.h
Molecule.o: Molecule.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/AtomsProp.h \
 ../Geometry/Fragments.h ../Geometry/DrawGeom.h \
 ../Geometry/DrawGeomCairo.h ../Geometry/Fragments.h \
 ../Geometry/Measure.h Atom.h Molecule.h
ForceField.o: ForceField.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/AtomsProp.h \
 ../Geometry/Fragments.h ../Geometry/DrawGeom.h \
 ../Geometry/DrawGeomCairo.h ../Geometry/Fragments.h Atom.h Molecule.h \
 ForceField.h
MolecularMechanics.o: MolecularMechanics.c ../../Config.h \
 ../Common/Global.h ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/AtomsProp.h ../Utils/Utils.h \
 ../Utils/Constants.h ../Geometry/Fragments.h ../Geometry/DrawGeom.h \
 ../Geometry/DrawGeomCairo.h ../Geometry/Fragments.h Atom.h Molecule.h \
 ForceField.h MolecularMechanics.h LoadMMParameters.h \
 CreatePersonalMMFile.h CreateMolecularMechanicsFile.h
ConjugateGradient.o: ConjugateGradient.c ../../Config.h \
 ../Common/Global.h ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/AtomsProp.h \
 ../Geometry/Fragments.h ../Geometry/DrawGeom.h \
 ../Geometry/DrawGeomCairo.h ../Geometry/Fragments.h Atom.h Molecule.h \
 ForceField.h ConjugateGradient.h
SteepestDescent.o: SteepestDescent.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/AtomsProp.h \
 ../Geometry/Fragments.h ../Geometry/DrawGeom.h \
 ../Geometry/DrawGeomCairo.h ../Geometry/Fragments.h Atom.h Molecule.h \
 ForceField.h SteepestDescent.h
QuasiNewton.o: QuasiNewton.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/AtomsProp.h \
 ../Utils/Constants.h ../Geometry/Fragments.h ../Geometry/DrawGeom.h \
 ../Geometry/DrawGeomCairo.h ../Geometry/Fragments.h Atom.h Molecule.h \
 ForceField.h QuasiNewton.h
MolecularMechanicsDlg.o: MolecularMechanicsDlg.c ../../Config.h \
 ../Common/Global.h ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/UtilsInterface.h \
 ../Utils/Constants.h ../Geometry/Fragments.h ../Geometry/DrawGeom.h \
 ../Geometry/DrawGeomCairo.h ../Geometry/Fragments.h \
 ../Geometry/Measure.h ../Geometry/ResultsAnalise.h \
 ../Geometry/GeomGlobal.h ../Geometry/../Common/GabeditType.h \
 ../Geometry/GeomXYZ.h ../Utils/Utils.h ../Utils/AtomsProp.h \
 ../Files/FolderChooser.h ../Files/GabeditFolderChooser.h \
 ../MolecularMechanics/Atom.h ../MolecularMechanics/Molecule.h \
 ../MolecularMechanics/ForceField.h \
 ../MolecularMechanics/MolecularMechanics.h \
 ../MolecularMechanics/ConjugateGradient.h \
 ../MolecularMechanics/SteepestDescent.h \
 ../MolecularMechanics/QuasiNewton.h \
 ../MolecularMechanics/MolecularDynamics.h
CreateMolecularMechanicsFile.o: CreateMolecularMechanicsFile.c \
 ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/Utils.h
CreatePersonalMMFile.o: CreatePersonalMMFile.c ../../Config.h \
 ../Common/Global.h ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/AtomsProp.h ../Utils/Utils.h \
 ../Geometry/Fragments.h ../Geometry/DrawGeom.h \
 ../Geometry/DrawGeomCairo.h ../Geometry/Fragments.h Atom.h Molecule.h \
 ForceField.h MolecularMechanics.h
LoadMMParameters.o: LoadMMParameters.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/AtomsProp.h ../Utils/Utils.h \
 ../Utils/Constants.h ../Geometry/Fragments.h ../Geometry/DrawGeom.h \
 ../Geometry/DrawGeomCairo.h ../Geometry/Fragments.h Atom.h Molecule.h \
 ForceField.h MolecularMechanics.h
SetMMParameters.o: SetMMParameters.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/AtomsProp.h ../Utils/Utils.h \
 ../Utils/UtilsInterface.h ../Geometry/Fragments.h ../Geometry/DrawGeom.h \
 ../Geometry/DrawGeomCairo.h ../Geometry/Fragments.h Atom.h Molecule.h \
 ForceField.h MolecularMechanics.h ../Common/StockIcons.h
CreateDefaultPDBTpl.o: CreateDefaultPDBTpl.c ../../Config.h \
 ../Common/Global.h ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/Utils.h
LoadPDBTemplate.o: LoadPDBTemplate.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../MolecularMechanics/PDBTemplate.h \
 ../Utils/Utils.h ../Utils/Constants.h
PDBTemplate.o: PDBTemplate.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/Utils.h \
 ../Utils/UtilsInterface.h ../MolecularMechanics/PDBTemplate.h \
 ../MolecularMechanics/LoadPDBTemplate.h \
 ../MolecularMechanics/CreateDefaultPDBTpl.h \
 ../MolecularMechanics/SavePDBTemplate.h
SetPDBTemplate.o: SetPDBTemplate.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/AtomsProp.h ../Utils/Utils.h \
 ../Geometry/Fragments.h ../Geometry/DrawGeom.h \
 ../Geometry/DrawGeomCairo.h ../Geometry/Fragments.h \
 ../MolecularMechanics/Atom.h ../MolecularMechanics/Molecule.h \
 ../MolecularMechanics/ForceField.h \
 ../MolecularMechanics/MolecularMechanics.h ../Utils/UtilsInterface.h \
 ../MolecularMechanics/PDBTemplate.h \
 ../MolecularMechanics/LoadPDBTemplate.h \
 ../MolecularMechanics/CreateDefaultPDBTpl.h ../Common/StockIcons.h
SavePDBTemplate.o: SavePDBTemplate.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../MolecularMechanics/PDBTemplate.h \
 ../Utils/Utils.h
CalculTypesAmber.o: CalculTypesAmber.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Geometry/Fragments.h \
 ../Geometry/DrawGeom.h ../Geometry/DrawGeomCairo.h \
 ../Geometry/Fragments.h ../Utils/Transformation.h ../Utils/Constants.h \
 ../Utils/Utils.h ../Utils/UtilsInterface.h ../Utils/Vector3d.h \
 ../Utils/HydrogenBond.h ../Utils/AtomsProp.h \
 ../MolecularMechanics/CalculTypesAmber.h
MolecularDynamics.o: MolecularDynamics.c ../../Config.h \
 ../Common/Global.h ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/AtomsProp.h \
 ../Geometry/Fragments.h ../Geometry/DrawGeom.h \
 ../Geometry/DrawGeomCairo.h ../Geometry/Fragments.h ../Utils/Utils.h \
 Atom.h Molecule.h ForceField.h MolecularDynamics.h
