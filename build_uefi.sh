#!/bin/bash

# Function to display Help Message
function _help(){
	echo "Usage: ./build_uefi.sh -d <Codename> [-r <Build Mode>]"
	echo
	echo "Build Project Mu UEFI for Nvidia Tegra Platforms."
	echo
	echo "Options:"
	echo "	--device <Codename>, -d <Codename>:         Build a Device."
	echo "	--release <Build Mode>, -r <Build Mode>:    Release mode for building, 'RELEASE' is the default or use 'DEBUG' alternatively."
	echo "	--help, -h:                                 Shows this Help."
	echo "  --acpi, -a:                                 Use iasl to recompile device specific ACPI tables present on ACPI.inc."
	echo
	echo "MainPage: https://github.com/Robotix22/Mu-Tegra"
	exit 1
}

# Functions to display the Message Type (Error, Warning or Notification)
function _error(){ echo -e "\033[1;31m${@}\033[0m" >&2;exit 1; }
function _warn(){ echo -e "\033[0;33m${@}\033[0m" >&2; }
function _ntf(){ echo -e "\033[0;32m${@}\033[0m" >&2; }

# Set Default Defines
TARGET_BUILD_MODE=RELEASE

function _acpi(){
	echo -e "\nUpdating device ACPI tables\n"
	echo -e "PROGRESS - ACPI updater" > ./Build/acpi.log
	TARGET_DEVICE_VENDOR=$(grep TARGET_DEVICE_VENDOR ./configs/$TARGET_DEVICE.conf | tr -d 'TARGET_DEVICE_VENDOR=' | tr -d '"')
	if [ -f ./Platforms/$TARGET_DEVICE_VENDOR/${TARGET_DEVICE}Pkg/Include/ACPI.inc ]; then
		TABLES="$(grep "SECTION RAW" ./Platforms/$TARGET_DEVICE_VENDOR/${TARGET_DEVICE}Pkg/Include/ACPI.inc | sed '/#.*SECTION RAW/d' | grep $TARGET_DEVICE | awk '{print $4}')"
		for TABLE in $TABLES; do
			echo "INFO - $TABLE" >> ./Build/acpi.log
			TABLE_DIR=$(dirname $TABLE)
			TABLE_NAME=$(basename $TABLE)
			TABLE_NAME_ASL=$(basename $TABLE | sed 's/.aml$/.asl/g')
			TABLE_NAME_DSL=$(basename $TABLE | sed 's/.aml$/.dsl/g')
			[ -f ./Platforms/$TARGET_DEVICE_VENDOR/$TABLE_DIR/$TABLE_NAME ] && mv -f ./Platforms/$TARGET_DEVICE_VENDOR/$TABLE_DIR/$TABLE_NAME ./Platforms/$TARGET_DEVICE_VENDOR/$TABLE_DIR/$TABLE_NAME.tablebkp > /dev/null 2>&1
			if [ -f ./Platforms/$TARGET_DEVICE_VENDOR/$TABLE_DIR/$TABLE_NAME_ASL ]; then
				RUN=$(iasl ./Platforms/$TARGET_DEVICE_VENDOR/$TABLE_DIR/$TABLE_NAME_ASL 2>&1 >/dev/null)
				if [ $(echo "$RUN" | grep Error |wc -c) -gt 1 ]; then
					_warn "Could not update $TABLE_NAME: $RUN"
					echo "DEBUG - Could not update $TABLE_NAME: $RUN" >> ./Build/acpi.log
					sleep 3
				else
					_ntf "$TABLE_NAME updated successfully"
					echo "DEBUG - $TABLE_NAME updated successfully" >> ./Build/acpi.log
				fi
			elif [ -f ./Platforms/$TARGET_DEVICE_VENDOR/$TABLE_DIR/$TABLE_NAME_DSL ]; then
				RUN=$(iasl ./Platforms/$TARGET_DEVICE_VENDOR/$TABLE_DIR/$TABLE_NAME_DSL 2>&1 >/dev/null)
				if [ $(echo "$RUN" | grep Error |wc -c) -gt 1 ]; then
					_warn "Could not update $TABLE_NAME: $RUN"
					echo "DEBUG - Could not update $TABLE_NAME: $RUN" >> ./Build/acpi.log
					sleep 3
				else
					_ntf "$TABLE_NAME updated successfully"
					echo "DEBUG - $TABLE_NAME updated successfully" >> ./Build/acpi.log
				fi
			else
				_warn "Could not find $TABLE_NAME source file on $TABLE_DIR"
				echo "DEBUG - Could not find $TABLE_NAME source file on $TABLE_DIR" >> ./Build/acpi.log
				sleep 3
			fi
			if [ $(echo "$RUN" | grep Error |wc -c) -gt 1 ] && [ -f ./Platforms/$TARGET_DEVICE_VENDOR/$TABLE_DIR/$TABLE_NAME.tablebkp ]; then
				mv -f ./Platforms/$TARGET_DEVICE_VENDOR/$TABLE_DIR/$TABLE_NAME.tablebkp ./Platforms/$TARGET_DEVICE_VENDOR/$TABLE_DIR/$TABLE_NAME > /dev/null 2>&1
			else
				rm -f ./Platforms/$TARGET_DEVICE_VENDOR/$TABLE_DIR/$TABLE_NAME.tablebkp > /dev/null 2>&1
			fi
			echo
		done
		echo -e "ACPI update done\n\n"
		echo -e "PROGRESS - ACPI update done" >> ./Build/acpi.log
		sleep 2
	else
		_error "\nThere's no ACPI.inc for $TARGET_DEVICE\nAborting...\n"
		echo -e "CRITICAL - There's no ACPI.inc for $TARGET_DEVICE" >> ./Build/acpi.log
	fi
}

