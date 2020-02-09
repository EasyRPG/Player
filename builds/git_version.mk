GIT_ROOT = $(git rev-parse --absolute-git-dir)
GIT_HEAD_FILE = HEAD
GIT_REF_FILE = $(shell git symbolic-ref -q HEAD)
ifeq ($(strip $(GIT_REF_FILE)),)
	GIT_REV_FILE=$(GIT_ROOT)/$(GIT_HEAD_FILE)
else
	GIT_REV_FILE=$(GIT_ROOT)/$(GIT_REF_FILE)
endif


GIT_VERSION_IN = $(GIT_ROOT)/../src/git_version.cpp.in
GIT_VERSION_OUT =$(GIT_ROOT)/../src/git_version.cpp

$(GIT_VERSION_OUT): $(GIT_VERSION_IN) $(GIT_REV_FILE)
	sed "s/@GIT_VERSION@/`cat $(GIT_REV_FILE)`/" $(GIT_VERSION_IN) > $@

