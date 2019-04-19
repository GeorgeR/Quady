#include "QuadTree.h"

#include "ContentStreaming.h"
#include "AssertionMacros.h"
#include "Async.h"

#include "Quady.h"
#include "QuadTreeObserver.h"

#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif

#define LOCTEXT_NAMESPACE "Quady"

DECLARE_CYCLE_STAT(TEXT("QuadTree Update"), STAT_QuadTreeUpdate, STATGROUP_Quady);

UQuadTree::UQuadTree()
    : bFloatingOrigin(false),
    MinimumQuadSize(1600),
    MaximumQuadSize(102400),
    ViewerRadiusMultiplier(1.0f)
{
    Observer = MakeShared<FQuadTreeObserver>();
    Build();
}

void UQuadTree::Build()
{
    check(MinimumQuadSize > 0);
    check(MaximumQuadSize > MinimumQuadSize); // Max should be greater than Min
    check(FMath::Frac(MaximumQuadSize / MinimumQuadSize) == 0.0f); // Max should be divisible by Min

    LevelCount = 0;
    for (auto Level = MinimumQuadSize; Level <= MaximumQuadSize; Level <<= 1)
        LevelCount++;
    
    TArray<float> Ranges;
    Ranges.Empty(LevelCount);
    auto Range = MinimumQuadSize >> 1;
    for (auto i = 0; i < LevelCount; i++)
    {
        Ranges.Add(Range);
        Range <<= 1;
    }

    Observer->SetRanges(Ranges);

    auto HalfSize = MaximumQuadSize * 0.5f;;
    FBox RootBounds(FVector(-HalfSize, -HalfSize, -HalfSize), FVector(HalfSize, HalfSize, HalfSize));
    Root = FQuadTreeNode(nullptr, EQuadrant::None, RootBounds, LevelCount - 1);
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
    Observer->SetLocation(FirstViewer.Origin);

    if (Observer->HasLocationChanged() || Observer->HasDirectionChanged())
    {
        Root.Select(Observer);
    }

    Observer->PostSelect();

#if WITH_EDITOR
    PrevousViewLocation = FirstViewer.GetSphere().Center;
#endif
}

void UQuadTree::Draw(const UWorld* World)
{
    check(World);

    Observer->Draw(World);
    Root.Draw(World);
}

#undef LOCTEXT_NAMESPACE
