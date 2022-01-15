module top;
  reg in1, in2, pass, checkc, checkg, checkp;
  wire outc, outg, outp;

  assign #400 outc = in1 | in2;
  or #400 g1(outg, in1, in2);
  my_or #400 g2(outp, in1, in2);

  initial begin
//    $monitor($time,,outc, outg, outp,, in1,, in2);
    pass = 1'b1;
    checkc = 1'b0;
    checkg = 1'b0;
    checkp = 1'b0;
    in1 = 1'b0;
    in2 = 1'b0;
    #100 in1 = 1'b1;
    #200 in2 = 1'b1;
    #199;
    // Check to see if the output changed early.
    if (outc !== 1'bz && outc !== 1'bx) begin
      $display("CA output changed early!");
      pass = 1'b0;
    end
    if (outg !== 1'bz && outg !== 1'bx) begin
      $display("Gate output changed early!");
      pass = 1'b0;
    end
    if (outp !== 1'bz && outp !== 1'bx) begin
      $display("UDP output changed early!");
      pass = 1'b0;
    end
    #2;
    // Check to see if the output changed late.
    if (outc !== 1'b1) begin
      $display("CA output changed late!");
      pass = 1'b0;
      checkc = 1'b1;
    end
    if (outg !== 1'b1) begin
      $display("Gate output changed late!");
      pass = 1'b0;
      checkg = 1'b1;
    end
    if (outp !== 1'b1) begin
      $display("UDP output changed late!");
      pass = 1'b0;
      checkp = 1'b1;
    end
    #198;
    // We need to execute the three if checks in parallel.
    fork
      if (checkc) begin
        if (outc === 1'bz || outc === 1'bx) begin
          #2;
          // Check to see if the output changed off of the wrong edge.
          if (outc === 1'b1)
            $display("CA output triggered off of in2 change instead of in1.");
          else
            $display("CA output triggered very late.");
        end
      end
      if (checkg) begin
        if (outg === 1'bz || outg === 1'bx) begin
          #2;
          // Check to see if the output changed off of the wrong edge.
          if (outg === 1'b1)
            $display("Gate output triggered off of in2 change instead of in1.");
          else
            $display("Gate output triggered very late.");
        end
      end
      if (checkp) begin
        if (outp === 1'bz || outp === 1'bx) begin
          #2;
          // Check to see if the output changed off of the wrong edge.
          if (outp === 1'b1)
            $display("UDP output triggered off of in2 change instead of in1.");
          else
            $display("UDP output triggered very late.");
        end
      end
      #2; // This keeps the passing case alligned with the fails.
    join

    // Generate a 399 wide negative pulse that should be skipped.
    in1 = 1'b0;
    in2 = 1'b0;
    #399;
    in1 = 1'b1;
    in2 = 1'b1;
    #2;
    // Check that the pulse did not propagate.
    if (outc !== 1'b1) begin
      $display("CA does not have inertial delay.");
      pass = 1'b0;
    end
    if (outg !== 1'b1) begin
      $display("Gate does not have inertial delay.");
      pass = 1'b0;
    end
    if (outp !== 1'b1) begin
      $display("UDP does not have inertial delay.");
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule

primitive my_or (out, in1, in2);
  output out;
  input in1, in2;

  table
    0 0 : 0;
    1 ? : 1;
    ? 1 : 1;
  endtable
endprimitive
