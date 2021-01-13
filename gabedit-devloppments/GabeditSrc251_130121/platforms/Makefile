
include CONFIG

COMMON     = src/Common/*.o
CURVE      = src/Spectrum/*.o
FILES      = src/Files/*.o
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
ORCA      = src/Orca/*.o
QCHEM      = src/QChem/*.o
SEMIEMP    = src/SemiEmpirical/*.o
UTILS      = src/Utils/*.o
SYMMETRY   = src/Symmetry/*.o
VIBCORR   = src/VibrationalCorrections/*.o
IDC   = src/IsotopeDistribution/*.o

 
OBJECTS = \
	$(COMMON) $(CURVE) $(FILES) $(GAMESS) $(GAUSS) $(GEOM) $(MODELS) \
	$(MOLCAS) $(MOLPRO) $(MOPAC) $(MPQC) $(MULTIGRID) $(NETWORK) \
	$(OPENGL) $(FIREFLY) $(ORCA) $(QCHEM) $(SEMIEMP) $(UTILS) \
	$(SYMMETRY) $(VIBCORR) $(IDC)

SUBDIRS = \
	src/Display src/Common src/Files src/Gaussian \
 	src/Geometry src/Spectrum src/MolecularMechanics src/Molpro src/Molcas \
	src/MPQC src/Gamess src/Orca src/QChem src/Mopac src/NetWork src/Utils \
	src/Symmetry src/FireFly src/MultiGrid src/SemiEmpirical \
	src/VibrationalCorrections src/IsotopeDistribution

ifneq ($(external_gl2ps),1)
GL2PS     = gl2ps/*.o
OBJECTS  += $(GL2PS)
SUBDIRS  += gl2ps
else
OBJECTS  += -lgl2ps
endif
 
TARGETS = all clean dep
 
.PHONY: $(SUBDIRS)

$(TARGETS): $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) --directory=$@ $(MAKECMDGOALS)

all: gabedit

gabedit: $(SUBDIRS)
	mkdir tmp
	cp $(OBJECTS) tmp
	$(CC) $(LDFLAGS) tmp/*.o -o gabedit $(X11LIB) $(GTKLIB) $(OGLLIB) $(JPEGLIB) $(LIBPTHREAD) $(WIN32LIB) $(OMPLIB) -g
	@$(RMTMP) > ptmp
	rmdir tmp
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
