PROJECT_NAME := esp32-remote-garden
VERSION ?= latest
BUILD_DATE := $(shell date +%Y%m%d-%H%M%S)
COMMIT_HASH := $(shell git rev-parse --short HEAD 2>/dev/null || echo "unknown")

GITHUB_USER ?= $(shell echo $$REGISTRY_GITHUB_USER)
GITHUB_TOKEN ?= $(shell echo $$REGISTRY_GITHUB_TOKEN)
GITHUB_REPO ?= $(shell git remote get-url origin 2>/dev/null | sed 's/.*github.com[:/]\(.*\)\.git/\1/' || echo "owner/repo")

BUILD_DIR := build
RELEASE_DIR := release
ARTIFACTS_DIR := $(RELEASE_DIR)/$(PROJECT_NAME)-$(VERSION)-$(BUILD_DATE)

BOOTLOADER := $(BUILD_DIR)/bootloader/bootloader.bin
PARTITION_TABLE := $(BUILD_DIR)/partition_table/partition-table.bin
APP_BIN := $(BUILD_DIR)/$(PROJECT_NAME).bin
APP_ELF := $(BUILD_DIR)/$(PROJECT_NAME).elf
APP_MAP := $(BUILD_DIR)/$(PROJECT_NAME).map

ARCHIVE_NAME := $(PROJECT_NAME)-$(VERSION)-$(BUILD_DATE).tar.gz

.PHONY: help build clean fullclean flash monitor package upload release info setup

help:
	@echo "ESP32 Firmware Build & Deploy"
	@echo ""
	@echo "Available commands:"
	@echo "  make build      - Build firmware"
	@echo "  make clean      - Clean build"
	@echo "  make fullclean  - Full clean"
	@echo "  make flash      - Flash ESP32"
	@echo "  make monitor    - Run monitor"
	@echo "  make package    - Package firmware"
	@echo "  make upload        - Upload to GitHub Packages"
	@echo "  make release    - Full release (build + package + upload)"
	@echo "  make info       - Show build info"
	@echo ""
	@echo "Variables:"
	@echo "  GITHUB_REPO    = $(GITHUB_REPO)"
	@echo "  VERSION        = $(VERSION)"

build:
	@echo "==> Building firmware..."
	@. $(IDF_PATH)/export.sh && idf.py build

clean:
	idf.py clean

fullclean:
	idf.py fullclean
	rm -rf $(RELEASE_DIR)

flash:
	idf.py flash

monitor:
	idf.py monitor

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
	@echo "IDF Version: $$(. $(IDF_PATH)/export.sh && idf.py --version 2>/dev/null | head -n1)" >> $(ARTIFACTS_DIR)/build_info.txt
	@echo '#!/bin/bash' > $(ARTIFACTS_DIR)/flash.sh
	@echo 'PORT=$${1:-/dev/ttyUSB0}' >> $(ARTIFACTS_DIR)/flash.sh
	@echo 'esptool.py --chip esp32 --port $$PORT --baud 921600 \\' >> $(ARTIFACTS_DIR)/flash.sh
	@echo '  --before default_reset --after hard_reset write_flash -z \\' >> $(ARTIFACTS_DIR)/flash.sh
	@echo '  --flash_mode dio --flash_freq 40m --flash_size detect \\' >> $(ARTIFACTS_DIR)/flash.sh
	@echo '  0x1000 bootloader.bin \\' >> $(ARTIFACTS_DIR)/flash.sh
	@echo '  0x8000 partition-table.bin \\' >> $(ARTIFACTS_DIR)/flash.sh
	@echo '  0x10000 $(PROJECT_NAME).bin' >> $(ARTIFACTS_DIR)/flash.sh
	@chmod +x $(ARTIFACTS_DIR)/flash.sh
	@echo "" >> $(ARTIFACTS_DIR)/README.md
	@echo "# ESP32 Firmware $(VERSION)" >> $(ARTIFACTS_DIR)/README.md
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
	@echo "==> Uploading to GitHub Release..."
	@if [ -z "$(GITHUB_TOKEN)" ]; then \
		echo "ERROR: Set GITHUB_TOKEN environment variable"; \
		exit 1; \
	fi
	@echo "Creating tag $(VERSION)..."
	git tag -a $(VERSION) -m "Release $(VERSION)" || true
	@echo "Pushing tag $(VERSION)..."
	git push origin $(VERSION)
	@echo "Getting/creating release $(VERSION)..."
	@RESPONSE=$$(curl -s -H "Authorization: Bearer $(GITHUB_TOKEN)" \
		"https://api.github.com/repos/$(GITHUB_REPO)/releases/tags/$(VERSION)"); \
	RELEASE_ID=$$(echo "$$RESPONSE" | sed -n 's/.*"id": *\([0-9][0-9]*\).*/\1/p' | head -1); \
	if [ -z "$$RELEASE_ID" ]; then \
		echo "Creating new release $(VERSION)..."; \
		RESPONSE=$$(curl -s -X POST \
			-H "Authorization: Bearer $(GITHUB_TOKEN)" \
			-H "Content-Type: application/json" \
			-d '{"tag_name":"$(VERSION)","name":"Release $(VERSION)","body":"Firmware build $(BUILD_DATE) - Commit $(COMMIT_HASH)","draft":false,"prerelease":false}' \
			"https://api.github.com/repos/$(GITHUB_REPO)/releases"); \
		RELEASE_ID=$$(echo "$$RESPONSE" | sed -n 's/.*"id": *\([0-9][0-9]*\).*/\1/p' | head -1); \
	fi; \
	if [ -z "$$RELEASE_ID" ]; then \
		echo "ERROR: Failed to get release ID. Response:"; \
		echo "$$RESPONSE"; \
		exit 1; \
	fi; \
	echo "Release ID: $$RELEASE_ID"; \
	echo "Uploading $(ARCHIVE_NAME)..."; \
	curl -X POST \
		-H "Authorization: Bearer $(GITHUB_TOKEN)" \
		-H "Content-Type: application/gzip" \
		--data-binary @$(RELEASE_DIR)/$(ARCHIVE_NAME) \
		"https://uploads.github.com/repos/$(GITHUB_REPO)/releases/$$RELEASE_ID/assets?name=$(ARCHIVE_NAME)"
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
	@echo "=== GitHub Packages ==="
	@echo "Repository:   $(GITHUB_REPO)"
	@echo "Package URL:  https://maven.pkg.github.com/$(GITHUB_REPO)/esp32-firmware/$(VERSION)/$(ARCHIVE_NAME)"

setup:
	@echo "Setup instructions:"
	@echo ""
	@echo "1. Create GitHub Personal Access Token:"
	@echo "   https://github.com/settings/tokens"
	@echo "   Required scopes: repo, write:packages"
	@echo ""
	@echo "2. Add to ~/.bashrc or ~/.zshrc:"
	@echo ""
	@echo 'export GITHUB_TOKEN="ghp_xxxxxxxxxxxxx"'
	@echo ""
	@echo "3. Install GitHub CLI (if not installed):"
	@echo "   brew install gh          # macOS"
	@echo "   sudo apt install gh      # Ubuntu/Debian"
	@echo ""
	@echo "4. Run: source ~/.bashrc"