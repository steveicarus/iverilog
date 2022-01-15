module test;

  reg [7:0] bus;
  reg [7:0] skewed_bus;

  integer delay0;  initial delay0 =  4;
  integer delay1;  initial delay1 =  8;
  integer delay2;  initial delay2 = 12;
  integer delay3;  initial delay3 = 16;
  integer delay4;  initial delay4 = 20;
  integer delay5;  initial delay5 = 24;
  integer delay6;  initial delay6 = 28;
  integer delay7;  initial delay7 = 32;

  /* model skew across the bus using transport delays */

  always @( bus[0] )
    begin
      skewed_bus[0] <= #delay0 bus[0];
    end

   always @( bus[1] )
    begin
      skewed_bus[1] <= #delay1 bus[1];
    end

  always @( bus[2] )
    begin
      skewed_bus[2] <= #delay2 bus[2];
    end

   always @( bus[3] )
    begin
      skewed_bus[3] <= #delay3 bus[3];
    end

  always @( bus[4] )
    begin
      skewed_bus[4] <= #delay4 bus[4];
    end

  always @( bus[5] )
    begin
      skewed_bus[5] <= #delay5 bus[5];
    end

  always @( bus[6] )
    begin
      skewed_bus[6] <= #delay6 bus[6];
    end

  always @( bus[7] )
    begin
      skewed_bus[7] <= #delay7 bus[7];
    end


   initial
     begin
       bus = {8{1'b0}};
       #4;
       bus = 8'b00100100;
       #4;
       bus = 8'b10000001;
       #4;
       bus = 8'b00001001;
       #4;
       bus = 8'b01100011;
       #4;
       bus = 8'b00001101;
       #4;
       bus = 8'b10001101;
       #4;
       bus = 8'b01100101;
       #4;
       bus = 8'b00010010;
       #4;
       bus = 8'b00000001;
       #4;
       bus = 8'b00001101;
       #4;
       bus = 8'b01110110;
       #4;
       bus = 8'b00111101;
       #4;
       bus = 8'b11101101;
       #4;
       bus = 8'b10001100;
       #4;
       bus = 8'b11111001;
       #4;
       bus = 8'b11000110;
       #4;
       bus = 8'b11000101;
       #4;
       bus = 8'b10101010;
       #4;
       bus = 8'b11100101;
       #4;
       bus = 8'b01110111;
       #4;
       bus = 8'b00010010;
       #4;
       bus = 8'b10001111;
       #4;
       bus = 8'b11110010;
       #4;
       bus = 8'b11001110;
       #4;
       bus = 8'b11101000;
       #4;
       bus = 8'b11000101;
       #4;
       bus = 8'b01011100;
       #4;
       bus = 8'b10111101;
       #4;
       bus = 8'b00101101;
       #4;
       bus = 8'b01100101;
       #4;
       bus = 8'b01100011;
       #4;
       bus = 8'b00001010;
       #4;
       bus = 8'b10000000;
       #4;
       bus = 8'b00100000;
       #4;
       bus = 8'b10101010;
       #4;
       bus = 8'b10011101;
       #4;
       bus = 8'b10010110;
       #4;
       bus = 8'b00010011;
       #4;
       bus = 8'b00001101;
       #4;
       bus = 8'b01010011;
       #4;
       bus = 8'b01101011;
       #4;
       bus = 8'b11010101;
       #4;
       bus = 8'b00000010;
       #4;
       bus = 8'b10101110;
       #4;
       bus = 8'b00011101;
       #4;
       bus = 8'b11001111;
       #4;
       bus = 8'b00100011;
       #4;
       bus = 8'b00001010;
       #4;
       bus = 8'b11001010;
       #4;
       bus = 8'b00111100;
       #4;
       bus = 8'b11110010;
       #4;
       bus = 8'b10001010;
       #4;
       bus = 8'b01000001;
       #4;
       bus = 8'b11011000;
       #4;
       bus = 8'b01111000;
       #4;
       bus = 8'b10001001;
       #4;
       bus = 8'b11101011;
       #4;
       bus = 8'b10110110;
       #4;
       bus = 8'b11000110;
       #4;
       bus = 8'b10101110;
       #4;
       bus = 8'b10111100;
       #4;
       bus = 8'b00101010;
       #4;
       bus = 8'b00001011;
       #4;
       bus = 8'b01110001;
       #4;
       bus = 8'b10000101;
       #4;
       bus = 8'b01001111;
       #4;
       bus = 8'b00111011;
       #4;
       bus = 8'b00111010;
       #4;
       bus = 8'b01111110;
       #4;
       bus = 8'b00010101;
       #4;
       bus = 8'b11110001;
       #4;
       bus = 8'b11011001;
       #4;
       bus = 8'b01100010;
       #4;
       bus = 8'b01001100;
       #4;
       bus = 8'b10011111;
       #4;
       bus = 8'b10001111;
       #4;
       bus = 8'b11111000;
       #4;
       bus = 8'b10110111;
       #4;
       bus = 8'b10011111;
       #4;
       bus = 8'b01011100;
       #4;
       bus = 8'b01011011;
       #4;
       bus = 8'b10001001;
       #4;
       bus = 8'b01001001;
       #4;
       bus = 8'b11010000;
       #4;
       bus = 8'b11010111;
       #4;
       bus = 8'b01010001;
       #4;
       bus = 8'b10010110;
       #4;
       bus = 8'b00001100;
       #4;
       bus = 8'b11000010;
       #4;
       bus = 8'b11001000;
       #4;
       bus = 8'b01110111;
       #4;
       bus = 8'b00111101;
       #4;
       bus = 8'b00010010;
       #4;
       bus = 8'b01111110;
       #4;
       bus = 8'b01101101;
       #4;
       bus = 8'b00111001;
       #4;
       bus = 8'b00011111;
       #4;
       bus = 8'b11010011;
       #4;
       bus = 8'b10000101;
       #4;
       bus = 8'b01111000;
       #4;
       bus = 8'b01011011;
       #4;
       bus = 8'b01001001;
       #4;
       bus = 8'b00111111;
       #4;
       bus = 8'b00101010;
       #4;
       bus = 8'b01011000;
       #4;
       bus = 8'b10000110;
       #4;
       bus = 8'b10001110;
       #4;
       bus = 8'b10011100;
       #4;
       bus = 8'b11111010;
       #4;
       bus = 8'b00100110;
       #4;
       bus = 8'b01110011;
       #4;
       bus = 8'b10100011;
       #4;
       bus = 8'b00101111;
       #4;
       bus = 8'b10110011;
       #4;
       bus = 8'b01011111;
       #4;
       bus = 8'b01000100;
       #4;
       bus = 8'b11110111;
       #4;
       bus = 8'b11001011;
       #4;
       bus = 8'b11100110;
       #4;
       bus = 8'b01011010;
       #4;
       bus = 8'b00101001;
       #4;
       bus = 8'b11101101;
       #4;
       bus = 8'b11011010;
       #4;
       bus = 8'b01100101;
       #4;
       bus = 8'b10110101;
       #4;
       bus = 8'b11011111;
       #4;
       bus = 8'b01111001;
       #4;
       bus = 8'b01000100;
     end

   initial
     begin
       #2;
       if (skewed_bus !== 8'bxxxxxxxx)
         begin
           $write("FAILED -- expected xxxxxxxx  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'bxxxxxxx0)
         begin
           $write("FAILED -- expected xxxxxxx0  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'bxxxxxx00)
         begin
           $write("FAILED -- expected xxxxxx00  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'bxxxxx001)
         begin
           $write("FAILED -- expected xxxxx001  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'bxxxx0101)
         begin
           $write("FAILED -- expected xxxx0101  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'bxxx00001)
         begin
           $write("FAILED -- expected xxx00001  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'bxx000011)
         begin
           $write("FAILED -- expected xx000011  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'bx0101001)
         begin
           $write("FAILED -- expected x0101001  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00000101)
         begin
           $write("FAILED -- expected 00000101  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00001100)
         begin
           $write("FAILED -- expected 00001100  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10101111)
         begin
           $write("FAILED -- expected 10101111  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01000001)
         begin
           $write("FAILED -- expected 01000001  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00000000)
         begin
           $write("FAILED -- expected 00000000  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00110111)
         begin
           $write("FAILED -- expected 00110111  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11001101)
         begin
           $write("FAILED -- expected 11001101  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00000100)
         begin
           $write("FAILED -- expected 00000100  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00011101)
         begin
           $write("FAILED -- expected 00011101  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00111100)
         begin
           $write("FAILED -- expected 00111100  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01101011)
         begin
           $write("FAILED -- expected 01101011  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00101100)
         begin
           $write("FAILED -- expected 00101100  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01010111)
         begin
           $write("FAILED -- expected 01010111  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10100001)
         begin
           $write("FAILED -- expected 10100001  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11001110)
         begin
           $write("FAILED -- expected 11001110  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11000111)
         begin
           $write("FAILED -- expected 11000111  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11100010)
         begin
           $write("FAILED -- expected 11100010  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10110110)
         begin
           $write("FAILED -- expected 10110110  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11111010)
         begin
           $write("FAILED -- expected 11111010  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11000101)
         begin
           $write("FAILED -- expected 11000101  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00011000)
         begin
           $write("FAILED -- expected 00011000  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00101101)
         begin
           $write("FAILED -- expected 00101101  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11000101)
         begin
           $write("FAILED -- expected 11000101  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11101101)
         begin
           $write("FAILED -- expected 11101101  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11011101)
         begin
           $write("FAILED -- expected 11011101  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11011110)
         begin
           $write("FAILED -- expected 11011110  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11100010)
         begin
           $write("FAILED -- expected 11100010  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00100000)
         begin
           $write("FAILED -- expected 00100000  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10101000)
         begin
           $write("FAILED -- expected 10101000  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01100011)
         begin
           $write("FAILED -- expected 01100011  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01000000)
         begin
           $write("FAILED -- expected 01000000  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00001111)
         begin
           $write("FAILED -- expected 00001111  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00101111)
         begin
           $write("FAILED -- expected 00101111  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10110001)
         begin
           $write("FAILED -- expected 10110001  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00010111)
         begin
           $write("FAILED -- expected 00010111  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10011011)
         begin
           $write("FAILED -- expected 10011011  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10000000)
         begin
           $write("FAILED -- expected 10000000  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10011110)
         begin
           $write("FAILED -- expected 10011110  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00000011)
         begin
           $write("FAILED -- expected 00000011  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01110101)
         begin
           $write("FAILED -- expected 01110101  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01001111)
         begin
           $write("FAILED -- expected 01001111  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01001110)
         begin
           $write("FAILED -- expected 01001110  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10111010)
         begin
           $write("FAILED -- expected 10111010  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00000010)
         begin
           $write("FAILED -- expected 00000010  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10001000)
         begin
           $write("FAILED -- expected 10001000  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01101110)
         begin
           $write("FAILED -- expected 01101110  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10001011)
         begin
           $write("FAILED -- expected 10001011  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00010000)
         begin
           $write("FAILED -- expected 00010000  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01111000)
         begin
           $write("FAILED -- expected 01111000  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10100001)
         begin
           $write("FAILED -- expected 10100001  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01001001)
         begin
           $write("FAILED -- expected 01001001  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10011010)
         begin
           $write("FAILED -- expected 10011010  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11011010)
         begin
           $write("FAILED -- expected 11011010  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01101110)
         begin
           $write("FAILED -- expected 01101110  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11000110)
         begin
           $write("FAILED -- expected 11000110  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00110100)
         begin
           $write("FAILED -- expected 00110100  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11101111)
         begin
           $write("FAILED -- expected 11101111  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10001011)
         begin
           $write("FAILED -- expected 10001011  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11111001)
         begin
           $write("FAILED -- expected 11111001  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10101001)
         begin
           $write("FAILED -- expected 10101001  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10100111)
         begin
           $write("FAILED -- expected 10100111  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10010110)
         begin
           $write("FAILED -- expected 10010110  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00101010)
         begin
           $write("FAILED -- expected 00101010  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01001011)
         begin
           $write("FAILED -- expected 01001011  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00011101)
         begin
           $write("FAILED -- expected 00011101  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11111101)
         begin
           $write("FAILED -- expected 11111101  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00110000)
         begin
           $write("FAILED -- expected 00110000  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00110010)
         begin
           $write("FAILED -- expected 00110010  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01011001)
         begin
           $write("FAILED -- expected 01011001  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00110111)
         begin
           $write("FAILED -- expected 00110111  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01001110)
         begin
           $write("FAILED -- expected 01001110  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11101101)
         begin
           $write("FAILED -- expected 11101101  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11011011)
         begin
           $write("FAILED -- expected 11011011  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01001110)
         begin
           $write("FAILED -- expected 01001110  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00010101)
         begin
           $write("FAILED -- expected 00010101  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10111111)
         begin
           $write("FAILED -- expected 10111111  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11111001)
         begin
           $write("FAILED -- expected 11111001  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10011000)
         begin
           $write("FAILED -- expected 10011000  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10011001)
         begin
           $write("FAILED -- expected 10011001  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11001011)
         begin
           $write("FAILED -- expected 11001011  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01000100)
         begin
           $write("FAILED -- expected 01000100  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00010010)
         begin
           $write("FAILED -- expected 00010010  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11010100)
         begin
           $write("FAILED -- expected 11010100  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01010110)
         begin
           $write("FAILED -- expected 01010110  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11011001)
         begin
           $write("FAILED -- expected 11011001  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11000011)
         begin
           $write("FAILED -- expected 11000011  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00001100)
         begin
           $write("FAILED -- expected 00001100  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10000110)
         begin
           $write("FAILED -- expected 10000110  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01011011)
         begin
           $write("FAILED -- expected 01011011  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11110101)
         begin
           $write("FAILED -- expected 11110101  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11111101)
         begin
           $write("FAILED -- expected 11111101  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00011011)
         begin
           $write("FAILED -- expected 00011011  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00101111)
         begin
           $write("FAILED -- expected 00101111  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01111000)
         begin
           $write("FAILED -- expected 01111000  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01110101)
         begin
           $write("FAILED -- expected 01110101  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00010011)
         begin
           $write("FAILED -- expected 00010011  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00001001)
         begin
           $write("FAILED -- expected 00001001  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01011010)
         begin
           $write("FAILED -- expected 01011010  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10111110)
         begin
           $write("FAILED -- expected 10111110  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11001000)
         begin
           $write("FAILED -- expected 11001000  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01011010)
         begin
           $write("FAILED -- expected 01011010  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01101110)
         begin
           $write("FAILED -- expected 01101110  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00110100)
         begin
           $write("FAILED -- expected 00110100  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00001110)
         begin
           $write("FAILED -- expected 00001110  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01001011)
         begin
           $write("FAILED -- expected 01001011  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00011111)
         begin
           $write("FAILED -- expected 00011111  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10010011)
         begin
           $write("FAILED -- expected 10010011  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10100011)
         begin
           $write("FAILED -- expected 10100011  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11110111)
         begin
           $write("FAILED -- expected 11110111  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10101010)
         begin
           $write("FAILED -- expected 10101010  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01100101)
         begin
           $write("FAILED -- expected 01100101  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00111111)
         begin
           $write("FAILED -- expected 00111111  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b10110110)
         begin
           $write("FAILED -- expected 10110110  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b00000010)
         begin
           $write("FAILED -- expected 00000010  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11011111)
         begin
           $write("FAILED -- expected 11011111  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01100001)
         begin
           $write("FAILED -- expected 01100001  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b01001000)
         begin
           $write("FAILED -- expected 01001000  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11111111)
         begin
           $write("FAILED -- expected 11111111  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       #4;
       if (skewed_bus !== 8'b11001001)
         begin
           $write("FAILED -- expected 11001001  ");
           $display("received %b  ", skewed_bus);
           $finish;
         end
       $display("PASSED");
     end

endmodule
