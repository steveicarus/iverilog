module test;
  reg [31:0] lat;
  wire sign_bit = 1'b1;

  initial begin
    // This should be reported as a syntax error (replicating a zero width).
    lat = {{2{{0{sign_bit}}}}, 16'h0001};

    $display("FAILED");
  end

endmodule
