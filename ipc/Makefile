BUILD_DIRS += domain_socket

all: build

build:
	@for i in $(BUILD_DIRS); do \
		$(MAKE) -C $$i ; \
	done

clean:
	@for i in $(BUILD_DIRS); do \
		$(MAKE) -C $$i $@ ; \
	done
