/**
 * Author:  Evan Lavelle, Riverside Machines Ltd.
 * Version: 0.1 (2007-05-22)
 * Licence: This code is released into the public domain.
 *
 * Test implicit Verilog-95 style ports. According to 12.3.2 of the 2005
 * LRM:
 *
 * "The port reference for each port in the list of ports at the top of each
 * module declaration can be one of the following:
 *
 * A simple identifier or escaped identifier
 * A bit-select of a vector declared within the module
 * A part-select of a vector declared within the module
 * A concatenation of any of the above
 *
 * The port expression is optional because ports can be defined that do not
 * connect to anything internal to the module."
 *
 * The expected output is:
 *
 * sum[          1] = 0101010101100000
 * sum[          2] = 0101010101100000
 * sum[          3] = 0101010101100000
 * sum[          4] = 0101010101100000
 * sum[          5] = 0101010101100000
 * sum[          6] = 0101010101100000
 * sum[          7] = 0101010101100000
 * sum[          8] = 0101010101100000
 * sum[          9] = 0101010101100000
 * sum[         10] = 0101010101100000
 * sum[         11] = 0101010101100000
 * sum[         12] = 0101010101100000
 * sum[         13] = 0101010101100000
 * sum[         14] = 0101010101100000
 * sum[         15] = 0101010101100000
 * sum[         16] = 0101010101100000
 * sum[         17] = 0101010101100000
 * sum[         18] = 0101010101100000
 * sum[         19] = 0101010101100000
 * sum[         20] = 0101010101100000
 * sum[         21] = 0101010101100000
 * sum[         22] = 0101010101100000
 * sum[         23] = 0101010101100000
 *
 */
module test;

   reg  [15:0] sum[23:1];
   wire [7:0]  data = 1;
   wire        dummy1, dummy2;
   wire [15:0] wire5, wire9, wire13, wire17, wire21;

   initial
     main;

   task main;
      integer i;
      begin
	 for(i=1; i<=23; i=i+1)
	   sum[i] = 'h555f;
	 #2;

	 sum[5]  = wire5;
	 sum[9]  = wire9;
	 sum[13] = wire13;
	 sum[17] = wire17;
	 sum[21] = wire21;

	 for(i=1; i<=23; i=i+1)
	   $display("sum[%d] = %b", i, sum[i]);
      end
   endtask

   m1 m1();
   m2 m2(dummy1, dummy2);
   m3 m3(dummy1, , dummy2);

   m4 m4(data);
   m5 m5(data,   wire5);
   m6 m6(dummy1, data);
   m7 m7(data,       );

   m8  m8 (data);
   m9  m9 (data, wire9);
   m10 m10(    , data);
   m11 m11(data,     );

   m12 m12(data[0]);
   m13 m13(data[0], wire13);
   m14 m14(       , data[0]);
   m15 m15(data[0],     );

   m16 m16(data);
   m17 m17(data,   wire17);
   m18 m18(dummy1, data);
   m19 m19(data,       );

   m20 m20(data);
   m21 m21(data,   wire21);
   m22 m22(dummy1, data);
   m23 m23(data,       );

endmodule

/* ----------------------------------------------------------------------------
 * the test modules
 * ------------------------------------------------------------------------- */

// 95, no ports
module m1;
   initial #1 test.sum[1] = test.sum[1] + test.data;
endmodule

// 95, two ports, but neither has an internal connection
module m2(,);
   initial #1 test.sum[2] = test.sum[2] + test.data;
endmodule

// 95, three ports, but none have an internal connection
module m3(,,);
   initial #1 test.sum[3] = test.sum[3] + test.data;
endmodule

/* ----------------------------------------------------------------------------
 * 95, one and two ports, with implicit and simple identifiers
 * ------------------------------------------------------------------------- */

// 95, one implicit port, simple identifier
module m4(a);
   input a;
   wire [7:0] a;
   initial #1 test.sum[4] = test.sum[4] + a;
endmodule

// 95, two implicit ports, simple identifiers
module m5(a, b);
   input  a;
   output b;
   wire [7:0] a;
   reg [15:0] b;
   initial #1 b <= test.sum[5] + a;
endmodule

// 95, two ports; the first has no internal connection; the second is implicit/
// simple
module m6(,a);
   input a;
   wire [7:0] a;
   initial #1 test.sum[6] = test.sum[6] + a;
endmodule

// 95, two ports; the second has no internal connection; the first is implicit/
// simple
module m7(a,);
   input  a;
   wire [7:0] a;
   initial #1 test.sum[7] = test.sum[7] + a;
endmodule

/* ----------------------------------------------------------------------------
 * 95, one and two ports, with implicit and extended identifiers
 * ------------------------------------------------------------------------- */

