// Check that when a enum type is declared inside a class that the enum is
// properly installed in the scope and the enum items are available.
//
// Also check that when using a typedef of a enum inside a class that the enum
// is not elaborated inside the class and it is possible to have a enum with the
// same names inside the class scope.

module test;

typedef enum integer {
  A = 1
} e1_t;

class C;
  typedef enum integer {
    A = 10
  } e2_t;
  e1_t e1;
  e2_t e2;

  function new();
    e1 = test.A;
    e2 = A;
  endfunction

  function void set(e2_t new_e2);
    e2 = new_e2;
  endfunction

endclass

C c;

initial begin
  c = new;
  c.e1 = A;
  c.set(c.e2);

  // Not yet supported
  // c.e2 = C::A;
  c.e2 = c.A;

  // Check that they have the numerical value from the right scope
  if (c.e1 == 1 && c.e2 == 10) begin
    $display("PASSED");
  end else begin
    $display("FAILED");
  end
end


endmodule
