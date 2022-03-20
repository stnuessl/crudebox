#
# The MIT License (MIT)
#
# Copyright (c) 2021  Steffen Nuessle
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

SHELL := bash -o pipefail
CC := clang
CXX := clang++
TAR := tar
PKGCONF := pkg-config

#
# Show / suppress compiler invocations. 
# Set 'Q :=' to show them.
# Set 'Q := @' to suppress compiler invocations.
#
Q :=

#
# Set name of the binary
#
BIN := crudebox

#
# Utility variables to deal with spaces
#
EMPTY :=
SPACE := $(EMPTY) $(EMPTY)


#
# Version information
#
VERSION_MAJOR	:= 0
VERSION_MINOR	:= 1
VERSION_PATCH	:= 0
VERSION_CORE	:= $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

#
# Paths for the build-, objects- and dependency-directories
#
BUILD_DIR		:= build
RELEASE_DIR		:= $(BUILD_DIR)/release
DEBUG_DIR		:= $(BUILD_DIR)/debug
GEN_DIR			:= $(BUILD_DIR)/gen


#
# Set installation directory used in 'make install'
#
DESTDIR := /usr/local/bin


ifndef BIN
$(error No binary name specified)
endif


#
# Use the current unix time as the build timestamp.
#
UNIX_TIME := $(shell date --utc +"%s")
BUILD_UUID := $(shell uuidgen --random)

ifdef ARTIFACTORY_API_KEY

OS_NAME := $(shell sed -E -n "s/^ID=([a-z0-9\._-]+)\s*$$/\1/p" /etc/os-release)
DATE	:= $(shell date --utc --date="@$(UNIX_TIME)" +"%Y-%m-%d")
TIME	:= $(shell date --utc --date="@$(UNIX_TIME)" +"%H:%M:%S")

ARTIFACTORY_UPLOAD_URL := \
	https://nuessle.jfrog.io/artifactory$\
	/crudebox-local$\
	;action=$(GITHUB_RUN_ID)$\
	;branch=$(notdir $(GITHUB_REF))$\
	;uuid=$(BUILD_UUID)$\
	;commit=$(GITHUB_SHA)$\
	;compiler=$(shell $(CC) --version | head -n 1 | tr -s " " "+")$\
	;date=$(DATE)$\
	;time=$(TIME)$\
	;timezone=utc$\
	;job=$(GITHUB_JOB)$\
	;os=$(OS_NAME)$\
	;version=$(VERSION_CORE)$\
	;xdg_session_type=$(XDG_SESSION_TYPE)$\
	/$(OS_NAME)$\
	/$(XDG_SESSION_TYPE)$\
	/$(notdir $(CC))$\
	/$(DATE)$\
	/$(TIME)

endif

#
# Specifiy the additional wayland protocols
#
XDG_SHELL := /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml
XDG_GEN := $(GEN_DIR)/xdg-shell
XDG_SRC := $(XDG_GEN)/xdg-shell.c
XDG_HDR := $(XDG_GEN)/xdg-shell-client.h

#
# Specify all source files. The paths should be relative to this file.
#
REG_SRC := $(shell find src/ -iname "*.c")
GEN_SRC := $(XDG_SRC)
SRC := $(REG_SRC) $(GEN_SRC)

# 
# Optional: This variable is used by the 'format' and 'tags' targets 
# which are not necessary to build the target.
#
REG_HDR := $(shell find src/ -iname "*.h")
GEN_HDR := $(XDG_HDR)
HDR := $(REG_HDR) $(GEN_HDR)

ifndef SRC
$(error No source files specified)
endif

#
# Define variable which can be used to check if the clang compiler
# family is used for this invocation of make.
#
CLANG := $(findstring clang,$(CC))

#
# Uncomment if 'VPATH' is needed. 'VPATH' is a list of directories in which
# make searches for source files.
#
# VPATH		:= $(subst $(SPACE),:,$(sort $(dir $(SRC))))

RELPATHS	:= $(filter ../%, $(SRC))
ifdef RELPATHS

NAMES		:= $(notdir $(RELPATHS))
UNIQUE		:= $(sort $(NAMES))

