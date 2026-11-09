.data
   n : 0
   v : 0, 0, 0

.text
_PRINCIPAL:
   LD $in_port
   STO n
   LDI 0
   STO $indr
   LD $in_port
   STOV v
   LD n
   STO $indr
   LD $in_port
   STOV v
   LD n
   SUBI 1
   STO $indr
   LD $in_port
   STOV v
   LD n
   STO $out_port
   LDI 0
   STO $indr
   LDV v
   STO $out_port
   LDI 1
   STO $indr
   LDV v
   STO $out_port
   LDI 2
   STO $indr
   LDV v
   STO $out_port
   HLT 0
