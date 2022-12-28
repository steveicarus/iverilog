// Check that a '/' directly after opening a C-style comment gets handled
// correctly in a macro.

`define x(a, b) a /*/ b */ + b

 module test;

  initial begin
    if (`x(2, 3) === 5) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

 endmodule
