// Copyright 2024 Boos501. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SceneViewExtension.h"

/**
 * 层2：下游 ViewExtension 截杀
 * Layer 2: Downstream ViewExtension interceptor.
 *
 * 在渲染管线的 SetupView 阶段强制清零所有 Motion Blur 相关参数，
 * 确保即使上游 FakeShutterAngle 骗值被部分使用，最终渲染帧也不会出现 Motion Blur。
 *
 * Forcibly zeroes all Motion Blur-related parameters at the SetupView stage of the
 * rendering pipeline. This guarantees no Motion Blur appears in the final rendered
 * frames even if the upstream FakeShutterAngle trick is partially consumed by MRQ.
 */
class MRQTEMPORALFIX_API FMRQMotionBlurKillExtension : public FWorldSceneViewExtension
{
public:
	/**
	 * 构造函数，绑定到指定 World
	 * Constructor, binds to the specified World.
	 */
	explicit FMRQMotionBlurKillExtension(const FAutoRegister& AutoRegister, UWorld* InWorld);

	//~ Begin ISceneViewExtension Interface
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override;
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual bool IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const override;
	//~ End ISceneViewExtension Interface

	/**
	 * 激活或停用此扩展
	 * Activate or deactivate this extension.
	 * @param bInActive - true 时启用截杀 / true to enable interception
	 */
	void SetActive(bool bInActive) { bActive = bInActive; }

	/** 当前是否激活 / Whether currently active */
	bool IsActive() const { return bActive; }

private:
	/** 是否激活截杀逻辑 / Whether the interception logic is active */
	bool bActive = false;
};
