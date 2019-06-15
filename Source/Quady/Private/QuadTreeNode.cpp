#include "QuadTreeNode.h"

#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif

#include "QuadTreeObserver.h"
#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "Quady"

FQuadTreeNode::FQuadTreeNode(const FQuadTreeNode* Parent, EQuadrant Quadrant, const FBox& Bounds, const uint8 Level)
    : Quadrant(Quadrant),
    Bounds(Bounds),
    Level(Level)
{
	UpdateKey(Parent);

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

bool FQuadTreeNode::Select(const TSharedPtr<FQuadTreeObserver>& Observer, TArray<TSharedPtr<FQuadTreeNode>>& OutSelected)
{
	auto RangeBounds = Observer->GetRange(Level);

	auto bIsInSphere = IsInSphere(RangeBounds.GetSphere());
	if (!bIsInSphere)
		return false;

	// #todo Revisit check
	/*if (Observer->HasLocationChanged())
	{

	}*/

	//auto bIsInFrustum = IsInFrustum();
	//if (!bIsInFrustum)
	//	return true;

	// #todo Revisit check
	///* #todo Frustum representation */
	//if (Observer->HasDirectionChanged())
	//{
	//    if(!bIsSelected) return true;
	//}

	if (Level == 0)
	{
		OutSelected.Add(this->AsShared());
		return true;
	}
	else
	{
		if (Level < Observer->GetLevelNum() - 1 && !IsInSphere(Observer->GetRange(Level + 1).GetSphere()))
		{
			OutSelected.Add(this->AsShared());
		}
		else
		{
			//auto bAnyChildWasSplit = AnyChild([&Observer](EQuadrant Quadrant, TSharedPtr<FQuadTreeNode>& Child)
			//{
			//	return Child->Select(Observer);
			//}, false);

			ForEachChild([&](EQuadrant Quadrant, TSharedPtr<FQuadTreeNode>& Child) {
				if (!Child->Select(Observer, OutSelected))
				{
					OutSelected.Add(Child);
				}
			});

			///* Constrain, ensures no non-square spaces */
			//if (bAnyChildWasSplit)
			//	ForEachChild([](EQuadrant Quadrant, TSharedPtr<FQuadTreeNode>& Child) { Child->SetSelected(true); });
		}
	}

	return true;
}

bool FQuadTreeNode::Select(const TSharedPtr<FQuadTreeObserver>& Observer, TSet<FQuadTreeNodeSelectionEvent>& SelectionEvents)
{
    auto RangeSphere = Observer->GetRange(Level);
    auto Event = FQuadTreeNodeSelectionEvent(Key);

    if (Observer->HasLocationChanged())
    {
        bool bIsInSphere = IsInSphere(RangeSphere.GetSphere());
        if (bIsInSphere)
            Event.Type |= EQuadTreeNodeSelectionEventType::InRange;
        else
            Event.Type |= EQuadTreeNodeSelectionEventType::OutOfRange;

		
		if (!bIsInSphere)
		{
			SelectionEvents.Add(MoveTemp(Event));
			return false;
		}
    }

    /* #todo Frustum representation */
    if (Observer->HasDirectionChanged())
    {
        bool bIsInFrustum = IsInFrustum();
        if (bIsInFrustum)
            Event.Type |= EQuadTreeNodeSelectionEventType::InFrustum;
        else
            Event.Type |= EQuadTreeNodeSelectionEventType::OutOfFrustum;

        if (!bIsInFrustum)
        {
            SelectionEvents.Add(MoveTemp(Event));
            return true;
        }
    }

    if (Level == 0)
    {
        SelectionEvents.Add(MoveTemp(Event));
        return true;
    }
    else
    {
		ForEachChild([&](EQuadrant Quadrant, TSharedPtr<FQuadTreeNode>& Child) {
			if (!Child->Select(Observer, SelectionEvents))
			{
				auto ChildEvent = FQuadTreeNodeSelectionEvent(Child->GetKey());
				SelectionEvents.Add(ChildEvent);
			}
		});
    }

    return true;
}

const bool FQuadTreeNode::IsInSphere(const FSphere& Sphere)
{
	return FMath::SphereAABBIntersection(Sphere, Bounds);
}

const bool FQuadTreeNode::IsInFrustum()
{
    return true;
}

void FQuadTreeNode::Draw(const UWorld* World, const FVector& Origin)
{
#if !UE_BUILD_SHIPPING
	auto Center = Bounds.GetCenter();
	Center += Origin;

	auto Extent = Bounds.GetExtent();

	Extent.Z = 0.0f;
	DrawDebugBox(World, Center, Extent, FQuat::Identity, FColor::Red);
#endif
}

void FQuadTreeNode::OnOriginChanged(const FQuadTreeNode* Parent, const FVector& NewOrigin, const bool bRecursive /*= true*/)
{
	auto Center = Bounds.GetCenter();
	Center += NewOrigin;
	Bounds = Bounds.MoveTo(Center);

	UpdateKey(Parent);

	Center -= NewOrigin;
	Bounds = Bounds.MoveTo(Center);

	if (bRecursive)
	{
		ForEachChild([&](EQuadrant Quadrant, TSharedPtr<FQuadTreeNode>& Child) {
			Child->OnOriginChanged(Parent, NewOrigin, bRecursive);
		});
	}
}

void FQuadTreeNode::UpdateKey(const FQuadTreeNode* Parent)
{
	if (Parent == nullptr)
		Key = FQuadTreeNodeKey(Bounds.Min, Level);
	else
		Key = FQuadTreeNodeKey(Parent->Key, Bounds.Min, Quadrant, Level);
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

void FQuadTreeNode::Empty()
{
	Children.Empty();
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
