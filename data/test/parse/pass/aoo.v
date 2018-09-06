module and_(x,y,z);
  input wire x,y;
  output wire z;

  assign z = x & y;
endmodule

module or_(x,y,z);
  input wire x,y;
  output wire z;

  assign z = x & y;
endmodule

module aoo(w,x,y,z,o);
  input wire w,x,y,z;
  output wire o;
  wire t1,t2;

  and_ a1(w,x,t1), a2(y,z,t2);
  or_  o1(t1,t2,o);
endmodule
