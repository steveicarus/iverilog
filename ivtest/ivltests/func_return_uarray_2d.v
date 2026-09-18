// A constant function that returns a two-dimensional unpacked array, used to
// initialize a localparam ROM. Exercises the flattened multi-index arithmetic
// during compile-time function evaluation. See the RS(576,514) GF decoder
// reproducer (the alpha^(o*m) tables).

module test;

  typedef logic [9:0] rom_t [0:7][0:31];

  function automatic rom_t build();
    rom_t t;
    int o, m;
    for (o = 0 ; o < 8 ; o = o + 1)
      for (m = 0 ; m < 32 ; m = m + 1)
        t[o][m] = o * 32 + m;
    return t;
  endfunction

  localparam rom_t ROM = build();

  integer o, m;
  integer errors = 0;
  logic [9:0] v;

  initial begin
    for (o = 0 ; o < 8 ; o = o + 1)
      for (m = 0 ; m < 32 ; m = m + 1) begin
        v = ROM[o][m];
        if (v !== (o*32 + m)) begin
          errors = errors + 1;
          $display("FAILED: ROM[%0d][%0d] = %0d, expected %0d",
                   o, m, v, o*32 + m);
        end
      end
    if (errors == 0)
      $display("PASSED");
  end

endmodule
