// Check that structs with invalid member declarations are deteceted, report an
// error and do not cause a crash

module test;

struct packed {
  logic x;      // OK
  logic y, z;   // OK
  logic bit;    // Invalid
  logic a b c;  // Invalid
  logc d;       // Invalid
  e;            // Invalid
} s;

endmodule
