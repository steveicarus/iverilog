module signed_logic_operators_bug();

reg         sel;
reg   [7:0] a, b;
reg   [5:0] c;
wire [15:0] y_mux_uu, y_mux_us, y_mux_su, y_mux_ss;
wire        y_eql_uu, y_eql_us, y_eql_su, y_eql_ss;
wire        y_neq_uu, y_neq_us, y_neq_su, y_neq_ss;
wire [15:0] y_sgn_u,  y_sgn_s;
wire [15:0] y_add_uu, y_add_us, y_add_su, y_add_ss;
wire [15:0] y_sub_uu, y_sub_us, y_sub_su, y_sub_ss;
wire [15:0] y_mul_uu, y_mul_us, y_mul_su, y_mul_ss;
wire        y_ltn_uu, y_ltn_us, y_ltn_su, y_ltn_ss;
wire        y_leq_uu, y_leq_us, y_leq_su, y_leq_ss;
wire [15:0] z_mux_uu, z_mux_us, z_mux_su, z_mux_ss;
wire        z_eql_uu, z_eql_us, z_eql_su, z_eql_ss;
wire        z_neq_uu, z_neq_us, z_neq_su, z_neq_ss;
wire [15:0] z_sgn_u,  z_sgn_s;
wire [15:0] z_add_uu, z_add_us, z_add_su, z_add_ss;
wire [15:0] z_sub_uu, z_sub_us, z_sub_su, z_sub_ss;
wire [15:0] z_mul_uu, z_mul_us, z_mul_su, z_mul_ss;
wire        z_ltn_uu, z_ltn_us, z_ltn_su, z_ltn_ss;
wire        z_leq_uu, z_leq_us, z_leq_su, z_leq_ss;

integer i;

initial begin
    for (i = 0; i < 100; i = i + 1) begin
        // Example vector
        sel = $random;
        a   = $random;
        b   = $random;
        c   = $random;

        // Wait for results to be calculated
        #1;

        // Display results
        $display("sel = %b", sel);
        $display("a   = %b", a);
        $display("b   = %b", b);
        $display("c   = %b", c);
        $display("y_mux_uu = %b   y_mux_us = %b   y_mux_su = %b   y_mux_ss = %b", y_mux_uu, y_mux_us, y_mux_su, y_mux_ss);
        $display("z_mux_uu = %b   z_mux_us = %b   z_mux_su = %b   z_mux_ss = %b", z_mux_uu, z_mux_us, z_mux_su, z_mux_ss);
        $display("y_eql_uu = %b   y_eql_us = %b   y_eql_su = %b   y_eql_ss = %b", y_eql_uu, y_eql_us, y_eql_su, y_eql_ss);
        $display("z_eql_uu = %b   z_eql_us = %b   z_eql_su = %b   z_eql_ss = %b", z_eql_uu, z_eql_us, z_eql_su, z_eql_ss);
        $display("y_neq_uu = %b   y_neq_us = %b   y_neq_su = %b   y_neq_ss = %b", y_neq_uu, y_neq_us, y_neq_su, y_neq_ss);
        $display("z_neq_uu = %b   z_neq_us = %b   z_neq_su = %b   z_neq_ss = %b", z_neq_uu, z_neq_us, z_neq_su, z_neq_ss);
        $display("y_sgn_u  = %b   y_sgn_s  = %b"                                , y_sgn_u,  y_sgn_s);
        $display("z_sgn_u  = %b   z_sgn_s  = %b"                                , z_sgn_u,  z_sgn_s);
        $display("y_add_uu = %b   y_add_us = %b   y_add_su = %b   y_add_ss = %b", y_add_uu, y_add_us, y_add_su, y_add_ss);
        $display("z_add_uu = %b   z_add_us = %b   z_add_su = %b   z_add_ss = %b", z_add_uu, z_add_us, z_add_su, z_add_ss);
        $display("y_sub_uu = %b   y_sub_us = %b   y_sub_su = %b   y_sub_ss = %b", y_sub_uu, y_sub_us, y_sub_su, y_sub_ss);
        $display("z_sub_uu = %b   z_sub_us = %b   z_sub_su = %b   z_sub_ss = %b", z_sub_uu, z_sub_us, z_sub_su, z_sub_ss);
        $display("y_mul_uu = %b   y_mul_us = %b   y_mul_su = %b   y_mul_ss = %b", y_mul_uu, y_mul_us, y_mul_su, y_mul_ss);
        $display("z_mul_uu = %b   z_mul_us = %b   z_mul_su = %b   z_mul_ss = %b", z_mul_uu, z_mul_us, z_mul_su, z_mul_ss);
        $display("y_ltn_uu = %b   y_ltn_us = %b   y_ltn_su = %b   y_ltn_ss = %b", y_ltn_uu, y_ltn_us, y_ltn_su, y_ltn_ss);
        $display("z_ltn_uu = %b   z_ltn_us = %b   z_ltn_su = %b   z_ltn_ss = %b", z_ltn_uu, z_ltn_us, z_ltn_su, z_ltn_ss);
        $display("y_leq_uu = %b   y_leq_us = %b   y_leq_su = %b   y_leq_ss = %b", y_leq_uu, y_leq_us, y_leq_su, y_leq_ss);
        $display("z_leq_uu = %b   z_leq_us = %b   z_leq_su = %b   z_leq_ss = %b", z_leq_uu, z_leq_us, z_leq_su, z_leq_ss);
    end

    // Finished
    $finish(0);
