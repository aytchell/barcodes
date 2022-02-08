# output binary
BIN := barcodes

# source files
SRCS := barcodes.c input_buffer.c input_device.c \
	send_http.c config.c string_utils.c \
	cfg_logger.c logger.c 

# intermediate directory for generated object files
OBJDIR := .objs
# intermediate directory for generated dependency files
DEPDIR := .deps

# object files, auto generated from source files
OBJS := $(patsubst %,$(OBJDIR)/%.o,$(basename $(SRCS)))
# dependency files, auto generated from source files
DEPS := $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))

# compilers (at least gcc and clang) don't create the subdirectories automatically
$(shell mkdir -p $(dir $(OBJS)) >/dev/null)
$(shell mkdir -p $(dir $(DEPS)) >/dev/null)

# C compiler
CC := gcc
# linker
LD := gcc

BASIC_CFLAGS = -Wall -pedantic -Wextra

LIBEV_CFLAGS = `pkg-config --cflags libevdev`
LIBEV_LDFLAGS = `pkg-config --libs libevdev`

LIBCURL_CFLAGS = `pkg-config --cflags libcurl`
LIBCURL_LDFLAGS = `pkg-config --libs libcurl`

# C flags
CFLAGS := $(BASIC_CFLAGS) $(LIBEV_CFLAGS) $(LIBCURL_CFLAGS)
# linker flags
LDFLAGS := 
# linker flags: libraries to link (e.g. -lfoo)
LDLIBS := $(LIBEV_LDFLAGS) $(LIBCURL_LDFLAGS)
# flags required for dependency generation; passed to compilers
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

# postcompile step
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

all: $(BIN)

.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(DEPDIR)

.PHONY: distclean
distclean: clean
	rm -f $(BIN)

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: %.c $(DEPDIR)/%.d
	$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
	$(POSTCOMPILE)

.PRECIOUS: $(DEPDIR)/%.d
$(DEPDIR)/%.d: ;

-include $(DEPS)

