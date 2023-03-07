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
    # winget install --id=Microsoft.VisualStudio.2019.BuildTools  -e
    # & 'C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin\MSBuild.exe' .\erpcgen\VisualStudio_v14\erpcgen.sln /property:Configuration=Release
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
