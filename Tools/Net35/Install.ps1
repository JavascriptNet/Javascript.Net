param($installPath, $toolsPath, $package, $project)

Import-Module (Join-Path $toolsPath VS.psd1)
$nativeBinDirectory = Join-Path $installPath "NativeBinaries\net35"
if ($project.Type -eq 'Web Site') {
    $projectRoot = Get-ProjectRoot $project
    if (!$projectRoot) {
        return;
    }

    $binDirectory = Join-Path $projectRoot "bin"
    $libDirectory = Join-Path $installPath "lib\net35"
    Add-FilesToDirectory $libDirectory $binDirectory
    Add-FilesToDirectory $nativeBinDirectory $binDirectory
}
elseif($project.ExtenderNames -contains "WebApplication") {
	$depAsm = Ensure-Folder $Project "_bin_deployableAssemblies";
	if($depAsm) {
		$amd64 = Ensure-Folder $depAsm "amd64";
		if($amd64) {
			$amd64dir = (Join-Path $nativeBinDirectory "amd64")
			$crt64 = Ensure-Folder $amd64 "Microsoft.VC90.CRT";
			if($crt64) {
				$crt64dir = (Join-Path $amd64dir "Microsoft.VC90.CRT")
				Add-ProjectItem $crt64 (Join-Path $crt64dir "Microsoft.VC90.CRT.manifest");
				Add-ProjectItem $crt64 (Join-Path $crt64dir "msvcm90.dll");
				Add-ProjectItem $crt64 (Join-Path $crt64dir "msvcp90.dll");
				Add-ProjectItem $crt64 (Join-Path $crt64dir "msvcr90.dll");
			}
		}
		$x86 = Ensure-Folder $depAsm "x86";
		if($x86) {
			$x86dir = (Join-Path $nativeBinDirectory "x86")
			$crt32 = Ensure-Folder $x86 "Microsoft.VC90.CRT";
			if($crt32) {
				$crt32dir = (Join-Path $x86dir "Microsoft.VC90.CRT")
				Add-ProjectItem $crt32 (Join-Path $crt32dir "Microsoft.VC90.CRT.manifest");
				Add-ProjectItem $crt64 (Join-Path $crt32dir "msvcm90.dll");
				Add-ProjectItem $crt64 (Join-Path $crt32dir "msvcp90.dll");
				Add-ProjectItem $crt64 (Join-Path $crt32dir "msvcr90.dll");
			}
		}
	}
}
else {
    Add-PostBuildEvent $project $installPath
}

$project.Object.References | Where-Object { $_.Name -eq 'Noesis.Javascript' } | ForEach-Object { write-host $_.Name }

Remove-Module VS


$allowedReferences = @("^(Noesis\.Javascript)(,.*)?$")

# Full assembly name is required
Add-Type -AssemblyName 'Microsoft.Build, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a'

$projectCollection = [Microsoft.Build.Evaluation.ProjectCollection]::GlobalProjectCollection
 
$allProjects = $projectCollection.GetLoadedProjects($project.Object.Project.FullName).GetEnumerator();

