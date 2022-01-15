// Still need to check real wires and what they return!
module top;
  // Any unsized negative value will be 32 bits or more!
  parameter pm1 = $clog2(-1);
  parameter prm1 = $clog2(-1.0);
  parameter prm30 = $clog2(-(2**31-1));
  parameter prm31 = $clog2(-(2**31));
  parameter prm32 = $clog2(-(33'sd2**32));
  parameter prm67 = $clog2(-(68'sd2**67));
  parameter p0 = $clog2(0);
  parameter p1 = $clog2(1);
  parameter p2 = $clog2(2);
  parameter p3 = $clog2(3);
  parameter p4 = $clog2(4);
  parameter p5 = $clog2(5);
  parameter p8 = $clog2(8);
  parameter p8r = $clog2(8.49999);
  parameter p128 = $clog2(129'h100000000000000000000000000000000);
  parameter p128p = $clog2(129'sd2**128);
  // These all return 'bx.
  parameter pminf = $clog2(-1.0/0.0); // -Inf
  parameter pinf = $clog2(1.0/0.0);  // +Inf
  parameter px = $clog2('bx);

  reg [$clog2(8)-1:0] reg8 = 'b0;

  reg pass = 1'b1;

  integer result;

  reg [128:0] in;
  wire [7:0] out = $clog2(in);

  real rin;
  wire [7:0] rout = $clog2(rin);

  wire real rin2 = rin * 2.0;
  wire [7:0] rout2 = $clog2(rin2);

  initial begin

    /* Test the elab_pexpr implementation. */

    if ($bits(reg8) !== 3) begin
      $display("Failed register size, expected 3, got %d", $bits(reg8));
      pass = 1'b0;
    end

    if (pm1 !== 32) begin
      $display("Failed with param. -1, expected 32, got %d", pm1);
      pass = 1'b0;
    end
    if (prm1 !== 32) begin
      $display("Failed with param. -1.0, expected 32, got %d", prm1);
      pass = 1'b0;
    end
    if (prm30 !== 32) begin
      $display("Failed with param. -(2**30-1), expected 32, got %d", prm30);
      pass = 1'b0;
    end
    if (prm31 !== 32) begin
      $display("Failed with param. -(2**31), expected 32, got %d", prm31);
      pass = 1'b0;
    end
    if (prm32 !== 32) begin
      $display("Failed with param. -(2**32), expected 32, got %d", prm32);
      pass = 1'b0;
    end
    if (prm67 !== 67) begin
      $display("Failed with param. -(2**67), expected 67, got %d", prm67);
      pass = 1'b0;
    end
    if (p0 !== 0) begin
      $display("Failed with param. 0, expected 0, got %d", p0);
      pass = 1'b0;
    end
    if (p1 !== 0) begin
      $display("Failed with param. 1, expected 0, got %d", p1);
      pass = 1'b0;
    end
    if (p2 !== 1) begin
      $display("Failed with param. 2, expected 1, got %d", p2);
      pass = 1'b0;
    end
    if (p3 !== 2) begin
      $display("Failed with param. 3, expected 2, got %d", p3);
      pass = 1'b0;
    end
    if (p4 !== 2) begin
      $display("Failed with param. 4, expected 2, got %d", p4);
      pass = 1'b0;
    end
    if (p5 !== 3) begin
      $display("Failed with param. 5, expected 3, got %d", p5);
      pass = 1'b0;
    end
    if (p8 !== 3) begin
      $display("Failed with param. 8, expected 3, got %d", p8);
      pass = 1'b0;
    end
    if (p8r !== 3) begin
      $display("Failed with param. 8 (real), expected 3, got %d", p8r);
      pass = 1'b0;
    end
    if (p128 !== 128) begin
      $display("Failed with param. 129'h10...0, expected 128, got %d", p128);
      pass = 1'b0;
    end
    if (p128p !== 128) begin
      $display("Failed with param. 2**128, expected 128, got %d", p128p);
      pass = 1'b0;
    end
    if (pinf !== 32'bx) begin
      $display("Failed with param. Inf, expected 32'bx, got %b", pinf);
      pass = 1'b0;
    end
    if (pminf !== 32'bx) begin
      $display("Failed with param. -Inf, expected 32'bx, got %b", pminf);
      pass = 1'b0;
    end
    if (px !== 32'bx) begin
      $display("Failed with param. `bx, expected 32'bx, got %b", px);
      pass = 1'b0;
    end

    /* Test the eval_tree implementation. */

    // Any unsized negative value will be 32 bits or more!
    result = $clog2(-1);
    if (result !== 32) begin
      $display("Failed with -1, expected 32, got %d", result);
      pass = 1'b0;
    end

    result = $clog2(-1.0);
    if (result !== 32) begin
      $display("Failed with -1.0, expected 32, got %d", result);
      pass = 1'b0;
    end

    result = $clog2(-(2**31));
    if (result !== 32) begin
      $display("Failed with -(2**31), expected 32, got %d", result);
      pass = 1'b0;
    end

    result = $clog2(-(33'sd2**32));
    if (result !== 32) begin
      $display("Failed with -(2**32), expected 32, got %d", result);
      pass = 1'b0;
    end

    result = $clog2(-(68'sd2**67));
    if (result !== 67) begin
      $display("Failed with -(2**67), expected 67, got %d", result);
      pass = 1'b0;
    end

    result = $clog2(0);
    if (result !== 0) begin
      $display("Failed with 0, expected 0, got %d", result);
      pass = 1'b0;
    end
    result = $clog2(1);
    if (result !== 0) begin
      $display("Failed with 1, expected 0, got %d", result);
      pass = 1'b0;
    end

    result = $clog2(2);
    if (result !== 1) begin
      $display("Failed with 2, expected 1, got %d", result);
      pass = 1'b0;
    end

    result = $clog2(3);
    if (result !== 2) begin
      $display("Failed with 3, expected 2, got %d", result);
      pass = 1'b0;
    end
    result = $clog2(4);
    if (result !== 2) begin
      $display("Failed with 4, expected 2, got %d", result);
      pass = 1'b0;
    end

    result = $clog2(5);
    if (result !== 3) begin
      $display("Failed with 5, expected 3, got %d", result);
      pass = 1'b0;
    end
    result = $clog2(8);
    if (result !== 3) begin
      $display("Failed with 8, expected 3, got %d", result);
      pass = 1'b0;
    end
    result = $clog2(8.1);
    if (result !== 3) begin
      $display("Failed with 8.1, expected 3, got %d", result);
      pass = 1'b0;
    end
    result = $clog2(8.49999);
    if (result !== 3) begin
      $display("Failed with 8.49999, expected 3, got %d", result);
      pass = 1'b0;
    end
    result = $clog2(8.5);
    if (result !== 4) begin
      $display("Failed with 8.5, expected 4, got %d", result);
      pass = 1'b0;
    end

    result = $clog2(129'h100000000000000000000000000000000);
    if (result !== 128) begin
      $display("Failed with 129'h10...0, expected 128, got %d", result);
      pass = 1'b0;
    end

    result = $clog2(129'sd2**128);
    if (result !== 128) begin
      $display("Failed with 2**128, expected 128, got %d", result);
      pass = 1'b0;
    end

    result = $clog2(1.0/0.0); // Inf
    if (result !== 32'bx) begin
      $display("Failed with Inf, expected 32'bx, got %b", result);
      pass = 1'b0;
    end

    result = $clog2(-1.0/0.0); // -Inf
    if (result !== 32'bx) begin
      $display("Failed with -Inf, expected 32'bx, got %b", result);
      pass = 1'b0;
    end

    result = $clog2('bx);
    if (result !== 32'bx) begin
      $display("Failed with `bx, expected 32'bx, got %b", result);
      pass = 1'b0;
    end

    /* Test the CA statements and the vpi implementation. */

    in = -1;  // This is not an unsized value ('in' is 129 bits)!
    #1 if (out != 129) begin
      $display("Failed CA with -1, expected 129, got %d", out);
      pass = 1'b0;
    end

    in = 0;
    #1 if (out !== 0) begin
      $display("Failed CA with 0, expected 0, got %d", out);
      pass = 1'b0;
    end
    in = 1;
    #1 if (out !== 0) begin
      $display("Failed CA with 1, expected 0, got %d", out);
      pass = 1'b0;
    end

    in = 2;
    #1 if (out !== 1) begin
      $display("Failed CA with 2, expected 1, got %d", out);
      pass = 1'b0;
    end

    in = 3;
    #1 if (out !== 2) begin
      $display("Failed CA with 3, expected 2, got %d", out);
      pass = 1'b0;
    end
    in = 4;
    #1 if (out !== 2) begin
      $display("Failed CA with 4, expected 2, got %d", out);
      pass = 1'b0;
    end

    in = 5;
    #1 if (out !== 3) begin
      $display("Failed CA with 5, expected 3, got %d", out);
      pass = 1'b0;
    end
    in = 8;
    #1 if (out !== 3) begin
      $display("Failed CA with 8, expected 3, got %d", out);
      pass = 1'b0;
    end

    rin = -1.0;  // This is an unsized value (reals are unsized)!
    #1 if (rout !== 32 && rout2 !== 32) begin
      $display("Failed CA with -1.0, expected 32/32, got %d/%d", rout, rout2);
      pass = 1'b0;
    end
    rin = 8.1;
    #1 if (rout !== 3 && rout2 !== 4) begin
      $display("Failed CA with 8.1, expected 3/4, got %d/%d", rout, rout2);
      pass = 1'b0;
    end
    rin = 8.49999;
    #1 if (rout !== 3 && rout2 !== 4) begin
      $display("Failed CA with 8.49999, expected 3/4, got %d/%d", rout, rout2);
      pass = 1'b0;
    end
    rin = 8.5;
    #1 if (rout !== 4 && rout2 !== 5) begin
      $display("Failed CA with 8.5, expected 4/5, got %d/%d", rout, rout2);
      pass = 1'b0;
    end

    in = 129'h100000000000000000000000000000000;
    #1 if (out !== 128) begin
      $display("Failed CA with 129'h10...0, expected 128, got %d", out);
      pass = 1'b0;
    end

    in = 2**128;
    #1 if (out !== 128) begin
      $display("Failed CA with 2**128, expected 128, got %d", out);
      pass = 1'b0;
    end

    in = 'bx;
    #1 if (out !== 8'bx) begin
      $display("Failed CA with 'bx, expected 8'bx, got %b", out);
      pass = 1'b0;
    end

    rin = 1.0/0.0;
    #1 if (rout !== 8'bx && rout2 !== 8'bx) begin
      $display("Failed CA with Inf, expected 8'bx/8'bx got %b/%b", rout, rout2);
      pass = 1'b0;
    end
    rin = -1.0/0.0;
    #1 if (rout !== 8'bx && rout2 !== 8'bx) begin
      $display("Failed CA with -Inf, expected 8'bx/8'bx got %b/%b", rout, rout2);
      pass = 1'b0;
    end

    /* Check that the result is sign extended correctly. */
    // Compile time generated.
    in = $clog2(1.0/0.0);
    if (in !== 129'bx) begin
      $display("Failed sign extended Inf (C), expected 129'bx got %b", in);
      pass = 1'b0;
    end

    // Run time generated.
    rin = 1.0;
    in = $clog2(rin/0.0);
    if (in !== 129'bx) begin
      $display("Failed sign extended Inf (RT), expected 129'bx got %b", in);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
