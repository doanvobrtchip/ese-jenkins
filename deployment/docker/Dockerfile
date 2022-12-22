FROM mcr.microsoft.com/windows/servercore:1809
LABEL Description="Windows Server Core development environment for Qbs with Qt, Chocolatey and various dependencies for testing Qbs modules and functionality"

# Disable crash dialog for release-mode runtimes
RUN reg add "HKLM\SOFTWARE\Microsoft\Windows\Windows Error Reporting" /v Disabled /t REG_DWORD /d 1 /f
RUN reg add "HKLM\SOFTWARE\Microsoft\Windows\Windows Error Reporting" /v DontShowUI /t REG_DWORD /d 1 /f

# Install VS from the website since chocolatey has broken .NET 4.8 (dotnetfx package) which is a
# dependency for the visualstudio2019buildtools package
RUN powershell -NoProfile -ExecutionPolicy Bypass -Command \
    Invoke-WebRequest "https://aka.ms/vs/16/release/vs_community.exe" \
    -OutFile "%TEMP%\vs_community.exe" -UseBasicParsing

RUN "%TEMP%\vs_community.exe"  --quiet --wait --norestart --noUpdateInstaller \
    --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 \
    --add Microsoft.VisualStudio.Component.Windows10SDK.18362

RUN powershell -NoProfile -ExecutionPolicy Bypass -Command \
    $Env:chocolateyVersion = '0.10.15' ; \
    $Env:chocolateyUseWindowsCompression = 'false' ; \
    "[Net.ServicePointManager]::SecurityProtocol = \"tls12, tls11, tls\"; iex ((New-Object System.Net.WebClient).DownloadString('http://chocolatey.org/install.ps1'))" && SET "PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin"

ARG PYTHON_VERSION=3.10.0
ARG 7ZIP_VERSION=19.0
ARG GIT_VERSION=2.24.0
ARG QBS_VERSION=1.23.1
ARG CMAKE_VERSION=3.25.1
ARG QT_VERSION=6.4.0
ARG INNOSETUP_VERSION=6.2.1

RUN choco install -y python --version %PYTHON_VERSION%
RUN choco install -y 7zip --version %7ZIP_VERSION%
RUN choco install -y git --version %GIT_VERSION% --params "/GitAndUnixToolsOnPath"
RUN choco install -y qbs --version %QBS_VERSION%
RUN choco install -y cmake --version %CMAKE_VERSION% --installargs "ADD_CMAKE_TO_PATH=System"
RUN choco install -y innosetup --version %INNOSETUP_VERSION%

RUN pip install --upgrade pip pillow

# Install Qt
COPY install-qt.sh install-qt.sh

RUN bash -c "./install-qt.sh -d /c/Qt --version ${QT_VERSION} --toolchain win64_msvc2019_64 qtbase qttools qtdeclarative qtscript qtsvg qt5compat opengl32sw"
ENV QTDIR64=C:\\Qt\\${QT_VERSION}\\msvc2019_64

RUN qbs setup-toolchains --detect && \
    qbs setup-qt %QTDIR64%/bin/qmake.exe qt64 && \
    qbs config defaultProfile qt64