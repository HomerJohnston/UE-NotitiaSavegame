#include "NotitiaEditor/PropertyViewer/NotitiaPropertyViewer.h"

#include "NotitiaEditor/PropertyViewer/NotitiaPropertyItem.h"
#include "NotitiaEditor/PropertyViewer/NotitiaPropertyItem.h"
#include "NotitiaEditor/PropertyViewer/NotitiaPropertyViewerNode.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SScrollBorder.h"
#include "Widgets/Layout/SSeparator.h"

SNotitiaPropertyViewer::~SNotitiaPropertyViewer()
{
}

void SNotitiaPropertyViewer::Construct(const FArguments& InArgs)
{
	SourceClass = AActor::StaticClass();

	Populate();
	
	SAssignNew(PropertyTree, STreeView<TSharedPtr<FNotitiaPropertyViewerNode>>)
	.SelectionMode(ESelectionMode::None)
	.TreeItemsSource(&RootTreeItems)
	.OnGetChildren(this, &SNotitiaPropertyViewer::OnGetChildrenForPropertyViewer)
	.OnSetExpansionRecursive(this, &SNotitiaPropertyViewer::SetAllExpansionStates_Helper)
	.OnGenerateRow(this, &SNotitiaPropertyViewer::OnGenerateRowForPropertyViewer)
	//.OnContextMenuOpening(OnContextMenuOpening)
	.OnSelectionChanged(this, &SNotitiaPropertyViewer::OnPropertyViewerSelectionChanged)
	.OnExpansionChanged(this, &SNotitiaPropertyViewer::OnPropertyViewerExpansionChanged)
	.ItemHeight(20.0f);
	
	TSharedRef<STreeView<TSharedPtr<FNotitiaPropertyViewerNode>>> PropertyTreeView = PropertyTree.ToSharedRef();
	
	TSharedPtr<SWidget> PropertyViewerContent;

	PropertyViewerContent = SNew(SBox)
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
				.FillWidth(1.0f)
				.Padding(2.0f, 2.0f)
				[
					SAssignNew(SearchBox, SSearchBox)
					.OnTextChanged(this, &SNotitiaPropertyViewer::OnFilterTextChanged)
					.OnTextCommitted(this, &SNotitiaPropertyViewer::OnFilterTextCommitted)
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 2.0f)
			[
				SNew(SCheckBox)
				.Padding(FMargin(4.0f, 0.0f))
				.Content()
				[
					SNew(STextBlock)
					.Text(INVTEXT("Show selected properties"))
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 2.0f)
			[
				SNew(SCheckBox)
				.Padding(FMargin(4.0f, 0.0f))
				.Content()
				[
					SNew(STextBlock)
					.Text(INVTEXT("Show unselected properties"))
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
					SNew(SScrollBorder, PropertyTreeView)
					[
						PropertyTreeView
					]
				]
			]
		]
	];
	
	ChildSlot
	[
		PropertyViewerContent.ToSharedRef()
	];
}

void SNotitiaPropertyViewer::OnGetChildrenForPropertyViewer(TSharedPtr<FNotitiaPropertyViewerNode> InParent, TArray<TSharedPtr<FNotitiaPropertyViewerNode>>& OutChildren)
{
	OutChildren = InParent->GetChildrenList();
}

void SNotitiaPropertyViewer::SetAllExpansionStates_Helper(TSharedPtr<FNotitiaPropertyViewerNode> InNode, bool bInExpansionState)
{
	PropertyTree->SetItemExpansion(InNode, bInExpansionState);

	for (int32 ChildIndex = 0; ChildIndex < InNode->GetChildrenList().Num(); ChildIndex++)
	{
		SetAllExpansionStates_Helper(InNode->GetChildrenList()[ChildIndex], bInExpansionState);
	}
}

TSharedRef<ITableRow> SNotitiaPropertyViewer::OnGenerateRowForPropertyViewer(TSharedPtr<FNotitiaPropertyViewerNode> Node, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedPtr<FString> PropertyName = MakeShareable(new FString(Node->GetPropertyDisplayName()));
	TSharedPtr<FString> PropertyType = MakeShareable(new FString(Node->GetPropertyType()));
	
	TSharedRef<SNotitiaPropertyItem> ReturnRow = SNew(SNotitiaPropertyItem, OwnerTable)
	.PropertyName(PropertyName)
	.PropertyType(PropertyType)
	.AssociatedNode(Node)
	//.PropertyViewer(MakeShareable<SNotitiaPropertyViewer>(this))
	.DisplayClassHeader(Node->GetDisplayClassHeader())
	.DisplayClassFooter(Node->GetDisplayClassFooter());

	ReturnRow->OnPropertyChanged.BindSP(this, &SNotitiaPropertyViewer::OnPropertyChanged);
	
	return ReturnRow;
}

void SNotitiaPropertyViewer::OnPropertyViewerSelectionChanged(TSharedPtr<FNotitiaPropertyViewerNode> Item, ESelectInfo::Type SelectInfo)
{
	
}

void SNotitiaPropertyViewer::OnPropertyViewerExpansionChanged(TSharedPtr<FNotitiaPropertyViewerNode> Item, bool bInExpansionState)
{
	if (!Item.IsValid())
	{
		return;
	}

	ExpansionStateMap.Add(*(Item->GetPropertyDisplayName()), bInExpansionState);
}

void SNotitiaPropertyViewer::OnFilterTextChanged(const FText& InText)
{
}

void SNotitiaPropertyViewer::OnFilterTextCommitted(const FText& InText, ETextCommit::Type CommitInfo)
{
}

void SNotitiaPropertyViewer::Tick(const FGeometry& AlottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (bNeedsRefresh)
	{
		bNeedsRefresh = false;

		Populate();
	}
}

void SNotitiaPropertyViewer::Populate()
{
	if (!PropertyHierarchy.IsValid())
	{
		PropertyHierarchy = MakeShareable(new FNotitiaPropertyHierarchy);
	}

	PropertyHierarchy->PopulatePropertyHierarchy(SourceClass);

	RootTreeItems.Empty();

	for (TSharedPtr<FNotitiaPropertyViewerNode> Node : PropertyHierarchy->GetRootProperties())
	{
		RootTreeItems.Add(Node);
	}
}

void SNotitiaPropertyViewer::OnClassPicked(UClass* InClass)
{
	SourceClass = InClass;

	Populate();

	PropertyTree->RequestTreeRefresh();
}

void SNotitiaPropertyViewer::OnPropertyChanged()
{
	OnPropertyChangedEvt.ExecuteIfBound();
}
