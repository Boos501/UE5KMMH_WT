// Copyright 2024 Boos501. All Rights Reserved.

#include "MoviePipelineTemporalFix.h"
#include "MoviePipeline.h"
#include "Engine/PostProcessVolume.h"
#include "Camera/CameraComponent.h"
#include "EngineUtils.h"

#define LOCTEXT_NAMESPACE "MRQTemporalFix"

DEFINE_LOG_CATEGORY_STATIC(LogMRQTemporalFix, Log, All);

UMoviePipelineTemporalFix::UMoviePipelineTemporalFix()
{
}

void UMoviePipelineTemporalFix::SetupForPipelineImpl(UMoviePipeline* InPipeline)
{
	if (!bEnabled)
	{
		UE_LOG(LogMRQTemporalFix, Log,
			TEXT("[MRQTemporalFix] Layer1: Fix is disabled, skipping SetupForPipelineImpl."));
		return;
	}

	UWorld* World = InPipeline ? InPipeline->GetWorld() : nullptr;
	if (!World)
	{
		UE_LOG(LogMRQTemporalFix, Warning,
			TEXT("[MRQTemporalFix] Layer1: Cannot get World from pipeline, aborting setup."));
		return;
	}

	OriginalMotionBlurAmounts.Reset();

	// ─────────────────────────────────────────────────────────────────────────
	// 遍历所有 PostProcessVolume，将 MotionBlurAmount=0 替换为 FakeShutterAngle
	// Iterate all PostProcessVolumes and replace MotionBlurAmount=0 with FakeShutterAngle
	// ─────────────────────────────────────────────────────────────────────────
	int32 PatchedPPVCount = 0;
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		APostProcessVolume* PPV = *It;
		if (!IsValid(PPV))
		{
			continue;
		}

		FPostProcessSettings& Settings = PPV->Settings;

		// 只处理显式覆盖了 MotionBlurAmount 且值为 0 的 PPV
		// Only patch PPVs that explicitly override MotionBlurAmount with a value of 0
		if (Settings.bOverride_MotionBlurAmount && FMath::IsNearlyZero(Settings.MotionBlurAmount))
		{
			OriginalMotionBlurAmounts.Add(TWeakObjectPtr<UObject>(PPV), Settings.MotionBlurAmount);
			Settings.MotionBlurAmount = FakeShutterAngle;
			++PatchedPPVCount;

			UE_LOG(LogMRQTemporalFix, Log,
				TEXT("[MRQTemporalFix] Layer1: Patched PostProcessVolume '%s': "
					 "MotionBlurAmount 0 -> %f"),
				*PPV->GetName(), FakeShutterAngle);
		}
	}

	// ─────────────────────────────────────────────────────────────────────────
	// 遍历所有 Actor 上的 UCameraComponent，处理其 PostProcessSettings
	// Iterate all Actors' UCameraComponents and patch their PostProcessSettings
	// ─────────────────────────────────────────────────────────────────────────
	int32 PatchedCamCount = 0;
	for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
	{
		AActor* Actor = *ActorIt;
		if (!IsValid(Actor))
		{
			continue;
		}

		TArray<UCameraComponent*> CameraComponents;
		Actor->GetComponents<UCameraComponent>(CameraComponents);

		for (UCameraComponent* CameraComp : CameraComponents)
		{
			if (!IsValid(CameraComp))
			{
				continue;
			}

			FPostProcessSettings& CamPPSettings = CameraComp->PostProcessSettings;

			// 只处理显式覆盖了 MotionBlurAmount 且值为 0 的 CameraComponent
			// Only patch CameraComponents that explicitly override MotionBlurAmount with 0
			if (CamPPSettings.bOverride_MotionBlurAmount
				&& FMath::IsNearlyZero(CamPPSettings.MotionBlurAmount))
			{
				OriginalMotionBlurAmounts.Add(
					TWeakObjectPtr<UObject>(CameraComp), CamPPSettings.MotionBlurAmount);
				CamPPSettings.MotionBlurAmount = FakeShutterAngle;
				++PatchedCamCount;

				UE_LOG(LogMRQTemporalFix, Log,
					TEXT("[MRQTemporalFix] Layer1: Patched CameraComponent '%s' on Actor '%s': "
						 "MotionBlurAmount 0 -> %f"),
					*CameraComp->GetName(), *Actor->GetName(), FakeShutterAngle);
			}
		}
	}

	UE_LOG(LogMRQTemporalFix, Log,
		TEXT("[MRQTemporalFix] Layer1: Setup complete. "
			 "Patched %d PostProcessVolume(s) and %d CameraComponent(s) with FakeShutterAngle=%f."),
		PatchedPPVCount, PatchedCamCount, FakeShutterAngle);

	// ─────────────────────────────────────────────────────────────────────────
	// 注册层2 ViewExtension，在渲染阶段强制清零 Motion Blur
	// Register Layer 2 ViewExtension to forcibly zero Motion Blur at render time
	// ─────────────────────────────────────────────────────────────────────────
	if (!MotionBlurKillExtension.IsValid())
	{
		MotionBlurKillExtension = FSceneViewExtensions::NewExtension<FMRQMotionBlurKillExtension>(World);
	}
	MotionBlurKillExtension->SetActive(true);

	UE_LOG(LogMRQTemporalFix, Log,
		TEXT("[MRQTemporalFix] Layer2: FMRQMotionBlurKillExtension registered and activated."));
}

