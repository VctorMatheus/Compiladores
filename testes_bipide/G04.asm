.data
   a : 0
   b : 0
   c : 0
   v : 0, 0, 0

.text
_PRINCIPAL:
   LD $in_port
   STO a
   LD $in_port
   STO b
   LD $in_port
   STO c
   LDI 0
   STO $indr
   LD a
   STOV v
   LDI 1
   STO $indr
   LD b
   STOV v
   LDI 2
   STO $indr
   LD c
   STOV v
   LD a
   ADD b
   SUB c
   STO $out_port
   LD b
   SUB c
   STO 1000
   LD a
   SUB 1000
   STO $out_port
   LD a
   SUB b
   STO 1000
   LD c
   SUB a
   STO 1001
   LD 1000
   SUB 1001
   STO $out_port
   LDI 0
   STO $indr
   LDV v
   STO 1000
   LDI 1
   STO $indr
   LDV v
   STO 1001
   LDI 2
   STO $indr
   LDV v
   ADD 1001
   STO 1001
   LD 1000
   SUB 1001
   STO 1000
   LDI 2
   STO $indr
   LDV v
   STO 1001
   LDI 0
   STO $indr
   LDV v
   STO 1002
   LD 1001
   SUB 1002
   STO 1001
   LD 1000
   SUB 1001
   STO $out_port
   LDI 1
   SUB a
   STO $out_port
   HLT 0
