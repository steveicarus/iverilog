// Check that task and function argument names can shadow visible type identifiers.

typedef int T;

module test;

  reg failed;

  `define check(value, expected, error) \
    if ((value) !== (expected)) begin \
      $display("FAILED(%0d). %s", `__LINE__, error); \
      $display("  expected %0h, got %0h", expected, value); \
      failed = 1'b1; \
    end

  function int f_name(input T);
    return ($bits(T) == 1 && T === 1'b1) ? 32'd33 : -1;
  endfunction

  function int f_type(input T value);
    return ($bits(value) == 32) ? value : -1;
  endfunction

  function int f_type_list(input T value, T);
    return $bits(value) + $bits(T);
  endfunction

  function int f_type_name(input T T);
    return $bits(T);
  endfunction

  function int f_decl_name;
    input T;
    return $bits(T);
  endfunction

  function int f_decl_type;
    input T value;
    return $bits(value);
  endfunction

  function int f_decl_type_name;
    input T T;
    return $bits(T);
  endfunction

  task t_name(input T, output int value);
    value = ($bits(T) == 1 && T === 1'b1) ? 32'd44 : -1;
  endtask

  task t_type(input T value, output int result);
    result = ($bits(value) == 32) ? value : -1;
  endtask

  task t_type_list(input T value, T, output int result);
    result = $bits(value) + $bits(T);
  endtask

  task t_type_name(input T T, output int value);
    value = $bits(T);
  endtask

  task t_decl_name;
    input T;
    output int value;
    value = $bits(T);
  endtask

  task t_decl_type;
    input T value;
    output int result;
    result = $bits(value);
  endtask

  task t_decl_type_name;
    input T T;
    output int value;
    value = $bits(T);
  endtask

  initial begin
    int r0;
    int r1;
    int r2;
    int r3;
    int r4;
    int r5;
    int r6;

    failed = 1'b0;

    t_name(1'b1, r0);
    t_type(32'd55, r1);
    t_type_list(32'd11, 32'd22, r2);
    t_decl_name(1'b1, r3);
    t_decl_type(32'd33, r4);
    t_type_name(32'd44, r5);
    t_decl_type_name(32'd55, r6);

    `check(f_name(1'b1), 32'd33, "Function argument did not hide typedef");
    `check(f_type(32'd66), 32'd66, "Function typed argument regressed");
    `check(f_type_list(32'd11, 32'd22), 64, "Function typed argument list shadowing mismatch");
    `check(f_type_name(32'd33), 32, "Function type-name argument did not keep typedef type");
    `check(f_decl_name(1'b1), 1, "Function non-ANSI argument did not hide typedef");
    `check(f_decl_type(32'd66), 32, "Function non-ANSI typed argument regressed");
    `check(f_decl_type_name(32'd77), 32, "Function non-ANSI type-name argument did not keep typedef type");
    `check(r0, 32'd44, "Task argument did not hide typedef");
    `check(r1, 32'd55, "Task typed argument regressed");
    `check(r2, 64, "Task typed argument list shadowing mismatch");
    `check(r3, 1, "Task non-ANSI argument did not hide typedef");
    `check(r4, 32, "Task non-ANSI typed argument regressed");
    `check(r5, 32, "Task type-name argument did not keep typedef type");
    `check(r6, 32, "Task non-ANSI type-name argument did not keep typedef type");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
