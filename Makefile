#
# Makefile for buliding all
#

SHELL = /bin/sh

RM = @rm -f

BLDDIR = ./build
BLGDIR = ./src
DRVDIR = ./bcc
BUDIR = ./riscvbin
B = $(BLDDIR)
C = $(BLGDIR)
R = $(DRVDIR)
U = $(BUDIR)


COMPILER = $B/beluga $B/conf.lst
DRIVER = $B/bcc $B/xfloat.o


all: $(COMPILER) $(DRIVER) BU

clean:
	cd $C && $(MAKE) clean
	cd $R && $(MAKE) clean
	cd $U && $(MAKE) clean
	$(RM) -f $(COMPILER) $(DRIVER)

test:
	cd $C && $(MAKE) test

$(COMPILER):
	cd $C && $(MAKE) all

$(DRIVER):
	cd $R && $(MAKE) all

BU:
	cd $U && $(MAKE) install

# end of Makefile
