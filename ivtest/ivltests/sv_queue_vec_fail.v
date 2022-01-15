module top;
  int bound = 2;
  int q_vec [$];
  int q_vec1 [$:-1];
  int q_vec2 [$:'X];
  int q_vec3 [$:bound];

  initial begin
    $display(q_vec.size(1));
    $display(q_vec.pop_front(1));
    $display(q_vec.pop_back(1));
    q_vec.push_front(1, 2);
    q_vec.push_back(1, 2);
    $display("FAILED");
  end
endmodule : top
