#include "NotitiaEditor/ClassViewer/NotitiaClassViewer.h"

#include "AssetSelection.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassHierarchy.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassItem.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerFilter.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerNode.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerNodeNameLess.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerNameTypeToDisplay.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerSettings.h"
#include "SlateOptMacros.h"
#include "SListViewSelectorDropdownMenu.h"
#include "Logging/MessageLog.h"
#include "Misc/FeedbackContext.h"
#include "Widgets/Input/SSearchBox.h"
#include "EditorWidgets/Public/EditorWidgetsModule.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScrollBorder.h"
#include "Widgets/Layout/SSeparator.h"

bool SNotitiaClassViewer::bPopulateClassHierarchy;
TSharedPtr<FNotitiaClassHierarchy> SNotitiaClassViewer::ClassHierarchy;
FPopulateClassViewer SNotitiaClassViewer::PopulateClassViewerDelegate;

SNotitiaClassViewer::~SNotitiaClassViewer()
{
	PopulateClassViewerDelegate.RemoveAll(this);
	UNotitiaClassViewerSettings::OnSettingChanged().RemoveAll(this);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SNotitiaClassViewer::Construct(const FArguments& InArgs, const FNotitiaClassViewerInitializationOptions& InInitOptions)
{
	bPopulateClassHierarchy = true;
	
	bNeedsRefresh = true;
	
	NumClasses = 0;

	// TODO
	UNotitiaClassViewerSettings::OnSettingChanged().AddSP(this, &SNotitiaClassViewer::HandleSettingChanged);

	InitOptions = InInitOptions;

	OnClassPicked = InArgs._OnClassPickedDelegate;

	bSaveExpansionStates = true;

	bPendingSetExpansionStates = false;

	ClassViewerFilter = MakeShareable(new FNotitiaClassViewerFilter(InitOptions));

	bEnableClassDynamicLoading = InitOptions.bEnableClassDynamicLoading;

	TSharedRef<SWidget> FiltersWidget = GetFiltersWidget();

	FEditorWidgetsModule& EditorWidgetsModule = FModuleManager::LoadModuleChecked<FEditorWidgetsModule>("EditorWidgets");

	TSharedRef<SWidget> AssetDiscoveryIndicator = EditorWidgetsModule.CreateAssetDiscoveryIndicator(EAssetDiscoveryIndicatorScaleMode::Scale_Vertical);

	FOnContextMenuOpening OnContextMenuOpening = FOnContextMenuOpening::CreateSP(this, &SNotitiaClassViewer::BuildMenuWidget);

	SAssignNew(ClassTree, STreeView<TSharedPtr<FNotitiaClassViewerNode>>)
		.SelectionMode(ESelectionMode::Single)
		.TreeItemsSource(&RootTreeItems)
		.OnGetChildren(this, &SNotitiaClassViewer::OnGetChildrenForClassViewerTree)
		.OnSetExpansionRecursive(this, &SNotitiaClassViewer::SetAllExpansionStates_Helper)
		.OnGenerateRow(this, &SNotitiaClassViewer::OnGenerateRowForClassViewer)
		.OnContextMenuOpening(OnContextMenuOpening)
		.OnSelectionChanged(this, &SNotitiaClassViewer::OnClassViewerSelectionChanged)
		.OnExpansionChanged(this, &SNotitiaClassViewer::OnClassViewerExpansionChanged)
		.ItemHeight(20.0f);

	TSharedRef<STreeView<TSharedPtr<FNotitiaClassViewerNode>>> ClassTreeView = ClassTree.ToSharedRef();

	TSharedPtr<SWidget> ClassViewerContent;

	ClassViewerContent = SNew(SBox)
		.MaxDesiredHeight(800.0f)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 1.0f)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.Padding(2.0f, 2.0f)
					[
						SAssignNew(SearchBox, SSearchBox)
						.OnTextChanged(this, &SNotitiaClassViewer::OnFilterTextChanged)
						.OnTextCommitted(this, &SNotitiaClassViewer::OnFilterTextCommitted)
					]
					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2.0f, 2.0f)
					[
						FiltersWidget
					]
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 2.0f)
				[
					SNew(SCheckBox)
					//.IsChecked(this, &SProjectLauncherDeployToDeviceSettings::HandleIncrementalCheckBoxIsChecked)
					//.OnCheckStateChanged(this, &SProjectLauncherDeployToDeviceSettings::HandleIncrementalCheckBoxCheckStateChanged)
					.Padding(FMargin(4.0f, 0.0f))
					.ToolTipText(INVTEXT("If checked, only modified content will be deployed, resulting in much faster deploy times. It is recommended to enable this option whenever possible."))
					.Content()
					[
						SNew(STextBlock)
						.Text(INVTEXT("Show classes which have selected properties"))
					]
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 2.0f)
				[
					SNew(SCheckBox)
					//.IsChecked(this, &SProjectLauncherDeployToDeviceSettings::HandleIncrementalCheckBoxIsChecked)
					//.OnCheckStateChanged(this, &SProjectLauncherDeployToDeviceSettings::HandleIncrementalCheckBoxCheckStateChanged)
					.Padding(FMargin(4.0f, 0.0f))
					.ToolTipText(INVTEXT("If checked, only modified content will be deployed, resulting in much faster deploy times. It is recommended to enable this option whenever possible."))
					.Content()
					[
						SNew(STextBlock)
						.Text(INVTEXT("Show classes which do not have any selected properties"))
					]
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 1.0f)
			[
				SNew(SSeparator)
			]
			+SVerticalBox::Slot()
			[
					SNew(SOverlay)
					+SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.FillHeight(1.0f)
						[
							SNew(SScrollBorder, ClassTreeView)
							[
								ClassTreeView
							]
						]
					]
					+SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Bottom)
					.Padding(FMargin(24, 0, 24, 0))
					[
						AssetDiscoveryIndicator
					]
			]
		];

	if (ViewOptionsComboButton.IsValid())
	{
		ViewOptionsComboButton->SetVisibility(InitOptions.bAllowViewOptions ? EVisibility::Visible : EVisibility::Collapsed);
	}

	this->ChildSlot
	[
		ClassViewerContent.ToSharedRef()
	];

	ConstructClassHierarchy();

	MenuPlaceableOnly_Execute();

	PopulateClassViewerDelegate.AddSP(this, &SNotitiaClassViewer::Refresh);

	bPendingFocusNextFrame = true;
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedPtr<SWidget> SNotitiaClassViewer::BuildMenuWidget()
{
	bool bIsBlueprint;
	bool bHasBlueprint;

	TArray<TSharedPtr<FNotitiaClassViewerNode>> SelectedList;

	SelectedList = ClassTree->GetSelectedItems();

	if (SelectedList.Num() == 0)
	{
		return SNullWidget::NullWidget;
	}

	if (bEnableClassDynamicLoading && !SelectedList[0]->GetClassWeakPtr().IsStale() && !SelectedList[0]->GetClassWeakPtr().IsValid() && SelectedList[0]->UnloadedBlueprintData.IsValid())
	{
		LoadViewedClass(SelectedList[0]);

		Refresh();
	}

	RightClickClass = SelectedList[0]->GetClassWeakPtr().Get();

	RightClickBlueprint = SelectedList[0]->Blueprint.Get();

	GetClassInfo(RightClickClass, bIsBlueprint, bHasBlueprint);

	if (RightClickBlueprint)
	{
		bHasBlueprint = true;
	}

	return CreateMenu(RightClickClass, bIsBlueprint, bHasBlueprint);
}

