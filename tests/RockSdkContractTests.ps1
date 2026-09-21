param(
    [string]$Root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$failures = [System.Collections.Generic.List[string]]::new()
$moduleRoot = Join-Path $Root 'SDK/ROCK'
$includeRoot = Join-Path $moduleRoot 'include/ROCK'
$docsRoot = Join-Path $moduleRoot 'docs'
$indexPath = Join-Path $docsRoot 'ApiIndex.md'

function Read-Required([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path)) {
        $failures.Add("Missing SDK artifact: $Path")
        return ''
    }
    return (Get-Content -Raw -LiteralPath $Path).Replace("`r`n", "`n").Trim()
}
function Assert-Declaration([string]$Document, [string]$Name) {
    $header = Read-Required (Join-Path $includeRoot $Name)
    $pattern = '(?s)<summary>Complete public declarations: ROCK/' +
        [regex]::Escape($Name) + '</summary>\s*```cpp\n(.*?)\n```'
    $block = [regex]::Match($Document, $pattern)
    if (-not $block.Success -or $block.Groups[1].Value.Trim() -cne $header) {
        $failures.Add("Published declarations differ from ROCK/$Name")
    }
}

# The SDK headers define the interface census and versions; documentation does
# not maintain a second hardcoded list of members or legacy capability words.
$abi = Read-Required (Join-Path $includeRoot 'Abi.h')
$enum = [regex]::Match($abi, '(?s)enum class InterfaceId[^\{]*\{([^}]+)\}').Groups[1].Value
$families = @([regex]::Matches($enum, '(\w+)\s*=\s*(\d+)'))
if ($families.Count -eq 0) { $failures.Add('No interface IDs found in the public ABI.') }
$index = Read-Required $indexPath
$indexRows = @([regex]::Matches($index,
    '(?m)^\|\s*(\d+)\s*\|\s*\[(\w+)\]\(modular/\w+\.md\)\s*\|\s*(\d+)\.(\d+)\s*\|\s*`ROCK/(\w+)\.h`\s*\|\s*`RPS::ROCK(\w+)`\s*\|\s*(\d+)\s*\|'))
if ($indexRows.Count -ne $families.Count) {
    $failures.Add("Interface index has $($indexRows.Count) rows; source has $($families.Count).")
}
$memberCount = 0
foreach ($family in $families) {
    $name = $family.Groups[1].Value
    $id = $family.Groups[2].Value
    $header = Read-Required (Join-Path $includeRoot "$name.h")
    $document = Read-Required (Join-Path $docsRoot "modular/$name.md")
    Assert-Declaration $document "$name.h"
    $major = [regex]::Match($header, 'kMajor\s*=\s*(\d+)').Groups[1].Value
    $minor = [regex]::Match($header, 'kMinor\s*=\s*(\d+)').Groups[1].Value
    $functions = @([regex]::Matches($header, 'Status\(ROCK_CALL\*\s*(\w+)\)') |
        ForEach-Object { $_.Groups[1].Value })
    $memberCount += $functions.Count
    $entries = [regex]::Match($document, '(?s)## Entry points\n(.*?)(?:\n## |$)').Groups[1].Value
    $rows = @([regex]::Matches($entries, '(?m)^\|\s*`(\w+)`\s*\|'))
    if ($rows.Count -ne $functions.Count) {
        $failures.Add("$name documents $($rows.Count) calls; source has $($functions.Count).")
    } else {
        for ($i = 0; $i -lt $functions.Count; ++$i) {
            if ($rows[$i].Groups[1].Value -cne $functions[$i]) {
                $failures.Add("$name callable order differs at member $i.")
                break
            }
        }
    }
    $matches = @($indexRows | Where-Object { $_.Groups[2].Value -ceq $name })
    if ($matches.Count -ne 1) { $failures.Add("Expected one index row for $name."); continue }
    $row = $matches[0]
    if ($row.Groups[1].Value -ne $id -or $row.Groups[3].Value -ne $major -or
        $row.Groups[4].Value -ne $minor -or $row.Groups[5].Value -cne $name -or
        $row.Groups[6].Value -cne $name -or [int]$row.Groups[7].Value -ne $functions.Count) {
        $failures.Add("Incorrect identity/version/header/target/member count for $name.")
    }
}
$core = Read-Required (Join-Path $docsRoot 'modular/Core.md')
Assert-Declaration $core 'Discovery.h'
Assert-Declaration $core 'Client.h'
Assert-Declaration (Read-Required (Join-Path $docsRoot 'modular/Abi.md')) 'Abi.h'
foreach ($required in @('PublicApi.md', 'GettingStarted.md', 'RuntimeContract.md',
        'DiscoveryAndCapabilities.md', 'Install.md', 'Recipes.md', 'VersionMatrix.md')) {
    $null = Read-Required (Join-Path $docsRoot $required)
}

# Examples stay generic; documentation may discuss integration with other
# published providers. Actual example behavior/compilation has its own tests.
$examplesRoot = Join-Path $moduleRoot 'examples'
foreach ($file in Get-ChildItem -LiteralPath $examplesRoot -Recurse -File |
        Where-Object { $_.Extension -in @('.cpp', '.h', '.cmake', '.txt') }) {
    $text = Get-Content -Raw -LiteralPath $file.FullName
    if ($text -match '(?i)\bPAPER(?:_Toolkit)?\b|\bSCISSORS\b|\bROCK_Addons\b') {
        $failures.Add("Example names an internal/reference project: $($file.FullName)")
    }
}
if ($failures.Count -ne 0) {
    foreach ($failure in $failures) { [Console]::Error.WriteLine($failure) }
    exit 1
}
Write-Host "RPS SDK ROCK documentation checks passed: $($families.Count) interfaces, $memberCount members, complete declarations."
