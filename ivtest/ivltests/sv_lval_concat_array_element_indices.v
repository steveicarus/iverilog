// Check index evaluation for array elements in l-value concatenations.

module test;

  bit failed = 1'b0;
  logic [7:0] a[];
  logic [7:0] q[$];
  logic [7:0] value;
  logic signed [31:0] index;
  int a_calls = 0;
  int q_calls = 0;
  int rhs_calls = 0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function int a_index();
    a_calls++;
    return 1;
  endfunction

  function int q_index();
    q_calls++;
    return 0;
  endfunction

  function logic [15:0] rhs();
    rhs_calls++;
    return 16'h1234;
  endfunction

  initial begin
    a = new[2];
    a[0] = 8'h11;
    q = {8'h00};

    {a[a_index()], q[q_index()]} = rhs();
    `check({a[1], q[0]}, 16'h12_34);
    `check(a_calls, 1);
    `check(q_calls, 1);
    `check(rhs_calls, 1);

    {a[a_index()], q[q_index()]} += rhs();
    `check({a[1], q[0]}, 16'h24_68);
    `check(a_calls, 2);
    `check(q_calls, 2);
    `check(rhs_calls, 2);

    index = 'x;
    {value, a[index], q[index]} = 24'h123456;
    `check({value, a[0], a[1], q[0]}, 32'h12_11_24_68);
    `check(q.size(), 1);

    {value, a[index], q[0]} |= 24'h010001;
    `check({value, a[0], a[1], q[0]}, 32'h13_11_24_69);
    `check(q.size(), 1);

    {value, q[index], a[1]} |= 24'h010001;
    `check({value, a[0], a[1], q[0]}, 32'h13_11_25_69);
    `check(q.size(), 1);

    index = 3;
    {q[index], a[index], value} = 24'h56789a;
    `check({value, a[0], a[1], q[0]}, 32'h9a_11_25_69);
    `check(q.size(), 1);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
