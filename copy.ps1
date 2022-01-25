Param(
    [Parameter(Mandatory=$false)]
    [Switch] $clean,

    [Parameter(Mandatory=$false)]
    [Switch] $log,

    [Parameter(Mandatory=$false)]
    [Switch] $useDebug,

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

    echo "-Clean `t`t Performs a clean build (equvilant to running `"Build -clean`")"
    echo "-UseRelease `t Copied the release version of the mod to your quest, instead of the debug version"
    echo "-Log `t`t Logs Beat Saber using the `"Start-Logging`" command"

    echo "`n-- Logging Arguments --`n"

    & $PSScriptRoot/start-logging.ps1 -help -excludeHeader

    exit
}

& $PSScriptRoot/build.ps1 -clean:$clean

if ($LASTEXITCODE -ne 0) {
    echo "Failed to build, exiting..."
    exit $LASTEXITCODE
}

$fileName = Get-ChildItem lib*.so -Path "build/" -Name
$fileToCopy = "debug_$fileName"

if ($useRelease -eq $true) {
    $fileToCopy = $fileName
}

& adb push build/$fileToCopy /sdcard/Android/data/com.beatgames.beatsaber/files/mods/$fileName

if ($log -eq $true) { & $PSScriptRoot/start-logging.ps1 -self:$self -all:$all -custom:$custom -file:$file -restart }