#include "QuadTreeComponent.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"

#include "QuadTree.h"

UQuadTreeComponent::UQuadTreeComponent()
	: TickInterval(0.1f)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostPhysics;
	PrimaryComponentTick.TickInterval = TickInterval;

	TransformComponent = this;

	QuadTree = CreateDefaultSubobject<UQuadTree>(TEXT("QuadTree"));
	QuadTree->TransformComponent = TransformComponent;

	InstancedMesh = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("InstancedMesh"));
	InstancedMesh->SetupAttachment(this);
}

void UQuadTreeComponent::PostLoad()
{
	Super::PostLoad();

	if (TransformComponent == nullptr)
		TransformComponent = this;

	QuadTree->TransformComponent = TransformComponent;

	PrimaryComponentTick.TickInterval = TickInterval;
}

void UQuadTreeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TransformComponent == nullptr)
		return;

	TArray<TSharedPtr<FQuadTreeNode>> Selection;
	QuadTree->Update(Selection);

	if (Selection.Num() == 0)
		return;

	QuadTree->Draw(GetWorld(), Selection);

	if (QuadTree->bFloatingOrigin)
	{
		auto CurrentLocation = TransformComponent->GetComponentLocation();
		if (CurrentLocation != QuadTree->TargetOrigin)
			TransformComponent->SetWorldLocation(QuadTree->TargetOrigin);
	}

	ArrangeQuads(Selection);
}

#if WITH_EDITOR
void UQuadTreeComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UQuadTreeComponent, TransformComponent))
	{
		if (TransformComponent == nullptr)
			TransformComponent = this;

		QuadTree->TransformComponent = TransformComponent;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UQuadTreeComponent, TickInterval))
	{
		PrimaryComponentTick.TickInterval = TickInterval;
	}
}
#endif

void UQuadTreeComponent::ArrangeQuads(const TArray<TSharedPtr<FQuadTreeNode>>& Nodes)
{
	if (InstancedMesh == nullptr)
		return;

	if (InstancedMesh->GetStaticMesh() == nullptr && GridMesh != nullptr)
		InstancedMesh->SetStaticMesh(GridMesh);

	if (InstancedMesh->GetStaticMesh() == nullptr)
		return;

	// TODO: Don't reset, do delta update
	InstancedMesh->ClearInstances();

	for (auto& Node : Nodes)
	{
		auto NodeBounds = Node->GetBounds();
		FTransform InstanceTransform(FQuat::Identity, NodeBounds.GetCenter() + TransformComponent->GetComponentLocation(), NodeBounds.GetSize() / 100.0f);
		auto InstanceIndex = InstancedMesh->AddInstance(InstanceTransform);
	}
}