end

// Manually sign extended operators
manually_extended_operators INST1(
    sel,
    a, b,
    c,
    y_mux_uu, y_mux_us, y_mux_su, y_mux_ss,
    y_eql_uu, y_eql_us, y_eql_su, y_eql_ss,
    y_neq_uu, y_neq_us, y_neq_su, y_neq_ss,
    y_sgn_u,  y_sgn_s,
    y_add_uu, y_add_us, y_add_su, y_add_ss,
    y_sub_uu, y_sub_us, y_sub_su, y_sub_ss,
    y_mul_uu, y_mul_us, y_mul_su, y_mul_ss,
    y_ltn_uu, y_ltn_us, y_ltn_su, y_ltn_ss,
    y_leq_uu, y_leq_us, y_leq_su, y_leq_ss
    );

// $signed() sign extended operators
signed_operators INST2(
    sel,
    a, b,
    c,
    z_mux_uu, z_mux_us, z_mux_su, z_mux_ss,
    z_eql_uu, z_eql_us, z_eql_su, z_eql_ss,
    z_neq_uu, z_neq_us, z_neq_su, z_neq_ss,
    z_sgn_u,  z_sgn_s,
    z_add_uu, z_add_us, z_add_su, z_add_ss,
    z_sub_uu, z_sub_us, z_sub_su, z_sub_ss,
    z_mul_uu, z_mul_us, z_mul_su, z_mul_ss,
    z_ltn_uu, z_ltn_us, z_ltn_su, z_ltn_ss,
    z_leq_uu, z_leq_us, z_leq_su, z_leq_ss
    );

endmodule

module signed_operators(
    sel,
    a, b,
    c,
    mux_uu, mux_us, mux_su, mux_ss,
    eql_uu, eql_us, eql_su, eql_ss,
    neq_uu, neq_us, neq_su, neq_ss,
    sgn_u,  sgn_s,
    add_uu, add_us, add_su, add_ss,
    sub_uu, sub_us, sub_su, sub_ss,
    mul_uu, mul_us, mul_su, mul_ss,
    ltn_uu, ltn_us, ltn_su, ltn_ss,
    leq_uu, leq_us, leq_su, leq_ss
    );

input         sel;
input   [7:0] a, b;
input   [5:0] c;
output [15:0] mux_uu, mux_us, mux_su, mux_ss;
output        eql_uu, eql_us, eql_su, eql_ss;
output        neq_uu, neq_us, neq_su, neq_ss;
output [15:0] sgn_u,  sgn_s;
output [15:0] add_uu, add_us, add_su, add_ss;
output [15:0] sub_uu, sub_us, sub_su, sub_ss;
output [15:0] mul_uu, mul_us, mul_su, mul_ss;
output        ltn_uu, ltn_us, ltn_su, ltn_ss;
output        leq_uu, leq_us, leq_su, leq_ss;

// Note that the operation is only consider signed if ALL data operands are signed
//   - Therefore $signed(a) does NOT sign extend "a" in expression "X_su"
//   - But "a" and "b" are both sign extended before the operation in expression "X_ss"

assign mux_uu = sel ?         a  :         b ;
assign mux_us = sel ?         a  : $signed(b);
assign mux_su = sel ? $signed(a) :         b ;
assign mux_ss = sel ? $signed(a) : $signed(b);

assign eql_uu =         a  ==         c ;
assign eql_us =         a  == $signed(c);
assign eql_su = $signed(a) ==         c ;
assign eql_ss = $signed(a) == $signed(c);

assign neq_uu =         a  !=         c ;
assign neq_us =         a  != $signed(c);
assign neq_su = $signed(a) !=         c ;
assign neq_ss = $signed(a) != $signed(c);

assign sgn_u = ~$unsigned(c) ;
assign sgn_s = ~$signed(c) ;

assign add_uu =         a  +         c ;
assign add_us =         a  + $signed(c);
assign add_su = $signed(a) +         c ;
assign add_ss = $signed(a) + $signed(c);

assign sub_uu =         a  -         c ;
assign sub_us =         a  - $signed(c);
assign sub_su = $signed(a) -         c ;
assign sub_ss = $signed(a) - $signed(c);

