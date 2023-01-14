# Compile with gcc
CC=gcc
# Command to auto-generate dependencies with gcc
DEPCC=$(CC) -MM

# Additional flags to pass to gcc:
# -g : generate debug information so we can use gdb to debug
# -I <dir> : look in directory <dir> for additional .h files
CFLAGS=-g -Iinclude
# all warnings + warnings are considered as errors
CFLAGS += -Wall -Werror

# Linker options: link program against libhamster.a
# to be found in directory ../libsrc
LDLIBS=-Llibsrc -lhamster
LIB=libsrc/libhamster.a

# compile all .c files found in src directory
FILES=$(shell find src -name '*.c')

BINARY = hamster

OBJS=$(FILES:.c=.o)
DEPS=.depend

all: $(BINARY)


-include $(DEPS)

$(BINARY): $(OBJS) $(LIB)
	$(CC) $^ -o $@ $(LDLIBS) $(LDFLAGS)

$(LIB): libsrc
	$(MAKE) -C libsrc

/src/%.o: ./src/%.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DEPS):
	$(DEPCC) $(CFLAGS) $(FILES) >$(DEPS)

clean:
	rm -f $(OBJS) $(DEPS) $(BINARY)

distclean: clean
	$(MAKE) -C libsrc distclean
	rm -f $(PROGS) *~

test: $(BINARY)
	cd scripts; ./testhamster.sh

#
# Run our exhaustive, non-interactive testsuite: For this, package DejaGnu must be installed
#
NEEDED=runtest expect

check: $(BINARY)
	@for cmd in $(NEEDED) ; do \
		if [ -z `which $$cmd` ] ; then \
			echo "missing tool: $$cmd" ; \
			echo "To run this test, you need to have package dejagnu installed!" ; \
			exit 1; \
		fi \
	done
	runtest HMSTR=`pwd`/$(BINARY) --tool $(BINARY) --srcdir ./testsuite
