CC=g++ -Wall -Werror
LIBS=-lX11 -lpcrecpp

CXXTESTGEN=cxxtestgen.pl
CXXTESTINCLUDE=-I/home/derat/local/include

TESTLIBS=anchor.o command.o config.o config-parser.o desktop.o \
	 drawing-engine.o key-bindings.o mock-x.o util.o window.o \
	 window-classifier.o window-manager.o x.o

PROGNAME=wham

$(PROGNAME): \
  main.cc anchor.o command.o config.o config-parser.o desktop.o \
  drawing-engine.o key-bindings.o mock-x.o util.o window.o \
  window-classifier.o window-manager.o x.o
	$(CC) -o $@ $(LIBS) $^

anchor.o: anchor.cc anchor.h config.h util.h window.h x.h
	$(CC) -c $<

command.o: command.cc command.h util.h
	$(CC) -c $<

config.o: config.cc config.h util.h
	$(CC) -c $<

config-parser.o: config-parser.cc config-parser.h util.h
	$(CC) -c $<

desktop.o: desktop.cc desktop.h anchor.h util.h
	$(CC) -c $<

drawing-engine.o: drawing-engine.cc drawing-engine.h \
  anchor.h config.h window.h util.h x.h
	$(CC) -c $<

key-bindings.o: key-bindings.cc key-bindings.h command.h util.h
	$(CC) -c $<

mock-x.o: mock-x.cc mock-x.h
	$(CC) -c $<

util.o: util.cc util.h
	$(CC) -c $<

window.o: window.cc window.h \
  config.h util.h window-classifier.h x.h
	$(CC) -c $<

window-classifier.o: window-classifier.cc window-classifier.h util.h
	$(CC) -c $<

window-manager.o: window-manager.cc window-manager.h \
  anchor.h command.h config.h key-bindings.h util.h window.h \
  window-classifier.h
	$(CC) -c $<

x.o: x.cc x.h command.h key-bindings.h util.h window-classifier.h \
  window-manager.h
	$(CC) -c $<

clean:
	rm -f $(PROGNAME) *.o *_test *_test.cc

TEST_HEADERS=$(wildcard *_test.h)
TEST_SOURCES=$(subst .h,.cc,$(TEST_HEADERS))
TEST_BINARIES=$(subst .h,,$(TEST_HEADERS))

%_test.cc: %_test.h
	$(CXXTESTGEN) --error-printer -o $@ $<

%_test: %_test.cc %_test.h $(TESTLIBS)
	$(CC) $(CXXTESTINCLUDE) $(LIBS) -o $@ $^

test: $(TEST_BINARIES)
	for test in $(TEST_BINARIES); do echo -n "$${test}: "; ./$$test; done

tests: test
