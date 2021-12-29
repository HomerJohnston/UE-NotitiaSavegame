#pragma once

#include "CoreMinimal.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassSelectorNameTypeToDisplay.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerNameTypeToDisplay.h"

class FNotitiaClassViewerInitializationOptions
{

public:
	/** The filter to use on classes in this instance. */
	TSharedPtr<class INotitiaClassViewerFilter> ClassFilter;

	/** Filters so only actors will be displayed. */
	bool bIsActorsOnly;

	/** Filters so only placeable actors will be displayed. Forces bIsActorsOnly to true. */
	bool bIsPlaceableOnly;

	/** Filters so only base blueprints will be displayed. */
	bool bIsBlueprintBaseOnly;

	/** Shows unloaded blueprints. Will not be filtered out based on non-bool filter options. */
	bool bShowUnloadedBlueprints;

	/** Shows a "None" option, only available in Picker mode. */
	bool bShowNoneOption;

	/** true will show the UObject root class. */
	bool bShowObjectRootClass;

	/** If true, root nodes will be expanded by default. */
	bool bExpandRootNodes;

	/** true allows class dynamic loading on selection */
	bool bEnableClassDynamicLoading;

	/** Controls what name is shown for classes */
	ENotitiaClassViewerNameTypeToDisplay NameTypeToDisplay;

	/** The property this class viewer be working on. */
	TSharedPtr<class IPropertyHandle> PropertyHandle;

	/** The passed in property handle will be used to gather referencing assets. If additional referencing assets should be reported, supply them here. */
	TArray<FAssetData> AdditionalReferencingAssets;

	/** true (the default) shows the view options at the bottom of the class picker */
	bool bAllowViewOptions;

	/** true (the default) shows a background border behind the class viewer widget. */
	bool bShowBackgroundBorder = true;

	/** Defines additional classes you want listed in the "Common Classes" section for the picker. */
	TArray<UClass*> ExtraPickerCommonClasses;

	/** false by default, restricts the class picker to only showing editor module classes */
	bool bEditorClassesOnly;

	/** Will set the initially selected row, if possible, to this class when the viewer is created */
	UClass* InitiallySelectedClass;

public:

	/** Constructor */
	
	FNotitiaClassViewerInitializationOptions()	
		: bIsActorsOnly(false)
		, bIsPlaceableOnly(false)
		, bIsBlueprintBaseOnly(false)
		, bShowUnloadedBlueprints(true)
		, bShowNoneOption(false)
		, bShowObjectRootClass(false)
		, bExpandRootNodes(true)
		, bEnableClassDynamicLoading(true)
		, NameTypeToDisplay(ENotitiaClassViewerNameTypeToDisplay::ClassName)
		, bAllowViewOptions(true)
		, bEditorClassesOnly(false)
		, InitiallySelectedClass(nullptr)
	{
	}
};
