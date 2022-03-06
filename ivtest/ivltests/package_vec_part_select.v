// Check that it is possible to do a part select on a vector declared in
// package

package P;
  reg [7:0] x = 8'h5a;
  reg [1:0][7:0] y = 16'h5af0;
endpackage

module test;

  initial begin
    if (P::x[3:0] == 4'ha && P::x[7:4] == 4'h5 &&
        P::y[0] == 8'hf0 && P::y[1] == 8'h5a) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
