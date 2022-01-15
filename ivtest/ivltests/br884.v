
// This tests unalligned write/read access to packed arrays

module test ();

   // parameters for array sizes
   localparam WA = 4;
   localparam WB = 4;

   // 2D packed array parameters
//   localparam [WA-1:0] [WB-1:0] param_bg = {WA*WB{1'b1}};

   // 2D packed arrays
   logic        [WA-1:0] [WB-1:0] array_bg0, array_bg1, array_bg2, array_bg3, array_bg4, array_bg5, array_bg6, array_bg7, array_bg8, array_bg9;  // big    endian array
   logic        [0:WA-1] [0:WB-1] array_lt0, array_lt1, array_lt2, array_lt3, array_lt4, array_lt5, array_lt6, array_lt7, array_lt8, array_lt9;  // little endian array
   logic                [WA*WB:0] array_1d0, array_1d1, array_1d2, array_1d3, array_1d4, array_1d5, array_1d6, array_1d7, array_1d8, array_1d9;  // 1D array
   logic signed [WA-1:0] [WB-1:0] array_sg0, array_sg1, array_sg2, array_sg3, array_sg4, array_sg5, array_sg6, array_sg7, array_sg8, array_sg9;  // signed big endian array

   // error counter
   int err = 0;

   initial begin
      // test write to array LHS=RHS
      array_bg0 = {WA*WB{1'bx}};
      array_bg1 = {WA*WB{1'bx}};  array_bg1                            = {WA  *WB  +0{1'b1}};
      array_bg2 = {WA*WB{1'bx}};  array_bg2 [WA/2-1:0   ]              = {WA/2*WB  +0{1'b1}};
      array_bg3 = {WA*WB{1'bx}};  array_bg3 [WA  -1:WA/2]              = {WA/2*WB  +0{1'b1}};
      array_bg4 = {WA*WB{1'bx}};  array_bg4 [       0   ]              = {1   *WB  +0{1'b1}};
      array_bg5 = {WA*WB{1'bx}};  array_bg5 [WA  -1     ]              = {1   *WB  +0{1'b1}};
      array_bg6 = {WA*WB{1'bx}};  array_bg6 [       0   ][WB/2-1:0   ] = {1   *WB/2+0{1'b1}};
      array_bg7 = {WA*WB{1'bx}};  array_bg7 [WA  -1     ][WB  -1:WB/2] = {1   *WB/2+0{1'b1}};
      array_bg8 = {WA*WB{1'bx}};  array_bg8 [       0   ][       0   ] = {1   *1   +0{1'b1}};
      array_bg9 = {WA*WB{1'bx}};  array_bg9 [WA  -1     ][WB  -1     ] = {1   *1   +0{1'b1}};
      // check
      if (array_bg0 !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS=RHS -- array_bg0 = 'b%b", array_bg0); err=err+1; end
      if (array_bg1 !== 16'b1111_1111_1111_1111) begin $display("FAILED -- LHS=RHS -- array_bg1 = 'b%b", array_bg1); err=err+1; end
      if (array_bg2 !== 16'bxxxx_xxxx_1111_1111) begin $display("FAILED -- LHS=RHS -- array_bg2 = 'b%b", array_bg2); err=err+1; end
      if (array_bg3 !== 16'b1111_1111_xxxx_xxxx) begin $display("FAILED -- LHS=RHS -- array_bg3 = 'b%b", array_bg3); err=err+1; end
      if (array_bg4 !== 16'bxxxx_xxxx_xxxx_1111) begin $display("FAILED -- LHS=RHS -- array_bg4 = 'b%b", array_bg4); err=err+1; end
      if (array_bg5 !== 16'b1111_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS=RHS -- array_bg5 = 'b%b", array_bg5); err=err+1; end
      if (array_bg6 !== 16'bxxxx_xxxx_xxxx_xx11) begin $display("FAILED -- LHS=RHS -- array_bg6 = 'b%b", array_bg6); err=err+1; end
      if (array_bg7 !== 16'b11xx_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS=RHS -- array_bg7 = 'b%b", array_bg7); err=err+1; end
      if (array_bg8 !== 16'bxxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- LHS=RHS -- array_bg8 = 'b%b", array_bg8); err=err+1; end
      if (array_bg9 !== 16'b1xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS=RHS -- array_bg9 = 'b%b", array_bg9); err=err+1; end

      // test write to array LHS<RHS
      array_bg0 = {WA*WB{1'bx}};
      array_bg1 = {WA*WB{1'bx}};  array_bg1                            = {WA  *WB  +1{1'b1}};
      array_bg2 = {WA*WB{1'bx}};  array_bg2 [WA/2-1:0   ]              = {WA/2*WB  +1{1'b1}};
      array_bg3 = {WA*WB{1'bx}};  array_bg3 [WA  -1:WA/2]              = {WA/2*WB  +1{1'b1}};
      array_bg4 = {WA*WB{1'bx}};  array_bg4 [       0   ]              = {1   *WB  +1{1'b1}};
      array_bg5 = {WA*WB{1'bx}};  array_bg5 [WA  -1     ]              = {1   *WB  +1{1'b1}};
      array_bg6 = {WA*WB{1'bx}};  array_bg6 [       0   ][WB/2-1:0   ] = {1   *WB/2+1{1'b1}};
      array_bg7 = {WA*WB{1'bx}};  array_bg7 [WA  -1     ][WB  -1:WB/2] = {1   *WB/2+1{1'b1}};
      array_bg8 = {WA*WB{1'bx}};  array_bg8 [       0   ][       0   ] = {1   *1   +1{1'b1}};
      array_bg9 = {WA*WB{1'bx}};  array_bg9 [WA  -1     ][WB  -1     ] = {1   *1   +1{1'b1}};
      // check
      if (array_bg0 !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS<RHS -- array_bg0 = 'b%b", array_bg0); err=err+1; end
      if (array_bg1 !== 16'b1111_1111_1111_1111) begin $display("FAILED -- LHS<RHS -- array_bg1 = 'b%b", array_bg1); err=err+1; end
      if (array_bg2 !== 16'bxxxx_xxxx_1111_1111) begin $display("FAILED -- LHS<RHS -- array_bg2 = 'b%b", array_bg2); err=err+1; end
      if (array_bg3 !== 16'b1111_1111_xxxx_xxxx) begin $display("FAILED -- LHS<RHS -- array_bg3 = 'b%b", array_bg3); err=err+1; end
      if (array_bg4 !== 16'bxxxx_xxxx_xxxx_1111) begin $display("FAILED -- LHS<RHS -- array_bg4 = 'b%b", array_bg4); err=err+1; end
      if (array_bg5 !== 16'b1111_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS<RHS -- array_bg5 = 'b%b", array_bg5); err=err+1; end
      if (array_bg6 !== 16'bxxxx_xxxx_xxxx_xx11) begin $display("FAILED -- LHS<RHS -- array_bg6 = 'b%b", array_bg6); err=err+1; end
      if (array_bg7 !== 16'b11xx_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS<RHS -- array_bg7 = 'b%b", array_bg7); err=err+1; end
      if (array_bg8 !== 16'bxxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- LHS<RHS -- array_bg8 = 'b%b", array_bg8); err=err+1; end
      if (array_bg9 !== 16'b1xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS<RHS -- array_bg9 = 'b%b", array_bg9); err=err+1; end

      // test write to array LHS>RHS
      array_bg0 = {WA*WB{1'bx}};
      array_bg1 = {WA*WB{1'bx}};  array_bg1                            = {WA  *WB  -1{1'b1}};
      array_bg2 = {WA*WB{1'bx}};  array_bg2 [WA/2-1:0   ]              = {WA/2*WB  -1{1'b1}};
      array_bg3 = {WA*WB{1'bx}};  array_bg3 [WA  -1:WA/2]              = {WA/2*WB  -1{1'b1}};
      array_bg4 = {WA*WB{1'bx}};  array_bg4 [       0   ]              = {1   *WB  -1{1'b1}};
      array_bg5 = {WA*WB{1'bx}};  array_bg5 [WA  -1     ]              = {1   *WB  -1{1'b1}};
      array_bg6 = {WA*WB{1'bx}};  array_bg6 [       0   ][WB/2-1:0   ] = {1   *WB/2-1{1'b1}};
      array_bg7 = {WA*WB{1'bx}};  array_bg7 [WA  -1     ][WB  -1:WB/2] = {1   *WB/2-1{1'b1}};
    //array_bg8 = {WA*WB{1'bx}};  array_bg8 [       0   ][       0   ] = {1   *1   -1{1'b1}};
    //array_bg9 = {WA*WB{1'bx}};  array_bg9 [WA  -1     ][WB  -1     ] = {1   *1   -1{1'b1}};
      // check
      if (array_bg0 !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS>RHS -- array_bg0 = 'b%b", array_bg0); err=err+1; end
      if (array_bg1 !== 16'b0111_1111_1111_1111) begin $display("FAILED -- LHS>RHS -- array_bg1 = 'b%b", array_bg1); err=err+1; end
      if (array_bg2 !== 16'bxxxx_xxxx_0111_1111) begin $display("FAILED -- LHS>RHS -- array_bg2 = 'b%b", array_bg2); err=err+1; end
      if (array_bg3 !== 16'b0111_1111_xxxx_xxxx) begin $display("FAILED -- LHS>RHS -- array_bg3 = 'b%b", array_bg3); err=err+1; end
      if (array_bg4 !== 16'bxxxx_xxxx_xxxx_0111) begin $display("FAILED -- LHS>RHS -- array_bg4 = 'b%b", array_bg4); err=err+1; end
      if (array_bg5 !== 16'b0111_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS>RHS -- array_bg5 = 'b%b", array_bg5); err=err+1; end
      if (array_bg6 !== 16'bxxxx_xxxx_xxxx_xx01) begin $display("FAILED -- LHS>RHS -- array_bg6 = 'b%b", array_bg6); err=err+1; end
      if (array_bg7 !== 16'b01xx_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS>RHS -- array_bg7 = 'b%b", array_bg7); err=err+1; end
    //if (array_bg8 !== 16'bxxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- LHS>RHS -- array_bg8 = 'b%b", array_bg8); err=err+1; end
    //if (array_bg9 !== 16'b1xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS>RHS -- array_bg9 = 'b%b", array_bg9); err=err+1; end

      // test write to array LHS=RHS
      array_lt0 = {WA*WB{1'bx}};
      array_lt1 = {WA*WB{1'bx}};  array_lt1                            = {WA  *WB  +0{1'b1}};
      array_lt2 = {WA*WB{1'bx}};  array_lt2 [0   :WA/2-1]              = {WA/2*WB  +0{1'b1}};
      array_lt3 = {WA*WB{1'bx}};  array_lt3 [WA/2:WA  -1]              = {WA/2*WB  +0{1'b1}};
      array_lt4 = {WA*WB{1'bx}};  array_lt4 [0          ]              = {1   *WB  +0{1'b1}};
      array_lt5 = {WA*WB{1'bx}};  array_lt5 [     WA  -1]              = {1   *WB  +0{1'b1}};
      array_lt6 = {WA*WB{1'bx}};  array_lt6 [0          ][0   :WB/2-1] = {1   *WB/2+0{1'b1}};
      array_lt7 = {WA*WB{1'bx}};  array_lt7 [     WA  -1][WB/2:WB  -1] = {1   *WB/2+0{1'b1}};
      array_lt8 = {WA*WB{1'bx}};  array_lt8 [0          ][0          ] = {1   *1   +0{1'b1}};
      array_lt9 = {WA*WB{1'bx}};  array_lt9 [     WA  -1][     WB  -1] = {1   *1   +0{1'b1}};
      // check
      if (array_lt0 !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS=RHS -- array_lt0 = 'b%b", array_lt0); err=err+1; end
      if (array_lt1 !== 16'b1111_1111_1111_1111) begin $display("FAILED -- LHS=RHS -- array_lt1 = 'b%b", array_lt1); err=err+1; end
      if (array_lt2 !== 16'b1111_1111_xxxx_xxxx) begin $display("FAILED -- LHS=RHS -- array_lt2 = 'b%b", array_lt2); err=err+1; end
      if (array_lt3 !== 16'bxxxx_xxxx_1111_1111) begin $display("FAILED -- LHS=RHS -- array_lt3 = 'b%b", array_lt3); err=err+1; end
      if (array_lt4 !== 16'b1111_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS=RHS -- array_lt4 = 'b%b", array_lt4); err=err+1; end
      if (array_lt5 !== 16'bxxxx_xxxx_xxxx_1111) begin $display("FAILED -- LHS=RHS -- array_lt5 = 'b%b", array_lt5); err=err+1; end
      if (array_lt6 !== 16'b11xx_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS=RHS -- array_lt6 = 'b%b", array_lt6); err=err+1; end
      if (array_lt7 !== 16'bxxxx_xxxx_xxxx_xx11) begin $display("FAILED -- LHS=RHS -- array_lt7 = 'b%b", array_lt7); err=err+1; end
      if (array_lt8 !== 16'b1xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS=RHS -- array_lt8 = 'b%b", array_lt8); err=err+1; end
      if (array_lt9 !== 16'bxxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- LHS=RHS -- array_lt9 = 'b%b", array_lt9); err=err+1; end

      // test write to array LHS<RHS
      array_lt0 = {WA*WB{1'bx}};
      array_lt1 = {WA*WB{1'bx}};  array_lt1                            = {WA  *WB  +1{1'b1}};
      array_lt2 = {WA*WB{1'bx}};  array_lt2 [0   :WA/2-1]              = {WA/2*WB  +1{1'b1}};
      array_lt3 = {WA*WB{1'bx}};  array_lt3 [WA/2:WA  -1]              = {WA/2*WB  +1{1'b1}};
      array_lt4 = {WA*WB{1'bx}};  array_lt4 [0          ]              = {1   *WB  +1{1'b1}};
      array_lt5 = {WA*WB{1'bx}};  array_lt5 [     WA  -1]              = {1   *WB  +1{1'b1}};
      array_lt6 = {WA*WB{1'bx}};  array_lt6 [0          ][0   :WB/2-1] = {1   *WB/2+1{1'b1}};
      array_lt7 = {WA*WB{1'bx}};  array_lt7 [     WA  -1][WB/2:WB  -1] = {1   *WB/2+1{1'b1}};
      array_lt8 = {WA*WB{1'bx}};  array_lt8 [0          ][0          ] = {1   *1   +1{1'b1}};
      array_lt9 = {WA*WB{1'bx}};  array_lt9 [     WA  -1][     WB  -1] = {1   *1   +1{1'b1}};
      // check
      if (array_lt0 !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS<RHS -- array_lt0 = 'b%b", array_lt0); err=err+1; end
      if (array_lt1 !== 16'b1111_1111_1111_1111) begin $display("FAILED -- LHS<RHS -- array_lt1 = 'b%b", array_lt1); err=err+1; end
      if (array_lt2 !== 16'b1111_1111_xxxx_xxxx) begin $display("FAILED -- LHS<RHS -- array_lt2 = 'b%b", array_lt2); err=err+1; end
      if (array_lt3 !== 16'bxxxx_xxxx_1111_1111) begin $display("FAILED -- LHS<RHS -- array_lt3 = 'b%b", array_lt3); err=err+1; end
      if (array_lt4 !== 16'b1111_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS<RHS -- array_lt4 = 'b%b", array_lt4); err=err+1; end
      if (array_lt5 !== 16'bxxxx_xxxx_xxxx_1111) begin $display("FAILED -- LHS<RHS -- array_lt5 = 'b%b", array_lt5); err=err+1; end
      if (array_lt6 !== 16'b11xx_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS<RHS -- array_lt6 = 'b%b", array_lt6); err=err+1; end
      if (array_lt7 !== 16'bxxxx_xxxx_xxxx_xx11) begin $display("FAILED -- LHS<RHS -- array_lt7 = 'b%b", array_lt7); err=err+1; end
      if (array_lt8 !== 16'b1xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS<RHS -- array_lt8 = 'b%b", array_lt8); err=err+1; end
      if (array_lt9 !== 16'bxxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- LHS<RHS -- array_lt9 = 'b%b", array_lt9); err=err+1; end

      // test write to array LHS>RHS
      array_lt0 = {WA*WB{1'bx}};
      array_lt1 = {WA*WB{1'bx}};  array_lt1                            = {WA  *WB  -1{1'b1}};
      array_lt2 = {WA*WB{1'bx}};  array_lt2 [0   :WA/2-1]              = {WA/2*WB  -1{1'b1}};
      array_lt3 = {WA*WB{1'bx}};  array_lt3 [WA/2:WA  -1]              = {WA/2*WB  -1{1'b1}};
      array_lt4 = {WA*WB{1'bx}};  array_lt4 [0          ]              = {1   *WB  -1{1'b1}};
      array_lt5 = {WA*WB{1'bx}};  array_lt5 [     WA  -1]              = {1   *WB  -1{1'b1}};
      array_lt6 = {WA*WB{1'bx}};  array_lt6 [0          ][0   :WB/2-1] = {1   *WB/2-1{1'b1}};
      array_lt7 = {WA*WB{1'bx}};  array_lt7 [     WA  -1][WB/2:WB  -1] = {1   *WB/2-1{1'b1}};
    //array_lt8 = {WA*WB{1'bx}};  array_lt8 [0          ][0          ] = {1   *1   -1{1'b1}};
    //array_lt9 = {WA*WB{1'bx}};  array_lt9 [     WA  -1][     WB  -1] = {1   *1   -1{1'b1}};
      // check
      if (array_lt0 !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS>RHS -- array_lt0 = 'b%b", array_lt0); err=err+1; end
      if (array_lt1 !== 16'b0111_1111_1111_1111) begin $display("FAILED -- LHS>RHS -- array_lt1 = 'b%b", array_lt1); err=err+1; end
      if (array_lt2 !== 16'b0111_1111_xxxx_xxxx) begin $display("FAILED -- LHS>RHS -- array_lt2 = 'b%b", array_lt2); err=err+1; end
      if (array_lt3 !== 16'bxxxx_xxxx_0111_1111) begin $display("FAILED -- LHS>RHS -- array_lt3 = 'b%b", array_lt3); err=err+1; end
      if (array_lt4 !== 16'b0111_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS>RHS -- array_lt4 = 'b%b", array_lt4); err=err+1; end
      if (array_lt5 !== 16'bxxxx_xxxx_xxxx_0111) begin $display("FAILED -- LHS>RHS -- array_lt5 = 'b%b", array_lt5); err=err+1; end
      if (array_lt6 !== 16'b01xx_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS>RHS -- array_lt6 = 'b%b", array_lt6); err=err+1; end
      if (array_lt7 !== 16'bxxxx_xxxx_xxxx_xx01) begin $display("FAILED -- LHS>RHS -- array_lt7 = 'b%b", array_lt7); err=err+1; end
    //if (array_lt8 !== 16'b1xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- LHS>RHS -- array_lt8 = 'b%b", array_lt8); err=err+1; end
    //if (array_lt9 !== 16'bxxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- LHS>RHS -- array_lt9 = 'b%b", array_lt9); err=err+1; end

      // assign a constant value to the array
      array_bg1 = {WA*WB{1'b1}};
      array_bg2 = {WA*WB{1'b1}};
      array_bg3 = {WA*WB{1'b1}};
      array_bg4 = {WA*WB{1'b1}};
      array_bg5 = {WA*WB{1'b1}};
      array_bg6 = {WA*WB{1'b1}};
      array_bg7 = {WA*WB{1'b1}};
      array_bg8 = {WA*WB{1'b1}};
      array_bg9 = {WA*WB{1'b1}};

      // test read from array LHS=RHS
      array_1d1 = {WA*WB+1{1'bx}};  array_1d1[WA  *WB  -1+0:0] = array_bg1                           ;
      array_1d2 = {WA*WB+1{1'bx}};  array_1d2[WA/2*WB  -1+0:0] = array_bg2 [WA/2-1:0   ]             ;
      array_1d3 = {WA*WB+1{1'bx}};  array_1d3[WA/2*WB  -1+0:0] = array_bg3 [WA  -1:WA/2]             ;
      array_1d4 = {WA*WB+1{1'bx}};  array_1d4[1   *WB  -1+0:0] = array_bg4 [       0   ]             ;
      array_1d5 = {WA*WB+1{1'bx}};  array_1d5[1   *WB  -1+0:0] = array_bg5 [WA  -1     ]             ;
      array_1d6 = {WA*WB+1{1'bx}};  array_1d6[1   *WB/2-1+0:0] = array_bg6 [       0   ][WB/2-1:0   ];
      array_1d7 = {WA*WB+1{1'bx}};  array_1d7[1   *WB/2-1+0:0] = array_bg7 [WA  -1     ][WB  -1:WB/2];
      array_1d8 = {WA*WB+1{1'bx}};  array_1d8[1   *1   -1+0:0] = array_bg8 [       0   ][       0   ];
      array_1d9 = {WA*WB+1{1'bx}};  array_1d9[1   *1   -1+0:0] = array_bg9 [WA  -1     ][WB  -1     ];
      // check
      if (array_1d1 !== 17'bx_1111_1111_1111_1111) begin $display("FAILED -- LHS=RHS BE -- array_1d1 = 'b%b", array_1d1); err=err+1; end
      if (array_1d2 !== 17'bx_xxxx_xxxx_1111_1111) begin $display("FAILED -- LHS=RHS BE -- array_1d2 = 'b%b", array_1d2); err=err+1; end
      if (array_1d3 !== 17'bx_xxxx_xxxx_1111_1111) begin $display("FAILED -- LHS=RHS BE -- array_1d3 = 'b%b", array_1d3); err=err+1; end
      if (array_1d4 !== 17'bx_xxxx_xxxx_xxxx_1111) begin $display("FAILED -- LHS=RHS BE -- array_1d4 = 'b%b", array_1d4); err=err+1; end
      if (array_1d5 !== 17'bx_xxxx_xxxx_xxxx_1111) begin $display("FAILED -- LHS=RHS BE -- array_1d5 = 'b%b", array_1d5); err=err+1; end
      if (array_1d6 !== 17'bx_xxxx_xxxx_xxxx_xx11) begin $display("FAILED -- LHS=RHS BE -- array_1d6 = 'b%b", array_1d6); err=err+1; end
      if (array_1d7 !== 17'bx_xxxx_xxxx_xxxx_xx11) begin $display("FAILED -- LHS=RHS BE -- array_1d7 = 'b%b", array_1d7); err=err+1; end
      if (array_1d8 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- LHS=RHS BE -- array_1d8 = 'b%b", array_1d8); err=err+1; end
      if (array_1d9 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- LHS=RHS BE -- array_1d9 = 'b%b", array_1d9); err=err+1; end

      // test read from array LHS>RHS
      array_1d1 = {WA*WB+1{1'bx}};  array_1d1[WA  *WB  -1+1:0] = array_bg1                           ;
      array_1d2 = {WA*WB+1{1'bx}};  array_1d2[WA/2*WB  -1+1:0] = array_bg2 [WA/2-1:0   ]             ;
      array_1d3 = {WA*WB+1{1'bx}};  array_1d3[WA/2*WB  -1+1:0] = array_bg3 [WA  -1:WA/2]             ;
      array_1d4 = {WA*WB+1{1'bx}};  array_1d4[1   *WB  -1+1:0] = array_bg4 [       0   ]             ;
      array_1d5 = {WA*WB+1{1'bx}};  array_1d5[1   *WB  -1+1:0] = array_bg5 [WA  -1     ]             ;
      array_1d6 = {WA*WB+1{1'bx}};  array_1d6[1   *WB/2-1+1:0] = array_bg6 [       0   ][WB/2-1:0   ];
      array_1d7 = {WA*WB+1{1'bx}};  array_1d7[1   *WB/2-1+1:0] = array_bg7 [WA  -1     ][WB  -1:WB/2];
      array_1d8 = {WA*WB+1{1'bx}};  array_1d8[1   *1   -1+1:0] = array_bg8 [       0   ][       0   ];
      array_1d9 = {WA*WB+1{1'bx}};  array_1d9[1   *1   -1+1:0] = array_bg9 [WA  -1     ][WB  -1     ];
      // check
      if (array_1d1 !== 17'b0_1111_1111_1111_1111) begin $display("FAILED -- LHS>RHS BE -- array_1d1 = 'b%b", array_1d1); err=err+1; end
      if (array_1d2 !== 17'bx_xxxx_xxx0_1111_1111) begin $display("FAILED -- LHS>RHS BE -- array_1d2 = 'b%b", array_1d2); err=err+1; end
      if (array_1d3 !== 17'bx_xxxx_xxx0_1111_1111) begin $display("FAILED -- LHS>RHS BE -- array_1d3 = 'b%b", array_1d3); err=err+1; end
      if (array_1d4 !== 17'bx_xxxx_xxxx_xxx0_1111) begin $display("FAILED -- LHS>RHS BE -- array_1d4 = 'b%b", array_1d4); err=err+1; end
      if (array_1d5 !== 17'bx_xxxx_xxxx_xxx0_1111) begin $display("FAILED -- LHS>RHS BE -- array_1d5 = 'b%b", array_1d5); err=err+1; end
      if (array_1d6 !== 17'bx_xxxx_xxxx_xxxx_x011) begin $display("FAILED -- LHS>RHS BE -- array_1d6 = 'b%b", array_1d6); err=err+1; end
      if (array_1d7 !== 17'bx_xxxx_xxxx_xxxx_x011) begin $display("FAILED -- LHS>RHS BE -- array_1d7 = 'b%b", array_1d7); err=err+1; end
      if (array_1d8 !== 17'bx_xxxx_xxxx_xxxx_xx01) begin $display("FAILED -- LHS>RHS BE -- array_1d8 = 'b%b", array_1d8); err=err+1; end
      if (array_1d9 !== 17'bx_xxxx_xxxx_xxxx_xx01) begin $display("FAILED -- LHS>RHS BE -- array_1d9 = 'b%b", array_1d9); err=err+1; end

      // test read from array LHS<RHS
      array_1d1 = {WA*WB+1{1'bx}};  array_1d1[WA  *WB  -1-1:0] = array_bg1                           ;
      array_1d2 = {WA*WB+1{1'bx}};  array_1d2[WA/2*WB  -1-1:0] = array_bg2 [WA/2-1:0   ]             ;
      array_1d3 = {WA*WB+1{1'bx}};  array_1d3[WA/2*WB  -1-1:0] = array_bg3 [WA  -1:WA/2]             ;
      array_1d4 = {WA*WB+1{1'bx}};  array_1d4[1   *WB  -1-1:0] = array_bg4 [       0   ]             ;
      array_1d5 = {WA*WB+1{1'bx}};  array_1d5[1   *WB  -1-1:0] = array_bg5 [WA  -1     ]             ;
      array_1d6 = {WA*WB+1{1'bx}};  array_1d6[1   *WB/2-1-1:0] = array_bg6 [       0   ][WB/2-1:0   ];
      array_1d7 = {WA*WB+1{1'bx}};  array_1d7[1   *WB/2-1-1:0] = array_bg7 [WA  -1     ][WB  -1:WB/2];
    //array_1d8 = {WA*WB+1{1'bx}};  array_1d8[1   *1   -1-1:0] = array_bg8 [       0   ][       0   ];
    //array_1d9 = {WA*WB+1{1'bx}};  array_1d9[1   *1   -1-1:0] = array_bg9 [WA  -1     ][WB  -1     ];
      // check
      if (array_1d1 !== 17'bx_x111_1111_1111_1111) begin $display("FAILED -- LHS<RHS BE -- array_1d1 = 'b%b", array_1d1); err=err+1; end
      if (array_1d2 !== 17'bx_xxxx_xxxx_x111_1111) begin $display("FAILED -- LHS<RHS BE -- array_1d2 = 'b%b", array_1d2); err=err+1; end
      if (array_1d3 !== 17'bx_xxxx_xxxx_x111_1111) begin $display("FAILED -- LHS<RHS BE -- array_1d3 = 'b%b", array_1d3); err=err+1; end
      if (array_1d4 !== 17'bx_xxxx_xxxx_xxxx_x111) begin $display("FAILED -- LHS<RHS BE -- array_1d4 = 'b%b", array_1d4); err=err+1; end
      if (array_1d5 !== 17'bx_xxxx_xxxx_xxxx_x111) begin $display("FAILED -- LHS<RHS BE -- array_1d5 = 'b%b", array_1d5); err=err+1; end
      if (array_1d6 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- LHS<RHS BE -- array_1d6 = 'b%b", array_1d6); err=err+1; end
      if (array_1d7 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- LHS<RHS BE -- array_1d7 = 'b%b", array_1d7); err=err+1; end
    //if (array_1d8 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- LHS<RHS BE -- array_1d8 = 'b%b", array_1d8); err=err+1; end
    //if (array_1d9 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- LHS<RHS BE -- array_1d9 = 'b%b", array_1d9); err=err+1; end

      // assign a constant value to the array
      array_lt1 = {WA*WB{1'b1}};
      array_lt2 = {WA*WB{1'b1}};
      array_lt3 = {WA*WB{1'b1}};
      array_lt4 = {WA*WB{1'b1}};
      array_lt5 = {WA*WB{1'b1}};
      array_lt6 = {WA*WB{1'b1}};
      array_lt7 = {WA*WB{1'b1}};
      array_lt8 = {WA*WB{1'b1}};
      array_lt9 = {WA*WB{1'b1}};

      // test read from array LHS=RHS
      array_1d1 = {WA*WB+1{1'bx}};  array_1d1[WA  *WB  -1+0:0] = array_lt1                           ;
      array_1d2 = {WA*WB+1{1'bx}};  array_1d2[WA/2*WB  -1+0:0] = array_lt2 [0   :WA/2-1]             ;
      array_1d3 = {WA*WB+1{1'bx}};  array_1d3[WA/2*WB  -1+0:0] = array_lt3 [WA/2:WA  -1]             ;
      array_1d4 = {WA*WB+1{1'bx}};  array_1d4[1   *WB  -1+0:0] = array_lt4 [0          ]             ;
      array_1d5 = {WA*WB+1{1'bx}};  array_1d5[1   *WB  -1+0:0] = array_lt5 [     WA  -1]             ;
      array_1d6 = {WA*WB+1{1'bx}};  array_1d6[1   *WB/2-1+0:0] = array_lt6 [0          ][0   :WB/2-1];
      array_1d7 = {WA*WB+1{1'bx}};  array_1d7[1   *WB/2-1+0:0] = array_lt7 [     WA  -1][WB/2:WB  -1];
      array_1d8 = {WA*WB+1{1'bx}};  array_1d8[1   *1   -1+0:0] = array_lt8 [0          ][0          ];
      array_1d9 = {WA*WB+1{1'bx}};  array_1d9[1   *1   -1+0:0] = array_lt9 [     WA  -1][     WB  -1];
      // check
      if (array_1d1 !== 17'bx_1111_1111_1111_1111) begin $display("FAILED -- LHS=RHS LE -- array_1d1 = 'b%b", array_1d1); err=err+1; end
      if (array_1d2 !== 17'bx_xxxx_xxxx_1111_1111) begin $display("FAILED -- LHS=RHS LE -- array_1d2 = 'b%b", array_1d2); err=err+1; end
      if (array_1d3 !== 17'bx_xxxx_xxxx_1111_1111) begin $display("FAILED -- LHS=RHS LE -- array_1d3 = 'b%b", array_1d3); err=err+1; end
      if (array_1d4 !== 17'bx_xxxx_xxxx_xxxx_1111) begin $display("FAILED -- LHS=RHS LE -- array_1d4 = 'b%b", array_1d4); err=err+1; end
      if (array_1d5 !== 17'bx_xxxx_xxxx_xxxx_1111) begin $display("FAILED -- LHS=RHS LE -- array_1d5 = 'b%b", array_1d5); err=err+1; end
      if (array_1d6 !== 17'bx_xxxx_xxxx_xxxx_xx11) begin $display("FAILED -- LHS=RHS LE -- array_1d6 = 'b%b", array_1d6); err=err+1; end
      if (array_1d7 !== 17'bx_xxxx_xxxx_xxxx_xx11) begin $display("FAILED -- LHS=RHS LE -- array_1d7 = 'b%b", array_1d7); err=err+1; end
      if (array_1d8 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- LHS=RHS LE -- array_1d8 = 'b%b", array_1d8); err=err+1; end
      if (array_1d9 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- LHS=RHS LE -- array_1d9 = 'b%b", array_1d9); err=err+1; end

      // test read from array LHS>RHS
      array_1d1 = {WA*WB+1{1'bx}};  array_1d1[WA  *WB  -1+1:0] = array_lt1                           ;
      array_1d2 = {WA*WB+1{1'bx}};  array_1d2[WA/2*WB  -1+1:0] = array_lt2 [0   :WA/2-1]             ;
      array_1d3 = {WA*WB+1{1'bx}};  array_1d3[WA/2*WB  -1+1:0] = array_lt3 [WA/2:WA  -1]             ;
      array_1d4 = {WA*WB+1{1'bx}};  array_1d4[1   *WB  -1+1:0] = array_lt4 [0          ]             ;
      array_1d5 = {WA*WB+1{1'bx}};  array_1d5[1   *WB  -1+1:0] = array_lt5 [     WA  -1]             ;
      array_1d6 = {WA*WB+1{1'bx}};  array_1d6[1   *WB/2-1+1:0] = array_lt6 [0          ][0   :WB/2-1];
      array_1d7 = {WA*WB+1{1'bx}};  array_1d7[1   *WB/2-1+1:0] = array_lt7 [     WA  -1][WB/2:WB  -1];
      array_1d8 = {WA*WB+1{1'bx}};  array_1d8[1   *1   -1+1:0] = array_lt8 [0          ][0          ];
      array_1d9 = {WA*WB+1{1'bx}};  array_1d9[1   *1   -1+1:0] = array_lt9 [     WA  -1][     WB  -1];
      // check
      if (array_1d1 !== 17'b0_1111_1111_1111_1111) begin $display("FAILED -- LHS>RHS LE -- array_1d1 = 'b%b", array_1d1); err=err+1; end
      if (array_1d2 !== 17'bx_xxxx_xxx0_1111_1111) begin $display("FAILED -- LHS>RHS LE -- array_1d2 = 'b%b", array_1d2); err=err+1; end
      if (array_1d3 !== 17'bx_xxxx_xxx0_1111_1111) begin $display("FAILED -- LHS>RHS LE -- array_1d3 = 'b%b", array_1d3); err=err+1; end
      if (array_1d4 !== 17'bx_xxxx_xxxx_xxx0_1111) begin $display("FAILED -- LHS>RHS LE -- array_1d4 = 'b%b", array_1d4); err=err+1; end
      if (array_1d5 !== 17'bx_xxxx_xxxx_xxx0_1111) begin $display("FAILED -- LHS>RHS LE -- array_1d5 = 'b%b", array_1d5); err=err+1; end
      if (array_1d6 !== 17'bx_xxxx_xxxx_xxxx_x011) begin $display("FAILED -- LHS>RHS LE -- array_1d6 = 'b%b", array_1d6); err=err+1; end
      if (array_1d7 !== 17'bx_xxxx_xxxx_xxxx_x011) begin $display("FAILED -- LHS>RHS LE -- array_1d7 = 'b%b", array_1d7); err=err+1; end
      if (array_1d8 !== 17'bx_xxxx_xxxx_xxxx_xx01) begin $display("FAILED -- LHS>RHS LE -- array_1d8 = 'b%b", array_1d8); err=err+1; end
      if (array_1d9 !== 17'bx_xxxx_xxxx_xxxx_xx01) begin $display("FAILED -- LHS>RHS LE -- array_1d9 = 'b%b", array_1d9); err=err+1; end

      // test read from array LHS<RHS
      array_1d1 = {WA*WB+1{1'bx}};  array_1d1[WA  *WB  -1-1:0] = array_lt1                           ;
      array_1d2 = {WA*WB+1{1'bx}};  array_1d2[WA/2*WB  -1-1:0] = array_lt2 [0   :WA/2-1]             ;
      array_1d3 = {WA*WB+1{1'bx}};  array_1d3[WA/2*WB  -1-1:0] = array_lt3 [WA/2:WA  -1]             ;
      array_1d4 = {WA*WB+1{1'bx}};  array_1d4[1   *WB  -1-1:0] = array_lt4 [0          ]             ;
      array_1d5 = {WA*WB+1{1'bx}};  array_1d5[1   *WB  -1-1:0] = array_lt5 [     WA  -1]             ;
      array_1d6 = {WA*WB+1{1'bx}};  array_1d6[1   *WB/2-1-1:0] = array_lt6 [0          ][0   :WB/2-1];
      array_1d7 = {WA*WB+1{1'bx}};  array_1d7[1   *WB/2-1-1:0] = array_lt7 [     WA  -1][WB/2:WB  -1];
    //array_1d8 = {WA*WB+1{1'bx}};  array_1d8[1   *1   -1-1:0] = array_lt8 [0          ][0          ];
    //array_1d9 = {WA*WB+1{1'bx}};  array_1d9[1   *1   -1-1:0] = array_lt9 [     WA  -1][     WB  -1];
      // check
      if (array_1d1 !== 17'bx_x111_1111_1111_1111) begin $display("FAILED -- LHS<RHS LE -- array_1d1 = 'b%b", array_1d1); err=err+1; end
      if (array_1d2 !== 17'bx_xxxx_xxxx_x111_1111) begin $display("FAILED -- LHS<RHS LE -- array_1d2 = 'b%b", array_1d2); err=err+1; end
      if (array_1d3 !== 17'bx_xxxx_xxxx_x111_1111) begin $display("FAILED -- LHS<RHS LE -- array_1d3 = 'b%b", array_1d3); err=err+1; end
      if (array_1d4 !== 17'bx_xxxx_xxxx_xxxx_x111) begin $display("FAILED -- LHS<RHS LE -- array_1d4 = 'b%b", array_1d4); err=err+1; end
      if (array_1d5 !== 17'bx_xxxx_xxxx_xxxx_x111) begin $display("FAILED -- LHS<RHS LE -- array_1d5 = 'b%b", array_1d5); err=err+1; end
      if (array_1d6 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- LHS<RHS LE -- array_1d6 = 'b%b", array_1d6); err=err+1; end
      if (array_1d7 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- LHS<RHS LE -- array_1d7 = 'b%b", array_1d7); err=err+1; end
    //if (array_1d8 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- LHS<RHS LE -- array_1d8 = 'b%b", array_1d8); err=err+1; end
    //if (array_1d9 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- LHS<RHS LE -- array_1d9 = 'b%b", array_1d9); err=err+1; end

      if (err)  $finish();
      else      $display("PASSED");
   end

endmodule // test
