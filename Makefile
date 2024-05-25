PROJECT_SOURCE_DIR ?= $(abspath ./)
BUILD_DIR ?= $(PROJECT_SOURCE_DIR)/build
NUM_JOB ?= 32

clean:
	rm -rf $(BUILD_DIR)
.PHONY: clean

prepare:
	mkdir -p $(BUILD_DIR) && \
	cd $(BUILD_DIR) && \
	cmake $(PROJECT_SOURCE_DIR)
.PHONY: prepare

test: prepare
	cd $(BUILD_DIR) && \
	make CTZ_TEST -j $(NUM_JOB) && \
	./CTZ_TEST
.PHONY: test

format:
	./format.sh run
.PHONY: format

check_format:
	./format.sh check
.PHONY: check_format
