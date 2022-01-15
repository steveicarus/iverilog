module main;

   typedef struct packed {
      logic [7:0] high;
      logic [7:0] low;
   } word;

   word [2] array;      // word[0:1] exposes the bug as well
   word single;

   initial begin
       array[0].high = "a";
       array[0].low = "b";
       array[1].high = "c";
       array[1].low = "d";

       $display("%s", array[0]);        // good
       $display("%s %s", array[0].high, array[0].low);

       $display("%s", array[1]);        // good
       // the line below displays contents of array[0] instead of array[1]
       $display("%s %s", array[1].high, array[1].low);

       // below everything is fine
       single = array[0];
       $display("%s", single);
       $display("%s %s", single.high, single.low);
   end
endmodule
