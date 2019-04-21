#pragma once

#include "CoreMinimal.h"
#include "Array.h"

#include "QuadTreeNodeKey.h"
#include "QuadTreeNodeSelectionEvent.h"

class FQuadTreeObserver;

// TODO: Deterministic key for nodes
// TODO: Return added and removed nodes on Update

class FQuadTreeNode
	: public TSharedFromThis<FQuadTreeNode>
{
public:
    FQuadTreeNode() = default;
    FQuadTreeNode(const FQuadTreeNode* Parent, EQuadrant Quadrant, const FBox& Bounds, const uint8 Level);

    virtual ~FQuadTreeNode();

	const FBox& GetBounds() const { return Bounds; }

    /* Returns true if was split, used for constraints */
    bool Select(const TSharedPtr<FQuadTreeObserver>& Observer);
	bool Select(const TSharedPtr<FQuadTreeObserver>& Observer, TArray<TSharedPtr<FQuadTreeNode>>& OutSelected);
    bool Select(const TSharedPtr<FQuadTreeObserver>& Observer, TSet<FQuadTreeNodeSelectionEvent>& SelectionEvents);

	inline void ClearSelected() { SetSelected(false, true); }
    inline const bool IsSelected() const { return bIsSelected; }
    const bool IsInSphere(const FSphere& Sphere);
    const bool IsInFrustum(); // TODO

    virtual void Draw(const UWorld* World, const FVector& Origin);

    inline const FQuadTreeNodeKey GetKey() const { return Key; }

    bool operator==(const FQuadTreeNode& Other) const { return Key == Other.Key; }
    bool operator!=(const FQuadTreeNode& Other) const { return !operator==(Other); }

	void OnOriginChanged(const FQuadTreeNode* Parent, const FVector& NewOrigin, const bool bRecursive = true);

    friend uint32 GetTypeHash(const FQuadTreeNode& Node) { return GetTypeHash(Node.Key); }

private:
    FQuadTreeNodeKey Key;
    EQuadrant Quadrant;
    FBox Bounds;
    uint8 Level;
    bool bIsSelected;
    TMap<EQuadrant, TSharedPtr<FQuadTreeNode>> Children;

    bool Split();
    void Empty();
    
    void SetSelected(const bool InIsSelected, const bool bRecursive = false);
    inline void ForEachChild(TFunction<void(EQuadrant, TSharedPtr<FQuadTreeNode>&)> Func);
    inline bool AnyChild(TFunction<bool(EQuadrant, TSharedPtr<FQuadTreeNode>&)> Func, bool bTerminateOnFirst = true);
};
