#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats2.h"
#include "LogMacros.h"

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_CLASS(LogQuady, Log, All);

DECLARE_STATS_GROUP(TEXT("Quady"), STATGROUP_Quady, STATCAT_Advanced);

/*
Landscape to Quady terminology:
--------------------------------
Quad        |       Cell
Section     |       Quad
Component   |       Quad
*/

class FQuadyModule 
    : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};