#include "NotitiaEditor/PropertyViewer/NotitiaPropertyItem.h"
#include "NotitiaEditor/PropertyViewer/NotitiaPropertyViewerNode.h"
#include "Widgets/Layout/SSeparator.h"

void SNotitiaPropertyItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	FLinearColor CheckBoxColor = FLinearColor::Gray;

	PropertyViewer = InArgs._PropertyViewer;
	
	AssociatedNode = InArgs._AssociatedNode;

	if (!AssociatedNode)
	{
		ChildSlot
		[
			SNew(STextBlock)
			// TODO Localization
			.Text(FText::FromString("Invalid node! This should never happen"))
		];
		return;
	}

	CheckBox = SNew(SCheckBox)
		.ForegroundColor(CheckBoxColor)
		.OnCheckStateChanged(this, &SNotitiaPropertyItem::OnCheckStateChanged)
		.IsChecked(AssociatedNode->GetPropertyVisibleCheckState())
		.IsEnabled(AssociatedNode->PropertyNotFromSuperClass())
		.BorderBackgroundColor(FLinearColor::White);

	PropertyName = SNew(STextBlock)
		// TODO Localization
		.Text(FText::FromString(AssociatedNode->GetPropertyDisplayName()));

	UClass* SourceClass = AssociatedNode.Get()->GetOwnerClass();
	FString ClassName = SourceClass->GetPrefixCPP() + SourceClass->GetName();
		
	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.Visibility(InArgs._DisplayClassHeader ? EVisibility::Visible : EVisibility::Collapsed)
			.MinDesiredHeight(24)
			[
				SNew(SBox)
				.MinDesiredHeight(24)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(ClassName))
				]
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SExpanderArrow, SharedThis(this))
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				CheckBox.ToSharedRef()
			]
			+SHorizontalBox::Slot()
			.FillWidth(0.6f)
			.Padding(0.0f, 3.0f, 6.0f, 3.0f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			[
				PropertyName.ToSharedRef()
			]
			+SHorizontalBox::Slot()
			.FillWidth(0.4f)
			.Padding(0.0f, 3.0f, 6.0f, 3.0f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Right)
			[
				SNew(STextBlock)
				// TODO Localization
				.Text(FText::FromString(*InArgs._PropertyType))
			]		
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.Visibility(InArgs._DisplayClassFooter ? EVisibility::Visible : EVisibility::Collapsed)
			[
				SNew(SSeparator)
			]
		]
	];
	
	STableRow<TSharedPtr<FString>>::ConstructInternal(STableRow::FArguments().ShowSelection(true), InOwnerTableView);

	AssociatedNode->MarkDirty();
	
	Update();
}

FSlateColor SNotitiaPropertyItem::GetTextColor() const
{
	return FSlateColor::UseForeground();
}

EVisibility SNotitiaPropertyItem::ShowOptions() const
{
	return EVisibility::Visible;
}

void SNotitiaPropertyItem::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	Update();
}

void SNotitiaPropertyItem::Update()
{	
	if (!AssociatedNode)
	{
		CheckBox->SetColorAndOpacity(FLinearColor::Black);
		PropertyName->SetColorAndOpacity(FLinearColor::Black);
		return;
	}
	
	if (!AssociatedNode->CleanDirty())
	{
		return;
	}

	ECheckBoxState CheckState = AssociatedNode->GetPropertyVisibleCheckState();
		
	CheckBox->SetIsChecked(CheckState);

	bool bOwnerClassNull = AssociatedNode->SourceProperty->GetOwnerClass() == nullptr;
	bool bOwnerClassSame = AssociatedNode->PropertyNotFromSuperClass();
	
	FLinearColor ParentColor = FColorList::Feldspar;

	if (bOwnerClassSame)
	{
		ParentColor = FLinearColor::White;
	}

	FLinearColor PrimaryColor = CheckState != ECheckBoxState::Unchecked ? FLinearColor::White : FLinearColor::Gray;
	FLinearColor FinalColor = ParentColor * PrimaryColor;

	CheckBox->SetColorAndOpacity(FinalColor);
	PropertyName->SetColorAndOpacity(FinalColor);
}

void SNotitiaPropertyItem::OnCheckStateChanged(ECheckBoxState NewState)
{
	//PropertyViewer->OnPropertyChanged.Execute();
	AssociatedNode->OnCheckStateChanged(NewState);

	OnPropertyChanged.ExecuteIfBound();
}
