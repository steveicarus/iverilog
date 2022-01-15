module top;

function automatic [2:1] f1(input [3:0] i);
  begin
    f1[0] = i[0];
    f1[1] = i[1];
    f1[2] = i[2];
    f1[3] = i[3];
  end
endfunction

function automatic [3:0] f2(input [2:1] i);
  begin
    f2[0] = i[0];
    f2[1] = i[1];
    f2[2] = i[2];
    f2[3] = i[3];
  end
endfunction

function automatic [2:1] f3(input [3:0] i);
  begin
    f3[1:0] = i[1:0];
    f3[3:2] = i[3:2];
  end
endfunction

function automatic [3:0] f4(input [2:1] i);
  begin
    f4[1:0] = i[1:0];
    f4[3:2] = i[3:2];
  end
endfunction

function automatic [2:1] f5(input [3:0] i);
  reg [2:1] tmp;
  begin
    tmp[3:0]  = 4'b0000;
    tmp[3:0] |= i[3:0];
    f5 = tmp;
  end
endfunction

function automatic [3:0] f6(input [2:1] i);
  reg [3:0] tmp;
  begin
    tmp[3:0]  = 4'b0000;
    tmp[3:0] |= i[3:0];
    f6 = tmp;
  end
endfunction

localparam C1 = f1(4'b0011);
localparam C2 = f2(2'b01);
localparam C3 = f3(4'b0011);
localparam C4 = f4(2'b01);
localparam C5 = f5(4'b0011);
localparam C6 = f6(2'b01);

initial begin
  $display("C1 %b", C1);
  $display("C2 %b", C2);
  $display("C3 %b", C3);
  $display("C4 %b", C4);
  $display("C5 %b", C5);
  $display("C6 %b", C6);
end

endmodule
