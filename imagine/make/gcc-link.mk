LD = $(CXX)

ifdef LINK_MAP
 MAPFILE := link.map
 LDFLAGS += -Wl,-Map=$(MAPFILE)
endif

ifdef PROFILE
 LDFLAGS_SYSTEM += -pg
else
 OPTIMIZE_LDFLAGS = -s
endif

ifdef O_RELEASE
 LDFLAGS_SYSTEM += $(OPTIMIZE_LDFLAGS)
endif

linkLoadableModuleAction ?= -shared
loadableModuleExt := .so

LDFLAGS += $(CFLAGS_TARGET) $(EXTRA_LDFLAGS)
