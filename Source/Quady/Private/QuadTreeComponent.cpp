#include "QuadTreeComponent.h"

#include "TimerManager.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"

#include "Quady.h"
#include "QuadTree.h"

#define LOCTEXT_NAMESPACE "Quady"

DECLARE_CYCLE_STAT(TEXT("QuadTree Arrange"), STAT_QuadTreeArrange, STATGROUP_Quady);

UQuadTreeComponent::UQuadTreeComponent()
	: UpdateInterval(0.1f)
{
	PrimaryComponentTick.bCanEverTick = true;

	TransformComponent = this;

	QuadTree = CreateDefaultSubobject<UQuadTree>(TEXT("QuadTree"));
	QuadTree->TransformComponent = TransformComponent;

	InstancedMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedMesh"));
	InstancedMesh->SetupAttachment(this);

	InstancedMesh->ClearInstances();
}

void UQuadTreeComponent::PostLoad()
{
	Super::PostLoad();

	if (TransformComponent == nullptr)
		TransformComponent = this;

	QuadTree->TransformComponent = TransformComponent;

	if(InstancedMesh != nullptr)
		InstancedMesh->ClearInstances();
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UQuadTreeComponent, UpdateInterval))
	{
		if (GetWorld() == nullptr)
			return;

		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		if (!TimerManager.TimerExists(UpdateHandle))
			StartUpdate();
	}
}
#endif

void UQuadTreeComponent::BeginPlay()
{
	Super::BeginPlay();

	StartUpdate();
}

void UQuadTreeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TransformComponent == nullptr)
		return;

#if WITH_EDITOR
	if (GetWorld() == nullptr)
		return;

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (!TimerManager.TimerExists(UpdateHandle))
		StartUpdate();
#endif

	QuadTree->Draw(GetWorld(), LastSelection);
}

void UQuadTreeComponent::StartUpdate()
{
	if (GetWorld() == nullptr)
		return;

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (TimerManager.TimerExists(UpdateHandle))
		TimerManager.ClearTimer(UpdateHandle);

	TimerManager.SetTimer(UpdateHandle, this, &UQuadTreeComponent::Update, UpdateInterval, true);
}

void UQuadTreeComponent::Update()
{
	if (QuadTree->bFloatingOrigin && TransformComponent == nullptr)
		return;

	TArray<TSharedPtr<FQuadTreeNode>> Selection;
	QuadTree->Update(Selection);

	LastSelection = Selection;

	if (Selection.Num() == 0)
		return;

	if (QuadTree->bFloatingOrigin)
	{
		auto CurrentLocation = TransformComponent->GetComponentLocation();
		if (CurrentLocation != QuadTree->TargetOrigin)
			TransformComponent->SetWorldLocation(QuadTree->TargetOrigin);
	}

	ArrangeQuads(Selection);
}

void UQuadTreeComponent::StopUpdate()
{
	if (GetWorld() == nullptr)
		return;

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (TimerManager.TimerExists(UpdateHandle))
		TimerManager.ClearTimer(UpdateHandle);
}

void UQuadTreeComponent::SetMaterial(UMaterialInterface* Value)
{
	Material = Value;

	if (InstancedMesh != nullptr)
		InstancedMesh->SetMaterial(0, Value);
}

void UQuadTreeComponent::ArrangeQuads(const TArray<TSharedPtr<FQuadTreeNode>>& Nodes)
{
	{
		SCOPE_CYCLE_COUNTER(STAT_QuadTreeArrange);

		if (InstancedMesh == nullptr)
			return;

		if (InstancedMesh->GetStaticMesh() == nullptr && GridMesh != nullptr)
		{
			InstancedMesh->ClearInstances();
			InstancedMesh->SetStaticMesh(GridMesh);
		}

		if (InstancedMesh->GetStaticMesh() == nullptr)
			return;

		// #todo Don't reset, do delta update
		InstancedMesh->ClearInstances();
	}

	for (auto& Node : Nodes)
	{
		auto NodeBounds = Node->GetBounds();
		FTransform InstanceTransform(FQuat::Identity, NodeBounds.GetCenter() + TransformComponent->GetComponentLocation(), NodeBounds.GetSize() / 100.0f);
		auto InstanceIndex = InstancedMesh->AddInstanceWorldSpace(InstanceTransform);
	}
}

#undef LOCTEXT_NAMESPACE