assign mul_uu =         a  *         c ;
assign mul_us =         a  * $signed(c);
assign mul_su = $signed(a) *         c ;
assign mul_ss = $signed(a) * $signed(c);

assign ltn_uu =         a  <         c ;
assign ltn_us =         a  < $signed(c);
assign ltn_su = $signed(a) <         c ;
assign ltn_ss = $signed(a) < $signed(c);

assign leq_uu =         a  <=         c ;
assign leq_us =         a  <= $signed(c);
assign leq_su = $signed(a) <=         c ;
assign leq_ss = $signed(a) <= $signed(c);

endmodule

module manually_extended_operators(
    sel,
    a, b,
    c,
    mux_uu, mux_us, mux_su, mux_ss,
    eql_uu, eql_us, eql_su, eql_ss,
    neq_uu, neq_us, neq_su, neq_ss,
    sgn_u,  sgn_s,
    add_uu, add_us, add_su, add_ss,
    sub_uu, sub_us, sub_su, sub_ss,
    mul_uu, mul_us, mul_su, mul_ss,
    ltn_uu, ltn_us, ltn_su, ltn_ss,
    leq_uu, leq_us, leq_su, leq_ss
    );

input         sel;
input   [7:0] a, b;
input   [5:0] c;
output [15:0] mux_uu, mux_us, mux_su, mux_ss;
output        eql_uu, eql_us, eql_su, eql_ss;
output        neq_uu, neq_us, neq_su, neq_ss;
output [15:0] sgn_u,  sgn_s;
output [15:0] add_uu, add_us, add_su, add_ss;
output [15:0] sub_uu, sub_us, sub_su, sub_ss;
output [15:0] mul_uu, mul_us, mul_su, mul_ss;
output        ltn_uu, ltn_us, ltn_su, ltn_ss;
output        leq_uu, leq_us, leq_su, leq_ss;

// Manually zero or sign extend operands before the operation.
//   - Note the operands are zero extended in "X_uu", "X_us" and "X_su"
//   - The operands are sign extended in "X_ss"

assign mux_uu = sel ? {{8{1'b0}}, a} : {{8{1'b0}}, b};
assign mux_us = sel ? {{8{1'b0}}, a} : {{8{1'b0}}, b};
assign mux_su = sel ? {{8{1'b0}}, a} : {{8{1'b0}}, b};
assign mux_ss = sel ? {{8{a[7]}}, a} : {{8{b[7]}}, b};

assign eql_uu = {a} == {{2{1'b0}}, c};
assign eql_us = {a} == {{2{1'b0}}, c};
assign eql_su = {a} == {{2{1'b0}}, c};
assign eql_ss = {a} == {{2{c[5]}}, c};

assign neq_uu = {a} != {{2{1'b0}}, c};
assign neq_us = {a} != {{2{1'b0}}, c};
assign neq_su = {a} != {{2{1'b0}}, c};
assign neq_ss = {a} != {{2{c[5]}}, c};

assign sgn_u = ~{{10{1'b0}}, c} ;
assign sgn_s = ~{{10{c[5]}}, c} ;

assign add_uu = {{8{1'b0}}, a} + {{10{1'b0}}, c};
assign add_us = {{8{1'b0}}, a} + {{10{1'b0}}, c};
assign add_su = {{8{1'b0}}, a} + {{10{1'b0}}, c};
assign add_ss = {{8{a[7]}}, a} + {{10{c[5]}}, c};

assign sub_uu = {{8{1'b0}}, a} - {{10{1'b0}}, c};
assign sub_us = {{8{1'b0}}, a} - {{10{1'b0}}, c};
assign sub_su = {{8{1'b0}}, a} - {{10{1'b0}}, c};
assign sub_ss = {{8{a[7]}}, a} - {{10{c[5]}}, c};

assign mul_uu = {{8{1'b0}}, a} * {{10{1'b0}}, c};
assign mul_us = {{8{1'b0}}, a} * {{10{1'b0}}, c};
assign mul_su = {{8{1'b0}}, a} * {{10{1'b0}}, c};
assign mul_ss = {{8{a[7]}}, a} * {{10{c[5]}}, c};

assign ltn_uu = {{8{1'b0}}, a} < {{10{1'b0}}, c};
assign ltn_us = {{8{1'b0}}, a} < {{10{1'b0}}, c};
assign ltn_su = {{8{1'b0}}, a} < {{10{1'b0}}, c};
assign ltn_ss = {c[5],{7{a[7]}}, a} < {a[7],{9{c[5]}}, c};

assign leq_uu = {{8{1'b0}}, a} <= {{10{1'b0}}, c};
assign leq_us = {{8{1'b0}}, a} <= {{10{1'b0}}, c};
assign leq_su = {{8{1'b0}}, a} <= {{10{1'b0}}, c};
assign leq_ss = {c[5],{7{a[7]}}, a} <= {a[7],{9{c[5]}}, c};

endmodule
