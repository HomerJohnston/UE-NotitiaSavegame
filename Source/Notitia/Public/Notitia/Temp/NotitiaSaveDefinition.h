#pragma once

#include "CoreMinimal.h"
#include "NotitiaClassSettings.h"
#include "NotitiaSaveDefinition.generated.h"

UCLASS()
class NOTITIA_API UNotitiaSaveDefinition : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY(EditDefaultsOnly)
	TMap<TSoftClassPtr<UObject>, FNotitiaClassSettings> TestTestTest;
};
