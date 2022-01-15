module top;
  reg pass = 1'b1;
  real in;
  wire [7:0] out = in;

  initial begin
//    $monitor(in,, out);
    in = sqrt(-1.0);
    #1;
    if (out !== 8'bxxxxxxxx) begin
      $display("Failed: nan expected 8'bxxxxxxxx, got %b", out);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
