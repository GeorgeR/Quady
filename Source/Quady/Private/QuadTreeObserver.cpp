#include "QuadTreeObserver.h"

#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif
#include "UObject/NoExportTypes.h"

#define LOCTEXT_NAMESPACE "Quady"

void FQuadTreeObserver::SetOrigin(const FVector& InOrigin)
{
	if (!this->Origin.Equals(InOrigin))
	{
		this->Origin = InOrigin;
		this->bLocationDirty = true;

		for (auto& Range : Ranges)
			Range.Origin = GetLocation();
	}
}

bool FQuadTreeObserver::HasLocationChanged(bool bClearFlag /*= false*/)
{
    if (bClearFlag && bLocationDirty)
    {
        bLocationDirty = false;
        return true;
    }

    return bLocationDirty;
}

const bool FQuadTreeObserver::HasLocationChanged() const
{
    return bLocationDirty;
}

const FVector FQuadTreeObserver::GetLocation(bool bRelativeToOrigin) const
{
    return bRelativeToOrigin ? Location - Origin : Location;
}

void FQuadTreeObserver::SetLocation(const FVector& InLocation)
{
    if (!this->Location.Equals(InLocation))
    {
        this->Location = InLocation;
        this->bLocationDirty = true;

        for (auto& Range : Ranges)
            Range.Origin = InLocation - Origin;
    }
}

const bool FQuadTreeObserver::HasDirectionChanged(bool bClearFlag /*= false*/)
{
    if (bClearFlag && bLocationDirty)
    {
        bDirectionDirty = false;
        return true;
    }

    return bDirectionDirty;
}

const bool FQuadTreeObserver::HasDirectionChanged() const
{
    return bDirectionDirty;
}

const FVector& FQuadTreeObserver::GetDirection() const
{
    return Direction;
}

void FQuadTreeObserver::SetDirection(const FVector& InDirection)
{
    if (!this->Direction.Equals(InDirection))
    {
        this->Direction = InDirection;
        this->bDirectionDirty = true;
    }
}

const FBoxSphereBounds& FQuadTreeObserver::GetRange(const uint8& InLevel) const
{
    check(InLevel < Ranges.Num());

    return Ranges[InLevel];
}

void FQuadTreeObserver::SetRanges(const TArray<float>& InRanges)
{
    this->Ranges.Empty(InRanges.Num());
    for (auto i = 0; i < InRanges.Num(); i++)
    {
        auto RangeSphere = FSphere(Location, InRanges[i]);
        auto Range = FBoxSphereBounds(RangeSphere);
        this->Ranges.Emplace(Range);
    }
}

void FQuadTreeObserver::PostSelect()
{
    bLocationDirty = false;
    bDirectionDirty = false;
}

void FQuadTreeObserver::Draw(const UWorld* InWorld)
{
#if !UE_BUILD_SHIPPING
    static const FQuat Rotation = FQuat::MakeFromEuler(FVector(0, 90, 0));

	auto RangeLocation = Ranges[0].Origin;
	RangeLocation.Z = 0.0f;
	RangeLocation += Origin;

    for (auto& Range : Ranges)
    {
        auto Transform = FTransform(Rotation, RangeLocation, FVector::OneVector);
        DrawDebugCircle(InWorld, Transform.ToMatrixNoScale(), Range.SphereRadius, 64, FColor::Green);
    }
#endif
}

void FQuadTreeObserver::UpdateFrustum()
{
	bool bIsStereo = false;
	if(bIsStereo)
	{
		const FMatrix LeftEyeViewProjection;
		const FMatrix RightEyeViewProjection;

		FConvexVolume LeftEyeBounds, RightEyeBounds;
		GetViewFrustumBounds(LeftEyeBounds, LeftEyeViewProjection, false);
		GetViewFrustumBounds(RightEyeBounds, RightEyeViewProjection, false);

		Frustum.Planes.Empty(5);
		Frustum.Planes.Add(LeftEyeBounds.Planes[0]);
		Frustum.Planes.Add(RightEyeBounds.Planes[1]);
		Frustum.Planes.Add(LeftEyeBounds.Planes[2]);
		Frustum.Planes.Add(LeftEyeBounds.Planes[3]);
		Frustum.Planes.Add(LeftEyeBounds.Planes[4]);
		Frustum.Init();
	}
	else
	{
		const FMatrix ViewProjection;
		GetViewFrustumBounds(Frustum, ViewProjection, false);
	}

	bool bIsPerspectiveProjection = false;
	if (bIsPerspectiveProjection)
	{
		if (Frustum.Planes.Num() == 5)
		{
			Frustum.Planes.Pop(false);

			FMatrix ThreePlanes;
			ThreePlanes.SetIdentity();
			ThreePlanes.SetAxes(&Frustum.Planes[0], &Frustum.Planes[1], &Frustum.Planes[2]);
			auto ProjectionOrigin = ThreePlanes.Inverse().GetTransposed().TransformVector(FVector(Frustum.Planes[0].W, Frustum.Planes[1].W, Frustum.Planes[2].W));
			for (auto i = 0; i < Frustum.Planes.Num(); i++)
			{
				auto Source = Frustum.Planes[i];
				auto Normal = Source.GetSafeNormal();
				Frustum.Planes[i] = FPlane(Normal, Normal | ProjectionOrigin);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
