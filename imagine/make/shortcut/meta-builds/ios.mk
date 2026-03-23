include $(IMAGINE_PATH)/make/config.mk
-include $(projectPath)/config.mk
include $(IMAGINE_PATH)/make/iOS-metadata.mk

.PHONY: all
all : ios-build

CONFIG ?= Release

ios_buildName := ios
ifeq ($(CONFIG), Debug)
 ios_buildName := $(ios_buildName)-debug
else ifeq ($(CONFIG), RelWithDebInfo)
 ios_buildName := $(ios_buildName)-rdebug
endif

ios_targetPath ?= build/$(ios_buildName)
ios_targetBinPath := $(ios_targetPath)/bin
ios_bundleDirectory = $(iOS_metadata_bundleName).app
ios_deviceAppBundlePath := /Applications/$(ios_bundleDirectory)
ios_deviceExecPath := $(ios_deviceAppBundlePath)/$(iOS_metadata_exec)
ios_resourcePath := $(projectPath)/res/ios
ios_iconPath := $(projectPath)/res/icons/iOS
ios_plistTxt := $(ios_targetPath)/Info.txt
ios_plist := $(ios_targetPath)/Info.plist
ios_icons := $(wildcard $(ios_iconPath)/*)
ifdef CCTOOLS_TOOCHAIN_PATH
LIPO := $(firstword $(wildcard $(CCTOOLS_TOOCHAIN_PATH)/bin/*-lipo))
PLISTUTIL := plistutil
else
LIPO := lipo
PLUTIL := plutil
endif
SSH := ssh -oHostKeyAlgorithms=+ssh-rsa
SCP := scp -oHostKeyAlgorithms=+ssh-rsa

# Host/IP of the iOS device to install the app over SSH
ios_installHost := iphone5

ios_arch ?= armv7 arm64
ifeq ($(filter armv7, $(ios_arch)),)
 ios_noARMv7 := 1
endif
ifeq ($(filter armv7s, $(ios_arch)),)
 ios_noARMv7s := 1
endif
ifeq ($(filter arm64, $(ios_arch)),)
 ios_noARM64 := 1
endif
ifeq ($(filter x86, $(ios_arch)),)
 ios_noX86 := 1
endif

ifndef ios_noARMv7

ios_armv7CMakeCache := build/ios-armv7/CMakeCache.txt
ios_armv7Exec := build/ios-armv7/$(CONFIG)/$(iOS_metadata_exec).app/$(iOS_metadata_exec)
ios_execs += $(ios_armv7Exec)
ios_targets += ios-armv7
ios_cleanTargets += ios-armv7-clean

$(ios_armv7CMakeCache) : CMakeLists.txt
	@echo "Configuring ARMv7 Executable"
	$(PRINT_CMD)cmake --preset ios-armv7 --fresh

.PHONY: ios-armv7
ios-armv7 : $(ios_armv7CMakeCache)
	@echo "Building ARMv7 Executable"
	$(PRINT_CMD)cmake --build build/ios-armv7 --config=$(CONFIG) $(VERBOSE_ARG)

.PHONY: ios-armv7-clean
ios-armv7-clean :
	@echo "Cleaning ARMv7 Build"
	$(PRINT_CMD)rm -r build/ios-armv7

.PHONY: ios-armv7-install
ios-armv7-install : $(ios_armv7Exec)
	$(SSH) root@$(ios_installHost) rm -f $(ios_deviceExecPath)
	$(SCP) $^ root@$(ios_installHost):$(ios_deviceExecPath)
	$(SSH) root@$(ios_installHost) chmod a+x $(ios_deviceExecPath)
ifdef iOS_metadata_setuid
	$(SSH) root@$(ios_installHost) chmod gu+s $(ios_deviceExecPath)
endif

endif

ifndef ios_noARM64

ios_arm64CMakeCache := build/ios-arm64/CMakeCache.txt
ios_arm64Exec := build/ios-arm64/$(CONFIG)/$(iOS_metadata_exec).app/$(iOS_metadata_exec)
ios_execs += $(ios_arm64Exec)
ios_targets += ios-arm64
ios_cleanTargets += ios-arm64-clean

$(ios_arm64CMakeCache) : CMakeLists.txt
	@echo "Configuring ARM64 Executable"
	$(PRINT_CMD)cmake --preset ios-arm64 --fresh

.PHONY: ios-arm64
ios-arm64 : $(ios_arm64CMakeCache)
	@echo "Building ARM64 Executable"
	$(PRINT_CMD)cmake --build build/ios-arm64 --config=$(CONFIG) $(VERBOSE_ARG)

.PHONY: ios-arm64-clean
ios-arm64-clean :
	@echo "Cleaning ARM64 Build"
	$(PRINT_CMD)rm -r build/ios-arm64

.PHONY: ios-arm64-install
ios-arm64-install : $(ios_arm64Exec)
	$(SSH) root@$(ios_installHost) rm -f $(ios_deviceExecPath)
	$(SCP) $^ root@$(ios_installHost):$(ios_deviceExecPath)
	$(SSH) root@$(ios_installHost) chmod a+x $(ios_deviceExecPath)
ifdef iOS_metadata_setuid
	$(SSH) root@$(ios_installHost) chmod gu+s $(ios_deviceExecPath)
endif

endif

ifndef ios_noX86

ios_x86CMakeCache := build/ios-x86/CMakeCache.txt
ios_x86Exec := build/ios-x86/$(CONFIG)/$(iOS_metadata_exec).app/$(iOS_metadata_exec)-x86
ios_execs += $(ios_x86Exec)
ios_targets += ios-x86
ios_cleanTargets += ios-x86-clean

$(ios_x86CMakeCache) : CMakeLists.txt
	@echo "Configuring X86 Executable"
	$(PRINT_CMD)cmake --preset ios-x86 --fresh

.PHONY: ios-x86
ios-x86 : $(ios_x86CMakeCache)
	@echo "Building X86 Executable"
	$(PRINT_CMD)cmake --build build/ios-x86 --config=$(CONFIG) $(VERBOSE_ARG)

.PHONY: ios-x86-clean
ios-x86-clean :
	@echo "Cleaning X86 Build"
	$(PRINT_CMD)rm -r build/ios-x86

endif

ios_fatExec := $(ios_targetBinPath)/$(iOS_metadata_exec)
$(ios_fatExec) : $(ios_targets)
	@mkdir -p $(@D)
	$(LIPO) -create $(ios_execs) -output $@

.PHONY: ios-build
ios-build : $(ios_fatExec) $(ios_plist)

ifdef iOS_metadata_setuid

ios_setuidLauncher := $(ios_resourcePath)/$(iOS_metadata_exec)_

$(ios_setuidLauncher) :
	echo '#!/bin/bash' > $@
	echo 'dir=$$(dirname "$$0")' >> $@
	echo 'exec "$${dir}"/$(iOS_metadata_exec) "$$@"' >> $@
	chmod a+x $@

endif

.PHONY: ios-resources-install
ios-resources-install : $(ios_plist) $(ios_setuidLauncher)
	$(SSH) root@$(ios_installHost) mkdir -p $(ios_deviceAppBundlePath)
	$(SCP) -r $(ios_resourcePath)/* root@$(ios_installHost):$(ios_deviceAppBundlePath)/
	$(SCP) $(ios_icons) root@$(ios_installHost):$(ios_deviceAppBundlePath)/
	$(SSH) root@$(ios_installHost) chmod -R a+r $(ios_deviceAppBundlePath)

.PHONY: ios-plist-install
ios-plist-install : $(ios_plist)
	$(SCP) $(ios_plist) root@$(ios_installHost):$(ios_deviceAppBundlePath)/

.PHONY: ios-install
ios-install : ios-build
	$(SSH) root@$(ios_installHost) rm -f $(ios_deviceExecPath)
	$(SCP) $(ios_fatExec) root@$(ios_installHost):$(ios_deviceExecPath)
	$(SSH) root@$(ios_installHost) chmod a+x $(ios_deviceExecPath)
ifdef iOS_metadata_setuid
	$(SSH) root@$(ios_installHost) chmod gu+s $(ios_deviceExecPath)
endif

.PHONY: ios-install-only
ios-install-only :
	$(SSH) root@$(ios_installHost) rm -f $(ios_deviceExecPath)
	$(SCP) $(ios_fatExec) root@$(ios_installHost):$(ios_deviceExecPath)
	$(SSH) root@$(ios_installHost) chmod a+x $(ios_deviceExecPath)
ifdef iOS_metadata_setuid
	$(SSH) root@$(ios_installHost) chmod gu+s $(ios_deviceExecPath)
endif

# metadata

$(ios_plistTxt) : $(projectPath)/metadata/conf.mk $(metadata_confDeps)
	@mkdir -p $(@D)
	bash $(IMAGINE_PATH)/tools/genIOSMeta.sh $(iOS_gen_metadata_args) $@
$(ios_plist) : $(ios_plistTxt)
	@mkdir -p $(@D)
ifdef PLISTUTIL
	$(PLISTUTIL) -o $@ -i $<
else
	$(PLUTIL) -convert binary1 -o $@ $<
endif
.PHONY: ios-metadata
ios-metadata : $(ios_plist)

# tar package

# Note: a version of tar with proper --transform support is needed for this rule (gnutar from MacPorts)
ios_tar := $(ios_targetPath)/$(iOS_metadata_bundleName)-$(iOS_metadata_version)-iOS.tar.gz
$(ios_tar) : # depends on $(ios_fatExec) $(ios_plist) $(ios_setuidLauncher)
	chmod a+x $(ios_fatExec)
ifdef iOS_metadata_setuid
	chmod gu+s $(ios_fatExec)
endif
	gnutar -cPhzf $@ $(ios_fatExec) $(ios_resourcePath)/* $(ios_icons)  $(ios_plist) \
	--transform='s,^$(ios_targetBinPath)/,$(ios_bundleDirectory)/,;s,^$(ios_resourcePath)/,$(ios_bundleDirectory)/,;s,^$(ios_iconPath)/,$(ios_bundleDirectory)/,;s,^$(ios_targetPath)/,$(ios_bundleDirectory)/,'
.PHONY: ios-tar
ios-tar : $(ios_tar)

.PHONY: ios-clean-tar
ios-clean-tar:
	rm -f $(ios_tar)

.PHONY: ios-ready
ios-ready : $(ios_tar)
	cp $(ios_tar) $(IMAGINE_PATH)/../releases-bin/iOS

.PHONY: ios-check
ios-check :
	@echo "Checking compiled version of $(iOS_metadata_bundleName) $(iOS_metadata_version)"
	strings $(ios_fatExec) | grep " $(iOS_metadata_version)"

.PHONY: ios-clean
ios-clean: $(ios_cleanTargets)
	rm -f $(ios_fatExec)
