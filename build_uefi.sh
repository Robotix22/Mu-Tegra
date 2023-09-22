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
	echo
	echo "MainPage: https://github.com/Robotix22/Mu-Tegra"
	exit 1
}

# Set Correct Variables
export CLANG38_ARM_PREFIX=arm-linux-gnueabihf-

# Functions to display the Message Type (Error or Warning)
function _error(){ echo -e "\033[1;31m${@}\033[0m" >&2;exit 1; }
function _warn(){ echo -e "\033[0;33m${@}\033[0m" >&2; }

# Set Default Defines
TARGET_BUILD_MODE="RELEASE"

# Check if any args were given
OPTS="$(getopt -o d:hfabcACDO:r: -l device:,help,release:: -n 'build_uefi.sh' -- "$@")"||exit 1
eval set -- "${OPTS}"
while true
do	case "${1}" in
		-d|--device) TARGET_DEVICE="${2}";shift 2;;
		-h|--help) _help 0;shift;;
		-r|--release) TARGET_BUILD_MODE="${2}";shift 2;;
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
rm Mu-${TARGET_DEVICE}.elf &> /dev/null
rm UEFILoader.efi

# Setup and Update UEFI workspace
stuart_setup -c "Platforms/${TARGET_DEVICE_VENDOR}/${TARGET_DEVICE}Pkg/PlatformBuild.py" TOOL_CHAIN_TAG=CLANG38||_error "\nFailed to Setup UEFI Env!\n"
stuart_update -c "Platforms/${TARGET_DEVICE_VENDOR}/${TARGET_DEVICE}Pkg/PlatformBuild.py" TOOL_CHAIN_TAG=CLANG38||_error "\nFailed to Update UEFI Env!\n"

# Copy fixed tools_def.txt to Build Folder
rm ./Conf/tools_def.txt &> /dev/null
cp ./configs/tools_def.txt ./Conf/ &> /dev/null

# Start the UEFI Build
stuart_build -c "Platforms/${TARGET_DEVICE_VENDOR}/${TARGET_DEVICE}Pkg/PlatformBuild.py" TOOL_CHAIN_TAG=CLANG38 "TARGET=${_TARGET_BUILD_MODE}" "FD_BASE=${TARGET_FD_BASE}" "FD_SIZE=${TARGET_FD_SIZE}" "FD_BLOCKS=${TARGET_FD_BLOCKS}"||_error "\nFailed to Compile UEFI!\n"

# Copy UEFI FD File to a ELF Payload File
${CLANG38_ARM_PREFIX}objcopy -I binary -O elf32-littlearm --binary-architecture arm "./Build/${TARGET_DEVICE}Pkg/${_TARGET_BUILD_MODE}_CLANG38/FV/${TARGET_DEVICE^^}_UEFI.fd" "./Build/${TARGET_DEVICE}Pkg/${_TARGET_BUILD_MODE}_CLANG38/FV/${TARGET_DEVICE^^}_UEFI.o"||_error "\nFailed to Build ELF Payload!\n"
${CLANG38_ARM_PREFIX}ld -m armelf_linux_eabi "./Build/${TARGET_DEVICE}Pkg/${_TARGET_BUILD_MODE}_CLANG38/FV/${TARGET_DEVICE^^}_UEFI.o" -T "./configs/${TARGET_DEVICE}.ld" -o "./Mu-${TARGET_DEVICE}.elf"||_error "\nFailed to Build ELF Payload!\n"

# Copy UEFI Loader to Root Dir
cp "./Build/${TARGET_DEVICE}Pkg/${_TARGET_BUILD_MODE}_CLANG38/ARM/UEFILoader.efi" ./UEFILoader.efi||_error "\nFailed to Copy EFI Application!\n"

if [[ ${STATUS} != "STABLE" ]]; then
	if [[ ${STATUS} == "UNSTABLE" ]];
	then _warn "\n${TARGET_DEVICE} is marked as Unstable.\nThings are expected to be broken.\n"
	elif [[ ${STATUS} == "LIMITED" ]];
	then _warn "\n${TARGET_DEVICE} is marked as Limited.\nIt's stable but does not have much functionality\n"
	fi
fi

#git fetch &> /dev/null
#UPDATE_CHECK=$(git status)
#if [[ ${UPDATE_CHECK} == *"git pull"* ]];
#then _warn "\nYou are using an old Version Mu-Tegra.\nThis Payload may be unstable.\n"
#fi
