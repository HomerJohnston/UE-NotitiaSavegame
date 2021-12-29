#pragma once

#include "CoreMinimal.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerNameTypeToDisplay.h"

class INotitiaUnloadedBlueprintData;
class IPropertyHandle;

class FNotitiaClassViewerNode : public TSharedFromThis<FNotitiaClassViewerNode>
{
	
protected:
	FName ClassFName;
	
	TSharedPtr<FString> ClassName;

	TSharedPtr<FString> ClassDisplayName;

	TArray<TSharedPtr<FNotitiaClassViewerNode>> ChildrenList;

	int32 NumProperties = 0;
	
	bool bIsDirty = true;

	TWeakObjectPtr<UClass> Class;
	
public:
	TWeakObjectPtr<UBlueprint> Blueprint;

	FName ClassPath;

	FName ParentClassPath;

	FName BlueprintAssetPath;

	bool bPassesFilter;

	bool bPassesFilterRegardlessTextFilter;

	TWeakPtr<FNotitiaClassViewerNode> ParentNode;

	TSharedPtr<INotitiaUnloadedBlueprintData> UnloadedBlueprintData;

	TSharedPtr<IPropertyHandle> PropertyHandle;

public:
	
	FNotitiaClassViewerNode(UClass* InClass);

	FNotitiaClassViewerNode(const FString& InClassName, const FString& InClassDisplayName);

	FNotitiaClassViewerNode(const FNotitiaClassViewerNode& InCopyObject);

	TWeakObjectPtr<UClass> GetClassWeakPtr() const;
	
	void SetClass(UClass* NewClass);
	
	void AddChild(TSharedPtr<FNotitiaClassViewerNode> Child);

	void AddUniqueChild(TSharedPtr<FNotitiaClassViewerNode> NewChild);

	TArray<TSharedPtr<FNotitiaClassViewerNode>>& GetChildrenList();
	
	TSharedPtr<FString> GetClassName(bool bUseDisplayName = false) const;

	TSharedPtr<FString> GetClassName(ENotitiaClassViewerNameTypeToDisplay NameType) const;

	FName GetClassFName() const;
	
	int32 GetNumProperties() const;

	int32 GetNumSelectedProperties() const;
	
	bool IsClassPlaceable() const;

	bool IsBlueprintClass() const;

	bool IsEditorOnlyClass() const;

	bool IsRestricted() const;
	
	void MarkDirty();

	bool CleanDirty();

	int32 GetPropertyCount(FProperty* Property);
};
