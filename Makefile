BUILD ?= build-host
CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -Werror -Wno-misleading-indentation -Isrc
SOURCES=src/main.c src/files.c src/terminal.c
.PHONY: host
host: $(BUILD)/cmdr
$(BUILD)/cmdr: $(SOURCES) src/cmdr.h
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $(SOURCES) -o $@

.PHONY: test elks clean install
test: host
	$(CC) $(CFLAGS) tests/ops.c src/files.c src/terminal.c -o $(BUILD)/test-ops
	$(CC) $(CFLAGS) tests/test_faults.c -o $(BUILD)/test-faults
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
clean:
	rm -rf build build-host
