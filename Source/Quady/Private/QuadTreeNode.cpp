#include "QuadTreeNode.h"

#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif
#include "QuadTreeViewer.h"

#define LOCTEXT_NAMESPACE "Quady"

FQuadTreeNode::FQuadTreeNode(const FQuadTreeNode* Parent, EQuadrant Quadrant, const FBox& Bounds, const uint8 Level)
    : Quadrant(Quadrant),
    Bounds(Bounds),
    Level(Level),
    bIsSelected(false)
{
    if (Parent == nullptr)
        Key = FQuadTreeNodeKey(Bounds.Min, Level);
    else
        Key = FQuadTreeNodeKey(Parent->Key, Bounds.Min, Quadrant, Level);

    if (Level == 0)
        Children.Empty();
    else
        Split();
}

FQuadTreeNode::~FQuadTreeNode()
{
    ForEachChild([](EQuadrant Quadrant, TSharedPtr<FQuadTreeNode>& Child) {
        Child.Reset();
    });

    Children.Empty();
}

bool FQuadTreeNode::Select(const TSharedPtr<FQuadTreeViewer>& Viewer)
{
    auto RangeSphere = Viewer->GetRange(Level);

    if (Viewer->HasLocationChanged())
    {
        SetSelected(IsInSphere(RangeSphere.GetSphere()));
        if(!bIsSelected) return false;
    }
    
    /* TODO: Frustum representation */
    if (Viewer->HasDirectionChanged())
    {
        SetSelected(IsInFrustum());
        if(!bIsSelected) return false;
    }

    if(Level == 0)
    {
        SetSelected(true);
        return true;
    }
    else
    {
        auto bAnyChildWasSplit = AnyChild([&Viewer](EQuadrant Quadrant, TSharedPtr<FQuadTreeNode>& Child)
        {
            return Child->Select(Viewer);
        }, false);

        /* Constrain, ensures no non-square spaces */
        if (bAnyChildWasSplit)
            ForEachChild([](EQuadrant Quadrant, TSharedPtr<FQuadTreeNode>& Child) { Child->SetSelected(true); });
    }

    return true;
}

bool FQuadTreeNode::Select(const TSharedPtr<FQuadTreeViewer>& Viewer, TSet<FQuadTreeNodeSelectionEvent>& SelectionEvents)
{
    auto RangeSphere = Viewer->GetRange(Level);
    auto Event = FQuadTreeNodeSelectionEvent(Key);

    if (Viewer->HasLocationChanged())
    {
        bool bIsInSphere = IsInSphere(RangeSphere.GetSphere());
        if (bIsInSphere)
            Event.Type |= EQuadTreeNodeSelectionEventType::InRange;
        else
            Event.Type |= EQuadTreeNodeSelectionEventType::OutOfRange;

        SetSelected(bIsInSphere);
        
        if (!bIsSelected)
        {
            /* TODO: Get events */
            SetSelected(false, true);

            SelectionEvents.Add(MoveTemp(Event));
            return false;
        }
    }

    /* TODO: Frustum representation */
    if (Viewer->HasDirectionChanged())
    {
        bool bIsInFrustum = IsInFrustum();
        if (bIsInFrustum)
            Event.Type |= EQuadTreeNodeSelectionEventType::InFrustum;
        else
            Event.Type |= EQuadTreeNodeSelectionEventType::OutOfFrustum;

        SetSelected(bIsInFrustum);
        
        if (!bIsSelected)
        {
            /* TODO: Get events */
            SetSelected(false, true);

            SelectionEvents.Add(MoveTemp(Event));
            return false;
        }
    }

    if (Level == 0)
    {
        SetSelected(true);

        SelectionEvents.Add(MoveTemp(Event));
        return true;
    }
    else
    {
        auto bAnyChildWasSplit = AnyChild([&Viewer, &SelectionEvents](EQuadrant Quadrant, TSharedPtr<FQuadTreeNode>& Child)
        {
            return Child->Select(Viewer, SelectionEvents);
        }, false);

        /* Constrain, ensures no non-square spaces */
        if (bAnyChildWasSplit)
            ForEachChild([](EQuadrant Quadrant, TSharedPtr<FQuadTreeNode>& Child) { Child->SetSelected(true); });
    }

    return true;
}

