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
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "RHI", "RenderCore", "SBGUnrealAdapter" });

		if (Target.Platform != UnrealTargetPlatform.Win64) {
			throw new System.Exception("This plugin is only available for Win64 right now.");
		}

		PrivateIncludePaths.Add(Path.Combine(NvEncodePath, "include/"));
		PrivateIncludePaths.Add(Path.Combine(NvEncodePath, "include/CUDA/"));
		
		addLibrary("Win64", "cuda.lib");
		addLibrary("Win64", "cudart.lib");
		
		bEnableExceptions = true;
	}
	
	private void addLibrary(string arch, string lib) {
		PublicAdditionalLibraries.Add(Path.Combine(NvEncodePath, "lib/", arch, lib));
	}
	
	private void addDependency(string arch, string lib) {
		RuntimeDependencies.Add(Path.Combine(NvEncodePath, "lib/", arch, lib));
	}
}
