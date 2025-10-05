PROJECT_NAME := esp32-remote-garden
VERSION ?= latest
BUILD_DATE := $(shell date +%Y%m%d-%H%M%S)
COMMIT_HASH := $(shell git rev-parse --short HEAD 2>/dev/null || echo "unknown")

# Detect CI/CD environment
IS_CI := $(if $(CI),true,false)
IS_GITHUB_ACTIONS := $(if $(GITHUB_ACTIONS),true,false)
IS_GITLAB_CI := $(if $(GITLAB_CI),true,false)

# GitHub configuration
GITHUB_REPO ?= $(shell git remote get-url origin 2>/dev/null | sed 's/.*github.com[:/]\(.*\)\.git/\1/' || echo "owner/repo")

# GitHub CLI is required for uploads
GH_CLI := $(shell command -v gh 2>/dev/null)

BUILD_DIR := build
RELEASE_DIR := release
ARTIFACTS_DIR := $(RELEASE_DIR)/$(PROJECT_NAME)-$(VERSION)-$(BUILD_DATE)

BOOTLOADER := $(BUILD_DIR)/bootloader/bootloader.bin
PARTITION_TABLE := $(BUILD_DIR)/partition_table/partition-table.bin
APP_BIN := $(BUILD_DIR)/$(PROJECT_NAME).bin
APP_ELF := $(BUILD_DIR)/$(PROJECT_NAME).elf
APP_MAP := $(BUILD_DIR)/$(PROJECT_NAME).map

ARCHIVE_NAME := $(PROJECT_NAME)-$(VERSION)-$(BUILD_DATE).tar.gz

.PHONY: help build clean fullclean flash monitor package upload release info

help:
	@echo "ESP32 Firmware Build & Deploy (Local & CI/CD)"
	@echo ""
	@echo "Available commands:"
	@echo "  make build      - Build firmware"
	@echo "  make clean      - Clean build"
	@echo "  make fullclean  - Full clean"
	@echo "  make flash      - Flash ESP32 (local only)"
	@echo "  make monitor    - Run monitor (local only)"
	@echo "  make package    - Package firmware"
	@echo "  make upload     - Upload to GitHub (requires gh CLI)"
	@echo "  make release    - Full release (build + package + upload)"
	@echo "  make info       - Show build info"
	@echo ""
	@echo "Environment:"
	@echo "  CI/CD Mode:     $(IS_CI)"
	@echo "  GitHub Actions: $(IS_GITHUB_ACTIONS)"
	@echo "  GitLab CI:      $(IS_GITLAB_CI)"
	@echo "  Has gh CLI:     $(if $(GH_CLI),yes ($(GH_CLI)),no - install required!)"
	@echo ""
	@echo "Variables:"
	@echo "  GITHUB_REPO     = $(GITHUB_REPO)"
	@echo "  VERSION         = $(VERSION)"

build:
	@echo "==> Building firmware..."
ifeq ($(IS_CI),true)
	@echo "CI/CD mode detected - using direct idf.py"
	@idf.py build
else
	@echo "Local mode detected - sourcing ESP-IDF environment"
	@. $(IDF_PATH)/export.sh && idf.py build
endif

clean:
	idf.py clean

fullclean:
	idf.py fullclean
	rm -rf $(RELEASE_DIR)

flash:
ifeq ($(IS_CI),true)
	@echo "ERROR: Flash not available in CI/CD environment"
	@exit 1
else
	idf.py flash
endif

monitor:
ifeq ($(IS_CI),true)
	@echo "ERROR: Monitor not available in CI/CD environment"
	@exit 1
else
	idf.py monitor
endif

package: build
	@echo "==> Creating package..."
	@mkdir -p $(ARTIFACTS_DIR)
	@cp $(BOOTLOADER) $(ARTIFACTS_DIR)/ 2>/dev/null || true
	@cp $(PARTITION_TABLE) $(ARTIFACTS_DIR)/ 2>/dev/null || true
	@cp $(APP_BIN) $(ARTIFACTS_DIR)/ 2>/dev/null || true
	@cp $(APP_ELF) $(ARTIFACTS_DIR)/ 2>/dev/null || true
	@cp $(APP_MAP) $(ARTIFACTS_DIR)/ 2>/dev/null || true
	@find $(BUILD_DIR) -maxdepth 1 -name "*.bin" -exec cp {} $(ARTIFACTS_DIR)/ \;
	@echo "Project: $(PROJECT_NAME)" > $(ARTIFACTS_DIR)/build_info.txt
	@echo "Version: $(VERSION)" >> $(ARTIFACTS_DIR)/build_info.txt
	@echo "Build Date: $(BUILD_DATE)" >> $(ARTIFACTS_DIR)/build_info.txt
	@echo "Commit: $(COMMIT_HASH)" >> $(ARTIFACTS_DIR)/build_info.txt
	@echo "CI/CD: $(IS_CI)" >> $(ARTIFACTS_DIR)/build_info.txt
