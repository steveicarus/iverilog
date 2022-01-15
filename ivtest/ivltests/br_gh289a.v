package pkg;

typedef enum logic [3:0] {
    ABC = 4'h1
} enum_t;

typedef struct packed {
    enum_t item;
} w_enum;

typedef struct packed {
    logic [3:0] item;
} w_logic;

typedef union packed {
    w_enum el1;
    w_logic el2;
} foo_t;

endpackage

module main();

import pkg::*;

foo_t val;

initial begin
  val.el1.item = ABC;
  if (val.el2.item === 4'h1)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
