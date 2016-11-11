BUILD_DIRS += ipc 

all: build

build:
	@for i in $(BUILD_DIRS); do \
		$(MAKE) -C $$i ; \
	done

clean:
	@for i in $(BUILD_DIRS); do \
		$(MAKE) -C $$i $@ ; \
	done
