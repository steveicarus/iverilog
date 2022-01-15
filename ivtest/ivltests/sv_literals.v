// This tests literal values, from verilog 2001 and SystemVerilog
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2012 by Iztok Jeras.

module test ();

   // logic vector
   logic unsigned [15:0] luv;  // logic unsigned vector
   logic   signed [15:0] lsv;  // logic   signed vector

   // error counter
   bit err = 0;

   initial begin
      // unsized literals without base
      luv = '0;      if (luv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- luv = 'b%b != '0",     luv); err=1; end
      luv = '1;      if (luv !== 16'b1111_1111_1111_1111) begin $display("FAILED -- luv = 'b%b != '1",     luv); err=1; end
      luv = 'x;      if (luv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- luv = 'b%b != 'x",     luv); err=1; end
      luv = 'z;      if (luv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- luv = 'b%b != 'z",     luv); err=1; end

      // unsized binary literals single character
      luv = 'b0;     if (luv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- luv = 'b%b != 'b0",    luv); err=1; end
      luv = 'b1;     if (luv !== 16'b0000_0000_0000_0001) begin $display("FAILED -- luv = 'b%b != 'b1",    luv); err=1; end
      luv = 'bx;     if (luv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- luv = 'b%b != 'bx",    luv); err=1; end
      luv = 'bz;     if (luv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- luv = 'b%b != 'bz",    luv); err=1; end
      // unsized binary literals two characters
      luv = 'b00;    if (luv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- luv = 'b%b != 'b00",   luv); err=1; end
      luv = 'b11;    if (luv !== 16'b0000_0000_0000_0011) begin $display("FAILED -- luv = 'b%b != 'b11",   luv); err=1; end
      luv = 'bxx;    if (luv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- luv = 'b%b != 'bxx",   luv); err=1; end
      luv = 'bzz;    if (luv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- luv = 'b%b != 'bzz",   luv); err=1; end
      luv = 'b1x;    if (luv !== 16'b0000_0000_0000_001x) begin $display("FAILED -- luv = 'b%b != 'b1x",   luv); err=1; end
      luv = 'b1z;    if (luv !== 16'b0000_0000_0000_001z) begin $display("FAILED -- luv = 'b%b != 'b1z",   luv); err=1; end
      luv = 'bx1;    if (luv !== 16'bxxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- luv = 'b%b != 'bx1",   luv); err=1; end
      luv = 'bz1;    if (luv !== 16'bzzzz_zzzz_zzzz_zzz1) begin $display("FAILED -- luv = 'b%b != 'bz1",   luv); err=1; end

      // unsized binary literals single character
      luv = 'o0;     if (luv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- luv = 'b%b != 'o0",    luv); err=1; end
      luv = 'o5;     if (luv !== 16'b0000_0000_0000_0101) begin $display("FAILED -- luv = 'b%b != 'o5",    luv); err=1; end
      luv = 'ox;     if (luv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- luv = 'b%b != 'ox",    luv); err=1; end
      luv = 'oz;     if (luv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- luv = 'b%b != 'oz",    luv); err=1; end
      // unsized binary literals two characters
      luv = 'o00;    if (luv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- luv = 'b%b != 'o00",   luv); err=1; end
      luv = 'o55;    if (luv !== 16'b0000_0000_0010_1101) begin $display("FAILED -- luv = 'b%b != 'o55",   luv); err=1; end
      luv = 'oxx;    if (luv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- luv = 'b%b != 'oxx",   luv); err=1; end
      luv = 'ozz;    if (luv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- luv = 'b%b != 'ozz",   luv); err=1; end
      luv = 'o5x;    if (luv !== 16'b0000_0000_0010_1xxx) begin $display("FAILED -- luv = 'b%b != 'o5x",   luv); err=1; end
      luv = 'o5z;    if (luv !== 16'b0000_0000_0010_1zzz) begin $display("FAILED -- luv = 'b%b != 'o5z",   luv); err=1; end
      luv = 'ox5;    if (luv !== 16'bxxxx_xxxx_xxxx_x101) begin $display("FAILED -- luv = 'b%b != 'ox5",   luv); err=1; end
      luv = 'oz5;    if (luv !== 16'bzzzz_zzzz_zzzz_z101) begin $display("FAILED -- luv = 'b%b != 'oz5",   luv); err=1; end

      // unsized binary literals single character
      luv = 'h0;     if (luv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- luv = 'b%b != 'h0",    luv); err=1; end
      luv = 'h9;     if (luv !== 16'b0000_0000_0000_1001) begin $display("FAILED -- luv = 'b%b != 'h9",    luv); err=1; end
      luv = 'hx;     if (luv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- luv = 'b%b != 'hx",    luv); err=1; end
      luv = 'hz;     if (luv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- luv = 'b%b != 'hz",    luv); err=1; end
      // unsized binary literals two characters
      luv = 'h00;    if (luv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- luv = 'b%b != 'h00",   luv); err=1; end
      luv = 'h99;    if (luv !== 16'b0000_0000_1001_1001) begin $display("FAILED -- luv = 'b%b != 'h99",   luv); err=1; end
      luv = 'hxx;    if (luv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- luv = 'b%b != 'hxx",   luv); err=1; end
      luv = 'hzz;    if (luv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- luv = 'b%b != 'hzz",   luv); err=1; end
      luv = 'h9x;    if (luv !== 16'b0000_0000_1001_xxxx) begin $display("FAILED -- luv = 'b%b != 'h9x",   luv); err=1; end
      luv = 'h9z;    if (luv !== 16'b0000_0000_1001_zzzz) begin $display("FAILED -- luv = 'b%b != 'h9z",   luv); err=1; end
      luv = 'hx9;    if (luv !== 16'bxxxx_xxxx_xxxx_1001) begin $display("FAILED -- luv = 'b%b != 'hx9",   luv); err=1; end
      luv = 'hz9;    if (luv !== 16'bzzzz_zzzz_zzzz_1001) begin $display("FAILED -- luv = 'b%b != 'hz9",   luv); err=1; end

      // unsized binary literals single character
      luv = 'd0;     if (luv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- luv = 'b%b != 'd0",    luv); err=1; end
      luv = 'd9;     if (luv !== 16'b0000_0000_0000_1001) begin $display("FAILED -- luv = 'b%b != 'd9",    luv); err=1; end
      luv = 'dx;     if (luv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- luv = 'b%b != 'dx",    luv); err=1; end
      luv = 'dz;     if (luv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- luv = 'b%b != 'dz",    luv); err=1; end
      // unsized binary literals two characters
      luv = 'd00;    if (luv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- luv = 'b%b != 'd00",   luv); err=1; end
      luv = 'd99;    if (luv !== 16'b0000_0000_0110_0011) begin $display("FAILED -- luv = 'b%b != 'd99",   luv); err=1; end
