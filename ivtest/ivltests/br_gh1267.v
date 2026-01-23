// Test for GitHub issue #1267
// Wire logic connected to uwire port should not trigger multi-driver error
// The uwire semantics apply only to the uwire signal, not to wires connected to it.

module a(input uwire logic a1);
endmodule

module b();
  wire logic b1;

  a b_inst(.a1(b1));
  not not_inst(b1, b1);

  assign b1 = 'b0;

  initial begin
    #1;
    // b1 has multiple drivers, which is allowed for wire types
    // The value will be X due to conflicting drivers
    $display("b1 = %b (expected X due to conflicting drivers)", b1);
    $display("PASSED");
  end
endmodule
