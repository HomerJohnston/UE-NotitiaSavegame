#pragma once

#include "CoreMinimal.h"
#include "NotitiaEditor/Core/NotitiaEditorModule.h"

class FNotitiaPropertyViewerNode;
class SNotitiaPropertyViewer;

class SNotitiaPropertyItem : public STableRow<TSharedPtr<FString>>
{
protected:
	TSharedPtr<SNotitiaPropertyViewer> PropertyViewer;
	
	TSharedPtr<FNotitiaPropertyViewerNode> AssociatedNode;
	
	TSharedPtr<SCheckBox> CheckBox;

	TSharedPtr<STextBlock> PropertyName;

public:
	SLATE_BEGIN_ARGS(SNotitiaPropertyItem) :
		_PropertyName(),
		_PropertyType(),
		_TextColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)),
		_DisplayClassHeader(),
		_DisplayClassFooter()
	{}
		SLATE_ARGUMENT(TSharedPtr<FString>, PropertyName)
		SLATE_ARGUMENT(TSharedPtr<FString>, PropertyType)
		SLATE_ARGUMENT(FSlateColor, TextColor)
		SLATE_ARGUMENT(TSharedPtr<FNotitiaPropertyViewerNode>, AssociatedNode)
		SLATE_ARGUMENT(TSharedPtr<SNotitiaPropertyViewer>, PropertyViewer)
		SLATE_ARGUMENT(bool, DisplayClassHeader)
		SLATE_ARGUMENT(bool, DisplayClassFooter)
	SLATE_END_ARGS()

	FNotitiaOnPropertyChanged OnPropertyChanged;
	
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

	FSlateColor GetTextColor() const;

	EVisibility ShowOptions() const;
	
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void Update();

	void OnCheckStateChanged(ECheckBoxState NewState);
};