//    luv = 'dxx;    if (luv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- luv = 'b%b != 'dxx",   luv); err=1; end
//    luv = 'dzz;    if (luv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- luv = 'b%b != 'dzz",   luv); err=1; end


      // unsized binary literals single character
      luv = 15'b0;   if (luv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- luv = 'b%b != 15'b0",  luv); err=1; end
      luv = 15'b1;   if (luv !== 16'b0000_0000_0000_0001) begin $display("FAILED -- luv = 'b%b != 15'b1",  luv); err=1; end
      luv = 15'bx;   if (luv !== 16'b0xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- luv = 'b%b != 15'bx",  luv); err=1; end
      luv = 15'bz;   if (luv !== 16'b0zzz_zzzz_zzzz_zzzz) begin $display("FAILED -- luv = 'b%b != 15'bz",  luv); err=1; end
      // unsized binary literals two characters
      luv = 15'b00;  if (luv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- luv = 'b%b != 15'b00", luv); err=1; end
      luv = 15'b11;  if (luv !== 16'b0000_0000_0000_0011) begin $display("FAILED -- luv = 'b%b != 15'b11", luv); err=1; end
      luv = 15'bxx;  if (luv !== 16'b0xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- luv = 'b%b != 15'bxx", luv); err=1; end
      luv = 15'bzz;  if (luv !== 16'b0zzz_zzzz_zzzz_zzzz) begin $display("FAILED -- luv = 'b%b != 15'bzz", luv); err=1; end
      luv = 15'b1x;  if (luv !== 16'b0000_0000_0000_001x) begin $display("FAILED -- luv = 'b%b != 15'b1x", luv); err=1; end
      luv = 15'b1z;  if (luv !== 16'b0000_0000_0000_001z) begin $display("FAILED -- luv = 'b%b != 15'b1z", luv); err=1; end
      luv = 15'bx1;  if (luv !== 16'b0xxx_xxxx_xxxx_xxx1) begin $display("FAILED -- luv = 'b%b != 15'bx1", luv); err=1; end
      luv = 15'bz1;  if (luv !== 16'b0zzz_zzzz_zzzz_zzz1) begin $display("FAILED -- luv = 'b%b != 15'bz1", luv); err=1; end

      // unsized binary literals single character
      luv = 15'o0;   if (luv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- luv = 'b%b != 15'o0",  luv); err=1; end
      luv = 15'o5;   if (luv !== 16'b0000_0000_0000_0101) begin $display("FAILED -- luv = 'b%b != 15'o5",  luv); err=1; end
      luv = 15'ox;   if (luv !== 16'b0xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- luv = 'b%b != 15'ox",  luv); err=1; end
      luv = 15'oz;   if (luv !== 16'b0zzz_zzzz_zzzz_zzzz) begin $display("FAILED -- luv = 'b%b != 15'oz",  luv); err=1; end
      // unsized binary literals two characters
      luv = 15'o00;  if (luv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- luv = 'b%b != 15'o00", luv); err=1; end
      luv = 15'o55;  if (luv !== 16'b0000_0000_0010_1101) begin $display("FAILED -- luv = 'b%b != 15'o55", luv); err=1; end
      luv = 15'oxx;  if (luv !== 16'b0xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- luv = 'b%b != 15'oxx", luv); err=1; end
      luv = 15'ozz;  if (luv !== 16'b0zzz_zzzz_zzzz_zzzz) begin $display("FAILED -- luv = 'b%b != 15'ozz", luv); err=1; end
      luv = 15'o5x;  if (luv !== 16'b0000_0000_0010_1xxx) begin $display("FAILED -- luv = 'b%b != 15'o5x", luv); err=1; end
      luv = 15'o5z;  if (luv !== 16'b0000_0000_0010_1zzz) begin $display("FAILED -- luv = 'b%b != 15'o5z", luv); err=1; end
      luv = 15'ox5;  if (luv !== 16'b0xxx_xxxx_xxxx_x101) begin $display("FAILED -- luv = 'b%b != 15'ox5", luv); err=1; end
      luv = 15'oz5;  if (luv !== 16'b0zzz_zzzz_zzzz_z101) begin $display("FAILED -- luv = 'b%b != 15'oz5", luv); err=1; end

      // unsized binary literals single character
      luv = 15'h0;   if (luv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- luv = 'b%b != 15'h0",  luv); err=1; end
      luv = 15'h9;   if (luv !== 16'b0000_0000_0000_1001) begin $display("FAILED -- luv = 'b%b != 15'h9",  luv); err=1; end
      luv = 15'hx;   if (luv !== 16'b0xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- luv = 'b%b != 15'hx",  luv); err=1; end
      luv = 15'hz;   if (luv !== 16'b0zzz_zzzz_zzzz_zzzz) begin $display("FAILED -- luv = 'b%b != 15'hz",  luv); err=1; end
      // unsized binary literals two characters
      luv = 15'h00;  if (luv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- luv = 'b%b != 15'h00", luv); err=1; end
      luv = 15'h99;  if (luv !== 16'b0000_0000_1001_1001) begin $display("FAILED -- luv = 'b%b != 15'h99", luv); err=1; end
      luv = 15'hxx;  if (luv !== 16'b0xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- luv = 'b%b != 15'hxx", luv); err=1; end
      luv = 15'hzz;  if (luv !== 16'b0zzz_zzzz_zzzz_zzzz) begin $display("FAILED -- luv = 'b%b != 15'hzz", luv); err=1; end
      luv = 15'h9x;  if (luv !== 16'b0000_0000_1001_xxxx) begin $display("FAILED -- luv = 'b%b != 15'h9x", luv); err=1; end
      luv = 15'h9z;  if (luv !== 16'b0000_0000_1001_zzzz) begin $display("FAILED -- luv = 'b%b != 15'h9z", luv); err=1; end
      luv = 15'hx9;  if (luv !== 16'b0xxx_xxxx_xxxx_1001) begin $display("FAILED -- luv = 'b%b != 15'hx9", luv); err=1; end
      luv = 15'hz9;  if (luv !== 16'b0zzz_zzzz_zzzz_1001) begin $display("FAILED -- luv = 'b%b != 15'hz9", luv); err=1; end

      // unsized binary literals single character
      luv = 15'd0;   if (luv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- luv = 'b%b != 15'd0",  luv); err=1; end
      luv = 15'd9;   if (luv !== 16'b0000_0000_0000_1001) begin $display("FAILED -- luv = 'b%b != 15'd9",  luv); err=1; end
      luv = 15'dx;   if (luv !== 16'b0xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- luv = 'b%b != 15'dx",  luv); err=1; end
      luv = 15'dz;   if (luv !== 16'b0zzz_zzzz_zzzz_zzzz) begin $display("FAILED -- luv = 'b%b != 15'dz",  luv); err=1; end
      // unsized binary literals two characters
      luv = 15'd00;  if (luv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- luv = 'b%b != 15'd00", luv); err=1; end
      luv = 15'd99;  if (luv !== 16'b0000_0000_0110_0011) begin $display("FAILED -- luv = 'b%b != 15'd99", luv); err=1; end
//    luv = 15'dxx;  if (luv !== 16'b0xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- luv = 'b%b != 15'dxx", luv); err=1; end
//    luv = 15'dzz;  if (luv !== 16'b0zzz_zzzz_zzzz_zzzz) begin $display("FAILED -- luv = 'b%b != 15'dzz", luv); err=1; end




      // unsized binary literals single character
      lsv = 'sb0;     if (lsv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- lsv = 'b%b != 'sb0",    lsv); err=1; end
      lsv = 'sb1;     if (lsv !== 16'b0000_0000_0000_0001) begin $display("FAILED -- lsv = 'b%b != 'sb1",    lsv); err=1; end
      lsv = 'sbx;     if (lsv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- lsv = 'b%b != 'sbx",    lsv); err=1; end
      lsv = 'sbz;     if (lsv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- lsv = 'b%b != 'sbz",    lsv); err=1; end
      // unsized binary literals two characters
      lsv = 'sb00;    if (lsv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- lsv = 'b%b != 'sb00",   lsv); err=1; end
      lsv = 'sb11;    if (lsv !== 16'b0000_0000_0000_0011) begin $display("FAILED -- lsv = 'b%b != 'sb11",   lsv); err=1; end
      lsv = 'sbxx;    if (lsv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- lsv = 'b%b != 'sbxx",   lsv); err=1; end
      lsv = 'sbzz;    if (lsv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- lsv = 'b%b != 'sbzz",   lsv); err=1; end
      lsv = 'sb1x;    if (lsv !== 16'b0000_0000_0000_001x) begin $display("FAILED -- lsv = 'b%b != 'sb1x",   lsv); err=1; end
      lsv = 'sb1z;    if (lsv !== 16'b0000_0000_0000_001z) begin $display("FAILED -- lsv = 'b%b != 'sb1z",   lsv); err=1; end
      lsv = 'sbx1;    if (lsv !== 16'bxxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- lsv = 'b%b != 'sbx1",   lsv); err=1; end
      lsv = 'sbz1;    if (lsv !== 16'bzzzz_zzzz_zzzz_zzz1) begin $display("FAILED -- lsv = 'b%b != 'sbz1",   lsv); err=1; end

      // unsized binary literals single character
      lsv = 'so0;     if (lsv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- lsv = 'b%b != 'so0",    lsv); err=1; end
      lsv = 'so5;     if (lsv !== 16'b0000_0000_0000_0101) begin $display("FAILED -- lsv = 'b%b != 'so5",    lsv); err=1; end
      lsv = 'sox;     if (lsv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- lsv = 'b%b != 'sox",    lsv); err=1; end
      lsv = 'soz;     if (lsv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- lsv = 'b%b != 'soz",    lsv); err=1; end
      // unsized binary literals two characters
      lsv = 'so00;    if (lsv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- lsv = 'b%b != 'so00",   lsv); err=1; end
      lsv = 'so55;    if (lsv !== 16'b0000_0000_0010_1101) begin $display("FAILED -- lsv = 'b%b != 'so55",   lsv); err=1; end
      lsv = 'soxx;    if (lsv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- lsv = 'b%b != 'soxx",   lsv); err=1; end
      lsv = 'sozz;    if (lsv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- lsv = 'b%b != 'sozz",   lsv); err=1; end
      lsv = 'so5x;    if (lsv !== 16'b0000_0000_0010_1xxx) begin $display("FAILED -- lsv = 'b%b != 'so5x",   lsv); err=1; end
      lsv = 'so5z;    if (lsv !== 16'b0000_0000_0010_1zzz) begin $display("FAILED -- lsv = 'b%b != 'so5z",   lsv); err=1; end
      lsv = 'sox5;    if (lsv !== 16'bxxxx_xxxx_xxxx_x101) begin $display("FAILED -- lsv = 'b%b != 'sox5",   lsv); err=1; end
      lsv = 'soz5;    if (lsv !== 16'bzzzz_zzzz_zzzz_z101) begin $display("FAILED -- lsv = 'b%b != 'soz5",   lsv); err=1; end

      // unsized binary literals single character
      lsv = 'sh0;     if (lsv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- lsv = 'b%b != 'sh0",    lsv); err=1; end
      lsv = 'sh9;     if (lsv !== 16'b0000_0000_0000_1001) begin $display("FAILED -- lsv = 'b%b != 'sh9",    lsv); err=1; end
      lsv = 'shx;     if (lsv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- lsv = 'b%b != 'shx",    lsv); err=1; end
      lsv = 'shz;     if (lsv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- lsv = 'b%b != 'shz",    lsv); err=1; end
      // unsized binary literals two characters
      lsv = 'sh00;    if (lsv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- lsv = 'b%b != 'sh00",   lsv); err=1; end
      lsv = 'sh99;    if (lsv !== 16'b0000_0000_1001_1001) begin $display("FAILED -- lsv = 'b%b != 'sh99",   lsv); err=1; end
      lsv = 'shxx;    if (lsv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- lsv = 'b%b != 'shxx",   lsv); err=1; end
      lsv = 'shzz;    if (lsv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- lsv = 'b%b != 'shzz",   lsv); err=1; end
      lsv = 'sh9x;    if (lsv !== 16'b0000_0000_1001_xxxx) begin $display("FAILED -- lsv = 'b%b != 'sh9x",   lsv); err=1; end
      lsv = 'sh9z;    if (lsv !== 16'b0000_0000_1001_zzzz) begin $display("FAILED -- lsv = 'b%b != 'sh9z",   lsv); err=1; end
      lsv = 'shx9;    if (lsv !== 16'bxxxx_xxxx_xxxx_1001) begin $display("FAILED -- lsv = 'b%b != 'shx9",   lsv); err=1; end
      lsv = 'shz9;    if (lsv !== 16'bzzzz_zzzz_zzzz_1001) begin $display("FAILED -- lsv = 'b%b != 'shz9",   lsv); err=1; end

      // unsized binary literals single character
      lsv = 'sd0;     if (lsv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- lsv = 'b%b != 'sd0",    lsv); err=1; end
      lsv = 'sd9;     if (lsv !== 16'b0000_0000_0000_1001) begin $display("FAILED -- lsv = 'b%b != 'sd9",    lsv); err=1; end
      lsv = 'sdx;     if (lsv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- lsv = 'b%b != 'sdx",    lsv); err=1; end
      lsv = 'sdz;     if (lsv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- lsv = 'b%b != 'sdz",    lsv); err=1; end
      // unsized binary literals two characters
      lsv = 'sd00;    if (lsv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- lsv = 'b%b != 'sd00",   lsv); err=1; end
      lsv = 'sd99;    if (lsv !== 16'b0000_0000_0110_0011) begin $display("FAILED -- lsv = 'b%b != 'sd99",   lsv); err=1; end