TSharedRef<SWidget> SNotitiaClassViewer::CreateMenu(UClass* Class, const bool bIsBlueprint, const bool bHasBlueprint)
{
	TSharedPtr<FUICommandList> Commands;

	const bool bShouldCloseWindowAfterMenuSelection = true;

	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, Commands);

	{
		if (bIsBlueprint)
		{
			//TAttribute<FText>::Getter DynamicTooltipGetter;
			
		}
	}

	// TODO
	return MenuBuilder.MakeWidget();
}

void SNotitiaClassViewer::GetClassInfo(TWeakObjectPtr<UClass> InClass, bool& bInOutIsBlueprintBase, bool& bInOutHasBlueprint)
{
	if (UClass* Class = InClass.Get())
	{
		bInOutIsBlueprintBase = CanCreateBlueprintOfClass_IgnoreDepreciation(Class);
		bInOutHasBlueprint = Class->ClassGeneratedBy != nullptr;
	}
	else
	{
		bInOutIsBlueprintBase = false;
		bInOutHasBlueprint = false;
	}
}

bool SNotitiaClassViewer::CanCreateBlueprintOfClass_IgnoreDepreciation(UClass* InClass)
{
	bool bIsClassDeprecated = InClass->HasAnyClassFlags(CLASS_Deprecated);

	InClass->ClassFlags &= ~CLASS_Deprecated;

	bool bCanCreateBlueprintsOfClass = FKismetEditorUtilities::CanCreateBlueprintOfClass(InClass);

	if (bIsClassDeprecated)
	{
		InClass->ClassFlags |= CLASS_Deprecated;
	}

	return bCanCreateBlueprintsOfClass;
}

