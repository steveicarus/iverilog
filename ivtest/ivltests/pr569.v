/*
 * Derived from PR#569
 */

module test();
     parameter foo = 8'b01010101;
     parameter bar = {foo,{2{foo}}}; // fails
     // parameter tmp = {2{foo}}; // this + next line succeed
     // parameter bar = {foo,tmp};
     reg[23:0] cnt;
     reg CLK;

     initial $monitor("%b", cnt);
     initial CLK = 0;
     initial cnt = bar;
endmodule
