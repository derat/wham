CC=g++ -Wall -Werror -g
LIBS=-lX11 -lpcrecpp

CXXTESTGEN=cxxtestgen.pl
CXXTESTINCLUDE=-I/home/derat/local/include

PROGNAME=wham

$(PROGNAME): \
  main.cc anchor.o command.o config.o config-parser.o desktop.o key-bindings.o \
  util.o window.o window-classifier.o window-manager.o x.o
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

key-bindings.o: key-bindings.cc key-bindings.h command.h util.h
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
	$(CXXTESTGEN) --error-printer -o $@ $<

command_test: \
  command_test.cc command_test.h command.o util.o
	$(CC) $(CXXTESTINCLUDE) $(LIBS) -o $@ $^

config-parser_test.cc: config-parser_test.h
	$(CXXTESTGEN) --error-printer -o $@ $<

config-parser_test: \
  config-parser_test.cc config-parser_test.h config-parser.o util.o
	$(CC) $(CXXTESTINCLUDE) $(LIBS) -o $@ $^

key-bindings_test.cc: key-bindings_test.h
	$(CXXTESTGEN) --error-printer -o $@ $<

key-bindings_test: \
  key-bindings_test.cc key-bindings_test.h key-bindings.o command.o util.o
	$(CC) $(CXXTESTINCLUDE) $(LIBS) -o $@ $^

util_test.cc: util_test.h
	$(CXXTESTGEN) --error-printer -o $@ $<

util_test: util_test.cc util_test.h util.o
	$(CC) $(CXXTESTINCLUDE) $(LIBS) -o $@ $^

window-classifier_test.cc: window-classifier_test.h
	$(CXXTESTGEN) --error-printer -o $@ $<

window-classifier_test: \
  window-classifier_test.cc window-classifier_test.h window-classifier.o util.o
	$(CC) $(CXXTESTINCLUDE) $(LIBS) -o $@ $^

window-manager_test.cc: window-manager_test.h
	$(CXXTESTGEN) --error-printer -o $@ $<

window-manager_test: \
  window-manager_test.cc window-manager_test.h window-manager.o \
  anchor.o command.o config.o desktop.o window.o window-classifier.o util.o x.o
	$(CC) $(CXXTESTINCLUDE) $(LIBS) -o $@ $^

x_test.cc: x_test.h
	$(CXXTESTGEN) --error-printer -o $@ $<

x_test: x_test.cc x_test.h x.o command.o config.o desktop.o key-bindings.o \
  util.o window.o anchor.o window-classifier.o window-manager.o
	$(CC) $(CXXTESTINCLUDE) $(LIBS) -o $@ $^

test: command_test config-parser_test key-bindings_test util_test \
  window-classifier_test window-manager_test x_test
	./command_test
	./config-parser_test
	./key-bindings_test
	./util_test
	./window-classifier_test
	./window-manager_test
	./x_test

tests: test
