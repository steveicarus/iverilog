module automatic_events();

reg [5:1] any;

integer k;

initial begin
  any = 5'b00000;
  #200;
  for (k = 1; k <= 5; k = k + 1) begin
    #10 any[k] = 1;
    #10 any[k] = 0;
  end
end

task automatic report_events;

input integer n;

begin:task_body
  reg [5:1] pos;
  reg [5:1] neg;

  #n;
  pos = 5'b00000;
  neg = 5'b00000;
  fork:task_threads
    integer i;
    integer j;

    for (i = 1; i <= 5; i = i + 1) begin
      #10 neg[i] = 1;
      #10 pos[i] = 1;
      #10 neg[i] = 0;
      #10 pos[i] = 0;
    end
    for (j = 1; j <= 20; j = j + 1) begin
      @( any[1] or posedge pos[1] or negedge neg[1]
      or any[2] or posedge pos[2] or negedge neg[2]
      or any[3] or posedge pos[3] or negedge neg[3]
      or any[4] or posedge pos[4] or negedge neg[4]
      or any[5] or posedge pos[5] or negedge neg[5] );
      #n $display("task %0d triggered: %b  %b  %b %4d", n, any, pos, neg, $time);
    end
  join
end

endtask

initial begin
  fork
    report_events(1);
    report_events(2);
  join
  $finish(0);
end

endmodule
