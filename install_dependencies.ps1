# Binson/flex
Set-Location ".\erpcgen\VisualStudio_v14"
$PatHBisonFlex7Zip=".\win_flex_bison.zip"
$PatHBisonFlex="win_flex_bison"
Remove-Item -ErrorAction Ignore $PatHBisonFlex7Zip
Remove-Item -ErrorAction Ignore -Recurse $PatHBisonFlex
Remove-Item -ErrorAction Ignore -Recurse ".\custom_build_rules"
Remove-Item -ErrorAction Ignore -Recurse ".\data"
Remove-Item -ErrorAction Ignore -Recurse ".\changelog.md"
Remove-Item -ErrorAction Ignore -Recurse ".\FlexLexer.h"
Remove-Item -ErrorAction Ignore -Recurse ".\README.md"
Remove-Item -ErrorAction Ignore -Recurse ".\win_bison.exe"
Remove-Item -ErrorAction Ignore -Recurse ".\win_flex.exe"
$URLBisonFlex = "https://github.com/lexxmark/winflexbison/releases/download/v2.5.25/win_flex_bison-2.5.25.zip"
Invoke-WebRequest -URI $URLBisonFlex -OutFile $PatHBisonFlex7Zip
Expand-Archive -Path $PatHBisonFlex7Zip
$PatHBisonFlexFull= ".\"+$PatHBisonFlex+"\*"
Move-Item -Path $PatHBisonFlexFull -Destination .\ -force
Remove-Item -ErrorAction Ignore $PatHBisonFlex7Zip
# Remove-Item -ErrorAction Ignore -Recurse $PatHBisonFlex
Set-Location "..\..\"

if ($args[0] -eq "VS")
{
    $URLBuildtools="https://download.visualstudio.microsoft.com/download/pr/33d686db-3937-4a19-bb3c-be031c5d69bf/66d85abf1020496b07c59aba176def5127352f2fbdd3c4c4143738ab7dfcb459/vs_BuildTools.exe"
    $PathBuildtools="vs_BuildTools.exe"
    Remove-Item -ErrorAction Ignore $PathBuildtools
    Invoke-WebRequest -URI $URLBuildtools -OutFile $PathBuildtools
    Start-Process $PathBuildtools -ArgumentList "--quiet --add Microsoft.VisualStudio.Workload.VCTools --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --wait"  -Wait
}
else
{
    # Install 7zip
    $URL7Zip = "https://gist.githubusercontent.com/dansmith65/7dd950f183af5f5deaf9650f2ad3226c/raw/8b8f6e96de7469cea73c9fe63a5da4d44a7c1ba7/Install-7zip.ps1"
    $Path7Zip = ".\Install-7zip.ps1"
    Remove-Item -ErrorAction Ignore $Path7Zip
    Invoke-WebRequest -URI $URL7Zip -OutFile $Path7Zip
    powershell $Path7Zip
    Remove-Item -ErrorAction Ignore $Path7Zip
    $Path7zipApp="C:\Program Files\7-Zip\7z.exe"

    # Install mingw
    $URLMingw = "https://github.com/niXman/mingw-builds-binaries/releases/download/12.2.0-rt_v10-rev2/x86_64-12.2.0-release-win32-seh-ucrt-rt_v10-rev2.7z"
    $PathMingw7Zip = "mingw.7z"
    $PathMingw = "mingw64"
    $UnzipMingw = 'x',$PathMingw7Zip,'-y'
    Remove-Item -ErrorAction Ignore $PathMingw7Zip
    Remove-Item -ErrorAction Ignore -Recurse $PathMingw
    Invoke-WebRequest -URI $URLMingw -OutFile $PathMingw7Zip
    & $Path7zipApp $UnzipMingw
    Remove-Item -ErrorAction Ignore $PathMingw7Zip
}
