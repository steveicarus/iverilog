module signed_logic_operators_bug();

reg   [7:0] a, b;
wire [15:0] yuu, yus, ysu, yss;
wire [15:0] zuu, zus, zsu, zss;

initial begin
    // Example vector
    a = 8'b10110110;
    b = 8'b10010010;

    // Wait for results to be calculated
    #1;

    // Display results
    $display("a   = %b", a);
    $display("b   = %b", b);
    $display("yuu = %b", yuu);
    $display("zuu = %b", zuu);
    $display("yus = %b", yus);
    $display("zus = %b", zus);
    $display("ysu = %b", ysu);
    $display("zsu = %b", zsu);
    $display("yss = %b", yss);
    $display("zss = %b", zss);

    // Finished
    $finish(0);
end

// Calculate signed logical OR
manually_extended_logical_or INST1(.a(a), .b(b), .yuu(yuu), .yus(yus), .ysu(ysu), .yss(yss));
signed_logical_or            INST2(.a(a), .b(b), .yuu(zuu), .yus(zus), .ysu(zsu), .yss(zss));

endmodule

module manually_extended_logical_or(a, b, yuu, yus, ysu, yss);

input   [7:0] a, b;
output [15:0] yuu, yus, ysu, yss;

// Manually zero or sign extend operands before logic OR
//   - Note the operands are zero extended in "yuu", "yus" and "ysu"
//   - The operands are sign extended in "yss"
assign yuu = {{8{1'b0}}, a} | {{8{1'b0}}, b};
assign yus = {{8{1'b0}}, a} | {{8{1'b0}}, b};
assign ysu = {{8{1'b0}}, a} | {{8{1'b0}}, b};
assign yss = {{8{a[7]}}, a} | {{8{b[7]}}, b};

endmodule

module signed_logical_or(a, b, yuu, yus, ysu, yss);

input   [7:0] a, b;
output [15:0] yuu, yus, ysu, yss;

// Note that the operation is only consider signed if ALL data operands are signed
//   - Therefore $signed(a) does NOT sign extend "a" in expression "ysu"
//   - But "a" and "b" are both sign extended before the OR in expression "yss"
assign yuu =         a  |         b ;
assign yus =         a  | $signed(b);
assign ysu = $signed(a) |         b ;
assign yss = $signed(a) | $signed(b);

endmodule
