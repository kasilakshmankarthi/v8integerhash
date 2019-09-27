# Copyright (c) 2018 SARC
# All Rights Reserved.
# Confidential and Proprietary - SARC.

PROG1 = v8integerhash.elf

ifeq ($(ARCH),aarch64)
  COMPILER=/work/kasilka/bin/ndk21-arm64/bin/aarch64-linux-android-
  INC=/work/kasilka/bin/ndk21-arm64/lib64/clang/6.0.2/include/arm_acle.h
else
  COMPILER=/work/kasilka/bin/ndk21-arm32/bin/arm-linux-androideabi-
  INC=/work/kasilka/bin/ndk21-arm32/lib64/clang/6.0.2/include
endif

CC  = $(COMPILER)clang
CXX = $(COMPILER)clang

#-ggdb
ifeq ($(ARCH),aarch64)
    CFLAGS = -O3 -v -march=armv8.3-a+crypto -D__ARM_FEATURE_CRC32 -Wall -Wno-shift-count-overflow -I${INC}
else
    CFLAGS = -O3 -v -march=armv8.3-a+crypto -D__ARM_FEATURE_CRC32 -Wall -Wno-shift-count-overflow -I${INC}
endif

ifeq ($(ARCH),aarch64)
  LDFLAGS = -static -flto
else
  LDFLAGS = -static -flto
endif

CXXFLAGS = $(CFLAGS)

all: $(PROG1)

SRC1 = v8integerhash.cpp
OBJS1 = $(SRC1:.cpp=.o)

$(PROG1): $(SRC1)
	rm -f $(PROG1) $(OBJS1)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(PROG1) $(OBJS1)
