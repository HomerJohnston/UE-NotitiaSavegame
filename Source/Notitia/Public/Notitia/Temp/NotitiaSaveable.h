// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NotitiaSaveable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNotitiaSaveable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NOTITIA_API INotitiaSaveable
{
	GENERATED_BODY()

public:
	/** Perform any logic that is required to happen before saving, such as transferring specail save data into simple properties for Notitia to save to disk */
	void NotitiaPreSave();

	/** Perform any required cleanup logic after saving */
	void NotitiaPostSave();

	/** Perform any logic that is required before loading, such as resetting variables in preparation for Notitia to load data in */
	void NotitiaPreLoad();

	/** Perform any logic that is required after loading, such as unpacking special save data into runtime object propeties */
	void NotitiaPostLoad();
};
