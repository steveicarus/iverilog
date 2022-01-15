// This tests SystemVerilog packages
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2012 by Iztok Jeras.

package p1;
   localparam int p1_prmt = 100+10+1;
   typedef bit [10+1-1:0] p1_type;
   function int p1_func (int x);
      p1_func = x+10+1;
   endfunction
endpackage

package p2;
   localparam int p1_prmt = 100+20+1;
   typedef bit [20+1-1:0] p1_type;
   function int p1_func (int x);
      p1_func = x+20+1;
   endfunction

   localparam int p2_prmt = 100+20+2;
   typedef bit [20+2-1:0] p2_type;
   function int p2_func (int x);
      p2_func = x+20+2;
   endfunction
endpackage

package p3;
   localparam int p1_prmt = 100+30+1;
   typedef bit [30+1-1:0] p1_type;
   function int p1_func (int x);
      p1_func = x+30+1;
   endfunction

   localparam int p2_prmt = 100+30+2;
   typedef bit [30+2-1:0] p2_type;
   function int p2_func (int x);
      p2_func = x+30+2;
   endfunction

   localparam int p3_prmt = 100+30+3;
   typedef bit [30+3-1:0] p3_type;
   function int p3_func (int x);
      p3_func = x+30+3;
   endfunction
endpackage


module test ();

   // import all from p1
   import p1::*;
   // import only p2_* from p2
   import p2::p2_prmt;
   import p2::p2_type;
   import p2::p2_func;
   // import nothing from p3

   // declare a set of variables
       p1_type p1_var;
       p2_type p2_var;
   p3::p3_type p3_var;

   // error counter
   bit err = 0;

   initial begin
      // test parameters
      if (    p1_prmt !== 100+10+1) begin $display("FAILED --     p1_prmt = %d != 100+10+1",     p1_prmt); err=1; end
      if (    p2_prmt !== 100+20+2) begin $display("FAILED --     p2_prmt = %d != 100+20+2",     p2_prmt); err=1; end
      if (p3::p3_prmt !== 100+30+3) begin $display("FAILED -- p3::p3_prmt = %d != 100+30+3", p3::p3_prmt); err=1; end
      // test variable bit sizes
      if ($bits(p1_var) !== 10+1) begin $display("FAILED -- lv = %d != 10+1", $bits(p1_var)); err=1; end
      if ($bits(p2_var) !== 20+2) begin $display("FAILED -- lv = %d != 20+2", $bits(p2_var)); err=1; end
      if ($bits(p3_var) !== 30+3) begin $display("FAILED -- lv = %d != 30+3", $bits(p3_var)); err=1; end
      // test functions
      if (    p1_func(1000) !== 1000+10+1) begin $display("FAILED --     p1_func(1000) = %d != 1000+10+1",     p1_func(1000)); err=1; end
      if (    p2_func(1000) !== 1000+20+2) begin $display("FAILED --     p2_func(1000) = %d != 1000+20+2",     p2_func(1000)); err=1; end
      if (p3::p3_func(1000) !== 1000+30+3) begin $display("FAILED -- p3::p3_func(1000) = %d != 1000+30+3", p3::p3_func(1000)); err=1; end

      if (!err) $display("PASSED");
   end

endmodule // test
