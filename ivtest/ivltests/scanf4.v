`timescale 1ns/10ps

module top;
  reg pass;
  reg [80*8:1] str;
  reg [63:0] a, b, c;
  real r;
  integer code;

  initial begin
    pass = 1'b1;

    // Check that a failing match stops the matching process.
    str = "2 3";
    code = $sscanf(str, "%b%d", a, b);
    if (code !== 0) begin
      $display("FAILED(mb) to stop matching found %d", code);
      pass = 1'b0;
    end
    str = "8 3";
    code = $sscanf(str, "%o%d", a, b);
    if (code !== 0) begin
      $display("FAILED(mo) to stop matching found %d", code);
      pass = 1'b0;
    end
    str = "g 3";
    code = $sscanf(str, "%h%d", a, b);
    if (code !== 0) begin
      $display("FAILED(mh) to stop matching found %d", code);
      pass = 1'b0;
    end
    str = "g 3";
    code = $sscanf(str, "%x%d", a, b);
    if (code !== 0) begin
      $display("FAILED(mx) to stop matching found %d", code);
      pass = 1'b0;
    end
    str = "a 3";
    code = $sscanf(str, "%d%d", a, b);
    if (code !== 0) begin
      $display("FAILED(md) to stop matching found %d", code);
      pass = 1'b0;
    end

    // Check parsing all the binary values (%b).
    str = "01_?xzXZ";
    code = $sscanf(str, "%b", a);
    if (code !== 1) begin
      $display("FAILED(b) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'b01xxzxz) begin
      $display("FAILED(b) argument value, expected 01xxzxz, got %b", a);
      pass = 1'b0;
    end
    // Check that a leaading underscore is invalid.
    a = 'bx;
    str = "_01_?xzXZ";
    code = $sscanf(str, "%b", a);
    if (code !== 0) begin
      $display("FAILED(bi) to parse zero argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'bx) begin
      $display("FAILED(bi) argument value, expected x, got %b", a);
      pass = 1'b0;
    end

    // Check parsing all the octal values (%o).
    str = "01234567_xzXZ?";
    code = $sscanf(str, "%o", a);
    if (code !== 1) begin
      $display("FAILED(o) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 64'o01234567xzxzx) begin
      $display("FAILED(o) argument value, expected 01234567xzxzx, got %o", a);
      pass = 1'b0;
    end
    // Check that a leaading underscore is invalid.
    a = 'bx;
    str = "_01234567_xzXZ?";
    code = $sscanf(str, "%o", a);
    if (code !== 0) begin
      $display("FAILED(oi) to parse zero argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'ox) begin
      $display("FAILED(oi) argument value, expected x, got %o", a);
      pass = 1'b0;
    end

    // Check parsing all the decimal values (%d).
    str = "+01234_56789";
    code = $sscanf(str, "%d", a);
    if (code !== 1) begin
      $display("FAILED(d1) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'd0123456789) begin
      $display("FAILED(d1) argument value, expected 0123456789, got %d", a);
      pass = 1'b0;
    end
    str = "01234_56789";
    code = $sscanf(str, "%d", a);
    if (code !== 1) begin
      $display("FAILED(d2) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'd0123456789) begin
      $display("FAILED(d2) argument value, expected 0123456789, got %d", a);
      pass = 1'b0;
    end
    str = "-01234_56789";
    code = $sscanf(str, "%d", a);
    if (code !== 1) begin
      $display("FAILED(d3) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (a !== -'d0123456789) begin
      $display("FAILED(d3) argument value, expected -0123456789, got %d", a);
      pass = 1'b0;
    end
    str = "x";
    code = $sscanf(str, "%d", a);
    if (code !== 1) begin
      $display("FAILED(d4) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'dx) begin
      $display("FAILED(d4) argument value, expected x, got %d", a);
      pass = 1'b0;
    end
    str = "X";
    code = $sscanf(str, "%d", a);
    if (code !== 1) begin
      $display("FAILED(d5) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'dx) begin
      $display("FAILED(d5) argument value, expected x, got %d", a);
      pass = 1'b0;
    end
    str = "z";
    code = $sscanf(str, "%d", a);
    if (code !== 1) begin
      $display("FAILED(d6) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'dz) begin
      $display("FAILED(d6) argument value, expected z, got %d", a);
      pass = 1'b0;
    end
    str = "Z";
    code = $sscanf(str, "%d", a);
    if (code !== 1) begin
      $display("FAILED(d7) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'dz) begin
      $display("FAILED(d7) argument value, expected z, got %d", a);
      pass = 1'b0;
    end
    str = "?";
    code = $sscanf(str, "%d", a);
    if (code !== 1) begin
      $display("FAILED(d8) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'dx) begin
      $display("FAILED(d8) argument value, expected x, got %d", a);
      pass = 1'b0;
    end
    // A plus or minus must have a digit after to match.
    a = 'bx;
    str = "-q";
    code = $sscanf(str, "%d", a);
    if (code !== 0) begin
      $display("FAILED(d9) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (a !== 'dx) begin
      $display("FAILED(d9) argument value, expected x, got %d", a);
      pass = 1'b0;
    end
    a = 'bx;
    str = "+q";
    code = $sscanf(str, "%d", a);
    if (code !== 0) begin
      $display("FAILED(d0) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (a !== 'dx) begin
      $display("FAILED(d0) argument value, expected x, got %d", a);
      pass = 1'b0;
    end
    // Check that a leaading underscore is invalid.
    a = 'dx;
    str = "_01234_56789";
    code = $sscanf(str, "%d", a);
    if (code !== 0) begin
      $display("FAILED(di) to parse zero argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'dx) begin
      $display("FAILED(di) argument value, expected x, got %d", a);
      pass = 1'b0;
    end

    // Check parsing all the hex values (both %h and %x).
    str = "0123456789_xzXZ?";
    code = $sscanf(str, "%h", a);
    if (code !== 1) begin
      $display("FAILED(h1) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 64'h0123456789xzxzx) begin
      $display("FAILED(h1) argument value, expected 0123456789xzxzx, got %h", a);
      pass = 1'b0;
    end
    str = "aA_bB_cC_dD_eE_fF";
    code = $sscanf(str, "%h", a);
    if (code !== 1) begin
      $display("FAILED(h2) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 64'haabbccddeeff) begin
      $display("FAILED(h2) argument value, expected aabbccddeeff, got %h", a);
      pass = 1'b0;
    end
    str = "0123456789_xzXZ?";
    code = $sscanf(str, "%x", a);
    if (code !== 1) begin
      $display("FAILED(h3) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 64'h0123456789xzxzx) begin
      $display("FAILED(h3) argument value, expected 0123456789xzxzx, got %h", a);
      pass = 1'b0;
    end
    str = "aA_bB_cC_dD_eE_fF";
    code = $sscanf(str, "%x", a);
    if (code !== 1) begin
      $display("FAILED(h4) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 64'haabbccddeeff) begin
      $display("FAILED(h4) argument value, expected aabbccddeeff, got %h", a);
      pass = 1'b0;
    end
    // Check that a leaading underscore is invalid.
    a = 'dx;
    str = "_0123456789_xzXZ?";
    code = $sscanf(str, "%h", a);
    if (code !== 0) begin
      $display("FAILED(hi) to parse zero argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'hx) begin
      $display("FAILED(hi) argument value, expected x, got %h", a);
      pass = 1'b0;
    end

    // Check parsing real values %f.
    str = "+0123456789";
    code = $sscanf(str, "%f", r);
    if (code !== 1) begin
      $display("FAILED(f1) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (r != 123456789.0) begin
      $display("FAILED(f1) argument value, expected 123456789.0, got %f", r);
      pass = 1'b0;
    end
    str = "0123456789";
    code = $sscanf(str, "%f", r);
    if (code !== 1) begin
      $display("FAILED(f2) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (r != 123456789.0) begin
      $display("FAILED(f2) argument value, expected 123456789.0, got %f", r);
      pass = 1'b0;
    end
    str = "-0123456789";
    code = $sscanf(str, "%f", r);
    if (code !== 1) begin
      $display("FAILED(f3) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (r != -123456789.0) begin
      $display("FAILED(f3) argument value, expected -123456789.0, got %f", r);
      pass = 1'b0;
    end
    // A plus or minus must have a digit after to match.
    r = 1.0;
    str = "-q";
    code = $sscanf(str, "%f", r);
    if (code !== 0) begin
      $display("FAILED(f4) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0) begin
      $display("FAILED(f4) argument value, expected 1.0, got %f", r);
      pass = 1'b0;
    end
    r = 1.0;
    str = "+q";
    code = $sscanf(str, "%f", r);
    if (code !== 0) begin
      $display("FAILED(f5) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0) begin
      $display("FAILED(f5) argument value, expected 1.0, got %f", r);
      pass = 1'b0;
    end
    r = 1.0;
    str = "q";
    code = $sscanf(str, "%f", r);
    if (code !== 0) begin
      $display("FAILED(f6) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0) begin
      $display("FAILED(f6) argument value, expected 1.0, got %f", r);
      pass = 1'b0;
    end
    // Check starting/trailing decimal point.
    str = "2.";
    code = $sscanf(str, "%f", r);
    if (code !== 1) begin
      $display("FAILED(f7) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 2.0) begin
      $display("FAILED(f7) argument value, expected 2.0, got %f", r);
      pass = 1'b0;
    end
    str = ".2";
    code = $sscanf(str, "%f", r);
    if (code !== 1) begin
      $display("FAILED(f8) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 0.2) begin
      $display("FAILED(f8) argument value, expected 0.2, got %f", r);
      pass = 1'b0;
    end
    // Check with an exponent.
    str = "2.e1";
    code = $sscanf(str, "%f", r);
    if (code !== 1) begin
      $display("FAILED(f9) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 20.0) begin
      $display("FAILED(f9) argument value, expected 20.0, got %f", r);
      pass = 1'b0;
    end
    str = "2.e-1";
    code = $sscanf(str, "%f", r);
    if (code !== 1) begin
      $display("FAILED(f10) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 0.2) begin
      $display("FAILED(f10) argument value, expected 0.2, got %f", r);
      pass = 1'b0;
    end
    // Check failing exponent cases.
    r = 1.0;
    str = "2.ea";
    code = $sscanf(str, "%f", r);
    if (code !== 0) begin
      $display("FAILED(f11) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0) begin
      $display("FAILED(f11) argument value, expected 1.0, got %f", r);
      pass = 1'b0;
    end
    r = 1.0;
    str = "2.e+a";
    code = $sscanf(str, "%f", r);
    if (code !== 0) begin
      $display("FAILED(f12) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0) begin
      $display("FAILED(f12) argument value, expected 1.0, got %f", r);
      pass = 1'b0;
    end
    r = 1.0;
    str = "2.e-a";
    code = $sscanf(str, "%f", r);
    if (code !== 0) begin
      $display("FAILED(f13) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0) begin
      $display("FAILED(f13) argument value, expected 1.0, got %f", r);
      pass = 1'b0;
    end
    str = "1e5000";
    code = $sscanf(str, "%f", r);
    if (code !== 1) begin
      $display("FAILED(f14) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0/0.0) begin
      $display("FAILED(f14) argument value, expected inf, got %f", r);
      pass = 1'b0;
    end
    str = "-1e5000";
    code = $sscanf(str, "%f", r);
    if (code !== 1) begin
      $display("FAILED(f14) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (r != -1.0/0.0) begin
      $display("FAILED(f14) argument value, expected -inf, got %f", r);
      pass = 1'b0;
    end

    // Check parsing real values %e.
    str = "+0123456789";
    code = $sscanf(str, "%e", r);
    if (code !== 1) begin
      $display("FAILED(e1) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (r != 123456789.0) begin
      $display("FAILED(e1) argument value, expected 123456789.0, got %f", r);
      pass = 1'b0;
    end
    str = "0123456789";
    code = $sscanf(str, "%e", r);
    if (code !== 1) begin
      $display("FAILED(e2) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (r != 123456789.0) begin
      $display("FAILED(e2) argument value, expected 123456789.0, got %f", r);
      pass = 1'b0;
    end
    str = "-0123456789";
    code = $sscanf(str, "%e", r);
    if (code !== 1) begin
      $display("FAILED(e3) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (r != -123456789.0) begin
      $display("FAILED(e3) argument value, expected -123456789.0, got %f", r);
      pass = 1'b0;
    end
    // A plus or minus must have a digit after to match.
    r = 1.0;
    str = "-q";
    code = $sscanf(str, "%e", r);
    if (code !== 0) begin
      $display("FAILED(e4) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0) begin
      $display("FAILED(e4) argument value, expected 1.0, got %f", r);
      pass = 1'b0;
    end
    r = 1.0;
    str = "+q";
    code = $sscanf(str, "%e", r);
    if (code !== 0) begin
      $display("FAILED(e5) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0) begin
      $display("FAILED(e5) argument value, expected 1.0, got %f", r);
      pass = 1'b0;
    end
    r = 1.0;
    str = "q";
    code = $sscanf(str, "%e", r);
    if (code !== 0) begin
      $display("FAILED(e6) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0) begin
      $display("FAILED(e6) argument value, expected 1.0, got %f", r);
      pass = 1'b0;
    end
    // Check starting/trailing decimal point.
    str = "2.";
    code = $sscanf(str, "%e", r);
    if (code !== 1) begin
      $display("FAILED(e7) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 2.0) begin
      $display("FAILED(e7) argument value, expected 2.0, got %f", r);
      pass = 1'b0;
    end
    str = ".2";
    code = $sscanf(str, "%e", r);
    if (code !== 1) begin
      $display("FAILED(e8) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 0.2) begin
      $display("FAILED(e8) argument value, expected 0.2, got %f", r);
      pass = 1'b0;
    end
    // Check with an exponent.
    str = "2.e1";
    code = $sscanf(str, "%e", r);
    if (code !== 1) begin
      $display("FAILED(e9) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 20.0) begin
      $display("FAILED(e9) argument value, expected 20.0, got %f", r);
      pass = 1'b0;
    end
    str = "2.e-1";
    code = $sscanf(str, "%e", r);
    if (code !== 1) begin
      $display("FAILED(e10) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 0.2) begin
      $display("FAILED(e10) argument value, expected 0.2, got %f", r);
      pass = 1'b0;
    end
    // Check failing exponent cases.
    r = 1.0;
    str = "2.ea";
    code = $sscanf(str, "%e", r);
    if (code !== 0) begin
      $display("FAILED(e11) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0) begin
      $display("FAILED(e11) argument value, expected 1.0, got %f", r);
      pass = 1'b0;
    end
    r = 1.0;
    str = "2.e+a";
    code = $sscanf(str, "%e", r);
    if (code !== 0) begin
      $display("FAILED(e12) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0) begin
      $display("FAILED(e12) argument value, expected 1.0, got %f", r);
      pass = 1'b0;
    end
    r = 1.0;
    str = "2.e-a";
    code = $sscanf(str, "%e", r);
    if (code !== 0) begin
      $display("FAILED(e13) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0) begin
      $display("FAILED(e13) argument value, expected 1.0, got %f", r);
      pass = 1'b0;
    end
    str = "1e5000";
    code = $sscanf(str, "%e", r);
    if (code !== 1) begin
      $display("FAILED(e14) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0/0.0) begin
      $display("FAILED(e14) argument value, expected inf, got %f", r);
      pass = 1'b0;
    end
    str = "-1e5000";
    code = $sscanf(str, "%e", r);
    if (code !== 1) begin
      $display("FAILED(e14) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (r != -1.0/0.0) begin
      $display("FAILED(e14) argument value, expected -inf, got %f", r);
      pass = 1'b0;
    end

    // Check parsing real values %g.
    str = "+0123456789";
    code = $sscanf(str, "%g", r);
    if (code !== 1) begin
      $display("FAILED(g1) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (r != 123456789.0) begin
      $display("FAILED(g1) argument value, expected 123456789.0, got %f", r);
      pass = 1'b0;
    end
    str = "0123456789";
    code = $sscanf(str, "%g", r);
    if (code !== 1) begin
      $display("FAILED(g2) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (r != 123456789.0) begin
      $display("FAILED(g2) argument value, expected 123456789.0, got %f", r);
      pass = 1'b0;
    end
    str = "-0123456789";
    code = $sscanf(str, "%g", r);
    if (code !== 1) begin
      $display("FAILED(g3) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (r != -123456789.0) begin
      $display("FAILED(g3) argument value, expected -123456789.0, got %f", r);
      pass = 1'b0;
    end
    // A plus or minus must have a digit after to match.
    r = 1.0;
    str = "-q";
    code = $sscanf(str, "%g", r);
    if (code !== 0) begin
      $display("FAILED(g4) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0) begin
      $display("FAILED(g4) argument value, expected 1.0, got %f", r);
      pass = 1'b0;
    end
    r = 1.0;
    str = "+q";
    code = $sscanf(str, "%g", r);
    if (code !== 0) begin
      $display("FAILED(g5) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0) begin
      $display("FAILED(g5) argument value, expected 1.0, got %f", r);
      pass = 1'b0;
    end
    r = 1.0;
    str = "q";
    code = $sscanf(str, "%g", r);
    if (code !== 0) begin
      $display("FAILED(g6) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0) begin
      $display("FAILED(g6) argument value, expected 1.0, got %f", r);
      pass = 1'b0;
    end
    // Check starting/trailing decimal point.
    str = "2.";
    code = $sscanf(str, "%g", r);
    if (code !== 1) begin
      $display("FAILED(g7) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 2.0) begin
      $display("FAILED(g7) argument value, expected 2.0, got %f", r);
      pass = 1'b0;
    end
    str = ".2";
    code = $sscanf(str, "%g", r);
    if (code !== 1) begin
      $display("FAILED(g8) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 0.2) begin
      $display("FAILED(g8) argument value, expected 0.2, got %f", r);
      pass = 1'b0;
    end
    // Check with an exponent.
    str = "2.e1";
    code = $sscanf(str, "%g", r);
    if (code !== 1) begin
      $display("FAILED(g9) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 20.0) begin
      $display("FAILED(g9) argument value, expected 20.0, got %f", r);
      pass = 1'b0;
    end
    str = "2.e-1";
    code = $sscanf(str, "%g", r);
    if (code !== 1) begin
      $display("FAILED(g10) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 0.2) begin
      $display("FAILED(g10) argument value, expected 0.2, got %f", r);
      pass = 1'b0;
    end
    // Check failing exponent cases.
    r = 1.0;
    str = "2.ea";
    code = $sscanf(str, "%g", r);
    if (code !== 0) begin
      $display("FAILED(g11) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0) begin
      $display("FAILED(g11) argument value, expected 1.0, got %f", r);
      pass = 1'b0;
    end
    r = 1.0;
    str = "2.e+a";
    code = $sscanf(str, "%g", r);
    if (code !== 0) begin
      $display("FAILED(g12) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0) begin
      $display("FAILED(g12) argument value, expected 1.0, got %f", r);
      pass = 1'b0;
    end
    r = 1.0;
    str = "2.e-a";
    code = $sscanf(str, "%g", r);
    if (code !== 0) begin
      $display("FAILED(g13) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0) begin
      $display("FAILED(g13) argument value, expected 1.0, got %f", r);
      pass = 1'b0;
    end
    str = "1e5000";
    code = $sscanf(str, "%g", r);
    if (code !== 1) begin
      $display("FAILED(g14) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (r != 1.0/0.0) begin
      $display("FAILED(g14) argument value, expected inf, got %f", r);
      pass = 1'b0;
    end
    str = "-1e5000";
    code = $sscanf(str, "%g", r);
    if (code !== 1) begin
      $display("FAILED(g14) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (r != -1.0/0.0) begin
      $display("FAILED(g14) argument value, expected -inf, got %f", r);
      pass = 1'b0;
    end

    // Check parsing time values (%t).
    // The %t format uses the real code to parse the number so all the
    // corner cases are tested above.
    str = "+012345678925";
    code = $sscanf(str, "%t", r);
    if (code !== 1) begin
      $display("FAILED(t1) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (r != 123456789.25) begin
      $display("FAILED(t1) argument value, expected 123456789.25, got %f", r);
      pass = 1'b0;
    end
    str = "012345678925";
    code = $sscanf(str, "%t", r);
    if (code !== 1) begin
      $display("FAILED(t2) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (r != 123456789.25) begin
      $display("FAILED(t2) argument value, expected 123456789.25, got %f", r);
      pass = 1'b0;
    end
    str = "-012345678925";
    code = $sscanf(str, "%t", r);
    if (code !== 1) begin
      $display("FAILED(t3) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (r != -123456789.25) begin
      $display("FAILED(t3) argument value, expected -123456789.25, got %f", r);
      pass = 1'b0;
    end
    // Check using different scaling and rounding.
    $timeformat(-9, 3, "ns", 20);
    str = "10.125";
    code = $sscanf(str, "%t", r);
    if (code !== 1) begin
      $display("FAILED(t4) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (r != 10.125) begin
      $display("FAILED(t4) argument value, expected 10.125, got %f", r);
      pass = 1'b0;
    end
    str = "10.0625";
    code = $sscanf(str, "%t", r);
    if (code !== 1) begin
      $display("FAILED(t5) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (r != 10.063) begin
      $display("FAILED(t5) argument value, expected 10.063, got %f", r);
      pass = 1'b0;
    end
    str = "10.03125";
    code = $sscanf(str, "%t", r);
    if (code !== 1) begin
      $display("FAILED(t6) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (r != 10.031) begin
      $display("FAILED(t6) argument value, expected 10.031, got %f", r);
      pass = 1'b0;
    end
    $timeformat(-9, 1, "ns", 20);
    str = "10.543";
    code = $sscanf(str, "%t", r);
    if (code !== 1) begin
      $display("FAILED(t7) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (r != 10.5) begin
      $display("FAILED(t7) argument value, expected 10.5, got %f", r);
      pass = 1'b0;
    end

    // Check parsing a single character (%c).
    str = "t";
    code = $sscanf(str, "%c", a);
    if (code !== 1) begin
      $display("FAILED(c) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 116) begin // t has an ASCII code of 116
      $display("FAILED(c) argument value, expected t, got %c", a);
      pass = 1'b0;
    end

    // Check parsing a string value (%s).
    str = "hi ho";
    code = $sscanf(str, "%s %s", a, b);
    if (code !== 2) begin
      $display("FAILED(s) to parse two arguments found %d", code);
      pass = 1'b0;
    end
    if (a !== "hi") begin
      $display("FAILED(s) first argument value, expected hi, got %s", a);
      pass = 1'b0;
    end
    if (b !== "ho") begin
      $display("FAILED(s) second argument value, expected ho, got %s", b);
      pass = 1'b0;
    end
    // Check an empty %s match.
    a = "skip";
    str = " ";
    code = $sscanf(str, "%s", a);
    if (code !== 0) begin
      $display("FAILED(ep) to parse zero arguments found %d", code);
      pass = 1'b0;
    end
    if (a !== "skip") begin
      $display("FAILED(ep) first argument value, expected skip, got %s", a);
      pass = 1'b0;
    end
    // Check an empty %s second match.
    b = "skip";
    str = "one ";
    code = $sscanf(str, "%s %s", a, b);
    if (code !== 1) begin
      $display("FAILED(es) to parse one arguments found %d", code);
      pass = 1'b0;
    end
    if (a !== "one") begin
      $display("FAILED(es) first argument value, expected one, got %s", a);
      pass = 1'b0;
    end
    if (b !== "skip") begin
      $display("FAILED(es) second argument value, expected skip, got %s", a);
      pass = 1'b0;
    end

    // Check parsing 2 state unformatted binary values (%u).

    // Check parsing 4 state unformatted binary values (%z).

    // Check geting the current module (%m).
    code = $sscanf(" ", "%m", a);
    if (code !== 1) begin
      $display("FAILED(m) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (a !== "top") begin
      $display("FAILED(m) first argument value, expected top, got %s", a);
      pass = 1'b0;
    end

    // Check a string using a width.
    str = "helloworld";
    code = $sscanf(str, "%5s %s", a, b);
    if (code !== 2) begin
      $display("FAILED(sw) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (a !== "hello") begin
      $display("FAILED(sw) first argument value, expected hello, got %s", a);
      pass = 1'b0;
    end
    if (b !== "world") begin
      $display("FAILED(sw) second argument value, expected world, got %s", b);
      pass = 1'b0;
    end
    // Check a binary using a width.
    str = "01101001";
    code = $sscanf(str, "%4b %b", a, b);
    if (code !== 2) begin
      $display("FAILED(bw) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'b0110) begin
      $display("FAILED(bw) first argument value, expected 'b0110, got %b", a);
      pass = 1'b0;
    end
    if (b !== 'b1001) begin
      $display("FAILED(bw) second argument value, expected 'b1001, got %b", b);
      pass = 1'b0;
    end
    // Check an octal using a width.
    str = "234567";
    code = $sscanf(str, "%3o %o", a, b);
    if (code !== 2) begin
      $display("FAILED(ow) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'o234) begin
      $display("FAILED(ow) first argument value, expected 'o234, got %o", a);
      pass = 1'b0;
    end
    if (b !== 'o567) begin
      $display("FAILED(ow) second argument value, expected 'o567, got %o", b);
      pass = 1'b0;
    end
    // Check a hex using a width.
    str = "89abcdef";
    code = $sscanf(str, "%4h %h", a, b);
    if (code !== 2) begin
      $display("FAILED(hw) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'h89ab) begin
      $display("FAILED(hw) first argument value, expected 'h89ab, got %h", a);
      pass = 1'b0;
    end
    if (b !== 'hcdef) begin
      $display("FAILED(hw) second argument value, expected 'hcdef, got %h", b);
      pass = 1'b0;
    end
    // Check a decimal using a width.
    str = "23456789";
    code = $sscanf(str, "%4d %d", a, b);
    if (code !== 2) begin
      $display("FAILED(dw) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'd2345) begin
      $display("FAILED(dw) first argument value, expected 'd2345, got %d", a);
      pass = 1'b0;
    end
    if (b !== 'd6789) begin
      $display("FAILED(dw) second argument value, expected 'd6789, got %d", b);
      pass = 1'b0;
    end
    // Check a real using a width.
    str = "-2.2566789";
    code = $sscanf(str, "%6f %d", r, a);
    if (code !== 2) begin
      $display("FAILED(fw1) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (r != -2.256) begin
      $display("FAILED(fw1) first argument value, expected 2.256, got %f", r);
      pass = 1'b0;
    end
    if (a !== 'd6789) begin
      $display("FAILED(fw1) second argument value, expected 'd6789, got %d", a);
      pass = 1'b0;
    end
    str = "+2.e+16789";
    code = $sscanf(str, "%6f %d", r, a);
    if (code !== 2) begin
      $display("FAILED(fw2) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (r != 20.0) begin
      $display("FAILED(fw2) first argument value, expected 20.0, got %f", r);
      pass = 1'b0;
    end
    if (a !== 'd6789) begin
      $display("FAILED(fw2) second argument value, expected 'd6789, got %d", a);
      pass = 1'b0;
    end
    str = "-2.2566789";
    code = $sscanf(str, "%6e %d", r, a);
    if (code !== 2) begin
      $display("FAILED(ew1) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (r != -2.256) begin
      $display("FAILED(ew1) first argument value, expected 2.256, got %f", r);
      pass = 1'b0;
    end
    if (a !== 'd6789) begin
      $display("FAILED(ew1) second argument value, expected 'd6789, got %d", a);
      pass = 1'b0;
    end
    str = "+2.e+16789";
    code = $sscanf(str, "%6e %d", r, a);
    if (code !== 2) begin
      $display("FAILED(ew2) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (r != 20.0) begin
      $display("FAILED(ew2) first argument value, expected 20.0, got %f", r);
      pass = 1'b0;
    end
    if (a !== 'd6789) begin
      $display("FAILED(ew2) second argument value, expected 'd6789, got %d", a);
      pass = 1'b0;
    end
    str = "-2.2566789";
    code = $sscanf(str, "%6g %d", r, a);
    if (code !== 2) begin
      $display("FAILED(gw1) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (r != -2.256) begin
      $display("FAILED(gw1) first argument value, expected 2.256, got %f", r);
      pass = 1'b0;
    end
    if (a !== 'd6789) begin
      $display("FAILED(gw1) second argument value, expected 'd6789, got %d", a);
      pass = 1'b0;
    end
    str = "+2.e+16789";
    code = $sscanf(str, "%6g %d", r, a);
    if (code !== 2) begin
      $display("FAILED(gw2) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (r != 20.0) begin
      $display("FAILED(gw2) first argument value, expected 20.0, got %f", r);
      pass = 1'b0;
    end
    if (a !== 'd6789) begin
      $display("FAILED(gw2) second argument value, expected 'd6789, got %d", a);
      pass = 1'b0;
    end
    // Check a time using a width.
    $timeformat(-9, 3, "ns", 20);
    str = "-2.2566789";
    code = $sscanf(str, "%6t %d", r, a);
    if (code !== 2) begin
      $display("FAILED(tw1) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (r != -2.256) begin
      $display("FAILED(tw1) first argument value, expected 2.256, got %f", r);
      pass = 1'b0;
    end
    if (a !== 'd6789) begin
      $display("FAILED(tw1) second argument value, expected 'd6789, got %d", a);
      pass = 1'b0;
    end
    str = "+2.e+16789";
    code = $sscanf(str, "%6t %d", r, a);
    if (code !== 2) begin
      $display("FAILED(tw2) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (r != 20.0) begin
      $display("FAILED(tw2) first argument value, expected 20.0, got %f", r);
      pass = 1'b0;
    end
    if (a !== 'd6789) begin
      $display("FAILED(tw2) second argument value, expected 'd6789, got %d", a);
      pass = 1'b0;
    end

    // Check a suppressed string.
    str = "hello bad world";
    code = $sscanf(str, "%s %*s %s", a, b);
    if (code !== 2) begin
      $display("FAILED(ss) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (a !== "hello") begin
      $display("FAILED(sw) first argument value, expected hello, got %s", a);
      pass = 1'b0;
    end
    if (b !== "world") begin
      $display("FAILED(sw) second argument value, expected world, got %s", b);
      pass = 1'b0;
    end
    // Check a suppressed binary.
    str = "0110 xxz 1001";
    code = $sscanf(str, "%b %*b %b", a, b);
    if (code !== 2) begin
      $display("FAILED(bs) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'b0110) begin
      $display("FAILED(bs) first argument value, expected 'b0110, got %b", a);
      pass = 1'b0;
    end
    if (b !== 'b1001) begin
      $display("FAILED(bs) second argument value, expected 'b1001, got %b", b);
      pass = 1'b0;
    end
    // Check a suppressed octal.
    str = "234 xxz 567";
    code = $sscanf(str, "%o %*o %o", a, b);
    if (code !== 2) begin
      $display("FAILED(os) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'o234) begin
      $display("FAILED(os) first argument value, expected 'o234, got %o", a);
      pass = 1'b0;
    end
    if (b !== 'o567) begin
      $display("FAILED(os) second argument value, expected 'o567, got %o", b);
      pass = 1'b0;
    end
    // Check a suppressed hex.
    str = "89ab xz CDEF";
    code = $sscanf(str, "%h %*h %h", a, b);
    if (code !== 2) begin
      $display("FAILED(hs) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'h89ab) begin
      $display("FAILED(hs) first argument value, expected 'h89ab, got %h", a);
      pass = 1'b0;
    end
    if (b !== 'hcdef) begin
      $display("FAILED(hs) second argument value, expected 'hcdef, got %h", b);
      pass = 1'b0;
    end
    // Check a suppressed decimal.
    str = "2345 x 6789";
    code = $sscanf(str, "%d %*d %d", a, b);
    if (code !== 2) begin
      $display("FAILED(ds) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'd2345) begin
      $display("FAILED(ds) first argument value, expected 'd2345, got %d", a);
      pass = 1'b0;
    end
    if (b !== 'd6789) begin
      $display("FAILED(ds) second argument value, expected 'd6789, got %d", b);
      pass = 1'b0;
    end
    // Check a suppressed real.
    str = "2345 1.0 2.0";
    code = $sscanf(str, "%d %*f %f", a, r);
    if (code !== 2) begin
      $display("FAILED(fs) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'd2345) begin
      $display("FAILED(fs) first argument value, expected 'd2345, got %d", a);
      pass = 1'b0;
    end
    if (r != 2.0) begin
      $display("FAILED(fs) second argument value, expected 2.0, got %f", r);
      pass = 1'b0;
    end
    str = "2345 1.0 2.0";
    code = $sscanf(str, "%d %*e %e", a, r);
    if (code !== 2) begin
      $display("FAILED(es) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'd2345) begin
      $display("FAILED(es) first argument value, expected 'd2345, got %d", a);
      pass = 1'b0;
    end
    if (r != 2.0) begin
      $display("FAILED(es) second argument value, expected 2.0, got %f", r);
      pass = 1'b0;
    end
    str = "2345 1.0 2.0";
    code = $sscanf(str, "%d %*g %g", a, r);
    if (code !== 2) begin
      $display("FAILED(gs) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'd2345) begin
      $display("FAILED(gs) first argument value, expected 'd2345, got %d", a);
      pass = 1'b0;
    end
    if (r != 2.0) begin
      $display("FAILED(gs) second argument value, expected 2.0, got %f", r);
      pass = 1'b0;
    end
    // Check a suppressed time.
    str = "2345 1.0 2.0";
    code = $sscanf(str, "%d %*t %t", a, r);
    if (code !== 2) begin
      $display("FAILED(ts) to parse two argument found %d", code);
      pass = 1'b0;
    end
    if (a !== 'd2345) begin
      $display("FAILED(ts) first argument value, expected 'd2345, got %d", a);
      pass = 1'b0;
    end
    if (r != 2.0) begin
      $display("FAILED(ts) second argument value, expected 2.0, got %f", r);
      pass = 1'b0;
    end

    // Check matching normal characters. Also check %%.
    str = "test% str";
    code = $sscanf(str, "test%% %s", a);
    if (code !== 1) begin
      $display("FAILED(nc) to parse one argument found %d", code);
      pass = 1'b0;
    end
    if (a !== "str") begin
      $display("FAILED(nc) first argument value, expected str, got %s", a);
      pass = 1'b0;
    end

    // Check different spacing issues, tab, leading space, extra space, etc.
    str = " one \t\n	two  ";
    code = $sscanf(str, "%s %s", a, b);
    if (code !== 2) begin
      $display("FAILED(sp) to parse two arguments found %d", code);
      pass = 1'b0;
    end
    if (a !== "one") begin
      $display("FAILED(sp) first argument value, expected one, got %s", a);
      pass = 1'b0;
    end
    if (b !== "two") begin
      $display("FAILED(sp) second argument value, expected two, got %s", b);
      pass = 1'b0;
    end

    // Check for a failing match.
    a = 'bx;
    b = 'bx;
    str = "BAD";
    code = $sscanf(str, "GOOD %s %s", a, b);
    if (code !== 0) begin
      $display("FAILED(fl) to parse bad match arguments found %d", code);
      pass = 1'b0;
    end
    if (a !== 'bx) begin
      $display("FAILED(fl) first argument value, expected 'bx, got %b", a);
      pass = 1'b0;
    end
    if (b !== 'bx) begin
      $display("FAILED(fl) second argument value, expected 'bx, got %b", b);
      pass = 1'b0;
    end
    b = 'bx;
    str = "a ";
    // Check a failing character match at EOF.
    code = $sscanf(str, "%s %c", a, b);
    if (code !== 1) begin
      $display("FAILED(fle) character at end, expected one found %d", code);
      pass = 1'b0;
    end
    if (a !== "a") begin
      $display("FAILED(fle) first argument value, expected a, got %s", a);
      pass = 1'b0;
    end
    if (b !== 'bx) begin
      $display("FAILED(fle) second argument value, expected 'bx, got %b", b);
      pass = 1'b0;
    end

    // Check for no match.
    a = 'bx;
    b = 'bx;
    str = "";
    code = $sscanf(str, "GOOD %s %s", a, b);
    if (code !== -1) begin
      $display("FAILED(no) to parse no match arguments found %d", code);
      pass = 1'b0;
    end
    if (a !== 'bx) begin
      $display("FAILED(no) first argument value, expected 'bx, got %s", a);
      pass = 1'b0;
    end
    if (b !== 'bx) begin
      $display("FAILED(no) second argument value, expected 'bx, got %s", b);
      pass = 1'b0;
    end

    // Check for an undefined conversion string.
    a = 'bx;
    str = "foo";
    code = $sscanf(str, 'bx, a);
    if (code !== -1) begin
      $display("FAILED(udef) to parse undefined string returned %d", code);
      pass = 1'b0;
    end
    if (a !== 'bx) begin
      $display("FAILED(udef) first argument value, expected 'bx, got %s", a);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
