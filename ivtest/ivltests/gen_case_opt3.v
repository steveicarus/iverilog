module top;
  parameter NAME = "skip";
  wire i = 0;

  generate
    case(NAME)
      "test" : assign i = 1'b1;
      "skip" : ;
      default : ;
    endcase
  endgenerate

  initial begin
    #1 if (i !== 1'b0) $display("FAILED");
       else $display("PASSED");
  end
endmodule
