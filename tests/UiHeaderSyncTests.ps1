param(
    [string]$Root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path,
    [Parameter(Mandatory = $true)]
    [string]$UiRoot
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$sdkPath = Join-Path $Root 'SDK/UI/include/RPSUIFrameworkApi.h'
$runtimePath = Join-Path $UiRoot 'SDK/include/RPSUIFrameworkApi.h'
if ((Get-FileHash -Algorithm SHA256 -LiteralPath $sdkPath).Hash -ne
    (Get-FileHash -Algorithm SHA256 -LiteralPath $runtimePath).Hash) {
    throw 'RPS UI SDK header differs from the runtime-owned public header.'
}
Write-Host 'RPS SDK UI header matches the runtime source.'
