#include "NotitiaEditor/ClassViewer/NotitiaClassViewerFilter.h"

#include "ClassViewerFilter.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerInitializationOptions.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerNode.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerSettings.h"
#include "NotitiaEditor/ClassViewer/NotitiaUnloadedBlueprintDataInterface.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/KismetEditorUtilities.h"

FNotitiaClassViewerFilter::FNotitiaClassViewerFilter(const FNotitiaClassViewerInitializationOptions& InInitOptions) :
	TextFilter(MakeShared<FTextFilterExpressionEvaluator>(ETextFilterExpressionEvaluatorMode::BasicString)),
	FilterFunctions(MakeShared<FNotitiaClassViewerFilterFuncs>()),
	AssetRegistry(FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get())
{
	if (!GEditor)
	{
		return;
	}

	FAssetReferenceFilterContext AssetReferenceFilterContext;

	AssetReferenceFilterContext.ReferencingAssets = InInitOptions.AdditionalReferencingAssets;

	if (InInitOptions.PropertyHandle.IsValid())
	{
		TArray<UObject*> ReferencingObjects;

		InInitOptions.PropertyHandle->GetOuterObjects(ReferencingObjects);

		for (UObject* ReferencingObject : ReferencingObjects)
		{
			AssetReferenceFilterContext.ReferencingAssets.Add(FAssetData(ReferencingObject));
		}
	}

	AssetReferenceFilter = GEditor->MakeAssetReferenceFilter(AssetReferenceFilterContext);
}

bool FNotitiaClassViewerFilter::IsNodeAllowed(const FNotitiaClassViewerInitializationOptions& InInitOptions, const TSharedRef<FNotitiaClassViewerNode>& Node, const bool bCheckTextFilter)
{
	if (Node->GetClassWeakPtr().IsValid())
	{
		return IsClassAllowed(InInitOptions, Node->GetClassWeakPtr().Get(), FilterFunctions, bCheckTextFilter);
	}
	else if (InInitOptions.bShowUnloadedBlueprints && Node->UnloadedBlueprintData.IsValid())
	{
		return IsUnloadedClassAllowed(InInitOptions, Node->UnloadedBlueprintData.ToSharedRef(), FilterFunctions, bCheckTextFilter);
	}
	
	return false;
}

bool FNotitiaClassViewerFilter::IsClassAllowed(const FNotitiaClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FNotitiaClassViewerFilterFuncs> InFilterFuncs)
{
	const bool bCheckTextFilter = true;
	return IsClassAllowed(InInitOptions, InClass, InFilterFuncs, bCheckTextFilter);
}

