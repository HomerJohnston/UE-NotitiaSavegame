// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NotitiaWorldProxy.generated.h"

UCLASS()
class NOTITIA_API ANotitiaWorldProxy : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANotitiaWorldProxy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void PreInitializeComponents() override;

	virtual void PostLoad() override;

	virtual void PreSave(const ITargetPlatform* TargetPlatform) override;
	
	void OnWorldPreActorTick(UWorld* World, ELevelTick Type, float DeltaSeconds);
};
