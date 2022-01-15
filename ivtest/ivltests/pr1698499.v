module sysSimpleTest();
  reg CLK;
  reg RST_N;

   initial begin
      #0
      RST_N = 1'b0;
      #1;
      CLK = 1'b1;
      $display("reset");
      #1;
      RST_N = 1'b1;
      $display("reset done");
   end

   always
     begin
        #5;
        CLK = 1'b0 ;
        #5;
        CLK = 1'b1 ;
   end


  // register y
  reg [98 : 0] y;
  wire [98 : 0] y$D_IN;
  wire y$EN;

  // register z
  reg [98 : 0] z;
  wire [98 : 0] z$D_IN;
  wire z$EN;

  // remaining internal signals
  wire [98 : 0] IF_y_SLT_0_AND_NOT_z_SLT_0_OR_NOT_y_SLT_0_AND__ETC___d29,
		IF_y_SLT_0_THEN_NEG_IF_y_SLT_0_THEN_NEG_y_0_EL_ETC___d28,
		IF_y_SLT_0_THEN_NEG_y_0_ELSE_y_1_QUOT_IF_z_SLT_ETC___d30,
		IF_y_SLT_0_THEN_NEG_y_0_ELSE_y_1_REM_IF_z_SLT__ETC___d31,
		x__h201,
		y__h141,
		z_MUL_IF_y_SLT_0_AND_NOT_z_SLT_0_OR_NOT_y_SLT__ETC___d17;
  wire y_SLT_0___d33,
       z_MUL_IF_y_SLT_0_AND_NOT_z_SLT_0_OR_NOT_y_SLT__ETC___d22,
       z_SLT_0___d32;

  // register y
  assign y$D_IN = 99'h0 ;
  assign y$EN = 1'b0 ;

  // register z
  assign z$EN = 1'b0 ;
  assign z$D_IN = 99'h0 ;

  // remaining internal signals
  assign IF_y_SLT_0_AND_NOT_z_SLT_0_OR_NOT_y_SLT_0_AND__ETC___d29 =
	     (y_SLT_0___d33 && !z_SLT_0___d32 ||
	      !y_SLT_0___d33 && z_SLT_0___d32) ?
	       -IF_y_SLT_0_THEN_NEG_y_0_ELSE_y_1_QUOT_IF_z_SLT_ETC___d30 :
	       IF_y_SLT_0_THEN_NEG_y_0_ELSE_y_1_QUOT_IF_z_SLT_ETC___d30 ;
  assign IF_y_SLT_0_THEN_NEG_IF_y_SLT_0_THEN_NEG_y_0_EL_ETC___d28 =
	     y_SLT_0___d33 ?
	       -IF_y_SLT_0_THEN_NEG_y_0_ELSE_y_1_REM_IF_z_SLT__ETC___d31 :
	       IF_y_SLT_0_THEN_NEG_y_0_ELSE_y_1_REM_IF_z_SLT__ETC___d31 ;
  assign IF_y_SLT_0_THEN_NEG_y_0_ELSE_y_1_QUOT_IF_z_SLT_ETC___d30 =
	     x__h201 / y__h141 ;
  assign IF_y_SLT_0_THEN_NEG_y_0_ELSE_y_1_REM_IF_z_SLT__ETC___d31 =
	     x__h201 % y__h141 ;
  assign x__h201 = y_SLT_0___d33 ? -y : y ;
  assign y_SLT_0___d33 =
	     (y ^ 99'h4000000000000000000000000) <
	     99'h4000000000000000000000000 ;
  assign y__h141 = z_SLT_0___d32 ? -z : z ;
  assign z_MUL_IF_y_SLT_0_AND_NOT_z_SLT_0_OR_NOT_y_SLT__ETC___d17 =
	     z * IF_y_SLT_0_AND_NOT_z_SLT_0_OR_NOT_y_SLT_0_AND__ETC___d29 ;
  assign z_MUL_IF_y_SLT_0_AND_NOT_z_SLT_0_OR_NOT_y_SLT__ETC___d22 =
	     z_MUL_IF_y_SLT_0_AND_NOT_z_SLT_0_OR_NOT_y_SLT__ETC___d17 +
	     IF_y_SLT_0_THEN_NEG_IF_y_SLT_0_THEN_NEG_y_0_EL_ETC___d28 ==
	     y ;
  assign z_SLT_0___d32 =
	     (z ^ 99'h4000000000000000000000000) <
	     99'h4000000000000000000000000 ;

  // handling of inlined registers

  always@(posedge CLK)
  begin
    if (!RST_N)
      begin
        // y <= 1;
        // z <= 1;
        y <= 99'h7FFFFFF04A62A1453402211B2;
	z <= 99'h000000023E84321AAFCCC70C2;
      end
    else
      begin
        if (y$EN) y <= y$D_IN;
	if (z$EN) z <= z$D_IN;
      end
  end

  // synopsys translate_off
  initial
  begin
    y = 99'h2AAAAAAAAAAAAAAAAAAAAAAAA;
    z = 99'h2AAAAAAAAAAAAAAAAAAAAAAAA;
  end
  // synopsys translate_on

  // handling of system tasks

  // synopsys translate_off
  always@(negedge CLK)
  begin
    #0;
    if (RST_N)
      if (z_MUL_IF_y_SLT_0_AND_NOT_z_SLT_0_OR_NOT_y_SLT__ETC___d22)
	$display("OK:  %0d * %0d + %0d == %0d",
		 $signed(z),
		 $signed(IF_y_SLT_0_AND_NOT_z_SLT_0_OR_NOT_y_SLT_0_AND__ETC___d29),
		 $signed(IF_y_SLT_0_THEN_NEG_IF_y_SLT_0_THEN_NEG_y_0_EL_ETC___d28),
		 $signed(y));
    if (RST_N)
      if (!z_MUL_IF_y_SLT_0_AND_NOT_z_SLT_0_OR_NOT_y_SLT__ETC___d22)
	$display("BAD: %0d * %0d + %0d != %0d",
		 $signed(z),
		 $signed(IF_y_SLT_0_AND_NOT_z_SLT_0_OR_NOT_y_SLT_0_AND__ETC___d29),
		 $signed(IF_y_SLT_0_THEN_NEG_IF_y_SLT_0_THEN_NEG_y_0_EL_ETC___d28),
		 $signed(y));
    if (RST_N) $finish(32'd0);
  end
  // synopsys translate_on
endmodule  // sysSimpleTest
