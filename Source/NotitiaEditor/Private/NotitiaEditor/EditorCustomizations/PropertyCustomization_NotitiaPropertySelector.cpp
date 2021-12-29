#include "NotitiaEditor/EditorCustomizations/PropertyCustomization_NotitiaPropertySelector.h"

#include "DetailWidgetRow.h"
#include "Slate/Private/Widgets/Views/SListPanel.h"

#define LOCTEXT_NAMESPACE "Notitia"

TSharedRef<IPropertyTypeCustomization> FPropertyCustomization_NotitiaPropertySelector::MakeInstance()
{
	return MakeShareable(new FPropertyCustomization_NotitiaPropertySelector());
}

void FPropertyCustomization_NotitiaPropertySelector::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TSharedPtr<IPropertyHandle> PH1 = PropertyHandle->GetParentHandle();
	TSharedPtr<IPropertyHandle> PH2 = PH1->GetParentHandle();
	TSharedPtr<IPropertyHandle> PH3 = PH2->GetParentHandle();
	TSharedPtr<IPropertyHandle> PH4 = PH3->GetParentHandle();
	TSharedPtr<IPropertyHandle> PH5 = PH4->GetParentHandle();
	
	UE_LOG(LogTemp, Warning, TEXT("PropertyHandle: %s"), *PropertyHandle->GetPropertyDisplayName().ToString());
	UE_LOG(LogTemp, Warning, TEXT("PH1: %s"), *PH1->GetPropertyDisplayName().ToString());
	UE_LOG(LogTemp, Warning, TEXT("PH2: %s"), *PH2->GetPropertyDisplayName().ToString());
	UE_LOG(LogTemp, Warning, TEXT("PH3: %s"), *PH3->GetPropertyDisplayName().ToString());
	UE_LOG(LogTemp, Warning, TEXT("PH4: %s"), *PH4->GetPropertyDisplayName().ToString());
	UE_LOG(LogTemp, Warning, TEXT("PH5: %s"), *PH5->GetPropertyDisplayName().ToString());

	TSharedPtr<IPropertyHandle> ClassProp = PH2->GetChildHandle("Class");

	FProperty* Prop = ClassProp->GetProperty();
	
	//void* ValuePtr;
	//ClassProp->GetValueData(ValuePtr);

	FString FormattedClass = "";

	Items = TArray<TSharedPtr<FString>>();
	
	if (ClassProp.IsValid())
	{
		ClassProp->GetValueAsFormattedString(FormattedClass);
		UE_LOG(LogTemp, Warning, TEXT("Class: %s"), *FormattedClass);
		
		UClass* ContextClass = FindObject<UClass>(ANY_PACKAGE, *FormattedClass);

		for (TFieldIterator<FProperty> PropIt(ContextClass); PropIt; ++PropIt)
		{
			FProperty* P = *PropIt;
			UE_LOG(LogTemp, Warning, TEXT("Property: %s"), *P->GetName());
			Items.Add(MakeShareable(new FString(*P->GetName())));
		}
	}

	Items.Sort([=](const TSharedPtr<FString> A, const TSharedPtr<FString> B) {return *A < *B;} );

	TSharedRef<SListView<TSharedPtr<FString>>> ListView = SNew(SListView<TSharedPtr<FString>>).ItemHeight(24).ListItemsSource(&Items).OnGenerateRow(this, &FPropertyCustomization_NotitiaPropertySelector::MakeRow);

	HeaderRow.WholeRowContent()
	[
		ListView
	];
	
}

void FPropertyCustomization_NotitiaPropertySelector::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

TSharedRef<ITableRow> FPropertyCustomization_NotitiaPropertySelector::MakeRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedPtr<SWidget> TagItem;


	if (Item.IsValid())
	{
		TagItem = SNew(STextBlock).Text(FText::FromString(*Item.Get()));
	}
	else
	{
		TagItem = SNew(STextBlock).Text(FText::FromString("NONe"));
	}
	

	return SNew( STableRow< TSharedPtr<FString> >, OwnerTable )
	[
		SNew(SBorder)
		//.OnMouseButtonDown(this, &FGameplayTagContainerCustomization::OnSingleTagMouseButtonPressed, *Item.Get())
		.Padding(0.0f)
		.BorderImage(FEditorStyle::GetBrush("NoBorder"))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0,0,2,0)
			[
				SNew(SButton)
				//.IsEnabled(!StructPropertyHandle->IsEditConst())
				.ContentPadding(FMargin(0))
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Danger")
				.ForegroundColor(FSlateColor::UseForeground())
				/*.OnClicked(this, &FGameplayTagContainerCustomization::OnRemoveTagClicked, *Item.Get())
				[
					SNew(STextBlock)
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.9"))
					.Text(FEditorFontGlyphs::Times)
				]*/
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				TagItem.ToSharedRef()
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE