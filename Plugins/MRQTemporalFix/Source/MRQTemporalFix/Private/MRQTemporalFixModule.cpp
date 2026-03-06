// Copyright 2024 Boos501. All Rights Reserved.

#include "MRQTemporalFixModule.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogMRQTemporalFix, Log, All);

void FMRQTemporalFixModule::StartupModule()
{
	UE_LOG(LogMRQTemporalFix, Log, TEXT("[MRQTemporalFix] Module started. "
		"Use UMoviePipelineTemporalFix setting in your MRQ preset to decouple "
		"Temporal Sample time stepping from Motion Blur shutter angle."));
}

void FMRQTemporalFixModule::ShutdownModule()
{
	UE_LOG(LogMRQTemporalFix, Log, TEXT("[MRQTemporalFix] Module shut down."));
}

IMPLEMENT_MODULE(FMRQTemporalFixModule, MRQTemporalFix)
