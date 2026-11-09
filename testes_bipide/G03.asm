.data
   a : 0
   b : 0
   v : 0, 0, 0, 0

.text
_PRINCIPAL:
   LDI 7
   STO a
   LD a
   STO b
   LDI 0
   STO $indr
   LD b
   STOV v
   LD a
   SUBI 6
   STO 1000
   LD 1000
   STO $indr
   LDI 300
   STOV v
   LDI 1
   STO $indr
   LDV v
   SUBI 298
   STO 1000
   LDI 0
   STO $indr
   LDV v
   STO 1001
   LDI 1
   STO $indr
   LDV v
   ADD 1001
   STO 1001
   LD 1000
   STO $indr
   LD 1001
   STOV v
   LDI 3
   STO $indr
   LDI -5
   STOV v
   LD a
   STO $out_port
   LD b
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
   LDI 3
   STO $indr
   LDV v
   STO $out_port
   HLT 0
