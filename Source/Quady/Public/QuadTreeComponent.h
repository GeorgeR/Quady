#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "QuadTreeComponent.generated.h"

class UQuadTree;
class FQuadTreeNode;
class UHierarchicalInstancedStaticMeshComponent;
class UInstancedStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;
class UBlueprint;

UCLASS(ClassGroup=(Quady), meta=(BlueprintSpawnableComponent))
class QUADY_API UQuadTreeComponent 
    : public USceneComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintSetter = SetMaterial)
	UMaterialInterface* Material;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USceneComponent* TransformComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "QuadTree", meta = (ShowOnlyInnerProperties))
	UQuadTree* QuadTree;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QuadTree", meta = (UIMin = 0.0, ClampMin = 0.0))
	float UpdateInterval;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* GridMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UInstancedStaticMeshComponent* InstancedMesh;

	UQuadTreeComponent();

	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "QuadTree")
	virtual void SetMaterial(UPARAM(DisplayName = "Material") UMaterialInterface* Value);

protected:
	FTimerHandle UpdateHandle;
	TArray<TSharedPtr<FQuadTreeNode>> LastSelection;

	virtual void StartUpdate();
	virtual void Update();
	virtual void StopUpdate();

	virtual void ArrangeQuads(const TArray<TSharedPtr<FQuadTreeNode>>& Nodes);
};
