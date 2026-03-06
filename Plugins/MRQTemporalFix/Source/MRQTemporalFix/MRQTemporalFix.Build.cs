// Copyright 2024 Boos501. All Rights Reserved.

using UnrealBuildTool;

public class MRQTemporalFix : ModuleRules
{
	public MRQTemporalFix(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"MovieRenderPipelineCore",
			"MovieRenderPipelineRenderPasses",
			"RenderCore",
			"Renderer",
			"RHI"
		});
	}
}
