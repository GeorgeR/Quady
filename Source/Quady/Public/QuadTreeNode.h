#pragma once

#include "CoreMinimal.h"
#include "Array.h"

#include "QuadTreeNodeKey.h"
#include "QuadTreeNodeSelectionEvent.h"

class FQuadTreeObserver;

class FQuadTreeNode
	: public TSharedFromThis<FQuadTreeNode>
{
public:
    FQuadTreeNode() = default;
    FQuadTreeNode(const FQuadTreeNode* Parent, EQuadrant Quadrant, const FBox& Bounds, const uint8 Level);

    virtual ~FQuadTreeNode();

	const FBox& GetBounds() const { return Bounds; }

    /* Returns true if was split, used for constraints */
	bool Select(const TSharedPtr<FQuadTreeObserver>& Observer, TArray<TSharedPtr<FQuadTreeNode>>& OutSelected);
    bool Select(const TSharedPtr<FQuadTreeObserver>& Observer, TSet<FQuadTreeNodeSelectionEvent>& SelectionEvents);

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
    TMap<EQuadrant, TSharedPtr<FQuadTreeNode>> Children;

	void UpdateKey(const FQuadTreeNode* Parent);
    bool Split();
    void Empty();
    
    inline void ForEachChild(TFunction<void(EQuadrant, TSharedPtr<FQuadTreeNode>&)> Func);
    inline bool AnyChild(TFunction<bool(EQuadrant, TSharedPtr<FQuadTreeNode>&)> Func, bool bTerminateOnFirst = true);
};
