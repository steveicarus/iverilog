/*
 *  Verilog-A math library test code for Icarus Verilog.
 *  http://www.icarus.com/eda/verilog/
 *
 *  Copyright (C) 2007-2009  Cary R. (cygcary@yahoo.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * As of Dec. 2009 some systems have started returning the sign of
 * a NaN, because of this we can for some conditions get -nan. This
 * will cause mismatches with the gold file. Alan M. Feldstein
 * suggested on the iverilog-devel mailing list that we use fabs
 * (abs()) since C99 speifies that it will remove the sign of the
 * NaN. This appears to work, so I wrapped all functions that we
 * expect to return NaN with a call to abs().
 */

// Get the Verilog-A constants.
`include "constants.vams"

module top;
  real zero, mzero, pinf, minf, nan;

  initial begin
    // Define a few constants.
    zero = 0.0;
    mzero = -1.0 * zero;
    pinf = 1/0.0;
    minf = ln(0);
    nan = abs(sqrt(-1.0));

    $display("Using +0 = %f, -0 = %f, nan = %f, inf = %f and -inf = %f.\n",
             zero, mzero, nan, pinf, minf);

    // Check that the comparisons used to detection a NaN work correctly.
    if (nan != nan) $display("NaN != comparison works correctly.");
    else $display("NaN != comparison failed.");
    if (nan == nan) $display("NaN == comparison failed.\n");
    else $display("NaN == comparison works correctly.\n");

    check_sqrt;
    $display("");
    check_ln;
    $display("");
    check_log;
    $display("");
    check_exp;
    $display("");
    check_abs;
    $display("");
    check_ceil;
    $display("");
    check_floor;
    $display("");
    check_sin;
    $display("");
    check_cos;
    $display("");
    check_tan;
    $display("");
    check_asin;
    $display("");
    check_acos;
    $display("");
    check_atan;
    $display("");
    check_sinh;
    $display("");
    check_cosh;
    $display("");
    check_tanh;
    $display("");
    check_asinh;
    $display("");
    check_acosh;
    $display("");
    check_atanh;
    $display("");
    check_min;
    $display("");
    check_max;
    $display("");
    check_pow;
    $display("");
    check_atan2;
    $display("");
    check_hypot;
    $display("");
    check_constants;
  end

  // Task to check the square root function.
  task check_sqrt;
    begin
      $display("--- Checking the sqrt function ---");
      $display("The square root of  2.0 is %f.", sqrt(2.0));
      $display("The square root of  1.0 is %f.", sqrt(1.0));
      $display("The square root of  0.0 is %f.", sqrt(zero));
      $display("The square root of -0.0 is %f.", sqrt(mzero));
      $display("The square root of -1.0 is %f.", abs(sqrt(-1.0)));
      $display("The square root of  inf is %f.", sqrt(pinf));
      $display("The square root of -inf is %f.", abs(sqrt(minf)));
      $display("The square root of  nan is %f.", abs(sqrt(nan)));
    end
  endtask

  // Task to check the natural log function.
  task check_ln;
    begin
      $display("--- Checking the ln function ---");
      $display("The natural log of 10.0 is %f.", ln(10.0));
      $display("The natural log of  1.0 is %f.", ln(1.0));
      $display("The natural log of  0.5 is %f.", ln(0.5));
      $display("The natural log of  0.0 is %f.", ln(zero));
      $display("The natural log of -0.0 is %f.", ln(mzero));
      $display("The natural log of -1.0 is %f.", abs(ln(-1.0)));
      $display("The natural log of  inf is %f.", ln(pinf));
      $display("The natural log of -inf is %f.", abs(ln(minf)));
      $display("The natural log of  nan is %f.", abs(ln(nan)));
    end
  endtask

  // Task to check the log base 10 function.
  task check_log;
    begin
      $display("--- Checking the log function ---");
      $display("The log base 10 of 10.0 is %f.", log(10.0));
      $display("The log base 10 of  1.0 is %f.", log(1.0));
      $display("The log base 10 of  0.5 is %f.", log(0.5));
      $display("The log base 10 of  0.0 is %f.", log(zero));
      $display("The log base 10 of -0.0 is %f.", log(mzero));
      $display("The log base 10 of -1.0 is %f.", abs(log(-1.0)));
      $display("The log base 10 of  inf is %f.", log(pinf));
      $display("The log base 10 of -inf is %f.", abs(log(minf)));
      $display("The log base 10 of  nan is %f.", abs(log(nan)));
    end
  endtask

  // Task to check the exponential function.
  task check_exp;
    begin
      $display("--- Checking the exp function ---");
      $display("The exponential of  1.0 is %f.", exp(1.0));
      $display("The exponential of  0.0 is %f.", exp(zero));
      $display("The exponential of -0.0 is %f.", exp(mzero));
      $display("The exponential of -1.0 is %f.", exp(-1.0));
      $display("The exponential of  inf is %f.", exp(pinf));
      $display("The exponential of -inf is %f.", exp(minf));
      $display("The exponential of  nan is %f.", abs(exp(nan)));
    end
  endtask

  // Task to check the absolute value function.
  task check_abs;
    begin
      $display("--- Checking the abs function ---");
      $display("The absolute value of  1.0 is %f.", abs(1.0));
      $display("The absolute value of  0.0 is %f.", abs(zero));
      $display("The absolute value of -0.0 is %f.", abs(mzero));
      $display("The absolute value of -1.0 is %f.", abs(-1.0));
      $display("The absolute value of  inf is %f.", abs(pinf));
      $display("The absolute value of -inf is %f.", abs(minf));
      $display("The absolute value of  nan is %f.", abs(nan));
    end
  endtask

  // Task to check the ceiling function.
  task check_ceil;
    begin
      $display("--- Checking the ceil function ---");
      $display("The ceiling of  2.1 is %f.", ceil(2.1));
      $display("The ceiling of  0.5 is %f.", ceil(0.5));
      $display("The ceiling of -0.5 is %f.", ceil(-0.5) + 0.0);
      $display("The ceiling of -1.1 is %f.", ceil(-1.1));
      $display("The ceiling of  inf is %f.", ceil(pinf));
      $display("The ceiling of -inf is %f.", ceil(minf));
      $display("The ceiling of  nan is %f.", abs(ceil(nan)));
    end
  endtask

  // Task to check the floor function.
  task check_floor;
    begin
      $display("--- Checking the floor function ---");
      $display("The floor of  2.1 is %f.", floor(2.1));
      $display("The floor of  0.5 is %f.", floor(0.5));
      $display("The floor of -0.5 is %f.", floor(-0.5));
      $display("The floor of -1.1 is %f.", floor(-1.1));
      $display("The floor of  inf is %f.", floor(pinf));
      $display("The floor of -inf is %f.", floor(minf));
      $display("The floor of  nan is %f.", abs(floor(nan)));
    end
  endtask

  // Task to check the sin function.
  task check_sin;
    begin
      $display("--- Checking the sin function ---");
      $display("The sin of  4.0 is %f.", sin(4.0));
      $display("The sin of  1.0 is %f.", sin(1.0));
      $display("The sin of  0.0 is %f.", sin(zero));
      $display("The sin of -0.0 is %f.", sin(mzero));
      $display("The sin of -1.0 is %f.", sin(-1.0));
      $display("The sin of -4.0 is %f.", sin(-4.0));
      $display("The sin of  inf is %f.", abs(sin(pinf)));
      $display("The sin of -inf is %f.", abs(sin(minf)));
      $display("The sin of  nan is %f.", abs(sin(nan)));
    end
  endtask

  // Task to check the cos function.
  task check_cos;
    begin
      $display("--- Checking the cos function ---");
      $display("The cos of  4.0 is %f.", cos(4.0));
      $display("The cos of  1.0 is %f.", cos(1.0));
      $display("The cos of  0.0 is %f.", cos(zero));
      $display("The cos of -0.0 is %f.", cos(mzero));
      $display("The cos of -1.0 is %f.", cos(-1.0));
      $display("The cos of -4.0 is %f.", cos(-4.0));
      $display("The cos of  inf is %f.", abs(cos(pinf)));
      $display("The cos of -inf is %f.", abs(cos(minf)));
      $display("The cos of  nan is %f.", abs(cos(nan)));
    end
  endtask

  // Task to check the tan function.
  task check_tan;
    begin
      $display("--- Checking the tan function ---");
      $display("The tan of   4.0 is %f.", tan(4.0));
      $display("The tan of   1.0 is %f.", tan(1.0));
      $display("The tan of   0.0 is %f.", tan(zero));
      $display("The tan of  -0.0 is %f.", tan(mzero));
      $display("The tan of  -1.0 is %f.", tan(-1.0));
      $display("The tan of  -4.0 is %f.", tan(-4.0));
      // The underlying math libraries can give different results for
      // this corner case, so we can only use four significant digits
      // for these two tests.
      $display("The tan of  pi/2 is %.4g.", tan(asin(1.0)));
      $display("The tan of -pi/2 is %.4g.", tan(asin(-1.0)));
      $display("The tan of   inf is %f.", abs(tan(pinf)));
      $display("The tan of  -inf is %f.", abs(tan(minf)));
      $display("The tan of   nan is %f.", abs(tan(nan)));
    end
  endtask

  // Task to check the asin function.
  task check_asin;
    begin
      $display("--- Checking the asin function ---");
      $display("The asin of  1.1 is %f.", abs(asin(1.1)));
      $display("The asin of  1.0 is %f.", asin(1.0));
      $display("The asin of  0.5 is %f.", asin(0.5));
      $display("The asin of  0.0 is %f.", asin(zero));
      $display("The asin of -0.0 is %f.", asin(mzero));
      $display("The asin of -0.5 is %f.", asin(-0.5));
      $display("The asin of -1.0 is %f.", asin(-1.0));
      $display("The asin of -1.1 is %f.", abs(asin(-1.1)));
      $display("The asin of  inf is %f.", abs(asin(pinf)));
      $display("The asin of -inf is %f.", abs(asin(minf)));
      $display("The asin of  nan is %f.", abs(asin(nan)));
    end
  endtask

  // Task to check the acos function.
  task check_acos;
    begin
      $display("--- Checking the acos function ---");
      $display("The acos of  1.1 is %f.", abs(acos(1.1)));
      $display("The acos of  1.0 is %f.", acos(1.0));
      $display("The acos of  0.5 is %f.", acos(0.5));
      $display("The acos of  0.0 is %f.", acos(zero));
      $display("The acos of -0.0 is %f.", acos(mzero));
      $display("The acos of -0.5 is %f.", acos(-0.5));
      $display("The acos of -1.0 is %f.", acos(-1.0));
      $display("The acos of -1.1 is %f.", abs(acos(-1.1)));
      $display("The acos of  inf is %f.", abs(acos(pinf)));
      $display("The acos of -inf is %f.", abs(acos(minf)));
      $display("The acos of  nan is %f.", abs(acos(nan)));
    end
  endtask

  // Task to check the atan function.
  task check_atan;
    begin
      $display("--- Checking the atan function ---");
      $display("The atan of  2.0 is %f.", atan(2.0));
      $display("The atan of  0.5 is %f.", atan(0.5));
      $display("The atan of  0.0 is %f.", atan(zero));
      $display("The atan of -0.0 is %f.", atan(mzero));
      $display("The atan of -0.5 is %f.", atan(-0.5));
      $display("The atan of -2.0 is %f.", atan(-2.0));
      $display("The atan of  inf is %f.", atan(pinf));
      $display("The atan of -inf is %f.", atan(minf));
      $display("The atan of  nan is %f.", abs(atan(nan)));
    end
  endtask

  // Task to check the sinh function.
  task check_sinh;
    begin
      $display("--- Checking the sinh function ---");
      $display("The sinh of  2.0 is %f.", sinh(2.0));
      $display("The sinh of  1.0 is %f.", sinh(1.0));
      $display("The sinh of  0.5 is %f.", sinh(0.5));
      $display("The sinh of  0.0 is %f.", sinh(zero));
      $display("The sinh of -0.0 is %f.", sinh(mzero));
      $display("The sinh of -0.5 is %f.", sinh(-0.5));
      $display("The sinh of -1.0 is %f.", sinh(-1.0));
      $display("The sinh of -2.0 is %f.", sinh(-2.0));
      $display("The sinh of  inf is %f.", sinh(pinf));
      $display("The sinh of -inf is %f.", sinh(minf));
      $display("The sinh of  nan is %f.", abs(sinh(nan)));
    end
  endtask

  // Task to check the cosh function.
  task check_cosh;
    begin
      $display("--- Checking the cosh function ---");
      $display("The cosh of  2.0 is %f.", cosh(2.0));
      $display("The cosh of  1.0 is %f.", cosh(1.0));
      $display("The cosh of  0.5 is %f.", cosh(0.5));
      $display("The cosh of  0.0 is %f.", cosh(zero));
      $display("The cosh of -0.0 is %f.", cosh(mzero));
      $display("The cosh of -0.5 is %f.", cosh(-0.5));
      $display("The cosh of -1.0 is %f.", cosh(-1.0));
      $display("The cosh of -2.0 is %f.", cosh(-2.0));
      $display("The cosh of  inf is %f.", cosh(pinf));
      $display("The cosh of -inf is %f.", cosh(minf));
      $display("The cosh of  nan is %f.", abs(cosh(nan)));
    end
  endtask

  // Task to check the tanh function.
  task check_tanh;
    begin
      $display("--- Checking the tanh function ---");
      $display("The tanh of  2.0 is %f.", tanh(2.0));
      $display("The tanh of  1.0 is %f.", tanh(1.0));
      $display("The tanh of  0.5 is %f.", tanh(0.5));
      $display("The tanh of  0.0 is %f.", tanh(zero));
      $display("The tanh of -0.0 is %f.", tanh(mzero));
      $display("The tanh of -0.5 is %f.", tanh(-0.5));
      $display("The tanh of -1.0 is %f.", tanh(-1.0));
      $display("The tanh of -2.0 is %f.", tanh(-2.0));
      $display("The tanh of  inf is %f.", tanh(pinf));
      $display("The tanh of -inf is %f.", tanh(minf));
      $display("The tanh of  nan is %f.", abs(tanh(nan)));
    end
  endtask

  // Task to check the asinh function.
  task check_asinh;
    begin
      $display("--- Checking the asinh function ---");
      $display("The asinh of  2.0 is %f.", asinh(2.0));
      $display("The asinh of  1.0 is %f.", asinh(1.0));
      $display("The asinh of  0.5 is %f.", asinh(0.5));
      $display("The asinh of  0.0 is %f.", asinh(zero));
      $display("The asinh of -0.0 is %f.", asinh(mzero));
      $display("The asinh of -0.5 is %f.", asinh(-0.5));
      $display("The asinh of -1.0 is %f.", asinh(-1.0));
      $display("The asinh of -2.0 is %f.", asinh(-2.0));
      $display("The asinh of  inf is %f.", asinh(pinf));
      $display("The asinh of -inf is %f.", asinh(minf));
      $display("The asinh of  nan is %f.", abs(asinh(nan)));
    end
  endtask

  // Task to check the acosh function.
  task check_acosh;
    begin
      $display("--- Checking the acosh function ---");
      $display("The acosh of  2.0 is %f.", acosh(2.0));
      $display("The acosh of  1.0 is %f.", acosh(1.0));
      $display("The acosh of  0.5 is %f.", abs(acosh(0.5)));
      $display("The acosh of  0   is %f.", abs(acosh(zero)));
      $display("The acosh of -0   is %f.", abs(acosh(mzero)));
      $display("The acosh of -0.5 is %f.", abs(acosh(-0.5)));
      $display("The acosh of -1.0 is %f.", abs(acosh(-1.0)));
      $display("The acosh of -2.0 is %f.", abs(acosh(-2.0)));
      $display("The acosh of  inf is %f.", acosh(pinf));
      $display("The acosh of -inf is %f.", abs(acosh(minf)));
      $display("The acosh of  nan is %f.", abs(acosh(nan)));
    end
  endtask

  // Task to check the atanh function.
  task check_atanh;
    begin
      $display("--- Checking the atanh function ---");
      $display("The atanh of  2.0 is %f.", abs(atanh(2.0)));
      $display("The atanh of  1.0 is %f.", atanh(1.0));
      $display("The atanh of  0.5 is %f.", atanh(0.5));
      $display("The atanh of  0.0 is %f.", atanh(zero));
      $display("The atanh of -0.0 is %f.", atanh(mzero));
      $display("The atanh of -0.5 is %f.", atanh(-0.5));
      $display("The atanh of -1.0 is %f.", atanh(-1.0));
      $display("The atanh of -2.0 is %f.", abs(atanh(-2.0)));
      $display("The atanh of  inf is %f.", abs(atanh(pinf)));
      $display("The atanh of -inf is %f.", abs(atanh(minf)));
      $display("The atanh of  nan is %f.", abs(atanh(nan)));
    end
  endtask

  // Task to check the min function.
  task check_min;
    begin
      $display("--- Checking the min function ---");
      $display("The minimum of  1.0 and  2.0 is %f.", min(1.0, 2.0));
      $display("The minimum of  2.0 and  1.0 is %f.", min(2.0, 1.0));
      $display("The minimum of  1.0 and -1.0 is %f.", min(1.0, -1.0));
      $display("The minimum of -1.0 and -2.0 is %f.", min(-1.0, -2.0));
      $display("The minimum of  2.0 and  inf is %f.", min(2.0, pinf));
      $display("The minimum of  inf and  2.0 is %f.", min(pinf, 2.0));
      $display("The minimum of  2.0 and -inf is %f.", min(2.0, minf));
      $display("The minimum of -inf and  2.0 is %f.", min(minf, 2.0));
      $display("The minimum of  2.0 and  nan is %f.", min(2.0, nan));
      $display("The minimum of  nan and  2.0 is %f.", min(nan, 2.0));
    end
  endtask

  // Task to check the max function.
  task check_max;
    begin
      $display("--- Checking the max function ---");
      $display("The maximum of  1.0 and  2.0 is %f.", max(1.0, 2.0));
      $display("The maximum of  2.0 and  1.0 is %f.", max(2.0, 1.0));
      $display("The maximum of  1.0 and -1.0 is %f.", max(1.0, -1.0));
      $display("The maximum of -1.0 and -2.0 is %f.", max(-1.0, -2.0));
      $display("The maximum of  2.0 and  inf is %f.", max(2.0, pinf));
      $display("The maximum of  inf and  2.0 is %f.", max(pinf, 2.0));
      $display("The maximum of  2.0 and -inf is %f.", max(2.0, minf));
      $display("The maximum of -inf and  2.0 is %f.", max(minf, 2.0));
      $display("The maximum of  2.0 and  nan is %f.", max(2.0, nan));
      $display("The maximum of  nan and  2.0 is %f.", max(nan, 2.0));
    end
  endtask

  // Task to check the power function.
  task check_pow;
    begin
      $display("--- Checking the pow function ---");
      $display(" 0.0 to the power of   0.0 is %f.", pow(zero, zero));
      $display(" 1.0 to the power of   0.0 is %f.", pow(1.0, zero));
      $display("-1.0 to the power of   0.0 is %f.", pow(-1.0, zero));
      $display(" 0.0 to the power of   1.0 is %f.", pow(zero, 1.0));
      $display(" 1.0 to the power of   1.0 is %f.", pow(1.0, 1.0));
      $display("-1.0 to the power of   1.0 is %f.", pow(-1.0, 1.0));
      $display(" 8.0 to the power of   1/3 is %f.", pow(8.0, 1.0/3.0));
      $display(" 8.0 to the power of  -1/3 is %f.", pow(8.0, -1.0/3.0));
      $display(" 2.0 to the power of   3.0 is %f.", pow(2.0, 3.0));
      $display(" 2.0 to the power of  5000 is %f.", pow(2.0, 5000));
      $display("-2.0 to the power of  5001 is %f.", pow(-2.0, 5001));
      $display(" 2.0 to the power of -5000 is %f.", pow(2.0, -5000));
      $display(" inf to the power of   0.0 is %f.", pow(pinf, zero));
      $display("-inf to the power of   0.0 is %f.", pow(minf, zero));
      $display(" inf to the power of   1.0 is %f.", pow(pinf, 1.0));
      $display("-inf to the power of   1.0 is %f.", pow(minf, 1.0));
      $display(" inf to the power of   2.0 is %f.", pow(pinf, 2.0));
      $display("-inf to the power of   2.0 is %f.", pow(minf, 2.0));
      $display(" 1.0 to the power of   inf is %f.", pow(1.0, pinf));
      $display("-1.0 to the power of   inf is %f.", pow(-1.0, pinf));
      $display(" 0.5 to the power of   inf is %f.", pow(0.5, pinf));
      $display(" 2.0 to the power of   inf is %f.", pow(2.0, pinf));
      $display(" 1.0 to the power of  -inf is %f.", pow(1.0, minf));
      $display("-1.0 to the power of  -inf is %f.", pow(-1.0, minf));
      $display(" 0.5 to the power of  -inf is %f.", pow(0.5, minf));
      $display(" 2.0 to the power of  -inf is %f.", pow(2.0, minf));
      $display("-1.0 to the power of  -1/3 is %f.", abs(pow(-1.0, -1.0/3.0)));
      $display(" 1.0 to the power of   nan is %f.", pow(1.0, nan));
      $display(" nan to the power of   1.0 is %f.", abs(pow(nan, 1.0)));
      $display(" nan to the power of   0.0 is %f.", pow(nan, zero));
      $display(" nan to the power of   nan is %f.", abs(pow(nan, nan)));
    end
  endtask

  // Task to check the atan of x/y function.
  task check_atan2;
    begin
      $display("--- Checking the atan2 function ---");
      $display("The atan of  0.0/ 0.0 is %f.", atan2(zero, zero));
      $display("The atan of -0.0/ 0.0 is %f.", atan2(mzero, zero));
      $display("The atan of  0.0/-0.0 is %f.", atan2(zero, mzero));
      $display("The atan of -0.0/-0.0 is %f.", atan2(mzero, mzero));
      $display("The atan of  0.0/ 1.0 is %f.", atan2(zero, 1.0));
      $display("The atan of  1.0/ 0.0 is %f.", atan2(1.0, zero));
      $display("The atan of  1.0/ 1.0 is %f.", atan2(1.0, 1.0));
      $display("The atan of  0.0/-1.0 is %f.", atan2(zero, -1.0));
      $display("The atan of -1.0/ 0.0 is %f.", atan2(-1.0, zero));
      $display("The atan of -1.0/-1.0 is %f.", atan2(-1.0, -1.0));
      $display("The atan of  inf/ 0.0 is %f.", atan2(pinf, zero));
      $display("The atan of  0.0/ inf is %f.", atan2(zero, pinf));
      $display("The atan of  inf/ inf is %f.", atan2(pinf, pinf));
      $display("The atan of -inf/ 0.0 is %f.", atan2(minf, zero));
      $display("The atan of  0.0/-inf is %f.", atan2(zero, minf));
      $display("The atan of -inf/-inf is %f.", atan2(minf, minf));
      $display("The atan of  nan/ 0.0 is %f.", abs(atan2(nan, zero)));
      $display("The atan of  nan/ 1.0 is %f.", abs(atan2(nan, 1.0)));
      $display("The atan of  1.0/ nan is %f.", abs(atan2(1.0, nan)));
    end
  endtask

  // Task to check the distance from origin function.
  task check_hypot;
    begin
      $display("--- Checking the hypot function ---");
      $display("The distance to (  0.0,  0.0) is %f.", hypot(zero, zero));
      $display("The distance to (  2.0,  0.0) is %f.", hypot(2.0, zero));
      $display("The distance to ( -2.0,  0.0) is %f.", hypot(-2.0, zero));
      $display("The distance to (  0.0,  2.0) is %f.", hypot(zero, 2.0));
      $display("The distance to (  0.0, -2.0) is %f.", hypot(zero, -2.0));
      $display("The distance to (  inf,  0.0) is %f.", hypot(pinf, zero));
      $display("The distance to (  0.0,  inf) is %f.", hypot(zero, pinf));
      $display("The distance to ( -inf,  0.0) is %f.", hypot(minf, zero));
      $display("The distance to (  nan,  0.0) is %f.", abs(hypot(nan, zero)));
      $display("The distance to (  0.0,  nan) is %f.", abs(hypot(zero, nan)));
    end
  endtask

  // Task to check the mathematical constants.
  task check_constants;
    begin
      $display("--- Checking the mathematical constants ---");
      $display("        Pi is %.16f.", `M_PI);
      $display("      2*Pi is %.16f.", `M_TWO_PI);
      $display("      Pi/2 is %.16f.", `M_PI_2);
      $display("      Pi/4 is %.16f.", `M_PI_4);
      $display("      1/Pi is %.16f.", `M_1_PI);
      $display("      2/Pi is %.16f.", `M_2_PI);
      $display("2/sqrt(Pi) is %.16f.", `M_2_SQRTPI);
      $display("         e is %.16f.", `M_E);
      $display("   log2(e) is %.16f.", `M_LOG2E);
      $display("  log10(e) is %.16f.", `M_LOG10E);
      $display("   loge(2) is %.16f.", `M_LN2);
      $display("  loge(10) is %.16f.", `M_LN10);
      $display("   sqrt(2) is %.16f.", `M_SQRT2);
      $display(" 1/sqrt(2) is %.16f.", `M_SQRT1_2);
    end
  endtask
endmodule
