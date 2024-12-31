module dut(input wire [1:0] i, output wire [1:0] o);

assign o = i;

specify
  specparam tRISE = 1;
  specparam tFALL = 2;

  (i => o) = (tRISE, tFALL);
endspecify

endmodule

module test();

reg  [1:0] i;
wire [1:0] o;

dut dut(i, o);

reg failed = 0;

initial begin
  #1 $monitor("%0t %b %b", $time, i, o);

  i = 2'b00;
  #0      if (o !== 2'bxx) failed = 1;
  #1;  #0 if (o !== 2'bxx) failed = 1;
  #1;  #0 if (o !== 2'b00) failed = 1;
  #1;  #0 if (o !== 2'b00) failed = 1;

  i = 2'b11;
  #0      if (o !== 2'b00) failed = 1;
  #1;  #0 if (o !== 2'b11) failed = 1;
  #1;  #0 if (o !== 2'b11) failed = 1;
  #1;  #0 if (o !== 2'b11) failed = 1;

  i = 2'b10;
  #0      if (o !== 2'b11) failed = 1;
  #1;  #0 if (o !== 2'b11) failed = 1;
  #1;  #0 if (o !== 2'b10) failed = 1;
  #1;  #0 if (o !== 2'b10) failed = 1;

  i = 2'b01;
  #0      if (o !== 2'b10) failed = 1;
  #1;  #0 if (o !== 2'b11) failed = 1;
  #1;  #0 if (o !== 2'b01) failed = 1;
  #1;  #0 if (o !== 2'b01) failed = 1;

  #1;
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
