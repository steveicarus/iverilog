module top();
  reg pass = 1'b1;
  reg [31:0] in = 'bx;
  reg signed [31:0] sin = 'bx;
  wire [63:0] res, sres;

  lower lwr(res, in);
  slower slwr(sres, sin);

  initial begin
    #1;
    if (res !== {32'b0, 32'bx}) begin
      $display("FAILED: unsigned output (%b)", res);
      pass = 1'b0;
    end

    if (lwr.lout !== {32'b0, 32'bx}) begin
      $display("FAILED: unsigned input (%b)", lwr.lout);
      pass = 1'b0;
    end

    if (sres !== 64'bx) begin
      $display("FAILED: signed output (%b)", sres);
      pass = 1'b0;
    end

    if (slwr.lout !== 64'bx) begin
      $display("FAILED: signed input (%b)", slwr.lout);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule

module lower(lrtn, lin);
  output [31:0] lrtn;
  input [63:0] lin;

  wire [63:0] lout = lin;

  assign lrtn = lout[31:0];
endmodule

module slower(lrtn, lin);
  output signed [31:0] lrtn;
  input [63:0] lin;

  wire [63:0] lout = lin;

  assign lrtn = lout[31:0];
endmodule
