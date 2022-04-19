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
# Set the source directory
#
srcdir := src


#
# Utility variables to deal with spaces
#
empty :=
space := $(empty) $(empty)


#
# Version information
#
version_major	:= 0
version_minor	:= 1
version_patch	:= 0
version_core	:= $(version_major).$(version_minor).$(version_patch)

#
# Paths for the build-, objects- and dependency-directories
#
BUILD_DIR		:= build
release_dir		:= $(BUILD_DIR)/release
debug_dir		:= $(BUILD_DIR)/debug
gen_dir			:= $(BUILD_DIR)/gen
test_dir		:= $(BUILD_DIR)/test


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

ifdef ARTIFACTORY_API_KEY

os_name := $(shell sed -E -n "s/^ID=([a-z0-9\._-]+)\s*$$/\1/p" /etc/os-release)
date	:= $(shell date --utc --date="@$(UNIX_TIME)" +"%Y-%m-%d")
time	:= $(shell date --utc --date="@$(UNIX_TIME)" +"%H:%M:%S")

artifactory_upload_url := \
	https://nuessle.jfrog.io/artifactory$\
	/crudebox-local$\
	;action=$(GITHUB_RUN_ID)$\
	;branch=$(notdir $(GITHUB_REF))$\
	;uuid=$(shell uuidgen --random)$\
	;commit=$(GITHUB_SHA)$\
	;compiler=$(shell $(CC) --version | head -n 1 | tr -s " " "+")$\
	;date=$(date)$\
	;time=$(time)$\
	;timezone=utc$\
	;job=$(GITHUB_JOB)$\
	;os=$(os_name)$\
	;version=$(version_core)$\
	;xdg_session_type=$(XDG_SESSION_TYPE)$\
	/$(os_name)$\
	/$(XDG_SESSION_TYPE)$\
	/$(notdir $(CC))$\
	/$(date)$\
	/$(time)

endif

#
# Specifiy the additional wayland protocols
#
xdg_shell := /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml
xdg_dir := $(gen_dir)/xdg-shell
xdg_src := $(xdg_dir)/xdg-shell.c
xdg_hdr := $(xdg_dir)/xdg-shell-client.h

#
# Specify all source files. The paths should be relative to this file.
#
reg_src := $(shell find $(srcdir) -iname "*.c")
gen_src := $(xdg_src)
src := $(reg_src) $(gen_src)

# 
# Optional: This variable is used by the 'format' and 'tags' targets 
# which are not necessary to build the target.
#
reg_hdr := $(shell find $(srcdir) -iname "*.h")
gen_hdr := $(xdg_hdr)
hdr := $(reg_hdr) $(gen_hdr)

ifndef src
$(error No source files specified)
endif

#
# Define variable which can be used to check if the clang compiler
# family is used for this invocation of make.
#
clang_used := $(findstring clang,$(CC))

#
# Uncomment if 'VPATH' is needed. 'VPATH' is a list of directories in which
# make searches for source files.
#
# VPATH		:= $(subst $(space),:,$(sort $(dir $(src))))

RELPATHS	:= $(filter ../%, $(src))
ifdef RELPATHS

NAMES		:= $(notdir $(RELPATHS))
UNIQUE		:= $(sort $(NAMES))

#
# Check for duplicate file names (not regarding directories).
#
ifneq ($(words $(NAMES)),$(words $(UNIQUE)))
DUPS		:= $(shell printf "$(NAMES)" | tr -s " " "\n" | sort --unique)
dirs		:= $(dir $(filter %$(DUPS), $(src)))
$(error Detected name duplicates in relative paths [ $(DUPS) ] - [ $(dirs) ])
endif

#
# Only use file name as the source location and add the relative path to 'VPATH'
# This prevents object files to reside in paths like 'build/src/../relative/' or
# even worse 'build/src/../../relative' which would be a path outside of
# the specified build directory
#
src			:= $(filter-out ../%, $(src)) $(notdir $(RELPATHS))
VPATH		:= $(subst $(space),:, $(dir $(RELPATHS)))
endif

#
# Define all object and dependency files from $(src) and get
# a list of all inhabited directories. Special care is taken to prevent 
# file paths like "build/./src/main.o"
#

gen_src_c := $(filter %.c,$(gen_src))
reg_src_c := $(filter %.c,$(reg_src))
gen_src_cxx := $(filter %.cpp,$(gen_src))
reg_src_cxx := $(filter %.cpp,$(reg_src))

