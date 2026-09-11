param(
    [string]$Root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path,
    [Parameter(Mandatory = $true)]
    [string]$UiRoot
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
foreach ($header in @('RPSUIFrameworkApi.h', 'RPSUICooperationApi.h')) {
    $sdkPath = Join-Path $Root ('SDK/UI/include/' + $header)
    $runtimePath = Join-Path $UiRoot ('SDK/include/' + $header)
    if ((Get-FileHash -Algorithm SHA256 -LiteralPath $sdkPath).Hash -ne
        (Get-FileHash -Algorithm SHA256 -LiteralPath $runtimePath).Hash) {
        throw "RPS UI SDK header differs from the runtime source: $header"
    }
}
Write-Host 'RPS SDK UI headers match the runtime source.'
