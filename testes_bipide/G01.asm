.data
   a : 0
   v : 0, 0, 0, 0
   c : 0
   l : 0

.text
_PRINCIPAL:
   LDI 55
   STO $out_port
   LD a
   STO $out_port
   LDI 3
   STO $indr
   LDV v
   STO $out_port
   LDI 31
   STO $out_port
   LDI 5
   STO $out_port
   LDI 90
   STO $out_port
   LDI 1
   STO $out_port
   LDI 0
   STO $out_port
   HLT 0
