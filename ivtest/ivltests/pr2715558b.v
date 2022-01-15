/* The original test case submitted for pr2715558 should not have
   given the the results the bug reporter expected (see pr2986806).
   This is a reworked version that does give those results.
*/
module pr2715558b();

wire sup1_sup0;
wire sup1_str0;
wire sup1_pl0;
wire sup1_we0;

assign (supply0, supply1) sup1_sup0 = 1'b1;
assign (supply0, supply1) sup1_sup0 = 1'b0;
assign (supply0, supply1) sup1_str0 = 1'b1;
assign (strong0, strong1) sup1_str0 = 1'b0;
assign (supply0, supply1) sup1_pl0  = 1'b1;
assign (pull0,   pull1)   sup1_pl0  = 1'b0;
assign (supply0, supply1) sup1_we0  = 1'b1;
assign (weak0,   weak1)   sup1_we0  = 1'b0;

initial begin
    #1;
    $display("sup1_sup0 resulted in: %b", sup1_sup0);
    $display("sup1_str0 resulted in: %b", sup1_str0);
    $display("sup1_pl0  resulted in: %b", sup1_pl0);
    $display("sup1_we0  resulted in: %b", sup1_we0);
end

wire str1_sup0;
wire str1_str0;
wire str1_pl0;
wire str1_we0;

assign (strong0, strong1) str1_sup0 = 1'b1;
assign (supply0, supply1) str1_sup0 = 1'b0;
assign (strong0, strong1) str1_str0 = 1'b1;
assign (strong0, strong1) str1_str0 = 1'b0;
assign (strong0, strong1) str1_pl0  = 1'b1;
assign (pull0,   pull1)   str1_pl0  = 1'b0;
assign (strong0, strong1) str1_we0  = 1'b1;
assign (weak0,   weak1)   str1_we0  = 1'b0;

initial begin
    #1;
    $display("str1_sup0 resulted in: %b", str1_sup0);
    $display("str1_str0 resulted in: %b", str1_str0);
    $display("str1_pl0  resulted in: %b", str1_pl0);
    $display("str1_we0  resulted in: %b", str1_we0);
end

wire pl1_sup0;
wire pl1_str0;
wire pl1_pl0;
wire pl1_we0;

assign (pull0,   pull1)   pl1_sup0 = 1'b1;
assign (supply0, supply1) pl1_sup0 = 1'b0;
assign (pull0,   pull1)   pl1_str0 = 1'b1;
assign (strong0, strong1) pl1_str0 = 1'b0;
assign (pull0,   pull1)   pl1_pl0  = 1'b1;
assign (pull0,   pull1)   pl1_pl0  = 1'b0;
assign (pull0,   pull1)   pl1_we0  = 1'b1;
assign (weak0,   weak1)   pl1_we0  = 1'b0;

initial begin
    #1;
    $display("pl1_sup0 resulted in: %b", pl1_sup0);
    $display("pl1_str0 resulted in: %b", pl1_str0);
    $display("pl1_pl0  resulted in: %b", pl1_pl0);
    $display("pl1_we0  resulted in: %b", pl1_we0);
end

wire we1_sup0;
wire we1_str0;
wire we1_pl0;
wire we1_we0;

assign (weak0,   weak1)   we1_sup0 = 1'b1;
assign (supply0, supply1) we1_sup0 = 1'b0;
assign (weak0,   weak1)   we1_str0 = 1'b1;
assign (strong0, strong1) we1_str0 = 1'b0;
assign (weak0,   weak1)   we1_pl0  = 1'b1;
assign (pull0,   pull1)   we1_pl0  = 1'b0;
assign (weak0,   weak1)   we1_we0  = 1'b1;
assign (weak0,   weak1)   we1_we0  = 1'b0;

initial begin
    #1;
    $display("we1_sup0 resulted in: %b", we1_sup0);
    $display("we1_str0 resulted in: %b", we1_str0);
    $display("we1_pl0  resulted in: %b", we1_pl0);
    $display("we1_we0  resulted in: %b", we1_we0);
end

endmodule
