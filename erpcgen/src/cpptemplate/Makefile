#-------------------------------------------------------------------------------
# Copyright (c) 2014 Freescale Semiconductor, Inc.
# Copyright 2016 NXP
# All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#-------------------------------------------------------------------------------

TARGET = cpptempl_test

OBJECTS = cpptempl.o cpptempl_test.o

BOOST_ROOT = /usr/local/opt/boost

LIBRARIES = -lc -lstdc++ -lm -lboost_unit_test_framework-mt -L$(BOOST_ROOT)/lib

INCLUDES = -I$(BOOST_ROOT)/include

CXXFLAGS = -std=gnu++11 -Werror -g3 -O0 -MMD -MP $(INCLUDES)

.PHONY: all
all: cpptempl_test

.PHONY: clean
clean:
	@echo "Cleaning output..."
	@rm -rf *.o
	@rm -rf *.d
	@rm -f $(TARGET)

.PHONY: test testv
test: all
	./cpptempl_test
testv: all
	./cpptempl_test -l test_suite

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LIBRARIES) -o $@

# Include dependency files.
-include $(OBJECTS:.o=.d)