bool FNotitiaClassViewerFilter::IsClassAllowed(const FNotitiaClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FNotitiaClassViewerFilterFuncs> InFilterFuncs, const bool bCheckTextFilter)
{
	if (InInitOptions.bIsActorsOnly && !InClass->IsChildOf(AActor::StaticClass()))
	{
		return false;
	}

	bool bPassesBlueprintBaseFilter = !InInitOptions.bIsBlueprintBaseOnly || CanCreateBlueprintOfClass(const_cast<UClass*>(InClass));

	if (!bPassesBlueprintBaseFilter)
		return false;
	
	bool bPassesEditorClassFilter = !InInitOptions.bEditorClassesOnly || IsEditorOnlyObject(InClass);

	if (!bPassesEditorClassFilter)
		return false;
	
	static const FString DeveloperPathWithSlash = FPackageName::FilenameToLongPackageName(FPaths::GameDevelopersDir());
	static const FString UserDeveloperPathWithSlash = FPackageName::FilenameToLongPackageName(FPaths::GameUserDeveloperDir());
	const FString GeneratedClassPathString = InClass->GetPathName();

	const UNotitiaClassViewerSettings* ClassViewerSettings = GetDefault<UNotitiaClassViewerSettings>();

	bool bPassesDeveloperFilter = true;

	ENotitiaClassViewerDeveloperType AllowedDeveloperType = ClassViewerSettings->DeveloperFolderType;

	if (AllowedDeveloperType == ENotitiaClassViewerDeveloperType::CVDT_None)
	{
		bPassesDeveloperFilter = !GeneratedClassPathString.StartsWith(DeveloperPathWithSlash);
	}
	else if (AllowedDeveloperType == ENotitiaClassViewerDeveloperType::CVDT_CurrentUser)
	{
		if (GeneratedClassPathString.StartsWith(DeveloperPathWithSlash))
		{
			bPassesDeveloperFilter = GeneratedClassPathString.StartsWith(UserDeveloperPathWithSlash);
		}
	}

	if (!bPassesDeveloperFilter)
		return false;

	bool bPassesAllowedClasses = true;
	if (ClassViewerSettings->AllowedClasses.Num() > 0)
	{
		if (!ClassViewerSettings->AllowedClasses.Contains(GeneratedClassPathString))
		{
			bPassesAllowedClasses = false;
		}
	}

	if (!bPassesAllowedClasses)
		return false;

	bool bPassesPlaceableFilter = true;
	if (InInitOptions.bIsPlaceableOnly)
	{
		bPassesPlaceableFilter = IsPlaceable(InClass) && !IsBrush(InClass);
	}

	if (!bPassesPlaceableFilter)
		return false;

	bool bPassesCustomFilter = true;
	if (InInitOptions.ClassFilter.IsValid())
	{
		bPassesCustomFilter = InInitOptions.ClassFilter->IsClassAllowed(InInitOptions, InClass, FilterFunctions);
	}

	if (!bPassesCustomFilter)
		return false;

	bool bPassesTextFilter = true;
	if (bCheckTextFilter && (TextFilter->GetFilterType() != ETextFilterExpressionType::Empty))
	{
		FString ClassNameWithCppPrefix = FString::Printf(TEXT("%s%s"), InClass->GetPrefixCPP(), *InClass->GetName());

		bPassesTextFilter = PassesTextFilter(InClass->GetName(), TextFilter) || PassesTextFilter(ClassNameWithCppPrefix, TextFilter);

		if (!bPassesTextFilter && InClass->HasAnyClassFlags(CLASS_Deprecated))
		{
			ClassNameWithCppPrefix.RemoveAt(1, 11);
			bPassesTextFilter |= PassesTextFilter(ClassNameWithCppPrefix, TextFilter);
		}
	}

	if (!bPassesTextFilter)
		return false;

	bool bPassesAssetReferenceFilter = true;
	if (AssetReferenceFilter.IsValid() && !InClass->IsNative())
	{
		bPassesAssetReferenceFilter = AssetReferenceFilter->PassesFilter(FAssetData(InClass));
	}

	if (!bPassesAssetReferenceFilter)
		return false;

	return true;
}

bool FNotitiaClassViewerFilter::IsUnloadedClassAllowed(const FNotitiaClassViewerInitializationOptions& InInitOptions, const TSharedRef<const INotitiaUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FNotitiaClassViewerFilterFuncs> InFilterFuncs)
{
	const bool bCheckTextFilter = true;
	return IsUnloadedClassAllowed(InInitOptions, InUnloadedClassData, InFilterFuncs, bCheckTextFilter);
}

