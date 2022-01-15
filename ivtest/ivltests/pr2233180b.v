module top;
  reg passed;

  reg signed[31:0] m_one, m_two, zero, one, two;

  // Both argument positive.
  reg signed[31:0] rem;
  wire signed[31:0] wrem = two / one;

  // First argument negative.
  reg signed[31:0] rem1n;
  wire signed[31:0] wrem1n = m_two / one;

  // Second argument negative.
  reg signed[31:0] rem2n;
  wire signed[31:0] wrem2n = two / m_one;

  // Both arguments negative.
  reg signed[31:0] rembn;
  wire signed[31:0] wrembn = m_two / m_one;

  // Divide by zero.
  reg signed[31:0] remd0;
  wire signed[31:0] wremd0 = one / zero;

  initial begin
    passed = 1'b1;
    m_one = 32'hffffffff;
    m_two = 32'hfffffffe;
    zero = 32'h00000000;
    one = 32'h00000001;
    two = 32'h00000002;

    #1;
    // Both positive.
    if (wrem !== 32'h00000002) begin
      $display("Failed: CA divide, expected 32'h00...02, got %h",
                wrem);
      passed = 1'b0;
    end

    rem = two / one;
    if (rem !== 32'h00000002) begin
      $display("Failed: divide, expected 32'h00...02, got %h",
                rem);
      passed = 1'b0;
    end

    // First negative.
    if (wrem1n !== 32'hfffffffe) begin
      $display("Failed: CA divide (1n), expected 32'hff...fe, got %h",
                wrem1n);
      passed = 1'b0;
    end

    rem1n = m_two / one;
    if (rem1n !== 32'hfffffffe) begin
      $display("Failed: divide (1n), expected 32'hff...fe, got %h",
                rem1n);
      passed = 1'b0;
    end

    // Second negative.
    if (wrem2n !== 32'hfffffffe) begin
      $display("Failed: CA divide (2n), expected 32'hff...fe, got %h",
                wrem2n);
      passed = 1'b0;
    end

    rem2n = two / m_one;
    if (rem2n !== 32'hfffffffe) begin
      $display("Failed: divide (2n), expected 32'hff...fe, got %h",
                rem2n);
      passed = 1'b0;
    end

    // Both negative.
    if (wrembn !== 32'h00000002) begin
      $display("Failed: CA divide (bn), expected 32'h00...02, got %h",
                wrembn);
      passed = 1'b0;
    end

    rembn = m_two / m_one;
    if (rembn !== 32'h00000002) begin
      $display("Failed: divide (bn), expected 32'h00...02, got %h",
                rembn);
      passed = 1'b0;
    end

    // Divide by zero.
    if (wremd0 !== 32'hxxxxxxxx) begin
      $display("Failed: CA divide (d0), expected 32'hxx...xx, got %h",
                wremd0);
      passed = 1'b0;
    end

    remd0 = one / zero;
    if (remd0 !== 32'hxxxxxxxx) begin
      $display("Failed: divide (d0), expected 32'hxx...xx, got %h",
                remd0);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
