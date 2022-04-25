#
# Project files
#

BUILD_PREFIX = build
SRCS = $(shell find src -type f -name '*.cpp')
OBJS = $(patsubst %.cpp, %.o, $(SRCS))
EXE = xcov.run
CFLAGS = -Wall -Isrc -Ilibs -std=c++17
LDFLAGS = -L/usr/local/lib -lstdc++fs -lsource-highlight -lboost_filesystem

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

#
# Debug build settings
#

DBG_DIR = $(BUILD_PREFIX)/debug
DBG_EXE = $(DBG_DIR)/$(EXE)
DBG_OBJS = $(addprefix $(DBG_DIR)/, $(OBJS))
DBG_CFLAGS = -g -O0 --coverage

#
# Release build settings
#

REL_DIR = $(BUILD_PREFIX)/release
REL_EXE = $(REL_DIR)/$(EXE)
REL_OBJS = $(addprefix $(REL_DIR)/, $(OBJS))
REL_CFLAGS = -O3 -DDEFAULT_TEMPL_PATH='"$(PREFIX)/share/xcov/themes/default"'

.PHONY: all clean debug release remake install

# Default build
all: release

#
# Debug rules
#

test: debug
	@mkdir -p tmp_build
	$(DBG_EXE) -v -r tmp_build -o test_html --filenameing hash

run: debug
	$(DBG_EXE) $(ARGS)

debug: $(DBG_EXE)

$(DBG_EXE): $(DBG_OBJS)
	@mkdir -p "$(@D)"
	$(CXX) $(CFLAGS) $(DBG_CFLAGS) -o $@ $^ -m64 $(LDFLAGS)

$(DBG_DIR)/%.o: %.cpp
	@mkdir -p "$(@D)"
	$(CXX) -c -m64 $(CFLAGS) $(DBG_CFLAGS) -o $@ $^

#
# Release rules
#

release: $(REL_EXE)

$(REL_EXE): $(REL_OBJS)
	@mkdir -p "$(@D)"
	$(CXX) $(CFLAGS) $(REL_CFLAGS) -o $@ $^ -m64 $(LDFLAGS)

$(REL_DIR)/%.o: %.cpp
	@mkdir -p "$(@D)"
	$(CXX) -c -m64 $(CFLAGS) $(REL_CFLAGS) -o $@ $^

#
# Other rules
#

install: release
	install -d $(PREFIX)/bin/
	install -m 755 $(REL_EXE) $(PREFIX)/bin/xcov
	install -d $(PREFIX)/etc/xcov/
	install -m 644 default.cfg $(PREFIX)/etc/xcov/
	install -d $(PREFIX)/share/xcov/themes/default/
	install -m 644 xcov_data/themes/default/index.html $(PREFIX)/share/xcov/themes/default/
	install -m 644 xcov_data/themes/default/sourcefile.html $(PREFIX)/share/xcov/themes/default/
	install -m 644 xcov_data/themes/default/theme.css $(PREFIX)/share/xcov/themes/default/
	install -m 644 xcov_data/themes/default/bootstrap.min.css $(PREFIX)/share/xcov/themes/default/

remake: clean all

clean:
	rm -rf ./$(BUILD_PREFIX)