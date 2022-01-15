module br918d;
  reg pass;
  reg [1:0] v1, v2, v3, v4;

  wire [3:0] w1, w2, w3, w4;

  // Assign as pieces with matching strengths.
  assign (pull1,strong0) w1[1:0] = v1;
  assign (pull1,strong0) w1[1:0] = v2;
  assign (pull1,strong0) w1[3:2] = v3;
  assign (pull1,strong0) w1[3:2] = v4;

  // Assign with a concat.
  assign (pull1,strong0) w2 = {v3, v1};
  assign (pull1,strong0) w2 = {v4, v2};

  // Only assign part
  assign (pull1,strong0) w3[1:0] = v1;
  assign (pull1,strong0) w3[1:0] = v2;

  // Assign as pieces with different strengths.
  assign (pull1,strong0) w4[1:0] = v1;
  assign (pull1,strong0) w4[1:0] = v2;
  assign (strong1,pull0) w4[3:2] = v3;
  assign (strong1,pull0) w4[3:2] = v4;

  initial begin
    pass = 1'b1;
    v1 = 2'b00;
    v2 = 2'b10;
    v3 = 2'b11;
    v4 = 2'b10;
    #1;
    // Check the assign as pieces (this is the same as br918a)
    if (w1 !== 4'b1000) begin
      $display("FAILED: assign with pieces (1), expected 4'b1000, got %b", w1);
      pass = 1'b0;
    end
    // Check the assign with a concat.
    if (w2 !== 4'b1000) begin
      $display("FAILED: assign with concat, expected 4'b1000, got %b", w2);
      pass = 1'b0;
    end
    // Check when only a piece is assigned (other compilers may return xx00).
    if ((w3 !== 4'bzz00) && (w3 !== 4'bxx00)) begin
      $display("FAILED: assign part, expected 4'bzz00 or 4'bxx00, got %b", w3);
      pass = 1'b0;
    end
    // Check the assign as pieces (this is the same as br918a)
    if (w4 !== 4'b1100) begin
      $display("FAILED: assign with pieces (2), expected 4'b1000, got %b", w4);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