release_objs := \
	$(patsubst $(BUILD_DIR)/%.c,$(release_dir)/%.o,$(gen_src_c)) \
	$(patsubst $(BUILD_DIR)/%.cpp,$(release_dir)/%.o,$(gen_src_cxx)) \
	$(patsubst %.c,$(release_dir)/%.o,$(reg_src_c)) \
	$(patsubst %.cpp,$(release_dir)/%.o,$(reg_src_cxx))

debug_objs := \
	$(patsubst $(release_dir)/%.o,$(debug_dir)/%.o,$(release_objs))

#
# Paths to default targets.
#
release_bin		:= $(release_dir)/$(BIN)
debug_bin		:= $(debug_dir)/$(BIN)
release_cmds	:= $(release_dir)/compile_commands.json
debug_cmds		:= $(debug_dir)/compile_commands.json
envfile			:= $(BUILD_DIR)/env.txt
os_release		:= $(BUILD_DIR)/os-release.txt
tarball			:= $(BUILD_DIR)/$(BIN)-$(version_core).tar.gz

dirs := \
	$(BUILD_DIR) \
	$(debug_dir) \
	$(release_dir) \
	$(gen_dir) \
	$(test_dir) \
	$(sort $(dir $(debug_objs))) \
	$(sort $(dir $(release_objs))) \
	$(sort $(dir $(gen_src) $(gen_hdr))) \


#
# Define dependency files
#
deps := $(patsubst %.o,%.d,$(debug_objs) $(release_objs))

version_file := $(BUILD_DIR)/versions.txt
version_list := \
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
analyzer_dir := $(BUILD_DIR)/clang-analyzer
analyzer_files := \
	$(patsubst $(release_dir)/%.o,$(analyzer_dir)/%.txt,$(release_objs))

analyzer_flags = \
	--analyzer-output html \
	--output $(basename $@) \
	-Xclang -analyzer-config -Xclang ctu-dir=$(analyzer_dir) \
	-Xclang -analyzer-config -Xclang enable-naive-ctu-analysis=true \
	-Xclang -analyzer-config -Xclang display-ctu-progress=true \
	$(DEFS) \
	$(INC) \
	$(CFLAGS)

analyzer_defmap := $(analyzer_dir)/externalDefMap.txt
analyzer_output := $(BUILD_DIR)/clang-analysis

dirs += $(analyzer_dir) $(sort $(dir $(analyzer_files)))
ifndef clang_used
version_list += "$$($(ANALYZER) --version)"
endif
endif

#
# Variables for cppcheck
#
ifneq (,$(shell type -fP cppcheck))
cppcheck := cppcheck
cppcheck_dir := $(BUILD_DIR)/cppcheck-analysis
cppcheck_flags := \
	--cppcheck-build-dir=$(cppcheck_dir) \
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

cppcheck_output := $(BUILD_DIR)/cppcheck
cppcheck_results := $(cppcheck_dir)/cppcheck.xml

dirs += $(cppcheck_dir)
version_list += "$$(cppcheck --version)"
endif

#
# Variables for shellcheck
#
ifneq (,$(shell type -fP shellcheck))
shellcheck_dir := $(BUILD_DIR)/shellcheck
shellcheck := shellcheck
shellcheck_flags := \
	--color=auto \
	--external-sources \
	--format gcc \
	--enable all \
	--norc \
	--shell $(firstword $(notdir $(SHELL)))

shellcheck_input := \
	$(shell find . -name "*.sh") \
	hooks/pre-commit

shellcheck_output := $(shellcheck_dir)/shellcheck.txt

dirs += $(shellcheck_dir)
version_list += "$$(shellcheck --version)"
endif

#
# Add additional prepocessor macro definitions
#
DEFS := \
	-D_GNU_SOURCE \
	-DCRUDEBOX_VERSION_MAJOR=\"$(version_major)\" \
	-DCRUDEBOX_VERSION_MINOR=\"$(version_minor)\" \
	-DCRUDEBOX_VERSION_PATCH=\"$(version_patch)\" \
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
	-I$(xdg_dir)


#
# Add used libraries which are configurable with pkg-config
#
pkgconf_libs := \
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
# file "$(deps)" for each processed translation unit.
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