FText SNotitiaClassViewer::GetClassCountText() const
{
	const int32 NumAssets = GetNumItems();

	const int32 NumSelectedAssets = GetSelectedItems().Num();
	
	// TODO
	FText AssetCount;

	if (NumSelectedAssets == 0)
	{
		if (NumAssets == 1)
		{
			AssetCount = INVTEXT("1 item");
		}
		else
		{
			AssetCount = FText::Format(INVTEXT("{0} items"), FText::AsNumber(NumAssets));
		}
	}
	else
	{
		if (NumAssets == 1)
		{
			AssetCount = FText::Format(INVTEXT("1 item ({0} selected)"), FText::AsNumber(NumSelectedAssets));
		}
		else
		{
			AssetCount = FText::Format(INVTEXT("{0} items ({1} selected)"), FText::AsNumber(NumAssets), FText::AsNumber(NumSelectedAssets));
		}
	}

	return AssetCount;
}

const int SNotitiaClassViewer::GetNumItems() const
{
	return NumClasses;
}

void SNotitiaClassViewer::ConstructClassHierarchy()
{
	if (!ClassHierarchy.IsValid())
	{
		ClassHierarchy = MakeShareable(new FNotitiaClassHierarchy);

		// TODO
		//GWarn->BeginSlowTask
		ClassHierarchy->PopulateClassHierarchy();
		//GWarn->EndSlowTask
	}
	else
	{
		ClassHierarchy->RequestPopulateClassHierarchy();
	}
}


void SNotitiaClassViewer::HandleSettingChanged(FName PropertyName)
{
	if (PropertyName == "DisplayNameInternalClasses" || PropertyName == "DeveloperFolderType" || PropertyName == NAME_None)
	{
		Refresh();
	}
}


TSharedRef<SWidget> SNotitiaClassViewer::FillFilterEntries()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection("NotitiaClassViewerFilterEntries");
	{
		// TODO Localization
		MenuBuilder.AddMenuEntry(
			INVTEXT("Actors Only"),
			INVTEXT("Filter the Class Viewer to show only actors"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateRaw(this, &SNotitiaClassViewer::MenuActorsOnly_Execute),
				FCanExecuteAction::CreateRaw(this, &SNotitiaClassViewer::Menu_CanExecute),
				FIsActionChecked::CreateRaw(this, &SNotitiaClassViewer::MenuActorsOnly_IsChecked)),
			NAME_None,
			EUserInterfaceActionType::Check);
		MenuBuilder.AddMenuEntry(
			INVTEXT("Placeable Only"),
			INVTEXT("Filter the Class Viewer to show only placeable actors."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateRaw(this, &SNotitiaClassViewer::MenuPlaceableOnly_Execute),
				FCanExecuteAction::CreateRaw(this, &SNotitiaClassViewer::Menu_CanExecute),
				FIsActionChecked::CreateRaw(this, &SNotitiaClassViewer::MenuPlaceableOnly_IsChecked)),
			NAME_None,
			EUserInterfaceActionType::Check);
	}
	MenuBuilder.EndSection();
	
	MenuBuilder.BeginSection("NotitiaClassViewerFilterEntries2");
	{
		MenuBuilder.AddMenuEntry(
			INVTEXT("Blueprint Class Bases Only"),
			INVTEXT("Filter the Class Viewer to show only base blueprint classes."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateRaw(this, &SNotitiaClassViewer::MenuBlueprintBasesOnly_Execute),
				FCanExecuteAction::CreateRaw(this, &SNotitiaClassViewer::Menu_CanExecute),
				FIsActionChecked::CreateRaw(this, &SNotitiaClassViewer::MenuBlueprintBasesOnly_IsChecked)),
			NAME_None,
			EUserInterfaceActionType::Check);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

bool SNotitiaClassViewer::Menu_CanExecute()
{
	return true;
}

void SNotitiaClassViewer::MenuActorsOnly_Execute()
{
	InitOptions.bIsActorsOnly = !InitOptions.bIsActorsOnly;

	if (!InitOptions.bIsActorsOnly)
	{
		InitOptions.bIsActorsOnly = false;
	}

	Refresh();
}

bool SNotitiaClassViewer::MenuActorsOnly_IsChecked()
{
	return InitOptions.bIsActorsOnly;
}

void SNotitiaClassViewer::MenuPlaceableOnly_Execute()
{
	InitOptions.bIsPlaceableOnly = !InitOptions.bIsPlaceableOnly;

	if (InitOptions.bIsPlaceableOnly)
	{
		InitOptions.bIsActorsOnly = true;
	}

	Refresh();
}

bool SNotitiaClassViewer::MenuPlaceableOnly_IsChecked()
{
	return InitOptions.bIsPlaceableOnly;
}

void SNotitiaClassViewer::MenuBlueprintBasesOnly_Execute()
{
	InitOptions.bIsBlueprintBaseOnly = !InitOptions.bIsBlueprintBaseOnly;

	Refresh();
}

bool SNotitiaClassViewer::MenuBlueprintBasesOnly_IsChecked()
{
	return InitOptions.bIsBlueprintBaseOnly;
}

TSharedRef<SWidget> SNotitiaClassViewer::GetFiltersWidget()
{
	TSharedRef<SWidget> FiltersWidget = SNew(SComboButton)
	.ComboButtonStyle(FEditorStyle::Get(), "GenericFilters.ComboButtonStyle")
	.ForegroundColor(FLinearColor::White)
	.ContentPadding(0)
	// TODO Localization
	//.ToolTipText()
	.OnGetMenuContent(this, &SNotitiaClassViewer::FillFilterEntries)
	.HasDownArrow(true)
	.ContentPadding(FMargin(1,0))
	.ButtonContent()
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.TextStyle(FEditorStyle::Get(), "GenericFilters.TextStyle")
			.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.9"))
			.Text(FText::FromString(FString(TEXT("\xf0b0"))) /*fa-filter*/)
		]
		+SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2,0,0,0)
		[
			SNew(STextBlock)
			.TextStyle(FEditorStyle::Get(), "GenericFilters.TextStyle")
			// TODO Localization
			.Text(INVTEXT("Filter"))
		]
	];

	return FiltersWidget;
}

