module top;
  int bound = 2;
  real q_real [$];
  real q_real1 [$:-1];
  real q_real2 [$:'X];
  real q_real3 [$:bound];

  initial begin
    $display(q_real.size(1.0));
    $display(q_real.pop_front(1.0));
    $display(q_real.pop_back(1.0));
    q_real.push_front(1.0, 2.0);
    q_real.push_back(1.0, 2.0);
    $display("FAILED");
  end
endmodule : top