# Check if any args were given
OPTS="$(getopt -o d:hfbc:r:a -l device:,help,release:,acpi -n 'build_uefi.sh' -- "$@")"||exit 1
eval set -- "${OPTS}"
while true
do	case "${1}" in
		-d|--device) TARGET_DEVICE="${2}";shift 2;;
		-h|--help) _help 0;shift;;
		-r|--release) TARGET_BUILD_MODE="${2}";shift 2;;
		-a|--acpi) _acpi;shift;;
		--) shift;break;;
		*) _help 1;;
	esac
done


# If no Device arg is present, Display the Help Message
if [ -z ${TARGET_DEVICE} ]
then _help
fi

# Set Release Type of UEFI
case "${TARGET_BUILD_MODE}" in
	RELEASE) _TARGET_BUILD_MODE=RELEASE;;
	*) _TARGET_BUILD_MODE=DEBUG;;
esac

# Include Device Config if it exists
if [ -f "configs/${TARGET_DEVICE}.conf" ]
then source "configs/${TARGET_DEVICE}.conf"
else _error "\nDevice configuration not found!\nCheck if your .conf File is in the 'configs' Folder\n"
fi

# Delete Output Files if present
rm -r Conf &> /dev/null
rm Mu-${TARGET_DEVICE}.bin &> /dev/null

# Setup & Update UEFI Enviroment
python3 "Platforms/${TARGET_DEVICE_VENDOR}/${TARGET_DEVICE}Pkg/PlatformBuild.py" --setup -t ${_TARGET_BUILD_MODE}||_error "\nFailed to Setup UEFI Env!\n"
python3 "Platforms/${TARGET_DEVICE_VENDOR}/${TARGET_DEVICE}Pkg/PlatformBuild.py" --update -t ${_TARGET_BUILD_MODE}||_error "\nFailed to Update UEFI Env!\n"

# Start the Real Build of the UEFI
python3 "Platforms/${TARGET_DEVICE_VENDOR}/${TARGET_DEVICE}Pkg/PlatformBuild.py" "TARGET=${_TARGET_BUILD_MODE}" "FD_BASE=${TARGET_FD_BASE}" "FD_SIZE=${TARGET_FD_SIZE}" "FD_BLOCKS=${TARGET_FD_BLOCKS}"||_error "\nFailed to Compile UEFI!\n"

# Move .fd File to Mu-Tegra Folder
mv "./Build/${TARGET_DEVICE}Pkg/${_TARGET_BUILD_MODE}_CLANG38/FV/${TARGET_DEVICE^^}_UEFI.fd" "./Mu-${TARGET_DEVICE}.bin"||_error "\nFailed to Move UEFI Payload!\n"

# Create a ELF Payload
#aarch64-linux-gnu-objcopy -I binary -O elf64-littleaarch64 --binary-architecture aarch64 "./Build/${TARGET_DEVICE}Pkg/${_TARGET_BUILD_MODE}_CLANG38/FV/${TARGET_DEVICE^^}_UEFI.fd" "./Build/${TARGET_DEVICE}Pkg/${_TARGET_BUILD_MODE}_CLANG38/FV/${TARGET_DEVICE^^}_UEFI.o"||_error "\nFailed to Build ELF Payload!\n"
#aarch64-linux-gnu-ld -m aarch64elf "./Build/${TARGET_DEVICE}Pkg/${_TARGET_BUILD_MODE}_CLANG38/FV/${TARGET_DEVICE^^}_UEFI.o" -T "./configs/${TARGET_DEVICE}.ld" -o "./Mu-${TARGET_DEVICE}.elf"||_error "\nFailed to Build ELF Payload!\n"

git fetch &> /dev/null
UPDATE_CHECK=$(git status)
if [[ ${UPDATE_CHECK} == *"git pull"* ]];
then _warn "\nYou are using an old Version of Mu-Tegra.\nThis Image may be unstable.\n"
fi

[ -f ./Build/acpi.log ] && sed -i '1 r ./Build/acpi.log' ./Build/BUILDLOG_${TARGET_DEVICE}Pkg.txt && rm -f ./Build/acpi.log
