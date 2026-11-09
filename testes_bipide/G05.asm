.data
   a : 0
   b : 0
   v : 0, 0

.text
_PRINCIPAL:
   LD $in_port
   STO a
   LD $in_port
   STO b
   LDI 1
   STO $indr
   LD a
   STOV v
   LD a
   AND b
   STO $out_port
   LD a
   OR b
   STO $out_port
   LD a
   XOR b
   STO $out_port
   LD a
   NOT 0
   STO $out_port
   LDI 1
   STO $indr
   LDV v
   ANDI 15
   STO 1000
   LD b
   SLL 3
   OR 1000
   STO $out_port
   LD a
   SRL 1
   STO $out_port
   LDI 1
   STO $indr
   LDV v
   STO 1000
   LDI 0
   SUB 1000
   STO $out_port
   HLT 0
