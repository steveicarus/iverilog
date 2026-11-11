// Check that part selects are rejected on package-scoped event trigger targets.

package p;
  event e;
endpackage

module test;

  initial begin
    -> p::e[1:0]; // Error: An event trigger target cannot have a part select.
    $display("FAILED");
  end

endmodule