void SNotitiaClassViewer::OnFilterTextChanged(const FText& InText)
{
	ClassViewerFilter->TextFilter->SetFilterText(InText);

	SearchBox->SetError(ClassViewerFilter->TextFilter->GetFilterErrorText());

	Refresh();
}

void SNotitiaClassViewer::OnFilterTextCommitted(const FText& InText, ETextCommit::Type CommitInfo)
{
	if (CommitInfo != ETextCommit::OnEnter)
	{
		return;
	}

	//TArray<TSharedPtr<FClassl
}

TSharedRef<ITableRow> SNotitiaClassViewer::OnGenerateRowForClassViewer(TSharedPtr<FNotitiaClassViewerNode> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	float AlphaValue = Item->bPassesFilter ? 1.0f :  0.5f;

	FText SearchBoxTextForHighlight = SearchBox->GetText();

	TSharedPtr<FString> ClassNameDisplay = Item->GetClassName(InitOptions.NameTypeToDisplay);

	if (!SearchBoxTextForHighlight.IsEmpty() && (ClassNameDisplay->Find(*SearchBoxTextForHighlight.ToString()) == INDEX_NONE))
	{
		SearchBoxTextForHighlight = FText::FromString(UObjectBase::RemoveClassPrefix(*SearchBoxTextForHighlight.ToString()));
	}

	FLinearColor Color = FLinearColor::White;

	//= Item->IsClassPlaceable() ? FLinearColor(0.2f, 0.4f, 0.6f, AlphaValue) : FLinearColor(1.0f, 1.0f, 1.0f, AlphaValue);

	//UE_LOG(LogTemp, Warning, TEXT("OnGenRow for %s, prop count: %i"), *Item->GetClassName().Get(), Item->GetNumProperties());
	
	TSharedRef<SNotitiaClassItem> ReturnRow = SNew(SNotitiaClassItem, OwnerTable)
		.ClassName(ClassNameDisplay)
		// TODO
		//.bIsPlaceable(...)
		.HighlightText(SearchBoxTextForHighlight)
		.AssociatedNode(Item)
		// TODO
		//.bIsInClassViewer()
		.bDynamicClassLoading(bEnableClassDynamicLoading)
		.OnClassItemDoubleClicked(FOnClassItemDoubleClickDelegate::CreateSP(this, &SNotitiaClassViewer::ToggleExpansionState_Helper));

	if (!bPendingSetExpansionStates)
	{
		bool* bIsExpanded = ExpansionStateMap.Find(*(Item->GetClassName()));

		if (bIsExpanded && *bIsExpanded)
		{
			bPendingSetExpansionStates = true;
		}
	}

	return ReturnRow;
}

