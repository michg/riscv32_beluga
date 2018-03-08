BELUGADIR=../../../build
BINUTILSDIR=../../../riscvbin/bin


${BELUGADIR}/beluga  -I. --target=riscv32 sdmm.c -o sdmm.s
${BELUGADIR}/beluga  -I. --target=riscv32 ff.c -o ff.s
${BELUGADIR}/beluga  -I. --target=riscv32 loader.c -o loader.s
${BELUGADIR}/beluga  -I. --target=riscv32 xprintf.c -o xprintf.s
${BINUTILSDIR}/as -o start.o start.s
${BINUTILSDIR}/as -g -o sdmm.o sdmm.s
${BINUTILSDIR}/as -g -o ff.o ff.s
${BINUTILSDIR}/as -g -o loader.o loader.s
${BINUTILSDIR}/as -g -o xprintf.o xprintf.s
${BINUTILSDIR}/ld -g -h -m firmware.mem -o firmware.bin start.o ff.o sdmm.o xprintf.o loader.o
python3 mkhex.py firmware






