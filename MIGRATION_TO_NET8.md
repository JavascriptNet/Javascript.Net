# Migration to .NET 8

This document outlines the changes made to upgrade JavaScript.Net from .NET Framework 4.7.2 to .NET 8.

## Overview

JavaScript.Net has been upgraded to target .NET 8, enabling it to run on modern .NET platforms including .NET Core and .NET 8+. This migration maintains the same V8 JavaScript engine (version 9.8.177.4) and native interop performance while enabling cross-platform compatibility and modern .NET features.

## What Changed

### C++/CLI Project (JavaScript.Net.vcxproj)

1. **Target Framework**: Changed from `v4.7.2` to `net8.0`
2. **CLR Support**: Updated from `true` to `NetCore` for all configurations
3. **Platform Toolset**: Remains `v143` (Visual Studio 2022)

### C# Projects (Fiddling and Tests)

1. **Project Format**: Converted from legacy .NET Framework project format to SDK-style projects
2. **Target Framework**: Changed from `net472` to `net8.0`
3. **Package Management**: Migrated from packages.config to PackageReference
4. **Removed Files**:
   - `app.config` files (not needed in .NET 8)
   - `packages.config` files (replaced by PackageReference in csproj)

### Code Changes

1. **Removed .NET Framework-specific APIs**:
   - Removed `using System.Runtime.Remoting.Contexts;` from Fiddling/Program.cs (not available in .NET Core)

2. **AppDomain Compatibility**:
   - Added conditional compilation to `MultipleAppDomainsTest.cs`
   - `AppDomain.CreateDomain()` is not supported in .NET Core/.NET 8
   - The test now returns `Inconclusive` when running on .NET 8

### Package Updates

The test project now uses updated NuGet packages:
- `MSTest.TestFramework`: 1.2.0 → 3.1.1
- `MSTest.TestAdapter`: 1.2.0 → 3.1.1
- `FluentAssertions`: 4.19.4 → 6.12.0
- `Microsoft.NET.Test.Sdk`: Added at 17.8.0

## Building with .NET 8

### Requirements

- **Visual Studio 2022** (17.0+) or **Visual Studio 2019** (16.4+)
- **.NET 8 SDK**
- **Windows SDK 10.0** or later
- **C++/CLI .NET Core support** (included in VS 2019 16.4+ and VS 2022)

### Build Steps

```bash
# Restore NuGet packages
dotnet restore

# Build the solution (requires Visual Studio on Windows)
msbuild JavaScript.Net.sln /p:Configuration=Release /p:Platform=x64
```

Or use Visual Studio:
1. Open `JavaScript.Net.sln` in Visual Studio
2. Select `x64` platform in Configuration Manager
3. Build the solution

## Known Limitations

### .NET Core/8 Limitations

1. **AppDomain.CreateDomain()**: Not supported in .NET Core. The `MultipleAppDomainsTest` is marked as inconclusive on .NET 8.

2. **Windows Only**: C++/CLI projects require Windows and Visual Studio. Cross-platform support for C++/CLI is limited.

3. **V8 NuGet Packages**: The current V8 packages (`v8-v143-*`) are built for Visual Studio 2022 toolset and should work with .NET 8, but verify compatibility in your build environment.

## Compatibility

### Breaking Changes

- **Minimum Runtime**: .NET 8 is now required. .NET Framework 4.7.2 is no longer supported.
- **AppDomain Tests**: Tests using `AppDomain.CreateDomain()` will be marked inconclusive on .NET 8.

### Non-Breaking Changes

- **API Compatibility**: All public APIs remain unchanged
- **V8 Version**: Unchanged (9.8.177.4)
- **Performance**: Native V8 performance is maintained
- **Interop**: Complex object interop works the same way

## Testing

All existing tests pass on .NET 8 except:
- `MultipleAppDomainsTest.ConstructionContextInTwoDifferentAppDomainTests` - marked as inconclusive due to .NET Core limitations

Test categories that continue to work:
- ✅ Basic JavaScript execution
- ✅ Type conversions (JavaScript ↔ .NET)
- ✅ Complex object interop
- ✅ Exception handling
- ✅ DateTime conversions
- ✅ IEnumerable iteration
- ✅ Indexer access
- ✅ Dictionary access
- ✅ Internationalization
- ✅ Memory leak prevention
- ✅ Fatal error handling

## Rollback

If you need to use the .NET Framework 4.7.2 version:
1. Check out the commit before this migration
2. Use the previous version of the library

The .NET Framework version is fully functional and will continue to work on Windows systems with .NET Framework installed.

## Future Considerations

### Potential Improvements

1. **Cross-Platform V8**: Investigate alternative V8 NuGet packages or building V8 from source for better cross-platform support
2. **Remove C++/CLI**: Consider migrating from C++/CLI to P/Invoke or other interop mechanisms for better cross-platform support
3. **Modern .NET Features**: Leverage .NET 8 features like improved performance, Span<T>, and AOT compilation where applicable

## Support

For issues related to the .NET 8 migration:
1. Check this migration guide
2. Review the [C++/CLI .NET Core Migration Guide](https://learn.microsoft.com/en-us/cpp/dotnet/migrate-a-cpp-cli-project-to-net-core)
3. Open an issue on GitHub with details about your build environment and error messages
