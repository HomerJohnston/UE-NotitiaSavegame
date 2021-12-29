#include "NotitiaEditor/ClassViewer/NotitiaClassItem.h"

#include "EditorClassUtils.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewer.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerNode.h"
#include "IDocumentation.h"
#include "Notitia/Core/NotitiaDeveloperSettings.h"
#include "NotitiaEditor/Debug/NotitiaEditorLogging.h"
#include "Styling/SlateIconFinder.h"

void SNotitiaClassItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	ClassName = InArgs._ClassName;
	bIsInClassViewer = InArgs._bIsInClassViewer;
	bDynamicClassLoading = InArgs._bDynamicClassLoading;
	AssociatedNode = InArgs._AssociatedNode;
	OnDoubleClicked = InArgs._OnClassItemDoubleClicked;
	
	bool bIsBlueprint = false;
	bool bHasBlueprint = false;
	bool bIsRestricted = AssociatedNode->IsRestricted();
	
	SNotitiaClassViewer::GetClassInfo(AssociatedNode->GetClassWeakPtr(), bIsBlueprint, bHasBlueprint);

	struct Local
	{
		static TSharedPtr<SToolTip> GetToolTip(TSharedPtr<FNotitiaClassViewerNode> AssociatedNode)
		{
			TSharedPtr<SToolTip> ToolTip;

			if (AssociatedNode->PropertyHandle.IsValid() && AssociatedNode->IsRestricted())
			{
				FText RestrictionToolTip;
				AssociatedNode->PropertyHandle->GenerateRestrictionToolTip(*AssociatedNode->GetClassName(), RestrictionToolTip);

				ToolTip = IDocumentation::Get()->CreateToolTip(RestrictionToolTip, nullptr, "", "");
			}
			else if (UClass* Class = AssociatedNode->GetClassWeakPtr().Get())
			{
				UPackage* Package = Class->GetOutermost();
				ToolTip = FEditorClassUtils::GetTooltip(Class);
			}
			else if (AssociatedNode->ClassPath != NAME_None)
			{
				ToolTip = SNew(SToolTip).Text(FText::FromName(AssociatedNode->ClassPath));
			}

			return ToolTip;
		}
	};
	
	CheckBox = SNew(SCheckBox)
		//.ForegroundColor(CheckBoxColor)
		.OnCheckStateChanged(this, &SNotitiaClassItem::OnCheckStateChanged)
		//.IsChecked(AssociatedNode->GetPropertyVisibleCheckState())
		//.IsEnabled(AssociatedNode->PropertyNotFromSuperClass())
		.BorderBackgroundColor(FLinearColor::White);
	
	ItemText = SNew(STextBlock)
	.Text(FText::FromString(*ClassName.Get()))
	.HighlightText(InArgs._HighlightText)
	.ToolTip(Local::GetToolTip(AssociatedNode))
	.IsEnabled(!bIsRestricted);

	ItemPropertyCountText = SNew(STextBlock);
		
	const FSlateBrush* ClassIcon = FSlateIconFinder::FindIconBrushForClass(AssociatedNode->GetClassWeakPtr().Get());

	this->ChildSlot
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
		.AutoWidth()
		.Padding(0.0f, 2.0f, 6.0f, 2.0f)
		[
			SNew(SImage)
			.Image(ClassIcon)
			.Visibility(ClassIcon != FEditorStyle::GetDefaultBrush() ? EVisibility::Visible : EVisibility::Collapsed)
		]
		+SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0.0f, 3.0f, 6.0f, 3.0f)
		.VAlign(VAlign_Center)
		[
			ItemText.ToSharedRef()
		]
		+SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(6.0f, 3.0f, 0.0f, 3.0f)
		[
			ItemPropertyCountText.ToSharedRef()
		]/*
		+SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(0.0f, 0.0f, 6.0f, 0.0f)
		[
			SNew(SComboButton)
			.ContentPadding(FMargin(2.0f))
			.Visibility(this, &SNotitiaClassItem::ShowOptions)
			.OnGetMenuContent(this, &SNotitiaClassItem::GenerateDropDown)
		]*/
	];

	AssociatedNode->MarkDirty();
	
	Update();
	
	STableRow<TSharedPtr<FString>>::ConstructInternal(STableRow::FArguments().ShowSelection(true), InOwnerTableView);
}

void SNotitiaClassItem::OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	
}

