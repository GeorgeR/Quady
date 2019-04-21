#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "QuadTreeNodeKey.h"

#include "QuadTreeTestActor.generated.h"

class UQuadTree;
class FQuadTreeNode;
class UHierarchicalInstancedStaticMeshComponent;
class UStaticMesh;

UCLASS(BlueprintType, Blueprintable)
class QUADY_API AQuadTreeTestActor 
    : public AActor
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "QuadTree", meta = (ShowOnlyInnerProperties))
    UQuadTree* QuadTree;

	UPROPERTY(EditAnywhere)
	UStaticMesh* GridMesh;

	UPROPERTY(BlueprintReadOnly)
	UHierarchicalInstancedStaticMeshComponent* InstancedMesh;

	AQuadTreeTestActor();

    virtual bool ShouldTickIfViewportsOnly() const override { return true; }

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	TMap<FQuadTreeNodeKey, int32> KeyToInstance;

	void ArrangeQuads(const TArray<TSharedPtr<FQuadTreeNode>>& Nodes);
};
