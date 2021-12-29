#pragma once

#include "IDetailCustomization.h"

class FDetailCustomization_NotitiaDeveloperSettings : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	
	TArray<TSharedRef<FString>> SourceArray;
	
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	
	TSharedRef<ITableRow> HandleGeneratePropertyRow(TSharedRef<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);
	
	void HandleGetPropertyChildren(TSharedRef<FString> InItem, TArray<TSharedRef<FString>>& OutChildren);
};


// Container for an entry in the property view
class SNotitiaPropertyNode : public SMultiColumnTableRow<TSharedRef<FString>>
{
public:
	TSharedPtr<FString> Node;
	
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TSharedRef<FString> InNode);

	// SMultiColumnTableRow interface
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnName) override;
};
