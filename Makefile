all: bin bin/keygen bin/keyuser bin/bench-runner bin/keybuilder

bin:
	@mkdir -p bin/

bin/keygen: $(shell find src/keygen/src/ -type f)
	cd src/keygen && cargo build --release
	cp src/keygen/target/release/keygen $@

bin/keyuser: $(shell find src/keyuser/src/ -type f)
	cd src/keyuser && make keyuser
	cp src/keyuser/keyuser $@

bin/keyuser-debug: $(shell find src/keyuser/src/ -type f)
	cd src/keyuser && make keyuser-debug
	cp src/keyuser/keyuser-debug $@

bin/keybuilder: $(shell find src/keybuilder/src/ -type f)
	cd src/keybuilder && make keybuilder
	cp src/keybuilder/keybuilder $@

bin/keybuilder-debug: $(shell find src/keybuilder/src/ -type f)
	cd src/keybuilder && make keybuilder-debug
	cp src/keybuilder/keybuilder-debug $@

bin/bench-runner: $(shell find src/bench-runner/src/ -type f)
	cd src/bench-runner && cargo build --release
	cp src/bench-runner/target/release/bench-runner $@

clean:
	cd src/keygen && cargo clean
	cd src/keyuser && make clean
	cd src/bench-runner && cargo clean
	rm -rfv bin

.PHONY: all clean
