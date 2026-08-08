// Check that a class property and enum named constant cannot share a name.

module test;

  class C;
    int value;
    typedef enum { value } value_t;
  endclass

endmodule
