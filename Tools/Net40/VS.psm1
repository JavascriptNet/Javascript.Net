function Get-VsFileSystem {
    $componentModel = Get-VSComponentModel
    $fileSystemProvider = $componentModel.GetService([NuGet.VisualStudio.IFileSystemProvider])
    $solutionManager = $componentModel.GetService([NuGet.VisualStudio.ISolutionManager])
    
    $fileSystem = $fileSystemProvider.GetFileSystem($solutionManager.SolutionDirectory)
    
    return $fileSystem
}

function Add-PostBuildEvent ($project, $installPath) {
    $currentPostBuildCmd = $project.Properties.Item("PostBuildEvent").Value
    $sqlCEPostBuildCmd = Get-PostBuildCommand $installPath
    # Append our post build command if it's not already there
    if (!$currentPostBuildCmd.Contains($sqlCEPostBuildCmd)) {
        $project.Properties.Item("PostBuildEvent").Value += $SqlCEPostBuildCmd
    }
}

function Add-FilesToDirectory ($srcDirectory, $destDirectory) {
    ls $srcDirectory -Recurse -Filter *.dll  | %{
        $srcPath = $_.FullName

        $relativePath = $srcPath.Substring($srcDirectory.Length + 1)
        $destPath = Join-Path $destDirectory $relativePath
        
        $fileSystem = Get-VsFileSystem
        if (!(Test-Path $destPath)) {
            $fileStream = $null
            try {
                $fileStream = [System.IO.File]::OpenRead($_.FullName)
                $fileSystem.AddFile($destPath, $fileStream)
            } catch {
                # We don't want an exception to surface if we can't add the file for some reason
            } finally {
                if ($fileStream -ne $null) {
                    $fileStream.Dispose()
                }
            }
        }
    }
}

function Remove-FilesFromDirectory ($srcDirectory, $destDirectory) {
    $fileSystem = Get-VsFileSystem
    
    ls $srcDirectory -Recurse -Filter *.dll | %{
        $relativePath = $_.FullName.Substring($srcDirectory.Length + 1)
        $fileInBin = Join-Path $destDirectory $relativePath
        if ($fileSystem.FileExists($fileInBin) -and ((Get-Item $fileInBin).Length -eq $_.Length)) {
            # If a corresponding file exists in bin and has the exact file size as the one inside the package, it's most likely the same file.
            try {
                $fileSystem.DeleteFile($fileInBin)
            } catch {
                # We don't want an exception to surface if we can't delete the file
            }
        }
    }
}

function Remove-PostBuildEvent ($project, $installPath) {
    $sqlCEPostBuildCmd = Get-PostBuildCommand $installPath
    
    try {
        # Get the current Post Build Event cmd
        $currentPostBuildCmd = $project.Properties.Item("PostBuildEvent").Value

        # Remove our post build command from it (if it's there)
        $project.Properties.Item("PostBuildEvent").Value = $currentPostBuildCmd.Replace($SqlCEPostBuildCmd, '')
    } catch {
        # Accessing $project.Properties might throw
    }
}

function Get-PostBuildCommand ($installPath) {
    Write-Host $dte.Solution.FullName $installPath
    $solutionDir = [IO.Path]::GetDirectoryName($dte.Solution.FullName) + "\"
    $path = $installPath.Replace($solutionDir, "`$(SolutionDir)")

    $NativeAssembliesDir = Join-Path $path "NativeBinaries"
    $x86 = $(Join-Path $NativeAssembliesDir "net40\x86\*.*")
    $x64 = $(Join-Path $NativeAssembliesDir "net40\amd64\*.*")

    return "
    if not exist `"`$(TargetDir)x86`" md `"`$(TargetDir)x86`"
    xcopy /s /y `"$x86`" `"`$(TargetDir)x86`"
    if not exist `"`$(TargetDir)amd64`" md `"`$(TargetDir)amd64`"
    xcopy /s /y `"$x64`" `"`$(TargetDir)amd64`""
}

function Get-ProjectRoot($project) {
    try {
        $project.Properties.Item("FullPath").Value
    } catch {

    }
}

function Get-ChildProjectItem($parent, $name) {
	try {
		return $parent.ProjectItems.Item($name);
	} catch {
	
	}
}

function Ensure-Folder($parent, $name) {
	$item = Get-ChildProjectItem $parent $name
	if(!$item) {
		$item = (Get-Interface $parent.ProjectItems "EnvDTE.ProjectItems").AddFolder($name)
	}
	return $item;
}

function Remove-Child($parent, $name) {
	$item = Get-ChildProjectItem $parent $name
	if($item) {
		(Get-Interface $item "EnvDTE.ProjectItem").Delete()
	}
}

function Remove-EmptyFolder($item) {
	if($item.ProjectItems.Count -eq 0) {
		(Get-Interface $item "EnvDTE.ProjectItem").Delete()
	}
}

function Add-ProjectItem($item, $src, $itemtype = "None") {
	$newitem = (Get-Interface $item.ProjectItems "EnvDTE.ProjectItems").AddFromFileCopy($src)
	$newitem.Properties.Item("ItemType").Value = $itemtype
}

Export-ModuleMember -function Add-PostBuildEvent, Add-FilesToDirectory, Remove-PostBuildEvent, Remove-FilesFromDirectory, Get-ProjectRoot, Get-ChildProjectItem, Ensure-Folder, Remove-Child, Remove-EmptyFolder, Add-ProjectItem
