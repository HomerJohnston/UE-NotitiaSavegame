#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"


class FPropertyCustomization_NotitiaPropertySelector : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	TArray<TSharedPtr<FString>> Items;

protected:
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	virtual void  CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	TSharedRef<ITableRow> MakeRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);
};