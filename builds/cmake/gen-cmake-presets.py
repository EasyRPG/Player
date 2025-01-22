#!/bin/env python3

# Script to generate all combinations for the CMakePresets.json
# Edit the CMakePresets.json.template and run this script
# License for the script and all json files including generated: Unlicense
# (c) Ghabry

from copy import deepcopy
import json
import os

script_dir = os.path.dirname(os.path.abspath(__file__))
repo_dir = f"{script_dir}/../.."

with open(f"{script_dir}/CMakePresets.json.template", "r") as f:
	j = json.load(f)

conf_presets = j["configurePresets"]
conf_presets_out = []

def append_name(name):
	if len(item["name"]) > 0:
		item["name"] += "-"
	item["name"] += name

platform_display = dict(
	sdl1="SDL1",
	sdl2="SDL2",
	sdl3="SDL3",
	libretro="libretro core"
)

# This creates the following configurePresets from the one in the template:
# - As specified in the template
# - For every entry in easyrpg_platforms (N)
# For all of them the build types Debug, RelWithDebInfo and Release are generated.
# Making this (N+1) * 3 entries per preset.

# The resulting "triplet" is always:
# {name_from_template}-{libretro}-{build_type}
# Omit libretro to disable it

# The build dirs are always:
# build/{name_from_template}-{libretro}-{build_type}

for base_item in conf_presets:
	if base_item.get("hidden"):
		conf_presets_out.append(base_item)
		continue

	# Create "base class" the build types inherit from
	item = deepcopy(base_item)
	append_name("parent")
	del item["displayName"]
	item["hidden"] = True

	if item.get("inherits") is None:
		item["inherits"] = "base-user"

	ep_platforms = ["default"]
	if "easyrpg_platforms" in item:
		ep_platforms += item["easyrpg_platforms"]
		del item["easyrpg_platforms"]

	parent_item = deepcopy(item)
	conf_presets_out.append(parent_item)

	for platform in ep_platforms:
		# Ugly: Generates a huge amount of configurePresets
		# Cannot be improved until limitations in buildPresets are resolved
		# (see comment below)
		for build_type in ["Debug", "RelWithDebInfo", "Release"]:
			item = dict(name=base_item["name"], displayName=base_item["displayName"])
			name = item["name"]

			item["inherits"] = [parent_item["name"]]

			if platform != "default":
				append_name(platform)
				item["inherits"].insert(0, f"build-{platform}")
				item["displayName"] += f" ({platform_display[platform]}, {build_type})"
			else:
				item["displayName"] += f" ({build_type})"

			item["inherits"] += [f"type-{build_type.lower()}"]

			append_name(build_type.lower())

			conf_presets_out.append(item)

j["configurePresets"] = conf_presets_out

bp = j["buildPresets"]
for item in conf_presets_out:
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
	json.dump(j, f, indent='\t')
	f.write("\n")
