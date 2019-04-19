#pragma once

#include "CoreMinimal.h"
#include "Array.h"

#include "QuadTreeNode.h"

#include "QuadTree.generated.h"

class UWorld;
class FQuadTreeObserver;

UCLASS(BlueprintType)
class QUADY_API UQuadTree
    : public UObject
{
    GENERATED_BODY()

public:
    /* NOTE: Not Implemented */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QuadTree")
    bool bFloatingOrigin;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QuadTree")
    int32 MinimumQuadSize;

    /* Must be a multiple of MinimumQuadSize */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QuadTree")
    int32 MaximumQuadSize;

    /* A viewers radius is the MinimumQuadSize, increase this when the viewer is moving quickly */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QuadTree")
    float ViewerRadiusMultiplier;

    UQuadTree();

    /* Validate and construct QuadTree */
    UFUNCTION(BlueprintCallable, Category = "QuadTree")
    virtual void Build();

    /* Update QuadTree state for viewer */
    UFUNCTION(BlueprintCallable, Category = "QuadTree")
    virtual void Update();

    /* Draw Quads */
    virtual void Draw(const UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "QuadTree", meta = (WorldContext = "WorldContextObject"))
    void Draw(UObject* WorldContextObject) { Draw(WorldContextObject->GetWorld()); }
    
private:
    UPROPERTY(Transient)
    TArray<FBoxSphereBounds> PreviousViewLocations;

    uint8 LevelCount;
    TSharedPtr<FQuadTreeObserver> Observer;

#if WITH_EDITOR
    /* For drawing */
    FVector PrevousViewLocation;
#endif

    FQuadTreeNode Root;
};
