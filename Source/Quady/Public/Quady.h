#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats2.h"
#include "LogMacros.h"

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_CLASS(LogQuady, Log, All);

DECLARE_STATS_GROUP(TEXT("Quady"), STATGROUP_Quady, STATCAT_Advanced);

class FQuadyModule 
    : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};