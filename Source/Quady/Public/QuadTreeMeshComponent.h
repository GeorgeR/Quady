#pragma once

#include "CoreMinimal.h"

#include "QuadTreeMeshComponent.generated.h"

//
//struct FQuadyVertexRef
//{
//public:
//    public int16 X;
//    public int16 Y;
//    public int16 QuadX;
//    public int16 QuadY;
//
//    FQuadyVertexRef(const int16 X, const int16 Y, const int16 QuadX, const int16 QuadY)
//        : X(X), Y(Y),
//        QuadX(QuadX), QuadY(QuadY) { }
//
//    uint64 MakeKey() const
//    {
//        return (uint64)X << 32 | (uint64)Y << 16 | (uint64)QuadX << 8 | (uint64)QuadY;
//    }
//};

// , Within = QuadTreeMeshProxy
UCLASS(HideCategories = (Display, Attachment, Physics, Debug, Collision, Movement, Rendering, PrimitiveComponent, Object, Transform, Mobility), ShowCategories = ("Rendering|Material"), MinimalAPI)
class UQuadTreeMeshComponent
    : public UPrimitiveComponent
{
    GENERATED_BODY()

public:

};