const bool FQuadTreeNode::IsInSphere(const FSphere& Sphere)
{
    return FBoxSphereBounds::BoxesIntersect(Bounds, Sphere);
}

const bool FQuadTreeNode::IsInFrustum()
{
    return true;
}

void FQuadTreeNode::Draw(const UWorld* World)
{
#if !UE_BUILD_SHIPPING
    if (!bIsSelected)
        return;

    auto Center = Bounds.GetCenter();
    auto Extent = Bounds.GetExtent();

    Center.Z = Level * 100.0f;

    //DrawDebugBox(World, Center, Extent, FQuat::Identity, FColor::White);

    Extent.Z = 0.0f;
    DrawDebugBox(World, Center, Extent, FQuat::Identity, FColor::Red);

    ForEachChild([&World](EQuadrant Quadrant, TSharedPtr<FQuadTreeNode>& Child) {
        Child->Draw(World);
    });
#endif
}

bool FQuadTreeNode::Split()
{
    /* Already Split or Leaf */
    if (Children.Num() > 0 || Level == 0)
        return false;

    auto& Min = Bounds.Min;
    auto& Max = Bounds.Max;

    auto HalfSize = (Max - Min) * 0.5f; 
    auto QuarterSize = HalfSize * 0.5f;

    auto NextLevel = Level - 1;

    const auto TopLeft = FBox(FVector(Min.X + HalfSize.X, Min.Y, -QuarterSize.Z), FVector(Max.X, Min.Y + HalfSize.Y, QuarterSize.Z));
    Children.Emplace(EQuadrant::TopLeft, MakeShareable(new FQuadTreeNode(this, EQuadrant::TopLeft, TopLeft, NextLevel)));

    const auto TopRight = FBox(FVector(Min.X + HalfSize.X, Min.Y + HalfSize.Y, -QuarterSize.Z), FVector(Max.X, Max.Y, QuarterSize.Z));
    Children.Emplace(EQuadrant::TopRight, MakeShareable(new FQuadTreeNode(this, EQuadrant::TopRight, TopRight, NextLevel)));

    const auto BottomLeft = FBox(FVector(Min.X, Min.Y, -QuarterSize.Z), FVector(Min.X + HalfSize.X, Min.Y + HalfSize.Y, QuarterSize.Z));
    Children.Emplace(EQuadrant::BottomLeft, MakeShareable(new FQuadTreeNode(this, EQuadrant::BottomLeft, BottomLeft, NextLevel)));

    const auto BottomRight = FBox(FVector(Min.X, Min.Y + HalfSize.Y, -QuarterSize.Z), FVector(Min.X + HalfSize.X, Max.Y, QuarterSize.Z));
    Children.Emplace(EQuadrant::BottomRight, MakeShareable(new FQuadTreeNode(this, EQuadrant::BottomRight, BottomRight, NextLevel)));
    
    return true;
}

void FQuadTreeNode::SetSelected(const bool bIsSelected, const bool bRecursive /*= false*/)
{
    this->bIsSelected = bIsSelected;

    if(bRecursive)
        ForEachChild([&bIsSelected, &bRecursive](EQuadrant Quadrant, TSharedPtr<FQuadTreeNode>& Child) {
            Child->SetSelected(bIsSelected, bRecursive);
        });
}

void FQuadTreeNode::ForEachChild(TFunction<void(EQuadrant, TSharedPtr<FQuadTreeNode>&)> Func)
{
    if (Children.Num() > 0)
    {
        for (auto& KVP : Children)
            Func(KVP.Key, KVP.Value);
    }
}

bool FQuadTreeNode::AnyChild(TFunction<bool(EQuadrant, TSharedPtr<FQuadTreeNode>&)> Func, bool bTerminateOnFirst)
{
    bool bResult = false;

    if (Children.Num() > 0)
    {
        for (auto& KVP : Children)
            if (Func(KVP.Key, KVP.Value))
            {
                bResult = true;
                if (bTerminateOnFirst)
                    return bResult;
            }
    }

    return bResult;
}

#undef LOCTEXT_NAMESPACE