module top;
  reg [7:0] result;

  initial begin
    result = {0{1'b1}}; // This fails top level zero replication.
  end
endmodule
