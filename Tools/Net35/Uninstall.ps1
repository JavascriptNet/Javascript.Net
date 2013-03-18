param($installPath, $toolsPath, $package, $project)

Import-Module (Join-Path $toolsPath VS.psd1)
if ($project.Type -eq 'Web Site') {
    $projectRoot = Get-ProjectRoot $project
    if (!$projectRoot) {
        return;
    }

    $binDirectory = Join-Path $projectRoot "bin"
    $libDirectory = Join-Path $installPath "lib\net35"
    $nativeBinDirectory = Join-Path $installPath "NativeBinaries"

    Remove-FilesFromDirectory $libDirectory $binDirectory
    Remove-FilesFromDirectory $nativeBinDirectory $binDirectory
}
elseif($project.ExtenderNames -contains "WebApplication") {
	$depAsm = Get-ChildProjectItem $Project "_bin_deployableAssemblies";
	if($depAsm) {
		$amd64 = Get-ChildProjectItem $depAsm "amd64";
		if($amd64) {
			$crt64 = Get-ChildProjectItem $amd64 "Microsoft.VC90.CRT";
			if($crt64) {
				Remove-Child $crt64 "Microsoft.VC90.CRT.manifest";
				Remove-Child $crt64 "msvcm90.dll";
				Remove-Child $crt64 "msvcp90.dll";
				Remove-Child $crt64 "msvcr90.dll";
				Remove-EmptyFolder $crt64;
			}
			Remove-EmptyFolder $amd64;
		}
		$x86 = Get-ChildProjectItem $depAsm "x86";
		if($x86) {
			$crt32 = Get-ChildProjectItem $x86 "Microsoft.VC90.CRT";
			if($crt32) {
				Remove-Child $crt32 "Microsoft.VC90.CRT.manifest";
				Remove-Child $crt32 "msvcm90.dll";
				Remove-Child $crt32 "msvcp90.dll";
				Remove-Child $crt32 "msvcr90.dll";
				Remove-EmptyFolder $crt32;
			}
			Remove-EmptyFolder $x86;
		}
	}
	Remove-EmptyFolder $depAsm
}
else {
    Remove-PostBuildEvent $project $installPath
}
Remove-Module VS

$allowedReferences = @("Noesis.Javascript")

# Full assembly name is required
Add-Type -AssemblyName 'Microsoft.Build, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a'

$projectCollection = [Microsoft.Build.Evaluation.ProjectCollection]::GlobalProjectCollection
 
$allProjects = $projectCollection.GetLoadedProjects($project.Object.Project.FullName).GetEnumerator();

if($allProjects.MoveNext())
{
	foreach($Reference in $allProjects.Current.GetItems('Reference') | ? {$allowedReferences -contains $_.UnevaluatedInclude })
	{
		$allProjects.Current.RemoveItem($Reference)
	}
}