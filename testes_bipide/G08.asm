.data
   a : 0
   v : 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

.text
_PRINCIPAL:
   LDI 12
   STO a
   LDI 9
   STO $indr
   LDI 10
   STOV v
   LD a
   STO $out_port
   LDI 3
   STO $out_port
   LDI 3
   STO $out_port
   LDI 1
   STO $out_port
   LDI 9
   STO $indr
   LDV v
   STO $out_port
   HLT 0
