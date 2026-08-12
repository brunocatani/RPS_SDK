param(
    [string]$Root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$failures = [System.Collections.Generic.List[string]]::new()
$moduleRoot = Join-Path $Root 'SDK/PAPER'
$headerPath = Join-Path $moduleRoot 'include/PAPERApi.h'
$apiIndexPath = Join-Path $moduleRoot 'docs/ApiIndex.md'
$capabilitiesPath = Join-Path $moduleRoot 'docs/DiscoveryAndCapabilities.md'
$gettingStartedPath = Join-Path $moduleRoot 'docs/GettingStarted.md'
$examplesRoot = Join-Path $moduleRoot 'examples'

$requiredFiles = @(
    $headerPath,
    (Join-Path $moduleRoot 'README.md'),
    $apiIndexPath,
    $capabilitiesPath,
    $gettingStartedPath,
    (Join-Path $moduleRoot 'docs/RuntimeContract.md'),
    (Join-Path $moduleRoot 'docs/DataModel.md'),
    (Join-Path $moduleRoot 'docs/FeatureGuide.md'),
    (Join-Path $moduleRoot 'docs/DevelopmentCaptureAndStorage.md'),
    (Join-Path $moduleRoot 'docs/Recipes.md'),
    (Join-Path $moduleRoot 'docs/VersionMatrix.md'),
    (Join-Path $examplesRoot 'README.md'),
    (Join-Path $examplesRoot 'CMakeLists.txt'),
    (Join-Path $examplesRoot 'common/ExampleRuntime.cpp'),
    (Join-Path $examplesRoot 'cmake/VerifyFo4VrLoader.cmake'))
foreach ($path in $requiredFiles) {
    if (-not (Test-Path -LiteralPath $path)) {
        $failures.Add("Missing required PAPER SDK artifact: $path")
    }
}

if (Test-Path -LiteralPath $headerPath) {
    $header = Get-Content -Raw -LiteralPath $headerPath
    $functions = @(
        [regex]::Matches(
            $header,
            'PAPER_CALL\s*\*\s*([A-Za-z_][A-Za-z0-9_]*)') |
            ForEach-Object { $_.Groups[1].Value })
    if ($functions.Count -ne 50) {
        $failures.Add("Expected 50 PAPER V1 table slots; found $($functions.Count)")
    }

    if (Test-Path -LiteralPath $apiIndexPath) {
        $apiIndex = Get-Content -Raw -LiteralPath $apiIndexPath
        $rows = @([regex]::Matches(
            $apiIndex,
            '(?m)^\|\s*(\d+)\s*\|\s*`([A-Za-z_][A-Za-z0-9_]*)`\s*\|'))
        if ($rows.Count -ne $functions.Count) {
            $failures.Add(
                "PAPER API index documents $($rows.Count) slots; header exposes $($functions.Count)")
        } else {
            for ($index = 0; $index -lt $functions.Count; ++$index) {
                if ([int]$rows[$index].Groups[1].Value -ne $index -or
                    $rows[$index].Groups[2].Value -ne $functions[$index]) {
                    $failures.Add("PAPER API index mismatch at slot $index")
                    break
                }
            }
        }
    }

    foreach ($extent in @(
            'PAPER_PROVIDER_API_V1_BASE_TABLE_BYTES',
            'PAPER_PROVIDER_API_V1_RELOAD_OBSERVATION_TABLE_BYTES',
            'PAPER_PROVIDER_API_V1_RELOAD_ANIMATION_TABLE_BYTES',
            'PAPER_PROVIDER_API_V1_RELOAD_STAGE_TABLE_BYTES',
            'PAPER_PROVIDER_API_V1_NATIVE_POSE_PIPELINE_TABLE_BYTES',
            'PAPER_PROVIDER_API_V1_WEAPON_MOTION_CATALOG_TABLE_BYTES',
            'PAPER_PROVIDER_API_V1_WEAPON_MANIPULATION_TABLE_BYTES',
            'PAPER_PROVIDER_API_V1_WEAPON_MOTION_DIAGNOSTICS_TABLE_BYTES',
            'PAPER_PROVIDER_API_V1_DEVELOPMENT_CAPTURE_TABLE_BYTES')) {
        if ($header -notmatch [regex]::Escape($extent)) {
            $failures.Add("PAPER header is missing table extent '$extent'")
        }
    }
}

if (Test-Path -LiteralPath $capabilitiesPath) {
    $capabilities = Get-Content -Raw -LiteralPath $capabilitiesPath
    foreach ($name in @(
            'RuntimeState', 'CapturedTransforms', 'NativeHandPose',
            'AnimationAuthority', 'FrameCallbacks', 'ReloadObservations',
            'ReloadEvidenceGeometry', 'ReloadAnimationEvidence',
            'ReloadAnimationTelemetry', 'ReloadStageIdentification',
            'NativePosePipeline', 'WeaponMotionCatalog',
            'WeaponMotionDiagnostics', 'WeaponManipulationTelemetry',
            'DevelopmentCaptureControl')) {
        if ($capabilities -notmatch [regex]::Escape("``$name``")) {
            $failures.Add("PAPER capability documentation is missing '$name'")
        }
    }
}

if (Test-Path -LiteralPath $gettingStartedPath) {
    $gettingStarted = Get-Content -Raw -LiteralPath $gettingStartedPath
    foreach ($code in 0..5) {
        if ($gettingStarted -notmatch "(?m)^\|\s*$code\s*\|") {
            $failures.Add("Getting started is missing PAPER initialize code $code")
        }
    }
}

$exampleCmakePath = Join-Path $examplesRoot 'CMakeLists.txt'
if (Test-Path -LiteralPath $exampleCmakePath) {
    $exampleCmake = Get-Content -Raw -LiteralPath $exampleCmakePath
    $sources = @(Get-ChildItem -LiteralPath (Join-Path $examplesRoot 'mods') -Filter '*.cpp' -File)
    if ($sources.Count -ne 12) {
        $failures.Add("Expected 12 buildable PAPER examples; found $($sources.Count)")
    }
    foreach ($source in $sources) {
        if ($exampleCmake -notmatch [regex]::Escape($source.Name)) {
            $failures.Add("PAPER example is not wired into CMake: $($source.Name)")
        }
        $text = Get-Content -Raw -LiteralPath $source.FullName
        if ($text -notmatch 'FrameCallbacks') {
            $failures.Add("PAPER event example does not request FrameCallbacks: $($source.Name)")
        }
    }
}

$mutatingExamples = @{
    'mods/DevelopmentCaptureController.cpp' = @(
        'kEnableDevelopmentCapture = false',
        'clearDevelopmentCaptureV1',
        'kLeaseFrames')
    'mods/AnimationAuthorityLease.cpp' = @(
        'kEnableAnimationAuthority = false',
        'clearAnimationAuthorityV1',
        'kLeaseFrames')
}
foreach ($entry in $mutatingExamples.GetEnumerator()) {
    $path = Join-Path $examplesRoot $entry.Key
    if (-not (Test-Path -LiteralPath $path)) {
        continue
    }
    $text = Get-Content -Raw -LiteralPath $path
    foreach ($required in $entry.Value) {
        if ($text -notmatch [regex]::Escape($required)) {
            $failures.Add("$($entry.Key) is missing safety contract '$required'")
        }
    }
}

$publicFiles = Get-ChildItem -LiteralPath $moduleRoot -Recurse -File |
    Where-Object { $_.Extension -in @('.md', '.cpp', '.h', '.cmake', '.txt') }
foreach ($file in $publicFiles) {
    $text = Get-Content -Raw -LiteralPath $file.FullName
    if ($text -match '(?i)PAPER_Toolkit|PAPER Toolkit') {
        $failures.Add("PAPER SDK artifact leaks the separate Toolkit product: $($file.FullName)")
    }
    if ($text -match '#include\s*[<\"](?:PaperMain|PCH|api/PAPERProvider|weapon_motion|animation_evidence)') {
        $failures.Add("PAPER SDK artifact includes a private runtime header: $($file.FullName)")
    }
}

$rootCmake = Get-Content -Raw -LiteralPath (Join-Path $Root 'CMakeLists.txt')
foreach ($target in @('RPS::ROCK', 'RPS::PAPER', 'RPS::SDK', 'PAPERSDKExamplePlugins')) {
    if ($rootCmake -notmatch [regex]::Escape($target)) {
        $failures.Add("Root CMake does not expose '$target'")
    }
}

if ($failures.Count -ne 0) {
    $failures | ForEach-Object { Write-Error $_ }
    exit 1
}

Write-Host 'RPS SDK PAPER documentation/source checks passed.'
