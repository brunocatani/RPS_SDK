param(
    [string]$Root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path,
    [Parameter(Mandatory = $true)]
    [string]$RockRoot
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$pairs = @(
    @('SDK/ROCK/include/ROCKProviderApi.h', 'src/api/ROCKProviderApi.h'),
    @('SDK/ROCK/include/ROCKApi.h', 'src/api/ROCKApi.h'),
    @('SDK/ROCK/include/ROCKConfigurationApi.h', 'src/api/ROCKConfigurationApi.h')
)

foreach ($pair in $pairs) {
    $sdkPath = Join-Path $Root $pair[0]
    $runtimePath = Join-Path $RockRoot $pair[1]
    if (-not (Test-Path -LiteralPath $sdkPath)) {
        throw "Missing SDK header: $sdkPath"
    }
    if (-not (Test-Path -LiteralPath $runtimePath)) {
        throw "Missing ROCK runtime header: $runtimePath"
    }

    $sdkHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $sdkPath).Hash
    $runtimeHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $runtimePath).Hash
    if ($sdkHash -ne $runtimeHash) {
        throw "Header drift: $($pair[0]) differs from $($pair[1])"
    }
}

Write-Host 'RPS SDK ROCK headers match the runtime source.'
