presets="arm64 armv7 x86 x86_64"
config=Release

parseCMakeActionArgs () {
	while [[ "$#" -gt 0 ]]; do
		case "$1" in
			--presets)
				if [[ -z "$2" ]]; then
					echo "Error: Option $1 requires an argument." >&2
					exit 1
				fi
				presets="$2"
				shift
				;;
			--config)
				if [[ -z "$2" ]]; then
					echo "Error: Option $1 requires an argument." >&2
					exit 1
				fi
				config="$2"
				shift
				;;
			config)
				action=config
				;;
			build)
				action=build
				;;
			install)
				action=install
				;;
			installLinks)
				action=installLinks
				;;
			*) # All others are directly passed to cmake
				cmakeArgs+=($1)
				;;
		esac
		shift
	done
}

configBuildIfNeeded () {
	if [ -f "build/android-$1/CMakeCache.txt" ]; then
		return
	fi
	cmake --preset android-$1 --fresh ${cmakeArgs[@]}
}

executeCMakeActions () {
	case $action in
		config)
			for a in $presets
			do
				cmake --preset android-$a --fresh ${cmakeArgs[@]}
			done
		;;
		build)
			for a in $presets
			do
				configBuildIfNeeded $a
				cmake --build build/android-$a --config $config ${cmakeArgs[@]} &
			done
			wait
		;;
		install)
			for a in $presets
			do
				configBuildIfNeeded $a
				cmake --build build/android-$a --target install --config $config ${cmakeArgs[@]}
			done
		;;
		installLinks)
			for a in $presets
			do
				configBuildIfNeeded $a
				CMAKE_INSTALL_MODE=REL_SYMLINK cmake --build build/android-$a --target install --config $config ${cmakeArgs[@]}
			done
		;;
		*)
			echo "Please specify an action: config, build, install, or installLinks"
		;;
	esac
}
