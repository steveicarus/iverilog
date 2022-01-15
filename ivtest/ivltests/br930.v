`begin_keywords "1364-2005"
// Check that attributes are supported on all forms of
// module port declaration.

module m1(

(* type=1, name="a" *) input  wire a,
(* type=1, name="b" *) inout  wire b,
(* type=1, name="c" *) output wire c,
(* type=1, name="d" *) output reg  d

);

endmodule

module m2(a, b, c, d);

(* type=2, name="a" *) input  wire a;
(* type=2, name="b" *) inout  wire b;
(* type=2, name="c" *) output wire c;
(* type=2, name="d" *) output reg  d;

endmodule

module m3(a, b, c, d);

(* type=3 *) input  a;
(* type=3 *) inout  b;
(* type=3 *) output c;
(* type=3 *) output d;

(* name="a" *) wire a;
(* name="b" *) wire b;
(* name="c" *) wire c;
(* name="d" *) reg  d;

endmodule
`end_keywords
