
CXXFLAGS = -O -g -Wall -Wno-uninitialized

%.o dep/%.d: %.cc
	$(CXX) $(CXXFLAGS) -MD -c $< -o $*.o
	mv $*.d dep/$*.d

#TT = t-debug.o t-vvm.o
TT = t-verilog.o t-vvm.o t-xnf.o
FF = nobufz.o sigfold.o stupid.o xnfio.o

O = main.o cprop.o design_dump.o elaborate.o emit.o eval.o lexor.o mangle.o \
netlist.o parse.o parse_misc.o pform.o pform_dump.o verinum.o target.o \
targets.o Module.o PExpr.o Statement.o $(FF) $(TT)

vl: $O
	$(CXX) $(CXXFLAGS) -o vl $O

clean:
	rm *.o parse.cc parse.cc.output parse.h dep/*.d lexor.cc

lexor.o dep/lexor.d: lexor.cc parse.h

parse.h parse.cc: parse.y
	bison --verbose -t -p VL -d parse.y -o parse.cc
	mv parse.cc.h parse.h

lexor.cc: lexor.lex
	flex -PVL -s -olexor.cc lexor.lex

-include $(patsubst %.o, dep/%.d, $O)
