// Check the error message produced by a procedural assignment to an input or
// inout port. See GitHub issue #1284.
//
// An input or inout port is a net unless it is explicitly declared to be a
// variable, so a procedural assignment to one is an error. Declaring the port
// with the "reg" data type does not change that ("input reg" is accepted as
// part of the extended data type support enabled by -gxtypes). Reporting only
// the resolved net type is confusing in that case, because the source says
// "reg" but the message says "wire", so the port direction is reported too.

module test_input(input reg i, output reg o);
  always @* begin
    i <= o;
  end
endmodule

module test_inout(inout b, output reg o);
  always @* begin
    b <= o;
  end
endmodule
