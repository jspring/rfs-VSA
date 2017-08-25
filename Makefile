all:
	make -C mxml-release-2.10
	make -C src
	make -C scripts

clean:
	make -C src clean
