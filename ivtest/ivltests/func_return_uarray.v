// A constant function that returns an unpacked array, used to initialize a
// localparam ROM (a common way to build lookup tables). The array is evaluated
// at elaboration time and then indexed. This is legal SystemVerilog that other
// simulators accept. See the RS(576,514) GF decoder reproducer.

module test;

  typedef logic [9:0] rom_t [0:7];

  function automatic rom_t build();
    rom_t r;
    int i;
    for (i = 0 ; i < 8 ; i = i + 1)
      r[i] = i * 3 + 1;
    return r;
  endfunction

  localparam rom_t ROM = build();

  // A constant element select must work in a constant expression too.
  localparam int K = ROM[4];

  integer i;
  integer errors = 0;
  logic [9:0] v;

  initial begin
    for (i = 0 ; i < 8 ; i = i + 1) begin
      v = ROM[i];
      if (v !== (i*3 + 1)) begin
        errors = errors + 1;
        $display("FAILED: ROM[%0d] = %0d, expected %0d", i, v, i*3 + 1);
      end
    end
    if (K !== 13) begin
      errors = errors + 1;
      $display("FAILED: constant ROM[4] = %0d, expected 13", K);
    end
    if (errors == 0)
      $display("PASSED");
  end

endmodule
