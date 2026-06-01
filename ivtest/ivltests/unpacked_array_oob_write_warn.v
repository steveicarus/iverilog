module main;
  logic [7:0] a [0:3];
  real r [0:1];
  integer idx;
  integer errors;

  initial begin
    errors = 0;

    a[0] = 8'h11;
    a[1] = 8'h22;
    a[2] = 8'h33;
    a[3] = 8'h44;
    r[0] = 1.25;
    r[1] = 2.50;

    idx = 2;
    a[idx] = 8'haa;

    idx = 4;
    a[idx] = 8'hff;

    idx = -1;
    a[idx] <= 8'hee;

    idx = 5;
    repeat (2) a[idx] = 8'h77;

    idx = 3;
    r[idx] <= 9.50;

    idx = 'bx;
    a[idx] = 8'h66;

    idx = 'bz;
    a[idx] <= 8'h55;

    idx = 'bx;
    r[idx] = 7.25;

    #1;

    if (a[0] !== 8'h11) errors = errors + 1;
    if (a[1] !== 8'h22) errors = errors + 1;
    if (a[2] !== 8'haa) errors = errors + 1;
    if (a[3] !== 8'h44) errors = errors + 1;
    if (r[0] != 1.25) errors = errors + 1;
    if (r[1] != 2.50) errors = errors + 1;

    if (errors) begin
      $display("FAILED");
      $finish(1);
    end

    $display("PASSED");
  end
endmodule
