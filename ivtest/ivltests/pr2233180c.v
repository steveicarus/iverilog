module top;
  reg passed;

  reg signed[95:0] m_one, m_two, zero, one, two;

  // Both argument positive.
  reg signed[95:0] rem;
  wire signed[95:0] wrem = two / one;

  // First argument negative.
  reg signed[95:0] rem1n;
  wire signed[95:0] wrem1n = m_two / one;

  // Second argument negative.
  reg signed[95:0] rem2n;
  wire signed[95:0] wrem2n = two / m_one;

  // Both arguments negative.
  reg signed[95:0] rembn;
  wire signed[95:0] wrembn = m_two / m_one;

  // Divide by zero.
  reg signed[95:0] remd0;
  wire signed[95:0] wremd0 = one / zero;

  initial begin
    passed = 1'b1;
    m_one = 96'hffffffffffffffffffffffff;
    m_two = 96'hfffffffffffffffffffffffe;
    zero = 96'h000000000000000000000000;
    one = 96'h000000000000000000000001;
    two = 96'h000000000000000000000002;

    #1;
    // Both positive.
    if (wrem !== 96'h000000000000000000000002) begin
      $display("Failed: CA divide, expected 96'h00...02, got %h",
                wrem);
      passed = 1'b0;
    end

    rem = two / one;
    if (rem !== 96'h000000000000000000000002) begin
      $display("Failed: divide, expected 96'h00...02, got %h",
                rem);
      passed = 1'b0;
    end

    // First negative.
    if (wrem1n !== 96'hfffffffffffffffffffffffe) begin
      $display("Failed: CA divide (1n), expected 96'hff...fe, got %h",
                wrem1n);
      passed = 1'b0;
    end

    rem1n = m_two / one;
    if (rem1n !== 96'hfffffffffffffffffffffffe) begin
      $display("Failed: divide (1n), expected 96'hff...fe, got %h",
                rem1n);
      passed = 1'b0;
    end

    // Second negative.
    if (wrem2n !== 96'hfffffffffffffffffffffffe) begin
      $display("Failed: CA divide (2n), expected 96'hff...fe, got %h",
                wrem2n);
      passed = 1'b0;
    end

    rem2n = two / m_one;
    if (rem2n !== 96'hfffffffffffffffffffffffe) begin
      $display("Failed: divide (2n), expected 96'hff...fe, got %h",
                rem2n);
      passed = 1'b0;
    end

    // Both negative.
    if (wrembn !== 96'h000000000000000000000002) begin
      $display("Failed: CA divide (bn), expected 96'h00...02, got %h",
                wrembn);
      passed = 1'b0;
    end

    rembn = m_two / m_one;
    if (rembn !== 96'h000000000000000000000002) begin
      $display("Failed: divide (bn), expected 96'h00...02, got %h",
                rembn);
      passed = 1'b0;
    end

    // Divide by zero.
    if (wremd0 !== 96'hxxxxxxxxxxxxxxxxxxxxxxxx) begin
      $display("Failed: CA divide (d0), expected 96'hxx...xx, got %h",
                wremd0);
      passed = 1'b0;
    end

    remd0 = one / zero;
    if (remd0 !== 96'hxxxxxxxxxxxxxxxxxxxxxxxx) begin
      $display("Failed: divide (d0), expected 96'hxx...xx, got %h",
                remd0);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
