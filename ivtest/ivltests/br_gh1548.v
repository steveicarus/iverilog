// Check that assigning an undefined system function to a string reports an
// error instead of crashing the compiler.
module top;
  string s;
  initial s = $no_such_function;
endmodule
