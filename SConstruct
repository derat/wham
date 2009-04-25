# Copyright 2008 Daniel Erat <dan@erat.org>
# All rights reserved.

import os
import subprocess

Help('''
Type: 'scons wham' to build Wham
      'scons test' to build and run all tests
''')


def run_tests(target, source, env):
  '''Run all test binaries listed in 'source'.'''

  tests = sorted([str(t) for t in source])
  max_length = max([len(t) for t in tests])

  for test in tests:
    padded_name = test + ' ' * (max_length - len(test))

    proc = subprocess.Popen('./%s' % test,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT,
                            close_fds=True)
    proc.wait()
    if proc.returncode == 0:
      print '%s OK' % padded_name
    else:
      print '%s FAILED' % padded_name


test_source_builder = Builder(
    action=('cxxtestgen.pl --error-printer -o $TARGET $SOURCE'),
    suffix='.cc',
    src_suffix='.h')
run_tests_builder = Builder(action=run_tests)


env = Environment(
    BUILDERS={
      'TestSource': test_source_builder,
      'RunTests': run_tests_builder,
    },
    ENV=os.environ)
env['CCFLAGS'] = '-Wall -Werror -g'
env.ParseConfig('pkg-config --cflags --libs ' +
                'x11 libpcrecpp xcb x11-xcb xcb-atom xcb-icccm')


srcs = Split('''\
  anchor.cc
  command.cc
  config.cc
  config-parser.cc
  desktop.cc
  drawing-engine.cc
  key-bindings.cc
  mock-x-window.cc
  util.cc
  window.cc
  window-classifier.cc
  window-manager.cc
  window-properties.cc
  x-server.cc
  x-window.cc
''')

libwham = env.Library('wham', srcs)
env['LIBS'] += libwham

env.Program('wham', 'main.cc')


tests = []
for header in Glob('*_test.h', strings=True):
  src = env.TestSource(header)
  tests += env.Program(src, CPPPATH='/home/derat/local/include')
env.RunTests('test', tests)
