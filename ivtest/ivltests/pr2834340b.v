`timescale 1ns/1ps

module test;

  reg pass;
  reg c1reg,c2reg;
  wire rla, rlc, rlo;
  wire rha, rhc, rho;
  wire c1 = c1reg;
  wire c2 = c2reg;

  // Pull the pins opposite to the arm.
  pulldown pd1 (rla);
  pullup (weak1) pu1 (rlc,rlo);

  pulldown (weak0) pd2 (rhc,rho);
  pullup pu2 (rha);

  SPDT_RELAY rl (.COIL1(c1), .COIL2(c2), .ARM(rla), .NC(rlc), .NO(rlo));
  SPDT_RELAY rh (.COIL1(c1), .COIL2(c2), .ARM(rha), .NC(rhc), .NO(rho));

  initial begin
    pass = 1'b1;

    // Test both coil terminals low.
    c1reg = 0;
    c2reg = 0;
    #10;
    if (rla !== 1'b0 || rlo !== 1'b1 || rlc !== 1'b0) begin
      $display("Failed R1 coil (%b-%b), arm=%b, NC=%b, NO=%b",
               c1, c2, rla, rlc, rlo);
      pass = 1'b0;
    end
    if (rha !== 1'b1 || rho !== 1'b0 || rhc !== 1'b1) begin
      $display("Failed R2 coil (%b-%b), arm=%b, NC=%b, NO=%b",
               c1, c2, rha, rhc, rho);
      pass = 1'b0;
    end

    // Test c1 low and c2 high.
    c2reg = 1;
    #10;
    if (rla !== 1'b0 || rlo !== 1'b0 || rlc !== 1'b1) begin
      $display("Failed R1 coil (%b-%b), arm=%b, NC=%b, NO=%b",
               c1, c2, rla, rlc, rlo);
      pass = 1'b0;
    end
    if (rha !== 1'b1 || rho !== 1'b1 || rhc !== 1'b0) begin
      $display("Failed R2 coil (%b-%b), arm=%b, NC=%b, NO=%b",
               c1, c2, rha, rhc, rho);
      pass = 1'b0;
    end

    // Test both coil terminal high.
    c1reg = 1;
    #10;
    if (rla !== 1'b0 || rlo !== 1'b1 || rlc !== 1'b0) begin
      $display("Failed R1 coil (%b-%b), arm=%b, NC=%b, NO=%b",
               c1, c2, rla, rlc, rlo);
      pass = 1'b0;
    end
    if (rha !== 1'b1 || rho !== 1'b0 || rhc !== 1'b1) begin
      $display("Failed R2 coil (%b-%b), arm=%b, NC=%b, NO=%b",
               c1, c2, rha, rhc, rho);
      pass = 1'b0;
    end

    // Test c1 high and c2 low.
    c2reg = 0;
    #10;
    if (rla !== 1'b0 || rlo !== 1'b0 || rlc !== 1'b1) begin
      $display("Failed R1 coil (%b-%b), arm=%b, NC=%b, NO=%b",
               c1, c2, rla, rlc, rlo);
      pass = 1'b0;
    end
    if (rha !== 1'b1 || rho !== 1'b1 || rhc !== 1'b0) begin
      $display("Failed R2 coil (%b-%b), arm=%b, NC=%b, NO=%b",
               c1, c2, rha, rhc, rho);
      pass = 1'b0;
    end

    if (pass) $display ("PASSED");
    $finish;
  end
endmodule


module SPDT_RELAY (COIL1, COIL2, ARM, NC, NO);
  inout COIL1, COIL2, ARM, NC, NO;
  wire coil = ((COIL1===1'b1) && (COIL2===1'b0)) ||
               ((COIL1===1'b0) && (COIL2===1'b1));

  wire #1 dly_coil = coil;
  wire coil_on = coil & dly_coil;
  wire coil_off = !coil & !dly_coil;

  tranif1 t1 (ARM,NC,coil_off);
  tranif1 t2 (ARM,NO,coil_on);
endmodule
