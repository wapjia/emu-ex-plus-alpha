#!/bin/bash

filteredArgs=()
skipNext=false

# Strip any Apple-specific args for cross compiling
for arg in "$@"; do
	if [ "$skipNext" = true ]; then
		skipNext=false
		continue
	fi
	case "$arg" in
		-arch) # args with flags
			skipNext=true
			continue
		;;
		-mdynamic-no-pic|-mthumb|-miphoneos-version-min=*)
			continue
		;;
		*)
			filteredArgs+=("$arg")
		;;
	esac
done

exec clang-scan-deps "${filteredArgs[@]}"
