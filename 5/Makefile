BINARIES =  hamster_climenue
LIB=libsrc/libhamster.a

ifneq (,$(wildcard src/hamster.x))
BINARIES += hamster_cliclient hamster_server
endif

all: $(LIB) $(BINARIES)

$(BINARIES):
	$(MAKE) -C src ../$@

$(LIB):
	$(MAKE) -C libsrc


clean:
	make -C src clean
	-rm $(BINARIES)

distclean: clean
	$(MAKE) -C src distclean
	$(MAKE) -C libsrc distclean
	rm -f $(BINARIES) *~

test: $(BINARIES)
	rm -f hamsterfile.dat output.txt
	-killall -q hamster_server | true
	./hamster_server &
	sleep 1; cat scripts/refinput.txt | ./hamster_cliclient >output.txt
	killall hamster_server
	-diff output.txt scripts/refoutput.txt

.PHONY: $(BINARIES)
