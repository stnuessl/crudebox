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

Q :=

DOCKER_DIR			:= build/docker
ARCH_BUILD_IMAGE	:= $(DOCKER_DIR)/archlinux-build-image
ALPINE_BUILD_IMAGE	:= $(DOCKER_DIR)/alpine-build-image
FEDORA_BUILD_IMAGE	:= $(DOCKER_DIR)/fedora-build-image

ARCH_BUILD_TAG		:= crudebox:archlinux-build
ALPINE_BUILD_TAG	:= crudebox:alpine-build
FEDORA_BUILD_TAG	:= crudebox:fedora-build

DIRS := \
	$(DOCKER_DIR)

all: $(ARCH_BUILD_IMAGE) $(ALPINE_BUILD_IMAGE) $(FEDORA_BUILD_IMAGE)

$(ARCH_BUILD_IMAGE): docker/archlinux-build/Dockerfile | $(DIRS)
	$(Q)docker build --tag $(ARCH_BUILD_TAG) $(^D)
	$(Q)touch $@

archlinux-build: $(ARCH_BUILD_IMAGE)
	$(Q)docker run \
		--interactive \
		--tty=$(if $(CMD),false,true) \
		--rm \
		--env XDG_SESSION_TYPE \
		--env ARTIFACTORY_API_KEY \
		--volume ${PWD}:/crudebox \
		--user $$(id -u):$$(id -g) \
		--workdir /crudebox \
		$(ARCH_BUILD_TAG) $(CMD)

$(ALPINE_BUILD_IMAGE): docker/alpine-build/Dockerfile | $(DIRS)
	$(Q)docker build --tag $(ALPINE_BUILD_TAG) $(^D)
	$(Q)touch $@

alpine-build: $(ALPINE_BUILD_IMAGE)
	$(Q)docker run \
		--interactive \
		--tty=$(if $(CMD),false,true) \
		--rm \
		--env XDG_SESSION_TYPE \
		--volume ${PWD}:/home/crudebox \
		--workdir /home/crudebox \
		$(ALPINE_BUILD_TAG) $(CMD)

$(FEDORA_BUILD_IMAGE): docker/fedora-build/Dockerfile | $(DIRS)
	$(Q)docker build --tag $(FEDORA_BUILD_TAG) $(^D)
	$(Q)touch $@

fedora-build: $(FEDORA_BUILD_IMAGE)
	$(Q)docker run \
		--interactive \
		--tty=$(if $(CMD),false,true) \
		--rm \
		--env XDG_SESSION_TYPE \
		--env ARTIFACTORY_API_KEY \
		--volume ${PWD}:/crudebox \
		--user $$(id -u):$$(id -g) \
		--workdir /crudebox \
		$(FEDORA_BUILD_TAG) $(CMD)



#build/docker-workspace-image: docker/workspace/Dockerfile | $(DIRS)
#	docker build --tag crudebox:workspace $(^D) \
#		&& touch $@
#
#docker-workspace: build/docker-workspace-image
#	docker run \
#		--interactive \
#		--tty=$(if $(WORKSPACE_COMMAND),false,true) \
#		--rm \
#		--env XDG_RUNTIME_DIR \
#		--env DISPLAY \
#		--hostname $(shell hostname) \
#		--volume ~/.Xauthority:/home/docker/.Xauthority \
#		--volume ${XDG_RUNTIME_DIR}:/run/user/1000 \
#		--volume ${PWD}:/home/docker/crudebox \
#		--volume /tmp/.X11-unix:/tmp/.X11-unix \
#		--workdir /home/docker/crudebox \
#		crudebox:workspace $(WORKSPACE_COMMAND)

docker-clean:
	docker image ls \
		| grep "crudebox" \
		| awk '{ print $$3 }' \
		| xargs --no-run-if-empty docker image rm --force
	rm -f build/docker-workspace-image

$(DIRS):
	mkdir -p $@

.PHONY: \
	all \
	docker-clean

.SILENT: \
	docker-clean \
	$(DIRS)

