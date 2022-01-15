module top;
  reg [7:0] array [7:0];

  initial begin
    $readmemb();
    $readmemb(top);
    $readmemb("ivltests/readmemb.txt");
    $readmemb("ivltests/readmemb.txt", top);
    $readmemb("ivltests/readmemb.txt", array, top);
    $readmemb("ivltests/readmemb.txt", array, 0, top);
    $readmemb("ivltests/readmemb.txt", array, 0, 7, top);

    $readmemh();
    $readmemh(top);
    $readmemh("ivltests/readmemh.txt");
    $readmemh("ivltests/readmemh.txt", top);
    $readmemh("ivltests/readmemh.txt", array, top);
    $readmemh("ivltests/readmemh.txt", array, 0, top);
    $readmemh("ivltests/readmemh.txt", array, 0, 7, top);
  end
endmodule
