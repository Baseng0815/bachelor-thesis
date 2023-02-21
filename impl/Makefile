CC	= clang
TARGET 	= aarch64-linux-gnu
SYSROOT	= $(HOME)/odroid_sysroot
FLAGS 	= --target=$(TARGET) --sysroot=$(SYSROOT) -fuse-ld=lld -z notext -lz --verbose
UFLAGS	= -O3 -Wall -gdwarf-4
SESNAME	= thesis
HOST 	= odroid

SOURCE_FILES 	= naive/gift.c naive/gift_sliced.c naive/gift_neon.c table/gift_table.c
BENCH_SOURCE	= benchmark.c
BENCH_OUT 	= benchmark
TEST_SOURCE	= test.c
TEST_OUT 	= test

.PHONY: all clean run-all run-test run-benchmark deploy

all: run-all

run-all: run-test run-benchmark

# auto run on remote tmux session named $(SESNAME)
run-test: deploy
	ssh bastian@$(HOST) -p 65534 "tmux send -t $(SESNAME).0 './$(TEST_OUT)' ENTER"

run-benchmark: deploy
	ssh bastian@$(HOST) -p 65534 "tmux send -t $(SESNAME).0 './$(BENCH_OUT)' ENTER"

deploy: $(BENCH_OUT) $(TEST_OUT)
	rsync -av -e 'ssh -p 65534' --progress $(TEST_OUT) $(BENCH_OUT) bastian@$(HOST):/home/bastian/

$(TEST_OUT): $(SOURCE_FILES) $(TEST_SOURCE)
	$(CC) $(FLAGS) $(UFLAGS) -o $@ $^

$(BENCH_OUT): $(SOURCE_FILES) $(BENCH_SOURCE)
	$(CC) $(FLAGS) $(UFLAGS) -o $@ $^

clean:
	rm -f $(BENCH_OUT) $(TEST_OUT)