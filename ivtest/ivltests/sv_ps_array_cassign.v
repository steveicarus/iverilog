// Check that continuous array assignments from package scoped identifiers are
// supported.

package P;
  reg [3:0] y[2];
  task init;
    y[0] = 1;
    y[1] = 2;
  endtask
endpackage

module test;

  import P::init;

  wire [3:0] x[2];

  assign x = P::y;

  initial begin
    init();
    if (x[0] === 1 && x[1] === 2) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
