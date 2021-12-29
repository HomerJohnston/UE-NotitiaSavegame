// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "NotitiaEditor/Core/NotitiaEditorModule.h"

#include "AssetToolsModule.h"
#include "LevelEditor.h"
#include "NotitiaEditor/Core/NotitiaEditorCommands.h"
#include "NotitiaEditor/Core/NotitiaEditorStyle.h"
#include "NotitiaEditor/Core/NotitiaEditorWindow.h"
#include "PropertyEditorModule.h"
#include "ToolMenus.h"
#include "Styling/SlateStyle.h"
#include "Interfaces/IPluginManager.h"

#include "NotitiaEditor/AssetFactories/AssetFactory_NotitiaSaveDefinition.h"
#include "NotitiaEditor/EditorCustomizations/DetailCustomization_NotitiaSaveDefinition.h"
#include "NotitiaEditor/EditorCustomizations/PropertyCustomization_NotitiaPropertySelector.h"
//#include "EditorCustomizations/PropertyCustomization_NotitiaSaveProperty.h"

#include "NotitiaEditor/ClassViewer/NotitiaClassViewer.h"
#include "NotitiaEditor/EditorCustomizations/DetailCustomization_NotitiaDeveloperSettings.h"
#include "Styling/SlateStyleRegistry.h"

// ------------------------------------------------------------------------------------------------
IMPLEMENT_MODULE(FNotitiaEditorModule, NotitiaEditor)
DEFINE_LOG_CATEGORY(LogNotitiaEditor);
#define LOCTEXT_NAMESPACE "NotitiaEditorModule"

// ------------------------------------------------------------------------------------------------
#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( FNotitiaEditorModule::InContent( RelativePath, ".png" ), __VA_ARGS__ )
#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) FSlateBoxBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

// ------------------------------------------------------------------------------------------------
#pragma region Notitia_EDITOR_CUSTOMIZATION_MACROS
#define REGISTER_PROP_CUSTOMIZATION(_MODULE_, _NAME_) _MODULE_##.RegisterCustomPropertyTypeLayout("Notitia" ###_NAME_##, FOnGetPropertyTypeCustomizationInstance::CreateStatic(FPropertyCustomization_Notitia ## _NAME_ ## ::MakeInstance))
#define REGISTER_CLASS_CUSTOMIZATION(_MODULE_, _NAME_) _MODULE_##.RegisterCustomClassLayout("Notitia" ###_NAME_##, FOnGetDetailCustomizationInstance::CreateStatic(&FDetailCustomization_Notitia ##_NAME_## ::MakeInstance))
#define REGISTER_ASSET_TYPE_ACTION(_MODULE_, _NAME_) RegisterAssetTypeAction(##_MODULE_##, MakeShareable(new FAssetTypeActions_Notitia ##_NAME_##()))
#define UNREGISTER_PROP_CUSTOMIZATION(_MODULE_, _NAME_) _MODULE_##.UnregisterCustomPropertyTypeLayout("Notitia" ###_NAME_##)
#define UNREGISTER_CLASS_CUSTOMIZATION(_MODULE_, _NAME_) _MODULE_##.UnregisterCustomClassLayout("Notitia" ###_NAME_##)
#pragma endregion

// ------------------------------------------------------------------------------------------------
TSharedRef<FWorkspaceItem> FNotitiaEditorModule::MenuRoot = FWorkspaceItem::NewGroup(FText::FromString("Menu Root Test"));
EAssetTypeCategories::Type FNotitiaEditorModule::NotitiaAssetCategory;

TArray<FName> FNotitiaEditorModule::RegisteredPropertyCustomizations;
TArray<FName> FNotitiaEditorModule::RegisteredDetailCustomizations;

static const FName NotitiaEditorWindowTabName("NotitiaEditorWindow");

TMap<const char*, TSharedRef<IPropertyTypeCustomization>(*)()> FNotitiaEditorModule::PropertyCustomizations
{
	//{ "NotitiaPropertySelector", &FPropertyCustomization_NotitiaPropertySelector::MakeInstance }
};

TMap<const char*, TSharedRef<IDetailCustomization>(*)()> FNotitiaEditorModule::DetailCustomizations
{
	{ "NotitiaDeveloperSettings", &FDetailCustomization_NotitiaDeveloperSettings::MakeInstance }
};

// ------------------------------------------------------------------------------------------------
void FNotitiaEditorModule::StartupModule()
{
	FNotitiaEditorStyle::Initialize();

	RegisterCommands();
	
	RegisterAssets();

	RegisterCustomizations();

	RegisterWindow();
}

void FNotitiaEditorModule::ShutdownModule()
{	
	UnregisterAssets();

	UnregisterCustomizations();
	
	FNotitiaEditorStyle::Shutdown();

	SNotitiaClassViewer::DestroyClassHierarchy();
}

void FNotitiaEditorModule::RegisterCommands()
{
	FNotitiaEditorCommands::Register();
}

void FNotitiaEditorModule::RegisterAssets()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		RegisterAssetCategories(AssetToolsModule);
		RegisterAssetTypeActions(AssetToolsModule);
	}
}

void FNotitiaEditorModule::RegisterAssetCategories(IAssetTools& AssetTools)
{
	NotitiaAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Notitia")), LOCTEXT("Notitia", "Notitia"));
}

void FNotitiaEditorModule::RegisterAssetTypeActions(IAssetTools& AssetTools)
{
	RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_NotitiaSaveDefinition()));
}

void FNotitiaEditorModule::RegisterAssetTypeAction(IAssetTools& AssetTools, const TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	RegisteredAssetTypeActions.Add(Action);
}

