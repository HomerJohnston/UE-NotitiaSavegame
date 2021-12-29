#include "NotitiaEditor/AssetFactories//AssetFactory_NotitiaSaveDefinition.h"
#include "Notitia/Temp/NotitiaSaveDefinition.h"

#include "NotitiaEditor/Core/NotitiaEditorModule.h"

#include "AssetTypeCategories.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "NOTITIA"

UAssetFactory_NotitiaSaveDefinition::UAssetFactory_NotitiaSaveDefinition()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UNotitiaSaveDefinition::StaticClass();
}

UObject* UAssetFactory_NotitiaSaveDefinition::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	// Create an Object asset of this class (this creates an instance of the class in the Content Folder as a project asset)
	return NewObject<UNotitiaSaveDefinition>(InParent, Class, Name, Flags | RF_Transactional);

	// Create a Blueprint Class asset of this class (this creates a child Blueprint class in the Content Folder, same as right clicking on class in C++ folder and choosing "Create Blueprint Child from Class")
	//return FKismetEditorUtilities::CreateBlueprint(Class, InParent, Name, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

uint32 UAssetFactory_NotitiaSaveDefinition::GetMenuCategories() const
{
	return FNotitiaEditorModule::GetAssetCategory();
}

FText UAssetFactory_NotitiaSaveDefinition::GetDisplayName() const
{
	return LOCTEXT("CLASSNAME", "CLASSNAME");
}

// ================================================================================================
FText FAssetTypeActions_NotitiaSaveDefinition::GetName() const
{
	return INVTEXT("CLASSNAME");
}

FColor FAssetTypeActions_NotitiaSaveDefinition::GetTypeColor() const
{
	return FColor(255, 255, 255);
}

UClass* FAssetTypeActions_NotitiaSaveDefinition::GetSupportedClass() const
{
	return UNotitiaSaveDefinition::StaticClass();
}

uint32 FAssetTypeActions_NotitiaSaveDefinition::GetCategories()
{
	return FNotitiaEditorModule::GetAssetCategory();
}

#undef LOCTEXT_NAMESPACE

