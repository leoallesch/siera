all:
	cmake -B build/posix -DSIERA_PLATFORM=posix
	cmake --build build/posix -- -j$(shell nproc)

clean:
	rm -rf build