#
# Check for duplicate file names (not regarding directories).
#
ifneq ($(words $(NAMES)),$(words $(UNIQUE)))
DUPS		:= $(shell printf "$(NAMES)" | tr -s " " "\n" | sort --unique)
DIRS		:= $(dir $(filter %$(DUPS), $(SRC)))
$(error Detected name duplicates in relative paths [ $(DUPS) ] - [ $(DIRS) ])
endif

#
# Only use file name as the source location and add the relative path to 'VPATH'
# This prevents object files to reside in paths like 'build/src/../relative/' or
# even worse 'build/src/../../relative' which would be a path outside of
# the specified build directory
#
SRC			:= $(filter-out ../%, $(SRC)) $(notdir $(RELPATHS))
VPATH		:= $(subst $(SPACE),:, $(dir $(RELPATHS)))
endif

#
# Define all object and dependency files from $(SRC) and get
# a list of all inhabited directories. Special care is taken to prevent 
# file paths like "build/./src/main.o"
#

GEN_C_SRC := $(filter %.c,$(GEN_SRC))
REG_C_SRC := $(filter %.c,$(REG_SRC))
GEN_CXX_SRC := $(filter %.cpp,$(GEN_SRC))
REG_CXX_SRC := $(filter %.cpp,$(REG_SRC))

RELEASE_OBJS := \
	$(patsubst $(BUILD_DIR)/%.c,$(RELEASE_DIR)/%.o,$(GEN_C_SRC)) \
	$(patsubst $(BUILD_DIR)/%.cpp,$(RELEASE_DIR)/%.o,$(GEN_CXX_SRC)) \
	$(patsubst %.c,$(RELEASE_DIR)/%.o,$(REG_C_SRC)) \
	$(patsubst %.cpp,$(RELEASE_DIR)/%.o,$(REG_CXX_SRC))

DEBUG_OBJS := \
	$(patsubst $(RELEASE_DIR)/%.o,$(DEBUG_DIR)/%.o,$(RELEASE_OBJS))

#
# Paths to default targets.
#
RELEASE_BIN		:= $(RELEASE_DIR)/$(BIN)
DEBUG_BIN		:= $(DEBUG_DIR)/$(BIN)
RELEASE_CMDS	:= $(RELEASE_DIR)/compile_commands.json
DEBUG_CMDS		:= $(DEBUG_DIR)/compile_commands.json
ENVFILE			:= $(BUILD_DIR)/env.txt
OS_RELEASE		:= $(BUILD_DIR)/os-release.txt
TARBALL			:= $(BUILD_DIR)/$(BIN)-$(VERSION_CORE).tar.gz

DIRS := \
	$(BUILD_DIR) \
	$(DEBUG_DIR) \
	$(RELEASE_DIR) \
	$(GEN_DIR) \
	$(sort $(dir $(DEBUG_OBJS))) \
	$(sort $(dir $(RELEASE_OBJS))) \
	$(sort $(dir $(GEN_SRC) $(GEN_HDR))) \


#
# Define dependency files
#
DEPS := $(patsubst %.o,%.d,$(DEBUG_OBJS) $(RELEASE_OBJS))

VERSION_FILE := $(BUILD_DIR)/versions.txt
VERSION_LIST := \
	"$$(make --version)" \
	"$$($(CC) --version)" \
	"$$(curl --version)" \
	"$$($(TAR) --version)" \
	"$$(column --version)" \
	"$$(sed --version)"

#
# Variables for the clang analyzer
#
ifneq (,$(shell type -fP clang-extdef-mapping))
ANALYZER := clang --analyze
ANALYZER_DIR := $(BUILD_DIR)/clang-analyzer
ANALYZER_FILES := \
	$(patsubst $(RELEASE_DIR)/%.o,$(ANALYZER_DIR)/%.txt,$(RELEASE_OBJS))

ANALYZER_FLAGS = \
	--analyzer-output html \
	--output $(basename $@) \
	-Xclang -analyzer-config -Xclang ctu-dir=$(ANALYZER_DIR) \
	-Xclang -analyzer-config -Xclang enable-naive-ctu-analysis=true \
	-Xclang -analyzer-config -Xclang display-ctu-progress=true \
	$(DEFS) \
	$(INC) \
	$(CFLAGS)

ANALYZER_DEFMAP := $(ANALYZER_DIR)/externalDefMap.txt
ANALYZER_OUTPUT := $(BUILD_DIR)/clang-analysis

