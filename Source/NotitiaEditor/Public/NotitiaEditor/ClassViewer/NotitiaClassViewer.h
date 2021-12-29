#pragma once

#include "CoreMinimal.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerInitializationOptions.h"
#include "NotitiaEditor/Core/NotitiaEditorModule.h"

class FNotitiaClassHierarchy;
class FClassViewerNode;
class FNotitiaClassViewerFilter;
class FNotitiaClassViewerNode;

DECLARE_MULTICAST_DELEGATE( FPopulateClassViewer );

// TODO on hot reload, disable the entire panel

class SNotitiaClassViewer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNotitiaClassViewer) {}

	SLATE_ARGUMENT(FNotitiaOnClassPicked, OnClassPickedDelegate)

	SLATE_END_ARGS()

	virtual ~SNotitiaClassViewer();

public:
	// TODO move to a static namespace?
	static FPopulateClassViewer PopulateClassViewerDelegate;

public:
	FNotitiaOnClassPicked OnClassPicked;
	
private:
	FNotitiaClassViewerInitializationOptions InitOptions;

	TSharedPtr<FNotitiaClassViewerFilter> ClassViewerFilter;

	TArray<TSharedPtr<FNotitiaClassViewerNode>> RootTreeItems;

	bool bNeedsRefresh = true;

	int32 NumClasses = 0;

	bool bSaveExpansionStates = true;

	bool bPendingSetExpansionStates = false;
	
	bool bEnableClassDynamicLoading = false;

	TSharedPtr<SSearchBox> SearchBox;

	TMap<FString, bool> ExpansionStateMap;

	TSharedPtr<SComboButton> ViewOptionsComboButton;

	TSharedPtr<STreeView<TSharedPtr<FNotitiaClassViewerNode>>> ClassTree;

	static bool bPopulateClassHierarchy;
	
	bool bPendingFocusNextFrame = false;

	FName ParentClassPath;

	static TSharedPtr<FNotitiaClassHierarchy> ClassHierarchy;

	UClass* RightClickClass = nullptr;

	UBlueprint* RightClickBlueprint = nullptr;
	
public:
	void Construct(const FArguments& InArgs, const FNotitiaClassViewerInitializationOptions& InInitOptions);

	TSharedPtr<SWidget> BuildMenuWidget();

	static TSharedRef<SWidget> CreateMenu(UClass* Class, const bool bIsBlueprint, const bool bHasBlueprint);
	
	static void GetClassInfo(TWeakObjectPtr<UClass> InClass, bool& bInOutIsBlueprintBase, bool& bInOutHasBlueprint);
	
	static bool CanCreateBlueprintOfClass_IgnoreDepreciation(UClass* InClass);

	FText GetClassCountText() const;

	const int GetNumItems() const;
	
	static void ConstructClassHierarchy();
	
	void HandleSettingChanged(FName PropertyName);
	
	TSharedRef<SWidget> FillFilterEntries();

	bool Menu_CanExecute();
	
	void MenuActorsOnly_Execute();

	bool MenuActorsOnly_IsChecked();

	void MenuPlaceableOnly_Execute();

	bool MenuPlaceableOnly_IsChecked();

	void MenuBlueprintBasesOnly_Execute();

	bool MenuBlueprintBasesOnly_IsChecked();
	
	TSharedRef<SWidget> GetFiltersWidget();

	void OnFilterTextChanged(const FText& InText);

	void OnFilterTextCommitted(const FText& InText, ETextCommit::Type CommitInfo);

	TSharedRef<ITableRow> OnGenerateRowForClassViewer(TSharedPtr<FNotitiaClassViewerNode> Item, const TSharedRef<STableViewBase>& OwnerTable);

	void ToggleExpansionState_Helper(TSharedPtr<FNotitiaClassViewerNode> InNode);
	
	void OnClassViewerSelectionChanged(TSharedPtr<FNotitiaClassViewerNode> Item, ESelectInfo::Type SelectInfo);

	static void LoadViewedClass(TSharedPtr<FNotitiaClassViewerNode> InOutClassNode);

	static void UpdateClassInNode(FName InGeneratedClassPath, UClass* InNewClass, UBlueprint* InNewBluePrint);
	
	void OnGetChildrenForClassViewerTree(TSharedPtr<FNotitiaClassViewerNode> InParent, TArray<TSharedPtr<FNotitiaClassViewerNode>>& OutChildren);

	void SetAllExpansionStates_Helper(TSharedPtr<FNotitiaClassViewerNode> InNode, bool bInExpansionState);
	
	void OnClassViewerExpansionChanged(TSharedPtr<FNotitiaClassViewerNode> Item, bool bExpanded);
	
	virtual TSharedRef<SWidget> GetContent();

	virtual void ClearSelection();

	virtual void Tick(const FGeometry& AlottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	static void PopulateClassHierarchy();

	void Populate();

	void MapExpansionStatesInTree(TSharedPtr<FNotitiaClassViewerNode> InItem);

	static void GetClassTree(TSharedPtr<FNotitiaClassViewerNode>& InOutRootNode, const TSharedPtr<FNotitiaClassViewerFilter>& InClassFilter, const FNotitiaClassViewerInitializationOptions& InInitOptions);

	static bool AddChildren_Tree(TSharedPtr<FNotitiaClassViewerNode>& InOutRootNode, const TSharedPtr<FNotitiaClassViewerNode>& InOriginalRootNode, const TSharedPtr<FNotitiaClassViewerFilter>& InClassFilter, const FNotitiaClassViewerInitializationOptions& InInitOptions);

	void SetExpansionStatesInTree(TSharedPtr<FNotitiaClassViewerNode> InItem);

	bool ExpandFilteredInNodes(TSharedPtr<FNotitiaClassViewerNode> InNode);

	int32 CountTreeItems(FNotitiaClassViewerNode* Node);

	TSharedPtr<FNotitiaClassViewerNode> FindNodeByGeneratedClassPath(const TSharedPtr<FNotitiaClassViewerNode>& InRootNode, FName InGeneratedClassPath);
	
	virtual void ExpandRootNodes();

	const TArray<TSharedPtr<FNotitiaClassViewerNode>> GetSelectedItems() const;
	
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;

	virtual bool SupportsKeyboardFocus() const override;

	virtual bool IsClassAllowed(const UClass* InClass) const;

	NOTITIAEDITOR_API void Refresh();

	static void DestroyClassHierarchy();

	void OnPropertyChanged();
	
private:
	
};
