#include "QuadTreeTestActor.h"
#include "QuadTree.h"

AQuadTreeTestActor::AQuadTreeTestActor()
{
	PrimaryActorTick.bCanEverTick = true;
    
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    QuadTree = CreateDefaultSubobject<UQuadTree>(TEXT("QuadTree"));
}

void AQuadTreeTestActor::BeginPlay()
{
	Super::BeginPlay();
}

void AQuadTreeTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
    QuadTree->Update();
    QuadTree->Draw(this);
}