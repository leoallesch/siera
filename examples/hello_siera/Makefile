all:
	cmake -B build -DSIERA_PLATFORM=posix
	cmake --build build -- -j$(shell nproc)

clean:
	rm -rf build