void SNotitiaClassViewer::ToggleExpansionState_Helper(TSharedPtr<FNotitiaClassViewerNode> InNode)
{
	bool bExpanded = ClassTree->IsItemExpanded(InNode);

	ClassTree->SetItemExpansion(InNode, !bExpanded);
}

void SNotitiaClassViewer::OnClassViewerSelectionChanged(TSharedPtr<FNotitiaClassViewerNode> Item, ESelectInfo::Type SelectInfo)
{
	if (!Item.IsValid() || Item->IsRestricted())
	{
		return;
	}

	UClass* Class = Item->GetClassWeakPtr().Get();

	if (bEnableClassDynamicLoading && !Class && Item->UnloadedBlueprintData.IsValid())
	{
		LoadViewedClass(Item);
	}

	if (Item->GetClassWeakPtr().IsValid() || !Class)
	{
		if (Item->bPassesFilterRegardlessTextFilter || Item->bPassesFilter)
		{
			OnClassPicked.ExecuteIfBound(Item->GetClassWeakPtr().Get());
		}
		else
		{
			OnClassPicked.ExecuteIfBound(nullptr);
		}
	}
}

void SNotitiaClassViewer::LoadViewedClass(TSharedPtr<FNotitiaClassViewerNode> InOutClassNode)
{
	// TODO GWarn->BeginSlowTanks
	UClass* Class = LoadObject<UClass>(nullptr, *InOutClassNode->ClassPath.ToString());

	if (Class)
	{
		InOutClassNode->Blueprint = Cast<UBlueprint>(Class->ClassGeneratedBy);
		InOutClassNode->SetClass(Class);

		UpdateClassInNode(InOutClassNode->ClassPath, InOutClassNode->GetClassWeakPtr().Get(), InOutClassNode->Blueprint.Get());
	}
	else
	{
		FMessageLog EditorErrors("EditorErrors");
		FFormatNamedArguments Arguments;
		Arguments.Add(TEXT("ObjectName"), FText::FromName(InOutClassNode->ClassPath));
		// TODO EditorErrors.Error(FText::Format(LOCTEXT("PackageLoadFail", "Failed to load class {ObjectName}"), Arguments));
	}
}

void SNotitiaClassViewer::UpdateClassInNode(FName InGeneratedClassPath, UClass* InNewClass, UBlueprint* InNewBluePrint)
{
	ClassHierarchy->UpdateClassInNode(InGeneratedClassPath, InNewClass, InNewBluePrint);
}

void SNotitiaClassViewer::OnGetChildrenForClassViewerTree(TSharedPtr<FNotitiaClassViewerNode> InParent, TArray<TSharedPtr<FNotitiaClassViewerNode>>& OutChildren)
{
	OutChildren = InParent->GetChildrenList();
}

void SNotitiaClassViewer::SetAllExpansionStates_Helper(TSharedPtr<FNotitiaClassViewerNode> InNode, bool bInExpansionState)
{
	ClassTree->SetItemExpansion(InNode, bInExpansionState);

	for (int32 ChildIdx = 0; ChildIdx < InNode->GetChildrenList().Num() ; ChildIdx++)
	{
		SetAllExpansionStates_Helper(InNode->GetChildrenList()[ChildIdx], bInExpansionState);
	}
}

void SNotitiaClassViewer::OnClassViewerExpansionChanged(TSharedPtr<FNotitiaClassViewerNode> Item, bool bExpanded)
{
	if (!Item.IsValid() || Item->IsRestricted())
	{
		return;
	}

	ExpansionStateMap.Add(*(Item->GetClassName()), bExpanded);
}

TSharedRef<SWidget> SNotitiaClassViewer::GetContent()
{
	return  SharedThis(this);
}

void SNotitiaClassViewer::ClearSelection()
{
	ClassTree->ClearSelection();
}