#
# Enable addtional targets if there are pkgconf libraries defined
#
ifneq (,$(shell type -f $(PKGCONF)))
ifneq (,$(pkgconf_libs))
pkgconf_check	:= pkgconf-check
pkgconf_dir		:= $(BUILD_DIR)/pkgconf
pkgconf_data	:= $(pkgconf_dir)/libs.json

dirs += $(pkgconf_dir)
version_list += "$(PKGCONF) $$($(PKGCONF) --version)"

#
# Add build flags for all required libraries
#
CFLAGS		+= $(shell $(PKGCONF) --cflags $(pkgconf_libs))
CXXFLAGS	+= $(shell $(PKGCONF) --cflags $(pkgconf_libs))
LDLIBS		+= $(shell $(PKGCONF) --libs $(pkgconf_libs))
endif
endif


#
# Append extra arguments passed on the command-line
#
CPPFLAGS	+= $(EXTRA_CPPFLAGS)
CFLAGS		+= $(EXTRA_CFLAGS)
CXXFLAGS	+= $(EXTRA_CXXFLAGS)
LDFLAGS		+= $(EXTRA_LDFLAGS)


#
# Define unit test targets
#
ifeq (,$(shell $(PKGCONF) --print-errors --exists cmocka 2>&1))
$(info **************************************)
ut_target	:= unit-tests
ut_srcdir	:= test/units
ut_dir		:= $(test_dir)/units
ut_bindir	:= $(ut_dir)/bin
ut_cpus		:= $(shell nproc)
ut_src		:= $(shell find $(ut_srcdir) -name "*.c" -printf "%P\n")

ut_objs := \
	$(patsubst %.c,$(ut_bindir)/%.o,$(ut_src)) \
	$(patsubst $(release_dir)/%.o,$(ut_dir)/%.o,$(release_objs))

#
# Every unit test source file has its own executable and report file.
#
ut_units	:= $(patsubst %.c,$(ut_bindir)/%.elf,$(ut_src))
ut_reports	:= $(patsubst %.elf,%.txt,$(ut_units))

dirs += \
	$(ut_dir) \
	$(ut_bindir) \
	$(sort $(dir $(ut_objs)))

version_list += "cmocka $$($(PKGCONF) --modversion cmocka)"

ifdef clang_used
version_list += "$$(gcc --version)"
endif

ifneq (,$(shell type -fP lcov))
ut_info := $(ut_dir)/crudebox.info
ut_cov := $(ut_dir)/coverage

version_list += "$$(lcov --version)"
endif
endif



#
# Setting terminal colors
#

ifneq ($(MAKEFILE_COLOR), 0)

