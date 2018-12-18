#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "QuadTreeTestActor.generated.h"

class UQuadTree;

UCLASS(BlueprintType, Blueprintable)
class QUADY_API AQuadTreeTestActor 
    : public AActor
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "QuadTree", meta = (ShowOnlyInnerProperties))
    UQuadTree* QuadTree;

	AQuadTreeTestActor();

    virtual bool ShouldTickIfViewportsOnly() const override { return true; }

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};