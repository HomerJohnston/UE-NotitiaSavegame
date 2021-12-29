#include "NotitiaEditor/ClassViewer/NotitiaClassViewerSettings.h"

FSettingChangedEvent UNotitiaClassViewerSettings::SettingChangedEvent;

void UNotitiaClassViewerSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName Name = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if  (!FUnrealEdMisc::Get().IsDeletePreferences())
	{
		SaveConfig();
	}

	SettingChangedEvent.Broadcast(Name);
}
 