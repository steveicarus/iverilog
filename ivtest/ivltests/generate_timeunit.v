// Check that declaring a timeunit inside a generate block is an error

module test #(
  parameter A = 1
);

generate
  if (A) begin
    timeunit 10ns/1ns; // Error
  end
endgenerate

initial begin
  $display("FAILED");
end

endmodule
