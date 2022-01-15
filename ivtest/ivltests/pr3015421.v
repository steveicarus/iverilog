// This test verifies that an incorrect function and task definition
// does not crash the compiler.
module main();
  // A number of errors here: int and return are not supported
  // (SystemVerilog), so the function definition will fail. The
  // return should also be inside the begin/end pair.
  function int pick;
    input myvar;
    begin
    end
    return 0
  endfunction

  // This is a syntax error missing ';' on the task line.
  task foo
  endtask
endmodule
