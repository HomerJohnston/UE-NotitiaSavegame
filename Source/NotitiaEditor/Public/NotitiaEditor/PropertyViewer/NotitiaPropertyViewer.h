#pragma once

#include "CoreMinimal.h"
#include "NotitiaPropertyHierarchy.h"
#include "NotitiaEditor/Core/NotitiaEditorModule.h"

class FNotitiaPropertyViewerNode;

class SNotitiaPropertyViewer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNotitiaPropertyViewer)
	{
		
	}

	SLATE_END_ARGS()

	virtual ~SNotitiaPropertyViewer();

	FNotitiaOnPropertyChanged OnPropertyChangedEvt;
	
protected:

	UClass* SourceClass = nullptr;

	TArray<TSharedPtr<FNotitiaPropertyViewerNode>> RootTreeItems;

	TSharedPtr<STreeView<TSharedPtr<FNotitiaPropertyViewerNode>>> PropertyTree;
	
	TSharedPtr<SSearchBox> SearchBox;

	TMap<FString, bool> ExpansionStateMap;

	TSharedPtr<FNotitiaPropertyHierarchy> PropertyHierarchy;
	
	bool bNeedsRefresh = true;

	
public:
	void Construct(const FArguments& InArgs);
	
protected:
	void OnGetChildrenForPropertyViewer(TSharedPtr<FNotitiaPropertyViewerNode> InParent, TArray<TSharedPtr<FNotitiaPropertyViewerNode>>& OutChildren);

	void SetAllExpansionStates_Helper(TSharedPtr<FNotitiaPropertyViewerNode> InNode, bool bInExpansionState);

	TSharedRef<ITableRow> OnGenerateRowForPropertyViewer(TSharedPtr<FNotitiaPropertyViewerNode> Node, const TSharedRef<STableViewBase>& OwnerTable);

	void OnPropertyViewerSelectionChanged(TSharedPtr<FNotitiaPropertyViewerNode> Item, ESelectInfo::Type SelectInfo);

	void OnPropertyViewerExpansionChanged(TSharedPtr<FNotitiaPropertyViewerNode> Item, bool bInExpansionState);
	
	void OnFilterTextChanged(const FText& InText);

	void OnFilterTextCommitted(const FText& InText, ETextCommit::Type CommitInfo);

	void Tick(const FGeometry& AlottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void Populate();
	
public:
	void OnClassPicked(UClass* InClass);

	void OnPropertyChanged();
};