// 95, one implicit port, extended identifier
module m8(\a );
   input \a ;
   wire [7:0] \a ;
   initial #1 test.sum[8] = test.sum[8] + \a ;
endmodule

// 95, two implicit ports, extended identifiers
module m9(\a , \b );
   input  \a ;
   output \b ;
   wire [7:0] \a ;
   reg [15:0] \b ;
   initial #1 \b  = test.sum[9] + \a ;
endmodule

// 95, two ports; the first has no internal connection; the second is implicit/
// extended
module m10(,\a );
   input  \a ;
   wire [7:0] \a ;
   initial #1 test.sum[10] = test.sum[10] + \a ;
endmodule

// 95, two ports; the second has no internal connection; the first is implicit/
// extended
module m11(\a ,);
   input  \a ;
   wire [7:0] \a ;
   initial #1 test.sum[11] = test.sum[11] + \a ;
endmodule

/* ----------------------------------------------------------------------------
 * 95, one and two ports, with implicit and vector bit-select ports
 * ------------------------------------------------------------------------- */

// 95, one implicit port, vector bit-select
module m12(a[0]);
   input a;
   wire [7:0] a;
   initial #1 test.sum[12] = test.sum[12] + {test.data[7:1], a[0]};
endmodule

// 95, two implicit ports, vector bit-selects. the output is actually a part
// select, since ISE core dumps on the assign below, and doing anything
// else in -95 is difficult
module m13(a[0], b[15:0]);
   input  a;
   output b;
   wire [7:0]  a;
   reg  [31:0] b;
   reg  [15:0] temp;

//   assign test.wire13[15:1] = temp[15:1];        // drives the rest of wire13

   initial begin
      #1 temp = test.sum[13] + {test.data[7:1], a[0]};
      b = temp;                                    // drives wire13[0]
   end
endmodule

// 95, two ports; the first has no internal connection; the second is implicit/
// vector bit-select
module m14(,a[0]);
   input  [7:0] a;
   initial #1 test.sum[14] = test.sum[14] + {test.data[7:1], a[0]};
endmodule

// 95, two ports; the second has no internal connection; the first is implicit/
// vector bit-select
module m15(a[0],);
   input  [7:0] a;
   initial #1 test.sum[15] = test.sum[15] + {test.data[7:1], a[0]};
endmodule

/* ----------------------------------------------------------------------------
 * 95, one and two ports, with implicit and vector part-select ports
 * ------------------------------------------------------------------------- */

// 95, one implicit port, vector bit-select
module m16(a[7:0]);
   input a;
   wire [15:0] a;
   initial #1 test.sum[16] = test.sum[16] + {8'h00, a[7:0]};
endmodule

// 95, two implicit ports, vector bit-selects
module m17(a[7:0], b[15:0]);
   input  a;
   output b;
   wire [7:0] a;
   reg [31:0] b;
   initial #1 b[15:0] = test.sum[17] + a;
endmodule

// 95, two ports; the first has no internal connection; the second is implicit/
// vector part-select
module m18(,a[7:0]);
   input  [15:0] a;
   initial #1 test.sum[18] = test.sum[18] + {8'h00, a[7:0]};
endmodule

// 95, two ports; the second has no internal connection; the first is implicit/
// vector part-select
module m19(a[7:0],);
   input  [15:0] a;
// initial #1 test.sum[19] = test.sum[19] + a;
   initial #1 test.sum[19] = test.sum[19] + {8'h00, a[7:0]};
endmodule

/* ----------------------------------------------------------------------------
 * 95, one and two ports, with ports which are a concatenation of a bit select
 * and a 3-bit part select
 * ------------------------------------------------------------------------- */

// 95, one implicit port, concatenation
module m20({a[7], a[6:0]});
   input a;
   wire [15:0] a;
   initial #1 test.sum[20] = test.sum[20] + {8'h00, a[7:0]};
endmodule

// 95, two implicit ports, concatenations
module m21({a[7], a[6:0]}, {b[15], b[14:0]});
   input  a;
   output b;
   wire [7:0] a;
   reg [15:0] b;
   initial #1 b = test.sum[21] + {8'h00, a[7:0]};
endmodule

// 95, two ports; the first has no internal connection; the second is implicit/
// concatenation
module m22(,{a[7], a[6:0]});
   input  [15:0] a;
   initial #1 test.sum[22] = test.sum[22] + {8'h00, a[7:0]};
endmodule

// 95, two ports; the second has no internal connection; the first is implicit/
// concatenation. note that both modelsim and ISE set the entire sum, and not
// just the top 8 bits, to all x's for 'test.sum[23] = test.sum[23] + a'
module m23({a[7], a[6:0]},);
   input a;
   wire [15:0] a;
   initial #1 test.sum[23] = test.sum[23] + a[7:0];
endmodule
