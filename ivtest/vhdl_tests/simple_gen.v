// A simple generate example for VHDL conversion
module main();
  wire [39:0] data;
  integer     j;

  generate
    genvar   i;
    for (i = 0; i < 4; i = i + 1) begin
      inc u(data[(i+1)*8 - 1:i*8], data[(i+2)*8 - 1:(i+1)*8]);
    end
  endgenerate

  assign data[7:0] = 1;

  initial begin
    #1;
    $display(data[7:0]);
    $display(data[15:8]);
    $display(data[23:16]);
    $display(data[31:24]);
    $display(data[39:32]);
  end
endmodule // simple_gen

module inc(in, out);
  input [7:0] in;
  output [7:0] out;

  assign out = in + 1;
endmodule // inc
