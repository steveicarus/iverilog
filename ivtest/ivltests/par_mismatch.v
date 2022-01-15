module top;
  reg in;
  wire [7:0] out;

  lwr dut(out, in);

  initial begin
    $display("FAILED");
  end
endmodule

module lwr(out, in);
  output [7:0] out;
  input in;

  assign out = {8{in}};

  specify
    // It is an error to use a parallel connection here since the input
    // and output (source/destination) do not have the same width.
    (in => out) = 2;
  endspecify
endmodule
