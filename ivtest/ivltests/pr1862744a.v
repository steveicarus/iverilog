module main;
  reg pass = 1'b1;

  reg v1 = 1'b0;
  reg v2 = 1'b0;
  reg v3 = 1'b0;
  reg v4 = 1'b0;
  reg v5 = 1'b0;
  reg v6 = 1'b0;
  reg v7 = 1'b0;
  reg v8 = 1'b0;
  reg v9 = 1'b0;
  reg v10 = 1'b0;
  reg v11 = 1'b0;
  reg v12 = 1'b0;
  reg cond = 1'b1;
  reg [1:0] cval = 2'b00;

  always #1 v1 = 1'b1;

  always v2 = #1 1'b1;

  always if (1'b1) #1 v3 = 1'b1;

  // This will pass since the else is optimized away!
  always if (1'b1) #1 v4 = 1'b1; else v4 = 1'b0;

  always if (cond) #1 v5 = 1'b1; else #1 v5 = 1'b0;

  always begin #1 v6 = 1'b1; end  // 1

  always begin #1; v7 = 1'b1; end  // 2

  always begin #0; #1 v8 = 1'b1; end  // 3

  always begin if (cond) #1 v9 = 1'b0; else v9 = 1'b0; #1 v9 = 1'b1; end  // 4

  always repeat(1) #1 v10 = 1'b1;

  always case(cval)
    2'b00: #1 v11 = 1'b1;
    2'b01: #1 v11 = 1'bx;
    default: #1 v11 = 1'bz;
  endcase

  always definite_delay;

  task definite_delay;
    #1 v12 = 1'b1;
  endtask

  initial begin
    #3;
    if (v1 != 1'b1) begin
      $display("Failed delayed assignment.");
      pass = 1'b0;
    end

    if (v2 != 1'b1) begin
      $display("Failed intra-assignment delay.");
      pass = 1'b0;
    end

    if (v3 != 1'b1) begin
      $display("Failed simple if statement.");
      pass = 1'b0;
    end

    if (v4 != 1'b1) begin
      $display("Failed constant if/else statement.");
      pass = 1'b0;
    end

    if (v5 != 1'b1) begin
      $display("Failed if/else statement.");
      pass = 1'b0;
    end

    if (v6 != 1'b1) begin
      $display("Failed block (1).");
      pass = 1'b0;
    end

    if (v7 != 1'b1) begin
      $display("Failed block (2).");
      pass = 1'b0;
    end

    if (v8 != 1'b1) begin
      $display("Failed block (3).");
      pass = 1'b0;
    end

    if (v9 != 1'b1) begin
      $display("Failed block (4).");
      pass = 1'b0;
    end

    if (v10 != 1'b1) begin
      $display("Failed repeat.");
      pass = 1'b0;
    end

    if (v11 != 1'b1) begin
      $display("Failed case.");
      pass = 1'b0;
    end

    if (v12 != 1'b1) begin
      $display("Failed task.");
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
    $finish;
  end
endmodule
