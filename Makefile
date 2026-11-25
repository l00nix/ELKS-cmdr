BUILD ?= build-host
CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -Werror -Wno-misleading-indentation -Isrc
TUI=vendor/elks-tui/tty.c vendor/elks-tui/unikey.c vendor/elks-tui/runes.c
CPPFLAGS += -Ivendor/elks-tui
SOURCES=src/main.c src/files.c src/terminal.c $(TUI)
.PHONY: host
host: $(BUILD)/cmdr
$(BUILD)/cmdr: $(SOURCES) src/cmdr.h vendor/elks-tui/unikey.h vendor/elks-tui/runes.h
	mkdir -p $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) -o $@

.PHONY: test elks clean install
test: host
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/ops.c src/files.c src/terminal.c $(TUI) -o $(BUILD)/test-ops
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_faults.c -o $(BUILD)/test-faults
	python3 tests/test_files.py $(BUILD)/test-ops
	$(BUILD)/test-faults
	python3 tests/test_ui.py $(BUILD)/cmdr
	python3 tests/check_install.py
elks:
	sh build.sh
install:
	test -f build/cmdr
	mkdir -p "$(DESTDIR)/bin" "$(DESTDIR)/lib/cmdr"
	cp build/cmdr "$(DESTDIR)/bin/cmdr"
	chmod 755 "$(DESTDIR)/bin/cmdr"
	cp COPYING "$(DESTDIR)/lib/cmdr/copying"
	cp USAGE.TXT "$(DESTDIR)/lib/cmdr/usage.txt"
	cp THIRD-PARTY.txt "$(DESTDIR)/lib/cmdr/tuilic.txt"
clean:
	rm -rf build build-host
