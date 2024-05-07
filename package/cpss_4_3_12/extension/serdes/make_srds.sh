#!/usr/bin/env bash

# Script Name: create_serdes_fw.sh
# Description: This script is used to create SerDes firmware (SRDS) files for various devices based on the filenames present in the current directory.
# Parameters: None

# Define paths and variables
CALLER_WORK_DIR=`pwd`
CPSS_PATH=`dirname $0`/../..
CPSS_PATH=`realpath $CPSS_PATH`
source ${CPSS_PATH}/common/h/cpssCommon/private/fw_ids.h
source ${CPSS_PATH}/common/h/cpssCommon/private/serdes_ids.h

tmp_file=tmp

COLS=2

# Define the device list with device names and their corresponding IDs
device_list=("BC2"        "$BC2_DEV_ID"
            "BOBK"       "$BOBK_DEV_ID"
            "ALDRIN"     "$ALDRIN_DEV_ID"
            "ALDRIN2"    "$ALDRIN2_DEV_ID"
            "PIPE"       "$PIPE_DEV_ID"
            "BC3"        "$BC3_DEV_ID"
            # AC5X must precede AC5
            "AC5X"       "$DUMMY_AC5X_FW_ID"
            "AC5"        "$AC5_DEV_ID"
            "AC5P"       "$AC5P_DEV_ID"
            "HARRIER"    "$HARRIER_DEV_ID"
            "IRONMAN"    "$IRONMAN_DEV_ID"
            )

# Calculate the number of rows in the device list
rows=$(( (${#device_list[@]} ) / $COLS))

# Function to create SerDes firmware (SRDS) files for a given device
createSRDS()
{
    dev=$1
    devID=$2

    # Iterate through the files in the current directory
    for filename in *.*; do
        # Check if the filename contains the device name
        if [[ "$filename" == *"$dev"* ]] ; then
            echo "Processing file: $filename for device: $dev"

            # Extract the version from the filename (assuming the version is in the format XX.XX.XX.XX)
            ver="${filename:(-7)}"
            post_len=$((${#dev} + 8))
            prefix=${filename::-$post_len}

            # Find the SerDes ID by searching for the prefix in the SerDes array
            for ((i = 0 ; i < ${#srds_arr[@]}; i++))
            do
                if [[ "${srds_arr[$i]}" == "$prefix" ]]; then
                    break;
                fi
            done

            # Check if the SerDes ID was found
            if [[ $i -eq ${#srds_arr[@]} ]]; then
                echo "SerDes ID not found for $prefix"
                exit
            fi

            # Create a directory for the device if it does not exist
            mkdir -p "$dev"

            # Extract device type and firmware ID from the device ID
            dev_type=$(( devID & 0xFFFF ))
            fw_id=$(( (devID >> 16) & 0xFFFF ))

            # Generate the SRDS header and write it to a temporary file
            printf "%04x %04x %04x %04x %02x %02x %02x %02x" 32 1 $dev_type $fw_id ${ver:0:1} ${ver:2:1} ${ver:4:1} ${ver:6:1} | xxd -r -p -g0 > $tmp_file
                    
            # Append the content of the filename to the temporary file
            cat "$filename" >> $tmp_file

            # Calculate the checksum of header+FW (the first dword of the .fw file will be 0 - checksum placeholder, so no need to add it)
            checksum=`od -t u4 -An -w4 -v --endian=big $tmp_file | awk '{s=(s+$1)%4294967296; } END {print s}'`

            # Insert the checksum as 32 bits into the .srds file
            srds_file="$dev/$filename.srds"
            printf "%08x" $checksum | xxd -r -p -g0 > $srds_file
            cat $tmp_file >> $srds_file

            # Remove FW version from the current filename
            h_file_name=${filename%%.*}

            # Build header from binary file
            "./serdes_header_build" -b "$dev" -i "$filename" -C -n "$h_file_name"
        fi
    done
    rm -f $tmp_file
}

# Iterate through the files in the current directory
for filename in *.*; do
    # Skip *.sh files
    if [[ "${filename##*.}" == "sh" ]]; then
        continue;
    fi

    # Iterate through the device list and check if the filename contains any of the device names
    for ((i = 0 ; i < $rows; i+=2))
    do
        dev=${device_list[(($i * $COLS))]}
        devID=${device_list[(($i * $COLS + 1))]}
        devIDHex=$(printf "0x%08x" $devID)

        # Check if the device name is found in the filename
        if [[ "$filename" == *$dev* ]] ; then
            echo "Device: $dev, $devIDHex"
            createSRDS "$dev" "$devIDHex"
        fi
    done
done

