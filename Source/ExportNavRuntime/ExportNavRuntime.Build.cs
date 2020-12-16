// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class ExportNavRuntime : ModuleRules
{
	public ExportNavRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(EngineDirectory,"Source/Runtime/Launch")
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"NavigationSystem",
				"NavMesh"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);


		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
		
		PublicDefinitions.AddRange(
                new string[]
                {
                    "USE_DETOUR_BUILT_INTO_UE4",
                    "EXPORT_NAV_MESH_AS_M",
                    "EXPORT_NAV_MESH_AS_CM",
                }
            );
	}
}
