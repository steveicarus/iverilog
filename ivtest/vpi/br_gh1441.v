module test;
  logic scalar_logic = 1'b0;
  logic single_element_scalar = 1'b0;
  logic [1:0] vector_logic = 2'b00;
  logic [0:0] unit_vector_zero = 1'b0;
  logic [3:3] unit_vector_nonzero = 1'b0;
  wire scalar_wire = scalar_logic;
  wire [0:0] unit_wire = unit_vector_zero;
  wire [0:0] unit_net_array [0:0];
  real scalar_real;
  wire real scalar_real_wire = scalar_real;

  assign unit_net_array[0] = unit_vector_zero;

  initial $check_scalar_vector;
endmodule
