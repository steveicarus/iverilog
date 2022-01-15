module test;

// The SystemVerilog standard requires that the right side of a logical operator
// is not evaluated under certain conditions.
//  For && if the left hand side is false the right hand side is not evalualted
//  For || if the left hand side is true the right hand side is not evalualted

wire a0 = 1'b0;
wire a1 = 1'b1;
wire ax = 1'bx;
wire az = 1'bz;

integer b;
logic [1:0] c;

bit failed = 1'b0;

initial begin
  // AND with first parameter 1'b0
  b = 0;
  c = 2'b00;

  if (a0 && b++ && ++b)
    c = 2'b01;

  failed |= b !== 0;
  failed |= c !== 2'b00;

  c = a0 && b++ && ++b;

  failed |= b !== 0;
  failed |= c !== 2'b00;

  // AND with first parameter 1'b1
  b = 0;
  c = 2'b00;

  if (a1 && b++ && ++b)
    c = 2'b01;

  failed |= b !== 1;
  failed |= c !== 2'b00;

  c = a1 && b++ && ++b;

  failed |= b !== 3;
  failed |= c !== 2'b01;

  // AND with first parameter 1'bz
  b = 0;
  c = 2'b00;

  if (az && b++ && ++b)
    c = 2'b01;

  failed |= b !== 1;
  failed |= c !== 2'b00;

  c = az && b++ && ++b;

  failed |= b !== 3;
  failed |= c !== 2'b0x;

  // AND with first parameter 1'bz
  b = 0;
  c = 0;

  if (ax && b++ && ++b)
    c = 2'b01;

  failed |= b !== 1;
  failed |= c !== 2'b00;

  c = ax && b++ && ++b;

  failed |= b !== 3;
  failed |= c !== 2'b0x;

  // OR with first parameter 1'b0
  b = 0;
  c = 0;

  if (a0 || b++ || ++b)
    c = 2'b01;

  failed |= b !== 2;
  failed |= c !== 2'b01;

  c = a0 || b++ || ++b;

  failed |= b !== 3;
  failed |= c !== 2'b01;

  // OR with first parameter 1'b1
  b = 0;
  c = 2'b00;

  if (a1 || b++ || ++b)
    c = 2'b01;

  failed |= b !== 0;
  failed |= c !== 2'b01;

  c = a1 || b++ || ++b;

  failed |= b !== 0;
  failed |= c !== 2'b01;

  // OR with first parameter 1'bz
  b = 0;
  c = 2'b00;

  if (az || b++ || ++b)
    c = 2'b01;

  failed |= b !== 2;
  failed |= c !== 2'b01;

  b = 0;
  c = az || b++;

  failed |= b !== 1;
  failed |= c !== 2'b0x;

  // OR with first parameter 1'bz
  b = 0;
  c = 0;

  if (ax || b++ || ++b)
    c = 2'b01;

  failed |= b !== 2;
  failed |= c !== 2'b01;

  b = 0;
  c = ax || b++;

  failed |= b !== 1;
  failed |= c !== 2'b0x;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
