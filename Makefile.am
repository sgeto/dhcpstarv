bin_PROGRAMS      = dhcpstarv

dhcpstarv_SOURCES = src/debug.c src/dhcp.c \
                    src/ether.c src/ip.c src/leases.c \
                    src/log.c src/main.c \
                    src/request.c src/sock.c \
                    src/udp.c src/utils.c

man_MANS          = doc/dhcpstarv.1

EXTRA_DIST        = autogen.sh src/common.h src/config.h \
                    src/debug.h src/dhcp.h src/ether.h \
                    src/ip.h src/leases.h src/log.h \
                    src/main.h src/request.h src/sock.h \
                    src/udp.h src/utils.h \
                    doc/rfc doc/dhcpstarv.Pod \
                    doc/pods2mans doc/dhcpstarv.1

# (GNU make only) Unless we say otherwise, spare us the
# “Entering/Leaving directory ...” messages
GNUMAKEFLAGS      = $(if $(value VERBOSE),,--no-print-directory)
