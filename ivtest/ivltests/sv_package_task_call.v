// Check that package-scoped tasks can be called as statements.

package p;
  // Call arguments are parsed in the caller's scope, not the package scope.
  typedef logic argument;
  integer value;

  task clear;
    value = 0;
  endtask

  task set(input integer new_value);
    value = new_value;
  endtask
endpackage

module test;
  integer argument = 42;

  initial begin
    p::clear;
    p::set(argument);

    if (p::value === 42) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
