Param(
    [Parameter(Mandatory=$false)]
    [Switch] $log,

    [Parameter(Mandatory=$false)]
    [Switch] $self,

    [Parameter(Mandatory=$false)]
    [Switch] $all,

    [Parameter(Mandatory=$false)]
    [Switch] $custom,

    [Parameter(Mandatory=$false)]
    [Switch] $file
)

& $PSScriptRoot/build.ps1
if ($LASTEXITCODE -eq 0) {
    adb push libs/arm64-v8a/libHotSwappableMods.so /sdcard/Android/data/com.beatgames.beatsaber/files/mods/libHotSwappableMods.so
    if ($?) {
        & $PSScriptRoot/restart-game.ps1
        if ($log.IsPresent) {
            if ($self.IsPresent) {
                & $PSScriptRoot/start-logging.ps1 --self
            } elseif ($file.IsPresent) {
                if ($all.IsPresent) {
                    & $PSScriptRoot/start-logging.ps1 --file --all
                } else {
                    & $PSScriptRoot/start-logging.ps1 --file
                }
            } elseif ($custom.IsPresent) {
                & $PSScriptRoot/start-logging.ps1 --custom $args[2]
            } else  {
                & $PSScriptRoot/start-logging.ps1
            }
        }
    }
}
