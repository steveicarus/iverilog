module top;
  reg [6:0] dx, dz, dz2;

  initial begin
    // Check the unsigned version.
    dx = 7'dx;
    dz = 7'dz;
    dz2 = 7'd?;
    $display(" 7'dx: %b", dx, ",  7'dz: %b", dz, ",  7'd?: %b", dz2);

    dx = 'dx;
    dz = 'dz;
    dz2 = 'd?;
    $display("  'dx: %b", dx, ",   'dz: %b", dz, ",   'd?: %b", dz2);

    dx = 2'dx;
    dz = 2'dz;
    dz2 = 2'd?;
    $display(" 2'dx: %b", dx, ",  2'dz: %b", dz, ",  2'd?: %b", dz2);

    // Check the signed version.
    dx = 7'sdx;
    dz = 7'sdz;
    dz2 = 7'sd?;
    $display("7'sdx: %b", dx, ", 7'sdz: %b", dz, ", 7'sd?: %b", dz2);

    dx = 'sdx;
    dz = 'sdz;
    dz2 = 'sd?;
    $display(" 'sdx: %b", dx, ",  'sdz: %b", dz, ",  'sd?: %b", dz2);

    dx = 2'sdx;
    dz = 2'sdz;
    dz2 = 2'sd?;
    $display("2'sdx: %b", dx, ", 2'sdz: %b", dz, ", 2'sd?: %b", dz2);

    // Check the trailing underscore.
    dx = 7'dx_;
    dz = 7'dz__;
    dz2 = 7'd?___;
    $display("7'dx_: %b", dx, ", 7'dz_: %b", dz, ", 7'd?_: %b", dz2);
  end
endmodule
