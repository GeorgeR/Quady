#pragma once

#include "CoreMinimal.h"
#include "ConvexVolume.h"

class QUADY_API FQuadTreeObserver
{
public:
	FQuadTreeObserver() = default;
	void SetOrigin(const FVector& InOrigin);

    const bool HasLocationChanged() const;
    bool HasLocationChanged(bool bClearFlag = false);
    const FVector GetLocation(bool bRelativeToOrigin = true) const;
    void SetLocation(const FVector& InLocation);

    const bool HasDirectionChanged() const;
    const bool HasDirectionChanged(bool bClearFlag = false);
    const FVector& GetDirection() const;
    void SetDirection(const FVector& InDirection);

	const FConvexVolume& GetFrustum() const { return Frustum; }

    uint16 GetLevelNum() const { return Ranges.Num(); }

    const FBoxSphereBounds& GetRange(const uint8& InLevel) const;
    void SetRanges(const TArray<float>& InRanges);

    void PostSelect();

    void Draw(const UWorld* InWorld);

private:
	FVector Origin;

    FVector Location;
    bool bLocationDirty;

    FVector Direction;
    bool bDirectionDirty;

	FConvexVolume Frustum;

    TArray<FBoxSphereBounds> Ranges;

	void UpdateFrustum();
};
