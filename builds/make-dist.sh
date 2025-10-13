#!/bin/bash

# make-dist.sh - maintainer utility script to generate distribution archives
# by carstene1ns 2019-2024, released under the MIT license

set -e
#set -x

# helper functions

colorize() {
  RESET="\e[0m"
  BOLD="\e[1m"
  BLUE="${BOLD}\e[34m"
  RED="${BOLD}\e[31m"
  YELLOW="${BOLD}\e[33m"
}

message() {
  if [ -z ${V} ]; then
    printf "${BLUE}${1}${RESET}\n"
  else
    printf "\n${BLUE}${1}${RESET}\n"
    local underline=$(printf "%-${#1}s" "=")
    printf "${BOLD}${underline// /=}${RESET}\n"
  fi
}

warning() {
  printf "${YELLOW}WARNING:${RESET}${BOLD} ${1}${RESET}\n" >&2
}

error() {
  printf "${RED}ERROR:${RESET}${BOLD} ${1}${RESET}\n" >&2
  exit 1
}

has() {
  hash ${1} 2> /dev/null || error "'${1}' not found"
}

cleanup() {
  warning "Something went wrong, cleaning up"
  [ -d "${TEMPDIR}" ] && rm ${V} -rf "${TEMPDIR}"
  [ -f ${TARFILE} ] && rm ${V} -f ${TARFILE}
}

# main

# check for terminal output
test -t 0 && colorize

[ -d .git ] || error "Not in a git repository (no .git directory found)"

# check tools
has git
has rm
has asciidoctor
has mktemp
has gzip
has xz

# options
V=""
WORKTREE_ATTRIBUTES=""
TREEISH=master
while getopts ":hvt:w" ARG; do
  case $ARG in
    v)
      V="-v"
      ;;
    w)
      WORKTREE_ATTRIBUTES="--worktree-attributes"
      ;;
    t)
      TREEISH=$OPTARG
      ;;
    h)
      echo "make-dist.sh - Generate EasyRPG Player distribution archives"
      echo "Options:"
      echo "  -h            - This help message"
      echo "  -v            - Run in verbose mode"
      echo "  -t <tree-ish> - Archive branch/tag/... instead of '${TREEISH}'"
      echo "  -w            - Use current working tree .gitattributes state"
      exit 0
      ;;
    \?)
      error "Invalid option: -$OPTARG."
      ;;
    :)
      error "Option -$OPTARG requires an argument."
  esac
done

APP="easyrpg-player"
RELEASE=$(git describe --abbrev=0 --tags ${TREEISH})
PREFIX="${APP}-${RELEASE}"
TARFILE="${PREFIX}.tar"
MANPREFIX="resources/unix" # since 0.8

message "Deleting old archives"
rm ${V} -f ${TARFILE} ${TARFILE}.gz ${TARFILE}.xz

message "Creating temporary directory for additional files"
TEMPDIR=$(mktemp -d dist.XXXXX)
trap cleanup EXIT

message "Generating manual page"
git show ${TREEISH}:${MANPREFIX}/${APP}.6.adoc \
  | asciidoctor ${V} -a player_version="${RELEASE}" -b manpage -o ${TEMPDIR}/${APP}.6 -

message "Archiving repository (@${TREEISH})"
git archive ${V} ${WORKTREE_ATTRIBUTES} -o ${TARFILE} \
  --prefix=${PREFIX}/${MANPREFIX}/ --add-file=${TEMPDIR}/${APP}.6 \
  --add-virtual-file=${PREFIX}/builds/cmake/.git-dist:"${TREEISH}" \
  --prefix=${PREFIX}/ ${TREEISH} || error "Cannot create archive"

message "Deleting temporary directory"
rm ${V} -rf ${TEMPDIR}
trap - EXIT

message "Compressing tar archive with gzip and xz"
GZIP= gzip -9 -k ${V} ${TARFILE}
XZ_OPT= xz -5 -T2 ${V} ${TARFILE}
