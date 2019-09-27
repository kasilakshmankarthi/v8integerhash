# IMPORTANT for A32 change ABI to armeabi-v7a in Application.mk
DIR=/sarc/spa/users/kasi.a

export NDK_PROJECT_PATH=${DIR}/v8-analysis/v8integerhash

${DIR}/Android/Sdk/ndk-bundle/ndk-build V=1 NDK_APPLICATION_MK=Application.mk clean
${DIR}/Android/Sdk/ndk-bundle/ndk-build V=1 NDK_APPLICATION_MK=Application.mk

