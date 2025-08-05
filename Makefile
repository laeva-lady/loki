r:
	cmake . -B build
	make -C build
	mv build/loki ~/.local/bin
	loki

b:
	cmake . -B build
	make -C build
	mv build/loki ~/.local/bin