void UMoviePipelineTemporalFix::TeardownForPipelineImpl(UMoviePipeline* InPipeline)
{
	// ─────────────────────────────────────────────────────────────────────────
	// 停用层2 ViewExtension
	// Deactivate Layer 2 ViewExtension
	// ─────────────────────────────────────────────────────────────────────────
	if (MotionBlurKillExtension.IsValid())
	{
		MotionBlurKillExtension->SetActive(false);
		MotionBlurKillExtension.Reset();
		UE_LOG(LogMRQTemporalFix, Log,
			TEXT("[MRQTemporalFix] Layer2: FMRQMotionBlurKillExtension deactivated and released."));
	}

	// ─────────────────────────────────────────────────────────────────────────
	// 恢复所有被修改对象的原始 MotionBlurAmount
	// Restore original MotionBlurAmount for all modified objects
	// ─────────────────────────────────────────────────────────────────────────
	int32 RestoredCount = 0;
	for (auto& Pair : OriginalMotionBlurAmounts)
	{
		UObject* Obj = Pair.Key.Get();
		if (!IsValid(Obj))
		{
			continue;
		}

		if (APostProcessVolume* PPV = Cast<APostProcessVolume>(Obj))
		{
			PPV->Settings.MotionBlurAmount = Pair.Value;
			++RestoredCount;
			UE_LOG(LogMRQTemporalFix, Log,
				TEXT("[MRQTemporalFix] Layer1: Restored PostProcessVolume '%s': "
					 "MotionBlurAmount -> %f"),
				*PPV->GetName(), Pair.Value);
		}
		else if (UCameraComponent* CameraComp = Cast<UCameraComponent>(Obj))
		{
			CameraComp->PostProcessSettings.MotionBlurAmount = Pair.Value;
			++RestoredCount;
			UE_LOG(LogMRQTemporalFix, Log,
				TEXT("[MRQTemporalFix] Layer1: Restored CameraComponent '%s': "
					 "MotionBlurAmount -> %f"),
				*CameraComp->GetName(), Pair.Value);
		}
	}

	OriginalMotionBlurAmounts.Reset();

	UE_LOG(LogMRQTemporalFix, Log,
		TEXT("[MRQTemporalFix] Layer1: Teardown complete. Restored %d object(s)."),
		RestoredCount);
}

FText UMoviePipelineTemporalFix::GetCategoryText() const
{
	return LOCTEXT("TemporalFixCategory", "Rendering");
}

FText UMoviePipelineTemporalFix::GetDisplayText() const
{
	return LOCTEXT("TemporalFixDisplayName", "Temporal Sample Fix (No Motion Blur)");
}

#undef LOCTEXT_NAMESPACE
