OBJECTS = GeomOrbXYZ.o BondsOrb.o GeomDraw.o TriangleDraw.o UtilsOrb.o Basis.o Grid.o IsoSurface.o ViewOrb.o GLArea.o OrbitalsGamess.o OrbitalsMolpro.o OrbitalsOrca.o OrbitalsQChem.o OrbitalsNWChem.o OrbitalsMopac.o OrbitalsNBO.o Orbitals.o StatusOrb.o AtomicOrbitals.o Images.o GridPlans.o Contours.o ContoursDraw.o PreferencesOrb.o GridCube.o GridAdfOrbitals.o GridAdfDensity.o Textures.o Dipole.o AxisGL.o PrincipalAxisGL.o Vibration.o VibrationDraw.o VibrationLocal.o ColorMap.o GridMolcas.o GridQChem.o AnimationRotation.o AnimationIsoSurface.o AnimationContours.o AnimationPlanesMapped.o AnimationGeomConv.o AnimationMD.o PovrayGL.o ContoursPov.o PlanesMappedDraw.o PlanesMapped.o PlanesMappedPov.o  SurfacesPov.o RingsPov.o MenuToolBarGL.o LabelsGL.o RingsOrb.o  ExportGL.o CaptureOrbitals.o IntegralOrbitals.o GridCP.o AnimationGrids.o NCI.o ReactivityIndices.o wfx.o GlobalOrb.o

include ../../CONFIG

CFLAGS =  $(COMMONCFLAGS)  $(GTKCFLAGS) $(OGLCFLAGS)

all: compiling

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

compiling: $(OBJECTS)

clean:
	@$(RM) *.chk p *o gabedit *.com *.out *.log *.test test* *.cpp  *old *.ps .*swp *~ 

#make dependance
SRCS = $(OBJECTS:%.o=%.c)
dep :
	gcc -MM $(SRCS) > Dep.mk

-include Dep.mk

