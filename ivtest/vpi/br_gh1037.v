int top_var = 1;

module top;
  int val = 2;
  initial begin
    #1;
    $check_items();
  end
endmodule

package top;
  int val = 3;
endpackage
