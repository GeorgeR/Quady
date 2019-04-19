#pragma once

#include "CoreMinimal.h"

#include "QuadTreeNodeKey.h"

/* Flags */
enum class EQuadTreeNodeSelectionEventType : uint8
{
	None = 0,
	InRange = 1,
	OutOfRange = 2,
	InFrustum = 4,
	OutOfFrustum = 8,
	DueToParent = 16
};
ENUM_CLASS_FLAGS(EQuadTreeNodeSelectionEventType);

struct FQuadTreeNodeSelectionEvent
{
public:
	EQuadTreeNodeSelectionEventType Type;
	const FQuadTreeNodeKey Key;
	bool bIsSelected;

	FQuadTreeNodeSelectionEvent(const FQuadTreeNodeKey Key)
		: Type(EQuadTreeNodeSelectionEventType::None),
		Key(Key),
		bIsSelected(false) { }

	bool operator==(const FQuadTreeNodeSelectionEvent& Other) const { return Key == Other.Key; }
	bool operator!=(const FQuadTreeNodeSelectionEvent& Other) const { return !operator==(Other); }

	friend uint32 GetTypeHash(const FQuadTreeNodeSelectionEvent& Event) { return Event.Key.GetKey(); }
};
