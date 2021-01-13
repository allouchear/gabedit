
include CONFIG

COMMON     = src/Common/*.o
CURVE      = src/Spectrum/*.o
FILES      = src/Files/*.o
DEMON      = src/DeMon/*.o
GAMESS     = src/Gamess/*.o
GAUSS      = src/Gaussian/*.o
GEOM       = src/Geometry/*.o
MODELS     = src/MolecularMechanics/*.o
MOLCAS     = src/Molcas/*.o
MOLPRO     = src/Molpro/*.o
MOPAC      = src/Mopac/*.o
MPQC       = src/MPQC/*.o
MULTIGRID  = src/MultiGrid/*.o
NETWORK    = src/NetWork/*.o
OPENGL     = src/Display/*.o
FIREFLY   = src/FireFly/*.o
NWCHEM    = src/NWChem/*.o
PSICODE    = src/Psicode/*.o
ORCA      = src/Orca/*.o
QCHEM      = src/QChem/*.o
CRYSTALLO  = src/Crystallography/*.o
SEMIEMP    = src/SemiEmpirical/*.o
UTILS      = src/Utils/*.o
SYMMETRY   = src/Symmetry/*.o
VIBCORR   = src/VibrationalCorrections/*.o
QFF   = src/QFF/*.o
IDC   = src/IsotopeDistribution/*.o

 
OBJECTS = \
	$(COMMON) $(CURVE) $(FILES) $(DEMON) $(GAMESS) $(GAUSS) $(GEOM) $(MODELS) \
	$(MOLCAS) $(MOLPRO) $(MOPAC) $(MPQC) $(MULTIGRID) $(NETWORK) \
	$(OPENGL) $(FIREFLY) $(NWCHEM) $(PSICODE) $(ORCA) $(QCHEM) $(SEMIEMP) $(UTILS) \
	$(SYMMETRY) $(VIBCORR) $(QFF) $(IDC) $(CRYSTALLO)

SUBDIRS = \
	src/Display src/Common src/Files src/Gaussian \
 	src/Geometry src/Spectrum src/MolecularMechanics src/Molpro src/Molcas \
	src/MPQC src/DeMon src/Gamess src/NWChem  src/Psicode src/Orca src/QChem src/Mopac src/NetWork src/Utils \
	src/Symmetry src/FireFly src/MultiGrid src/SemiEmpirical \
	src/VibrationalCorrections src/IsotopeDistribution src/QFF src/Crystallography \

ifneq ($(external_gl2ps),1)
GL2PS     = gl2ps/*.o
OBJECTS  += $(GL2PS)
SUBDIRS  += gl2ps
GLTOPS =
else
GLTOPS = -lgl2ps
endif

ifneq ($(external_spglib),1)
SPG     = spglib/*.o
OBJECTS  += $(SPG)
SUBDIRS  += spglib
SPGL =
else
SPGL = -lsymspg
endif
 
TARGETS = all clean dep
 
.PHONY: $(SUBDIRS)

$(TARGETS): $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) --directory=$@ $(MAKECMDGOALS)

all: gabedit

gabedit: $(SUBDIRS)
	-@$(RMTMP) 
	$(MKDIR) tmp
	cp $(OBJECTS) tmp
	$(CC) $(LDFLAGS) tmp/*.o -o gabedit $(GLTOPS) $(X11LIB) $(GTKLIB) $(OGLLIB) $(JPEGLIB) $(LIBPTHREAD) $(WIN32LIB) $(OMPLIB) -g -lm
	@$(RMTMP) > ptmp
	@$(RM) ptmp
 
clean:
	@$(RM) gabedit
	@$(RM) gabedit.exe
	@$(RM) *.com
	@$(RM) p
	@$(RM) pp
	@$(RM) p.c
	@$(RM) *.gab
	@$(RM) *.inp
	@$(RM) *.mop
	@$(RM) *.log
	@$(RM) *.out
	@$(RM) *.pun
	@$(RM) *.dat
	@$(RM) *.arc
	@$(RM) *.aux
	@$(RM) *.cmd
	@$(RM) *.chk
	@$(RM) *.sh
	@$(RM) pm6*.*
