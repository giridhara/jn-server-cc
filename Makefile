ifndef VERBOSE
MAKEFLAGS += --no-print-directory
endif
VERSION=$(shell cat jn.spec | grep '^Version:' | cut -f2)
RELEASE=$(shell date +%s)

all: debug

prep:
	test -d target || mkdir target

release: prep
	cd target && \
	{ test -d release || mkdir release; } && \
	cd release && \
	cmake -DBUILD_VERSION=$(VERSION) -DCMAKE_BUILD_TYPE=RelWithDebInfo ../.. && \
	$(MAKE)

debug: prep
	cd target && \
	{ test -d debug || mkdir debug; } && \
	cd debug && \
	cmake -DBUILD_VERSION=$(VERSION) -DCMAKE_BUILD_TYPE=DEBUG ../.. && \
	$(MAKE)

clean:
	rm -rf target

