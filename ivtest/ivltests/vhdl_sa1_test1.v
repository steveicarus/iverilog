// This module generates M pairs of N-1 bits unsigned numbers A, B
// and also serialises them starting from LSB bits between
// activation of active-high reset signal

module stimulus #(parameter N = 4, M = 10) (input clk,
                 output reg reset,
                 output reg sa, sb,
                 output reg unsigned [N-1:0] A, B );

parameter D = 5;
int unsigned i;
reg unsigned [N-1:0] r1, r2;

initial begin
  repeat(M) begin
    r1 = {$random} % N;
    r2 = {$random} % N;
    do_items(r1, r2);
  end
end

task do_items (input unsigned [N-1:0] v1, v2);
begin
    A = 0; B = 0; reset = 0;
    do_reset();
    A = v1; B = v2;
    for (i=0; i<N; i=i+1) begin
       #2 sa = A[i];
          sb = B[i];
       @(posedge clk);
    end
    #2 sa = 1'b0;
       sb = 1'b0;
       @(posedge clk);
end
endtask

task do_reset;
begin
  @(posedge clk);
  @(negedge clk) reset = 1'b1;
  repeat(D) @(negedge clk);
  reset = 1'b0;
end
endtask

endmodule

// This module takes M pairs of N-1 bits unsigned numbers A, B and a serial stream ss
// then it checks that following a negedge of a reset, after N positive clock cycles
// the reconstuction of N ss bits is equal to A+B.

module check #(parameter N = 4, M = 10)(input clk, reset, input unsigned [N-1:0] A, B, input ss);

reg unsigned [N:0] psum;
reg unsigned [N:0] ssum;
int unsigned i;


initial begin
  repeat(M)
    check_item();
end

task check_item;
begin
  @(posedge reset);
  @(negedge reset);
  #2;
  psum = A + B;
  for (i=0; i<=N; i=i+1) begin
    @(posedge clk);
    #1;
    ssum[i] = ss;
  end
  if (psum != ssum) begin
     $display("ERROR");
     $finish;
  end
end
endtask

endmodule


module test;
  parameter N = 8, M = 25;
  parameter  T = 10;
  parameter  S = (N+1+6)*M*T + 100; // 6 is duration of a reset
  reg reset, clk;
  wire sa, sb, ss;
  wire [N-1:0] A, B;

  initial begin
    clk = 0;
    forever #(T/2) clk = ~clk;
  end

  stimulus #(N, M) stim  (.clk(clk), .reset(reset), .sa(sa), .sb(sb), .A(A), .B(B));
  sa1           duv   (.clk(clk), .reset(reset), .a_i(sa), .b_i(sb), .s_o(ss) );
  check    #(N, M) check (.clk(clk), .reset(reset), .A(A), .B(B), .ss(ss));

  initial begin
    #S;
    $display("PASSED");
    $finish;
  end

endmodule
