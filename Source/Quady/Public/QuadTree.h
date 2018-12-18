#pragma once

#include "CoreMinimal.h"
#include "Array.h"

#include "QuadTree.generated.h"

struct FQuadTreeNode;
class UWorld;

// TODO: Deterministic key for nodes
// TODO: Return added and removed nodes on Update

struct FQuadTreeNode
{
public:
    FQuadTreeNode() = default;
    FQuadTreeNode(const FBox& Bounds);
    virtual ~FQuadTreeNode();

    /* Returns true if was split, used for constraints */
    bool RecursiveSplit(const FBoxSphereBounds& Location, const int32& MinimumQuadSize);
    void Split();
    void Empty();
    const bool ContainsOrIntersects(const FBoxSphereBounds& Location);
    const bool IsLeaf() const { return Children.Num() == 0; }

    virtual void Draw(const UWorld* World);
    
private:
    FBox Bounds;
    TArray<TSharedPtr<FQuadTreeNode>, TFixedAllocator<4>> Children;

    inline void ForEachChild(TFunction<void(TSharedPtr<FQuadTreeNode>&)> Func);
    inline bool AnyChild(TFunction<bool(TSharedPtr<FQuadTreeNode>&)> Func, bool bTerminateOnFirst = true);

    const bool LocalContainsOrIntersects(const FBoxSphereBounds& Location);
};

UCLASS(BlueprintType)
class QUADY_API UQuadTree
    : public UObject
{
    GENERATED_BODY()

public:
    /* Not Implemented */
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

#if WITH_EDITOR
    /* For drawing */
    FVector PrevousViewLocation;
#endif

    FQuadTreeNode Root;
};