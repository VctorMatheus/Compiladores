.data
   a : 0
   const_1502 : 1502
   const_3000 : 3000
   const_n32768 : -32768

.text
_PRINCIPAL:
   LD const_1502
   STO a
   LD a
   STO $out_port
   LD a
   SUB const_3000
   STO $out_port
   LD const_n32768
   STO $out_port
   HLT 0
