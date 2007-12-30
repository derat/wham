CC=g++ -Wall -Werror
LIBS=-lX11 -lpcrecpp

CXXTESTGEN=cxxtestgen.pl
CXXTESTINCLUDE=-I/home/derat/local/include

PROGNAME=wham

$(PROGNAME): \
  main.cc config.o config-parser.o key-bindings.o util.o window.o \
  window-anchor.o window-classifier.o window-manager.o x.o
	$(CC) -o $@ $(LIBS) main.cc \
	  config.o config-parser.o key-bindings.o util.o window.o \
	  window-anchor.o window-classifier.o window-manager.o x.o

config.o: config.cc config.h util.h
	$(CC) -c config.cc

config-parser.o: config-parser.cc config-parser.h util.h
	$(CC) -c config-parser.cc

key-bindings.o: key-bindings.cc key-bindings.h
	$(CC) -c key-bindings.cc

util.o: util.cc util.h
	$(CC) -c util.cc

window.o: window.cc window.h \
  config.h util.h window-classifier.h x.h
	$(CC) -c window.cc

window-anchor.o: window-anchor.cc window-anchor.h \
  config.h util.h window.h x.h
	$(CC) -c window-anchor.cc

window-classifier.o: window-classifier.cc window-classifier.h util.h
	$(CC) -c window-classifier.cc

window-manager.o: window-manager.cc window-manager.h \
  key-bindings.h util.h window.h window-anchor.h window-classifier.h
	$(CC) -c window-manager.cc

x.o: x.cc x.h util.h window-classifier.h window-manager.h
	$(CC) -c x.cc

clean:
	rm -f $(PROGNAME) config.o config-parser.o key-bindings.o util.o \
	  window.o window-anchor.o window-classifier.o window-manager.o x.o \
	  config-parser_test config-parser_test.cc \
	  key-bindings_test key-bindings_test.cc \
	  util_test util_test.cc \
	  window-classifier_test window-classifier_test.cc

config-parser_test.cc: config-parser_test.h
	$(CXXTESTGEN) --error-printer -o $@ config-parser_test.h

config-parser_test: \
  config-parser_test.cc config-parser_test.h config-parser.o util.o
	$(CC) $(CXXTESTINCLUDE) $(LIBS) -o $@ \
	  config-parser_test.cc config-parser.o util.o

key-bindings_test.cc: key-bindings_test.h
	$(CXXTESTGEN) --error-printer -o $@ key-bindings_test.h

key-bindings_test: \
  key-bindings_test.cc key-bindings_test.h key-bindings.o util.o
	$(CC) $(CXXTESTINCLUDE) $(LIBS) -o $@ \
	  key-bindings_test.cc key-bindings.o util.o

util_test.cc: util_test.h
	$(CXXTESTGEN) --error-printer -o $@ util_test.h

util_test: util_test.cc util_test.h util.o
	$(CC) $(CXXTESTINCLUDE) $(LIBS) -o $@ util_test.cc util.o

window-classifier_test.cc: window-classifier_test.h
	$(CXXTESTGEN) --error-printer -o $@ window-classifier_test.h

window-classifier_test: \
  window-classifier_test.cc window-classifier_test.h window-classifier.o util.o
	$(CC) $(CXXTESTINCLUDE) $(LIBS) -o $@ \
	  window-classifier_test.cc window-classifier.o util.o

test: config-parser_test key-bindings_test util_test window-classifier_test
	./config-parser_test
	./key-bindings_test
	./util_test
	./window-classifier_test
