using UnrealBuildTool;
using System.Collections.Generic;
using System.IO;

public class NvEncode : ModuleRules {

	private string NvEncodePath {
		get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/NvCodec/")); }
	}

	public NvEncode(ReadOnlyTargetRules Target) : base(Target) {
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "RHI", "RenderCore"});
		PrivatePCHHeaderFile = "Private/NvEncodePCH.h";

		if (Target.Platform != UnrealTargetPlatform.Win64) {
			throw new System.Exception("This plugin is only available for Win64 right now.");
		}

		PrivateIncludePaths.Add(Path.Combine(NvEncodePath, "include/"));
		
		PublicLibraryPaths.Add(Path.Combine(NvEncodePath, "lib/Win64/"));
		
		bEnableExceptions = true;
	}
	
	private void addDependency(string arch, string lib) {
		RuntimeDependencies.Add(Path.Combine(NvEncodePath, "lib/", arch, lib));
	}
}
