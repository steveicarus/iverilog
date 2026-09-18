// A function that returns an unpacked array can be evaluated in a constant
// context, but the run-time code generator has no representation for returning
// a whole unpacked array. Calling such a function at run time must be reported
// with a clean error (not a crash).

module test;

  typedef logic [9:0] rom_t [0:3];

  function automatic rom_t build();
    rom_t r;
    r[0] = 10'd1; r[1] = 10'd2; r[2] = 10'd3; r[3] = 10'd4;
    return r;
  endfunction

  rom_t ROM;

  initial begin
    ROM = build();   // run-time call: not supported
    $display("%0d", ROM[0]);
  end

endmodule
