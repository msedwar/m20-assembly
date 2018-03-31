ASSEMBLE = bin/cmake-build-debug/assemble
LINK = bin/cmake-build-debug/link
SIMULATE = bin/cmake-build-debug/simulate

for: for.mc
	@$(SIMULATE) $^

for.mc: for.obj print.obj string.obj
	@$(LINK) $@ $^

for.obj: for.as
	@$(ASSEMBLE) $^ $@

print.obj: print.as
	@$(ASSEMBLE) $^ $@

string.obj: string.as
	@$(ASSEMBLE) $^ $@

syscall.obj: syscall.as
	@$(ASSEMBLE) $^ $@

clean:
	@rm -rf *.obj
	@rm -rf *.mc

.PHONY:
	clean