//    lsv = 'sdxx;    if (lsv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- lsv = 'b%b != 'sdxx",   lsv); err=1; end
//    lsv = 'sdzz;    if (lsv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- lsv = 'b%b != 'sdzz",   lsv); err=1; end


      // unsized binary literals single character
      lsv = 15'sb0;   if (lsv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- lsv = 'b%b != 15'sb0",  lsv); err=1; end
      lsv = 15'sb1;   if (lsv !== 16'b0000_0000_0000_0001) begin $display("FAILED -- lsv = 'b%b != 15'sb1",  lsv); err=1; end
      lsv = 15'sbx;   if (lsv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- lsv = 'b%b != 15'sbx",  lsv); err=1; end
      lsv = 15'sbz;   if (lsv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- lsv = 'b%b != 15'sbz",  lsv); err=1; end
      // unsized binary literals two characters
      lsv = 15'sb00;  if (lsv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- lsv = 'b%b != 15'sb00", lsv); err=1; end
      lsv = 15'sb11;  if (lsv !== 16'b0000_0000_0000_0011) begin $display("FAILED -- lsv = 'b%b != 15'sb11", lsv); err=1; end
      lsv = 15'sbxx;  if (lsv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- lsv = 'b%b != 15'sbxx", lsv); err=1; end
      lsv = 15'sbzz;  if (lsv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- lsv = 'b%b != 15'sbzz", lsv); err=1; end
      lsv = 15'sb1x;  if (lsv !== 16'b0000_0000_0000_001x) begin $display("FAILED -- lsv = 'b%b != 15'sb1x", lsv); err=1; end
      lsv = 15'sb1z;  if (lsv !== 16'b0000_0000_0000_001z) begin $display("FAILED -- lsv = 'b%b != 15'sb1z", lsv); err=1; end
      lsv = 15'sbx1;  if (lsv !== 16'bxxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- lsv = 'b%b != 15'sbx1", lsv); err=1; end
      lsv = 15'sbz1;  if (lsv !== 16'bzzzz_zzzz_zzzz_zzz1) begin $display("FAILED -- lsv = 'b%b != 15'sbz1", lsv); err=1; end

      // unsized binary literals single character
      lsv = 15'so0;   if (lsv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- lsv = 'b%b != 15'so0",  lsv); err=1; end
      lsv = 15'so5;   if (lsv !== 16'b0000_0000_0000_0101) begin $display("FAILED -- lsv = 'b%b != 15'so5",  lsv); err=1; end
      lsv = 15'sox;   if (lsv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- lsv = 'b%b != 15'sox",  lsv); err=1; end
      lsv = 15'soz;   if (lsv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- lsv = 'b%b != 15'soz",  lsv); err=1; end
      // unsized binary literals two characters
      lsv = 15'so00;  if (lsv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- lsv = 'b%b != 15'so00", lsv); err=1; end
      lsv = 15'so55;  if (lsv !== 16'b0000_0000_0010_1101) begin $display("FAILED -- lsv = 'b%b != 15'so55", lsv); err=1; end
      lsv = 15'soxx;  if (lsv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- lsv = 'b%b != 15'soxx", lsv); err=1; end
      lsv = 15'sozz;  if (lsv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- lsv = 'b%b != 15'sozz", lsv); err=1; end
      lsv = 15'so5x;  if (lsv !== 16'b0000_0000_0010_1xxx) begin $display("FAILED -- lsv = 'b%b != 15'so5x", lsv); err=1; end
      lsv = 15'so5z;  if (lsv !== 16'b0000_0000_0010_1zzz) begin $display("FAILED -- lsv = 'b%b != 15'so5z", lsv); err=1; end
      lsv = 15'sox5;  if (lsv !== 16'bxxxx_xxxx_xxxx_x101) begin $display("FAILED -- lsv = 'b%b != 15'sox5", lsv); err=1; end
      lsv = 15'soz5;  if (lsv !== 16'bzzzz_zzzz_zzzz_z101) begin $display("FAILED -- lsv = 'b%b != 15'soz5", lsv); err=1; end

      // unsized binary literals single character
      lsv = 15'sh0;   if (lsv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- lsv = 'b%b != 15'sh0",  lsv); err=1; end
      lsv = 15'sh9;   if (lsv !== 16'b0000_0000_0000_1001) begin $display("FAILED -- lsv = 'b%b != 15'sh9",  lsv); err=1; end
      lsv = 15'shx;   if (lsv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- lsv = 'b%b != 15'shx",  lsv); err=1; end
      lsv = 15'shz;   if (lsv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- lsv = 'b%b != 15'shz",  lsv); err=1; end
      // unsized binary literals two characters
      lsv = 15'sh00;  if (lsv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- lsv = 'b%b != 15'sh00", lsv); err=1; end
      lsv = 15'sh99;  if (lsv !== 16'b0000_0000_1001_1001) begin $display("FAILED -- lsv = 'b%b != 15'sh99", lsv); err=1; end
      lsv = 15'shxx;  if (lsv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- lsv = 'b%b != 15'shxx", lsv); err=1; end
      lsv = 15'shzz;  if (lsv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- lsv = 'b%b != 15'shzz", lsv); err=1; end
      lsv = 15'sh9x;  if (lsv !== 16'b0000_0000_1001_xxxx) begin $display("FAILED -- lsv = 'b%b != 15'sh9x", lsv); err=1; end
      lsv = 15'sh9z;  if (lsv !== 16'b0000_0000_1001_zzzz) begin $display("FAILED -- lsv = 'b%b != 15'sh9z", lsv); err=1; end
      lsv = 15'shx9;  if (lsv !== 16'bxxxx_xxxx_xxxx_1001) begin $display("FAILED -- lsv = 'b%b != 15'shx9", lsv); err=1; end
      lsv = 15'shz9;  if (lsv !== 16'bzzzz_zzzz_zzzz_1001) begin $display("FAILED -- lsv = 'b%b != 15'shz9", lsv); err=1; end

      // unsized binary literals single character
      lsv = 15'sd0;   if (lsv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- lsv = 'b%b != 15'sd0",  lsv); err=1; end
      lsv = 15'sd9;   if (lsv !== 16'b0000_0000_0000_1001) begin $display("FAILED -- lsv = 'b%b != 15'sd9",  lsv); err=1; end
      lsv = 15'sdx;   if (lsv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- lsv = 'b%b != 15'sdx",  lsv); err=1; end
      lsv = 15'sdz;   if (lsv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- lsv = 'b%b != 15'sdz",  lsv); err=1; end
      // unsized binary literals two characters
      lsv = 15'sd00;  if (lsv !== 16'b0000_0000_0000_0000) begin $display("FAILED -- lsv = 'b%b != 15'sd00", lsv); err=1; end
      lsv = 15'sd99;  if (lsv !== 16'b0000_0000_0110_0011) begin $display("FAILED -- lsv = 'b%b != 15'sd99", lsv); err=1; end
//    lsv = 15'sdxx;  if (lsv !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- lsv = 'b%b != 15'sdxx", lsv); err=1; end
//    lsv = 15'sdzz;  if (lsv !== 16'bzzzz_zzzz_zzzz_zzzz) begin $display("FAILED -- lsv = 'b%b != 15'sdzz", lsv); err=1; end

      if (!err) $display("PASSED");
   end

endmodule // test