ifeq ($(IS_CI),true)
	@echo "IDF Version: $$(idf.py --version 2>/dev/null | head -n1)" >> $(ARTIFACTS_DIR)/build_info.txt
else
	@echo "IDF Version: $$(. $(IDF_PATH)/export.sh && idf.py --version 2>/dev/null | head -n1)" >> $(ARTIFACTS_DIR)/build_info.txt
endif
	@echo '#!/bin/bash' > $(ARTIFACTS_DIR)/flash.sh
	@echo 'PORT=$${1:-/dev/ttyUSB0}' >> $(ARTIFACTS_DIR)/flash.sh
	@echo 'esptool.py --chip esp32 --port $$PORT --baud 921600 \\' >> $(ARTIFACTS_DIR)/flash.sh
	@echo '  --before default_reset --after hard_reset write_flash -z \\' >> $(ARTIFACTS_DIR)/flash.sh
	@echo '  --flash_mode dio --flash_freq 40m --flash_size detect \\' >> $(ARTIFACTS_DIR)/flash.sh
	@echo '  0x1000 bootloader.bin \\' >> $(ARTIFACTS_DIR)/flash.sh
	@echo '  0x8000 partition-table.bin \\' >> $(ARTIFACTS_DIR)/flash.sh
	@echo '  0x10000 $(PROJECT_NAME).bin' >> $(ARTIFACTS_DIR)/flash.sh
	@chmod +x $(ARTIFACTS_DIR)/flash.sh
	@echo "" > $(ARTIFACTS_DIR)/README.md
	@echo "# ESP32 Firmware $(VERSION)" >> $(ARTIFACTS_DIR)/README.md
	@echo "" >> $(ARTIFACTS_DIR)/README.md
	@echo "Build: $(BUILD_DATE) | Commit: $(COMMIT_HASH)" >> $(ARTIFACTS_DIR)/README.md
	@echo "" >> $(ARTIFACTS_DIR)/README.md
	@echo "## Flash Instructions" >> $(ARTIFACTS_DIR)/README.md
	@echo "" >> $(ARTIFACTS_DIR)/README.md
	@echo "\`\`\`bash" >> $(ARTIFACTS_DIR)/README.md
	@echo "./flash.sh /dev/ttyUSB0" >> $(ARTIFACTS_DIR)/README.md
	@echo "\`\`\`" >> $(ARTIFACTS_DIR)/README.md
	@cd $(RELEASE_DIR) && tar -czf $(ARCHIVE_NAME) $(PROJECT_NAME)-$(VERSION)-$(BUILD_DATE)
	@echo "==> Package created: $(RELEASE_DIR)/$(ARCHIVE_NAME)"
	@ls -lh $(RELEASE_DIR)/$(ARCHIVE_NAME)

upload: package
	@echo "==> Uploading to GitHub using gh CLI..."
ifndef GH_CLI
	@echo "ERROR: GitHub CLI (gh) is not installed!"
	@exit 1
endif
	@echo "Checking authentication..."
	@gh auth status >/dev/null 2>&1 || (echo "ERROR: Not authenticated. Run: gh auth login" && exit 1)
	@echo "Checking for existing release $(VERSION)..."
	@gh release delete $(VERSION) --repo $(GITHUB_REPO) --yes 2>/dev/null || true
	@echo "Creating/updating release $(VERSION)..."
	@gh release create $(VERSION) \
		$(RELEASE_DIR)/$(ARCHIVE_NAME) \
		--repo $(GITHUB_REPO) \
		--title "Release $(VERSION)" \
		--notes "Firmware build $(BUILD_DATE) - Commit $(COMMIT_HASH)" \
		2>/dev/null || \
	gh release upload $(VERSION) $(RELEASE_DIR)/$(ARCHIVE_NAME) \
		--repo $(GITHUB_REPO) \
		--clobber
	@echo ""
	@echo "==> Upload complete!"
	@echo "View at: https://github.com/$(GITHUB_REPO)/releases/tag/$(VERSION)"

release: fullclean build package upload
	@echo "==> Release $(VERSION) complete!"

info:
	@echo "=== Build Info ==="
	@echo "Project:      $(PROJECT_NAME)"
	@echo "Version:      $(VERSION)"
	@echo "Build Date:   $(BUILD_DATE)"
	@echo "Commit:       $(COMMIT_HASH)"
	@echo "Archive:      $(ARCHIVE_NAME)"
	@echo ""
	@echo "=== Environment ==="
	@echo "CI/CD:        $(IS_CI)"
	@echo "GitHub Actions: $(IS_GITHUB_ACTIONS)"
	@echo "GitLab CI:    $(IS_GITLAB_CI)"
	@echo "Has gh CLI:   $(if $(GH_CLI),yes ($(GH_CLI)),no)"
	@echo ""
	@echo "=== GitHub ==="
	@echo "Repository:   $(GITHUB_REPO)"
	@echo "Release URL:  https://github.com/$(GITHUB_REPO)/releases/tag/$(VERSION)"

