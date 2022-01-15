/*
 */

module main();

   reg [8:0] foo;

   reg [1:0] bar;
   initial begin
      foo = 2'b00 ? 9'b000111xxx : 9'b01x01x01x;
      $display("00: foo = %b", foo);

      foo = 2'b01 ? 9'b000111xxx : 9'b01x01x01x;
      $display("01: foo = %b", foo);

      foo = 2'b0x ? 9'b000111xxx : 9'b01x01x01x;
      $display("0x: foo = %b", foo);

      foo = 2'b11 ? 9'b000111xxx : 9'b01x01x01x;
      $display("11: foo = %b", foo);

      foo = 2'b1x ? 9'b000111xxx : 9'b01x01x01x;
      $display("1x: foo = %b", foo);

      bar = 2'b00;
      foo = bar? 9'b000111xxx : 9'b01x01x01x;
      $display("%b: foo = %b", bar, foo);

      bar = 2'b01;
      foo = bar? 9'b000111xxx : 9'b01x01x01x;
      $display("%b: foo = %b", bar, foo);

      bar = 2'b0x;
      foo = bar? 9'b000111xxx : 9'b01x01x01x;
      $display("%b: foo = %b", bar, foo);

      bar = 2'b11;
      foo = bar? 9'b000111xxx : 9'b01x01x01x;
      $display("%b: foo = %b", bar, foo);

      bar = 2'b1x;
      foo = bar? 9'b000111xxx : 9'b01x01x01x;
      $display("%b: foo = %b", bar, foo);

   end

endmodule