DIRS += $(ANALYZER_DIR) $(sort $(dir $(ANALYZER_FILES)))
ifndef CLANG
VERSION_LIST += "$$($(ANALYZER) --version)"
endif
endif

#
# Variables for cppcheck
#
ifneq (,$(shell type -fP cppcheck))
CPPCHECK := cppcheck
CPPCHECK_DIR := $(BUILD_DIR)/cppcheck-analysis
CPPCHECK_FLAGS := \
	--cppcheck-build-dir=$(CPPCHECK_DIR) \
	--enable=all \
	--inconclusive \
	--inline-suppr \
	--library=posix \
	--platform=native \
	--suppress=allocaCalled \
	--suppress=missingInclude \
	--suppress=readdirCalled \
	--suppress=unusedFunction \
	--template=gcc \
	--verbose \
	--xml

CPPCHECK_OUTPUT := $(BUILD_DIR)/cppcheck
CPPCHECK_RESULTS := $(CPPCHECK_DIR)/cppcheck.xml

DIRS += $(CPPCHECK_DIR)
VERSION_LIST += "$$(cppcheck --version)"
endif

#
# Variables for shellcheck
#
ifneq (,$(shell type -fP shellcheck))
SHELLCHECK_DIR := $(BUILD_DIR)/shellcheck
SHELLCHECK := shellcheck
SHELLCHECK_FLAGS := \
	--color=auto \
	--external-sources \
	--format gcc \
	--enable all \
	--norc \
	--shell $(firstword $(notdir $(SHELL)))

SHELLCHECK_INPUT := \
	$(shell find . -name "*.sh") \
	hooks/pre-commit

SHELLCHECK_OUTPUT := $(SHELLCHECK_DIR)/shellcheck.txt

DIRS += $(SHELLCHECK_DIR)
VERSION_LIST += "$$(shellcheck --version)"
endif

#
# Add additional prepocessor macro definitions
#
DEFS := \
	-D_GNU_SOURCE \
	-DCRUDEBOX_VERSION_MAJOR=\"$(VERSION_MAJOR)\" \
	-DCRUDEBOX_VERSION_MINOR=\"$(VERSION_MINOR)\" \
	-DCRUDEBOX_VERSION_PATCH=\"$(VERSION_PATCH)\" \
	-DCOPYRIGHT_YEAR=\"$(shell date --date "@$(UNIX_TIME)" +"%Y")\"


#
# Automatically detect which windowing system is to be used
#
ifeq (x11,$(XDG_SESSION_TYPE))
DEFS += -DCONFIG_USE_X11
else ifeq (wayland,$(XDG_SESSION_TYPE))
DEFS += -DCONFIG_USE_WAYLAND
else
$(error Unknown "XDG_SESSION_TYPE": Use either "x11" or "wayland")
endif

#
# Add additional include paths
#
INC := \
	-I$(XDG_GEN)


#
# Add used libraries which are configurable with pkg-config
#
PKGCONF_LIBS := \
	cairo \
	freetype2 \
	wayland-client \
	xcb \
	xcb-keysyms \
	xkbcommon \
#	gstreamer-1.0 \
#	gstreamer-pbutils-1.0 \
#	libcurl \
#	libxml-2.0 \

#
# Set non-pkg-configurable libraries flags 
#
LDLIBS := \
#	-lstdc++fs \
#	-lm \

#
# Set linker flags, here: 'rpath' for libraries in non-standard directories
# If '-shared' is specified: '-fpic' or '-fPIC' should be set here 
# as in the CFLAGS / CXXFLAGS
#
LDFLAGS := \
	-pthread \
#	-Wl,--end-group \
#	-Wl,--start-group \
#	-Wl,-Map=$@.map \
#	-Wl,-rpath,/usr/local/lib \
#	-fPIC \
#	-fpic \
#	-shared \


#
# Set the preprocessor flags and also generate a dependency 
# file "$(DEPS)" for each processed translation unit.
#

CPPFLAGS = \
	-MMD \
	-MF $(patsubst %.o,%.d,$@) \
	-MT $@ \
	$(DEFS) \
	$(INC) 


#
# Set compiler flags that you want to be present for every make invocation.
# Specific flags for release and debug builds can be added later on
# with target-specific variable values.
#
CFLAGS := \
	-std=c11 \
	-Wall \
	-Wextra \
	-pedantic \
	-fstack-protector-strong \
	-fno-plt \
