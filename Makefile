CC=g++ -Wall -Werror -g
LIBS=-lX11 -lpcrecpp

CXXTESTGEN=cxxtestgen.pl
CXXTESTINCLUDE=-I/home/derat/local/include

PROGNAME=wham

$(PROGNAME): \
  main.cc anchor.o command.o config.o config-parser.o desktop.o key-bindings.o \
  util.o window.o window-classifier.o window-manager.o x.o
	$(CC) -o $@ $(LIBS) main.cc \
	  anchor.o command.o config.o config-parser.o desktop.o key-bindings.o \
	  util.o window.o window-classifier.o window-manager.o x.o

anchor.o: anchor.cc anchor.h \
  config.h util.h window.h x.h
	$(CC) -c anchor.cc

command.o: command.cc command.h util.h
	$(CC) -c command.cc

config.o: config.cc config.h util.h
	$(CC) -c config.cc

config-parser.o: config-parser.cc config-parser.h util.h
	$(CC) -c config-parser.cc

desktop.o: desktop.cc desktop.h anchor.h util.h
	$(CC) -c desktop.cc

key-bindings.o: key-bindings.cc key-bindings.h command.h util.h
	$(CC) -c key-bindings.cc

util.o: util.cc util.h
	$(CC) -c util.cc

window.o: window.cc window.h \
  config.h util.h window-classifier.h x.h
	$(CC) -c window.cc

window-classifier.o: window-classifier.cc window-classifier.h util.h
	$(CC) -c window-classifier.cc

window-manager.o: window-manager.cc window-manager.h \
  anchor.h command.h config.h key-bindings.h util.h window.h \
  window-classifier.h
	$(CC) -c window-manager.cc

x.o: x.cc x.h command.h key-bindings.h util.h window-classifier.h \
  window-manager.h
	$(CC) -c x.cc

clean:
	rm -f $(PROGNAME) anchor.o command.o config.o config-parser.o \
	  desktop.o key-bindings.o util.o window.o window-classifier.o \
	  window-manager.o x.o \
	  command_test command_test.cc \
	  config-parser_test config-parser_test.cc \
	  key-bindings_test key-bindings_test.cc \
	  util_test util_test.cc \
	  window-classifier_test window-classifier_test.cc \
	  window-manager_test window-manager_test.cc \
	  x_test x_test.cc

command_test.cc: command_test.h
	$(CXXTESTGEN) --error-printer -o $@ command_test.h

command_test: \
  command_test.cc command_test.h command.o util.o
	$(CC) $(CXXTESTINCLUDE) $(LIBS) -o $@ \
	  command_test.cc command.o util.o

config-parser_test.cc: config-parser_test.h
	$(CXXTESTGEN) --error-printer -o $@ config-parser_test.h

config-parser_test: \
  config-parser_test.cc config-parser_test.h config-parser.o util.o
	$(CC) $(CXXTESTINCLUDE) $(LIBS) -o $@ \
	  config-parser_test.cc config-parser.o util.o

key-bindings_test.cc: key-bindings_test.h
	$(CXXTESTGEN) --error-printer -o $@ key-bindings_test.h

key-bindings_test: \
  key-bindings_test.cc key-bindings_test.h key-bindings.o command.o util.o
	$(CC) $(CXXTESTINCLUDE) $(LIBS) -o $@ \
	  key-bindings_test.cc key-bindings.o command.o util.o

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

window-manager_test.cc: window-manager_test.h
	$(CXXTESTGEN) --error-printer -o $@ window-manager_test.h

window-manager_test: \
  window-manager_test.cc window-manager_test.h window-manager.o \
  anchor.o command.o config.o window.o window-classifier.o util.o x.o
	$(CC) $(CXXTESTINCLUDE) $(LIBS) -o $@ \
	  window-manager_test.cc window-manager.o \
	  anchor.o command.o config.o key-bindings.o util.o window.o \
	  window-classifier.o x.o

x_test.cc: x_test.h
	$(CXXTESTGEN) --error-printer -o $@ x_test.h

x_test: x_test.cc x_test.h x.o config.o key-bindings.o util.o window.o \
  anchor.o window-classifier.o window-manager.o
	$(CC) $(CXXTESTINCLUDE) $(LIBS) -o $@ x_test.cc x.o \
	  anchor.o command.o config.o key-bindings.o util.o window.o \
	  window-classifier.o window-manager.o

test: command_test config-parser_test key-bindings_test util_test \
  window-classifier_test window-manager_test x_test
	./command_test
	./config-parser_test
	./key-bindings_test
	./util_test
	./window-classifier_test
	./window-manager_test
	./x_test
