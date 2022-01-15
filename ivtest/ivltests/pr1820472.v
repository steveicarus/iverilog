module test;
//  parameter j=0;

  reg [5:0] j;
  reg [5:0] in [7:0];
  wire [5:0] out [7:0];

  assign out[0][1:0] = 2'b10;
  assign out[0][3:2] = 2'b01;
  assign out[1] = in[j]; // This uses the current j!
  assign out[2] = in[2];
  assign out[3] = in[3];

  initial begin
    j = 1;
    in[j] = 2'b10;
    in[2][3:2] = 2'b01;
    in[j+2][3:2] = 2'b10;

    #1;
    $display("out[0]: %b", out[0]);
    $display("out[1]: %b", out[1]);
    $display("out[2]: %b", out[2]);
    $display("out[3]: %b", out[3]);
    for (j=0; j<4; j=j+1) begin
      #0; // wait for change to propagate
      $display("out[1]-%0d: %b", j, out[1]);
    end
  end

endmodule
