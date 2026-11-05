// Check that empty parameter-port lists are supported in SystemVerilog.

module M #()(output [3:0] y); // Error: Verilog does not allow empty parameter-port lists.
  parameter P = 7;
  assign y = P;
endmodule

module test;
  wire [3:0] y;
  M i_m(y);

  initial begin
    #1;
    if (y === 4'd7) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
