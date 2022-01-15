// Check that all sorts of enum dimension declarations are handled correctly and
// do not cause an assert or segfault.

module test;

// These are invalid

enum logic [$] {
	A
} a;

enum logic [] {
  B
} b;

enum logic [-1] {
	C
} c;

enum logic [0] {
	D
} d;

enum logic [1:0][3:0] {
	E
} e;

// These are valid

enum logic [0:2] {
	F
} f;

enum logic [2:0] {
	G
} g;

enum logic [-1:-2] {
	H
} h;

// These are valid as an extension in iverilog

enum logic [16] {
	I
} i;

int x;

endmodule
