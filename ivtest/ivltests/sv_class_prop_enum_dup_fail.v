// Check that a class property and enum named constant cannot share a name.

module test;

  class C;
    int value;
    typedef enum {
      value // Error: The class property already declares this name.
    } value_t;
  endclass

endmodule
