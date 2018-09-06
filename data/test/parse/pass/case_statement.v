module cases();
  always begin
    case (cond) 
      1: ;
      default ;
    endcase
    casex (cond) 
      1,
      2: ;
      default: ;
    endcase
    casez (cond)
      1,
      2: x = 1;
      default y = 2;
    endcase
  end
endmodule
