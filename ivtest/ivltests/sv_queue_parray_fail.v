module top;
  typedef reg [4:0] T1;
  typedef T1 [7:0] T2;

  int bound = 2;
  T2 q_vec [$];
  T2 q_vec1 [$:-1];
  T2 q_vec2 [$:'X];
  T2 q_vec3 [$:bound];

  initial begin
    $display(q_vec.size(1));
    $display(q_vec.pop_front(1));
    $display(q_vec.pop_back(1));
    q_vec.push_front(1, 2);
    q_vec.push_back(1, 2);
    $display("FAILED");
  end
endmodule : top
