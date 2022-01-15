// Copyright C(O) 2004 Burnell G West
// The following text may be utilized and / or reproduced by anybody for
// any reason.
//
//  verr.v
//

module verr (clk, vout);

input        clk;
output       vout;

reg          vout;
real         start_edge;
real         end_edge;

wire         trigger_en;
wire [9:0]   v_value;

initial vout = 1'b0;

always @( posedge clk)
   begin
     if (trigger_en)
        begin
          start_edge = ( v_value[0] * 1.95)
          + ( v_value[1] * 3.9 )
          + ( v_value[2] * 7.8 )
          + ( v_value[3] * 15.6 )
          + ( v_value[4] * 31.2 )
          + ( v_value[5] * 62.5 )
          + ( v_value[6] * 125 )
          + ( v_value[7] * 250 )
          + ( v_value[8] * 0 )
          + ( v_value[9] * 0 )
          + 0;
          end_edge = start_edge + 100;  // make pulse width = 1ns
        end
     else
       begin
         start_edge <= start_edge;
         end_edge   <= end_edge;
       end
   end

endmodule

module vtest;

wire vout0, vout1, vout2, vout3, vout4, vout5, vout6, vout7, vout8, vout9;
wire vout10, vout11, vout12, vout13, vout14, vout15, vout16, vout17,
vout18, vout19;

reg  clk, bit0;

verr v0     (clk, vout0);
verr v1     (clk, vout1);
verr v2     (clk, vout2);
verr v3     (clk, vout3);
verr v4     (clk, vout4);
verr v5     (clk, vout5);
verr v6     (clk, vout6);
verr v7     (clk, vout7);
verr v8     (clk, vout8);
verr v9     (clk, vout9);
verr v10     (clk, vout10);
verr v11     (clk, vout11);
verr v12     (clk, vout12);
verr v13     (clk, vout13);
verr v14     (clk, vout14);
verr v15     (clk, vout15);
verr v16     (clk, vout16);
verr v17     (clk, vout17);
verr v18     (clk, vout18);
verr v19     (clk, vout19);

initial begin
   #10000 $display("This test doesn't check itself.");
   $display("PASSED");
end

endmodule
