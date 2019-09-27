#adb push libs/arm64-v8a/v8integerhash /data/local/tmp/
#adb shell chmod +x /data/local/tmp/v8integerhash

#echo performance > /sys/devices/system/cpu/cpufreq/policy6/scaling_governor
#echo 2470000 >  /sys/devices/system/cpu/cpufreq/policy6/scaling_min_freq
#echo 2470000 >  /sys/devices/system/cpu/cpufreq/policy6/scaling_max_freq

adb shell /data/local/tmp/v8integerhash --cpu_to_lock 6 --locked_freq 2470000
adb shell /data/local/tmp/v8integerhash --cpu_to_lock 4 --locked_freq 2314000
adb shell /data/local/tmp/v8integerhash --cpu_to_lock 3 --locked_freq 1950000
