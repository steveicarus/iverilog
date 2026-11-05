module test;
  typedef enum logic [2:0] { A = 0, B = 5 } first_t;
  typedef enum logic [2:0] { C = 1, D = 4 } second_t;
  first_t a, shared;
  second_t b;
  enum logic signed [2:0] { E = -2, F = 3 } signed_logic;
  enum bit signed [2:0] { G = -1, H = 2 } signed_bit;
  enum bit [2:0] { I = 2, J = 6 } unsigned_bit;
  enum integer { K = -3, L = 8 } integer_enum;
  logic [2:0] plain;
  initial $check_enum_typespec(a, shared, b, signed_logic, signed_bit,
                               unsigned_bit, integer_enum, plain);
endmodule