#	-Werror \
#	-fpic \
#	-fno-omit-frame-pointer \


CXXFLAGS := \
	-std=c++14 \
	-Wall \
	-Wextra \
	-pedantic \
	-fstack-protector-strong \
	-fno-plt \
	-Werror \
#	-fpic \
#	-Weffc++ \
#	-fvisibility-inlines-hidden \
#	-fno-rtti \
#	-fno-omit-frame-pointer \


TAR_FLAGS = \
	--create \
	--file $@ \
	--gzip


#
# Enable addtional targets if there are pkgconf libraries defined
#
ifneq (,$(shell type -f $(PKGCONF)))
ifneq (,$(PKGCONF_LIBS))
PKGCONF_CHECK	:= pkgconf-check
PKGCONF_DIR		:= $(BUILD_DIR)/pkgconf
PKGCONF_VERSION	:= $(PKGCONF_DIR)/version.txt
PKGCONF_DATA	:= $(PKGCONF_DIR)/libs.json

DIRS += $(PKGCONF_DIR)
VERSION_LIST += "$(PKGCONF) $$($(PKGCONF) --version)"

#
# Add build flags for all required libraries
#
CFLAGS		+= $(shell $(PKGCONF) --cflags $(PKGCONF_LIBS))
CXXFLAGS	+= $(shell $(PKGCONF) --cflags $(PKGCONF_LIBS))
LDLIBS		+= $(shell $(PKGCONF) --libs $(PKGCONF_LIBS))
endif
endif


#
# Append extra arguments passed on the command-line
#
CPPFLAGS	+= $(EXTRA_CPPFLAGS)
CFLAGS		+= $(EXTRA_CFLAGS)
CXXFLAGS	+= $(EXTRA_CXXFLAGS)
LDFLAGS		+= $(EXTRA_LDFLAGS)

ANALYZER_FLAGS	+= $(EXTRA_ANALYZER_FLAGS)
CPPCHECK_FLAGS	+= $(EXTRA_CPPCHECK_FLAGS)


#
# Define unit test targets
#
ifeq (,$(shell $(PKGCONF) --print-errors --exists criterion 2>&1))
UT_DIR := $(BUILD_DIR)/test
UT_REPORT := $(UT_DIR)/report.xml
UT_BIN := $(UT_DIR)/crudebox
UT_CPUS := $(shell nproc)
UT_SRC := $(shell find test/ -name "*.c")

UT_OBJS := \
	$(patsubst %.c,$(UT_DIR)/%.o,$(UT_SRC)) \
	$(patsubst $(RELEASE_DIR)/%.o,$(UT_DIR)/%.o,$(RELEASE_OBJS))

DIRS += \
	$(UT_DIR) \
	$(sort $(dir $(UT_OBJS)))

ifneq (,$(shell type -fP lcov))
UT_INFO := $(UT_DIR)/crudebox.info
UT_COV := $(UT_DIR)/coverage

VERSION_LIST += $$(lcov --version)
endif
endif



#
# Setting terminal colors
#

ifneq ($(MAKEFILE_COLOR), 0)

