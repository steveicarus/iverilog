// Check that objects declared with a packed range report vpiVector even
// when the range has a single element, and that plain scalars stay scalar.
// A real is neither.

module sub(input s_in, input [0:0] v_in, output s_out, output [0:0] v_out);
  assign s_out = s_in;
  assign v_out = v_in;
endmodule

module test;

  reg       s_reg;
  logic     s_logic;
  wire      s_wire = 1'b0;
  bit       s_bit;
  reg [0:0] v00_reg;
  logic [0:0] v00_logic;
  bit [0:0] v00_bit;
  logic [3:3] v33_logic;
  reg [0:3] v03_reg;
  reg [3:0] v30_reg;
  wire [0:0] v00_wire = 1'b0;
  wire [3:3] v33_wire = 1'b0;
  wire [3:0] v30_wire = 4'b0;
  logic [0:0][0:0] v_packed2;
  typedef struct packed {logic a;} ps_t;
  ps_t ps;
  logic signed       s_signed;
  logic signed [0:0] v00_signed;
  bit signed         s_bit_signed;
  bit signed [0:0]   v00_bit_signed;
  wire signed        s_wire_signed = 1'b0;
  wire signed [0:0]  v00_wire_signed = 1'b0;
  int       s_int;
  integer   s_integer;
  real      s_real;
  wire      s_arr [1:0];
  wire [0:0] v_arr [1:0];
  assign s_arr[0] = 1'b0;
  assign v_arr[0] = 1'b0;

  sub dut(.s_in(s_wire), .v_in(v00_wire), .s_out(), .v_out());

  integer failed;

  initial begin
    failed = 0;
    s_reg = 0; s_logic = 0; s_bit = 0; v00_reg = 0; v00_logic = 0;
    v00_bit = 0; v33_logic = 0; v03_reg = 0; v30_reg = 0; v_packed2 = 0; ps = 0;
    failed |= $check_sv(s_reg, 0, 1);
    failed |= $check_sv(s_logic, 0, 1);
    failed |= $check_sv(s_wire, 0, 1);
    failed |= $check_sv(s_bit, 0, 1);
    failed |= $check_sv(v00_reg, 1, 1);
    failed |= $check_sv(v00_logic, 1, 1);
    failed |= $check_sv(v00_bit, 1, 1);
    failed |= $check_sv(v33_logic, 1, 1);
    failed |= $check_sv(v03_reg, 1, 4);
    failed |= $check_sv(v30_reg, 1, 4);
    failed |= $check_sv(v00_wire, 1, 1);
    failed |= $check_sv(v33_wire, 1, 1);
    failed |= $check_sv(v30_wire, 1, 4);
    s_signed = 0; v00_signed = 0; s_bit_signed = 0; v00_bit_signed = 0;
    s_int = 0; s_integer = 0; s_real = 0.0;
    failed |= $check_sv(s_signed, 0, 1);
    failed |= $check_sv(v00_signed, 1, 1);
    failed |= $check_sv(s_bit_signed, 0, 1);
    failed |= $check_sv(v00_bit_signed, 1, 1);
    failed |= $check_sv(s_wire_signed, 0, 1);
    failed |= $check_sv(v00_wire_signed, 1, 1);
    failed |= $check_sv(s_int, 1, 32);
    failed |= $check_sv(s_integer, 1, 32);
    failed |= $check_sv(s_real, 2, 1);
    failed |= $check_sv(v_packed2, 1, 1);
    failed |= $check_sv(ps, 1, 1);
    failed |= $check_sv(dut.s_in, 0, 1);
    failed |= $check_sv(dut.v_in, 1, 1);
    failed |= $check_sv(dut.s_out, 0, 1);
    failed |= $check_sv(dut.v_out, 1, 1);
    failed |= $check_sv(s_arr[0], 0, 1);
    failed |= $check_sv(v_arr[0], 1, 1);
    if (!failed) $display("PASSED");
  end

endmodule
