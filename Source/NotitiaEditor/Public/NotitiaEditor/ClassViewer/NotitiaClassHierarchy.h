#pragma once

#include "CoreMinimal.h"
#include "NotitiaEditor/ClassViewer/NotitiaClassViewerNode.h"

class FNotitiaClassHierarchy
{
protected:
	TSharedPtr<FNotitiaClassViewerNode> ObjectClassRoot;

protected:
	FDelegateHandle OnFilesLoadedRequestPopulateClassHierarchyDelegateHandle;
	FDelegateHandle OnBlueprintCompiledRequestPopulateClassHierarchyDelegateHandle;
	FDelegateHandle OnClassPackageLoadedOrUnloadedRequestPopulateClassHierarchyDelegateHandle;

	static bool bPopulateClassHierarchy;
	
public:
	FNotitiaClassHierarchy();
	
	~FNotitiaClassHierarchy();

	void PopulateClassHierarchy();

	void AddChildren_NoFilter(TSharedPtr<FNotitiaClassViewerNode>& InOutRootNode, TMap<FName, TSharedPtr<FNotitiaClassViewerNode>>& InOutClassPathToNode);
	
	void LoadUnloadedTagData(TSharedPtr<FNotitiaClassViewerNode>& InOutClassViewerNode, const FAssetData& InAssetData);

	const TSharedPtr<FNotitiaClassViewerNode> GetObjectRootNode() const;

	TSharedPtr<FNotitiaClassViewerNode> FindNodeByGeneratedClassPath(const TSharedPtr<FNotitiaClassViewerNode>& InRootNode, FName InGeneratedClassPath);
	
	void UpdateClassInNode(FName InGeneratedClassPath, UClass* InNewClass, UBlueprint* InNewBluePrint);

	static void RequestPopulateClassHierarchy();

	void AddAsset(const FAssetData& InAddedAssetData);

	void FindClass(TSharedPtr<FNotitiaClassViewerNode> InOutClassNode);

	TSharedPtr<FNotitiaClassViewerNode> FindParent(const TSharedPtr<FNotitiaClassViewerNode>& InRootNode, FName InParentClassName, const UClass* InParentClass);

	void SortChildren(TSharedPtr<FNotitiaClassViewerNode>& InRootNode);
	
	void RemoveAsset(const FAssetData& InRemovedAssetData);

	bool FindAndRemoveNodeByClassPath(const TSharedPtr<FNotitiaClassViewerNode>& InRootNode, FName InClassPath);
	
	void OnHotReload(bool bWasTriggeredAutomatically);

	static void OnModulesChanged(FName ModuleThatChanged, EModuleChangeReason ReasonForChange);
};
