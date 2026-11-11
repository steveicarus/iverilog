// Check that event controls reject a select on a package-scoped scalar event.

package p;
  event e;
endpackage

module test;

  initial begin
    @(p::e[0]); // Error: A scalar event cannot have a bit select.
    $display("FAILED");
  end

endmodule
