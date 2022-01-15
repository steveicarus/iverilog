module kk_timing (A, B, C, D, E, F);

 input A, B, D, E, F;
 output C;
 wire A, B, D, E, F;
 reg C;
 wire [1:0] BL;
 wire [1:0] BL_X;

 assign BL[0] = E;
 assign BL_X[0] = F;
 wire BL_0 = BL[0] ;
 wire BL_X_0 = BL_X[0];

 specify
 $setuphold(posedge A &&& B, BL[0], 0, 0, C,,,D, BL_X[0]); // line 14 compile fail iverilog_20060618
 $setuphold(posedge A &&& B, BL_0 , 0, 0, C,,,D, BL_X[0]); // line 15 compile fail iverilog_20060618
 $setuphold(posedge A &&& B, BL[0], 0, 0, C,,,D, BL_X_0 ); // line 16 compile pass iverilog_20060618
 $setuphold(posedge A &&& B, BL_0 , 0, 0, C,,,D, BL_X_0 ); // line 17 compile pass iverilog_20060618
 endspecify


 endmodule
