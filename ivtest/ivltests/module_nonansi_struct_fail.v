// Check that it is an error to declare a non-ANSI module port with implicit
// packed dimensions if it is later redeclared as a packed struct typed
// variable. Even if the size of the packed dimensions matches that of the size
// of the struct.

typedef struct packed {
  reg [31:0] x;
  reg [7:0] y;
} T;

module test(x);
  output [47:0] x;
  T x;

  initial begin
    $display("FAILED");
  end

endmodule