red			:= \e[1;31m
green		:= \e[1;32m
yellow		:= \e[1;33m
blue		:= \e[1;34m
magenta		:= \e[1;35m
cyan		:= \e[1;36m
reset		:= \e[0m

endif


all: release $(release_cmds) tags 
release: $(release_bin)
debug: $(debug_bin)

#
# Note that if "-flto" is specified you may want to pass the optimization
# flags used for compiling to the linker (as done below).
#

analysis-build: CPPFLAGS	+= -DNDEBUG -DTIMING_ANALYSIS
analysis-build: CFLAGS		+= -fno-omit-frame-pointer
analysis-build: CXXFLAGS	+= -fno-omit-frame-pointer
analysis-build: release

$(release_bin): CPPFLAGS	+= -DNDEBUG
$(release_bin): CFLAGS		+= -O2 -flto -fdata-sections -ffunction-sections
$(release_bin): CXXFLAGS	+= -O2 -flto -fdata-sections -ffunction-sections
$(release_bin): LDFLAGS		+= -O2 -flto -Wl,--gc-sections

$(debug_bin): CPPFLAGS		+= -DMEM_NOLEAK
$(debug_bin): CFLAGS		+= -Og -g2
$(debug_bin): CXXFLAGS		+= -Og -g2

$(ut_units): CPPFLAGS 		+= -DUNIT_TESTS_ENABLED -DMEM_NO_LEAK -I$(srcdir)
$(ut_units): CFLAGS			+= -Og -g2 -ftest-coverage -fprofile-arcs
$(ut_units): CXXFLAGS		+= -Og -g2 -ftest-coverage -fprofile-arcs
$(ut_units): LDLIBS			:= $(shell $(PKGCONF) --libs cmocka)
$(ut_units): LDFLAGS			+= -lgcov

syntax-check: CFLAGS   += -fsyntax-only
syntax-check: CXXFLAGS += -fsyntax-only
syntax-check: $(debug_objs)

$(release_bin): $(release_objs)
	@printf "$(yellow)Linking [ $@ ]$(reset)\n"
	$(Q)$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS) 
	$(Q)strip --strip-all $@
	@printf "$(green)Built target [ $@ ]$(reset)\n"
	@sha256sum --tag $@

$(debug_bin): $(debug_objs)
	@printf "$(yellow)Linking [ $@ ]$(reset)\n"
	$(Q)$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)
	@printf "$(green)Built target [ $@ ]$(reset)\n"
	@sha256sum --tag $@

-include $(deps)

src/wl-window.c: $(xdg_hdr)

$(release_dir)/%.o: %.c
	@printf "$(blue)Building [ $@ ]$(reset)\n"
ifdef clang_used
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
	> $(release_dir)/$*.json
endif

$(release_dir)/%.o: $(BUILD_DIR)/%.c
	@printf "$(blue)Building [ $@ ]$(reset)\n"
ifdef clang_used
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
	> $(release_dir)/$*.json
endif

$(debug_dir)/%.o: %.c
	@printf "$(blue)Building [ $@ ]$(reset)\n"
ifdef clang_used
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
	> $(debug_dir)/$*.json
endif

$(debug_dir)/%.o: $(BUILD_DIR)/%.c
	@printf "$(blue)Building [ $@ ]$(reset)\n"
ifdef clang_used
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
	> $(debug_dir)/$*.json
endif

$(xdg_src): $(xdg_shell) | $(dirs)
	@printf "$(cyan)Generating [ $@ ]$(reset)\n"
	$(Q)wayland-scanner private-code $< $@

$(xdg_hdr): $(xdg_shell) | $(dirs)
	@printf "$(cyan)Generating [ $@ ]$(reset)\n"
	$(Q)wayland-scanner client-header $< $@

$(release_objs) $(debug_objs) $(ut_objs): | $(dirs)

$(dirs):
	mkdir -p $@

$(release_cmds): $(release_objs)
	sed -e '1s/^/[/' -e '$$s/,\s*$$/]/' $(patsubst %.o,%.json,$^) \
		> $@ || (rm -f $@ && false)

$(debug_cmds): $(debug_objs)
	sed -e '1s/^/[/' -e '$$s/,\s*$$/]/' $(patsubst %.o,%.json,$^) \
		> $@ || (rm -f $@ && false)

$(ut_target): $(ut_reports) $(ut_cov)

$(ut_cov): $(ut_info)
	@printf "$(magenta)Generating [ $@ ]$(reset)\n"
	$(Q)genhtml \
		--branch-coverage \
		--function-coverage \
		--output-directory $@ \
		--rc geninfo_auto_base=1 \
		--show-details \
		--title "$(BIN)-$(version_core)$(if $(GITHUB_SHA), ($(GITHUB_SHA)))" \
		$<

$(ut_info): $(ut_reports)
	@printf "$(magenta)Generating [ $@ ]$(reset)\n"
	$(Q)lcov \
		--base-directory $(CURDIR) \
		--capture \
		--directory $(ut_dir) \
		--exclude "/usr/include/*" \
		--exclude "*/build/gen/*" \
		--output-file $@ \
		--rc lcov_branch_coverage=1 \
		--rc lcov_function_coverage=1

$(ut_bindir)/%.txt: $(ut_bindir)/%.elf
	@printf "$(magenta)Executing [ $< ]$(reset)\n"
	$(Q)./$< | tee $@ || (rm -f $@ && false)

$(ut_bindir)/%.elf: $(ut_bindir)/%.o $(ut_dir)/$(srcdir)/%.o
	@printf "$(yellow)Linking [ $@ ]$(reset)\n"
	$(Q)gcc -o $@ $^ $(LDLIBS) $(LDFLAGS)

$(ut_bindir)/%.o: $(ut_srcdir)/%.c
	@printf "$(blue)Building [ $@ ]$(reset)\n"
	$(Q)gcc -c -o $@ $(CPPFLAGS) $(CFLAGS) $<

$(ut_dir)/%.o: %.c
	@printf "$(blue)Building [ $@ ]$(reset)\n"
	$(Q)gcc -c -o $@ $(CPPFLAGS) $(CFLAGS) $<

clean:
	rm -rf $(BUILD_DIR)

format:
	$(SHELL) scripts/mk-format.sh

tags: $(hdr) $(src)
	ctags -f tags $^

install: $(release_bin)
	cp $< $(DESTDIR)

uninstall:
	rm -f $(DESTDIR)/$(BIN)

$(envfile): | $(dirs)
	@env \
		$(if $(ARTIFACTORY_API_KEY),ARTIFACTORY_API_KEY=) \
		$(if $(DOCKER_USERNAME),DOCKER_USERNAME=) \
		$(if $(DOCKER_PASSWORD),DOCKER_PASSWORD=) \
		> $@

$(os_release): /etc/os-release | $(dirs)
	cp -f $< $@

$(version_file): | $(dirs)
	@printf "%s\n--\n" $(version_list) > $@ || (rm -f $@ && false)

$(pkgconf_check): $(pkgconf_data)
	@printf "$(green)Performed [ $@ ]$(reset)\n"

$(pkgconf_data): | $(dirs)
	@printf "$(blue)Generating [ $@ ]$(reset)\n"
	$(Q)$(PKGCONF) --print-errors --print-provides $(pkgconf_libs) \
		| tr --squeeze-repeats " =" " " \
		| column --table-name pkgconf --table-columns library,version --json \
		| tee $@ || (rm -f $@ && false)

clang-analysis: $(analyzer_files)

$(analyzer_output): | $(analyzer_files)
	ln --relative --symbolic --force  $(analyzer_dir) $@

$(analyzer_dir)/%.txt: %.c $(analyzer_defmap)
	@printf "$(blue)Generating [ $@ ]$(reset)\n"
	$(Q)clang --analyze $(analyzer_flags) $< 2>&1 | tee $@

$(analyzer_dir)/%.txt: $(BUILD_DIR)/%.c $(analyzer_defmap)
	@printf "$(blue)Generating [ $@ ]$(reset)\n"
	$(Q)$(ANALYZER) $(analyzer_flags) $< 2>&1 | tee $@

$(analyzer_defmap): $(debug_cmds) $(src)
	$(Q)clang-extdef-mapping -p $(debug_cmds) $(src) > $@ || (rm -f $@ && false)

$(cppcheck): $(cppcheck_output)

$(cppcheck_output): $(cppcheck_results) 
	@printf "$(yellow)Generating report [ $@ ]$(reset)\n"
	@rm -rf $@
	$(Q)cppcheck-htmlreport --file=$< --title=$(BIN) --report-dir=$@

$(cppcheck_results): $(debug_cmds) | $(dirs)
	@printf "$(yellow)Analyzing project [ $< ]$(reset)\n"
	$(Q)cppcheck $(cppcheck_flags) --project=$< --output-file=$@

$(shellcheck): $(shellcheck_output)

$(shellcheck_output): $(shellcheck_input) | $(dirs)
	$(Q)shellcheck $(shellcheck_flags) $^ | tee $@ || (rm -f $@ && false)

# Use sed to strip the directories in which the tarball will be created off of
# all input paths. This is required so the paths don't appear when extracting
# the tarball.
$(tarball): \
		$(release_bin) \
		$(release_cmds) \
		$(debug_bin) \
		$(debug_cmds) \
		$(analyzer_output) \
		$(cppcheck_output) \
		$(envfile) \
		$(os_release) \
		$(pkgconf_data) \
		$(shellcheck_output) \
		$(ut_reports) \
		$(ut_cov) \
		$(version_file)
	@printf "$(magenta)Packaging [ $@ ]$(reset)\n"
	$(Q)find -H $^ -type f -size +0 \
		| sed -e 's/^\(\.\/\)\?$(@D)\///g' \
		| $(TAR) --create --file $@ --gzip --directory $(@D) --files-from -

artifactory-upload: $(tarball)
	@printf "$(magenta)Uploading [ $^ ]$(reset)\n"
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
		"$(artifactory_upload_url)/$(^F)"
	@printf "$(green)Uploaded [ $^ ]$(reset)\n"
else
	@printf "** ERROR: $@: \"ARTIFACTORY_API_KEY\" not specified\n"
	@false
endif


.PHONY: \
	$(envfile) \
	$(pkgconf_check) \
	$(shellcheck) \
	$(ut_target) \
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
	$(analyzer_output) \
	clean \
	format \
	$(release_cmds) \
	$(debug_cmds) \
	tags \
	$(dirs)

