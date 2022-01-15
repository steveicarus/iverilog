/*
 * This is the crux of PR487.
 */

module  test();

parameter[1:4]    async_wrport = 4'b1100;
reg           async_wri;
reg[1:4]      async_i;

initial   begin
   for(async_i=1;async_i<=4;async_i=async_i+1)  begin
      async_wri=async_wrport[async_i];
      $display("async_wrport[%d] --> %b", async_i, async_wri);
   end
end
endmodule
