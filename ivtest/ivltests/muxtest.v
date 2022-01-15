module test ;

wire a;
reg  sel,in0, in1;
reg  error;

assign a = sel ? in1 : in0 ;

initial
  begin
    error = 0;
    #1;
    sel = 0;
    in0 = 0;
    in1 = 0;
    #1;
    if(a !== 0)
       begin
         $display("FAILED - (1) Mux error sel=0, in0=in0=0 yet out != 0");
         $display("sel=%b,in0=%b,in1=%b,out=%b",
                   sel,in0,in1,a);
         error = 1;
       end
    #1;
    in0 = 1;
    #1;
    if(a !== 1)
       begin
         $display("FAILED - (2) Mux error sel=0, in0=1,in1=0 yet out != 1");
         $display("sel=%b,in0=%b,in1=%b,out=%b",
                   sel,in0,in1,a);
         error = 1;
       end

    #1;
    sel = 1;
    #1;
    if(a !== 0)
       begin
         $display("FAILED - (3) Mux error sel=1, in0=1,in1=0 yet out != 0");
         $display("sel=%b,in0=%b,in1=%b,out=%b",
                   sel,in0,in1,a);
         error = 1;
       end
    #1;
    in1 = 1;
    #1;
    if(a !== 1)
       begin
         $display("FAILED - (5) Mux error sel=1, in0=1,in1=1 yet out != 1");
         $display("sel=%b,in0=%b,in1=%b,out=%b",
                   sel,in0,in1,a);
         error = 1;
       end
    #1;
    in0 = 0;
    #1;
    if(a !== 1)
       begin
         $display("FAILED - (6) Mux error sel=1, in0=0,in1=1 yet out != 1");
         $display("sel=%b,in0=%b,in1=%b,out=%b",
                   sel,in0,in1,a);
         error = 1;
       end
   #1;
    in1 = 0;
    sel = 1'bx;
    #1;
    if(a !== 0)
       begin
         $display("FAILED - (8) Mux error sel=X, in0=0,in1=0 yet out != 0");
         $display("sel=%b,in0=%b,in1=%b,out=%b",
                   sel,in0,in1,a);
         error = 1;
       end
    #1;
    in0 = 1;
    in1 = 1;
    sel = 1'bx;
    #1;
    if(a !== 1)
       begin
         $display("FAILED - (9) Mux error sel=X, in0=1,in1=1 yet out != 1");
         $display("sel=%b,in0=%b,in1=%b,out=%b",
                   sel,in0,in1,a);
         error = 1;
       end
    if(error === 0)
      $display("PASSED");
  end

endmodule
