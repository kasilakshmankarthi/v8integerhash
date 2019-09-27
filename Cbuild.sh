# Copyright (c) 2016 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

TARGET=$1
echo "Target chosen: " $TARGET

if [ "$TARGET" = "ALL" ]; then
    #Build aarch64 binaries
    make -f Makefile clean
    make -f Makefile ARCH=aarch64
    mv v8integerhash.elf binaries/v8integerhash.aarch64.elf
    echo "Completed building AARCH64 elf"

    #make -f Makefile clean
    #make -f Makefile ARCH=aarch32
    #mv v8integerhash.elf binaries/v8integerhash.aarch32.elf
    #echo "Completed building AARCH64 elf"
fi


