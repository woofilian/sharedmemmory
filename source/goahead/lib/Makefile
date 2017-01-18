LIBNVRAM=nvram

dirs=$(LIBNVRAM)

all: $(TARGETS)
	$(foreach N,$(dirs),make -C $(N);)

$(LIBNVRAM):
	make -C nvram

clean:
	make -C nvram clean
