#include "NotitiaEditor/EditorCustomizations/DetailCustomization_NotitiaDeveloperSettings.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewer.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerInitializationOptions.h"
#include "NotitiaEditor/PropertyViewer/NotitiaPropertyViewer.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Slate/Private/Widgets/Views/SListPanel.h"

TSharedRef<IDetailCustomization> FDetailCustomization_NotitiaDeveloperSettings::MakeInstance()
{
	return MakeShareable(new FDetailCustomization_NotitiaDeveloperSettings);
}

void FDetailCustomization_NotitiaDeveloperSettings::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TSharedRef<SVerticalBox> Test = SNew(SVerticalBox);

	TSharedRef<STextBlock> Txt = SNew(STextBlock).Text(INVTEXT("Hello World"));

	SourceArray.Add(MakeShareable(new FString("Hello")));
	SourceArray.Add(MakeShareable(new FString("World")));

	FNotitiaClassViewerInitializationOptions InitOptions;

	TSharedRef<SNotitiaClassViewer> ClassViewer = SNew(SNotitiaClassViewer, InitOptions);//.OnClassPickedDelegate(FOnClassPicked());
	TSharedRef<SNotitiaPropertyViewer> PropertyViewer = SNew(SNotitiaPropertyViewer);

	ClassViewer->OnClassPicked.BindSP(PropertyViewer, &SNotitiaPropertyViewer::OnClassPicked);
	PropertyViewer->OnPropertyChangedEvt.BindSP(ClassViewer, &SNotitiaClassViewer::OnPropertyChanged);
	
	IDetailCategoryBuilder& Cat = DetailBuilder.EditCategory(FName("ClassPropertySaveMatrix"), INVTEXT("Class Property Save Matrix"));
	// TODO 
	Cat.AddCustomRow(INVTEXT("LolOlOl"))
	.WholeRowContent()
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.FillWidth(0.4f)
		[
			ClassViewer
		]
		+SHorizontalBox::Slot()
		.FillWidth(0.6f)
		[
			PropertyViewer
		]
	];
	
	TArray<TAssetSubclassOf<UClass>> Subclasses;
	
	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		UClass* Class = *ClassIt;

		if (!Class->IsNative())
		{
			continue;
		}

		if (Class->HasAnyClassFlags(CLASS_Deprecated | CLASS_NewerVersionExists))
		{
			continue;
		}

#if WITH_EDITOR
		if (FKismetEditorUtilities::IsClassABlueprintSkeleton(Class))
		{
			continue;
		}
#endif

		Subclasses.Add(Class);
	}
}

TSharedRef<ITableRow> FDetailCustomization_NotitiaDeveloperSettings::HandleGeneratePropertyRow(TSharedRef<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return 
		SNew(SNotitiaPropertyNode, OwnerTable, Item);
		//.FilterText_Lambda([this](){ return FilterText; });
}

void FDetailCustomization_NotitiaDeveloperSettings::HandleGetPropertyChildren(TSharedRef<FString> InItem, TArray<TSharedRef<FString>>& OutChildren)
{
	return;
	/*
	for(const TSharedRef<SNotitiaPropertyNode>& Child : InItem->FlattenedLinearChildren)
	{
		if(Child->FilterState != EAnimGraphSchematicFilterState::Hidden)
		{
			OutChildren.Add(Child);
		}
	}
	*/
}

void SNotitiaPropertyNode::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TSharedRef<FString> InNode)
{
	Node = InNode;

	SMultiColumnTableRow<TSharedRef<FString>>::Construct(
		FSuperRowType::FArguments()
		.Padding(1.0f),
		InOwnerTable
	);
}

TSharedRef<SWidget> SNotitiaPropertyNode::GenerateWidgetForColumn(const FName& InColumnName)
{
	return
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			 .AutoWidth()
			 .VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(*Node.Get()))
			]
		];
}
