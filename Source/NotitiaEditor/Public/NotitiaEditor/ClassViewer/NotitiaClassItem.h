#pragma once

#include "CoreMinimal.h"

class FNotitiaClassViewerNode;

DECLARE_DELEGATE_OneParam( FOnClassItemDoubleClickDelegate, TSharedPtr<FNotitiaClassViewerNode> );

class SNotitiaClassItem : public STableRow<TSharedPtr<FString>>
{
protected:
	TSharedPtr<FString> ClassName;
	
	bool bIsInClassViewer = false;

	bool bDynamicClassLoading = false;
	
	TSharedPtr<FNotitiaClassViewerNode> AssociatedNode;

	FOnClassItemDoubleClickDelegate OnDoubleClicked;

	TSharedPtr<STextBlock> ItemText;

	TSharedPtr<STextBlock> ItemPropertyCountText;

	TSharedPtr<SCheckBox> CheckBox;
	
public:
	SLATE_BEGIN_ARGS(SNotitiaClassItem) :
		_ClassName(),
		_bIsPlaceable(false),
		_bIsInClassViewer(true),
		_bDynamicClassLoading(true),
		_HighlightText()
	{}
		SLATE_ARGUMENT(TSharedPtr<FString>, ClassName)
		SLATE_ARGUMENT(bool, bIsPlaceable)
		SLATE_ARGUMENT(bool, bIsInClassViewer)
		SLATE_ARGUMENT(bool, bDynamicClassLoading)
		SLATE_ARGUMENT(FText, HighlightText)
		SLATE_ARGUMENT(TSharedPtr<FNotitiaClassViewerNode>, AssociatedNode)
		SLATE_ARGUMENT(FOnClassItemDoubleClickDelegate, OnClassItemDoubleClicked)
		
	SLATE_END_ARGS()


	
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

	void OnCheckStateChanged(ECheckBoxState CheckBoxState);
	
	FSlateColor GetTextColor() const;
	
	EVisibility ShowOptions() const;

	TSharedRef<SWidget> GenerateDropDown();

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void Update();
};