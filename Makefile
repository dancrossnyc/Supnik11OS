# Chopped down version of the SIMH 3.9 Makefile.
#
ifeq ($(GCC),)
  GCC = mips-none-elf-gcc
endif
OSTYPE = $(shell uname)
# OSNAME is used in messages to indicate the source of libpcap components
OSNAME = $(OSTYPE)
GCC_VERSION = $(shell $(GCC) -v /dev/null 2>&1 | grep 'gcc version' | awk '{ print $$3 }')
# OSX's XCode gcc doesn't support LTO, but gcc built to explicitly enable it will work
ifneq (binexists,$(shell if test -e bin; then echo binexists; fi))
  MKDIRBIN = if test ! -e bin; then mkdir bin; fi
endif
CFLAGS_O = -O2
CFLAGS_O += -finline-functions
CFLAGS_O += -fgcse-after-reload
CFLAGS_O += -fpredictive-commoning
CFLAGS_O += -fipa-cp-clone
CFLAGS_O += -fno-unsafe-loop-optimizations
CFLAGS_O += -fno-strict-overflow
CFLAGS_O += -flto -fwhole-program
CFLAGS_O += -Wno-unused-result
LDFLAGS_O = 
LDFLAGS_O += -flto -fwhole-program
ifneq (clean,$(MAKECMDGOALS))
  $(info ***)
  $(info *** PDP11 Simulator being built with:)
  $(info *** GCC Version: $(GCC_VERSION).)
  $(info ***)
endif

CC_STD = -std=c11
CC_SA = #-ffreestanding -nostdinc -nostdlib -I include
CC_OUTSPEC = -o $@
CC = $(GCC) $(CC_SA) $(CC_STD) $(CFLAGS_G) $(CFLAGS_O) -I .
LDFLAGS = $(OS_LDFLAGS) $(NETWORK_LDFLAGS) $(LDFLAGS_O)

#
# Common Libraries
#
BIN = bin
SIM = scp.c sim_console.c sim_fio.c sim_timer.c sim_sock.c sim_tmxr.c sim_tape.c

#
# Emulator source files and compile time options
#
# Unused files:
#
#	${PDP11D}/pdp11_cr.c ${PDP11D}/pdp11_rc.c ${PDP11D}/pdp11_vh.c \
#	${PDP11D}/pdp11_rf.c ${PDP11D}/pdp11_xq.c ${PDP11D}/pdp11_xu.c
#

PDP11D = PDP11
PDP11 = ${PDP11D}/pdp11_fp.c ${PDP11D}/pdp11_cpu.c ${PDP11D}/pdp11_dz.c \
	${PDP11D}/pdp11_cis.c ${PDP11D}/pdp11_lp.c ${PDP11D}/pdp11_rk.c \
	${PDP11D}/pdp11_rl.c ${PDP11D}/pdp11_rp.c ${PDP11D}/pdp11_rx.c \
	${PDP11D}/pdp11_stddev.c ${PDP11D}/pdp11_sys.c ${PDP11D}/pdp11_tc.c \
	${PDP11D}/pdp11_tm.c ${PDP11D}/pdp11_ts.c ${PDP11D}/pdp11_io.c \
	${PDP11D}/pdp11_rq.c ${PDP11D}/pdp11_tq.c ${PDP11D}/pdp11_pclk.c \
	${PDP11D}/pdp11_ry.c ${PDP11D}/pdp11_pt.c ${PDP11D}/pdp11_hk.c \
	${PDP11D}/pdp11_rh.c ${PDP11D}/pdp11_tu.c ${PDP11D}/pdp11_cpumod.c \
	${PDP11D}/pdp11_dl.c ${PDP11D}/pdp11_ta.c \
	${PDP11D}/pdp11_kg.c \
	${PDP11D}/pdp11_ke.c ${PDP11D}/pdp11_dc.c ${PDP11D}/pdp11_io_lib.c
PDP11_OPT = -DVM_PDP11 -I ${PDP11D} ${NETWORK_OPT}

#
# Build everything
#
ALL = pdp11

all : ${ALL}

clean :
	${RM} -r ${BIN}

#
# Individual builds
#
pdp11 : ${BIN}/pdp11

${BIN}/pdp11 : ${PDP11} ${SIM}
	${MKDIRBIN}
	${CC} ${PDP11} ${SIM} ${PDP11_OPT} $(CC_OUTSPEC) ${LDFLAGS}
