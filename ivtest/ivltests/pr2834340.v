//`timescale 1ns/1ps

module test;

reg c1reg,c2reg;
pulldown (weak0) pd1 (r1a,r1c,r1o);
pulldown (weak0) pd2 (r2a,r2c,r2o);

pulldown pd (r1a);
pullup pu (r2a);

wire c1 = c1reg;
wire c2 = c2reg;
SPDT_RELAY r1 (.COIL1(c1), .COIL2(c2), .ARM(r1a), .NC(r1c), .NO(r1o));
SPDT_RELAY r2 (.COIL1(c1), .COIL2(c2), .ARM(r2a), .NC(r2c), .NO(r2o));

initial
begin
  c1reg = 0;
  c2reg = 0;
  repeat (16)
    begin
      c1reg = 1;
      #10;
      c1reg = 0;
      #10;
    end
  $display ("%t: Test passed.",$realtime);
  $display ("PASSED");
  $finish;
end
endmodule


module SPDT_RELAY (COIL1, COIL2, ARM, NC, NO);
inout COIL1, COIL2, ARM, NC, NO;
wire coil = ((COIL1===1'b1) && (COIL2===1'b0)) || ((COIL1===1'b0) && (COIL2===1'b1));

wire #1 dly_coil = coil;
wire coil_on = coil & dly_coil;
wire coil_off = !coil & !dly_coil;

//assign NC = (coil_off) ? ARM : 1'bz;
//assign NO = (coil_on) ? ARM : 1'bz;

tranif1 t1 (ARM,NC,coil_off);
tranif1 t2 (ARM,NO,coil_on);
endmodule
