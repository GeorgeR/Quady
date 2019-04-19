#pragma once

#include "CoreMinimal.h"

enum class EQuadrant : uint8
{
	TopLeft = 0,
	TopRight = 1,
	BottomLeft = 2,
	BottomRight = 3,
	None = 4 // Root
};

struct FQuadTreeNodeKey;

struct FQuadTreeNodeKey
{
public:
	FQuadTreeNodeKey() : Key(0) { }

	/* Root */
	FQuadTreeNodeKey(const FVector& BottomLeft, const uint8 Level)
	{
		Key = GetTypeHash(BottomLeft) << 16 | (uint32)EQuadrant::None << 8 | (uint32)Level;
	}

	FQuadTreeNodeKey(const FQuadTreeNodeKey& Parent, const FVector& BottomLeft, const EQuadrant Quadrant, const uint8 Level)
	{
		Key = Parent.GetKey() | GetTypeHash(BottomLeft) << 16 | (uint32)Quadrant << 8 | (uint32)Level;
	}

	inline const uint32 GetKey() const { return Key; }
	inline const bool IsValid() const { return Key > 0; }

	bool operator==(const FQuadTreeNodeKey& Other) const { return Key == Other.Key; }
	bool operator!=(const FQuadTreeNodeKey& Other) const { return !operator==(Other); }
	friend uint32 GetTypeHash(const FQuadTreeNodeKey& Key) { return Key.GetKey(); }

private:
	uint32 Key;
};