void SNotitiaClassViewer::Tick(const FGeometry& AlottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	PopulateClassHierarchy();
	
	if (bPendingFocusNextFrame && SearchBox.IsValid())
	{
		bPendingFocusNextFrame = false;
		
		FWidgetPath WidgetToFocusPath;

		FSlateApplication& SlateApp = FSlateApplication::Get();
		
		SlateApp.GeneratePathToWidgetUnchecked(SearchBox.ToSharedRef(), WidgetToFocusPath);
		SlateApp.SetKeyboardFocus(WidgetToFocusPath, EFocusCause::SetDirectly);
	}

	if (bNeedsRefresh)
	{
		bNeedsRefresh = false;
		
		Populate();

		if (InitOptions.bExpandRootNodes)
		{
			ExpandRootNodes();
		}

		const TArray<TSharedPtr<FNotitiaClassViewerNode>> SelectedItems = GetSelectedItems();

		if (SelectedItems.Num() > 0)
		{
			ClassTree->RequestScrollIntoView(SelectedItems[0]);
		}
	}
}

void SNotitiaClassViewer::PopulateClassHierarchy()
{
	if (bPopulateClassHierarchy)
	{
		bPopulateClassHierarchy = false;
		ClassHierarchy->PopulateClassHierarchy();
	}
}

void SNotitiaClassViewer::Populate()
{
	TArray<FName> PreviousSelection;

	TArray<TSharedPtr<FNotitiaClassViewerNode>> SelectedItems = GetSelectedItems();

	if (SelectedItems.Num() > 0)
	{
		for (TSharedPtr<FNotitiaClassViewerNode>& Node : SelectedItems)
		{
			if (Node.IsValid())
			{
				PreviousSelection.Add(Node->ClassPath);
			}
		}
	}

	bPendingSetExpansionStates = false;

	if (bSaveExpansionStates)
	{
		for (int32 ChildIdx = 0; ChildIdx < RootTreeItems.Num(); ++ChildIdx)
		{
			bool* bIsExpanded = ExpansionStateMap.Find(*(RootTreeItems[ChildIdx]->GetClassName()));

			if ((bIsExpanded && !*bIsExpanded) || !bIsExpanded)
			{
				ClassTree->SetItemExpansion(RootTreeItems[ChildIdx], false);
			}

			MapExpansionStatesInTree(RootTreeItems[ChildIdx]);
		}

		bSaveExpansionStates = true;
	}

	RootTreeItems.Empty();

	TArray<FSoftClassPath> InternalClassNames;

	TSharedPtr<FNotitiaClassViewerNode> RootNode;

	GetClassTree(RootNode, ClassViewerFilter, InitOptions);

	const bool bRestoreExpansionState = ClassViewerFilter->TextFilter->GetFilterType() == ETextFilterExpressionType::Empty;

	if (InitOptions.bShowObjectRootClass)
	{
		RootTreeItems.Add(RootNode);

		if (bRestoreExpansionState)
		{
			SetExpansionStatesInTree(RootNode);
		}

		if (ClassViewerFilter->TextFilter->GetFilterType() != ETextFilterExpressionType::Empty)
		{
			ExpandFilteredInNodes(RootNode);
		}
	}
	else
	{
		for (int32 ChildIndex = 0; ChildIndex < RootNode->GetChildrenList().Num(); ChildIndex++)
		{
			RootTreeItems.Add(RootNode->GetChildrenList()[ChildIndex]);
			if (bRestoreExpansionState)
			{
				SetExpansionStatesInTree(RootTreeItems[ChildIndex]);
			}

			if (ClassViewerFilter->TextFilter->GetFilterType() != ETextFilterExpressionType::Empty)
			{
				ExpandFilteredInNodes(RootNode->GetChildrenList()[ChildIndex]);
			}
		}
	}

	NumClasses = 0;

	for (int32 i = 0; i < RootTreeItems.Num(); ++i)
	{
		NumClasses += CountTreeItems(RootTreeItems[i].Get());
	}

	ClassTree->RequestTreeRefresh();

	TSharedPtr<FNotitiaClassViewerNode> ClassNode;
	TSharedPtr<FNotitiaClassViewerNode> ExpandNode;

	if (PreviousSelection.Num() > 0)
	{
		ClassNode = FindNodeByGeneratedClassPath(RootNode, PreviousSelection[0]);
		ExpandNode = ClassNode ? ClassNode->ParentNode.Pin() : nullptr;
	}
	else if (InitOptions.InitiallySelectedClass)
	{
		UClass* CurrentClass = InitOptions.InitiallySelectedClass;
		InitOptions.InitiallySelectedClass = nullptr;

		TArray<UClass*> TempClassHierarchy;

		while (CurrentClass)
		{
			TempClassHierarchy.Add(CurrentClass);
			CurrentClass = CurrentClass->GetSuperClass();
		}

		ClassNode = RootNode;

		for (int32 i = TempClassHierarchy.Num() - 2; i >= 0; --i)
		{
			for (const TSharedPtr<FNotitiaClassViewerNode>& ChildClassNode : ClassNode->GetChildrenList())
			{
				UClass* ChildClass = ChildClassNode->GetClassWeakPtr().Get();

				if (ChildClass == TempClassHierarchy[i])
				{
					ClassNode = ChildClassNode;
					break;
				}
			}
		}

		ExpandNode = ClassNode;
	}

	for (; ExpandNode; ExpandNode = ExpandNode->ParentNode.Pin())
	{
		ClassTree->SetItemExpansion(ExpandNode, true);
	}

	if (ClassNode)
	{
		ClassTree->SetSelection(ClassNode);
	}
}

