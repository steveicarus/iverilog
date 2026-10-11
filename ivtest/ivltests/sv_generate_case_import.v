// Check that selected case-generate blocks retain their package imports.

package p;
  parameter VALUE = 7;
endpackage

package q;
  parameter VALUE = 9;
endpackage

module M #(parameter SELECT = 0) (output [7:0] value);

  case (SELECT)
    0: begin : g
      import p::VALUE;
      assign value = VALUE;
    end
    1: begin : g
      import p::*;
      assign value = VALUE;
    end
    default: begin : g
      import q::VALUE;
      assign value = VALUE;
    end
  endcase

endmodule

module test;

  wire [7:0] explicit_value;
  wire [7:0] wildcard_value;
  wire [7:0] default_value;

  M #(.SELECT(0)) i_explicit(explicit_value);
  M #(.SELECT(1)) i_wildcard(wildcard_value);
  M #(.SELECT(2)) i_default(default_value);

  initial begin
    #1;
    if (explicit_value !== 8'd7 || wildcard_value !== 8'd7 ||
        default_value !== 8'd9) begin
      $display("FAILED: explicit=%0d wildcard=%0d default=%0d",
               explicit_value, wildcard_value, default_value);
    end else begin
      $display("PASSED");
    end
  end

endmodule
