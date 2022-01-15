// Test error handling for duplicate variable declarations.

module bug();

typedef struct packed {
  logic        value;
} data_t;

typedef enum { A, B } enum_t;

wire w1;
wire w1;

data_t d1;
data_t d1;

enum_t e1;
enum_t e1;

reg r1;
reg r1;

endmodule
