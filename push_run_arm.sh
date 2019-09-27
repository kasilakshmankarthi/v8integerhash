ARCH=$1

adb push  binaries/v8integerhash.aarch64.elf /data/local/tmp/
adb shell chmod +x /data/local/tmp/v8integerhash.${ARCH}.elf

#echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor
#echo performance > /sys/devices/system/cpu/cpufreq/policy4/scaling_governor
#echo performance > /sys/devices/system/cpu/cpufreq/policy6/scaling_governor
#echo 2470000 >  /sys/devices/system/cpu/cpufreq/policy6/scaling_min_freq
#echo 2470000 >  /sys/devices/system/cpu/cpufreq/policy6/scaling_max_freq

adb shell "echo Running at 2.47GHz"
adb shell /data/local/tmp/v8integerhash.${ARCH}.elf --cpu_to_lock 6 --locked_freq 2470000
adb shell "echo Running at 2.314GHz"
adb shell /data/local/tmp/v8integerhash.${ARCH}.elf --cpu_to_lock 4 --locked_freq 2314000
adb shell "echo Running at 1.95GHz"
adb shell /data/local/tmp/v8integerhash.${ARCH}.elf --cpu_to_lock 3 --locked_freq 1950000
