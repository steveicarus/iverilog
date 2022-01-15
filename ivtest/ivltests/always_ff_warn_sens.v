module top;
  reg q1, q2, q3, q4, q5, q6, q7, d;
  reg clk;
  reg [5:4] rst;
  integer i;

  // The compiler should warn that clk is missing an edge keyword.
  always_ff @(clk) begin
    q1 <= d;
  end

  // The compiler should warn that rst is missing an edge keyword.
  always_ff @(posedge clk or rst[4]) begin
    if (rst[4])
      q2 <= 1'b0;
    else
      q2 <= d;
  end

  // The compiler should warn that rst is missing an edge keyword.
  always_ff @(posedge clk or rst[i]) begin
    if (rst[i])
      q3 <= 1'b0;
    else
      q3 <= d;
  end

  // The compiler should warn that rst is missing an edge keyword.
  always_ff @(posedge clk or !rst) begin
    if (!rst)
      q4 <= 1'b0;
    else
      q4 <= d;
  end

  // The compiler should warn that rst is missing an edge keyword.
  always_ff @(posedge clk or ~rst[4]) begin
    if (~rst[4])
      q5 <= 1'b0;
    else
      q5 <= d;
  end

  // The compiler should warn that rst is missing an edge keyword.
  always_ff @(posedge clk or &rst) begin
    if (&rst)
      q6 <= 1'b0;
    else
      q6 <= d;
  end

  // The compiler should warn that rst is not a single bit.
  always_ff @(posedge clk or posedge rst) begin
    if (rst)
      q7 <= 1'b0;
    else
      q7 <= d;
  end

  initial begin
    $display("Expect compile warnings!\nPASSED");
  end

endmodule
