$timestamp = Get-Date -Format "MM-dd HH:mm:ss.fff"
$bspid = adb shell pidof com.beatgames.beatsaber
while ([string]::IsNullOrEmpty($bspid)) {
    Start-Sleep -Milliseconds 100
    $bspid = adb shell pidof com.beatgames.beatsaber
}
if ($args.Count -eq 0) {
    echo "Start logging!"
    adb logcat -T "$timestamp" --pid $bspid | Select-String -pattern "(QuestHook|modloader|AndroidRuntime|CRASH)"
}
if ($args[0] -eq "--file") {
    echo "Logging and saving to file ($PSScriptRoot/logcat.log)!"

    if ($args[1] -eq "--all") {
        adb logcat -T "$timestamp" | Out-File -FilePath $PSScriptRoot\logcat.log
    } else {
        adb logcat -T "$timestamp" --pid $bspid | Select-String -pattern "(QuestHook|modloader|AndroidRuntime|CRASH)" | Out-File -FilePath $PSScriptRoot\logcat.log
    }
}
if ($args[0] -eq "--self") {
    echo "Logging only this mod!"
    adb logcat -T "$timestamp" --pid $bspid | Select-String -pattern "(HotSwappableMods|AndroidRuntime|CRASH)"
}
if ($args[0] -eq "--custom") {
    $pattern = "(" + $args[1] + "|AndriodRuntime|CRASH)"
    echo "Logging With Custom Pattern $pattern"
    adb logcat -T "$timestamp" --pid $bspid | Select-String -pattern $pattern
}
