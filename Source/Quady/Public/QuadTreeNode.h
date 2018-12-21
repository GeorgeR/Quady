#pragma once

#include "CoreMinimal.h"
#include "Array.h"

class FQuadTreeViewer;

// TODO: Deterministic key for nodes
// TODO: Return added and removed nodes on Update

enum class EQuadrant : uint8
{
    TopLeft = 0,
    TopRight = 1,
    BottomLeft = 2,
    BottomRight = 3,
    None = 4 // Root
};

struct FQuadTreeNodeKey;

struct FQuadTreeNodeKey
{
public:
    FQuadTreeNodeKey() : Key(0) { }

    /* Root */
    FQuadTreeNodeKey(const FVector& BottomLeft, const uint8 Level)
    {
        Key = GetTypeHash(BottomLeft) << 16 | (uint32)EQuadrant::None << 8 | (uint32)Level;
    }

    FQuadTreeNodeKey(const FQuadTreeNodeKey& Parent, const FVector& BottomLeft, const EQuadrant Quadrant, const uint8 Level)
    {
        Key = Parent.GetKey() | GetTypeHash(BottomLeft) << 16 | (uint32)Quadrant << 8 | (uint32)Level;
    }

    inline const uint32 GetKey() const { return Key; }
    inline const bool IsValid() const { return Key > 0; }
    
    bool operator==(const FQuadTreeNodeKey& Other) const { return Key == Other.Key; }
    bool operator!=(const FQuadTreeNodeKey& Other) const { return !operator==(Other); }
    friend uint32 GetTypeHash(const FQuadTreeNodeKey& Key) { return Key.GetKey(); }

private:
    uint32 Key;
};

/* Flags */
enum class EQuadTreeNodeSelectionEventType : uint8
{
    None = 0,
    InRange = 1,
    OutOfRange = 2,
    InFrustum = 4,
    OutOfFrustum = 8,
    DueToParent = 16
};
ENUM_CLASS_FLAGS(EQuadTreeNodeSelectionEventType);

struct FQuadTreeNodeSelectionEvent
{
public:
    EQuadTreeNodeSelectionEventType Type;
    const FQuadTreeNodeKey Key;
    bool bIsSelected;

    FQuadTreeNodeSelectionEvent(const FQuadTreeNodeKey Key) 
        : Type(EQuadTreeNodeSelectionEventType::None),
        Key(Key),
        bIsSelected(false) { }

    bool operator==(const FQuadTreeNodeSelectionEvent& Other) const { return Key == Other.Key; }
    bool operator!=(const FQuadTreeNodeSelectionEvent& Other) const { return !operator==(Other); }

    friend uint32 GetTypeHash(const FQuadTreeNodeSelectionEvent& Event) { return Event.Key.GetKey(); }
};

struct FQuadTreeNode
{
public:
    FQuadTreeNode() = default;
    FQuadTreeNode(const FQuadTreeNode* Parent, EQuadrant Quadrant, const FBox& Bounds, const uint8 Level);

    virtual ~FQuadTreeNode();

    /* Returns true if was split, used for constraints */
    bool Select(const TSharedPtr<FQuadTreeViewer>& Viewer);
    bool Select(const TSharedPtr<FQuadTreeViewer>& Viewer, TSet<FQuadTreeNodeSelectionEvent>& SelectionEvents);

    inline const bool IsSelected() const { return bIsSelected; }
    const bool IsInSphere(const FSphere& Sphere);
    const bool IsInFrustum(); // TODO

    virtual void Draw(const UWorld* World);

    inline const FQuadTreeNodeKey GetKey() const { return Key; }

    bool operator==(const FQuadTreeNode& Other) const { return Key == Other.Key; }
    bool operator!=(const FQuadTreeNode& Other) const { return !operator==(Other); }

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
    
    void SetSelected(const bool bIsSelected, const bool bRecursive = false);
    inline void ForEachChild(TFunction<void(EQuadrant, TSharedPtr<FQuadTreeNode>&)> Func);
    inline bool AnyChild(TFunction<bool(EQuadrant, TSharedPtr<FQuadTreeNode>&)> Func, bool bTerminateOnFirst = true);
};