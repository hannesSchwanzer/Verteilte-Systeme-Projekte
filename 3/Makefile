# Compile with gcc
CC=gcc
# Command to auto-generate dependencies with gcc
DEPCC=$(CC) -MM $(CFLAGS)

# Additional flags to pass to gcc:
# -g : generate debug information so we can use gdb to debug
# -I <dir> : look in directory <dir> for additional .h files
CFLAGS=-g -Iinclude 
# all warnings + warnings are considered as errors
CFLAGS+= -Wall -Werror

# Linker options: link program against libhamster.a
# to be found in directory ../libsrc
LDLIBS=-Llibsrc -lhamster
LIB=libsrc/libhamster.a

DOC=doc

# compile all .c files found in this directory
FILES=$(shell find ./src/ -name '*.c')

BINARY_SERVER = hamster_server

OBJS=$(FILES:.c=.o)
DEPS=.depend

all: $(DOC) $(BINARY_SERVER)

-include $(DEPS)

$(BINARY_SERVER): $(OBJS) $(LIB)
	$(CC) $^ -o $@ $(LDLIBS) $(LDFLAGS)

$(LIB): libsrc
	$(MAKE) -C libsrc



$(DOC): $(FILES) include/hamsterlib.h include/hamsterprotocol.h
	doxygen include/doxyfile

$(DEPS):
	$(DEPCC) $(FILES) >$(DEPS)

test:
	chmod +x tests/hamster
	java -jar tests/TestRunnerHamster.jar -P . -H tests/
#
# Run our exhaustive, non-interactive testsuite: For this, package DejaGnu must be installed
#
NEEDED=runtest expect

check: $(BINARY_SERVER)
	@for cmd in $(NEEDED) ; do \
		if [ -z `which $$cmd` ] ; then \
			echo "missing tool: $$cmd" ; \
			echo "To run this test, you need to have package dejagnu installed!" ; \
			exit 1; \
		fi \
	done
	runtest HMSTR=`pwd`/$(BINARY_SERVER) --tool $(BINARY_SERVER) --srcdir ./testsuite

clean:
	make -C libsrc clean
	rm -f $(OBJS) $(DEPS)
	rm -f $(BINARY_SERVER)

distclean: clean
	make -C libsrc distclean
	rm -f $(PROGS) *~ 
	rm -rf $(DOC)
