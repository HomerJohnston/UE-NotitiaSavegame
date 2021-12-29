#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Misc/TextFilterExpressionEvaluator.h"

class INotitiaUnloadedBlueprintData;
class FNotitiaClassViewerNode;
class FNotitiaClassViewerInitializationOptions;
class FNotitiaClassViewerFilterFuncs;
class INotitiaClassViewerFilter
{
public:
	virtual ~INotitiaClassViewerFilter(){}

	virtual bool IsClassAllowed(const FNotitiaClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FNotitiaClassViewerFilterFuncs> InFilterFuncs) = 0;

	virtual bool IsUnloadedClassAllowed(const FNotitiaClassViewerInitializationOptions& InInitOptions, const TSharedRef<const INotitiaUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FNotitiaClassViewerFilterFuncs> InFilterFuncs) = 0;
};

class FNotitiaClassViewerFilter : public INotitiaClassViewerFilter
{
public:
	FNotitiaClassViewerFilter(const FNotitiaClassViewerInitializationOptions& InInitOptions);
	
public:
	TArray<UClass*> InternalClasses;
	TArray<FDirectoryPath> InternalPaths;

	TSharedRef<FTextFilterExpressionEvaluator> TextFilter;
	TSharedRef<FNotitiaClassViewerFilterFuncs> FilterFunctions;
	TSharedPtr<IAssetReferenceFilter> AssetReferenceFilter;

	const IAssetRegistry& AssetRegistry;

	
public:
	
	virtual bool IsNodeAllowed(const FNotitiaClassViewerInitializationOptions& InInitOptions, const TSharedRef<FNotitiaClassViewerNode>& Node, const bool bCheckTextFilter);

	virtual bool IsClassAllowed(const FNotitiaClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FNotitiaClassViewerFilterFuncs> InFilterFuncs) override;
	virtual bool IsClassAllowed(const FNotitiaClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FNotitiaClassViewerFilterFuncs> InFilterFuncs, const bool bCheckTextFilter);
	
	virtual bool IsUnloadedClassAllowed(const FNotitiaClassViewerInitializationOptions& InInitOptions, const TSharedRef<const INotitiaUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FNotitiaClassViewerFilterFuncs> InFilterFuncs) override;
	virtual bool IsUnloadedClassAllowed(const FNotitiaClassViewerInitializationOptions& InInitOptions, const TSharedRef<const INotitiaUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FNotitiaClassViewerFilterFuncs> InFilterFuncs, const bool bCheckTextFilter);

	static bool CanCreateBlueprintOfClass(UClass* Class);

	static bool IsPlaceable(const UClass* Class);
	
	static bool IsPlaceable(const TSharedRef<const INotitiaUnloadedBlueprintData>& InBlueprintData);

	static bool IsBrush(const UClass* Class);
	static bool IsBrush(const TSharedRef<const INotitiaUnloadedBlueprintData>& InBlueprintData);

	static bool CheckIfBlueprintBase(const TSharedRef<const INotitiaUnloadedBlueprintData>& InBlueprintData);
	
	static bool PassesTextFilter(const FString& InTestString, const TSharedRef<FTextFilterExpressionEvaluator>& InTextFilter);
};

class NOTITIAEDITOR_API FNotitiaClassViewerFilterFuncs
{
	
};