#include "NotitiaEditor/Core/NotitiaEditorWindow.h"

#define LOCTEXT_NAMESPACE "NotitiaEditorModule"

TSharedRef<SDockTab> FNotitiaEditorWindow::Test()
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to overrihjkghjghjde this window's contents"),
		FText::FromString(TEXT("FNotitiaWindowModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("NotitiaWindow.cpp"))
		);
	
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(WidgetText)
			]
		];
}

#undef LOCTEXT_NAMESPACE