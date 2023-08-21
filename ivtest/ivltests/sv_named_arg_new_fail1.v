// Check that an error is reported when trying to bind an argument by nae that
// does not exist

module test;

  class C;
    function new(integer a, integer b);
      $display("FAILED");
    endfunction
  endclass

  initial begin
    C c;
    c = new(.b(2), .c(1)); // This should fail. `c` is not an arugment of the
                           // constructor.
  end

endmodule
