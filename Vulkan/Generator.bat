::%1 mshta vbscript:CreateObject("Shell.Application").ShellExecute("cmd.exe","/c %~s0 ::","","runas",1)(window.close)&&exit

echo off

::检测python是否存在
for /f "tokens=*" %%i in ('python -V') do (
	set bPythonExist=%%i
)

if defined bPythonExist (
	echo "python version "%bPythonExist%
) else (
	echo "not find python, please check"
	goto Failed
)

::当前文件夹
set CurDir=%~dp0

::virtual studio 版本号
::set VS_Version="Visual Studio 15 2017"
::set VS_Version="Visual Studio 16 2019"
set VS_Version="Visual Studio 17 2022"

::平台类型 Win32 x64 ARM ARM64
set VS_Platform=x64

::临时文件夹名称
set IntermediateName=Intermediate

::CMakebuild文件夹名称
set CMakeBuildDirName=Build

::设置临时文件夹全路径
set IntermediateFullPath=%CurDir%%IntermediateName%

::设置代码文件夹目录
set SourceDirName=Source

::设置代码文件夹全目录
set SourceFullPath=%CurDir%%SourceDirName%

::设置CMake build文件夹全路径
set CMakeBuildFullPath=%IntermediateFullPath%\%CMakeBuildDirName%

::设置 AutoGenHeader.cmake 文件路径
set AutoGenHeaderPath=%IntermediateFullPath%\%CMakeBuildDirName%\AutoGenHeader.cmake

::不存在就创建Intermediate文件夹
if not exist %IntermediateFullPath% (
    echo Not exist %IntermediateFullPath% dir, create it!!!
    md %IntermediateFullPath% || goto Failed
) 

::存在就删除
if exist %CMakeBuildFullPath% (
    echo Exist %CMakeBuildFullPath% dir, delete it!!!
    rd /s /q %CMakeBuildFullPath%
	@ping -n 2 127.1>nul
	if exist %CMakeBuildFullPath% (
		pause
	)
) 

::创建Build目录
md %CMakeBuildFullPath%

::设置临时文件夹全路径
set PreAutoGenDoFile=%CurDir%\CMake\PreAutoGenDo.py

::调用Python生成模块名和路径的对应关系
python -B %PreAutoGenDoFile% %SourceFullPath% %AutoGenHeaderPath%

::调用CMake, 生成sln
cmake -S %SourceFullPath% -B %CMakeBuildFullPath% -G %VS_Version% -A %VS_Platform%

if %ERRORLEVEL% NEQ 0 (
	pause
)

for %%i in (%CMakeBuildFullPath%\*.sln) do (
    ::获取完整路径名
    set SlnFullPathName=%%i

    ::获取完整文件名
    set "SlnName=%%~nxi"
)

set SlnUrl=%CurDir%%SlnName%

::echo %SlnUrl%
::echo %SlnFullPathName%

::存在就删除
if exist %SlnUrl% (
    echo Exist %SlnUrl% file, delete it!!!
    del /f /q %SlnUrl% || goto Failed
	if %ERRORLEVEL% NEQ 0 (
		goto Failed
	)
	
	if exist %SlnUrl% (
		goto Failed
	)
) 

::创建快捷方式
set TARGET="%SlnFullPathName%"
set SHORTCUT="%SlnUrl%"
mklink %SHORTCUT% %TARGET% || goto Failed

:Succeed
pause
exit


:Failed
echo "Failed"
pause
