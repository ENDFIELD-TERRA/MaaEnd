#Requires -Version 5.1
<#
.SYNOPSIS
Print a Qoder CN Agent Teams launch plan; -Start opens the foreground TUI.
.DESCRIPTION
No model session, package install, daemon, webhook, --help, or --version probe is
started in plan mode. The TUI is intentionally interactive; use /quit to end its
team, or Ctrl+C to interrupt. Session environment and working directory are restored.
#>
[CmdletBinding()]
param(
    [string] $RepoPath = (Get-Location).Path,
    [string] $ConfigRoot,
    [switch] $Start
)

$ErrorActionPreference = 'Stop'
$repoResolved = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($RepoPath)
$configResolved = $null
if (-not [string]::IsNullOrWhiteSpace($ConfigRoot)) {
    $configResolved = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($ConfigRoot)
}
$effectiveConfig = $configResolved
if ([string]::IsNullOrWhiteSpace($effectiveConfig)) {
    $effectiveConfig = [Environment]::GetEnvironmentVariable('QODERCN_CONFIG_DIR', 'Process')
    if ([string]::IsNullOrWhiteSpace($effectiveConfig)) {
        $effectiveConfig = Join-Path ([Environment]::GetFolderPath('UserProfile')) '.qoder-cn'
    }
}

Write-Host 'Qoder CN Agent Teams launch plan'
Write-Host ('Repository: ' + $repoResolved)
Write-Host ('User configuration: ' + $effectiveConfig)
Write-Host 'Session environment: QODERCN_AGENT_TEAMS=1'
Write-Host 'Foreground command: qodercn --permission-mode default --agent perlica-maaend'
Write-Host 'The team is beta. Confirm actual Agent, SendMessage, and shared Task schemas in /tools.'
Write-Host 'Read the installed private contract and use the explicit Agent Teams entry prompt.'
Write-Host 'Use /quit to end this TUI and its teammates. Resume requires rebuilding teammates.'
Write-Host 'This launcher does not enable cross-session sockets, install MCP, or publish to GitHub.'
if (-not $Start) {
    Write-Host 'Plan only. Add -Start after reviewing it to open an interactive, billable model session.'
    return
}

if (-not (Test-Path -LiteralPath $repoResolved -PathType Container)) {
    throw 'RepoPath must name an existing installed repository.'
}
if ($configResolved -and -not (Test-Path -LiteralPath $configResolved -PathType Container)) {
    throw 'ConfigRoot must exist. This launcher does not create configuration directories.'
}
$runtimePath = Join-Path $repoResolved '.qoder/maaend-private/runtime.json'
$mainPath = Join-Path $repoResolved '.qoder/agents/perlica-maaend.md'
if (-not (Test-Path -LiteralPath $runtimePath -PathType Leaf)) {
    throw 'Installed private runtime.json is missing. Review and apply install.py first.'
}
if (-not (Test-Path -LiteralPath $mainPath -PathType Leaf)) {
    throw 'The installed perlica-maaend definition is missing. Review install.py first.'
}
# Resolve an installed executable without running model or version/help probes.
$qoderCommand = Get-Command qodercn -CommandType Application,ExternalScript -ErrorAction Stop
$qoderPath = $qoderCommand.Source
if ([string]::IsNullOrWhiteSpace($qoderPath)) {
    throw 'qodercn executable could not be resolved on PATH.'
}

$hadTeamEnv = Test-Path Env:QODERCN_AGENT_TEAMS
$oldTeamEnv = [Environment]::GetEnvironmentVariable('QODERCN_AGENT_TEAMS', 'Process')
$hadConfigEnv = Test-Path Env:QODERCN_CONFIG_DIR
$oldConfigEnv = [Environment]::GetEnvironmentVariable('QODERCN_CONFIG_DIR', 'Process')
$pushedLocation = $false
try {
    [Environment]::SetEnvironmentVariable('QODERCN_AGENT_TEAMS', '1', 'Process')
    if ($configResolved) {
        [Environment]::SetEnvironmentVariable('QODERCN_CONFIG_DIR', $configResolved, 'Process')
    }
    Push-Location -LiteralPath $repoResolved
    $pushedLocation = $true
    & $qoderPath '--permission-mode' 'default' '--agent' 'perlica-maaend'
    $qoderExitCode = $LASTEXITCODE
    if ($null -ne $qoderExitCode -and $qoderExitCode -ne 0) {
        throw ('qodercn exited with code ' + $qoderExitCode + '. Check the original stderr above.')
    }
}
finally {
    if ($pushedLocation) { Pop-Location }
    if ($hadTeamEnv) {
        [Environment]::SetEnvironmentVariable('QODERCN_AGENT_TEAMS', $oldTeamEnv, 'Process')
    }
    else {
        [Environment]::SetEnvironmentVariable('QODERCN_AGENT_TEAMS', $null, 'Process')
    }
    if ($hadConfigEnv) {
        [Environment]::SetEnvironmentVariable('QODERCN_CONFIG_DIR', $oldConfigEnv, 'Process')
    }
    else {
        [Environment]::SetEnvironmentVariable('QODERCN_CONFIG_DIR', $null, 'Process')
    }
}
