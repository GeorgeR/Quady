#pragma

#include "CoreMinimal.h"

class QUADY_API FQuadTreeViewer
{
public:
    const bool HasLocationChanged() const;
    const bool HasLocationChanged(bool bClearFlag = false);
    const FVector& GetLocation() const;
    void SetLocation(const FVector& Location);

    const bool HasDirectionChanged() const;
    const bool HasDirectionChanged(bool bClearFlag = false);
    const FVector& GetDirection() const;
    void SetDirection(const FVector& Direction);

    const FBoxSphereBounds& GetRange(const uint8& Level) const;
    void SetRanges(const TArray<float>& Ranges);

    void PostSelect();

    void Draw(const UWorld* World);

private:
    FVector Location;
    bool bLocationDirty;

    FVector Direction;
    bool bDirectionDirty;

    TArray<FBoxSphereBounds> Ranges;
};