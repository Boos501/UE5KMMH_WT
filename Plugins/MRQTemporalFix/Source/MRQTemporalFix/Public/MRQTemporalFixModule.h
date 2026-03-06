// Copyright 2024 Boos501. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * MRQTemporalFix 模块接口
 * Module interface for MRQTemporalFix plugin.
 * 
 * 该插件通过双层夹击方案解决 MRQ 中 Temporal Sample 与 Motion Blur 强耦合的问题：
 * This plugin uses a dual-layer approach to decouple Temporal Sample time stepping
 * from Motion Blur shutter angle in Movie Render Queue.
 *
 * 层1 (Layer 1): UMoviePipelineTemporalFix — 上游骗值设置类，替换 MotionBlurAmount=0 为 FakeShutterAngle
 * 层2 (Layer 2): FMRQMotionBlurKillExtension — 下游 ViewExtension 截杀，渲染时强制关闭 Motion Blur
 */
class MRQTEMPORALFIX_API FMRQTemporalFixModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
