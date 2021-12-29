

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "EditorStyleSet.h"

class FNotitiaEditorCommands : public TCommands<FNotitiaEditorCommands>
{
public:
	TSharedPtr<FUICommandInfo> OpenPluginWindow;

public:
	FNotitiaEditorCommands();

public:
	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> GetOpenPluginWindow() const { return OpenPluginWindow; }
};
