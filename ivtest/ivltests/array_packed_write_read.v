// This tests unalligned write/read access to packed arrays
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2012 by Iztok Jeras.

module test ();

   // parameters for array sizes
   localparam WA = 4;
   localparam WB = 4;

   // 2D packed array parameters
//   localparam [WA-1:0] [WB-1:0] param_bg = {WA*WB{1'b1}};

   // 2D packed arrays
   logic        [WA-1:0] [WB-1:0] abg0, abg1, abg2, abg3, abg4, abg5, abg6, abg7, abg8, abg9;  // big    endian array
   logic        [0:WA-1] [0:WB-1] alt0, alt1, alt2, alt3, alt4, alt5, alt6, alt7, alt8, alt9;  // little endian array
   logic                [WA*WB:0] a1d0, a1d1, a1d2, a1d3, a1d4, a1d5, a1d6, a1d7, a1d8, a1d9;  // 1D array
   logic signed [WA-1:0] [WB-1:0] asg0, asg1, asg2, asg3, asg4, asg5, asg6, asg7, asg8, asg9;  // signed big endian array

   // error counter
   bit err = 0;

   initial begin
      // test write to array LHS=RHS
      abg0 = {WA*WB{1'bx}};
      abg1 = {WA*WB{1'bx}};  abg1                            = {WA  *WB  +0{1'b1}};
      abg2 = {WA*WB{1'bx}};  abg2 [WA/2-1:0   ]              = {WA/2*WB  +0{1'b1}};
      abg3 = {WA*WB{1'bx}};  abg3 [WA  -1:WA/2]              = {WA/2*WB  +0{1'b1}};
      abg4 = {WA*WB{1'bx}};  abg4 [       0   ]              = {1   *WB  +0{1'b1}};
      abg5 = {WA*WB{1'bx}};  abg5 [WA  -1     ]              = {1   *WB  +0{1'b1}};
      abg6 = {WA*WB{1'bx}};  abg6 [       0   ][WB/2-1:0   ] = {1   *WB/2+0{1'b1}};
      abg7 = {WA*WB{1'bx}};  abg7 [WA  -1     ][WB  -1:WB/2] = {1   *WB/2+0{1'b1}};
      abg8 = {WA*WB{1'bx}};  abg8 [       0   ][       0   ] = {1   *1   +0{1'b1}};
      abg9 = {WA*WB{1'bx}};  abg9 [WA  -1     ][WB  -1     ] = {1   *1   +0{1'b1}};
      // check
      if (abg0 !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- L=R -- abg0 = 'b%b", abg0); err=1; end
      if (abg1 !== 16'b1111_1111_1111_1111) begin $display("FAILED -- L=R -- abg1 = 'b%b", abg1); err=1; end
      if (abg2 !== 16'bxxxx_xxxx_1111_1111) begin $display("FAILED -- L=R -- abg2 = 'b%b", abg2); err=1; end
      if (abg3 !== 16'b1111_1111_xxxx_xxxx) begin $display("FAILED -- L=R -- abg3 = 'b%b", abg3); err=1; end
      if (abg4 !== 16'bxxxx_xxxx_xxxx_1111) begin $display("FAILED -- L=R -- abg4 = 'b%b", abg4); err=1; end
      if (abg5 !== 16'b1111_xxxx_xxxx_xxxx) begin $display("FAILED -- L=R -- abg5 = 'b%b", abg5); err=1; end
      if (abg6 !== 16'bxxxx_xxxx_xxxx_xx11) begin $display("FAILED -- L=R -- abg6 = 'b%b", abg6); err=1; end
      if (abg7 !== 16'b11xx_xxxx_xxxx_xxxx) begin $display("FAILED -- L=R -- abg7 = 'b%b", abg7); err=1; end
      if (abg8 !== 16'bxxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- L=R -- abg8 = 'b%b", abg8); err=1; end
      if (abg9 !== 16'b1xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- L=R -- abg9 = 'b%b", abg9); err=1; end

      // test write to array LHS<RHS
      abg0 = {WA*WB{1'bx}};
      abg1 = {WA*WB{1'bx}};  abg1                            = {WA  *WB  +1{1'b1}};
      abg2 = {WA*WB{1'bx}};  abg2 [WA/2-1:0   ]              = {WA/2*WB  +1{1'b1}};
      abg3 = {WA*WB{1'bx}};  abg3 [WA  -1:WA/2]              = {WA/2*WB  +1{1'b1}};
      abg4 = {WA*WB{1'bx}};  abg4 [       0   ]              = {1   *WB  +1{1'b1}};
      abg5 = {WA*WB{1'bx}};  abg5 [WA  -1     ]              = {1   *WB  +1{1'b1}};
      abg6 = {WA*WB{1'bx}};  abg6 [       0   ][WB/2-1:0   ] = {1   *WB/2+1{1'b1}};
      abg7 = {WA*WB{1'bx}};  abg7 [WA  -1     ][WB  -1:WB/2] = {1   *WB/2+1{1'b1}};
      abg8 = {WA*WB{1'bx}};  abg8 [       0   ][       0   ] = {1   *1   +1{1'b1}};
      abg9 = {WA*WB{1'bx}};  abg9 [WA  -1     ][WB  -1     ] = {1   *1   +1{1'b1}};
      // check
      if (abg0 !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- L<R -- abg0 = 'b%b", abg0); err=1; end
      if (abg1 !== 16'b1111_1111_1111_1111) begin $display("FAILED -- L<R -- abg1 = 'b%b", abg1); err=1; end
      if (abg2 !== 16'bxxxx_xxxx_1111_1111) begin $display("FAILED -- L<R -- abg2 = 'b%b", abg2); err=1; end
      if (abg3 !== 16'b1111_1111_xxxx_xxxx) begin $display("FAILED -- L<R -- abg3 = 'b%b", abg3); err=1; end
      if (abg4 !== 16'bxxxx_xxxx_xxxx_1111) begin $display("FAILED -- L<R -- abg4 = 'b%b", abg4); err=1; end
      if (abg5 !== 16'b1111_xxxx_xxxx_xxxx) begin $display("FAILED -- L<R -- abg5 = 'b%b", abg5); err=1; end
      if (abg6 !== 16'bxxxx_xxxx_xxxx_xx11) begin $display("FAILED -- L<R -- abg6 = 'b%b", abg6); err=1; end
      if (abg7 !== 16'b11xx_xxxx_xxxx_xxxx) begin $display("FAILED -- L<R -- abg7 = 'b%b", abg7); err=1; end
      if (abg8 !== 16'bxxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- L<R -- abg8 = 'b%b", abg8); err=1; end
      if (abg9 !== 16'b1xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- L<R -- abg9 = 'b%b", abg9); err=1; end

      // test write to array LHS>RHS
      abg0 = {WA*WB{1'bx}};
      abg1 = {WA*WB{1'bx}};  abg1                            = {WA  *WB  -1{1'b1}};
      abg2 = {WA*WB{1'bx}};  abg2 [WA/2-1:0   ]              = {WA/2*WB  -1{1'b1}};
      abg3 = {WA*WB{1'bx}};  abg3 [WA  -1:WA/2]              = {WA/2*WB  -1{1'b1}};
      abg4 = {WA*WB{1'bx}};  abg4 [       0   ]              = {1   *WB  -1{1'b1}};
      abg5 = {WA*WB{1'bx}};  abg5 [WA  -1     ]              = {1   *WB  -1{1'b1}};
      abg6 = {WA*WB{1'bx}};  abg6 [       0   ][WB/2-1:0   ] = {1   *WB/2-1{1'b1}};
      abg7 = {WA*WB{1'bx}};  abg7 [WA  -1     ][WB  -1:WB/2] = {1   *WB/2-1{1'b1}};
    //abg8 = {WA*WB{1'bx}};  abg8 [       0   ][       0   ] = {1   *1   -1{1'b1}};
    //abg9 = {WA*WB{1'bx}};  abg9 [WA  -1     ][WB  -1     ] = {1   *1   -1{1'b1}};
      // check
      if (abg0 !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- L>R -- abg0 = 'b%b", abg0); err=1; end
      if (abg1 !== 16'b0111_1111_1111_1111) begin $display("FAILED -- L>R -- abg1 = 'b%b", abg1); err=1; end
      if (abg2 !== 16'bxxxx_xxxx_0111_1111) begin $display("FAILED -- L>R -- abg2 = 'b%b", abg2); err=1; end
      if (abg3 !== 16'b0111_1111_xxxx_xxxx) begin $display("FAILED -- L>R -- abg3 = 'b%b", abg3); err=1; end
      if (abg4 !== 16'bxxxx_xxxx_xxxx_0111) begin $display("FAILED -- L>R -- abg4 = 'b%b", abg4); err=1; end
      if (abg5 !== 16'b0111_xxxx_xxxx_xxxx) begin $display("FAILED -- L>R -- abg5 = 'b%b", abg5); err=1; end
      if (abg6 !== 16'bxxxx_xxxx_xxxx_xx01) begin $display("FAILED -- L>R -- abg6 = 'b%b", abg6); err=1; end
      if (abg7 !== 16'b01xx_xxxx_xxxx_xxxx) begin $display("FAILED -- L>R -- abg7 = 'b%b", abg7); err=1; end
    //if (abg8 !== 16'bxxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- L>R -- abg8 = 'b%b", abg8); err=1; end
    //if (abg9 !== 16'b1xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- L>R -- abg9 = 'b%b", abg9); err=1; end

      // test write to array LHS=RHS
      alt0 = {WA*WB{1'bx}};
      alt1 = {WA*WB{1'bx}};  alt1                            = {WA  *WB  +0{1'b1}};
      alt2 = {WA*WB{1'bx}};  alt2 [0   :WA/2-1]              = {WA/2*WB  +0{1'b1}};
      alt3 = {WA*WB{1'bx}};  alt3 [WA/2:WA  -1]              = {WA/2*WB  +0{1'b1}};
      alt4 = {WA*WB{1'bx}};  alt4 [0          ]              = {1   *WB  +0{1'b1}};
      alt5 = {WA*WB{1'bx}};  alt5 [     WA  -1]              = {1   *WB  +0{1'b1}};
      alt6 = {WA*WB{1'bx}};  alt6 [0          ][0   :WB/2-1] = {1   *WB/2+0{1'b1}};
      alt7 = {WA*WB{1'bx}};  alt7 [     WA  -1][WB/2:WB  -1] = {1   *WB/2+0{1'b1}};
      alt8 = {WA*WB{1'bx}};  alt8 [0          ][0          ] = {1   *1   +0{1'b1}};
      alt9 = {WA*WB{1'bx}};  alt9 [     WA  -1][     WB  -1] = {1   *1   +0{1'b1}};
      // check
      if (alt0 !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- L=R -- alt0 = 'b%b", alt0); err=1; end
      if (alt1 !== 16'b1111_1111_1111_1111) begin $display("FAILED -- L=R -- alt1 = 'b%b", alt1); err=1; end
      if (alt2 !== 16'b1111_1111_xxxx_xxxx) begin $display("FAILED -- L=R -- alt2 = 'b%b", alt2); err=1; end
      if (alt3 !== 16'bxxxx_xxxx_1111_1111) begin $display("FAILED -- L=R -- alt3 = 'b%b", alt3); err=1; end
      if (alt4 !== 16'b1111_xxxx_xxxx_xxxx) begin $display("FAILED -- L=R -- alt4 = 'b%b", alt4); err=1; end
      if (alt5 !== 16'bxxxx_xxxx_xxxx_1111) begin $display("FAILED -- L=R -- alt5 = 'b%b", alt5); err=1; end
      if (alt6 !== 16'b11xx_xxxx_xxxx_xxxx) begin $display("FAILED -- L=R -- alt6 = 'b%b", alt6); err=1; end
      if (alt7 !== 16'bxxxx_xxxx_xxxx_xx11) begin $display("FAILED -- L=R -- alt7 = 'b%b", alt7); err=1; end
      if (alt8 !== 16'b1xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- L=R -- alt8 = 'b%b", alt8); err=1; end
      if (alt9 !== 16'bxxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- L=R -- alt9 = 'b%b", alt9); err=1; end

      // test write to array LHS<RHS
      alt0 = {WA*WB{1'bx}};
      alt1 = {WA*WB{1'bx}};  alt1                            = {WA  *WB  +1{1'b1}};
      alt2 = {WA*WB{1'bx}};  alt2 [0   :WA/2-1]              = {WA/2*WB  +1{1'b1}};
      alt3 = {WA*WB{1'bx}};  alt3 [WA/2:WA  -1]              = {WA/2*WB  +1{1'b1}};
      alt4 = {WA*WB{1'bx}};  alt4 [0          ]              = {1   *WB  +1{1'b1}};
      alt5 = {WA*WB{1'bx}};  alt5 [     WA  -1]              = {1   *WB  +1{1'b1}};
      alt6 = {WA*WB{1'bx}};  alt6 [0          ][0   :WB/2-1] = {1   *WB/2+1{1'b1}};
      alt7 = {WA*WB{1'bx}};  alt7 [     WA  -1][WB/2:WB  -1] = {1   *WB/2+1{1'b1}};
      alt8 = {WA*WB{1'bx}};  alt8 [0          ][0          ] = {1   *1   +1{1'b1}};
      alt9 = {WA*WB{1'bx}};  alt9 [     WA  -1][     WB  -1] = {1   *1   +1{1'b1}};
      // check
      if (alt0 !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- L<R -- alt0 = 'b%b", alt0); err=1; end
      if (alt1 !== 16'b1111_1111_1111_1111) begin $display("FAILED -- L<R -- alt1 = 'b%b", alt1); err=1; end
      if (alt2 !== 16'b1111_1111_xxxx_xxxx) begin $display("FAILED -- L<R -- alt2 = 'b%b", alt2); err=1; end
      if (alt3 !== 16'bxxxx_xxxx_1111_1111) begin $display("FAILED -- L<R -- alt3 = 'b%b", alt3); err=1; end
      if (alt4 !== 16'b1111_xxxx_xxxx_xxxx) begin $display("FAILED -- L<R -- alt4 = 'b%b", alt4); err=1; end
      if (alt5 !== 16'bxxxx_xxxx_xxxx_1111) begin $display("FAILED -- L<R -- alt5 = 'b%b", alt5); err=1; end
      if (alt6 !== 16'b11xx_xxxx_xxxx_xxxx) begin $display("FAILED -- L<R -- alt6 = 'b%b", alt6); err=1; end
      if (alt7 !== 16'bxxxx_xxxx_xxxx_xx11) begin $display("FAILED -- L<R -- alt7 = 'b%b", alt7); err=1; end
      if (alt8 !== 16'b1xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- L<R -- alt8 = 'b%b", alt8); err=1; end
      if (alt9 !== 16'bxxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- L<R -- alt9 = 'b%b", alt9); err=1; end

      // test write to array LHS>RHS
      alt0 = {WA*WB{1'bx}};
      alt1 = {WA*WB{1'bx}};  alt1                            = {WA  *WB  -1{1'b1}};
      alt2 = {WA*WB{1'bx}};  alt2 [0   :WA/2-1]              = {WA/2*WB  -1{1'b1}};
      alt3 = {WA*WB{1'bx}};  alt3 [WA/2:WA  -1]              = {WA/2*WB  -1{1'b1}};
      alt4 = {WA*WB{1'bx}};  alt4 [0          ]              = {1   *WB  -1{1'b1}};
      alt5 = {WA*WB{1'bx}};  alt5 [     WA  -1]              = {1   *WB  -1{1'b1}};
      alt6 = {WA*WB{1'bx}};  alt6 [0          ][0   :WB/2-1] = {1   *WB/2-1{1'b1}};
      alt7 = {WA*WB{1'bx}};  alt7 [     WA  -1][WB/2:WB  -1] = {1   *WB/2-1{1'b1}};
    //alt8 = {WA*WB{1'bx}};  alt8 [0          ][0          ] = {1   *1   -1{1'b1}};
    //alt9 = {WA*WB{1'bx}};  alt9 [     WA  -1][     WB  -1] = {1   *1   -1{1'b1}};
      // check
      if (alt0 !== 16'bxxxx_xxxx_xxxx_xxxx) begin $display("FAILED -- L>R -- alt0 = 'b%b", alt0); err=1; end
      if (alt1 !== 16'b0111_1111_1111_1111) begin $display("FAILED -- L>R -- alt1 = 'b%b", alt1); err=1; end
      if (alt2 !== 16'b0111_1111_xxxx_xxxx) begin $display("FAILED -- L>R -- alt2 = 'b%b", alt2); err=1; end
      if (alt3 !== 16'bxxxx_xxxx_0111_1111) begin $display("FAILED -- L>R -- alt3 = 'b%b", alt3); err=1; end
      if (alt4 !== 16'b0111_xxxx_xxxx_xxxx) begin $display("FAILED -- L>R -- alt4 = 'b%b", alt4); err=1; end
      if (alt5 !== 16'bxxxx_xxxx_xxxx_0111) begin $display("FAILED -- L>R -- alt5 = 'b%b", alt5); err=1; end
      if (alt6 !== 16'b01xx_xxxx_xxxx_xxxx) begin $display("FAILED -- L>R -- alt6 = 'b%b", alt6); err=1; end
      if (alt7 !== 16'bxxxx_xxxx_xxxx_xx01) begin $display("FAILED -- L>R -- alt7 = 'b%b", alt7); err=1; end
    //if (alt8 !== 16'b1xxx_xxxx_xxxx_xxxx) begin $display("FAILED -- L>R -- alt8 = 'b%b", alt8); err=1; end
    //if (alt9 !== 16'bxxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- L>R -- alt9 = 'b%b", alt9); err=1; end

      // assign a constant value to the array
      abg1 = {WA*WB{1'b1}};
      abg2 = {WA*WB{1'b1}};
      abg3 = {WA*WB{1'b1}};
      abg4 = {WA*WB{1'b1}};
      abg5 = {WA*WB{1'b1}};
      abg6 = {WA*WB{1'b1}};
      abg7 = {WA*WB{1'b1}};
      abg8 = {WA*WB{1'b1}};
      abg9 = {WA*WB{1'b1}};

      // test read from array LHS=RHS
      a1d1 = {WA*WB+1{1'bx}};  a1d1[WA  *WB  -1+0:0] = abg1                           ;
      a1d2 = {WA*WB+1{1'bx}};  a1d2[WA/2*WB  -1+0:0] = abg2 [WA/2-1:0   ]             ;
      a1d3 = {WA*WB+1{1'bx}};  a1d3[WA/2*WB  -1+0:0] = abg3 [WA  -1:WA/2]             ;
      a1d4 = {WA*WB+1{1'bx}};  a1d4[1   *WB  -1+0:0] = abg4 [       0   ]             ;
      a1d5 = {WA*WB+1{1'bx}};  a1d5[1   *WB  -1+0:0] = abg5 [WA  -1     ]             ;
      a1d6 = {WA*WB+1{1'bx}};  a1d6[1   *WB/2-1+0:0] = abg6 [       0   ][WB/2-1:0   ];
      a1d7 = {WA*WB+1{1'bx}};  a1d7[1   *WB/2-1+0:0] = abg7 [WA  -1     ][WB  -1:WB/2];
      a1d8 = {WA*WB+1{1'bx}};  a1d8[1   *1   -1+0:0] = abg8 [       0   ][       0   ];
      a1d9 = {WA*WB+1{1'bx}};  a1d9[1   *1   -1+0:0] = abg9 [WA  -1     ][WB  -1     ];
      // check
      if (a1d1 !== 17'bx_1111_1111_1111_1111) begin $display("FAILED -- L=R BE -- a1d1 = 'b%b", a1d1); err=1; end
      if (a1d2 !== 17'bx_xxxx_xxxx_1111_1111) begin $display("FAILED -- L=R BE -- a1d2 = 'b%b", a1d2); err=1; end
      if (a1d3 !== 17'bx_xxxx_xxxx_1111_1111) begin $display("FAILED -- L=R BE -- a1d3 = 'b%b", a1d3); err=1; end
      if (a1d4 !== 17'bx_xxxx_xxxx_xxxx_1111) begin $display("FAILED -- L=R BE -- a1d4 = 'b%b", a1d4); err=1; end
      if (a1d5 !== 17'bx_xxxx_xxxx_xxxx_1111) begin $display("FAILED -- L=R BE -- a1d5 = 'b%b", a1d5); err=1; end
      if (a1d6 !== 17'bx_xxxx_xxxx_xxxx_xx11) begin $display("FAILED -- L=R BE -- a1d6 = 'b%b", a1d6); err=1; end
      if (a1d7 !== 17'bx_xxxx_xxxx_xxxx_xx11) begin $display("FAILED -- L=R BE -- a1d7 = 'b%b", a1d7); err=1; end
      if (a1d8 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- L=R BE -- a1d8 = 'b%b", a1d8); err=1; end
      if (a1d9 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- L=R BE -- a1d9 = 'b%b", a1d9); err=1; end

      // test read from array LHS>RHS
      a1d1 = {WA*WB+1{1'bx}};  a1d1[WA  *WB  -1+1:0] = abg1                           ;
      a1d2 = {WA*WB+1{1'bx}};  a1d2[WA/2*WB  -1+1:0] = abg2 [WA/2-1:0   ]             ;
      a1d3 = {WA*WB+1{1'bx}};  a1d3[WA/2*WB  -1+1:0] = abg3 [WA  -1:WA/2]             ;
      a1d4 = {WA*WB+1{1'bx}};  a1d4[1   *WB  -1+1:0] = abg4 [       0   ]             ;
      a1d5 = {WA*WB+1{1'bx}};  a1d5[1   *WB  -1+1:0] = abg5 [WA  -1     ]             ;
      a1d6 = {WA*WB+1{1'bx}};  a1d6[1   *WB/2-1+1:0] = abg6 [       0   ][WB/2-1:0   ];
      a1d7 = {WA*WB+1{1'bx}};  a1d7[1   *WB/2-1+1:0] = abg7 [WA  -1     ][WB  -1:WB/2];
      a1d8 = {WA*WB+1{1'bx}};  a1d8[1   *1   -1+1:0] = abg8 [       0   ][       0   ];
      a1d9 = {WA*WB+1{1'bx}};  a1d9[1   *1   -1+1:0] = abg9 [WA  -1     ][WB  -1     ];
      // check
      if (a1d1 !== 17'b0_1111_1111_1111_1111) begin $display("FAILED -- L>R BE -- a1d1 = 'b%b", a1d1); err=1; end
      if (a1d2 !== 17'bx_xxxx_xxx0_1111_1111) begin $display("FAILED -- L>R BE -- a1d2 = 'b%b", a1d2); err=1; end
      if (a1d3 !== 17'bx_xxxx_xxx0_1111_1111) begin $display("FAILED -- L>R BE -- a1d3 = 'b%b", a1d3); err=1; end
      if (a1d4 !== 17'bx_xxxx_xxxx_xxx0_1111) begin $display("FAILED -- L>R BE -- a1d4 = 'b%b", a1d4); err=1; end
      if (a1d5 !== 17'bx_xxxx_xxxx_xxx0_1111) begin $display("FAILED -- L>R BE -- a1d5 = 'b%b", a1d5); err=1; end
      if (a1d6 !== 17'bx_xxxx_xxxx_xxxx_x011) begin $display("FAILED -- L>R BE -- a1d6 = 'b%b", a1d6); err=1; end
      if (a1d7 !== 17'bx_xxxx_xxxx_xxxx_x011) begin $display("FAILED -- L>R BE -- a1d7 = 'b%b", a1d7); err=1; end
      if (a1d8 !== 17'bx_xxxx_xxxx_xxxx_xx01) begin $display("FAILED -- L>R BE -- a1d8 = 'b%b", a1d8); err=1; end
      if (a1d9 !== 17'bx_xxxx_xxxx_xxxx_xx01) begin $display("FAILED -- L>R BE -- a1d9 = 'b%b", a1d9); err=1; end

      // test read from array LHS<RHS
      a1d1 = {WA*WB+1{1'bx}};  a1d1[WA  *WB  -1-1:0] = abg1                           ;
      a1d2 = {WA*WB+1{1'bx}};  a1d2[WA/2*WB  -1-1:0] = abg2 [WA/2-1:0   ]             ;
      a1d3 = {WA*WB+1{1'bx}};  a1d3[WA/2*WB  -1-1:0] = abg3 [WA  -1:WA/2]             ;
      a1d4 = {WA*WB+1{1'bx}};  a1d4[1   *WB  -1-1:0] = abg4 [       0   ]             ;
      a1d5 = {WA*WB+1{1'bx}};  a1d5[1   *WB  -1-1:0] = abg5 [WA  -1     ]             ;
      a1d6 = {WA*WB+1{1'bx}};  a1d6[1   *WB/2-1-1:0] = abg6 [       0   ][WB/2-1:0   ];
      a1d7 = {WA*WB+1{1'bx}};  a1d7[1   *WB/2-1-1:0] = abg7 [WA  -1     ][WB  -1:WB/2];
    //a1d8 = {WA*WB+1{1'bx}};  a1d8[1   *1   -1-1:0] = abg8 [       0   ][       0   ];
    //a1d9 = {WA*WB+1{1'bx}};  a1d9[1   *1   -1-1:0] = abg9 [WA  -1     ][WB  -1     ];
      // check
      if (a1d1 !== 17'bx_x111_1111_1111_1111) begin $display("FAILED -- L<R BE -- a1d1 = 'b%b", a1d1); err=1; end
      if (a1d2 !== 17'bx_xxxx_xxxx_x111_1111) begin $display("FAILED -- L<R BE -- a1d2 = 'b%b", a1d2); err=1; end
      if (a1d3 !== 17'bx_xxxx_xxxx_x111_1111) begin $display("FAILED -- L<R BE -- a1d3 = 'b%b", a1d3); err=1; end
      if (a1d4 !== 17'bx_xxxx_xxxx_xxxx_x111) begin $display("FAILED -- L<R BE -- a1d4 = 'b%b", a1d4); err=1; end
      if (a1d5 !== 17'bx_xxxx_xxxx_xxxx_x111) begin $display("FAILED -- L<R BE -- a1d5 = 'b%b", a1d5); err=1; end
      if (a1d6 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- L<R BE -- a1d6 = 'b%b", a1d6); err=1; end
      if (a1d7 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- L<R BE -- a1d7 = 'b%b", a1d7); err=1; end
    //if (a1d8 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- L<R BE -- a1d8 = 'b%b", a1d8); err=1; end
    //if (a1d9 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- L<R BE -- a1d9 = 'b%b", a1d9); err=1; end

      // assign a constant value to the array
      alt1 = {WA*WB{1'b1}};
      alt2 = {WA*WB{1'b1}};
      alt3 = {WA*WB{1'b1}};
      alt4 = {WA*WB{1'b1}};
      alt5 = {WA*WB{1'b1}};
      alt6 = {WA*WB{1'b1}};
      alt7 = {WA*WB{1'b1}};
      alt8 = {WA*WB{1'b1}};
      alt9 = {WA*WB{1'b1}};

      // test read from array LHS=RHS
      a1d1 = {WA*WB+1{1'bx}};  a1d1[WA  *WB  -1+0:0] = alt1                           ;
      a1d2 = {WA*WB+1{1'bx}};  a1d2[WA/2*WB  -1+0:0] = alt2 [0   :WA/2-1]             ;
      a1d3 = {WA*WB+1{1'bx}};  a1d3[WA/2*WB  -1+0:0] = alt3 [WA/2:WA  -1]             ;
      a1d4 = {WA*WB+1{1'bx}};  a1d4[1   *WB  -1+0:0] = alt4 [0          ]             ;
      a1d5 = {WA*WB+1{1'bx}};  a1d5[1   *WB  -1+0:0] = alt5 [     WA  -1]             ;
      a1d6 = {WA*WB+1{1'bx}};  a1d6[1   *WB/2-1+0:0] = alt6 [0          ][0   :WB/2-1];
      a1d7 = {WA*WB+1{1'bx}};  a1d7[1   *WB/2-1+0:0] = alt7 [     WA  -1][WB/2:WB  -1];
      a1d8 = {WA*WB+1{1'bx}};  a1d8[1   *1   -1+0:0] = alt8 [0          ][0          ];
      a1d9 = {WA*WB+1{1'bx}};  a1d9[1   *1   -1+0:0] = alt9 [     WA  -1][     WB  -1];
      // check
      if (a1d1 !== 17'bx_1111_1111_1111_1111) begin $display("FAILED -- L=R LE -- a1d1 = 'b%b", a1d1); err=1; end
      if (a1d2 !== 17'bx_xxxx_xxxx_1111_1111) begin $display("FAILED -- L=R LE -- a1d2 = 'b%b", a1d2); err=1; end
      if (a1d3 !== 17'bx_xxxx_xxxx_1111_1111) begin $display("FAILED -- L=R LE -- a1d3 = 'b%b", a1d3); err=1; end
      if (a1d4 !== 17'bx_xxxx_xxxx_xxxx_1111) begin $display("FAILED -- L=R LE -- a1d4 = 'b%b", a1d4); err=1; end
      if (a1d5 !== 17'bx_xxxx_xxxx_xxxx_1111) begin $display("FAILED -- L=R LE -- a1d5 = 'b%b", a1d5); err=1; end
      if (a1d6 !== 17'bx_xxxx_xxxx_xxxx_xx11) begin $display("FAILED -- L=R LE -- a1d6 = 'b%b", a1d6); err=1; end
      if (a1d7 !== 17'bx_xxxx_xxxx_xxxx_xx11) begin $display("FAILED -- L=R LE -- a1d7 = 'b%b", a1d7); err=1; end
      if (a1d8 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- L=R LE -- a1d8 = 'b%b", a1d8); err=1; end
      if (a1d9 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- L=R LE -- a1d9 = 'b%b", a1d9); err=1; end

      // test read from array LHS>RHS
      a1d1 = {WA*WB+1{1'bx}};  a1d1[WA  *WB  -1+1:0] = alt1                           ;
      a1d2 = {WA*WB+1{1'bx}};  a1d2[WA/2*WB  -1+1:0] = alt2 [0   :WA/2-1]             ;
      a1d3 = {WA*WB+1{1'bx}};  a1d3[WA/2*WB  -1+1:0] = alt3 [WA/2:WA  -1]             ;
      a1d4 = {WA*WB+1{1'bx}};  a1d4[1   *WB  -1+1:0] = alt4 [0          ]             ;
      a1d5 = {WA*WB+1{1'bx}};  a1d5[1   *WB  -1+1:0] = alt5 [     WA  -1]             ;
      a1d6 = {WA*WB+1{1'bx}};  a1d6[1   *WB/2-1+1:0] = alt6 [0          ][0   :WB/2-1];
      a1d7 = {WA*WB+1{1'bx}};  a1d7[1   *WB/2-1+1:0] = alt7 [     WA  -1][WB/2:WB  -1];
      a1d8 = {WA*WB+1{1'bx}};  a1d8[1   *1   -1+1:0] = alt8 [0          ][0          ];
      a1d9 = {WA*WB+1{1'bx}};  a1d9[1   *1   -1+1:0] = alt9 [     WA  -1][     WB  -1];
      // check
      if (a1d1 !== 17'b0_1111_1111_1111_1111) begin $display("FAILED -- L>R LE -- a1d1 = 'b%b", a1d1); err=1; end
      if (a1d2 !== 17'bx_xxxx_xxx0_1111_1111) begin $display("FAILED -- L>R LE -- a1d2 = 'b%b", a1d2); err=1; end
      if (a1d3 !== 17'bx_xxxx_xxx0_1111_1111) begin $display("FAILED -- L>R LE -- a1d3 = 'b%b", a1d3); err=1; end
      if (a1d4 !== 17'bx_xxxx_xxxx_xxx0_1111) begin $display("FAILED -- L>R LE -- a1d4 = 'b%b", a1d4); err=1; end
      if (a1d5 !== 17'bx_xxxx_xxxx_xxx0_1111) begin $display("FAILED -- L>R LE -- a1d5 = 'b%b", a1d5); err=1; end
      if (a1d6 !== 17'bx_xxxx_xxxx_xxxx_x011) begin $display("FAILED -- L>R LE -- a1d6 = 'b%b", a1d6); err=1; end
      if (a1d7 !== 17'bx_xxxx_xxxx_xxxx_x011) begin $display("FAILED -- L>R LE -- a1d7 = 'b%b", a1d7); err=1; end
      if (a1d8 !== 17'bx_xxxx_xxxx_xxxx_xx01) begin $display("FAILED -- L>R LE -- a1d8 = 'b%b", a1d8); err=1; end
      if (a1d9 !== 17'bx_xxxx_xxxx_xxxx_xx01) begin $display("FAILED -- L>R LE -- a1d9 = 'b%b", a1d9); err=1; end

      // test read from array LHS<RHS
      a1d1 = {WA*WB+1{1'bx}};  a1d1[WA  *WB  -1-1:0] = alt1                           ;
      a1d2 = {WA*WB+1{1'bx}};  a1d2[WA/2*WB  -1-1:0] = alt2 [0   :WA/2-1]             ;
      a1d3 = {WA*WB+1{1'bx}};  a1d3[WA/2*WB  -1-1:0] = alt3 [WA/2:WA  -1]             ;
      a1d4 = {WA*WB+1{1'bx}};  a1d4[1   *WB  -1-1:0] = alt4 [0          ]             ;
      a1d5 = {WA*WB+1{1'bx}};  a1d5[1   *WB  -1-1:0] = alt5 [     WA  -1]             ;
      a1d6 = {WA*WB+1{1'bx}};  a1d6[1   *WB/2-1-1:0] = alt6 [0          ][0   :WB/2-1];
      a1d7 = {WA*WB+1{1'bx}};  a1d7[1   *WB/2-1-1:0] = alt7 [     WA  -1][WB/2:WB  -1];
    //a1d8 = {WA*WB+1{1'bx}};  a1d8[1   *1   -1-1:0] = alt8 [0          ][0          ];
    //a1d9 = {WA*WB+1{1'bx}};  a1d9[1   *1   -1-1:0] = alt9 [     WA  -1][     WB  -1];
      // check
      if (a1d1 !== 17'bx_x111_1111_1111_1111) begin $display("FAILED -- L<R LE -- a1d1 = 'b%b", a1d1); err=1; end
      if (a1d2 !== 17'bx_xxxx_xxxx_x111_1111) begin $display("FAILED -- L<R LE -- a1d2 = 'b%b", a1d2); err=1; end
      if (a1d3 !== 17'bx_xxxx_xxxx_x111_1111) begin $display("FAILED -- L<R LE -- a1d3 = 'b%b", a1d3); err=1; end
      if (a1d4 !== 17'bx_xxxx_xxxx_xxxx_x111) begin $display("FAILED -- L<R LE -- a1d4 = 'b%b", a1d4); err=1; end
      if (a1d5 !== 17'bx_xxxx_xxxx_xxxx_x111) begin $display("FAILED -- L<R LE -- a1d5 = 'b%b", a1d5); err=1; end
      if (a1d6 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- L<R LE -- a1d6 = 'b%b", a1d6); err=1; end
      if (a1d7 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- L<R LE -- a1d7 = 'b%b", a1d7); err=1; end
    //if (a1d8 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- L<R LE -- a1d8 = 'b%b", a1d8); err=1; end
    //if (a1d9 !== 17'bx_xxxx_xxxx_xxxx_xxx1) begin $display("FAILED -- L<R LE -- a1d9 = 'b%b", a1d9); err=1; end

      if (!err) $display("PASSED");
   end

endmodule // test
