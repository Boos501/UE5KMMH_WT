// Copyright 2024 Boos501. All Rights Reserved.

#include "MRQMotionBlurKillExtension.h"
#include "SceneView.h"

DEFINE_LOG_CATEGORY_STATIC(LogMRQKillExt, Verbose, All);

FMRQMotionBlurKillExtension::FMRQMotionBlurKillExtension(
	const FAutoRegister& AutoRegister, UWorld* InWorld)
	: FWorldSceneViewExtension(AutoRegister, InWorld)
{
}

bool FMRQMotionBlurKillExtension::IsActiveThisFrame_Internal(
	const FSceneViewExtensionContext& Context) const
{
	return bActive;
}

void FMRQMotionBlurKillExtension::SetupView(
	FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	if (!bActive)
	{
		return;
	}

	// 层2 截杀：在渲染管线 SetupView 阶段强制清零所有 Motion Blur 相关参数
	// Layer 2 interception: forcibly zero all Motion Blur parameters at the SetupView stage.
	//
	// 这是必要的，因为层1 注入的 FakeShutterAngle 让 MRQ 的 TicksPerSample 计算正常工作，
	// 但我们不希望这个假值在渲染阶段产生实际的运动模糊效果。
	//
	// This is necessary because the FakeShutterAngle injected by Layer 1 keeps
	// MRQ's TicksPerSample calculation healthy, but we don't want that fake value
	// to produce actual motion blur in the rendered output.

	// 防止 Camera Cut 触发额外的 Motion Blur 清零逻辑
	// Prevent CameraCut from triggering extra Motion Blur clearing logic
	InView.bCameraCut = false;

	// 强制清零 Motion Blur 强度和最大值 / Force Motion Blur intensity and max to zero
	InView.FinalPostProcessSettings.MotionBlurAmount = 0.0f;
	InView.FinalPostProcessSettings.MotionBlurMax    = 0.0f;

	// 在 ShowFlags 层面也关闭 Motion Blur，提供双重保险
	// Disable Motion Blur at the ShowFlags level as well for redundant safety
	InViewFamily.EngineShowFlags.SetMotionBlur(false);

	UE_LOG(LogMRQKillExt, Verbose,
		TEXT("[MRQTemporalFix] Layer2: Motion Blur zeroed in SetupView "
			 "(MotionBlurAmount=0, MotionBlurMax=0, ShowFlags.MotionBlur=false)."));
}
