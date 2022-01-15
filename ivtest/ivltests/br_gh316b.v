module dut(input EN1, input I1, output O1,
           input EN2, input I2, output O2);

assign O1 = EN1 ? I1 : 1'bz;
assign O2 = EN2 ? I2 : 1'bz;

specify
  (I1 => O1)  = (2);
  (EN1 *> O1) = (4);
  (I2 => O2)  = (3);
  (EN2 *> O2) = (4);
endspecify

endmodule

module test();

reg     EN1;
reg     EN2;
reg     I1;
reg     I2;
tri     O;

pulldown(O);

dut dut(EN1, I1, O, EN2, I2, O);

reg failed = 0;

initial begin
  $monitor($time,,EN1,,I1,,EN2,,I2,,O);
  EN1 = 0;
  EN2 = 0;
  #4;
  #0 if (O !== 0) failed = 1;
  I1 = 1;
  I2 = 1;
  #1;
  EN1 = 1;
  #3;
  #0 if (O !== 0) failed = 1;
  #1;
  #0 if (O !== 1) failed = 1;
  I1 = 0;
  #1;
  #0 if (O !== 1) failed = 1;
  #1;
  #0 if (O !== 0) failed = 1;
  EN1 = 0;
  EN2 = 1;
  #3;
  #0 if (O !== 0) failed = 1;
  #1;
  #0 if (O !== 1) failed = 1;
  I2 = 0;
  #2;
  #0 if (O !== 1) failed = 1;
  #1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
