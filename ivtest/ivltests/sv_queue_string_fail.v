module top;
  int bound = 2;
  string q_str [$];
  string q_str1 [$:-1];
  string q_str2 [$:'X];
  string q_str3 [$:bound];

  initial begin
    $display(q_str.size("a"));
    $display(q_str.pop_front("a"));
    $display(q_str.pop_back("a"));
    q_str.push_front("a", "b");
    q_str.push_back("a", "b");
    $display("FAILED");
  end
endmodule : top
