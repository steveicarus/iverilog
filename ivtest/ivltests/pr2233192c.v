module top;
  reg passed;

  reg signed[95:0] m_one, m_two, zero, one, two;

  // Both argument positive.
  reg signed[95:0] rem;
  wire signed[95:0] wrem = one % two;

  // First argument negative.
  reg signed[95:0] rem1n;
  wire signed[95:0] wrem1n = m_one % two;

  // Second argument negative.
  reg signed[95:0] rem2n;
  wire signed[95:0] wrem2n = one % m_two;

  // Both arguments negative.
  reg signed[95:0] rembn;
  wire signed[95:0] wrembn = m_one % m_two;

  // Divide by zero.
  reg signed[95:0] remd0;
  wire signed[95:0] wremd0 = one % zero;

  initial begin
    passed = 1'b1;
    m_one = 96'hffffffffffffffffffffffff;
    m_two = 96'hfffffffffffffffffffffffe;
    zero = 96'h000000000000000000000000;
    one = 96'h000000000000000000000001;
    two = 96'h000000000000000000000002;

    #1;
    // Both positive.
    if (wrem !== 96'h000000000000000000000001) begin
      $display("Failed: CA remainder, expected 96'h00...01, got %h",
                wrem);
      passed = 1'b0;
    end

    rem = one % two;
    if (rem !== 96'h000000000000000000000001) begin
      $display("Failed: remainder, expected 96'h00...01, got %h",
                rem);
      passed = 1'b0;
    end

    // First negative.
    if (wrem1n !== 96'hffffffffffffffffffffffff) begin
      $display("Failed: CA remainder (1n), expected 96'hff...ff, got %h",
                wrem1n);
      passed = 1'b0;
    end

    rem1n = m_one % two;
    if (rem1n !== 96'hffffffffffffffffffffffff) begin
      $display("Failed: remainder (1n), expected 96'hff...ff, got %h",
                rem1n);
      passed = 1'b0;
    end

    // Second negative.
    if (wrem2n !== 96'h000000000000000000000001) begin
      $display("Failed: CA remainder (2n), expected 96'h00...01, got %h",
                wrem2n);
      passed = 1'b0;
    end

    rem2n = one % m_two;
    if (rem2n !== 96'h000000000000000000000001) begin
      $display("Failed: remainder (2n), expected 96'h00...01, got %h",
                rem2n);
      passed = 1'b0;
    end

    // Both negative.
    if (wrembn !== 96'hffffffffffffffffffffffff) begin
      $display("Failed: CA remainder (bn), expected 96'hff...ff, got %h",
                wrembn);
      passed = 1'b0;
    end

    rembn = m_one % m_two;
    if (rembn !== 96'hffffffffffffffffffffffff) begin
      $display("Failed: remainder (bn), expected 96'hff...ff, got %h",
                rembn);
      passed = 1'b0;
    end

    // Divide by zero.
    if (wremd0 !== 96'hxxxxxxxxxxxxxxxxxxxxxxxx) begin
      $display("Failed: CA remainder (d0), expected 96'hxx...xx, got %h",
                wremd0);
      passed = 1'b0;
    end

    remd0 = one % zero;
    if (remd0 !== 96'hxxxxxxxxxxxxxxxxxxxxxxxx) begin
      $display("Failed: remainder (d0), expected 96'hxx...xx, got %h",
                remd0);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
