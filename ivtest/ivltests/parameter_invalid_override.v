// Check that invalid parameter overrides generate an error

module a #(
  parameter A = 1, B = 2,
  localparam C = 3, localparam D = 4, // TODO: D should be localparam even when the keyword omitted
  parameter E = 5
);

  // TODO: parameter here should be treated as a localparam since the module has a
  // parameter port list
  /*parameter*/ localparam F = 6, G = 7;
  localparam H = 8, I = 9;

endmodule

module b;

  parameter A = 1, B = 2;
  localparam C = 3, D = 4;

endmodule

module test;

  a #(
    .A(10),
    .B(20),
    .C(30),
    .D(40),
    .E(50),
    .F(60),
    .G(70),
    .H(80),
    .I(90),
    .Z(99)
  ) i_a();

  defparam i_a.A = 100;
  defparam i_a.B = 200;
  defparam i_a.C = 300;
  defparam i_a.D = 400;
  defparam i_a.E = 500;
  defparam i_a.F = 600;
  defparam i_a.G = 700;
  defparam i_a.H = 800;
  defparam i_a.I = 900;
  defparam i_a.Z = 999;

  b #(
    .A(10),
    .B(20),
    .C(30),
    .D(40),
    .Z(99)
  ) i_b();

  defparam i_b.A = 100;
  defparam i_b.B = 200;
  defparam i_b.C = 300;
  defparam i_b.D = 400;
  defparam i_b.Z = 999;

endmodule