void FNotitiaEditorModule::RegisterCustomizations()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		
		for (TPair<const char*, TSharedRef<IPropertyTypeCustomization>(*)()>& kvp : PropertyCustomizations)
		{
			PropertyEditorModule.RegisterCustomPropertyTypeLayout(kvp.Key, FOnGetPropertyTypeCustomizationInstance::CreateStatic(kvp.Value));
			RegisteredPropertyCustomizations.Add(kvp.Key);
		}

		for (TPair<const char*, TSharedRef<IDetailCustomization>(*)()>& kvp : DetailCustomizations)
		{
			PropertyEditorModule.RegisterCustomClassLayout(kvp.Key, FOnGetDetailCustomizationInstance::CreateStatic(kvp.Value));
			RegisteredDetailCustomizations.Add(kvp.Key);
		}
		
		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}
}

void FNotitiaEditorModule::RegisterWindow()
{
	if (!IsRunningCommandlet())
	{
		MenuExtender = MakeShareable(new FExtender);
		//MenuExtender->AddMenuBarExtension("Window", EExtensionHook::After, nullptr, FMenuBarExtensionDelegate::CreateRaw(this, &FNotitiaEditorModule::MakePulldownMenu));
		//MenuExtender->AddMenuExtension(FName("Test"), EExtensionHook::After, nullptr, FMenuExtensionDelegate::CreateRaw(this, &FNotitiaEditorModule::FillPulldownMenu));

		if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
		{
			FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");	
			LevelEditorMenuExtensibilityManager = LevelEditorModule.GetMenuExtensibilityManager();
			LevelEditorMenuExtensibilityManager->AddExtender(MenuExtender);
		}

		
		// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
		FToolMenuOwnerScoped OwnerScoped(this);
		PluginCommands = MakeShareable(new FUICommandList);
		PluginCommands->MapAction(FNotitiaEditorCommands::Get().OpenPluginWindow, FExecuteAction::CreateRaw(this, &FNotitiaEditorModule::PluginButtonClicked), FCanExecuteAction());

		{
			UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.LevelToolbarQuickSettings");

			if (Menu)
			{
				FToolMenuSection& Section = Menu->FindOrAddSection("ProjectSettingsSection");
				{
					Section.AddMenuEntryWithCommandList(FNotitiaEditorCommands::Get().OpenPluginWindow, PluginCommands);
				}
			}
		}

		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(NotitiaEditorWindowTabName, FOnSpawnTab::CreateRaw(this, &FNotitiaEditorModule::OnSpawnEditorTab))
		.SetDisplayName(LOCTEXT("FNotitiaEditorWindowTabTitle", "NotitiaEditorWindow"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	}
}

void FNotitiaEditorModule::UnregisterAssets()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		UnregisterAssetTypeActions(AssetTools);
	}
}

void FNotitiaEditorModule::UnregisterCustomizations()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

		for (FName x : RegisteredPropertyCustomizations)
		{
			PropertyEditorModule.UnregisterCustomPropertyTypeLayout(x);	
		}
		
		for (FName x : RegisteredDetailCustomizations)
		{
			PropertyEditorModule.UnregisterCustomClassLayout(x);
		}
		
		PropertyEditorModule.NotifyCustomizationModuleChanged();
		
		RegisteredPropertyCustomizations.Empty();
		RegisteredDetailCustomizations.Empty();
	}
}

void FNotitiaEditorModule::UnregisterWindow()
{
}

TSharedRef<SDockTab> FNotitiaEditorModule::OnSpawnEditorTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return FNotitiaEditorWindow::Test();
}

// ------------------------------------------------------------------------------------------------


void FNotitiaEditorModule::UnregisterAssetTypeActions(IAssetTools& AssetTools)
{
	for (int32 i = 0; i < RegisteredAssetTypeActions.Num(); i++)
	{
		AssetTools.UnregisterAssetTypeActions(RegisteredAssetTypeActions[i].ToSharedRef());
	}
	
	RegisteredAssetTypeActions.Empty();
}

// ------------------------------------------------------------------------------------------------

void FNotitiaEditorModule::AddMenuExtension(const FMenuExtensionDelegate& ExtensionDelegate, FName ExtensionHook, const TSharedPtr<FUICommandList>& CommandList, EExtensionHook::Position Position)
{
	MenuExtender->AddMenuExtension(ExtensionHook, Position, CommandList, ExtensionDelegate);
}

void FNotitiaEditorModule::MakePulldownMenu(FMenuBarBuilder& MenuBuilder)
{
	MenuBuilder.AddPullDownMenu(
		FText::FromString("Example1"),
		FText::FromString("Example2"),
		FNewMenuDelegate::CreateRaw(this, &FNotitiaEditorModule::FillPulldownMenu),
		"Example3",
		FName(TEXT("Example4"))
	);
}

void FNotitiaEditorModule::FillPulldownMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("ExampleSection1", FText::FromString("Section1"));
	MenuBuilder.AddMenuSeparator(FName("Section 1"));
	MenuBuilder.EndSection();
	
	MenuBuilder.BeginSection("ExampleSection2", FText::FromString("Section2"));
	MenuBuilder.AddMenuSeparator(FName("Section 2"));
	MenuBuilder.EndSection();
}

void FNotitiaEditorModule::PluginButtonClicked()
{
	OpenPluginTab = FGlobalTabmanager::Get()->TryInvokeTab(NotitiaEditorWindowTabName);
}

FString FNotitiaEditorModule::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("Notitia"))->GetContentDir();
	FString s = (ContentDir / RelativePath) + Extension;
	return s;
}



#undef LOCTEXT_NAMESPACE