RED			:= \e[1;31m
GREEN		:= \e[1;32m
YELLOW		:= \e[1;33m
BLUE		:= \e[1;34m
MAGENTA		:= \e[1;35m
CYAN		:= \e[1;36m
RESET		:= \e[0m

endif


all: release $(RELEASE_CMDS) tags 
release: $(RELEASE_BIN)
debug: $(DEBUG_BIN)

#
# Note that if "-flto" is specified you may want to pass the optimization
# flags used for compiling to the linker (as done below).
#

analysis-build: CPPFLAGS	+= -DNDEBUG -DTIMING_ANALYSIS
analysis-build: CFLAGS		+= -fno-omit-frame-pointer
analysis-build: CXXFLAGS	+= -fno-omit-frame-pointer
analysis-build: release

$(RELEASE_BIN): CPPFLAGS	+= -DNDEBUG
$(RELEASE_BIN): CFLAGS		+= -O2 -flto -fdata-sections -ffunction-sections
$(RELEASE_BIN): CXXFLAGS	+= -O2 -flto -fdata-sections -ffunction-sections
$(RELEASE_BIN): LDFLAGS		+= -O2 -flto -Wl,--gc-sections

$(DEBUG_BIN): CPPFLAGS		+= -DMEM_NOLEAK
$(DEBUG_BIN): CFLAGS		+= -Og -g2
$(DEBUG_BIN): CXXFLAGS		+= -Og -g2

$(UT_BIN): CPPFLAGS 		+= -DUNIT_TEST_ -Isrc/
$(UT_BIN): CFLAGS			+= -Og -g2 -ftest-coverage -fprofile-arcs
$(UT_BIN): CXXFLAGS			+= -Og -g2 -ftest-coverage -fprofile-arcs
$(UT_BIN): LDLIBS			+= $(shell $(PKGCONF) --libs criterion)
$(UT_BIN): LDFLAGS			+= -lgcov

syntax-check: CFLAGS   += -fsyntax-only
syntax-check: CXXFLAGS += -fsyntax-only
syntax-check: $(DEBUG_OBJS)

$(RELEASE_BIN): $(RELEASE_OBJS)
	@printf "$(YELLOW)Linking [ $@ ]$(RESET)\n"
	$(Q)$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS) 
	$(Q)strip --strip-all $@
	@printf "$(GREEN)Built target [ $@ ]$(RESET)\n"
	@sha256sum --tag $@

$(DEBUG_BIN): $(DEBUG_OBJS)
	@printf "$(YELLOW)Linking [ $@ ]$(RESET)\n"
	$(Q)$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)
	@printf "$(GREEN)Built target [ $@ ]$(RESET)\n"
	@sha256sum --tag $@

-include $(DEPS)

src/wl-window.c: $(XDG_HDR)

$(RELEASE_DIR)/%.o: %.c
	@printf "$(BLUE)Building [ $@ ]$(RESET)\n"
ifdef CLANG
	$(Q)$(CC) -c -o $@ -MJ $(patsubst %.o,%.json,$@) $(CPPFLAGS) $(CFLAGS) $<
else
	$(Q)$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) $<
	@printf '%s' \
		'{' \
			'"directory":"$(CURDIR)",' \
			'"file":"$<",' \
			'"output":"$@",' \
			'"arguments": [' \
				$(foreach x,$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS),'"$(x)",') \
				'"$<"' \
			']' \
		'},' \
	> $(RELEASE_DIR)/$*.json
endif

$(RELEASE_DIR)/%.o: $(BUILD_DIR)/%.c
	@printf "$(BLUE)Building [ $@ ]$(RESET)\n"
ifdef CLANG
	$(Q)$(CC) -c -o $@ -MJ $(patsubst %.o,%.json,$@) $(CPPFLAGS) $(CFLAGS) $<
else
	$(Q)$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) $<
	@printf '%s' \
		'{' \
			'"directory":"$(CURDIR)",' \
			'"file":"$<",' \
			'"output":"$@",' \
			'"arguments": [' \
				$(foreach x,$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS),'"$(x)",') \
				'"$<"' \
			']' \
		'},' \
	> $(RELEASE_DIR)/$*.json
endif

$(DEBUG_DIR)/%.o: %.c
	@printf "$(BLUE)Building [ $@ ]$(RESET)\n"
ifdef CLANG
	$(Q)$(CC) -c -o $@ -MJ $(patsubst %.o,%.json,$@) $(CPPFLAGS) $(CFLAGS) $<
else
	$(Q)$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) $<
	@printf '%s' \
		'{' \
			'"directory":"$(CURDIR)",' \
			'"file":"$<",' \
			'"output":"$@",' \
			'"arguments": [' \
				$(foreach x,$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS),'"$(x)",') \
				'"$<"' \
			']' \
		'},' \
	> $(DEBUG_DIR)/$*.json
endif

$(DEBUG_DIR)/%.o: $(BUILD_DIR)/%.c
	@printf "$(BLUE)Building [ $@ ]$(RESET)\n"
ifdef CLANG
	$(Q)$(CC) -c -o $@ -MJ $(patsubst %.o,%.json,$@) $(CPPFLAGS) $(CFLAGS) $<
else
	$(Q)$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) $<
	@printf '%s' \
		'{' \
			'"directory":"$(CURDIR)",' \
			'"file":"$<",' \
			'"output":"$@",' \
			'"arguments": [' \
				$(foreach x,$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS),'"$(x)",') \
				'"$<"' \
			']' \
		'},' \
	> $(DEBUG_DIR)/$*.json
endif