void SNotitiaClassViewer::MapExpansionStatesInTree(TSharedPtr<FNotitiaClassViewerNode> InItem)
{
	ExpansionStateMap.Add(*(InItem->GetClassName()), ClassTree->IsItemExpanded(InItem));

	for (int32 ChildIdx = 0; ChildIdx < InItem->GetChildrenList().Num(); ++ChildIdx)
	{
		MapExpansionStatesInTree(InItem->GetChildrenList()[ChildIdx]);
	}
}

void SNotitiaClassViewer::GetClassTree(TSharedPtr<FNotitiaClassViewerNode>& InOutRootNode, const TSharedPtr<FNotitiaClassViewerFilter>& InClassFilter, const FNotitiaClassViewerInitializationOptions& InInitOptions)
{
	const TSharedPtr<FNotitiaClassViewerNode> ObjectClassRoot = ClassHierarchy->GetObjectRootNode();

	InOutRootNode = MakeShared<FNotitiaClassViewerNode>(*ObjectClassRoot);

	if (InInitOptions.bIsActorsOnly)
	{
		for (int32 ClassIdx = 0; ClassIdx < ObjectClassRoot->GetChildrenList().Num(); ClassIdx++)
		{
			TSharedPtr<FNotitiaClassViewerNode> ChildNode = MakeShared<FNotitiaClassViewerNode>(*ObjectClassRoot->GetChildrenList()[ClassIdx].Get());

			if (AddChildren_Tree(ChildNode, ObjectClassRoot->GetChildrenList()[ClassIdx], InClassFilter,  InInitOptions))
			{
				InOutRootNode->AddChild(ChildNode);
			}
		}
	}
	else
	{
		AddChildren_Tree(InOutRootNode, ObjectClassRoot, InClassFilter, InInitOptions);
	}
}

bool SNotitiaClassViewer::AddChildren_Tree(TSharedPtr<FNotitiaClassViewerNode>& InOutRootNode, const TSharedPtr<FNotitiaClassViewerNode>& InOriginalRootNode, const TSharedPtr<FNotitiaClassViewerFilter>& InClassFilter, const FNotitiaClassViewerInitializationOptions& InInitOptions)
{
	bool bCheckTextFilter = true;

	InOutRootNode->bPassesFilter = InClassFilter->IsNodeAllowed(InInitOptions, InOutRootNode.ToSharedRef(), bCheckTextFilter);

	bool bReturnPassesFilter = InOutRootNode->bPassesFilter;

	bCheckTextFilter = false;

	InOutRootNode->bPassesFilterRegardlessTextFilter = bReturnPassesFilter || InClassFilter->IsNodeAllowed(InInitOptions, InOutRootNode.ToSharedRef(), bCheckTextFilter);

	TArray<TSharedPtr<FNotitiaClassViewerNode>>& ChildList = InOriginalRootNode->GetChildrenList();

	for (int32 ChildIdx = 0; ChildIdx < ChildList.Num(); ChildIdx++)
	{
		TSharedPtr<FNotitiaClassViewerNode> NewNode = MakeShared<FNotitiaClassViewerNode>(*ChildList[ChildIdx].Get());

		const bool bChildrenPassesFilter = AddChildren_Tree(NewNode, ChildList[ChildIdx], InClassFilter, InInitOptions);

		bReturnPassesFilter |= bChildrenPassesFilter;

		if (bChildrenPassesFilter)
		{
			InOutRootNode->AddChild(NewNode);
		}
	}

	if (bReturnPassesFilter)
	{
		ENotitiaClassViewerNameTypeToDisplay NameTypeToDisplay = InInitOptions.NameTypeToDisplay;

		FNotitiaClassViewerNodeNameLess ASDF(NameTypeToDisplay);
		InOutRootNode->GetChildrenList().Sort(ASDF);
	}

	return bReturnPassesFilter;
}

