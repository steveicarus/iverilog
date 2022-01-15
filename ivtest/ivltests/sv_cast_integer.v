// This tests SystemVerilog casting support
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2012 by Iztok Jeras.
// Extended by Maciej Suminski
// Extended by Martin Whitaker

module test();

   // variables used in casting
   byte       var_08;
   shortint   var_16;
   int        var_32;
   longint    var_64;
   real       var_real;

   // error counter
   bit err = 0;

   initial begin
      var_08 = byte'(4'sh5);        if (var_08 !==  8'sh05) begin $display("FAILED -- var_08 =  'h%0h !=  8'h05", var_08); err=1; end
      var_16 = shortint'(var_08);   if (var_16 !== 16'sh05) begin $display("FAILED -- var_16 =  'h%0h != 16'h05", var_16); err=1; end
      var_32 = int'(var_16);        if (var_32 !== 32'sh05) begin $display("FAILED -- var_32 =  'h%0h != 32'h05", var_32); err=1; end
      var_64 = longint'(var_32);    if (var_64 !== 64'sh05) begin $display("FAILED -- var_64 =  'h%0h != 64'h05", var_64); err=1; end

      var_real = 13.4;  var_08 = byte'(var_real);       if (var_08 !==  13) begin $display("FAILED -- var_08 = %d != 13", var_08); err=1; end
      var_real = 14.5;  var_16 = shortint'(var_real);   if (var_16 !==  15) begin $display("FAILED -- var_16 = %d != 15", var_16); err=1; end
      var_real = 15.6;  var_32 = int'(var_real);        if (var_32 !==  16) begin $display("FAILED -- var_32 = %d != 16", var_32); err=1; end
      var_real = -15.6; var_64 = longint'(var_real);    if (var_64 !== -16) begin $display("FAILED -- var_64 = %d != -16", var_64); err=1; end

      var_08 = byte'(4'hf);         if (var_08 !==  8'sh0f) begin $display("FAILED -- var_08 =  'h%0h !=  8'h0f", var_08); err=1; end
      var_08 = byte'(4'shf);        if (var_08 !==  8'shff) begin $display("FAILED -- var_08 =  'h%0h !=  8'hff", var_08); err=1; end
      var_16 = byte'(16'h0f0f);     if (var_16 !== 16'sh0f) begin $display("FAILED -- var_16 =  'h%0h != 16'h0f", var_16); err=1; end
      var_16 = byte'(4'shf) + 'd0;  if (var_16 !== 16'shff) begin $display("FAILED -- var_16 =  'h%0h != 16'hff", var_16); err=1; end

      if (!err) $display("PASSED");
   end

endmodule // test
