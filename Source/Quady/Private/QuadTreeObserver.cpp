#include "QuadTreeObserver.h"

#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif
#include "NoExportTypes.h"

#define LOCTEXT_NAMESPACE "Quady"

void FQuadTreeObserver::SetOrigin(const FVector& Origin)
{
	if (!this->Origin.Equals(Origin))
	{
		this->Origin = Origin;
		this->bLocationDirty = true;

		for (auto& Range : Ranges)
			Range.Origin = GetLocation();
	}
}

const bool FQuadTreeObserver::HasLocationChanged(bool bClearFlag /*= false*/)
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

void FQuadTreeObserver::SetLocation(const FVector& Location)
{
    if (!this->Location.Equals(Location))
    {
        this->Location = Location;
        this->bLocationDirty = true;

        for (auto& Range : Ranges)
            Range.Origin = Location - Origin;
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

void FQuadTreeObserver::SetDirection(const FVector& Direction)
{
    if (!this->Direction.Equals(Direction))
    {
        this->Direction = Direction;
        this->bDirectionDirty = true;
    }
}

const FBoxSphereBounds& FQuadTreeObserver::GetRange(const uint8& Level) const
{
    check(Level < Ranges.Num());

    return Ranges[Level];
}

void FQuadTreeObserver::SetRanges(const TArray<float>& Ranges)
{
    this->Ranges.Empty(Ranges.Num());
    for (auto i = 0; i < Ranges.Num(); i++)
    {
        auto RangeSphere = FSphere(Location, Ranges[i]);
        auto Range = FBoxSphereBounds(RangeSphere);
        this->Ranges.Emplace(Range);
    }
}

void FQuadTreeObserver::PostSelect()
{
    bLocationDirty = false;
    bDirectionDirty = false;
}

void FQuadTreeObserver::Draw(const UWorld* World)
{
#if !UE_BUILD_SHIPPING
    static const FQuat Rotation = FQuat::MakeFromEuler(FVector(0, 90, 0));
    for (auto& Range : Ranges)
    {
        auto Transform = FTransform(Rotation, Range.Origin + Origin, FVector::OneVector);
        DrawDebugCircle(World, Transform.ToMatrixNoScale(), Range.SphereRadius, 64, FColor::Green);
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
