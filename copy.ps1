Param(
    [Parameter(Mandatory=$false)]
    [Switch] $log,

    [Parameter(Mandatory=$false)]
    [Switch] $self,

    [Parameter(Mandatory=$false)]
    [Switch] $all,

    [Parameter(Mandatory=$false)]
    [String] $custom="",

    [Parameter(Mandatory=$false)]
    [Switch] $file,

    [Parameter(Mandatory=$false)]
    [Switch] $help
)

if ($help -eq $true) {
    echo "`"Copy`" - Builds and copies your mod to your quest, and also starts Beat Saber with optional logging"
    echo "`n-- Arguments --`n"

    echo "-Log `t`t Logs Beat Saber using the `"Start-Logging`" command"

    echo "`n-- Logging Arguments --`n"

    & $PSScriptRoot/start-logging.ps1 -help -excludeHeader

    exit
}

& $PSScriptRoot/build.ps1
if ($LASTEXITCODE -eq 0) {
    adb push libs/arm64-v8a/libHotSwappableMods.so /sdcard/Android/data/com.beatgames.beatsaber/files/mods/libHotSwappableMods.so
    if ($?) {
        & $PSScriptRoot/restart-game.ps1
        if ($log -eq $true) { & $PSScriptRoot/start-logging.ps1 -self:$self -all:$all -custom:$custom -file:$file }
    }
}
