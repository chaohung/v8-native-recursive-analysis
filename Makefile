PROG = main
CXX = g++
CXX_FLAGS = -std=c++11 -O3 -I$(HOME)/Desktop/OSS/v8/include
LD_FLAGS = -L$(HOME)/Desktop/OSS/v8/lib
LIBS = -lv8_{base,libbase,external_snapshot,libplatform,libsampler} -licu{uc,i18n} -linspector

FIBONACCI_NUM = 0

all: $(PROG)

%: %.cpp
	$(CXX) $(CXX_FLAGS) $< $(LD_FLAGS) $(LIBS) -o $@

run:
	@BUILTIN_PATH=$(HOME)/Desktop/OSS/v8/bin/ FIBONACCI_NUM=$(FIBONACCI_NUM) ./$(PROG)

clean:
	$(RM) $(PROG)
