// Check mixed target kinds in ordinary and compound l-value concatenations.

module test;

  class C;
    C child;
    logic [7:0] value;
    bit [7:0] two;
  endclass

  bit failed = 1'b0;
  logic [7:0] value, selected, result;
  logic [7:0] words[0:1];
  bit [7:0] array[];
  logic [7:0] queue[$];
  string text;
  C obj, child;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function [7:0] assign_all();
    {assign_all, value, selected[3:0], words[1], array[0], queue[0],
     obj.value, obj.child.two, text[1]} = 68'h10112131415164142;
    `check({assign_all, value, selected, words[1], array[0], queue[0],
            obj.value, child.two, text[1]}, 72'h1011a2131415164142);

    {assign_all, value, selected[3:0], words[1], array[0], queue[0],
     obj.value, obj.child.two, text[1]} += 68'h01011010101010101;
    `check({assign_all, value, selected, words[1], array[0], queue[0],
            obj.value, child.two, text[1]}, 72'h1112a3141516174243);

    {assign_all, value, selected[3:0], words[1], array[0], queue[0],
     obj.value, obj.child.two, text[1]} /= 0;
    `check({assign_all, value, selected, words[1], array[0], queue[0],
            obj.value, child.two, text[1]}, 72'hxxxxaxxx00xxxx0043);
  endfunction

  initial begin
    obj = new;
    child = new;
    obj.child = child;
    array = new[1];
    queue = {8'h00};
    selected = 8'ha0;
    words[0] = 8'h5a;
    text = "abc";

    result = assign_all();
    `check(result, 8'hxx);
    `check(words[0], 8'h5a);
    `check({text[0], text[2]}, 16'h6163);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
