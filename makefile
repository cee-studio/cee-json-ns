JSON_SRC=value.cpp parser.cpp snprint.cpp tokenizer.cpp
JSON_HDR=json.hpp tokenizer.hpp utf8.h
CXXFLAGS = -fno-rtti -fno-exceptions -Wno-write-strings

HEADERS=stdlib.h string.h errno.h sys/types.h sys/stat.h unistd.h stdio.h

define json_amalgamation
	@echo "#ifndef CEE_JSON_ONE" > $(1)
	@echo "#define CEE_JSON_ONE" >> $(1)
	@echo "#define _GNU_SOURCE" >> $(1)
	@for ii in $(HEADERS); do echo '#include <'$$ii'>' >> $(1); done
	@echo "#include \"cee.hpp\"" >> $(1)
	@echo " " >> $(1)
	@for ii in $(JSON_HDR); do cat $$ii >> $(1); echo " " >> $(1); done
	@echo "#define CEE_JSON_AMALGAMATION" > tmp.cpp
	@for ii in $(JSON_SRC); do echo '#include "'$$ii'"' >> tmp.cpp; done
	$(CXX) -E -CC $(2) -nostdinc tmp.cpp >> $(1)
	@echo "#endif" >> $(1)
endef

.PHONY: release clean distclean

all: tester

json-one.cpp: $(JSON_SRC) cee.hpp
	$(call json_amalgamation, json-one.cpp)

json-one.o: json-one.cpp cee.hpp
	$(CXX) -c $(CXXFLAGS) json-one.cpp

cee.o: cee.cpp cee.hpp
	$(CXX) -c $(CXXFLAGSS) -g cee.cpp

release: $(JSON_SRC)
	$(call json_amalgamation, json.cpp, -P)
	@mkdir -p release
	@mv json.cpp release
	@cp json.hpp release

tester: json-one.o cee.o
	$(CXX)  -static -g tester.cpp json-one.o cee.o

clean:
	rm -f cee.o json-one.cpp json-one.o tmp.cpp

distclean: clean
	rm -f cee.cpp cee.hpp