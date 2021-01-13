OBJECTS = GabeditTextEdit.o AtomsProp.o Jacobi.o QL.o Transformation.o Utils.o UtilsInterface.o Vector3d.o Matrix3D.o HydrogenBond.o PovrayUtils.o UtilsGL.o ConvUtils.o GabeditXYPlot.o GabeditContoursPlot.o UtilsCairo.o Zlm.o MathFunctions.o GTF.o TTables.o Interpolation.o Point3D.o UtilsVASP.o

include ../../CONFIG
CFLAGS =  $(COMMONCFLAGS) $(GTKCFLAGS) $(OGLCFLAGS)

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

