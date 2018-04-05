# Constants --------------------------------------------------------------------

ASSEMBLE = bin/cmake-build-debug/assemble
LINK = bin/cmake-build-debug/link
SIMULATE = bin/cmake-build-debug/simulate

OUTDIR = bin/assembly
OBJDIR = $(OUTDIR)/obj
MCDIR = $(OUTDIR)/mc
ASDIR = assembly

# For --------------------------------------------------------------------------

for: $(MCDIR)/for.mc
	@$(SIMULATE) $^

$(MCDIR)/for.mc: $(OBJDIR)/test/for.obj \
	$(OBJDIR)/lib/stdio.obj \
	$(OBJDIR)/lib/stdlib.obj \
	$(OBJDIR)/lib/string.obj
	@$(LINK) $@ $^


# OS ---------------------------------------------------------------------------

os: $(MCDIR)/os.mc
	@$(SIMULATE) $^
	
$(MCDIR)/os.mc: $(OBJDIR)/os/*.obj
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
	clean for os
