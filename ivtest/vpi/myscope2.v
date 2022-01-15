//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
 module xor_try;

    reg  [1:0]  inp_xor;    // The two-bit inputs to the XOR
    reg     out_xor;        // The XOR output
    reg     clk;

    initial begin clk = 1'b1; #10 $sn; #160 $finish(0); end
    always #50 clk = ~clk;
                // The clock

    always @(posedge clk) out_xor = #1 (inp_xor[0] ^ inp_xor[1]);
                // The actual operation
 endmodule

