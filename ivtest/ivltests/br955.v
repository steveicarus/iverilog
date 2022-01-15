`timescale 1 ps / 1 ps

module mux_2_to_1
  (
    input sel_i,
    input [1:0] dat_i,
    output dat_o
  );

  assign dat_o = sel_i && dat_i[1] || ~sel_i && dat_i[0];

endmodule

module mux_n_to_1
  #(
    parameter sel_w = 4,
    parameter n_inputs = 2**sel_w
  )
  (
    input [n_inputs-1:0] inputs_i,
    input [sel_w-1:0] sel_i,
    output output_o
  );

  genvar i,j;
  generate
  if(sel_w == 1) begin
    mux_2_to_1 mux_simple
      (
        .sel_i(sel_i),
        .dat_i(inputs_i),
        .dat_o(output_o)
      );
  end else begin
    wire [n_inputs-2:0] inter_w;

    for(i=0; i<sel_w; i=i+1) begin : SELECT_STAGE
      if(i==0) begin
        for(j=0; j<n_inputs/2; j=j+1) begin : FIRST_STAGE
          mux_2_to_1 mux_first_stage
            (
              .sel_i(sel_i[i]),
              .dat_i(inputs_i[2*j+1:2*j]),
              .dat_o(inter_w[j])
            );
        end
      end else begin
        for(j=0; j<(n_inputs/(2**(i+1))); j=j+1) begin : INTERMEDIARY_STAGE
          mux_2_to_1 mux_intermediary_stages
            (
              .sel_i(sel_i[i]),
              .dat_i(inter_w[ (n_inputs/(2**(i-1)))*((2**(i-1))-1) + 2*j + 1 : (n_inputs/(2**(i-1)))*((2**(i-1))-1) + 2*j ]),
              .dat_o(inter_w[ (n_inputs/(2**i))*((2**i)-1) + j ])
            );
        end
      end
    end
    assign output_o = inter_w[ (n_inputs/(2**(sel_w-1)))*((2**(sel_w-1))-1) ];
  end
  endgenerate
endmodule

module top;
  parameter sel_wid = 3;
  parameter num_inputs = 2**sel_wid;

  reg [num_inputs-1:0] in;
  reg [sel_wid-1:0] sel;
  wire out;
  integer lp;
  reg pass;

  mux_n_to_1
    #(
      .sel_w(sel_wid),
      .n_inputs(num_inputs)
     )
     dut
     (
       .inputs_i(in),
       .sel_i(sel),
       .output_o(out)
     );

  initial begin
    pass = 1'b1;

    for (lp = 0; lp < num_inputs; lp = lp + 1) begin
      sel = lp;
      in = 2**lp;
      $display("Checking input %0d;", sel);
      #1;
      if (out !== 1'b1) begin
        $display("  Failed input high (%b), got %b", in, out);
        pass = 1'b0;
      end
      in = ~in;
      #1;
      if (out !== 1'b0) begin
        $display("  Failed input low (%b), got %b", in, out);
        pass = 1'b0;
      end
    end

    if (pass) $display("PASSED");
  end
endmodule
