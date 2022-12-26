// Check that another comment directly following a C-style comment is
// supported in a macro.

`define x(a, b) a /* comment */// * b
`define y(a, b) a /* comment *//*/ b*/

 module test;

  initial begin
    if (`x(2, 3) === 2 && `y(8, 2) === 8) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

 endmodule
