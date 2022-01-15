// This will not generate a RE if these are calling the correct warning.
module top;
  integer res;

  initial begin
    // $countdrivers is now implemented
    res = $getpattern;
    $input;
    $key;
    $nokey;
    $list;
    $log;
    $nolog;
    $save;
    $restart;
    $incsave;
    res = $scale;
    $scope;
    $showscopes;
    $showvars;
    $sreadmemb;
    $sreadmemh;
    $display("PASSED");
  end
endmodule
