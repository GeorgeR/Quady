#include "QuadTree.h"

#include "Quady.h"
#include "ContentStreaming.h"
#include "AssertionMacros.h"
#include "DrawDebugHelpers.h"

#define LOCTEXT_NAMESPACE "Quady"

DECLARE_CYCLE_STAT(TEXT("QuadTree Update"), STAT_QuadTreeUpdate, STATGROUP_Quady);

#pragma region QuadTreeNode

FQuadTreeNode::FQuadTreeNode(const FBox& Bounds)
    : Bounds(Bounds) { }

FQuadTreeNode::~FQuadTreeNode()
{
    ForEachChild([](TSharedPtr<FQuadTreeNode>& Child) {
        Child.Reset();
    });
}

bool FQuadTreeNode::RecursiveSplit(const FBoxSphereBounds& Location, const int32& MinimumQuadSize)
{
    if (!LocalContainsOrIntersects(Location))
    {
        Empty();
        return false;
    }

    if (Bounds.GetSize().X <= MinimumQuadSize)
        return false;

    Split();

    bool bAnyChildWasSplit = AnyChild([&Location, &MinimumQuadSize](TSharedPtr<FQuadTreeNode>& Child) {
        return Child->RecursiveSplit(Location, MinimumQuadSize);
    }, false);

    // TODO: Optimize, this is stupid
    // This performs the constraint step
    if (bAnyChildWasSplit)
        ForEachChild([](TSharedPtr<FQuadTreeNode>& Child) { Child->Split(); });

    return true;
}

void FQuadTreeNode::Split()
{
    /* Already Split */
    if (Children.Num() > 0)
        return;

    auto& Min = Bounds.Min;
    auto& Max = Bounds.Max;

    auto HalfSize = (Max - Min) * 0.5f;

    const auto TopLeft = FBox(FVector(Min.X + HalfSize.X, Min.Y, -HalfSize.Z), FVector(Max.X, Min.Y + HalfSize.Y, HalfSize.Z));
    Children.Insert(MakeShared<FQuadTreeNode>(FQuadTreeNode(TopLeft)), 0);

    const auto TopRight = FBox(FVector(Min.X + HalfSize.X, Min.Y + HalfSize.Y, -HalfSize.Z), FVector(Max.X, Max.Y, HalfSize.Z));
    Children.Insert(MakeShared<FQuadTreeNode>(FQuadTreeNode(TopRight)), 1);

    const auto BottomLeft = FBox(FVector(Min.X, Min.Y, -HalfSize.Z), FVector(Min.X + HalfSize.X, Min.Y + HalfSize.Y, HalfSize.Z));
    Children.Insert(MakeShared<FQuadTreeNode>(FQuadTreeNode(BottomLeft)), 2);

    const auto BottomRight = FBox(FVector(Min.X, Min.Y + HalfSize.Y, -HalfSize.Z), FVector(Min.X + HalfSize.X, Max.Y, HalfSize.Z));
    Children.Insert(MakeShared<FQuadTreeNode>(FQuadTreeNode(BottomRight)), 3);
}

void FQuadTreeNode::Empty()
{
    if (Children.Num() == 0)
        return;

    Children.Empty(4);
}

const bool FQuadTreeNode::ContainsOrIntersects(const FBoxSphereBounds& Location)
{
    if (LocalContainsOrIntersects(Location))
        return true;

    return AnyChild([&Location](TSharedPtr<FQuadTreeNode>& Child) -> bool {
        return Child->ContainsOrIntersects(Location);
    });
}

void FQuadTreeNode::Draw(const UWorld* World)
{
    auto Center = Bounds.GetCenter();
    auto Extent = Bounds.GetExtent();

    Center.Z = 0.0f;

    //DrawDebugBox(World, Center, Extent, FQuat::Identity, FColor::White);

    Extent.Z = 0.0f;
    DrawDebugBox(World, Center, Extent, FQuat::Identity, FColor::Red);

    ForEachChild([&World](TSharedPtr<FQuadTreeNode>& Child) {
        Child->Draw(World);
    });
}

void FQuadTreeNode::ForEachChild(TFunction<void(TSharedPtr<FQuadTreeNode>&)> Func)
{
    if (Children.Num() > 0)
    {
        for (auto& Child : Children)
            Func(Child);
    }
}

bool FQuadTreeNode::AnyChild(TFunction<bool(TSharedPtr<FQuadTreeNode>&)> Func, bool bTerminateOnFirst)
{
    bool bResult = false;

    if (Children.Num() > 0)
    {
        for (auto& Child : Children)
            if (Func(Child))
            {
                bResult = true;
                if (bTerminateOnFirst)
                    return bResult;
            }
                
    }

    return bResult;
}

const bool FQuadTreeNode::LocalContainsOrIntersects(const FBoxSphereBounds& Location)
{
    return FBoxSphereBounds::BoxesIntersect(Bounds, Location);
}

#pragma endregion QuadTreeNode

UQuadTree::UQuadTree()
    : bFloatingOrigin(false),
    MinimumQuadSize(1600),
    MaximumQuadSize(102400),
    ViewerRadiusMultiplier(1.0f)
{
    Build();
}

void UQuadTree::Build()
{
    check(MinimumQuadSize > 0);
    check(MaximumQuadSize > MinimumQuadSize); // Max should be greater than Min
    check(FMath::Frac(MinimumQuadSize / MaximumQuadSize) == 0.0f); // Max should be divisible by Min

    auto HalfSize = MaximumQuadSize * 0.5f;;
    FBox RootBounds(FVector(-HalfSize, -HalfSize, -HalfSize), FVector(HalfSize, HalfSize, HalfSize));
    Root = FQuadTreeNode(RootBounds);
}

void UQuadTree::Update()
{
    FStreamingManagerCollection& StreamingManager = IStreamingManager::Get();
    auto ViewCount = StreamingManager.GetNumViews();
    if (ViewCount <= 0) // Early out
        return;

    PreviousViewLocations.Reset(ViewCount);
    for (auto i = 0; i < ViewCount; i++)
    {
        auto& ViewInfo = StreamingManager.GetViewInformation(i);
        auto ViewOrigin = ViewInfo.ViewOrigin;
        ViewOrigin.Z = 0.0f;
        auto Sphere = FSphere(ViewOrigin, MinimumQuadSize * ViewerRadiusMultiplier);
        PreviousViewLocations.Add(FBoxSphereBounds(Sphere));
    }

    SCOPE_CYCLE_COUNTER(STAT_QuadTreeUpdate);

    // NOTE: Only supports single viewer for now

    auto& FirstViewer = PreviousViewLocations[0];
    Root.RecursiveSplit(FirstViewer, MinimumQuadSize);

#if WITH_EDITOR
    PrevousViewLocation = FirstViewer.GetSphere().Center;
#endif
}

void UQuadTree::Draw(const UWorld* World)
{
    check(World);

#if WITH_EDITOR
    DrawDebugCircle(World, PrevousViewLocation, MinimumQuadSize * ViewerRadiusMultiplier, 64, FColor::Green);
#endif

    Root.Draw(World);
}

#undef LOCTEXT_NAMESPACE