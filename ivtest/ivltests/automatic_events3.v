module automatic_events3();

reg [1:0] Source;

initial begin
  Source[0] = 1'b0;
  forever begin
    #20 Source[0] = 1'b1;
    #20 Source[0] = 1'b0;
  end
end

initial begin
  Source[1] = 1'b0;
  #15;
  forever begin
    #10 Source[1] = 1'bx;
    #10 Source[1] = 1'b1;
    #10 Source[1] = 1'bx;
    #10 Source[1] = 1'b0;
  end
end

task automatic ReportPosEdge0;

begin
  @(posedge Source[0]);
  $display("Time %t : Source[0] rise", $time);
end

endtask

task automatic ReportNegEdge0;

begin
  @(negedge Source[0]);
  $display("Time %t : Source[0] fall", $time);
end

endtask

task automatic ReportAnyEdge0;

time t;

begin
  @(Source[0]) t = $time;
  #1 $display("Time %t : Source[0] edge", t);
end

endtask

task automatic ReportPosEdge1;

begin
  @(posedge Source[1]);
  $display("Time %t : Source[1] rise", $time);
end

endtask

task automatic ReportNegEdge1;

begin
  @(negedge Source[1]);
  $display("Time %t : Source[1] fall", $time);
end

endtask

task automatic ReportAnyEdge1;

time t;

begin
  @(Source[1]) t = $time;
  #1 $display("Time %t : Source[1] edge", t);
end

endtask

initial begin
  #1;
  fork
    repeat(2) ReportPosEdge0;
    repeat(2) ReportNegEdge0;
    repeat(4) ReportAnyEdge0;

    repeat(4) ReportPosEdge1;
    repeat(4) ReportNegEdge1;
    repeat(8) ReportAnyEdge1;
  join
  $finish(0);
end

endmodule
