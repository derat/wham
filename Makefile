CC=g++
LIBS=-lX11 -lpcrecpp

CXXTESTGEN=cxxtestgen.pl
CXXTESTINCLUDE=-I/home/derat/local/include

PROGNAME=wham

$(PROGNAME): main.cc config-parser.o util.o window-classifier.o
	$(CC) -o $@ $(LIBS) main.cc \
	  config-parser.o util.o window-classifier.o

config-parser.o: config-parser.cc config-parser.h util.h
	$(CC) -c config-parser.cc

util.o: util.cc util.h
	$(CC) -c util.cc

window-classifier.o: window-classifier.cc window-classifier.h util.h
	$(CC) -c window-classifier.cc

clean:
	rm -f $(PROGNAME) config-parser.o util.o window-classifier.o \
	  config-parser_test config-parser_test.cc \
	  util_test util_test.cc \
	  window-classifier_test window-classifier_test.cc

config-parser_test.cc: config-parser_test.h
	$(CXXTESTGEN) --error-printer -o $@ config-parser_test.h

config-parser_test: \
  config-parser_test.cc config-parser_test.h config-parser.o util.o
	$(CC) $(CXXTESTINCLUDE) $(LIBS) -o $@ \
	  config-parser_test.cc config-parser.o util.o

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

test: config-parser_test util_test window-classifier_test
	./config-parser_test
	./util_test
	./window-classifier_test
