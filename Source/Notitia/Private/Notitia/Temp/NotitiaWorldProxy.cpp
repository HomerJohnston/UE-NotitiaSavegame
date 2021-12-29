// Fill out your copyright notice in the Description page of Project Settings.


#include "Notitia/Temp/NotitiaWorldProxy.h"

#include "Notitia/Debug/NotitiaLogging.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values
ANotitiaWorldProxy::ANotitiaWorldProxy()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ANotitiaWorldProxy::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ANotitiaWorldProxy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ANotitiaWorldProxy::PreInitializeComponents()
{
	Super::PreInitializeComponents();

}

void ANotitiaWorldProxy::PostLoad()
{
	float WorldTime = GetWorld()->GetTimeSeconds();
	NOTITIA_LOG_DEBUG("Running PostLoad %f", WorldTime);
	//FWorldDelegates::OnWorldPreActorTick.AddUObject(this, &ThisClass::OnWorldPreActorTick);	
	Super::PostLoad();
}

void ANotitiaWorldProxy::PreSave(const ITargetPlatform* TargetPlatform)
{
	NOTITIA_LOG_DEBUG("Running Presave");
	Super::PreSave(TargetPlatform);
}

void ANotitiaWorldProxy::OnWorldPreActorTick(UWorld* World, ELevelTick Type, float DeltaSeconds)
{
	int64 FrameCount = UKismetSystemLibrary::GetFrameCount();
	float WorldTime = GetWorld()->GetTimeSeconds();
	NOTITIA_LOG_DEBUG("On World Pre Actor Tick: %f", WorldTime);
}
