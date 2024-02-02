all: bin bin/keygen bin/keybuilder bin/keysynth

benchmark: bin/keyuser bin/sepe-runner

keyuser: bin/keyuser

bin:
	@mkdir -p bin/

bin/keygen: $(shell find src/keygen/src/ -type f)
	cd src/keygen && cargo build --release
	cp src/keygen/target/release/keygen $@

bin/keyuser: $(shell find src/keyuser/src/ -type f) src/keyuser/Makefile
	cd src/keyuser && make keyuser
	cp src/keyuser/keyuser $@

bin/keyuser-debug: $(shell find src/keyuser/src/ -type f) src/keyuser/Makefile
	cd src/keyuser && make keyuser-debug
	cp src/keyuser/keyuser-debug $@

bin/keybuilder: $(shell find src/keybuilder/src/ -type f) src/keybuilder/Makefile
	cd src/keybuilder && make keybuilder
	cp src/keybuilder/keybuilder $@

bin/keybuilder-debug: $(shell find src/keybuilder/src/ -type f) src/keybuilder/Makefile
	cd src/keybuilder && make keybuilder-debug
	cp src/keybuilder/keybuilder-debug $@

bin/keysynth:  $(shell find src/keysynth/src/ -type f) src/keysynth/Makefile
	cd src/keysynth && make keysynth
	cp src/keysynth/keysynth $@

bin/keysynth-debug:  $(shell find src/keysynth/src/ -type f) src/keysynth/Makefile
	cd src/keysynth && make keysynth-debug
	cp src/keysynth/keysynth $@

bin/sepe-runner: $(shell find src/sepe-runner/src/ -type f)
	cd src/sepe-runner && cargo build --release
	cp src/sepe-runner/target/release/sepe-runner $@

clean:
	cd src/keygen && cargo clean
	cd src/sepe-runner && cargo clean
	cd src/keyuser && make clean
	cd src/keysynth && make clean
	cd src/keybuilder && make clean
	rm -rfv bin output *.csv

.PHONY: all clean
