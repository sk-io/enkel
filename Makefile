
all:
	$(MAKE) -C enkel
	$(MAKE) -C framework
	cp framework/framework enkelfw

clean:
	$(MAKE) -C enkel clean
	$(MAKE) -C framework clean
	rm -f enkelfw


