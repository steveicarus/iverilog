
CXXFLAGS = -g

%.o dep/%.d: %.cc
	$(CXX) -Wall -fno-exceptions $(CXXFLAGS) -MD -c $< -o $*.o
	mv $*.d dep

TF = display.o
O = vvm_bit.o vvm_calltf.o vvm_event.o vvm_monitor.o vvm_pevent.o \
vvm_simulation.o vvm_thread.o $(TF)

libvvm.a: $O
	rm -f $@
	ar cvqf $@ $O

sample: sample.o libvvm.a
	$(CXX) -o sample sample.o libvvm.a

clean:
	rm -f *.o dep/*.d

-include $(patsubst %.o, dep/%.d, $O)
