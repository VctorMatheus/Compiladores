.data
   a : 0
   const_n25536 : -25536

.text
_PRINCIPAL:
   LD const_n25536
   STO a
   LD a
   STO $out_port
   HLT 0
