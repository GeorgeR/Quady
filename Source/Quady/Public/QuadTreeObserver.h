#pragma once

#include "CoreMinimal.h"
#include "ConvexVolume.h"

class QUADY_API FQuadTreeObserver
{
public:
	void SetOrigin(const FVector& Origin);

    const bool HasLocationChanged() const;
    const bool HasLocationChanged(bool bClearFlag = false);
    const FVector GetLocation(bool bRelativeToOrigin = true) const;
    void SetLocation(const FVector& Location);

    const bool HasDirectionChanged() const;
    const bool HasDirectionChanged(bool bClearFlag = false);
    const FVector& GetDirection() const;
    void SetDirection(const FVector& Direction);

	const FConvexVolume& GetFrustum() const { return Frustum; }

	const uint16 GetLevelNum() const { return Ranges.Num(); }

    const FBoxSphereBounds& GetRange(const uint8& Level) const;
    void SetRanges(const TArray<float>& Ranges);

    void PostSelect();

    void Draw(const UWorld* World);

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
