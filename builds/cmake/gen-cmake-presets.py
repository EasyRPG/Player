#!/bin/env python3

# Script to generate all combinations for the CMakePresets.json
# Edit the CMakePresets.json.template and run this script
# This script and all json files under CC0
# (c) 2023 Ghabry

from copy import deepcopy
import json
import os

# Omit libretro preset generation
no_libretro = ["emscripten"]

script_dir = os.path.dirname(os.path.abspath(__file__))
repo_dir = f"{script_dir}/../.."

with open(f"{script_dir}/CMakePresets.json.template", "r") as f:
	j = json.load(f)

cp = j["configurePresets"]
cp_out = []

def append_name(name):
	if len(item["name"]) > 0:
		item["name"] += "-"
	item["name"] += name

# This creates the following configurePresets from the one in the template:
# - As specified in the template
# - As specified but build a libretro core
# - As specified and build liblcf (-DPLAYER_BUILD_LIBLCF=ON)
# - liblcf + libretro
# For all of them the build types Debug, RelWithDebInfo and Release are generated.
# Making this 4 * 3 entries per preset.

# The resulting "triplet" is always:
# {name_from_template}-{libretro}-{liblcf}-{build_type}
# Omit libretro or liblcf to disable them

# The build dirs are always:
# build/{name_from_template}-{libretro}-{liblcf}-{build_type}

for base_item in cp:
	if base_item.get("hidden"):
		cp_out.append(base_item)
		continue

	for libretro in False, True:
		for lcf in False, True:
			# Ugly: Generates a huge amount of configurePresets
			# Cannot be improved until limitations in buildPresets are resolved
			# (see comment below)
			for build_type in ["Debug", "RelWithDebInfo", "Release"]:
				item = deepcopy(base_item)
				name = item["name"]

				if libretro and name in no_libretro:
					continue

				variables = item.get("cacheVariables", dict())
				item["cacheVariables"] = variables

				if libretro:
					append_name("libretro")
					item["displayName"] += " (libretro core)"
					variables["PLAYER_TARGET_PLATFORM"] = "libretro"

				if lcf:
					append_name("liblcf")

					item["displayName"] += " + Build liblcf"
					variables["PLAYER_BUILD_LIBLCF"] = "ON"

				append_name(build_type.lower())
				item["displayName"] += f" ({build_type})"

				if item.get("inherits") is None:
					item["inherits"] = "base-user"

				if not item["cacheVariables"]:
					del item["cacheVariables"]

				item["binaryDir"] = "${sourceDir}/build/" + item["name"]

				cp_out.append(item)

j["configurePresets"] = cp_out

bp = j["buildPresets"]
for item in cp_out:
	if item.get("hidden"):
		continue

	# Here it would make sense to provide the build types, however this will
	# only work for Multi Config generators.
	# Instead the build types are generated as part of the configurePreset.
	# Code kept for reference in case CMake improves this in a later version.

	#for build_type in ["Debug", "RelWithDebInfo", "Release", "MinSizeRel"]:
	#	name = item["name"] + f"-{build_type.lower()}"
	#	conf_preset = item["name"]
	#	bp.append(dict(
	#		name=name,
	#		displayName=build_type,
	#		configurePreset=conf_preset,
	#		configuration=build_type))

	bp.append(dict(name=item["name"], configurePreset=item["name"]))

# Add note that the file is auto-generated to the beginning
vendor = dict(vendor=dict(
	README=dict(
		line1="!!! GENERATED FILE. DO NOT EDIT !!!",
		line2="Modify builds/cmake/CMakePresets.json.template instead",
		line3="and run gen-cmake-presets.py to regenerate this file."
	)
))

j = {**vendor, **j}

with open(f"{repo_dir}/CMakePresets.json", "w") as f:
	json.dump(j, f, indent=4)
	f.write("\n")
