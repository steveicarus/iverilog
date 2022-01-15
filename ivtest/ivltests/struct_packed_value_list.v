// This tests assigning value lists to packed structures
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2012 by Iztok Jeras.

module test;

   typedef struct packed {
      logic [7:0] high;
      logic [7:0] low;
   } word_t;

   // Declare word* as a VARIABLE
   word_t word0, word1, word2, word3, word4, word5;

   // error counter
   bit err = 0;

   // access to structure elements
   assign word0 = '{2, 3};
   assign word1 = '{high:5, low:6};
   assign word2 = '{low:7, high:8};
   assign word3 = '{default:13};
   assign word4 = '{high:8'haa, default:1};
   assign word5 = '{high:9'h000, low:9'h1ff};

   initial begin
      #1;
      // check for correctness
      if (word0 !== 16'b00000010_00000011) begin $display("FAILED -- word0 = 'b%b", word0); err=1; end
      if (word1 !== 16'b00000101_00000110) begin $display("FAILED -- word1 = 'b%b", word1); err=1; end
      if (word2 !== 16'b00001000_00000111) begin $display("FAILED -- word2 = 'b%b", word2); err=1; end
      if (word3 !== 16'b00001101_00001101) begin $display("FAILED -- word3 = 'b%b", word3); err=1; end
      if (word4 !== 16'b10101010_00000001) begin $display("FAILED -- word4 = 'b%b", word4); err=1; end
      if (word5 !== 16'b00000000_11111111) begin $display("FAILED -- word5 = 'b%b", word5); err=1; end

      if (!err) $display("PASSED");
   end

endmodule // test
