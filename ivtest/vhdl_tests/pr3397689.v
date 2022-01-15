module top();
  reg p_clk, rst_in, reg_req_t;
  wire out;

  weird_ff uut(p_clk, rst_in, reg_req_t, out);

  initial begin
    p_clk = 0;
    rst_in = 1;
    reg_req_t = 0;
    #1;
    rst_in = 0;
    #1;
    p_clk = 1;
    #2;
    p_clk = 0;
    $display("%d", out);
    if (out != 1'bx) begin
      $display("FAILED 1 - ff was reset");
      $finish;
    end
    #1;
    rst_in = 1;
    #1;
    p_clk = 1;
    #1;
    p_clk = 0;
    $display("%d", out);
    if (out != 1'b0) begin
      $display("FAILED 2 - ff was not reset");
      $finish;
    end
    $display("PASSED");
  end

endmodule // top

module weird_ff(p_clk, rst_in, reg_req_t, out);
  input p_clk;
  input rst_in;
  input reg_req_t;
  output out;

  reg [1:0] wr_req_pipe;

  parameter G_ASYNC_RESET = 0;

  wire a_rst = (G_ASYNC_RESET != 0) ? rst_in : 1'b0;
  wire s_rst = (G_ASYNC_RESET == 0) ? rst_in : 1'b0;

  always @(posedge p_clk or posedge a_rst)
    if (a_rst | s_rst)
      wr_req_pipe <= 'b0;
    else
      wr_req_pipe <= {wr_req_pipe, reg_req_t};

  assign out = wr_req_pipe[1];

endmodule // weird_ff
