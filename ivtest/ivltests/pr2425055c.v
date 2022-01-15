module top;
  reg pass;
  reg [1:0] in, shift, result;
  reg signed [1:0] ins;

  wire [1:0] ls = in << shift;
  wire [1:0] als = in <<< shift;
  wire [1:0] rs = in >> shift;
  wire [1:0] rs2 = in >>> shift;
  wire [1:0] ars = ins >> shift;

  initial begin
    pass = 1'b1;
    in = 2'b01;
    ins = 2'b10;
    shift = 2'bx1;
    #1

    if (ls !== 2'bxx) begin
      $display("Failed << (CA), expected 2'bxx, got %b", ls);
      pass = 1'b0;
    end

    if (als !== 2'bxx) begin
      $display("Failed <<< (CA), expected 2'bxx, got %b", als);
      pass = 1'b0;
    end

    if (rs !== 2'bxx) begin
      $display("Failed >> (CA), expected 2'bxx, got %b", rs);
      pass = 1'b0;
    end

    if (rs2 !== 2'bxx) begin
      $display("Failed >>> (CA), expected 2'bxx, got %b", rs2);
      pass = 1'b0;
    end

    if (ars !== 2'bxx) begin
      $display("Failed >>> (signed, CA), expected 2'bxx, got %b", ars);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
