// Check that an error is reported when binding an empty value to an argument by
// name and the argument does not have a default value.

module test;

  class B;
    function new(integer a);
      $display("FAILED");
    endfunction
  endclass

  class C extends B;
    function new;
      super.new(.a()); // This should fail. `a` has no default value.
    endfunction
  endclass

  initial begin
    C c;
    c = new;
  end

endmodule
