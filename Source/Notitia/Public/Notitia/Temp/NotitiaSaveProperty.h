#pragma once

#include "CoreMinimal.h"
#include "NotitiaSaveProperty.generated.h"

USTRUCT(BlueprintType)
struct NOTITIA_API FNotitiaSaveProperty
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	bool IncludeSaveGameProperties = false;

	UPROPERTY(EditAnywhere)
	bool IncludeCustomProperties = false;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition = "ProcessCustomProperties", EditConditionHides))
	TArray<FName> SavedProperties;
};
