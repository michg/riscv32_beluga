BINUTILSDIR=../../riscvbin/bin
BUILDDIR=../../build
SRCDIR=./src/$1
RESDIR=./result/$1

rm -f -r result
mkdir result 
mkdir ${RESDIR}
${BUILDDIR}/beluga -I. --target=riscv32 ${SRCDIR}/$1.c -o ${RESDIR}/$1.s
${BUILDDIR}/beluga --target=riscv32 io.c -o ${RESDIR}/io.s
${BINUTILSDIR}/as -o ${RESDIR}/start.o start.s
${BINUTILSDIR}/as -o ${RESDIR}/io.o ${RESDIR}/io.s
${BINUTILSDIR}/as -o ${RESDIR}/$1.o ${RESDIR}/$1.s
${BINUTILSDIR}/ld -h -o ${RESDIR}/$1.bin ${RESDIR}/start.o ${RESDIR}/io.o ${RESDIR}/$1.o 
python3 mkhex.py ${RESDIR}/$1
cp ${RESDIR}/$1.hex firmware.mem
./simv >> ${RESDIR}/$1.log
if cmp -s "${SRCDIR}/$1.ref" "${RESDIR}/$1.log"
   then
      echo "Testcase $1 ok."
   else
      echo "Testcase $1 fail."
   fi 



