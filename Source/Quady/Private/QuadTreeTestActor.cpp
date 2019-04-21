#include "QuadTreeTestActor.h"
#include "QuadTree.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"

AQuadTreeTestActor::AQuadTreeTestActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_PrePhysics;
    
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    QuadTree = CreateDefaultSubobject<UQuadTree>(TEXT("QuadTree"));
	InstancedMesh = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("InstancedMesh"));
}

void AQuadTreeTestActor::BeginPlay()
{
	Super::BeginPlay();
}

void AQuadTreeTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TArray<TSharedPtr<FQuadTreeNode>> Selection;
	QuadTree->Update(Selection);

	if (Selection.Num() == 0)
		return;
	
    QuadTree->Draw(GetWorld(), Selection);

	if (QuadTree->bFloatingOrigin)
	{
		auto CurrentLocation = GetActorLocation();
		if (CurrentLocation != QuadTree->TargetOrigin)
			SetActorLocation(QuadTree->TargetOrigin, false, nullptr, ETeleportType::TeleportPhysics);
	}

	ArrangeQuads(Selection);
}

void AQuadTreeTestActor::ArrangeQuads(const TArray<TSharedPtr<FQuadTreeNode>>& Nodes)
{
	if (InstancedMesh == nullptr)
		return;

	if (InstancedMesh->GetStaticMesh() == nullptr && GridMesh != nullptr)
	{
		InstancedMesh->SetStaticMesh(GridMesh);
	}

	if (InstancedMesh->GetStaticMesh() == nullptr)
		return;

	auto Rotation = FQuat::Identity;

	InstancedMesh->ClearInstances();

	for (auto& Node : Nodes)
	{
		auto Bounds = Node->GetBounds();
		FTransform InstanceTransform(Rotation, Bounds.GetCenter() + GetActorLocation(), Bounds.GetSize() / 100.0f);
		auto InstanceIndex = InstancedMesh->AddInstance(InstanceTransform);
		KeyToInstance.Add(Node->GetKey(), InstanceIndex);
	}

	return;

	// Add new
	TArray<TSharedPtr<FQuadTreeNode>> NodesToAdd;	
	for (auto& Node : Nodes)
	{
		if (!KeyToInstance.Contains(Node->GetKey()))
			NodesToAdd.Add(Node);
	}

	for (auto& Node : NodesToAdd)
	{
		auto Bounds = Node->GetBounds();
		FTransform InstanceTransform(Rotation, Bounds.GetCenter() + GetActorLocation(), Bounds.GetSize() / 100.0f);
		auto InstanceIndex = InstancedMesh->AddInstance(InstanceTransform);
		KeyToInstance.Add(Node->GetKey(), InstanceIndex);
	}

	// Remove old
	TArray<FQuadTreeNodeKey> KeysToRemove;
	TArray<int32> NodesToRemove;
	for (auto& KVP : KeyToInstance)
	{
		if (!Nodes.ContainsByPredicate([&](TSharedPtr<FQuadTreeNode> Node) { return Node->GetKey() == KVP.Key; }))
		{
			NodesToRemove.Add(KVP.Value);
			KeysToRemove.Add(KVP.Key);
		}
	}

	

	for (auto& Key : KeysToRemove)
		KeyToInstance.Remove(Key);

	if(NodesToRemove.Num() > 0)
		InstancedMesh->RemoveInstances(NodesToRemove);
}
