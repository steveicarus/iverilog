// This tests unalligned write/read access to packed structures
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2012 by Iztok Jeras.

module test;

   typedef struct packed {
      logic [7:0] high;
      logic [7:0] low;
   } word_t;

   // Declare word* as a VARIABLE
   wire word_t word_se0, word_se1, word_se2, word_se3;
   wire word_t word_sw0, word_sw1, word_sw2, word_sw3;
   wire word_t word_sp0, word_sp1, word_sp2, word_sp3;
   wire word_t word_ep0, word_ep1, word_ep2, word_ep3;

   // error counter
   bit err = 0;

   // access to structure elements
   assign word_se1.high       = {8+0{1'b1}};
   assign word_se1.low        = {8+0{1'b0}};
   assign word_se2.high       = {8+1{1'b1}};
   assign word_se2.low        = {8+1{1'b0}};
   assign word_se3.high       = {8-1{1'b1}};
   assign word_se3.low        = {8-1{1'b0}};
   // access to whole structure
   assign word_sw1            = {16+0{1'b1}};
   assign word_sw2            = {16+1{1'b1}};
   assign word_sw3            = {16-1{1'b1}};
   // access to parts of structure elements
   assign word_ep1.high [3:0] = {4+0{1'b1}};
   assign word_ep1.low  [3:0] = {4+0{1'b0}};
   assign word_ep2.high [3:0] = {4+1{1'b1}};
   assign word_ep2.low  [3:0] = {4+1{1'b0}};
   assign word_ep3.high [3:0] = {4-1{1'b1}};
   assign word_ep3.low  [3:0] = {4-1{1'b0}};
   // access to parts of the whole structure
   assign word_sp1     [11:4] = {8+0{1'b1}};
   assign word_sp2     [11:4] = {8+1{1'b1}};
   assign word_sp3     [11:4] = {8-1{1'b1}};

   initial begin
      #1;
      // access to structure elements
      if (word_se0      !== 16'bzzzzzzzz_zzzzzzzz) begin $display("FAILED -- word_se0      = 'b%b", word_se0     ); err=1; end
      if (word_se1      !== 16'b11111111_00000000) begin $display("FAILED -- word_se1      = 'b%b", word_se1     ); err=1; end
      if (word_se1.high !==  8'b11111111         ) begin $display("FAILED -- word_se1.high = 'b%b", word_se1.high); err=1; end
      if (word_se1.low  !==  8'b00000000         ) begin $display("FAILED -- word_se1.low  = 'b%b", word_se1.low ); err=1; end
      if (word_se2      !== 16'b11111111_00000000) begin $display("FAILED -- word_se2      = 'b%b", word_se2     ); err=1; end
      if (word_se2.high !==  8'b11111111         ) begin $display("FAILED -- word_se2.high = 'b%b", word_se2.high); err=1; end
      if (word_se2.low  !==  8'b00000000         ) begin $display("FAILED -- word_se2.low  = 'b%b", word_se2.low ); err=1; end
      if (word_se3      !== 16'b01111111_00000000) begin $display("FAILED -- word_se3      = 'b%b", word_se3     ); err=1; end
      if (word_se3.high !==  8'b01111111         ) begin $display("FAILED -- word_se3.high = 'b%b", word_se3.high); err=1; end
      if (word_se3.low  !==  8'b00000000         ) begin $display("FAILED -- word_se3.low  = 'b%b", word_se3.low ); err=1; end
      // access to whole structure
      if (word_sw0      !== 16'bzzzzzzzz_zzzzzzzz) begin $display("FAILED -- word_sw0      = 'b%b", word_sw0     ); err=1; end
      if (word_sw1      !== 16'b11111111_11111111) begin $display("FAILED -- word_sw1      = 'b%b", word_sw1     ); err=1; end
      if (word_sw2      !== 16'b11111111_11111111) begin $display("FAILED -- word_sw2      = 'b%b", word_sw2     ); err=1; end
      if (word_sw3      !== 16'b01111111_11111111) begin $display("FAILED -- word_sw3      = 'b%b", word_sw3     ); err=1; end
      // access to parts of structure elements
      if (word_ep0      !== 16'bzzzzzzzz_zzzzzzzz) begin $display("FAILED -- word_ep0      = 'b%b", word_ep0     ); err=1; end
      if (word_ep1      !== 16'bzzzz1111_zzzz0000) begin $display("FAILED -- word_ep1      = 'b%b", word_ep1     ); err=1; end
      if (word_ep1.high !==  8'bzzzz1111         ) begin $display("FAILED -- word_ep1.high = 'b%b", word_ep1.high); err=1; end
      if (word_ep1.low  !==  8'bzzzz0000         ) begin $display("FAILED -- word_ep1.low  = 'b%b", word_ep1.low ); err=1; end
      if (word_ep2      !== 16'bzzzz1111_zzzz0000) begin $display("FAILED -- word_ep2      = 'b%b", word_ep2     ); err=1; end
      if (word_ep2.high !==  8'bzzzz1111         ) begin $display("FAILED -- word_ep2.high = 'b%b", word_ep2.high); err=1; end
      if (word_ep2.low  !==  8'bzzzz0000         ) begin $display("FAILED -- word_ep2.low  = 'b%b", word_ep2.low ); err=1; end
      if (word_ep3      !== 16'bzzzz0111_zzzz0000) begin $display("FAILED -- word_ep3      = 'b%b", word_ep3     ); err=1; end
      if (word_ep3.high !==  8'bzzzz0111         ) begin $display("FAILED -- word_ep3.high = 'b%b", word_ep3.high); err=1; end
      if (word_ep3.low  !==  8'bzzzz0000         ) begin $display("FAILED -- word_ep3.low  = 'b%b", word_ep3.low ); err=1; end
      // access to parts of the whole structure
      if (word_sp0      !== 16'bzzzzzzzz_zzzzzzzz) begin $display("FAILED -- word_sp0      = 'b%b", word_sp0     ); err=1; end
      if (word_sp1      !== 16'bzzzz1111_1111zzzz) begin $display("FAILED -- word_sp1      = 'b%b", word_sp1     ); err=1; end
      if (word_sp2      !== 16'bzzzz1111_1111zzzz) begin $display("FAILED -- word_sp2      = 'b%b", word_sp2     ); err=1; end
      if (word_sp3      !== 16'bzzzz0111_1111zzzz) begin $display("FAILED -- word_sp3      = 'b%b", word_sp3     ); err=1; end

      if (!err) $display("PASSED");
   end

endmodule // test
