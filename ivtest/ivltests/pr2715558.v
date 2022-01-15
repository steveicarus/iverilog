// NOTE: This test program is WRONG, in that it ignores the fact
// that continuous assigns drive with their own strength and drop
// any strength that the r-value may have.

module strength();

wire sup1; assign (supply0, supply1) sup1 = 1'b1;
wire str1; assign (strong0, strong1) str1 = 1'b1;
wire pl1;  assign (pull0, pull1) pl1 = 1'b1;
wire we1;  assign (weak0, weak1) we1 = 1'b1;

wire sup0; assign (supply0, supply1) sup0 = 1'b0;
wire str0; assign (strong0, strong1) str0 = 1'b0;
wire pl0;  assign (pull0, pull1) pl0 = 1'b0;
wire we0;  assign (weak0, weak1) we0 = 1'b0;

wire sup1_sup0;
wire sup1_str0;
wire sup1_pl0;
wire sup1_we0;

assign sup1_sup0 = sup1;
assign sup1_sup0 = sup0;
assign sup1_str0 = sup1;
assign sup1_str0 = str0;
assign sup1_pl0 = sup1;
assign sup1_pl0 = pl0;
assign sup1_we0 = sup1;
assign sup1_we0 = we0;

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
assign str1_sup0 = str1;
assign str1_sup0 = sup0;
assign str1_str0 = str1;
assign str1_str0 = str0;
assign str1_pl0 = str1;
assign str1_pl0 = pl0;
assign str1_we0 = str1;
assign str1_we0 = we0;

initial begin
    #2;
    $display("str1_sup0 resulted in: %b", str1_sup0);
    $display("str1_str0 resulted in: %b", str1_str0);
    $display("str1_pl0  resulted in: %b", str1_pl0);
    $display("str1_we0  resulted in: %b", str1_we0);
end

wire pl1_sup0;
wire pl1_str0;
wire pl1_pl0;
wire pl1_we0;
assign pl1_sup0 = pl1;
assign pl1_sup0 = sup0;
assign pl1_str0 = pl1;
assign pl1_str0 = str0;
assign pl1_pl0 = pl1;
assign pl1_pl0 = pl0;
assign pl1_we0 = pl1;
assign pl1_we0 = we0;

initial begin
    #3;
    $display("pl1_sup0 resulted in: %b", pl1_sup0);
    $display("pl1_str0 resulted in: %b", pl1_str0);
    $display("pl1_pl0  resulted in: %b", pl1_pl0);
    $display("pl1_we0  resulted in: %b", pl1_we0);
end

wire we1_sup0;
wire we1_str0;
wire we1_pl0;
wire we1_we0;
assign we1_sup0 = we1;
assign we1_sup0 = sup0;
assign we1_str0 = we1;
assign we1_str0 = str0;
assign we1_pl0 = we1;
assign we1_pl0 = pl0;
assign we1_we0 = we1;
assign we1_we0 = we0;

initial begin
    #4;
    $display("we1_sup0 resulted in: %b", we1_sup0);
    $display("we1_str0 resulted in: %b", we1_str0);
    $display("we1_pl0  resulted in: %b", we1_pl0);
    $display("we1_we0  resulted in: %b", we1_we0);
end

endmodule
