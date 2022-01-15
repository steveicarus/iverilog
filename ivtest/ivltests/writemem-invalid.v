module top;
  reg [7:0] array [7:0];

  initial begin
    $writememb();
    $writememb(top);
    $writememb("writemem.txt");
    $writememb("writemem.txt", top);
    $writememb("writemem.txt", array, top);
    $writememb("writemem.txt", array, 0, top);
    $writememb("writemem.txt", array, 0, 7, top);

    $writememh();
    $writememh(top);
    $writememh("writemem.txt");
    $writememh("writemem.txt", top);
    $writememh("writemem.txt", array, top);
    $writememh("writemem.txt", array, 0, top);
    $writememh("writemem.txt", array, 0, 7, top);
  end
endmodule
