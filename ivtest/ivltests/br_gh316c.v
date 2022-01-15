module dut(input EN, input DIR, inout A, inout B);

assign A = EN && DIR == 0 ? B : 1'bz;
assign B = EN && DIR == 1 ? A : 1'bz;

specify
  (A => B)  = (2);
  (B => A)  = (3);
  (EN => A)  = (4);
  (EN => B)  = (4);
endspecify

endmodule

module test();

wire    EN1;
wire    EN2;
wire    I1;
wire    I2;
tri     O;

pulldown(O);

dut dut1(EN1, 1'b1, I1, O);
dut dut2(EN2, 1'b0, O, I2);

reg failed = 0;

initial begin
  $monitor($time,,EN1,,I1,,EN2,,I2,,O);
  force EN1 = 0;
  force EN2 = 0;
  #4;
  #0 if (O !== 0) failed = 1;
  force I1 = 1;
  force I2 = 1;
  #1;
  force EN1 = 1;
  #3;
  #0 if (O !== 0) failed = 1;
  #1;
  #0 if (O !== 1) failed = 1;
  force I1 = 0;
  #1;
  #0 if (O !== 1) failed = 1;
  #1;
  #0 if (O !== 0) failed = 1;
  force EN1 = 0;
  force EN2 = 1;
  #3;
  #0 if (O !== 0) failed = 1;
  #1;
  #0 if (O !== 1) failed = 1;
  force I2 = 0;
  #2;
  #0 if (O !== 1) failed = 1;
  #1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