bool FNotitiaClassViewerFilter::IsUnloadedClassAllowed(const FNotitiaClassViewerInitializationOptions& InInitOptions, const TSharedRef<const INotitiaUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FNotitiaClassViewerFilterFuncs> InFilterFuncs, const bool bCheckTextFilter)
{
	if (InInitOptions.bIsActorsOnly && !InUnloadedClassData->IsChildOf(AActor::StaticClass()))
	{
		return false;
	}

	const bool bIsBlueprintBase = CheckIfBlueprintBase(InUnloadedClassData);
	const bool bPassesBlueprintBaseFilter = !InInitOptions.bIsBlueprintBaseOnly || bIsBlueprintBase;

	static const FString DeveloperPathWithSlash = FPackageName::FilenameToLongPackageName(FPaths::GameDevelopersDir());
	static const FString UserDeveloperPathWithSlash = FPackageName::FilenameToLongPackageName(FPaths::GameUserDeveloperDir());
	FString GeneratedClassPathString = InUnloadedClassData->GetClassPath().ToString();

	bool bPassesDeveloperFilter = true;

	const UClassViewerSettings* ClassViewerSettings = GetDefault<UClassViewerSettings>();
	EClassViewerDeveloperType AllowedDeveloperType = ClassViewerSettings->DeveloperFolderType;
	if (AllowedDeveloperType == EClassViewerDeveloperType::CVDT_None)
	{
		bPassesDeveloperFilter = !GeneratedClassPathString.StartsWith(DeveloperPathWithSlash);
	}
	else if (AllowedDeveloperType == EClassViewerDeveloperType::CVDT_CurrentUser)
	{
		if (GeneratedClassPathString.StartsWith(DeveloperPathWithSlash))
		{
			bPassesDeveloperFilter = GeneratedClassPathString.StartsWith(UserDeveloperPathWithSlash);
		}
	}

	// The INI files declare classes and folders that are considered internal only. Does this class match any of those patterns?
	// INI path: /Script/ClassViewer.ClassViewerProjectSettings
	bool bPassesInternalFilter = true;
	if (!ClassViewerSettings->DisplayInternalClasses)
	{
		for (const FDirectoryPath& DirPath : InternalPaths)
		{
			if (GeneratedClassPathString.StartsWith(DirPath.Path))
			{
				bPassesInternalFilter = false;
				break;
			}
		}
	}

	// The INI files can contain a list of globally allowed classes - if it does, then only classes whose names match will be shown.
	bool bPassesAllowedClasses = true;
	if (ClassViewerSettings->AllowedClasses.Num() > 0)
	{
		if (!ClassViewerSettings->AllowedClasses.Contains(GeneratedClassPathString))
		{
			bPassesAllowedClasses = false;
		}
	}

	bool bPassesPlaceableFilter = true;
	if (InInitOptions.bIsPlaceableOnly)
	{
		bPassesPlaceableFilter = IsPlaceable(InUnloadedClassData) && !IsBrush(InUnloadedClassData);
	}

	bool bPassesCustomFilter = true;
	if (InInitOptions.ClassFilter.IsValid())
	{
		bPassesCustomFilter = InInitOptions.ClassFilter->IsUnloadedClassAllowed(InInitOptions, InUnloadedClassData, FilterFunctions);
	}

	const bool bPassesTextFilter = PassesTextFilter(*InUnloadedClassData->GetClassName().Get(), TextFilter);

	bool bPassesAssetReferenceFilter = true;
	if (AssetReferenceFilter.IsValid() && bIsBlueprintBase)
	{
		FName BlueprintPath = FName(*GeneratedClassPathString.LeftChop(2)); // Chop off _C
		bPassesAssetReferenceFilter = AssetReferenceFilter->PassesFilter(AssetRegistry.GetAssetByObjectPath(BlueprintPath));
	}

	bool bPassesFilter = bPassesPlaceableFilter && bPassesBlueprintBaseFilter
		&& bPassesDeveloperFilter && bPassesInternalFilter && bPassesCustomFilter 
		&& (!bCheckTextFilter || bPassesTextFilter) && bPassesAssetReferenceFilter;

	return bPassesFilter;
}

bool FNotitiaClassViewerFilter::CanCreateBlueprintOfClass(UClass* Class)
{
	bool bIsClassDeprecated = Class->HasAnyClassFlags(CLASS_Deprecated);
	Class->ClassFlags &= ~CLASS_Deprecated;

	bool bCanCreateBlueprintOfClass = FKismetEditorUtilities::CanCreateBlueprintOfClass(Class);

	if (bIsClassDeprecated)
	{
		Class->ClassFlags |= CLASS_Deprecated;
	}

	return bCanCreateBlueprintOfClass;
}

bool FNotitiaClassViewerFilter::IsPlaceable(const UClass* Class)
{
	return !Class->HasAnyClassFlags(CLASS_Abstract | CLASS_NotPlaceable) && Class->IsChildOf(AActor::StaticClass());
}

bool FNotitiaClassViewerFilter::IsPlaceable(const TSharedRef<const INotitiaUnloadedBlueprintData>& InBlueprintData)
{
	return !InBlueprintData->HasAnyClassFlags(CLASS_Abstract | CLASS_NotPlaceable) && InBlueprintData->IsChildOf(AActor::StaticClass());
}

bool FNotitiaClassViewerFilter::IsBrush(const UClass* Class)
{
	return Class->IsChildOf(ABrush::StaticClass());
}

bool FNotitiaClassViewerFilter::IsBrush(const TSharedRef<const INotitiaUnloadedBlueprintData>& InBlueprintData)
{
	return InBlueprintData->IsChildOf(ABrush::StaticClass());
}

bool FNotitiaClassViewerFilter::CheckIfBlueprintBase(const TSharedRef<const INotitiaUnloadedBlueprintData>& InBlueprintData)
{
	if (InBlueprintData->IsNormalBlueprintType())
	{
		bool bAllowDerivedBlueprints = false;
		GConfig->GetBool(TEXT("Kismet"), TEXT("AllowDerivedBlueprints"), bAllowDerivedBlueprints, GEngineIni);
		return bAllowDerivedBlueprints;
	}
	
	return false;
}

bool FNotitiaClassViewerFilter::PassesTextFilter(const FString& InTestString, const TSharedRef<FTextFilterExpressionEvaluator>& InTextFilter)
{
	return InTextFilter->TestTextFilter(FBasicStringFilterExpressionContext(InTestString));
}