if($allProjects.MoveNext())
{
	$currentProject = $allProjects.Current

	foreach($Reference in $currentProject.GetItems('Reference') | % {$_})
	{
		$allowedReferenceMatches = $allowedReferences | ? {$Reference.Xml.Include -match $_}

		if (($allowedReferenceMatches | Measure-Object).Count -gt 0)
		{
			$allowedReferenceMatch = $allowedReferenceMatches | Select-Object -First 1
			$include = $Reference.Xml.Include -replace $allowedReferenceMatch, '$1'
			$hintPath = $Reference.GetMetadataValue("HintPath")
			
			write-host "Matched againt $hintPath"
			
			#If it is x64 specific add condition (Include 'Any Cpu' as x64)
			if ($hintPath -match '.*\\(amd64|x64)\\.*\.dll$')
			{
				$Reference.Xml.Condition = '''$(PlatformTarget)'' != ''x86'''
				
				$condition = $Reference.Xml.Condition
				write-host "hintPath = $hintPath"
				write-host "condition = $condition"
				
				#Visual Studio doesnt allow the same reference twice (so try add friends)
				$matchingReferences = $currentProject.GetItems('Reference') | ? {($_.Xml.Include -match $allowedReferenceMatch) -and ($_.GetMetadataValue("HintPath") -match ".*\\(x86)\\.*\.dll$")}
				
				if (($matchingReferences | Measure-Object).Count -eq 0)
				{
					$x86 = $hintPath -replace '(.*\\)(amd64|x64)(\\.*\.dll)$', '$1x86$3'
					$x86Path = Join-Path $installPath $x86
					
					if (Test-Path $x86Path) {
						#Add 
						write-host "Adding reference to $x86"
						
						$metaData = new-object "System.Collections.Generic.Dictionary``2[[System.String, mscorlib, Version=2.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089],[System.String, mscorlib, Version=2.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089]]"
						$metaData.Add("HintPath", $x86)
						$currentProject.AddItem('Reference', $include, $metaData)
						
						$newReference = $currentProject.GetItems('Reference') | ? {($_.Xml.Include -eq $include) -and ($_.GetMetadataValue("HintPath") -eq $x86)} | Select-Object -First 1
						
						$newReference.Xml.Condition = '''$(PlatformTarget)'' == ''x86'''			
					}
				}
			}
			
			#If it is x86 specific add condition 
			if ($hintPath -match '.*\\x86\\.*\.dll$')
			{
				$Reference.Xml.Condition = '''$(PlatformTarget)'' == ''x86'''
				
				$condition = $Reference.Xml.Condition
				write-host "hintPath = $hintPath"
				write-host "condition = $condition"
				
				#Visual Studio doesnt allow the same reference twice (so try add friends)
				$matchingReferences = $currentProject.GetItems('Reference') | ? {($_.Xml.Include -match $allowedReferenceMatch) -and ($_.GetMetadataValue("HintPath") -match ".*\\(amd64|x64)\\.*\.dll$")}

				if (($matchingReferences | Measure-Object).Count -eq 0)
				{
					$x64 = $hintPath -replace '(.*\\)(x86)(\\.*\.dll)$', '$1x64$3'
					$x64Path = Join-Path $installPath $x64
					
					if (Test-Path $x64Path) {
						#Add 
						write-host "Adding reference to $x64"
						
						$metaData = new-object "System.Collections.Generic.Dictionary``2[[System.String, mscorlib, Version=2.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089],[System.String, mscorlib, Version=2.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089]]"
						$metaData.Add("HintPath", $x64)
						$currentProject.AddItem('Reference', $include, $metaData)
						
						$newReference = $currentProject.GetItems('Reference') | ? {($_.Xml.Include -eq $include) -and ($_.GetMetadataValue("HintPath") -eq $x64)} | Select-Object -First 1
						
						$newReference.Xml.Condition = '''$(PlatformTarget)'' != ''x86'''	
					} else {
						$amd64 = $hintPath -replace '(.*\\)(x86)(\\.*\.dll)$', '$1amd64$3'
						$amd64Path = Join-Path $installPath $amd64
						
						if (Test-Path $amd64Path) {
							#Add 
							write-host "Adding reference to $amd64"
							
							$metaData = new-object "System.Collections.Generic.Dictionary``2[[System.String, mscorlib, Version=2.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089],[System.String, mscorlib, Version=2.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089]]"
							$metaData.Add("HintPath", $amd64)
							$currentProject.AddItem('Reference', $include, $metaData)
							
							$newReference = $currentProject.GetItems('Reference') | ? {($_.Xml.Include -eq $include) -and ($_.GetMetadataValue("HintPath") -eq $amd64)} | Select-Object -First 1
							
							$newReference.Xml.Condition = '''$(PlatformTarget)'' != ''x86'''			
						}				
					}				
				}			
			}
		}
	}
}