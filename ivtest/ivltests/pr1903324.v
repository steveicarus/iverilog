module top;
  reg pass = 1'b1;

  reg [7:0] d_reg = 8'b10100101;
  wire [7:0] d_wire = 8'b01011010;

  test tstr(d_reg);
  test tstw(d_wire);

  initial begin
    #1;
    /* Check with a register. */
    if (tstr.data_in_array[3] != d_reg)  begin
      $display("FAILED: with a register value.");
      pass = 1'b0;
    end

    /* Check with a wire. */
    if (tstw.data_in_array[3] != d_wire)  begin
      $display("FAILED: with a net value.");
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule

module test(input [8:1] data_in) ;

  wire [7:0] data_in_array[4:3];

  assign data_in_array[3] = data_in;
  assign data_in_array[4] = 8'b0;
endmodule