FSlateColor SNotitiaClassItem::GetTextColor() const
{
	const TSharedPtr<ITypedTableView<TSharedPtr<FString>>> OwnerWidget = OwnerTablePtr.Pin();
	const TSharedPtr<FString>* MyItem = OwnerWidget->Private_ItemFromWidget(this);
	const bool bIsSelected = OwnerWidget->Private_IsItemSelected(*MyItem);

	if (bIsSelected)
	{
		return FSlateColor::UseForeground();
	}

	return FLinearColor::White;
}

EVisibility SNotitiaClassItem::ShowOptions() const
{
	if (bIsInClassViewer)
	{
		bool bIsBlueprint = false;
		bool bHasBlueprint = false;

		SNotitiaClassViewer::GetClassInfo(AssociatedNode->GetClassWeakPtr(), bIsBlueprint, bHasBlueprint);

		return (bIsBlueprint || AssociatedNode->Blueprint.IsValid()) ? EVisibility::Visible : EVisibility::Collapsed;
	}

	return EVisibility::Collapsed;
}

TSharedRef<SWidget> SNotitiaClassItem::GenerateDropDown()
{
	if (UClass* Class = AssociatedNode->GetClassWeakPtr().Get())
	{
		bool bIsBlueprint = false;
		bool bHasBlueprint = false;

		SNotitiaClassViewer::GetClassInfo(Class, bIsBlueprint, bHasBlueprint);
		
		bHasBlueprint = AssociatedNode->Blueprint.IsValid();

		return SNotitiaClassViewer::CreateMenu(Class, bIsBlueprint, bHasBlueprint);
	}

	return SNullWidget::NullWidget;
}

void SNotitiaClassItem::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	Update();
	
	STableRow<TSharedPtr<FString, ESPMode::Fast>>::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

void SNotitiaClassItem::Update()
{
	if (!AssociatedNode->CleanDirty())
	{
		return;
	}

	const UNotitiaDeveloperSettings* DeveloperSettings = GetDefault<UNotitiaDeveloperSettings>();

	FColor IdleColor = FColor(192, 192, 192, 255);
	FColor ActiveColor = FColor(127, 255, 0, 255);
	FColor ActiveChildColor = FColor(150, 150, 0, 255);
	FColor ActiveUnloadedColor = FColor(92, 220, 0, 255);
	FColor ActiveChildUnloadedColor = FColor(127, 140, 0, 255);
	FColor UnloadedColor = FColor(128, 128, 128, 255);

	FColor TextColor;

	//NOTITIAED_LOG_WARNING("Checking %s", *AssociatedNode->GetClassFName().ToString());

	bool bExactMatch;
	bool bIsSelected = DeveloperSettings->ContainsClass(AssociatedNode->GetClassWeakPtr().Get(), bExactMatch);
	
	bool bIsLoaded = AssociatedNode->GetClassWeakPtr().IsValid();
	
	if (bIsSelected && bIsLoaded)
	{
		//NOTITIAED_LOG_WARNING("Active");
		TextColor = (bExactMatch) ? ActiveColor : ActiveChildColor;
	}
	else if (bIsSelected)
	{
		//NOTITIAED_LOG_WARNING("Active, Unloaded");
		TextColor = (bExactMatch) ? ActiveUnloadedColor : ActiveChildUnloadedColor;
	}
	else if (bIsLoaded)
	{
		//NOTITIAED_LOG_WARNING("Idle");
		TextColor = IdleColor;
	}
	else
	{
		//NOTITIAED_LOG_WARNING("Idle, Unloaded");
		TextColor = UnloadedColor;
	}

	ItemText->SetColorAndOpacity(TextColor);
	
	int32 NumSelectedProperties = AssociatedNode->GetNumSelectedProperties();
	int32 NumProperties = AssociatedNode->GetNumProperties();
	
	bool bShowPropertyCounts = NumSelectedProperties > 0;

	if (bShowPropertyCounts)
	{
		FString NumSelectedPropertiesString = FString::FromInt(NumSelectedProperties);
		FString NumPropertiesString = (NumProperties ? FString::FromInt(NumProperties) : "?");
		ItemPropertyCountText->SetText(FText::FromString(*FString::Printf(TEXT("(%s/%s)"), *NumSelectedPropertiesString, *NumPropertiesString)));
		ItemPropertyCountText->SetVisibility(EVisibility::Visible);
	}
	else
	{
		ItemPropertyCountText->SetVisibility(EVisibility::Collapsed);
	}
}
