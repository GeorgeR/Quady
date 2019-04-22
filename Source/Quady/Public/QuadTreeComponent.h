#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "QuadTreeComponent.generated.h"

class UQuadTree;
class FQuadTreeNode;
class UHierarchicalInstancedStaticMeshComponent;
class UStaticMesh;

UCLASS(ClassGroup=(Quady), meta=(BlueprintSpawnableComponent))
class QUADY_API UQuadTreeComponent 
    : public USceneComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USceneComponent* TransformComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "QuadTree", meta = (ShowOnlyInnerProperties))
	UQuadTree* QuadTree;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QuadTree", meta = (UIMin = 0.0, ClampMin = 0.0))
	float TickInterval;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* GridMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UHierarchicalInstancedStaticMeshComponent* InstancedMesh;

	UQuadTreeComponent();

	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void ArrangeQuads(const TArray<TSharedPtr<FQuadTreeNode>>& Nodes);
};
