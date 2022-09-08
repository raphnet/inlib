targets = all clean

$(targets):
	@$(MAKE) -C src $@

.PHONY: $(targets)
