all: bin bin/keygen bin/keybuilder bin/keysynth

benchmark: bin/keyuser bin/sepe-runner

keyuser: bin/keyuser

bin:
	@mkdir -p bin/

bin/keygen: $(shell find src/keygen/src/ -type f)
	cd src/keygen && cargo build --release
	cp src/keygen/target/release/keygen $@

# Example: make keyuser UPPER_SHIFT=32
bin/keyuser: $(shell find src/keyuser/src/ -type f) src/keyuser/Makefile
	make -C src/keyuser keyuser UPPER_SHIFT=$(UPPER_SHIFT) RQ8=$(RQ8)
	cp src/keyuser/keyuser $@

bin/keyuser-debug: $(shell find src/keyuser/src/ -type f) src/keyuser/Makefile
	make -C src/keyuser keyuser-debug UPPER_SHIFT=$(UPPER_SHIFT) RQ8=$(RQ8)
	cp src/keyuser/keyuser-debug $@

bin/keybuilder: $(shell find src/keybuilder/src/ -type f) src/keybuilder/Makefile
	make -C src/keybuilder keybuilder
	cp src/keybuilder/keybuilder $@

bin/keybuilder-debug: $(shell find src/keybuilder/src/ -type f) src/keybuilder/Makefile
	make -C src/keybuilder keybuilder-debug
	cp src/keybuilder/keybuilder-debug $@

bin/keysynth:  $(shell find src/keysynth/src/ -type f) src/keysynth/Makefile
	make -C src/keysynth keysynth
	cp src/keysynth/keysynth $@

bin/keysynth-debug:  $(shell find src/keysynth/src/ -type f) src/keysynth/Makefile
	make -C src/keysynth keysynth-debug
	cp src/keysynth/keysynth $@

bin/sepe-runner: $(shell find src/sepe-runner/src/ -type f)
	cd src/sepe-runner && cargo build --release
	cp src/sepe-runner/target/release/sepe-runner $@

clean:
	cd src/keygen &&      cargo clean
	cd src/sepe-runner && cargo clean
	make -C src/keyuser         clean
	make -C src/keysynth        clean
	make -C src/keybuilder      clean
	rm -rfv bin output *.csv

results-clear:
	make -C results/ clear

.PHONY: all clean