$(XDG_SRC): $(XDG_SHELL) | $(DIRS)
	@printf "$(CYAN)Generating [ $@ ]$(RESET)\n"
	$(Q)wayland-scanner private-code $< $@

$(XDG_HDR): $(XDG_SHELL) | $(DIRS)
	@printf "$(CYAN)Generating [ $@ ]$(RESET)\n"
	$(Q)wayland-scanner client-header $< $@

$(RELEASE_OBJS) $(DEBUG_OBJS): | $(DIRS)

$(DIRS):
	mkdir -p $@

$(RELEASE_CMDS): $(RELEASE_OBJS)
	sed -e '1s/^/[/' -e '$$s/,\s*$$/]/' $(patsubst %.o,%.json,$^) \
		> $@ || (rm -f $@ && false)

$(DEBUG_CMDS): $(DEBUG_OBJS)
	sed -e '1s/^/[/' -e '$$s/,\s*$$/]/' $(patsubst %.o,%.json,$^) \
		> $@ || (rm -f $@ && false)

$(UT_COV): $(UT_INFO)
	@printf "$(MAGENTA)Generating [ $@ ]$(RESET)\n"
	genhtml \
		--output-directory $@ \
		--rc geninfo_auto_base=1 \
		$<

$(UT_INFO): $(UT_REPORT)
	@printf "$(MAGENTA)Generating [ $@ ]$(RESET)\n"
	lcov \
		--test-name $(BIN) \
		--base-directory $(CURDIR) \
		--directory $(UT_DIR) \
		--capture \
		$(if $(GITHUB_SHA),$(GITHUB_SHA)) \
		--output-file $@

$(UT_REPORT): $(UT_BIN)
	$(UT_BIN) \
		--jobs $(UT_CPUS) \
		--xml=$@ \
		--tap

$(UT_BIN): $(UT_OBJS) 
	@printf "$(YELLOW)Linking [ $@ ]$(RESET)\n"
	$(Q)gcc -o $@ $^ $(LDLIBS) $(LDFLAGS)
	@printf "$(GREEN)Built target [ $@ ]$(RESET)\n"

$(UT_DIR)/%.o: %.c
	@printf "$(BLUE)Building [ $@ ]$(RESET)\n"
	$(Q)gcc -c -o $@ $(CPPFLAGS) $(CFLAGS) $<

$(UT_DIR)/%.o: $(BUILD_DIR)/%.c
	@printf "$(BLUE)Building [ $@ ]$(RESET)\n"
	$(Q)gcc -c -o $@ $(CPPFLAGS) $(CFLAGS) $<


$(UT_OBJS): | $(DIRS)

clean:
	rm -rf $(BUILD_DIR)

format:
	$(SHELL) scripts/mk-format.sh

tags: $(HDR) $(SRC)
	ctags -f tags $^

install: $(RELEASE_BIN)
	cp $< $(DESTDIR)

uninstall:
	rm -f $(DESTDIR)/$(BIN)

$(ENVFILE): | $(DIRS)
	@env \
		$(if $(ARTIFACTORY_API_KEY),ARTIFACTORY_API_KEY=) \
		$(if $(DOCKER_USERNAME),DOCKER_USERNAME=) \
		$(if $(DOCKER_PASSWORD),DOCKER_PASSWORD=) \
		> $@

$(OS_RELEASE): /etc/os-release | $(DIRS)
	cp -f $< $@

$(VERSION_FILE): | $(DIRS)
	@printf "%s\n--\n" $(VERSION_LIST) > $@ || (rm -f $@ && false)

$(PKGCONF_CHECK): $(PKGCONF_VERSION) $(PKGCONF_DATA)
	@printf "$(GREEN)Performed [ $@ ]$(RESET)\n"

$(PKGCONF_VERSION): | $(DIRS)
	@printf "$(BLUE)Generating [ $@ ]$(RESET)\n"
	$(Q)$(PKGCONF) --version 2>&1 > $@ || (rm -f $@ && false)

$(PKGCONF_DATA): | $(DIRS)
	@printf "$(BLUE)Generating [ $@ ]$(RESET)\n"
	$(Q)$(PKGCONF) --print-errors --print-provides $(PKGCONF_LIBS) \
		| tr --squeeze-repeats " =" " " \
		| column --table-name pkgconf --table-columns library,version --json \
		| tee $@ || (rm -f $@ && false)

