// Check that a forward declared base class cannot form an inheritance cycle.

typedef class B;
typedef class C;

class A extends B;
endclass

class B extends C;
endclass

class C extends A;
endclass

module test;
endmodule
