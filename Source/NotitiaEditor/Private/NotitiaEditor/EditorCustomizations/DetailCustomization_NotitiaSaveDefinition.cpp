#include "NotitiaEditor/EditorCustomizations/DetailCustomization_NotitiaSaveDefinition.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

TSharedRef<IDetailCustomization> FDetailCustomization_NotitiaSaveDefinition::MakeInstance()
{
	return MakeShareable(new FDetailCustomization_NotitiaSaveDefinition);
}

void FDetailCustomization_NotitiaSaveDefinition::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& Cat = DetailBuilder.EditCategory(FName("ClassPropertySaveMatrix"), INVTEXT("Class Property Save Matrix"));
	Cat.AddCustomRow(INVTEXT("MyRow"));
	return;
}
