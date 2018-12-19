#include "QuadTreeViewer.h"

#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif
#include "NoExportTypes.h"

#define LOCTEXT_NAMESPACE "Quady"

const bool FQuadTreeViewer::HasLocationChanged(bool bClearFlag /*= false*/)
{
    if (bClearFlag && bLocationDirty)
    {
        bLocationDirty = false;
        return true;
    }

    return bLocationDirty;
}

const bool FQuadTreeViewer::HasLocationChanged() const
{
    return bLocationDirty;
}

const FVector& FQuadTreeViewer::GetLocation() const
{
    return Location;
}

void FQuadTreeViewer::SetLocation(const FVector& Location)
{
    if (!this->Location.Equals(Location))
    {
        this->Location = Location;
        this->bLocationDirty = true;

        for (auto& Range : Ranges)
            Range.Origin = Location;
    }
}

const bool FQuadTreeViewer::HasDirectionChanged(bool bClearFlag /*= false*/)
{
    if (bClearFlag && bLocationDirty)
    {
        bDirectionDirty = false;
        return true;
    }

    return bDirectionDirty;
}

const bool FQuadTreeViewer::HasDirectionChanged() const
{
    return bDirectionDirty;
}

const FVector& FQuadTreeViewer::GetDirection() const
{
    return Direction;
}

void FQuadTreeViewer::SetDirection(const FVector& Direction)
{
    if (!this->Direction.Equals(Direction))
    {
        this->Direction = Direction;
        this->bDirectionDirty = true;
    }
}

const FBoxSphereBounds& FQuadTreeViewer::GetRange(const uint8& Level) const
{
    check(Level < Ranges.Num());

    return Ranges[Level];
}

void FQuadTreeViewer::SetRanges(const TArray<float>& Ranges)
{
    this->Ranges.Empty(Ranges.Num());
    for (auto i = 0; i < Ranges.Num(); i++)
    {
        auto RangeSphere = FSphere(Location, Ranges[i]);
        auto Range = FBoxSphereBounds(RangeSphere);
        this->Ranges.Emplace(Range);
    }
}

void FQuadTreeViewer::PostSelect()
{
    bLocationDirty = false;
    bDirectionDirty = false;
}

void FQuadTreeViewer::Draw(const UWorld* World)
{
#if !UE_BUILD_SHIPPING
    static const FQuat Rotation = FQuat::MakeFromEuler(FVector(0, 90, 0));
    for (auto& Range : Ranges)
    {
        auto Transform = FTransform(Rotation, Range.Origin, FVector::OneVector);
        DrawDebugCircle(World, Transform.ToMatrixNoScale(), Range.SphereRadius, 64, FColor::Green);
    }
#endif
}

#undef LOCTEXT_NAMESPACE