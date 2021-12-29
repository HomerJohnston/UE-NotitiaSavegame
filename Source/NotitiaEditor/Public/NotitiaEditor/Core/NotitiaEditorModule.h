// Copyright (c) 2020 Kyle J Wilcox (HoJo). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "AssetTypeCategories.h"
#include "IAssetTools.h"

class FPropertyEditorModule;
class FUICommandList;

DECLARE_DELEGATE_OneParam( FNotitiaOnClassPicked, UClass* );
DECLARE_DELEGATE( FNotitiaOnPropertyChanged );

class FNotitiaEditorModule : public IModuleInterface
{
// State ------------------------------------------------------------------------------------------
private:
	/**  */
	static TSharedRef<FWorkspaceItem> MenuRoot;
	
	/** Asset category, used for RMB context "Create Asset" menu */
	static EAssetTypeCategories::Type NotitiaAssetCategory;

	static TMap<const char*, TSharedRef<IPropertyTypeCustomization>(*)()> PropertyCustomizations;
	
	static TMap<const char*, TSharedRef<IDetailCustomization>(*)()> DetailCustomizations;

	static TArray<FName> RegisteredPropertyCustomizations;

	static TArray<FName> RegisteredDetailCustomizations;
	
	/** Store created Asset Type Actions here for unregistering on shutdown */
	TArray<TSharedPtr<IAssetTypeActions>> RegisteredAssetTypeActions;
	
	TSharedPtr<SDockTab> OpenPluginTab;
	
// API --------------------------------------------------------------------------------------------
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

private:
	void RegisterCommands();

	void RegisterAssets();

	void RegisterCustomizations();

	void RegisterWindow();

	void UnregisterAssets();

	void UnregisterCustomizations();

	void UnregisterWindow();

	TSharedRef<SDockTab> OnSpawnEditorTab(const FSpawnTabArgs& SpawnTabArgs);
	
	static void RegisterAssetCategories(IAssetTools& AssetTools);

	void RegisterAssetTypeActions(IAssetTools& AssetTools);

	void RegisterAssetTypeAction(IAssetTools& AssetTools, const TSharedRef<IAssetTypeActions> Action);

	void UnregisterAssetTypeActions(IAssetTools& AssetTools);

	
public:
	static EAssetTypeCategories::Type GetAssetCategory() { return NotitiaAssetCategory; }

// Custom window stuff
public:
	void AddMenuExtension(const FMenuExtensionDelegate& ExtensionDelegate, FName ExtensionHook, const TSharedPtr<FUICommandList>& CommandList = nullptr, EExtensionHook::Position Position = EExtensionHook::Before);

	TSharedRef<FWorkspaceItem> GetMenuRoot() { return MenuRoot; }

private:
	static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);
	
protected:
	TSharedPtr<FUICommandList> PluginCommands;
	TSharedPtr<FExtensibilityManager> LevelEditorMenuExtensibilityManager;
	TSharedPtr<FExtender> MenuExtender;
	void MakePulldownMenu(FMenuBarBuilder& MenuBuilder);
	void FillPulldownMenu(FMenuBuilder& MenuBuilder);
	void PluginButtonClicked();
};



DECLARE_LOG_CATEGORY_EXTERN(LogNotitiaEditor, Log, All);