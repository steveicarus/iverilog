// Check that a forward declared base class must be defined.

typedef class B;

class C extends B;
endclass

module test;
endmodule
