class aclass;
   reg [3:0] q[$];
endclass

module test;

aclass a;

reg [3:0] d;

reg failed = 0;

initial begin
  a = new;
  a.q.push_back(4'd1);
  a.q.push_back(4'd2);
  a.q.push_back(4'd3);
  a.q.push_back(4'd4);
  d = a.q.pop_front();
  $display("%h", d);
  if (d !== 4'd1) failed = 1;
  d = a.q.pop_front();
  $display("%h", d);
  if (d !== 4'd2) failed = 1;
  d = a.q.pop_front();
  $display("%h", d);
  if (d !== 4'd3) failed = 1;
  d = a.q.pop_front();
  $display("%h", d);
  if (d !== 4'd4) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
