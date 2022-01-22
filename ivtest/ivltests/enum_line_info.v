// Checks that the line and file information is correctly attached to a enum
// data type and will be used when printing an error message about the enum.

module test;

// Direct
enum logic {
  A, B = A
} e1;

// Used in a struct
typedef enum logic {
  C, D = C
} enum1_type;

// Used as a signal type
typedef enum logic {
  E, F = E
} enum2_type;

// Unreferenced
typedef enum logic {
  G, H = G
} enum3_type;

struct packed {
  enum1_type e;
} s;

enum2_type e2;

endmodule