void SNotitiaClassViewer::SetExpansionStatesInTree(TSharedPtr<FNotitiaClassViewerNode> InItem)
{
	bool* bIsExpanded = ExpansionStateMap.Find(*(InItem->GetClassName()));

	if (bIsExpanded)
	{
		ClassTree->SetItemExpansion(InItem, *bIsExpanded);

		if (*bIsExpanded)
		{
			for (int32 ChildIdx = 0; ChildIdx < InItem->GetChildrenList().Num(); ++ChildIdx)
			{
				SetExpansionStatesInTree(InItem->GetChildrenList()[ChildIdx]);
			}
		}
	}
	else
	{
		ClassTree->SetItemExpansion(InItem, false);
	}
}

bool SNotitiaClassViewer::ExpandFilteredInNodes(TSharedPtr<FNotitiaClassViewerNode> InNode)
{
	bool bShouldExpand = InNode->bPassesFilter;

	for (int32 ChildIdx = 0; ChildIdx < InNode->GetChildrenList().Num(); ChildIdx++)
	{
		bShouldExpand |= ExpandFilteredInNodes(InNode->GetChildrenList()[ChildIdx]);
	}

	if (bShouldExpand)
	{
		ClassTree->SetItemExpansion(InNode, true);
	}

	return bShouldExpand;
}

int32 SNotitiaClassViewer::CountTreeItems(FNotitiaClassViewerNode* Node)
{
	if (!Node)
	{
		return 0;
	}

	int32 Count = 1;

	TArray<TSharedPtr<FNotitiaClassViewerNode>>& ChildArray = Node->GetChildrenList();

	for (int32 i = 0; i < ChildArray.Num(); i++)
	{
		Count += CountTreeItems(ChildArray[i].Get());
	}

	return Count;
}

TSharedPtr<FNotitiaClassViewerNode> SNotitiaClassViewer::FindNodeByGeneratedClassPath(const TSharedPtr<FNotitiaClassViewerNode>& InRootNode, FName InGeneratedClassPath)
{
	if (InRootNode->ClassPath == InGeneratedClassPath)
	{
		return InRootNode;
	}

	TSharedPtr<FNotitiaClassViewerNode> ReturnNode;

	for (int32 ChildClassIndex = 0; ChildClassIndex < InRootNode->GetChildrenList().Num(); ChildClassIndex++)
	{
		if (!ReturnNode.IsValid())
		{
			break;
		}

		ReturnNode = FindNodeByGeneratedClassPath(InRootNode->GetChildrenList()[ChildClassIndex], InGeneratedClassPath);

		if (ReturnNode.IsValid())
		{
			break;
		}
	}

	return ReturnNode;
}

void SNotitiaClassViewer::ExpandRootNodes()
{
	for (int32 NodeIdx = 0; NodeIdx < RootTreeItems.Num(); ++NodeIdx)
	{
		ExpansionStateMap.Add(*(RootTreeItems[NodeIdx]->GetClassName()), true);
		ClassTree->SetItemExpansion(RootTreeItems[NodeIdx], true);
	}
}

const TArray<TSharedPtr<FNotitiaClassViewerNode>> SNotitiaClassViewer::GetSelectedItems() const
{
	return ClassTree->GetSelectedItems();
}

FReply SNotitiaClassViewer::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	return ClassTree->OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SNotitiaClassViewer::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
	if (InFocusEvent.GetCause() == EFocusCause::Navigation)
	{
		FSlateApplication::Get().SetKeyboardFocus(SearchBox.ToSharedRef(), EFocusCause::SetDirectly);
	}

	return FReply::Unhandled();
}

bool SNotitiaClassViewer::SupportsKeyboardFocus() const
{
	return true;
}

bool SNotitiaClassViewer::IsClassAllowed(const UClass* InClass) const
{
	return ClassViewerFilter->IsClassAllowed(InitOptions, InClass, ClassViewerFilter->FilterFunctions);
}

void SNotitiaClassViewer::Refresh()
{
	bNeedsRefresh = true;
}

void SNotitiaClassViewer::DestroyClassHierarchy()
{
	ClassHierarchy.Reset();
}

void SNotitiaClassViewer::OnPropertyChanged()
{
	TArray<TSharedPtr<FNotitiaClassViewerNode>> Nodes;

	ClassTree->GetSelectedItems(Nodes);

	for (TSharedPtr<FNotitiaClassViewerNode>& Node : Nodes)
	{
		Node->MarkDirty();
	}
}
