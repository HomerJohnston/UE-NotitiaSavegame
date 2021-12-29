#include "NotitiaEditor/Core/NotitiaEditorCommands.h"

#include "Framework/Commands/InputChord.h"

#define LOCTEXT_NAMESPACE "FNotitiaEditorModule"

FNotitiaEditorCommands::FNotitiaEditorCommands() : TCommands<FNotitiaEditorCommands>(TEXT("NotitiaEditor"), NSLOCTEXT("Contexts", "FNotitiaEditorModule", "Notitia Plugin"), NAME_None, FEditorStyle::GetStyleSetName())
{
	
}

void FNotitiaEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Notitia Editor", "Edit save properties for project classes", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE