// This module generate M single 2*HW-1 bit vector each T time steps

module stimulus #(parameter W = 8, M = 200, MAX = 256) (
                  input bit clk, reset,
                 output reg [W-1:0] x
                 );

int i;

initial begin
  @(negedge reset);
  for (i = 0; i < M; i=i+1) begin
    @(negedge clk);
    x = {$random} % MAX;
  end

end


endmodule

module test;
  parameter M = 200;  // number of test vectors
  parameter W = 8;   // bit width of input vecotrs
  parameter T = 10;  // for timing
  parameter D = 8;  // depth of pipeline, MAX of 8
  parameter K = 10; // distance between boundaries of pipeline
  parameter S = 2*M*T + 12*D;
  parameter MAX = D*K;

  bit clk =0, reset = 0;

  wire [W-1:0] xin;
  wire [W-1:0] din = K;

  wire [W-1:0] dout, bout, xout;

  wire [2:0] lin = 3'b111;  // -1 in fact
  wire [2:0] lout;

  int x_gold; // for computing expected result

  initial forever #T clk = ~clk;



  stimulus   #(W, M, MAX)   stim  (.clk(clk), .reset(reset), .x(xin));
  diq_array  #(W, D)         duv   (.clk(clk), .reset(reset),
                                    .din(din), .bin(8'd0), .xin(xin), .lin(lin),
                                    .dout(dout), .bout(bout), .xout(xout),
                                   .lout(lout) );


initial begin: checking
  @(negedge reset);
  @(posedge clk);
  repeat (D) @(negedge clk);
  forever begin
    @(posedge clk);
    #1;
    // checking dout
    if (dout !== din) begin
      $display("ERROR");
      $finish;
    end
    // checking bout
    if (bout !== MAX) begin
      $display("ERROR");
      $finish;
    end
    // checking lout
    x_gold = xout-1;   // dirty fix, for example xin = 30 muste be reported as 2
    if (lout !== x_gold/K) begin
      $display("ERROR");
      $finish;
    end

  end
end

  initial begin
    doreset();
    #S;
    $display("PASSED");
    $finish;
  end

  task doreset;
     begin
	@(negedge clk);
	reset = 1;
	repeat (5) @(negedge clk);
	reset = 0;
     end
  endtask

endmodule
