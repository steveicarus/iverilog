// Check that a class cannot extend a non-class type.

typedef integer NotAClass;

class Derived extends NotAClass;
endclass

module test;
endmodule
