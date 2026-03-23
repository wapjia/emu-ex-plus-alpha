include $(IMAGINE_PATH)/make/config.mk
-include $(projectPath)/config.mk
include $(IMAGINE_PATH)/make/pnd-metadata.mk

.PHONY: all
all : linux-bundle

CONFIG ?= Release
linux_arch ?= x86_64
linux_archPreset = linux-$(linux_arch)

linux_buildName := linux
ifeq ($(CONFIG), Debug)
 linux_buildName := $(linux_buildName)-debug
else ifeq ($(CONFIG), RelWithDebInfo)
 linux_buildName := $(linux_buildName)-rdebug
endif

linux_targetPath := build/$(linux_buildName)
linux_execPath := $(linux_targetPath)/$(metadata_exec)
linux_cMakeCache := build/$(linux_archPreset)/CMakeCache.txt

$(linux_cMakeCache) : CMakeLists.txt
	@echo "Configuring Build"
	$(PRINT_CMD)cmake --preset $(linux_archPreset) --fresh

.PHONY: linux-build
linux-build : $(linux_cMakeCache)
	@echo "Building Executable"
	$(PRINT_CMD)cmake --build build/$(linux_archPreset) --config=$(CONFIG) $(VERBOSE_ARG)

linux_resourcePath := $(projectPath)/res/linux
resFiles := $(wildcard $(linux_resourcePath)/*)
ifneq ($(resFiles),)
 targetResFiles := $(addprefix $(linux_targetPath)/, $(notdir $(resFiles)))
 linux_bundleDeps += linux-resources

 .PHONY: linux-resources
 linux-resources: $(targetResFiles)

 $(linux_targetPath)/%: $(linux_resourcePath)/%
	@mkdir -p $(linux_targetPath)
	@echo "Linking Asset: $<"
	$(PRINT_CMD)ln -sfr $< $@
endif

.PHONY: linux-bundle
linux-bundle : $(linux_bundleDeps) linux-build