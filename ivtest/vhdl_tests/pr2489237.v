module top
  (
   // Outputs
   e, f, g, h,
   // Inputs
   clk, a, b, c, d
   );

  input                 clk;
  input                 a, b, c, d;
  output                e, f, g, h;

  child child
    (/*AUTOINST*/
     // Outputs
     .e					(e),
     .f					(f),
     .g					(g),
     .h					(h),
     // Inputs
     .clk				(clk),
     .a					(a),
     .b					(b),
     .c					(c),
     .d					(d));
endmodule // top

module child(
  // Outputs
  e, f, g, h,
  // Inputs
  clk, a, b, c, d
  );

  input                 clk;
  input                 a, b, c, d;
  output                e, f, g, h;

  reg			e;
  reg			f;
  reg			g;
  reg			h;

  always @ (posedge clk) e <= a;
  always @ (posedge clk) f <= b;
  always @ (posedge clk) g <= c;

endmodule // child