clang-analysis: $(ANALYZER_FILES)

$(ANALYZER_OUTPUT): | $(ANALYZER_FILES)
	ln --relative --symbolic --force  $(ANALYZER_DIR) $@

$(ANALYZER_DIR)/%.txt: %.c $(ANALYZER_DEFMAP)
	@printf "$(BLUE)Generating [ $@ ]$(RESET)\n"
	$(Q)clang --analyze  $(ANALYZER_FLAGS) $< 2>&1 | tee $@

$(ANALYZER_DIR)/%.txt: $(BUILD_DIR)/%.c $(ANALYZER_DEFMAP)
	@printf "$(BLUE)Generating [ $@ ]$(RESET)\n"
	$(Q)$(ANALYZER) $(ANALYZER_FLAGS) $< 2>&1 | tee $@

$(ANALYZER_DEFMAP): $(DEBUG_CMDS) $(SRC)
	$(Q)clang-extdef-mapping -p $(DEBUG_CMDS) $(SRC) > $@ || (rm -f $@ && false)

$(CPPCHECK): $(CPPCHECK_OUTPUT)

$(CPPCHECK_RESULTS): $(DEBUG_CMDS) | $(DIRS)
	@printf "$(YELLOW)Analyzing project [ $< ]$(RESET)\n"
	$(Q)cppcheck $(CPPCHECK_FLAGS) --project=$< --output-file=$@

$(CPPCHECK_OUTPUT): $(CPPCHECK_RESULTS) 
	@printf "$(YELLOW)Generating report [ $@ ]$(RESET)\n"
	@rm -rf $@
	$(Q)cppcheck-htmlreport --file=$< --title=$(BIN) --report-dir=$@

$(SHELLCHECK): $(SHELLCHECK_OUTPUT)

$(SHELLCHECK_OUTPUT): $(SHELLCHECK_INPUT) | $(DIRS)
	$(Q)shellcheck $(SHELLCHECK_FLAGS) $^ | tee $@ || (rm -f $@ && false)

$(TARBALL): \
		$(RELEASE_BIN) \
		$(RELEASE_CMDS) \
		$(DEBUG_BIN) \
		$(DEBUG_CMDS) \
		$(ANALYZER_OUTPUT) \
		$(CPPCHECK_OUTPUT) \
		$(ENVFILE) \
		$(OS_RELEASE) \
		$(PKGCONF_DATA) \
		$(PKGCONF_VERSION) \
		$(SHELLCHECK_OUTPUT) \
		$(UT_TARGET) \
		$(UT_COV) \
		$(VERSION_FILE)
	@printf "$(MAGENTA)Packaging [ $@ ]$(RESET)\n"
	$(Q)find -H $^ -type f -size +0 \
		| sed -e 's/$(@D)\///g' \
		| $(TAR) $(TAR_FLAGS) --directory $(@D) --files-from -

artifactory-upload: $(TARBALL)
	@printf "$(MAGENTA)Uploading [ $^ ]$(RESET)\n"
ifdef ARTIFACTORY_API_KEY
	$(Q)curl \
		--silent \
		--show-error \
		--write-out "\n" \
		--request PUT \
		--header "X-JFrog-Art-Api:${ARTIFACTORY_API_KEY}" \
		--header "X-Checksum-Deploy:false" \
		--header "X-Checksum-Sha256:$$(sha256sum $^ | cut --fields=1 -d " ")" \
		--header "X-Checksum-Sha1:$$(sha1sum $^ | cut --fields=1 -d " ")" \
		--upload-file $^ \
		"$(ARTIFACTORY_UPLOAD_URL)/$(^F)"
	@printf "$(GREEN)Uploaded [ $^ ]$(RESET)\n"
else
	@printf "** ERROR: $@: \"ARTIFACTORY_API_KEY\" not specified\n"
	@false
endif


.PHONY: \
	$(ENVFILE) \
	$(PKGCONF_CHECK) \
	$(SHELLCHECK) \
	all \
	artifactory-upload \
	clang-analysis \
	clean \
	debug \
	format \
	install \
	release \
	syntax-check \
	uninstall

.SILENT: \
	$(ANALYZER_OUTPUT) \
	clean \
	format \
	$(RELEASE_CMDS) \
	$(DEBUG_CMDS) \
	tags \
	$(DIRS)

