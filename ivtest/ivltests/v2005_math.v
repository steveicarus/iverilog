module top;
  parameter pnani = $sqrt(-1);
  parameter pnanr = $sqrt(-1.0);
  parameter p0i = $sqrt(0);
  parameter p0r = $sqrt(0.0);
  parameter p2i = $sqrt(4);
  parameter p2r = $sqrt(4.0);
  parameter pln = $ln(1);
  parameter plog10 = $log10(10);
  parameter pexp = $exp(0);
  parameter pfloor = $floor(1.5);
  parameter pceil = $ceil(1.5);
  parameter psin = $sin(0);
  parameter pcos = $cos(0);
  parameter ptan = $tan(0);
  parameter pasin = $asin(0);
  parameter pacos = $acos(1);
  parameter patan = $atan(0);
  parameter psinh = $sinh(0);
  parameter pcosh = $cosh(0);
  parameter ptanh = $tanh(0);
  parameter pasinh = $asinh(0);
  parameter pacosh = $acosh(1);
  parameter patanh = $atanh(0);
  parameter ppow = $pow(-2, 2);
  parameter patan2 = $atan2(0, 0);
  parameter phypot = $hypot(-1, 0);

  reg [$sqrt(16)-1:0] reg4 = 'b0;

  reg pass = 1'b1;

  real result, rin;

  wire [7:0] out = $sqrt(rin);
  wire real rout = $sqrt(rin);

  initial begin

    /* Test the elab_pexpr implementation. */

    if ($bits(reg4) !== 4) begin
      $display("Failed register size, expected 4, got %d", $bits(reg4));
      pass = 1'b0;
    end

    if (!(pnani != pnani)) begin
      $display("Failed with param. NaN (int), expected NaN, got %f", pnani);
      pass = 1'b0;
    end
    if (!(pnanr != pnanr)) begin
      $display("Failed with param. NaN (real), expected NaN, got %f", pnanr);
      pass = 1'b0;
    end
    if (p0i != 0.0) begin
      $display("Failed with param. 0, expected 0.0, got %f", p0i);
      pass = 1'b0;
    end
    if (p0r != 0.0) begin
      $display("Failed with param. 0.0, expected 0.0, got %f", p0r);
      pass = 1'b0;
    end
    if (p2i != 2.0) begin
      $display("Failed with param. 4, expected 2.0, got %f", p2i);
      pass = 1'b0;
    end
    if (p2r != 2.0) begin
      $display("Failed with param. 4.0, expected 2.0, got %f", p2r);
      pass = 1'b0;
    end

    if (pln != 0.0) begin
      $display("Failed with param. $ln, expected 0.0, got %f", pln);
      pass = 1'b0;
    end
    if (plog10 != 1.0) begin
      $display("Failed with param. $log10, expected 1.0, got %f", plog10);
      pass = 1'b0;
    end
    if (pexp != 1.0) begin
      $display("Failed with param. $exp, expected 1.0, got %f", pexp);
      pass = 1'b0;
    end

    if (pfloor != 1.0) begin
      $display("Failed with param. $floor, expected 1.0, got %f", pfloor);
      pass = 1'b0;
    end
    if (pceil != 2.0) begin
      $display("Failed with param. $ceil, expected 2.0, got %f", pceil);
      pass = 1'b0;
    end

    if (psin != 0.0) begin
      $display("Failed with param. $sin, expected 0.0, got %f", psin);
      pass = 1'b0;
    end
    if (pcos != 1.0) begin
      $display("Failed with param. $cos, expected 1.0, got %f", pcos);
      pass = 1'b0;
    end
    if (ptan != 0.0) begin
      $display("Failed with param. $tan, expected 0.0, got %f", ptan);
      pass = 1'b0;
    end
    if (pasin != 0.0) begin
      $display("Failed with param. $asin, expected 0.0, got %f", pasin);
      pass = 1'b0;
    end
    if (pacos != 0.0) begin
      $display("Failed with param. $acos, expected 0.0, got %f", pacos);
      pass = 1'b0;
    end
    if (patan != 0.0) begin
      $display("Failed with param. $atan, expected 0.0, got %f", patan);
      pass = 1'b0;
    end

    if (psinh != 0.0) begin
      $display("Failed with param. $sinh, expected 0.0, got %f", psinh);
      pass = 1'b0;
    end
    if (pcosh != 1.0) begin
      $display("Failed with param. $cosh, expected 1.0, got %f", pcosh);
      pass = 1'b0;
    end
    if (ptanh != 0.0) begin
      $display("Failed with param. $tanh, expected 0.0, got %f", ptanh);
      pass = 1'b0;
    end
    if (pasinh != 0.0) begin
      $display("Failed with param. $asinh, expected 0.0, got %f", pasinh);
      pass = 1'b0;
    end
    if (pacosh != 0.0) begin
      $display("Failed with param. $acosh, expected 0.0, got %f", pacosh);
      pass = 1'b0;
    end
    if (patanh != 0.0) begin
      $display("Failed with param. $atanh, expected 0.0, got %f", patanh);
      pass = 1'b0;
    end

    if (ppow != 4.0) begin
      $display("Failed with param. $pow, expected 4.0, got %f", ppow);
      pass = 1'b0;
    end
    if (patan2 != 0.0) begin
      $display("Failed with param. $atan2, expected 4.0, got %f", patan2);
      pass = 1'b0;
    end
    if (phypot != 1.0) begin
      $display("Failed with param. $hypot, expected 1.0, got %f", phypot);
      pass = 1'b0;
    end

    /* Test the eval_tree implementation. */

    result = $sqrt(0);
    if (result != 0.0) begin
      $display("Failed with 0, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    result = $sqrt(0.0);
    if (result != 0.0) begin
      $display("Failed with 0.0, expected 0.0, got %f", result);
      pass = 1'b0;
    end

    result = $sqrt(4);
    if (result != 2.0) begin
      $display("Failed with 4, expected 2.0, got %f", result);
      pass = 1'b0;
    end
    result = $sqrt(4.0);
    if (result != 2.0) begin
      $display("Failed with 4.0, expected 2.0, got %f", result);
      pass = 1'b0;
    end

    result = $ln(1);
    if (result != 0.0) begin
      $display("Failed with $ln, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    result = $log10(10);
    if (result != 1.0) begin
      $display("Failed with $log10, expected 1.0, got %f", result);
      pass = 1'b0;
    end
    result = $exp(0);
    if (result != 1.0) begin
      $display("Failed with $exp, expected 1.0, got %f", result);
      pass = 1'b0;
    end
    result = $floor(1.5);
    if (result != 1.0) begin
      $display("Failed with $floor, expected 1.0, got %f", result);
      pass = 1'b0;
    end
    result = $ceil(1.5);
    if (result != 2.0) begin
      $display("Failed with $ceil, expected 2.0, got %f", result);
      pass = 1'b0;
    end

    result = $sin(0);
    if (result != 0.0) begin
      $display("Failed with $sin, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    result = $cos(0);
    if (result != 1.0) begin
      $display("Failed with $cos, expected 1.0, got %f", result);
      pass = 1'b0;
    end
    result = $tan(0);
    if (result != 0.0) begin
      $display("Failed with $tan, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    result = $asin(0);
    if (result != 0.0) begin
      $display("Failed with $asin, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    result = $acos(1);
    if (result != 0.0) begin
      $display("Failed with $acos, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    result = $atan(0);
    if (result != 0.0) begin
      $display("Failed with $atan, expected 0.0, got %f", result);
      pass = 1'b0;
    end

    result = $sinh(0);
    if (result != 0.0) begin
      $display("Failed with $sinh, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    result = $cosh(0);
    if (result != 1.0) begin
      $display("Failed with $cosh, expected 1.0, got %f", result);
      pass = 1'b0;
    end
    result = $tanh(0);
    if (result != 0.0) begin
      $display("Failed with $tanh, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    result = $asinh(0);
    if (result != 0.0) begin
      $display("Failed with $asinh, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    result = $acosh(1);
    if (result != 0.0) begin
      $display("Failed with $acosh, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    result = $atanh(0);
    if (result != 0.0) begin
      $display("Failed with $atanh, expected 0.0, got %f", result);
      pass = 1'b0;
    end

    result = $pow(-2, 2);
    if (result != 4.0) begin
      $display("Failed with $pow, expected 4.0, got %f", result);
      pass = 1'b0;
    end
    result = $atan2(0, 0);
    if (result != 0.0) begin
      $display("Failed with $atan2, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    result = $hypot(-1, 0);
    if (result != 1.0) begin
      $display("Failed with $hypot, expected 1.0, got %f", result);
      pass = 1'b0;
    end

    /* Test the CA statements and the vpi implementation. */

    rin = -1;
    #1 if (out !== 8'bx) begin
      $display("Failed CA (int) with -1.0, expected 'bx, got %d", out);
      pass = 1'b0;
    end
    if (!(rout != rout)) begin
      $display("Failed CA (real) with -1.0, expected NaN, got %f", rout);
      pass = 1'b0;
    end
    rin = 4.0;
    #1 if (out !== 2) begin
      $display("Failed CA (int) with 4.0, expected 2, got %d", out);
      pass = 1'b0;
    end
    if (rout != 2.0) begin
      $display("Failed CA (real) with 4.0, expected 2.0, got %f", rout);
      pass = 1'b0;
    end

    // Run time generated.
    rin = -1.0;
    result = $sqrt(rin);
    if (!(result != result)) begin
      $display("Failed run time with -1.0, expected NaN got %f", result);
      pass = 1'b0;
    end

    rin = 1.0;
    result = $ln(rin);
    if (result != 0.0) begin
      $display("Failed run time $ln, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    rin = 10.0;
    result = $log10(rin);
    if (result != 1.0) begin
      $display("Failed run time $log10, expected 1.0, got %f", result);
      pass = 1'b0;
    end
    rin = 0.0;
    result = $exp(rin);
    if (result != 1.0) begin
      $display("Failed run time $exp, expected 1.0, got %f", result);
      pass = 1'b0;
    end
    rin = 1.5;
    result = $floor(rin);
    if (result != 1.0) begin
      $display("Failed run time $floor, expected 1.0, got %f", result);
      pass = 1'b0;
    end
    result = $ceil(rin);
    if (result != 2.0) begin
      $display("Failed run time $ceil, expected 2.0, got %f", result);
      pass = 1'b0;
    end

    rin = 0.0;
    result = $sin(rin);
    if (result != 0.0) begin
      $display("Failed run time $sin, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    result = $cos(rin);
    if (result != 1.0) begin
      $display("Failed run time $cos, expected 1.0, got %f", result);
      pass = 1'b0;
    end
    result = $tan(rin);
    if (result != 0.0) begin
      $display("Failed run time $tan, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    result = $asin(rin);
    if (result != 0.0) begin
      $display("Failed run time $asin, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    rin = 1.0;
    result = $acos(rin);
    if (result != 0.0) begin
      $display("Failed run time $acos, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    rin = 0.0;
    result = $atan(rin);
    if (result != 0.0) begin
      $display("Failed run time $atan, expected 0.0, got %f", result);
      pass = 1'b0;
    end

    result = $sinh(rin);
    if (result != 0.0) begin
      $display("Failed run time $sinh, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    result = $cosh(rin);
    if (result != 1.0) begin
      $display("Failed run time $cosh, expected 1.0, got %f", result);
      pass = 1'b0;
    end
    result = $tanh(rin);
    if (result != 0.0) begin
      $display("Failed run time $tanh, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    result = $asinh(rin);
    if (result != 0.0) begin
      $display("Failed run time $asinh, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    rin = 1.0;
    result = $acosh(rin);
    if (result != 0.0) begin
      $display("Failed run time $acosh, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    rin = 0.0;
    result = $atanh(rin);
    if (result != 0.0) begin
      $display("Failed run time $atanh, expected 0.0, got %f", result);
      pass = 1'b0;
    end

    rin = 2.0;
    result = $pow(-2, rin);
    if (result != 4.0) begin
      $display("Failed run time $pow, expected 4.0, got %f", result);
      pass = 1'b0;
    end
    rin = 0.0;
    result = $atan2(0, rin);
    if (result != 0.0) begin
      $display("Failed run time $atan2, expected 0.0, got %f", result);
      pass = 1'b0;
    end
    result = $hypot(-1, rin);
    if (result != 1.0) begin
      $display("Failed run time $hypot, expected 1.0, got %f", result);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
