// Check that imported identifiers can't be accessed through hierarchical names.

package P;
  integer x;
endpackage

module test;

  initial begin : outer
    integer y;
    begin: inner
      import P::x;
      y = x;
    end
    y = inner.x; // This should fail. Imported identifiers are not visible
                 // through hierarchical names.
    $display("FAILED");
  end

endmodule
