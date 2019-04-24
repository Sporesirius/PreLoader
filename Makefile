EFIFILES = PreLoader.efi

MSGUID = 77FA9ABD-0359-4D32-BD60-28F4E78F784B

export TOPDIR	:= $(shell pwd)/

include Make.rules

all: $(EFIFILES)

lib/lib.a lib/lib-efi.a: FORCE
	$(MAKE) -C lib $(notdir $@)

.SUFFIXES: .crt

.KEEP: $(EFIFILES)

PreLoader.so: lib/lib-efi.a

clean:
	rm -f PK.* KEK.* DB.* *.o *.so
	rm -f noPK.*
	$(MAKE) -C lib clean

FORCE: