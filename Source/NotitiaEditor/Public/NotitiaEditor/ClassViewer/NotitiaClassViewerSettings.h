#pragma once

#include "CoreMinimal.h"
#include "NotitiaClassViewerSettings.generated.h"

UENUM()
enum class ENotitiaClassViewerDeveloperType : uint8
{
	CVDT_None,
	CVDT_CurrentUser,
	CVDT_All,
	CVDT_Max
};

DECLARE_EVENT_OneParam(UNotitiaClassViewerSettings, FSettingChangedEvent, FName)

UCLASS(config=EditorPerProjectUserSettings)
class NOTITIAEDITOR_API UNotitiaClassViewerSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Config)
	TArray<FString> AllowedClasses;

	ENotitiaClassViewerDeveloperType DeveloperFolderType;

protected:
	static FSettingChangedEvent SettingChangedEvent;
	
public:
	static FSettingChangedEvent& OnSettingChanged() { return SettingChangedEvent; }

protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};