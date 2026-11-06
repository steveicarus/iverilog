`timescale 1ns/1ps
package fst_enum_pkg;
  typedef enum logic [1:0] { PKG_IDLE = 0, PKG_BUSY = 3 } state_t;
endpackage

module fst_enum_child;
  fst_enum_pkg::state_t state;
  initial begin
    state = fst_enum_pkg::PKG_IDLE;
    #10 state = fst_enum_pkg::PKG_BUSY;
  end
endmodule

module fst_enum;
  fst_enum_child u0(), u1();
  fst_enum_pkg::state_t package_state;
  enum logic [1:0] { ANON_A = 1, ANON_B = 2 } anonymous_state;
  localparam integer XBAR_W_FSM_STATES = 7;
  typedef enum logic [$clog2(XBAR_W_FSM_STATES)-1:0] {
    S_IDLE, S_QUEUE_WAIT, S_QUEUE_READ, S_REQ_WAIT, S_WRITE, S_BRESP
  } state_t;
  state_t state, shared;
  enum logic [2:0] { OTHER_A = 1, OTHER_B = 6 } other;
  enum logic signed [2:0] { NEG = -2, POS = 3 } signed_state;
  enum bit [64:0] { WIDE_A = 65'h10000000000000000, WIDE_B = 65'h10000000000000001 } wide;
  enum logic [2:0] { UNKNOWN = 3'bx01, HIGH_Z = 3'bz10 } four_state;
  enum bit signed [2:0] { BIT_NEG = -1, BIT_POS = 2 } signed_bit;
  enum integer { INT_NEG = -3, INT_POS = 8 } integer_state;
  logic [2:0] plain;
  initial begin
    $dumpfile("fst_enum.fst");
    $dumpvars(0, fst_enum);
    state = S_IDLE; shared = S_QUEUE_WAIT; other = OTHER_A;
    signed_state = NEG; wide = WIDE_A; four_state = UNKNOWN; plain = 3;
    signed_bit = BIT_NEG; integer_state = INT_NEG;
    package_state = fst_enum_pkg::PKG_IDLE; anonymous_state = ANON_A;
    #10 state = S_QUEUE_WAIT;
    package_state = fst_enum_pkg::PKG_BUSY; anonymous_state = ANON_B;
    #10 state = S_QUEUE_READ;
    #10 state = S_REQ_WAIT;
    #10 state = S_WRITE;
    #10 state = S_BRESP;
    signed_state = POS; wide = WIDE_B; four_state = HIGH_Z;
    signed_bit = BIT_POS; integer_state = INT_POS;
    #10 $finish;
  end
endmodule
