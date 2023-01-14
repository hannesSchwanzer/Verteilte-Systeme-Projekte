# Compile with gcc
CC=gcc
# Command to auto-generate dependencies with gcc
DEPCC=$(CC) -MM

# Additional flags to pass to gcc:
# -g : generate debug information so we can use gdb to debug
# -I <dir> : look in directory <dir> for additional .h files
CFLAGS=-g -Iinclude 
# all warnings + warnings are considered as errors
CFLAGS+= -Wall -Werror

# Linker options: link program against libhamster.a
# to be found in directory ../libsrc
LDLIBS=-Llib -lpaho-mqtt3cs

DOC=doc

# compile all .c files found in directory 'src'
FILES=$(shell find ./src/ -name 'hamster_mqtt.c')


PROG = hamster_mqtt


OBJS=$(FILES:.c=.o)



.PHONY: all test clean distclean 


all: $(DOC) $(PROG) certs/fake_ca.crt

$(PROG): $(OBJS) $(LIB)
	$(CC) $^ -o $@ $(LDLIBS) $(LDFLAGS)


$(DOC): $(FILES) include/MQTTAsync.h include/MQTTClient.h include/MQTTClientPersistence.h include/doxyfile
	doxygen include/doxyfile 2> /dev/null # doxygen output is disabled!

certs/fake_ca.crt: certs/mqtt_ca.crt 
	@if [ -e certs/saved_mqtt_ca.crt ]; then \
		echo "clean up certs. Test killed before cleanup."; \
		mv certs/mqtt_ca.crt certs/fake_ca.crt; \
		mv certs/saved_mqtt_ca.crt certs/mqtt_ca.crt; \
	elif [ ! -e certs/fake_ca.crt ]; then \
		echo "certs/fake_ca.crt is lost restore from svn"; \
		svn revert certs/fake_ca.crt; \
		svn revert certs/mqtt_ca.crt; \
	fi

#
# Run our testsuite: For this, packages DejaGnu and mosquitto must be installed
#
NEEDED=runtest expect mosquitto_sub mosquitto_pub
# wether to use external or local broker: can be Yes or no
EXT_BROKER=Yes

test: all certs/fake_ca.crt
	@for cmd in $(NEEDED) ; do \
		if [ -z `which $$cmd` ] ; then \
			echo "missing tool: $$cmd" ; \
			echo "To run this test, you need to have Packages DejaGnu and mosquitto installed!" ; \
			exit 1; \
		fi \
	done
	@-killall mosquitto; sleep 0.5
	@if [ '$(EXT_BROKER)' = 'no' ]; then \
		ps -A | grep mosquitto; \
		if [ $$? = 0 ]; then \
			echo "\e[31mWarning: mosquitto deamon run in backgorund, try terminated deamon with 'sudo systemctl stop mosquitto'\e[39m";\
		else \
			cd configs; \
			pwd; \
			echo "Start mosquitto Server"; \
			mosquitto -c mosquitto.conf -d; \
			cd ..; \
		fi; \
	fi
	export HAMSTERID=`od -An -N3 -t uI < /dev/urandom | sed "s/ *//g"`; \
	export EXTBROKER=$(EXT_BROKER); \
	runtest --tool hamster_mqtt --srcdir ./test

clean:
	rm -f $(OBJS) $(DEPS)
	rm -f $(PROG)

distclean: clean
	rm -f $(PROG) *~ 
	rm -rf $(DOC)


subscribe: $(DOC) $(PROGSUB) certs/fake_ca.crt

$(PROGSUB): $(OBJSUB) $(LIB)
	$(CC) $^ -o $@ $(LDLIBS) $(LDFLAGS)


$(DOC): $(FILESSUB) include/MQTTAsync.h include/MQTTClient.h include/MQTTClientPersistence.h include/doxyfile
	doxygen include/doxyfile 2> /dev/null # doxygen output is disabled!
