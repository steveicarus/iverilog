module top;
  parameter rep = 4'bx;

  reg [31:0] a;

  initial begin
    a = {rep{8'hab}}; // This should be a compilation error!
    $display("FAILED");
  end
endmodule
