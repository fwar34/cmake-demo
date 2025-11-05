param (
    [Parameter(Mandatory = $false)][string]$task
)

$workspaceFolderBasename = (Get-Item -Path ".").Name
$elfFile = "$workspaceFolderBasename.elf"  # 明确指定ELF名称
$hexFile = "$workspaceFolderBasename.hex"
$binFile = "$workspaceFolderBasename.bin"

function Assert-ToolExists {
    param($toolName)
    if (-not (Get-Command $toolName -ErrorAction SilentlyContinue)) {
        Write-Host "$toolName not found in PATH!" -ForegroundColor Red
        exit 1
    }
}

function CMakeConfigure {
    Assert-ToolExists "cmake"
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -GNinja -Bbuild
    if (-not $?) {
        Write-Host "CMake配置失败！检查CMakeLists.txt" -ForegroundColor Red
        exit 1
    }
    Write-Host "CMake configure success" -ForegroundColor Green
}

function CMakeBuild {
    Assert-ToolExists "arm-none-eabi-objcopy"
    
    $buildSuccess = $true
    if (-not (Test-Path -Path "build/$elfFile")) {
        CMakeConfigure
        if (-not $?) { $buildSuccess = $false }
    }
    
    if ($buildSuccess) {
        cmake --build build --target all | ForEach-Object {
            if ($_ -imatch "\b(error|致命错误):\s") {
                Write-Host $_ -ForegroundColor Red
            } elseif ($_ -imatch "\b(warning|注意):\s") {
                Write-Host $_ -ForegroundColor Yellow
            } else {
                Write-Host $_
            }
        }
        
        if ($?) {
            arm-none-eabi-objcopy -Oihex "build/$elfFile" "build/$hexFile"
            if (-not $?) {
                Write-Host "生成HEX文件失败!" -ForegroundColor Red
                return $false
            }
            arm-none-eabi-objcopy -Obinary "build/$elfFile" "build/$binFile"
            if (-not $?) {
                Write-Host "生成BIN文件失败!" -ForegroundColor Red
                return $false
            }

            Write-Host "CMake build success" -ForegroundColor Green
            return $true
        } else {
            return $false
        }
    }
    return $false
}

function Flash {
    Assert-ToolExists "openocd"
    
    if (-not (Test-Path -Path "build/$elfFile")) {
        if (-not (CMakeBuild)) {
            Write-Host "编译失败，无法烧录！" -ForegroundColor Red
            return
        }
    }
    openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program ./build/$hexFile verify reset exit"
    if (-not $?) {
        Write-Host "烧录失败！检查硬件连接，是否硬件正在使用或者调试。" -ForegroundColor Red
        return
    }
    Write-Host "CMake build success" -ForegroundColor Green
}

function DeleteBuild {
    if (Test-Path -Path 'build') {
        Remove-Item -Path 'build' -Recurse -Force
        Write-Host 'Build directory deleted.' -ForegroundColor Green
    } else {
        Write-Host 'Build directory does not exist.' -ForegroundColor Yellow
    }
}

switch ($task) {
    "CMakeConfigure" { CMakeConfigure }
    # 函数执行成功时，除了预期的成功消息外，还会打印“True”。这可能是因为函数返回了$true，
    # 而PowerShell默认会输出函数的返回值到控制台
    # 使用管道传递到Out-Null**：例如，`CMakeBuild | Out-Null`，这样返回值被丢弃，不会显示。
    "CMakeBuild" { CMakeBuild | Out-Null } 
    "Flash" { Flash }
    "DeleteBuild" { DeleteBuild }
    default {
        Write-Host "可用任务: CMakeConfigure, CMakeBuild, Flash, DeleteBuild"
        exit 1
    }
}