// Invalid packed dimensions
// This should generate a error message and not crash during elaboration

typedef logic [] T1;
typedef logic [0] T2;
typedef logic [-1] T3;
typedef logic [$] T4;

module test (
  input [] port_a,
  input [0] port_b,
  output [-1] port_c,
  output [$] port_d
);
	logic [$] a;
	T1 b;
	T1 [$] c;

	logic [0] d;
	T2 e;
	T2 [0] f;

	logic [-1] g;
	T3 h;
	T3 [-1] i;

	logic [$] j;
	T4 k;
	T4 [$] l;
endmodule
