// Checks that the line and file information is correctly attached to a struct
// data type and will be used when printing an error message about the struct.

module test;

// Direct
struct packed {
  real r;
} s1;

// Used in a struct
typedef struct packed {
  real r;
} struct1_type;

// Used as a signal type
typedef struct packed {
  struct1_type s;
  real r;
} struct2_type;

struct2_type s2;

endmodule
