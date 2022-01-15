module test;
  parameter width = 32;
  localparam fill = 32 - width;
  localparam extend = 0;

  reg passed;
  reg [width-1:0] in;
  reg [31:0] lat1, lat2;
  wire sign_bit = in[width-1];
  wire lsb = in[0];

  initial begin
    passed = 1'b1;
    in = 32'h80000001;
    // The following are asserting().
    lat1 <= {{fill{sign_bit}}, in, {extend{lsb}}};
    lat2 = {{fill{sign_bit}}, in, {extend{lsb}}};
    #1;
    if (lat1 !== 32'h80000001) begin
      $display("FAILED: zero-replication (NB): got %h", lat1);
      passed = 1'b0;
    end
    if (lat2 !== 32'h80000001) begin
      $display("FAILED: zero-replication (B): got %h", lat2);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end

endmodule
