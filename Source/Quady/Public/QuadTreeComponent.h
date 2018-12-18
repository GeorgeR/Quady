#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "QuadTreeComponent.generated.h"

UCLASS(ClassGroup=(Quady), meta=(BlueprintSpawnableComponent))
class QUADY_API UQuadTreeComponent 
    : public USceneComponent
{
	GENERATED_BODY()

public:
	UQuadTreeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};