// Copyright 2024 Boos501. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MoviePipelineSetting.h"
#include "MRQMotionBlurKillExtension.h"
#include "MoviePipelineTemporalFix.generated.h"

/**
 * 层1：上游骗值设置类
 * Layer 1: Upstream value-spoofing pipeline setting.
 *
 * 问题背景 (Problem Background):
 *   MRQ 的 CalculateShotFrameMetrics() 将 TicksPerSample 与 MotionBlurAmount 强耦合。
 *   当 MotionBlurAmount=0 且有多个 Temporal Sample 时，TicksPerSample ≈ 0，
 *   MRQ 打 Error log 后 fallback 到 DeltaFrameTime=1 tick，导致 TAA 采样静默失效。
 *
 *   MRQ's CalculateShotFrameMetrics() tightly couples TicksPerSample with MotionBlurAmount.
 *   When MotionBlurAmount=0 with multiple Temporal Samples, TicksPerSample ≈ 0,
 *   MRQ logs an error and falls back to DeltaFrameTime=1 tick, silently breaking TAA sampling.
 *
 * 解决方案 (Solution):
 *   1. 在 SetupForPipelineImpl 中，将场景中所有 MotionBlurAmount=0 的 PPV 和 CameraComponent
 *      替换为 FakeShutterAngle（一个非零的小值），让 MRQ 的时间计算正常工作。
 *   2. 同时注册 FMRQMotionBlurKillExtension，在渲染阶段强制清零 Motion Blur，
 *      确保最终帧不会出现不需要的模糊效果。
 *   3. 在 TeardownForPipelineImpl 中恢复所有原始值，停用 ViewExtension。
 *
 *   1. In SetupForPipelineImpl, replace MotionBlurAmount=0 on all PPVs and CameraComponents
 *      with FakeShutterAngle (a small non-zero value) so MRQ's time calculation works correctly.
 *   2. Simultaneously register FMRQMotionBlurKillExtension to forcibly zero Motion Blur
 *      at render time, ensuring no unwanted blur appears in the final frames.
 *   3. In TeardownForPipelineImpl, restore all original values and deactivate the ViewExtension.
 */
UCLASS(Blueprintable, meta=(DisplayName="Temporal Sample Fix (No Motion Blur)"))
class MRQTEMPORALFIX_API UMoviePipelineTemporalFix : public UMoviePipelineSetting
{
	GENERATED_BODY()

public:
	UMoviePipelineTemporalFix();

	//~ Begin UMoviePipelineSetting Interface
	virtual void SetupForPipelineImpl(UMoviePipeline* InPipeline) override;
	virtual void TeardownForPipelineImpl(UMoviePipeline* InPipeline) override;
	virtual FText GetCategoryText() const override;
	virtual FText GetDisplayText() const override;
	virtual void BuildNewProcessCommandLineImpl(
		TArray<FString>& InOutUnrealURLParams,
		TArray<FString>& InOutCommandLineArgs,
		TArray<FString>& InOutDeviceProfileCVars,
		TArray<FString>& InOutExecCmds) const override {}
	//~ End UMoviePipelineSetting Interface

public:
	/**
	 * 用于欺骗 MRQ 时间步进计算的假快门角度（归一化值，对应实际角度 = FakeShutterAngle * 360°）
	 * Fake shutter angle used to trick MRQ's time-step calculation.
	 * (Normalized value; actual angle = FakeShutterAngle * 360°)
	 *
	 * 设置为一个足够小的非零值，让 TicksPerSample > 0 的同时不产生可见模糊。
	 * Set to a small non-zero value so TicksPerSample > 0 without producing visible blur.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Temporal Fix",
		meta=(ClampMin="0.01", ClampMax="1.0",
			  ToolTip="Fake shutter angle (normalized 0.01-1.0) injected to prevent MRQ TicksPerSample from collapsing to zero when MotionBlurAmount=0. The ViewExtension layer will zero out Motion Blur at render time."))
	float FakeShutterAngle = 0.05f;

	/**
	 * 是否启用此修复 / Whether this fix is enabled
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Temporal Fix",
		meta=(ToolTip="Enable or disable the Temporal Sample Fix. When disabled, no modifications are made to the scene."))
	bool bEnabled = true;

private:
	/**
	 * 保存被修改对象的原始 MotionBlurAmount，以便 Teardown 时恢复。
	 * Stores the original MotionBlurAmount of modified objects for restoration during Teardown.
	 * Key: weak pointer to the modified UObject (PostProcessVolume or CameraComponent)
	 * Value: original MotionBlurAmount before our modification
	 */
	TMap<TWeakObjectPtr<UObject>, float> OriginalMotionBlurAmounts;

	/**
	 * 层2 ViewExtension 引用，用于渲染阶段截杀 Motion Blur
	 * Layer 2 ViewExtension reference for intercepting Motion Blur at render time
	 */
	TSharedPtr<FMRQMotionBlurKillExtension, ESPMode::ThreadSafe> MotionBlurKillExtension;
};
