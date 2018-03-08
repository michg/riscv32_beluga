BINUTILSDIR=../../riscvbin/bin
BUILDDIR=../../build

rm -f -r result
mkdir result
for name in src/*/; do
	name=${name%/}
	name=${name#src/}
	SRCDIR=src/${name}
	RESDIR=result/${name}
	mkdir ${RESDIR}
	${BUILDDIR}/beluga -I. --target=riscv32 ${SRCDIR}/${name}.c -o ${RESDIR}/${name}.s
	${BUILDDIR}/beluga --target=riscv32 io.c -o ${RESDIR}/io.s
	${BINUTILSDIR}/as -o ${RESDIR}/start.o start.s
	${BINUTILSDIR}/as -o ${RESDIR}/io.o ${RESDIR}/io.s
	${BINUTILSDIR}/as -o ${RESDIR}/${name}.o ${RESDIR}/${name}.s
	${BINUTILSDIR}/ld -h -o ${RESDIR}/${name}.bin ${RESDIR}/start.o ${RESDIR}/io.o ${RESDIR}/${name}.o 
	python3 mkhex.py ${RESDIR}/${name}
	cp ${RESDIR}/${name}.hex firmware.mem
	./simv >> ${RESDIR}/${name}.log
	if cmp -s "${SRCDIR}/${name}.ref" "${RESDIR}/${name}.log"
	then
		echo "Testcase ${name} ok." >>result/results.log
	else
		echo "Testcase ${name} fail." >>result/results.log
	fi 
done



