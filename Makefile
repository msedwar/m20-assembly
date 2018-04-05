# Constants --------------------------------------------------------------------

ASSEMBLE = bin/cmake-build-debug/assemble
LINK = bin/cmake-build-debug/link
SIMULATE = bin/cmake-build-debug/simulate

OUTDIR = bin/assembly
OBJDIR = $(OUTDIR)/obj
MCDIR = $(OUTDIR)/mc
ASDIR = assembly

default: kernel

# For --------------------------------------------------------------------------

for: $(MCDIR)/for.mc
	@$(SIMULATE) $^

$(MCDIR)/for.mc: $(OBJDIR)/test/for.obj \
	$(OBJDIR)/lib/stdio.obj \
	$(OBJDIR)/lib/stdlib.obj \
	$(OBJDIR)/lib/string.obj
	@$(LINK) $@ $^


# Kernel -----------------------------------------------------------------------

kernel: $(MCDIR)/kernel.mc
	@$(SIMULATE) $^
	
$(MCDIR)/kernel.mc: $(OBJDIR)/kernel/boot.obj \
	$(OBJDIR)/kernel/io.obj \
	$(OBJDIR)/kernel/reset_handler.obj \
	$(OBJDIR)/lib/stdlib.obj \
	$(OBJDIR)/lib/string.obj
	@$(LINK) $@ $^


# Automatic Rules --------------------------------------------------------------

$(OBJDIR)/%.obj: $(ASDIR)/%.as
	@$(ASSEMBLE) $< $@


# Aliases ----------------------------------------------------------------------

clean:
	@rm -rf *.obj
	@rm -rf *.mc
	@rm -rf $(MCDIR)/*.mc
	@rm -rf $(OBJDIR)/**/*.obj

.PHONY:
	clean for kernel default
