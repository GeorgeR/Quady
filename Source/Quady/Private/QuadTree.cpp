#include "QuadTree.h"

#include "GameFramework/Actor.h"
#include "ContentStreaming.h"
#include "AssertionMacros.h"
#include "Async.h"
#include "UObjectBase.h"

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
    MaximumQuadSize(102400 * 4),
    ViewerRadiusMultiplier(1.0f)
{
    Observer = MakeShared<FQuadTreeObserver>();

	Build();
}

void UQuadTree::PostLoad()
{
	Super::PostLoad();

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
    auto Range = MinimumQuadSize;
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
	TArray<TSharedPtr<FQuadTreeNode>> Selection;
	Update(Selection);
}

void UQuadTree::Update(TArray<TSharedPtr<FQuadTreeNode>>& OutSelected)
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

	auto bOriginChanged = false;
	if (bFloatingOrigin)
	{
		if (GetOuter())
		{
			if (auto Outer = Cast<AActor>(GetOuter()))
			{
				auto PreviousOrigin = TargetOrigin;

				auto LocationZ = Outer->GetActorLocation().Z;

				TargetOrigin = Observer->GetLocation(false);

				const auto SnapRange = MaximumQuadSize >> 2;

				TargetOrigin.Z = 0.0f;
				TargetOrigin.X = FMath::GridSnap(TargetOrigin.X, SnapRange);
				TargetOrigin.Y = FMath::GridSnap(TargetOrigin.Y, SnapRange);

				TargetOrigin.Z = LocationZ;

				if (PreviousOrigin != TargetOrigin)
				{
					bOriginChanged = true;
					Observer->SetOrigin(TargetOrigin);
				}
			}
		}
	}

	if (bOriginChanged)
	{
		Root.OnOriginChanged(nullptr, TargetOrigin);
	}

	if (Observer->HasLocationChanged() || Observer->HasDirectionChanged())
	{
		//Root.ClearSelected();
		OutSelected.Empty(OutSelected.Num());
		Root.Select(Observer, OutSelected);
	}

	Observer->PostSelect();

#if WITH_EDITOR
	PrevousViewLocation = FirstViewer.GetSphere().Center;
#endif
}

void UQuadTree::Draw(const UWorld* World, const TArray<TSharedPtr<FQuadTreeNode>>& Nodes)
{
    check(World);

#if !UE_BUILD_SHIPPING
	auto Center = TargetOrigin;
	auto Extent = FVector(MaximumQuadSize >> 1, MaximumQuadSize >> 1, 0.0f);

	DrawDebugBox(World, Center, Extent, FQuat::Identity, FColor::Cyan);
#endif

    Observer->Draw(World);

	if(Nodes.Num() > 0)
		for (auto Node : Nodes)
			Node->Draw(World, TargetOrigin);
}

void UQuadTree::Draw(UObject* WorldContextObject)
{
	TArray<TSharedPtr<FQuadTreeNode>> Nodes;
	Draw(WorldContextObject->GetWorld(), Nodes);
}

TArray<TSharedPtr<FQuadTreeNode>> UQuadTree::GetSelectedNodes()
{
	TArray<TSharedPtr<FQuadTreeNode>> Result;


	return Result;
}

#undef LOCTEXT_NAMESPACE
